/**
 * @file distance_sensor.c
 * @brief HC-SR04 Ultrasonic Distance Sensor Driver Implementation
 *
 * DUAL-QUEUE REAL-TIME ARCHITECTURE:
 * ==================================
 *
 * This implementation follows real-time system design principles:
 *
 * 1. INTERRUPT SERVICE ROUTINE (ISR):
 *    - Executes in IRAM for deterministic timing
 *    - Only captures timestamps using esp_timer_get_time() (microsecond precision)
 *    - No floating-point calculations (violates real-time constraints)
 *    - Minimal processing to reduce interrupt latency
 *    - Uses xQueueSendFromISR for thread-safe communication
 *
 * 2. SENSOR TASK:
 *    - Triggers measurements at configurable intervals (default: 100ms)
 *    - Blocks on raw_measurement_queue with timeout
 *    - Performs floating-point calculations safely in task context
 *    - Validates measurements and handles errors
 *    - Applies exponential moving average (EMA) filter for smooth readings
 *    - Implements automatic queue overflow handling
 *    - Yields CPU between measurements (vTaskDelay)
 *
 * 3. PUBLIC API:
 *    - Blocking access to measurements (waits for new data)
 *    - Race-condition free (no shared variables)
 *    - Queue-based communication eliminates synchronization issues
 *
 * TIMING CONSIDERATIONS:
 * =====================
 * - HC-SR04 sends 8x 40kHz ultrasonic bursts (takes ~200μs)
 * - Maximum range 400cm = ~23ms echo time
 * - Minimum range 2cm = ~117μs echo time
 * - Temperature affects speed of sound: 331.3 + 0.606 * temp (m/s)
 * - ESP32 timer resolution: 1μs (esp_timer_get_time)
 *
 * QUEUE ARCHITECTURE:
 * ==================
 * Raw Queue (ISR → Task):
 *   - Size: 2 (minimal buffering, ISR priority)
 *   - Contains: Raw timestamps only
 *   - Timeout: 30ms (accommodates max range + margin)
 *
 * Processed Queue (Task → API):
 *   - Size: 5 (buffering for consumer applications)
 *   - Contains: Calculated distances with metadata
 *   - Overflow: Automatic with statistics tracking
 *
 * COMPARED TO VxWorks:
 * ===================
 * - VxWorks msgQ ≈ FreeRTOS xQueue
 * - VxWorks ISR ≈ ESP32 GPIO ISR with IRAM_ATTR
 * - VxWorks timestamps ≈ esp_timer_get_time()
 * - Priority inversion protection via proper task priorities
 */

#include "distance_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_check.h"
#include <stddef.h> // For offsetof macro

// Debug configuration - comment out to disable detailed debug output
// When defined: Shows struct layout, memory dumps, and detailed EMA filter tracing
// When undefined: Only essential operational logs (initialization, errors, measurements)
// #define DISTANCE_SENSOR_DEBUG

// Measurement logging - comment out to disable frequent measurement logs (every 100ms)
// When defined: Shows each raw/smoothed measurement result
// When undefined: Only initialization, errors, and status messages
// #define DISTANCE_SENSOR_MEASUREMENT_LOGS

static const char *TAG = "distance_sensor";

// Configuration
static distance_sensor_config_t sensor_config;

// Task and queue handles
static TaskHandle_t sensor_task_handle = NULL;
static QueueHandle_t raw_measurement_queue = NULL;       // ISR → Sensor Task
static QueueHandle_t processed_measurement_queue = NULL; // Sensor Task → API

// Statistics
static uint32_t queue_overflow_counter = 0;

// Exponential Moving Average (EMA) filter state
static float previous_smoothed_value = 0.0f;
static bool smoothing_initialized = false;

// ISR state
static volatile uint64_t echo_start_time = 0;
static volatile bool measurement_in_progress = false;

// Queue sizes
#define RAW_QUEUE_SIZE 2       // Minimal ISR buffering
#define PROCESSED_QUEUE_SIZE 5 // API consumer buffering

