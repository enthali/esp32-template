/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic.
 *
 * Design is documented in `docs/design/display-design.md` and linked to requirements
 * and design IDs (e.g. REQ-DSP-*, DSN-DSP-*). Keep this header short; full design
 * rationale lives in the design document for traceability.
 *
 * DESIGN TRACEABILITY:
 * - DSN-DSP-ANIM-01: Animation layer architecture
 * - DSN-DSP-ANIM-02: Ideal zone display
 * - DSN-DSP-ANIM-03: Emergency blinking pattern
 * - DSN-DSP-ANIM-04: Frame-based rendering pipeline
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ-DSP-ANIM-01: "Too far" blue animation
 * - REQ-DSP-ANIM-02: "Too close" orange animation
 * - REQ-DSP-ANIM-03: Ideal zone all-red display
 * - REQ-DSP-ANIM-04: Dual-layer compositing
 * - REQ-DSP-ANIM-05: Emergency proximity warning
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "display_logic";

// Zone configuration constants (configuration-independent, percentage-based)
#define IDEAL_CENTER_PERCENT 30   // Ideal zone at 30% of strip length
#define IDEAL_SIZE_PERCENT 10     // Ideal zone spans 10% of strip width

// Animation timing constants
#define ANIMATION_FRAME_MS 100    // 100ms per animation frame (10 FPS)
#define EMERGENCY_BLINK_MS 500    // 500ms ON/OFF for emergency blink (1 Hz)
#define ANIMATION_BRIGHTNESS 5    // 2% brightness (~5/255)

/**
 * @brief Animation state tracking
 * 
 * DESIGN: DSN-DSP-ANIM-01 - Animation layer state machine
 */
typedef struct {
    uint8_t current_position;    // Current animation LED position
    bool active;                 // Is animation running?
    bool direction_forward;      // true = forward (too close), false = backward (too far)
    uint8_t start_pos;          // Start position for animation
    uint8_t end_pos;            // End position for animation
} animation_state_t;

/**
 * @brief Emergency blink state tracking
 * 
 * DESIGN: DSN-DSP-ANIM-03 - Emergency blinking pattern state
 */
typedef struct {
    bool blink_on;              // true = LEDs ON, false = LEDs OFF
    uint32_t last_toggle_ms;    // Last toggle timestamp
} blink_state_t;

/**
 * @brief Zone calculation results
 * 
 * DESIGN: DSN-DSP-ALGO-01 - Configuration-independent zone boundaries
 */
typedef struct {
    uint8_t ideal_start;        // First LED in ideal zone
    uint8_t ideal_end;          // Last LED in ideal zone
    uint8_t ideal_center;       // Center position of ideal zone
    uint8_t ideal_size;         // Size of ideal zone
} zone_config_t;

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Animation timer handle
static esp_timer_handle_t animation_timer = NULL;

// Animation and blink state (shared between task and timer)
static animation_state_t anim_state = {0};
static blink_state_t blink_state = {0};
static zone_config_t zones = {0};

// Cached current distance for timer callback
static distance_measurement_t cached_measurement = {0};
static bool measurement_valid = false;

/**
 * @brief Calculate zone boundaries based on LED count
 * 
 * DESIGN: DSN-DSP-ALGO-01 - Configuration-independent zone calculation
 * Uses percentage-based zones with integer arithmetic for embedded efficiency.
 * 
 * @param led_count Number of LEDs in the strip
 * @param zones Pointer to zone_config_t to fill
 * 
 * REQUIREMENTS: REQ-DSP-VISUAL-02 AC-8
 */
static void calculate_zones(uint16_t led_count, zone_config_t *zones)
{
    // Calculate ideal zone size and position (integer arithmetic only)
    zones->ideal_size = (led_count * IDEAL_SIZE_PERCENT) / 100;
    zones->ideal_center = (led_count * IDEAL_CENTER_PERCENT) / 100;
    zones->ideal_start = zones->ideal_center - (zones->ideal_size / 2);
    zones->ideal_end = zones->ideal_start + zones->ideal_size - 1;
    
    // Ensure at least 1 LED in ideal zone
    if (zones->ideal_size == 0) {
        zones->ideal_size = 1;
        zones->ideal_end = zones->ideal_start;
    }
    
    ESP_LOGI(TAG, "Zone calculation: LED count=%d, ideal zone=[%d-%d] (center=%d, size=%d)",
             led_count, zones->ideal_start, zones->ideal_end, 
             zones->ideal_center, zones->ideal_size);
}

