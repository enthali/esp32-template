/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic.
 *
 * Design is documented in `docs/design/display-design.md` and linked to requirements
 * and design IDs (e.g. REQ-DSP-*, DSN-DSP-*). Keep this header short; full design
 * rationale lives in the design document for traceability.
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

/**
 * @brief Update LED display based on distance measurement
 *
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{
    // Get configuration once for entire function (optimization: avoid multiple config_get_current calls)
    system_config_t config;
    bool config_valid = (config_get_current(&config) == ESP_OK);
    
    // Clear all LEDs first
    led_clear_all();

    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        if (config_valid)
        {
            // Use config directly instead of calling map_distance_to_led (avoids duplicate config_get_current)
            if (measurement->distance_mm >= config.distance_min_mm && measurement->distance_mm <= config.distance_max_mm)
            {
                // Normal range: Calculate LED position directly with integer arithmetic
                uint16_t range_mm = config.distance_max_mm - config.distance_min_mm;
                uint16_t offset_mm = measurement->distance_mm - config.distance_min_mm;
                
                uint16_t led_count = led_get_count();
                // Use integer math with multiplication before division for precision
                int led_index = (int)((offset_mm * (led_count - 1)) / range_mm);
                
                // Ensure within bounds
                if (led_index < 0) led_index = 0;
                if (led_index >= led_count) led_index = led_count - 1;
                
                // Three-zone color scheme based on LED position
                uint16_t zone1_end = led_count / 4;        // 25% boundary
                uint16_t zone2_end = led_count / 2;        // 50% boundary
                
                led_color_t color;
                const char *zone_name;
                if (led_index < zone1_end) {
                    color = LED_COLOR_RED;     // Too close zone
                    zone_name = "too close";
                } else if (led_index < zone2_end) {
                    color = LED_COLOR_GREEN;   // Ideal zone
                    zone_name = "ideal";
                } else {
                    color = LED_COLOR_ORANGE;  // Acceptable zone
                    zone_name = "acceptable";
                }
                
                led_set_pixel(led_index, color);
                ESP_LOGD(TAG, "Distance %d mm → LED %d (%s zone)", measurement->distance_mm, led_index, zone_name);
            }
            else if (measurement->distance_mm < config.distance_min_mm)
            {
                // Too close: Red on first LED
                led_set_pixel(0, LED_COLOR_RED);
                ESP_LOGD(TAG, "Distance %d mm too close → LED 0 red", measurement->distance_mm);
            }
            else
            {
                // Too far: Red on last LED
                uint16_t led_count = led_get_count();
                led_set_pixel(led_count - 1, LED_COLOR_RED);
                ESP_LOGD(TAG, "Distance %d mm too far → LED %d red", measurement->distance_mm, led_count - 1);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to get configuration for display update");
        }
        break;
    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off (already cleared)
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Red on last LED
        {
            uint16_t led_count = led_get_count();
            led_set_pixel(led_count - 1, LED_COLOR_RED);
            ESP_LOGD(TAG, "Sensor out of range → LED %d red", led_count - 1);
        }
        break;

    case DISTANCE_SENSOR_NO_ECHO:
    case DISTANCE_SENSOR_INVALID_READING:
    default:
        // Other errors: Red on first LED
        led_set_pixel(0, LED_COLOR_RED);
        ESP_LOGD(TAG, "Sensor error → LED 0 red");
        break;
    }

    // Update physical LEDs
    led_show();
}

/**
 * @brief Main display logic task function
 *
 * Continuously waits for distance measurements and updates LED display.
 * Runs at priority 3 (between distance sensor priority 6 and test priority 2).
 * Blocks on distance_sensor_get_latest() until new measurements arrive.
 */
static void display_logic_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display logic task started (Priority: %d, Core: %d)",
             uxTaskPriorityGet(NULL), xPortGetCoreID());

    // Get current configuration to log the range
    system_config_t config;
    if (config_get_current(&config) == ESP_OK) {
        ESP_LOGI(TAG, "Distance range: %.1f-%.1fcm → LEDs 0-39, blocking until new measurements",
                 config.distance_min_mm / 10.0, config.distance_max_mm / 10.0);
    } else {
        ESP_LOGW(TAG, "Could not get configuration, using defaults");
    }

    distance_measurement_t measurement;

    while (1)
    {
        // This will now BLOCK until new measurement arrives
        if (distance_sensor_get_latest(&measurement) == ESP_OK)
        {
            update_led_display(&measurement);

            ESP_LOGD(TAG, "Processed distance: %d mm, status: %d",
                     measurement.distance_mm, measurement.status);
        }
        // No delay needed - function blocks until next measurement
    }
}

esp_err_t display_logic_start(void)
{
    if (display_task_handle != NULL)
    {
        ESP_LOGW(TAG, "Display logic task already running");
        return ESP_ERR_INVALID_STATE;
    }

    // Get current configuration from config_manager (REQ-CFG-2) - consolidated from display_logic_init()
    system_config_t config;
    esp_err_t ret = config_get_current(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current configuration: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configuration validation is handled by config_manager - no redundant checks needed

    // Check if LED controller is initialized - consolidated from display_logic_init()
    if (!led_is_initialized())
    {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Check if distance sensor is running
    if (!distance_sensor_is_running())
    {
        ESP_LOGW(TAG, "Distance sensor not running. Display may not update.");
    }

    // Get LED count for logging - consolidated from display_logic_init()
    uint16_t led_count = led_get_count();

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Config: %.1f-%.1fcm → LEDs 0-%d",
             config.distance_min_mm / 10.0, config.distance_max_mm / 10.0, led_count - 1);

    // Create display logic task
    BaseType_t result = xTaskCreatePinnedToCore(
        display_logic_task,     // Task function
        "display_logic",        // Task name
        4096,                   // Stack size (4KB)
        NULL,                   // Parameters
        3,                      // Priority 3 (between distance sensor 6 and test 2)
        &display_task_handle,   // Task handle
        1                       // Core ID (run on core 1)
    );

    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create display logic task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Display logic task started successfully");
    return ESP_OK;
}

// Legacy functions removed for architectural simplification:
// - display_logic_get_config(): Configuration access now via config_manager API (REQ-CFG-2)
// - display_logic_stop(): Restart-based architecture pattern
// - display_logic_is_running(): Simplified lifecycle management