/*
 * DATA FLOW DIAGRAM:
 * ==================
 *
 * [Sensor Task]                [ISR]                    [API Consumer]
 *      |                        |                           |
 *      | 1. Trigger pulse       |                           |
 *      |--->(GPIO14 HIGH)       |                           |
 *      |                        |                           |
 *      | 2. Wait on queue       | 3. RISING edge            |
 *      |    (blocking)          |    echo_start_time        |
 *      |                        |                           |
 *      |                        | 4. FALLING edge           |
 *      |                        |    raw_data.timestamps    |
 *      |                        |         |                 |
 *      | 5. Calculate distance  |<--------|                 |
 *      |    (floating-point)    |                           |
 *      |    Validate range      |                           |
 *      |    Apply EMA filter    |                           |
 *      |         |              |                           |
 *      |         v              |                           |
 *      | [processed_queue]------|-------------------------->| 6. get_latest()
 *      |                        |                           |    (blocking)
 *      | 7. Sleep 100ms         |                           |
 *      |    (yield CPU)         |                           |
 *      |         |              |                           |
 *      |<--------|              |                           |
 *
 * QUEUE OVERFLOW HANDLING:
 * - Raw queue: Size 2 (ISR critical, minimal buffering)
 * - Processed queue: Size 5 with automatic overflow (remove oldest)
 * - Statistics tracking for monitoring queue health
 */

// Default configuration
static const distance_sensor_config_t default_config = {
    .trigger_pin = GPIO_NUM_14,
    .echo_pin = GPIO_NUM_15,
    .measurement_interval_ms = 100, // 100ms for normal operation (10 Hz measurement rate)
    .timeout_ms = 30,
    .temperature_celsius = 20.0f,
    .smoothing_alpha = 0.3f}; // 30% new value, 70% previous (good balance)

/**
 * @brief GPIO ISR handler for echo pin - REAL-TIME CRITICAL
 *
 * This function executes in interrupt context and must be IRAM_ATTR for
 * deterministic timing. It only captures timestamps and queues raw data.
 *
 * TIMING REQUIREMENTS:
 * - Must complete within microseconds
 * - No floating-point calculations allowed
 * - No blocking operations
 * - Minimal stack usage
 *
 * HC-SR04 PROTOCOL:
 * - RISING edge: Ultrasonic burst transmitted
 * - FALLING edge: Echo received, calculate pulse width
 * - Pulse width = 2 * distance / speed_of_sound
 */