/**
 * @brief Update animation state based on current zone
 * 
 * DESIGN: DSN-DSP-ANIM-01 - Animation state machine
 * 
 * @param led_index Current LED position from distance measurement
 * @param led_count Total number of LEDs
 * 
 * REQUIREMENTS: REQ-DSP-ANIM-01, REQ-DSP-ANIM-02
 */
static void update_animation_state(int led_index, uint16_t led_count)
{
    if (led_index < zones.ideal_start) {
        // Too close zone - orange animation from 0 to ideal_start
        if (!anim_state.active || !anim_state.direction_forward) {
            // Start new animation
            anim_state.current_position = 0;
            anim_state.direction_forward = true;
            anim_state.start_pos = 0;
            anim_state.end_pos = zones.ideal_start;
            anim_state.active = true;
            ESP_LOGD(TAG, "Starting 'too close' animation: 0→%d", zones.ideal_start);
        }
    }
    else if (led_index > zones.ideal_end) {
        // Too far zone - blue animation from led_count-1 to ideal_end
        if (!anim_state.active || anim_state.direction_forward) {
            // Start new animation
            anim_state.current_position = led_count - 1;
            anim_state.direction_forward = false;
            anim_state.start_pos = led_count - 1;
            anim_state.end_pos = zones.ideal_end;
            anim_state.active = true;
            ESP_LOGD(TAG, "Starting 'too far' animation: %d→%d", led_count - 1, zones.ideal_end);
        }
    }
    else {
        // Ideal zone - no animation
        if (anim_state.active) {
            anim_state.active = false;
            ESP_LOGD(TAG, "Stopping animation (in ideal zone)");
        }
    }
}

/**
 * @brief Advance animation to next position
 * 
 * DESIGN: DSN-DSP-ANIM-01 - Animation position update with looping
 * Called every 100ms by timer callback.
 * 
 * REQUIREMENTS: REQ-DSP-ANIM-01 AC-4/5, REQ-DSP-ANIM-02 AC-4/5
 */
static void advance_animation(void)
{
    if (!anim_state.active) {
        return;
    }
    
    if (anim_state.direction_forward) {
        // Moving forward (too close animation)
        anim_state.current_position++;
        if (anim_state.current_position >= anim_state.end_pos) {
            // Loop back to start
            anim_state.current_position = anim_state.start_pos;
        }
    }
    else {
        // Moving backward (too far animation)
        if (anim_state.current_position <= anim_state.end_pos) {
            // Loop back to start
            anim_state.current_position = anim_state.start_pos;
        }
        else {
            anim_state.current_position--;
        }
    }
}

/**
 * @brief Update emergency blink state
 * 
 * DESIGN: DSN-DSP-ANIM-03 - 1 Hz blinking pattern timing
 * Called every 100ms by timer callback, toggles every 500ms.
 * 
 * REQUIREMENTS: REQ-DSP-ANIM-05 AC-1
 */
static void update_blink_state(void)
{
    uint32_t current_ms = (uint32_t)(esp_timer_get_time() / 1000);
    
    if (current_ms - blink_state.last_toggle_ms >= EMERGENCY_BLINK_MS) {
        blink_state.blink_on = !blink_state.blink_on;
        blink_state.last_toggle_ms = current_ms;
    }
}

/**
 * @brief Render dual-layer LED display frame
 * 
 * DESIGN: DSN-DSP-ALGO-02 - Frame-based rendering pipeline
 * Priority: Emergency > Ideal Zone > Position > Animation
 * 
 * @param measurement Current distance measurement
 * @param config Current system configuration
 * 
 * REQUIREMENTS: REQ-DSP-ANIM-04
 */
