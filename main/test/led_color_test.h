/**
 * @file led_color_test.h
 * @brief Color and Brightness Test for LED Controller
 *
 * Test functions for color accuracy and brightness control on the LED strip.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "../led_controller.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Test basic color display
     *
     * Displays primary and secondary colors on the first few LEDs.
     *
     * @param display_time_ms Time to display colors in milliseconds
     * @return ESP_OK on success
     */
    esp_err_t led_color_test_basic_colors(uint32_t display_time_ms);

    /**
     * @brief Test brightness scaling
     *
     * Tests brightness scaling from full to off on specified colors.
     *
     * @param colors Array of colors to test
     * @param color_count Number of colors in array
     * @param step_delay_ms Delay between brightness steps in milliseconds
     * @return ESP_OK on success
     */
    esp_err_t led_color_test_brightness_fade(const led_color_t *colors, uint8_t color_count, uint32_t step_delay_ms);

    /**
     * @brief Test custom color mixing
     *
     * Tests various RGB combinations to verify color accuracy.
     *
     * @param display_time_ms Time to display each color in milliseconds
     * @return ESP_OK on success
     */
    esp_err_t led_color_test_custom_colors(uint32_t display_time_ms);

    /**
     * @brief Test individual RGB channels
     *
     * Tests red, green, and blue channels individually at various intensities.
     *
     * @param step_delay_ms Delay between intensity steps in milliseconds
     * @return ESP_OK on success
     */
    esp_err_t led_color_test_rgb_channels(uint32_t step_delay_ms);

#ifdef __cplusplus
}
#endif