static void IRAM_ATTR echo_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio_get_level(sensor_config.echo_pin))
    {
        // RISING edge - start timing
        echo_start_time = esp_timer_get_time();
        measurement_in_progress = true;
    }
    else
    {
        // FALLING edge - send raw timestamps
        if (measurement_in_progress)
        {
            distance_raw_measurement_t raw_data = {
                .echo_start_us = echo_start_time,
                .echo_end_us = esp_timer_get_time(),
                .status = DISTANCE_SENSOR_OK};

            // Send to raw queue (non-blocking)
            xQueueSendFromISR(raw_measurement_queue, &raw_data, &xHigherPriorityTaskWoken);
            measurement_in_progress = false;
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Calculate speed of sound based on temperature
 */
static float calculate_speed_of_sound(float temperature_celsius)
{
    // Speed of sound = 331.3 + 0.606 * temperature (m/s)
    // Convert to cm/µs: divide by 10000
    return (331.3f + 0.606f * temperature_celsius) / 10000.0f;
}

/**
 * @brief Apply exponential moving average (EMA) filter for sensor smoothing
 *
 * The EMA filter provides smooth distance readings while maintaining responsiveness
 * to actual changes. It uses minimal memory (just previous value) and is
 * computationally efficient.
 *
 * Formula: smoothed = (alpha * new_value) + ((1 - alpha) * previous_smoothed)
 *
 * Where alpha (smoothing_alpha) controls the balance:
 * - 0.0: Maximum smoothing (100% previous, 0% new) - very stable but slow
 * - 0.3: Balanced smoothing (30% new, 70% previous) - good default
 * - 0.5: Moderate smoothing (50/50 mix)
 * - 1.0: No smoothing (100% new, 0% previous) - raw measurements
 *
 * BENEFITS:
 * - Reduces noise from electrical interference and air currents
 * - Maintains responsiveness to real distance changes
 * - Memory efficient (only stores one previous value)
 * - Computationally fast (just multiply and add)
 * - No startup delay (works from first measurement)
 *
 * @param new_measurement Raw distance measurement in centimeters
 * @return float Smoothed distance value in centimeters
 */
static float apply_ema_filter(float new_measurement)
{
#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: EMA entry - alpha=%.6f", sensor_config.smoothing_alpha);
#endif

    if (!smoothing_initialized)
    {
        // First measurement - initialize filter with raw value
        previous_smoothed_value = new_measurement;
        smoothing_initialized = true;
#ifdef DISTANCE_SENSOR_DEBUG
        ESP_LOGI(TAG, "EMA filter initialized with first measurement: %.2f cm (alpha=%.2f)",
                 new_measurement, sensor_config.smoothing_alpha);
#endif
        return new_measurement;
    }

    // Apply exponential moving average formula
    float smoothed = (sensor_config.smoothing_alpha * new_measurement) +
                     ((1.0f - sensor_config.smoothing_alpha) * previous_smoothed_value);

    // Store smoothed value for next iteration
    previous_smoothed_value = smoothed;

#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "EMA filter: raw=%.2f cm, smoothed=%.2f cm (alpha=%.2f)",
             new_measurement, smoothed, sensor_config.smoothing_alpha);
#endif

    return smoothed;
}

/**
 * @brief Distance sensor task - MEASUREMENT PROCESSING
 *
 * This task implements the main measurement loop:
 * 1. Triggers HC-SR04 with 10μs pulse
 * 2. Waits for ISR to provide raw timestamps (blocking with timeout)
 * 3. Calculates distance using temperature-compensated speed of sound
 * 4. Validates measurement range (2-400cm)
 * 5. Queues processed result for API consumption
 * 6. Sleeps to maintain measurement interval and yield CPU
 *
 * PRIORITY DESIGN:
 * - Higher priority than LED test task (5 vs 2)
 * - Lower priority than ISR (hardware interrupt)
 * - Allows precise timing while yielding to other tasks
 *
 * ERROR HANDLING:
 * - Timeout detection for missing echoes
 * - Range validation for impossible readings
 * - Queue overflow management with statistics
 */
static void distance_sensor_task(void *pvParameters)
{
    distance_raw_measurement_t raw_data;
    float speed_of_sound_cm_per_us = calculate_speed_of_sound(sensor_config.temperature_celsius);

    ESP_LOGI(TAG, "Distance sensor task started (interval: %lu ms, timeout: %lu ms)",
             sensor_config.measurement_interval_ms, sensor_config.timeout_ms);

#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: Task started - smoothing_alpha=%.6f", sensor_config.smoothing_alpha);
#endif

    while (1)
    {
        // 1. Trigger measurement
        gpio_set_level(sensor_config.trigger_pin, 1);
        esp_rom_delay_us(10); // 10µs trigger pulse
        gpio_set_level(sensor_config.trigger_pin, 0);

        // 2. Wait for ISR result with timeout
        if (xQueueReceive(raw_measurement_queue, &raw_data,
                          pdMS_TO_TICKS(sensor_config.timeout_ms)) == pdPASS)
        {

            // 3. Calculate distance from raw timestamps
            uint64_t echo_duration_us = raw_data.echo_end_us - raw_data.echo_start_us;

            // Distance = (echo_time * speed_of_sound) / 2
            float distance_cm = (echo_duration_us * speed_of_sound_cm_per_us) / 2.0f;

            // 4. Validate measurement range and apply smoothing filter
            distance_sensor_error_t status = DISTANCE_SENSOR_OK;
            float final_distance_cm = distance_cm;

            if (distance_cm < 2.0f || distance_cm > 400.0f)
            {
                status = DISTANCE_SENSOR_OUT_OF_RANGE;
                ESP_LOGW(TAG, "Measurement out of range: %.2f cm (no smoothing applied)", distance_cm);
                // Don't apply smoothing to invalid measurements - preserve error info
            }
            else
            {
                // Apply EMA filter only to valid measurements
#ifdef DISTANCE_SENSOR_DEBUG
                ESP_LOGI(TAG, "Applying EMA filter to valid measurement: %.2f cm", distance_cm);
#endif
                final_distance_cm = apply_ema_filter(distance_cm);
            }

            // 5. Create processed measurement
            distance_measurement_t processed = {
                .distance_cm = final_distance_cm,
                .timestamp_us = raw_data.echo_end_us,
                .status = status};

            // 6. Send to processed queue (handle overflow)
            if (xQueueSend(processed_measurement_queue, &processed, 0) != pdTRUE)
            {
                // Queue full - remove oldest measurement and add new one
                distance_measurement_t dummy;
                if (xQueueReceive(processed_measurement_queue, &dummy, 0) == pdTRUE)
                {
                    xQueueSend(processed_measurement_queue, &processed, 0);
                    queue_overflow_counter++;
                    ESP_LOGW(TAG, "Measurement queue overflow (count: %lu)", queue_overflow_counter);
                }
            }

#ifdef DISTANCE_SENSOR_MEASUREMENT_LOGS
#ifdef DISTANCE_SENSOR_MEASUREMENT_LOGS
            ESP_LOGI(TAG, "Distance: raw=%.2f cm, smoothed=%.2f cm (echo: %llu µs)",
                     distance_cm, final_distance_cm, echo_duration_us);
#endif
#endif
        }
        else
        {
            // Timeout - no echo received
            ESP_LOGW(TAG, "Distance measurement timeout");

            distance_measurement_t timeout_measurement = {
                .distance_cm = 0.0f,
                .timestamp_us = esp_timer_get_time(),
                .status = DISTANCE_SENSOR_TIMEOUT};

            // Send timeout result
            if (xQueueSend(processed_measurement_queue, &timeout_measurement, 0) != pdTRUE)
            {
                distance_measurement_t dummy;
                xQueueReceive(processed_measurement_queue, &dummy, 0);
                xQueueSend(processed_measurement_queue, &timeout_measurement, 0);
                queue_overflow_counter++;
            }
        }

        // 7. Sleep until next measurement
        vTaskDelay(pdMS_TO_TICKS(sensor_config.measurement_interval_ms));
    }
}

esp_err_t distance_sensor_init(const distance_sensor_config_t *config)
{
#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: distance_sensor_init called with config=%p", (void *)config);
    ESP_LOGI(TAG, "DEBUG: Struct size: %zu bytes", sizeof(distance_sensor_config_t));
    ESP_LOGI(TAG, "DEBUG: Field offsets: trigger_pin=%zu, echo_pin=%zu, interval=%zu, timeout=%zu, temp=%zu, alpha=%zu",
             offsetof(distance_sensor_config_t, trigger_pin),
             offsetof(distance_sensor_config_t, echo_pin),
             offsetof(distance_sensor_config_t, measurement_interval_ms),
             offsetof(distance_sensor_config_t, timeout_ms),
             offsetof(distance_sensor_config_t, temperature_celsius),
             offsetof(distance_sensor_config_t, smoothing_alpha));
    ESP_LOGI(TAG, "DEBUG: default_config.smoothing_alpha=%.6f (hex: 0x%08lX)",
             default_config.smoothing_alpha, *(uint32_t *)&default_config.smoothing_alpha);
#endif

    // Use default config if none provided
    if (config == NULL)
    {
#ifdef DISTANCE_SENSOR_DEBUG
        ESP_LOGI(TAG, "DEBUG: Using default config");
#endif
        sensor_config = default_config;
    }
    else
    {
#ifdef DISTANCE_SENSOR_DEBUG
        ESP_LOGI(TAG, "DEBUG: Using provided config, smoothing_alpha=%.6f (hex: 0x%08lX)",
                 config->smoothing_alpha, *(uint32_t *)&config->smoothing_alpha);
#endif
        sensor_config = *config;
    }

#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: Config after copy - smoothing_alpha=%.6f (hex: 0x%08lX)",
             sensor_config.smoothing_alpha, *(uint32_t *)&sensor_config.smoothing_alpha);
#endif

    // Validate smoothing alpha parameter
    if (sensor_config.smoothing_alpha < 0.0f)
    {
        sensor_config.smoothing_alpha = 0.0f;
        ESP_LOGW(TAG, "Smoothing alpha cannot be negative, using 0.0 (maximum smoothing)");
    }
    else if (sensor_config.smoothing_alpha > 1.0f)
    {
        sensor_config.smoothing_alpha = 1.0f;
        ESP_LOGW(TAG, "Smoothing alpha cannot exceed 1.0, using 1.0 (no smoothing)");
    }

#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: Config after validation - smoothing_alpha=%.6f", sensor_config.smoothing_alpha);
#endif

    // Initialize EMA filter state
    previous_smoothed_value = 0.0f;
    smoothing_initialized = false;

    ESP_LOGI(TAG, "Initializing distance sensor (trigger: GPIO%d, echo: GPIO%d, smoothing: %.6f)",
             sensor_config.trigger_pin, sensor_config.echo_pin, sensor_config.smoothing_alpha);

    // Configure trigger pin as output
    gpio_config_t trigger_conf = {
        .pin_bit_mask = (1ULL << sensor_config.trigger_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_RETURN_ON_ERROR(gpio_config(&trigger_conf), TAG, "Failed to configure trigger pin");

    // Configure echo pin as input with interrupt
    gpio_config_t echo_conf = {
        .pin_bit_mask = (1ULL << sensor_config.echo_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE};
    ESP_RETURN_ON_ERROR(gpio_config(&echo_conf), TAG, "Failed to configure echo pin");

    // Install GPIO ISR service
    ESP_RETURN_ON_ERROR(gpio_install_isr_service(0), TAG, "Failed to install GPIO ISR service");

    // Add ISR handler for echo pin
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(sensor_config.echo_pin, echo_isr_handler, NULL),
                        TAG, "Failed to add GPIO ISR handler");

    // Create queues
    raw_measurement_queue = xQueueCreate(RAW_QUEUE_SIZE, sizeof(distance_raw_measurement_t));
    if (raw_measurement_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create raw measurement queue");
        return ESP_ERR_NO_MEM;
    }

    processed_measurement_queue = xQueueCreate(PROCESSED_QUEUE_SIZE, sizeof(distance_measurement_t));
    if (processed_measurement_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create processed measurement queue");
        vQueueDelete(raw_measurement_queue);
        return ESP_ERR_NO_MEM;
    }

    // Initialize trigger pin to LOW
    gpio_set_level(sensor_config.trigger_pin, 0);

    ESP_LOGI(TAG, "Distance sensor initialized successfully");
    ESP_LOGI(TAG, "EMA filter configured with alpha=%.2f (%.0f%% new, %.0f%% previous)",
             sensor_config.smoothing_alpha,
             sensor_config.smoothing_alpha * 100.0f,
             (1.0f - sensor_config.smoothing_alpha) * 100.0f);
    return ESP_OK;
}

esp_err_t distance_sensor_start(void)
{
    if (sensor_task_handle != NULL)
    {
        ESP_LOGW(TAG, "Distance sensor task already running");
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        distance_sensor_task,
        "distance_sensor",
        4096, // Stack size
        NULL, // Parameters
        5,    // Priority (higher than LED tests)
        &sensor_task_handle,
        1 // Core 1
    );

    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create distance sensor task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Distance sensor started");
    return ESP_OK;
}

esp_err_t distance_sensor_stop(void)
{
    if (sensor_task_handle == NULL)
    {
        ESP_LOGW(TAG, "Distance sensor task not running");
        return ESP_ERR_INVALID_STATE;
    }

    vTaskDelete(sensor_task_handle);
    sensor_task_handle = NULL;

    ESP_LOGI(TAG, "Distance sensor stopped");
    return ESP_OK;
}

esp_err_t distance_sensor_get_latest(distance_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Block until new measurement is available
    return (xQueueReceive(processed_measurement_queue, measurement, portMAX_DELAY) == pdTRUE)
               ? ESP_OK
               : ESP_FAIL;
}

bool distance_sensor_has_new_measurement(void)
{
    return (uxQueueMessagesWaiting(processed_measurement_queue) > 0);
}

uint32_t distance_sensor_get_queue_overflows(void)
{
    return queue_overflow_counter;
}

bool distance_sensor_is_running(void)
{
    return (sensor_task_handle != NULL);
}
