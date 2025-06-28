/**
 * @file display_logic.h
 * @brief LED Distance Visualization Business Logic Component
 *
 * Business logic component that converts distance measurements into LED strip visualizations.
 * Provides clean separation between hardware abstraction (components) and application logic.
 *
 * FEATURES:
 * =========
 * - Distance Range Mapping: 10-50cm mapped linearly to 40 LEDs (LED 0 to LED 39)
 * - LED Spacing: Approximately 1cm per LED (40cm range / 40 LEDs)
 * - Visual Feedback:
 *   - Normal range: Green/blue gradient or solid color
 *   - Below 10cm: First LED red (error indicator)
 *   - Above 50cm: Last LED red (error indicator)
 *   - Sensor timeout/error: All LEDs off or specific error pattern
 *
 * ARCHITECTURE:
 * =============
 * - Priority 3 Task: LED visualization (between sensor priority 6 and test priority 2)
 * - Non-blocking reads from distance sensor queue
 * - Update rate matches or slightly slower than distance sensor (1Hz)
 * - Real-time safe with proper task priorities
 *
 * INTEGRATION:
 * ============
 * - Input: distance_sensor_get_latest() API (non-blocking)
 * - Output: led_controller APIs (led_set_pixel, led_clear_all, led_show)
 * - Error handling: Visual indicators for all sensor error states
 *
 * @author ESP32 Distance Project
 * @date 2025
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Display configuration structure
     *
     * Configuration parameters for the LED distance visualization system.
     */
    typedef struct
    {
        float min_distance_cm;       ///< Minimum distance for normal range (10.0f)
        float max_distance_cm;       ///< Maximum distance for normal range (50.0f)
        uint32_t update_interval_ms; ///< Update interval in milliseconds (1000)
        // Add calibration support for future extensions
    } display_config_t;

    /**
     * @brief Initialize the display logic component
     *
     * Initializes the display logic with specified configuration.
     * Must be called before display_logic_start().
     *
     * @param config Pointer to display configuration. If NULL, uses default values.
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note Default configuration: 10-50cm range, 1000ms update interval
     */
    esp_err_t display_logic_init(const display_config_t *config);

    /**
     * @brief Start the display logic task
     *
     * Creates and starts the display logic task with priority 3.
     * Task will continuously read distance measurements and update LED strip.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note Task runs on core 1 with 4KB stack at priority 3
     * @note display_logic_init() must be called first
     */
    esp_err_t display_logic_start(void);

    /**
     * @brief Stop the display logic task
     *
     * Stops the display logic task and clears all LEDs.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     */
    esp_err_t display_logic_stop(void);

    /**
     * @brief Check if display logic task is running
     *
     * @return true if task is running, false otherwise
     */
    bool display_logic_is_running(void);

    /**
     * @brief Get current display configuration
     *
     * @return Current display configuration structure
     */
    display_config_t display_logic_get_config(void);

#ifdef __cplusplus
}
#endif