static void render_frame(const distance_measurement_t *measurement, const system_config_t *config)
{
    uint16_t led_count = led_get_count();
    
    // Step 1: Clear all LEDs
    led_clear_all();
    
    // Determine LED index from distance
    int led_index = -1;
    bool in_range = false;
    bool below_min = false;
    
    if (measurement->status == DISTANCE_SENSOR_OK) {
        if (measurement->distance_mm >= config->distance_min_mm && 
            measurement->distance_mm <= config->distance_max_mm) {
            // Normal range - calculate position
            uint16_t range_mm = config->distance_max_mm - config->distance_min_mm;
            uint16_t offset_mm = measurement->distance_mm - config->distance_min_mm;
            led_index = (int)((offset_mm * (led_count - 1)) / range_mm);
            
            // Clamp to valid range
            if (led_index < 0) led_index = 0;
            if (led_index >= led_count) led_index = led_count - 1;
            
            in_range = true;
        }
        else if (measurement->distance_mm < config->distance_min_mm) {
            below_min = true;
        }
    }
    
    // Step 2: Render animation layer (base layer, 2% brightness)
    if (anim_state.active && in_range) {
        led_color_t anim_color;
        if (anim_state.direction_forward) {
            // Too close - orange animation
            anim_color = led_color_brightness(LED_COLOR_ORANGE, ANIMATION_BRIGHTNESS);
        }
        else {
            // Too far - blue animation
            anim_color = led_color_brightness(LED_COLOR_BLUE, ANIMATION_BRIGHTNESS);
        }
        led_set_pixel(anim_state.current_position, anim_color);
    }
    else if (!in_range && measurement->status == DISTANCE_SENSOR_OK && 
             measurement->distance_mm > config->distance_max_mm) {
        // Out of range - show blue animation only (no position)
        // Force "too far" animation
        if (!anim_state.active || anim_state.direction_forward) {
            anim_state.current_position = led_count - 1;
            anim_state.direction_forward = false;
            anim_state.start_pos = led_count - 1;
            anim_state.end_pos = zones.ideal_end;
            anim_state.active = true;
        }
        led_color_t anim_color = led_color_brightness(LED_COLOR_BLUE, ANIMATION_BRIGHTNESS);
        led_set_pixel(anim_state.current_position, anim_color);
    }
    
    // Step 3: Render position layer (overlay, 100% brightness)
    if (in_range && led_index >= 0) {
        // Check if in ideal zone
        if (led_index >= zones.ideal_start && led_index <= zones.ideal_end) {
            // Step 3b: Ideal zone - all LEDs red (overrides position and animation)
            for (uint8_t i = zones.ideal_start; i <= zones.ideal_end; i++) {
                led_set_pixel(i, LED_COLOR_RED);
            }
        }
        else {
            // Normal position indicator - green LED
            led_set_pixel(led_index, LED_COLOR_GREEN);
        }
    }
    
    // Step 4: Emergency layer (highest priority, overrides all)
    if (below_min && blink_state.blink_on) {
        // Blink every 10th LED
        for (uint16_t i = 0; i < led_count; i += 10) {
            led_set_pixel(i, LED_COLOR_RED);
        }
    }
    
    // Step 5: Commit to hardware
    led_show();
}

/**
 * @brief Animation timer callback
 * 
 * DESIGN: DSN-DSP-ANIM-04 - Frame-based timing architecture
 * Called every 100ms to update animation and render frame.
 * 
 * @param arg Unused
 * 
 * REQUIREMENTS: REQ-DSP-ANIM-04 AC-6
 */
static void animation_timer_callback(void *arg)
{
    if (!measurement_valid) {
        return;
    }
    
    // Update animation position
    advance_animation();
    
    // Update blink state
    update_blink_state();
    
    // Get current configuration
    system_config_t config;
    if (config_get_current(&config) != ESP_OK) {
        return;
    }
    
    // Render frame with current state
    render_frame(&cached_measurement, &config);
}

