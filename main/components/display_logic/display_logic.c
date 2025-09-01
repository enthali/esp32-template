/**
 * @file display_logic.c
 * @brief LED Distance Visualization Business Logic Implementation
 * 
 * DESIGN TRACEABILITY:
 * - DSN-DSP-OVERVIEW-01: WS2812 hardware integration design (reactive LED display system)
 * - DSN-DSP-ARCH-01: Task-based architecture (display_logic_task function)
 * - DSN-DSP-ARCH-02: Configuration integration design (config_manager API usage)
 * - DSN-DSP-ALGO-01: Distance-to-visual mapping algorithm (WHAT to display)
 * - DSN-DSP-ALGO-02: LED update pattern design (HOW to display)
 * - DSN-DSP-ALGO-03: Embedded arithmetic architecture design (integer-only calculations)
 * - DSN-DSP-API-01: Simplified API design (single display_logic_start entry point)
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ-DSP-OVERVIEW-01: WS2812 LED strip support via led_controller component
 * - REQ-DSP-OVERVIEW-02: Configuration integration via config_manager API
 * - REQ-DSP-VISUAL-01: Single LED illumination (clear-and-set pattern)
 * - REQ-DSP-VISUAL-02: Normal range green display (linear mapping to positions 0..led_count-1)
 * - REQ-DSP-VISUAL-03: Below minimum red display (first LED, position 0)
 * - REQ-DSP-VISUAL-04: Above maximum red display (last LED, position led_count-1)
 * - REQ-DSP-IMPL-01: Task-based architecture with priority below measurement task
 * - REQ-DSP-IMPL-02: LED buffer management via WS2812 clear-and-transmit pattern
 * - REQ-DSP-IMPL-03: Distance-to-LED calculation with range clamping
 * - REQ-CFG-1: Use centralized configuration header (config.h)
 * - REQ-CFG-2: Use centralized configuration via config_manager API
 * 
 * TARGET DESIGN ARCHITECTURE:
 * 
 * DSN-DSP-OVERVIEW-01: WS2812 Hardware Integration Design
 * ADDRESSES: REQ-DSP-OVERVIEW-01 (Hardware Platform - WS2812 LED strip integration)
 * DESIGN: WS2812 addressable LED strip as primary display hardware with config-driven parameters
 * - LED count: Configurable via config_manager API (1-100 LEDs, validated)
 * - Brightness: Configurable via config_manager API (0-255, hardware PWM control)
 * - GPIO pin: Hardware-specific configuration for WS2812 data line
 * - Initialization: LED hardware initialized before task starts processing measurements
 * - Real-time operation: Continuous reactive updates as distance measurements arrive
 * VALIDATION: LED strip responds to configuration changes, hardware initialization successful
 * 
 * DSN-DSP-ARCH-01: Task-Based Architecture Design
 * ADDRESSES: REQ-DSP-IMPL-01 (Task-based architecture with priority below measurement task)
 * DESIGN: Implement single FreeRTOS task that blocks on distance_sensor_get_latest()
 * - Task priority set below measurement task to ensure proper data flow hierarchy
 * - Task runs continuously until system restart (no complex lifecycle management)  
 * - Core assignment and stack size from centralized configuration
 * - Blocking wait pattern eliminates polling overhead and provides immediate response
 * VALIDATION: Task created successfully, priority hierarchy maintained, blocks efficiently
 * 
 * DSN-DSP-ARCH-02: Configuration Integration Design  
 * ADDRESSES: REQ-DSP-OVERVIEW-02 (Configuration integration), REQ-CFG-2 (Use centralized config)
 * DESIGN: Use config_manager API for all distance range parameters
 * - Obtain min/max distance values via config_manager_get_distance_min_cm() / config_manager_get_distance_max_cm()
 * - Cache config values locally at task startup for performance
 * - Configuration changes handled via system restart (restart-based architecture)
 * - Configuration validation responsibility belongs to config_manager
 * VALIDATION: All distance parameters obtained from config_manager API, no separate config structures
 * 
 * DSN-DSP-ALGO-01: Distance-to-Visual Mapping Algorithm (WHAT to display)
 * ADDRESSES: REQ-DSP-IMPL-03 (Distance calculation), REQ-DSP-VISUAL-01/02/03/04 (All visual behaviors)
 * DESIGN: Determines WHAT should be displayed based on distance measurement and user-configured range
 * - Normal range (min ≤ distance ≤ max): Green LED at calculated position using linear interpolation
 *   Formula: led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)
 * - Below minimum (distance < min): Red LED at position 0 (REQ-DSP-VISUAL-03)
 * - Above maximum (distance > max): Red LED at position led_count-1 (REQ-DSP-VISUAL-04)
 * - Boundary clamping ensures valid LED positions [0, led_count-1]
 * - Single LED illumination enforced by logic (REQ-DSP-VISUAL-01)
 * VALIDATION: Min distance → LED 0, max distance → LED led_count-1, linear interpolation between,
 *             below/above range → correct red LED positions
 * 
 * DSN-DSP-ALGO-02: LED Update Pattern Design (HOW to display)
 * ADDRESSES: REQ-DSP-IMPL-02 (LED buffer management)
 * DESIGN: Implements HOW to physically update LED strip hardware with WS2812-optimized pattern
 * - Step 1: led_clear_all() - set all LEDs to off state
 * - Step 2: led_set_pixel(position, color) - set desired LED from ALGO-01 decision
 * - Step 3: led_show() - transmit complete buffer to WS2812 strip
 * - WS2812 serial protocol requires complete buffer transmission (hardware characteristic)
 * - Clear-and-set pattern guarantees only one LED illuminated
 * VALIDATION: Only one LED illuminated after each update, WS2812 transmission successful
 * 
 * DSN-DSP-ALGO-03: Embedded Arithmetic Architecture Design
 * ADDRESSES: REQ-SYS-1 (ESP32 Hardware Platform - embedded microcontroller constraints)
 * DESIGN: Pure integer arithmetic for all distance calculations and display operations
 * - Distance representation: uint16_t millimeters (0-65535mm = 0-65.5m range)
 * - Position calculations: Multiplication before division for precision preservation
 * - Boundary checks: Integer comparisons (faster than floating-point)
 * - Memory efficiency: 2-byte integers vs 4-byte floats
 * - Execution speed: Integer ALU operations vs FPU operations
 * - Deterministic timing: No floating-point precision variations
 * - Code size: Smaller binary without float library linkage
 * RATIONALE: REQ-SYS-1 mandates ESP32 embedded platform - standard embedded practice avoids
 *            floating-point on resource-constrained microcontrollers unless absolutely necessary
 * VALIDATION: All arithmetic operations complete within deterministic time bounds
 * 
 * DSN-DSP-API-01: Simplified API Design
 * ADDRESSES: REQ-DSP-IMPL-01 (Task-based architecture), restart-based architecture philosophy
 * DESIGN: Single entry point for simplified lifecycle management
 * - esp_err_t display_logic_start(void) - primary public function
 * - Task runs continuously until system restart
 * - Error conditions handled by system restart mechanism
 * - No complex lifecycle management (no start/stop/pause/resume complexity)
 * - No configuration exposure through API (encapsulation)
 * VALIDATION: Single function call starts display system, no API complexity, no lifecycle state tracking
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

// Configuration removed - using config_manager API directly (REQ-CFG-2)
// static display_config_t display_config;  <- REMOVED: Violates centralized configuration
// static bool is_initialized = false;      <- REMOVED: Unnecessary state tracking

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
                
                // Normal range: Green color for distance visualization
                led_color_t color = LED_COLOR_GREEN;
                led_set_pixel(led_index, color);
                ESP_LOGD(TAG, "Distance %d mm → LED %d", measurement->distance_mm, led_index);
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