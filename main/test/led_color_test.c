/**
 * @file led_color_test.c
 * @brief Color and Brightness Test Implementation
 */

#include "led_color_test.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <inttypes.h>

static const char *TAG = "led_color_test";

esp_err_t led_color_test_basic_colors(uint32_t display_time_ms)
{
    if (!led_is_initialized())
    {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Testing basic colors...");

    // Clear all LEDs first
    led_clear_all();

    // Define test colors with improved visibility
    led_color_t test_colors[] = {
        LED_COLOR_RED,              // LED 0: Pure red
        LED_COLOR_GREEN,            // LED 1: Pure green
        LED_COLOR_BLUE,             // LED 2: Pure blue
        LED_COLOR_WHITE,            // LED 3: White
        LED_COLOR_YELLOW,           // LED 4: Yellow
        led_color_rgb(255, 165, 0), // LED 5: Orange
        led_color_rgb(128, 0, 128), // LED 6: Purple (dimmer magenta)
        led_color_rgb(0, 255, 255), // LED 7: Cyan
    };

    uint8_t color_count = sizeof(test_colors) / sizeof(test_colors[0]);

    // Set colors on first few LEDs
    for (uint8_t i = 0; i < color_count && i < led_get_count(); i++)
    {
        led_set_pixel(i, test_colors[i]);
        ESP_LOGI(TAG, "LED %d: R=%d G=%d B=%d", i,
                 test_colors[i].red, test_colors[i].green, test_colors[i].blue);
    }

    led_show();
    ESP_LOGI(TAG, "Displaying colors for %" PRIu32 "ms", display_time_ms);
    vTaskDelay(pdMS_TO_TICKS(display_time_ms));

    return ESP_OK;
}

esp_err_t led_color_test_brightness_fade(const led_color_t *colors, uint8_t color_count, uint32_t step_delay_ms)
{
    if (!led_is_initialized() || colors == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameters for brightness fade test");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Testing brightness fade on %d colors...", color_count);

    // Ensure we don't exceed LED count
    uint8_t test_count = (color_count < led_get_count()) ? color_count : led_get_count();

    // Set initial colors at full brightness
    led_clear_all();
    for (uint8_t i = 0; i < test_count; i++)
    {
        led_set_pixel(i, colors[i]);
    }
    led_show();
    vTaskDelay(pdMS_TO_TICKS(step_delay_ms * 2)); // Brief pause at full brightness

    // Fade from full brightness to off
    for (int brightness = 255; brightness >= 0; brightness -= 5)
    {
        for (uint8_t i = 0; i < test_count; i++)
        {
            led_color_t dimmed_color = led_color_brightness(colors[i], brightness);
            led_set_pixel(i, dimmed_color);
        }
        led_show();
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }

    ESP_LOGI(TAG, "Brightness fade test completed");
    return ESP_OK;
}

esp_err_t led_color_test_custom_colors(uint32_t display_time_ms)
{
    ESP_LOGI(TAG, "Testing custom color combinations...");

    // Test custom color combinations
    led_color_t custom_colors[] = {
        led_color_rgb(255, 100, 0),   // Red-Orange
        led_color_rgb(100, 255, 0),   // Yellow-Green
        led_color_rgb(0, 255, 100),   // Green-Cyan
        led_color_rgb(0, 100, 255),   // Cyan-Blue
        led_color_rgb(100, 0, 255),   // Blue-Purple
        led_color_rgb(255, 0, 100),   // Purple-Red
        led_color_rgb(255, 192, 203), // Pink
        led_color_rgb(64, 224, 208),  // Turquoise
    };

    uint8_t color_count = sizeof(custom_colors) / sizeof(custom_colors[0]);

    led_clear_all();
    for (uint8_t i = 0; i < color_count && i < led_get_count(); i++)
    {
        led_set_pixel(i, custom_colors[i]);
        ESP_LOGI(TAG, "Custom color %d: R=%d G=%d B=%d", i,
                 custom_colors[i].red, custom_colors[i].green, custom_colors[i].blue);
    }

    led_show();
    vTaskDelay(pdMS_TO_TICKS(display_time_ms));

    return ESP_OK;
}

esp_err_t led_color_test_rgb_channels(uint32_t step_delay_ms)
{
    ESP_LOGI(TAG, "Testing individual RGB channels...");

    if (led_get_count() < 3)
    {
        ESP_LOGE(TAG, "Need at least 3 LEDs for RGB channel test");
        return ESP_ERR_INVALID_STATE;
    }

    // Test each channel from 0 to 255
    for (int intensity = 0; intensity <= 255; intensity += 15)
    {
        led_clear_all();

        // LED 0: Red channel only
        led_set_pixel(0, led_color_rgb(intensity, 0, 0));

        // LED 1: Green channel only
        led_set_pixel(1, led_color_rgb(0, intensity, 0));

        // LED 2: Blue channel only
        led_set_pixel(2, led_color_rgb(0, 0, intensity));

        led_show();
        ESP_LOGI(TAG, "RGB intensity: %d", intensity);
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }

    // Clear after test
    led_clear_all();
    led_show();

    ESP_LOGI(TAG, "RGB channel test completed");
    return ESP_OK;
}