/**
 * @brief Update LED display based on distance measurement
 *
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{
    // Cache measurement for timer callback
    cached_measurement = *measurement;
    measurement_valid = true;
    
    // Get configuration
    system_config_t config;
    bool config_valid = (config_get_current(&config) == ESP_OK);
    
    if (!config_valid) {
        ESP_LOGE(TAG, "Failed to get configuration for display update");
        return;
    }

    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        // Calculate LED position for zone determination
        if (measurement->distance_mm >= config.distance_min_mm && 
            measurement->distance_mm <= config.distance_max_mm)
        {
            // Normal range: Calculate LED position
            uint16_t range_mm = config.distance_max_mm - config.distance_min_mm;
            uint16_t offset_mm = measurement->distance_mm - config.distance_min_mm;
            
            uint16_t led_count = led_get_count();
            int led_index = (int)((offset_mm * (led_count - 1)) / range_mm);
            
            // Ensure within bounds
            if (led_index < 0) led_index = 0;
            if (led_index >= led_count) led_index = led_count - 1;
            
            // Update animation state based on zone
            update_animation_state(led_index, led_count);
            
            // Log zone information
            const char *zone_name;
            if (led_index < zones.ideal_start) {
                zone_name = "too close";
            } else if (led_index <= zones.ideal_end) {
                zone_name = "ideal";
            } else {
                zone_name = "too far";
            }
            
            ESP_LOGD(TAG, "Distance %d mm → LED %d (%s zone)", 
                     measurement->distance_mm, led_index, zone_name);
        }
        else if (measurement->distance_mm < config.distance_min_mm)
        {
            // Emergency: Below minimum
            anim_state.active = false;
            ESP_LOGD(TAG, "Distance %d mm - EMERGENCY (below minimum)", measurement->distance_mm);
        }
        else
        {
            // Out of range: Above maximum
            // Animation will be handled in render_frame
            ESP_LOGD(TAG, "Distance %d mm - out of range (above maximum)", measurement->distance_mm);
        }
        break;
    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off
        anim_state.active = false;
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Show "too far" animation
        ESP_LOGD(TAG, "Sensor out of range → Animation mode");
        break;

    case DISTANCE_SENSOR_NO_ECHO:
    case DISTANCE_SENSOR_INVALID_READING:
    default:
        // Other errors: Stop animation
        anim_state.active = false;
        ESP_LOGD(TAG, "Sensor error → LEDs off");
        break;
    }
    
    // Immediate frame render (timer will handle subsequent updates)
    render_frame(measurement, &config);
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

    // Get LED count and calculate zones
    uint16_t led_count = led_get_count();
    calculate_zones(led_count, &zones);

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Config: %.1f-%.1fcm → LEDs 0-%d, ideal zone=[%d-%d]",
             config.distance_min_mm / 10.0, config.distance_max_mm / 10.0, 
             led_count - 1, zones.ideal_start, zones.ideal_end);

    // Initialize animation state
    anim_state.active = false;
    anim_state.current_position = 0;
    anim_state.direction_forward = false;
    
    // Initialize blink state
    blink_state.blink_on = false;
    blink_state.last_toggle_ms = 0;
    
    // Create animation timer
    esp_timer_create_args_t timer_args = {
        .callback = animation_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "anim_timer"
    };
    
    ret = esp_timer_create(&timer_args, &animation_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create animation timer: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Start periodic timer (100ms = 100000µs)
    ret = esp_timer_start_periodic(animation_timer, ANIMATION_FRAME_MS * 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start animation timer: %s", esp_err_to_name(ret));
        esp_timer_delete(animation_timer);
        animation_timer = NULL;
        return ret;
    }
    
    ESP_LOGI(TAG, "Animation timer started (period=%dms)", ANIMATION_FRAME_MS);

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
        esp_timer_stop(animation_timer);
        esp_timer_delete(animation_timer);
        animation_timer = NULL;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Display logic task started successfully");
    return ESP_OK;
}

// Legacy functions removed for architectural simplification:
// - display_logic_get_config(): Configuration access now via config_manager API (REQ-CFG-2)
// - display_logic_stop(): Restart-based architecture pattern
// - display_logic_is_running(): Simplified lifecycle management