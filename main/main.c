#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_controller.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Distance Measurement with LED Strip Display");
    ESP_LOGI(TAG, "Starting LED controller test...");

    // Configure LED strip
    led_config_t led_config = {
        .gpio_pin = GPIO_NUM_13,     // WS2812 data pin
        .led_count = 40,             // 40 LEDs in strip
        .rmt_channel = RMT_CHANNEL_0 // Use RMT channel 0
    };

    // Initialize LED controller
    esp_err_t ret = led_controller_init(&led_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize LED controller: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "LED controller initialized successfully");
    ESP_LOGI(TAG, "LED count: %d", led_get_count());

    // Test basic LED operations
    ESP_LOGI(TAG, "Testing LED operations...");

    // Clear all LEDs and show
    led_clear_all();
    led_show();
    vTaskDelay(pdMS_TO_TICKS(500));

    // Test running light effect
    for (int cycle = 0; cycle < 3; cycle++)
    {
        ESP_LOGI(TAG, "Running light cycle %d", cycle + 1);

        for (uint16_t i = 0; i < led_get_count(); i++)
        {
            // Clear previous LED
            if (i > 0)
            {
                led_clear_pixel(i - 1);
            }
            else
            {
                led_clear_pixel(led_get_count() - 1);
            }

            // Set current LED to green
            led_set_pixel(i, LED_COLOR_GREEN);
            led_show();

            vTaskDelay(pdMS_TO_TICKS(50)); // 50ms delay
        }
    }

    // Test different colors
    ESP_LOGI(TAG, "Testing different colors...");
    led_clear_all();

    // Set first few LEDs to different colors
    led_set_pixel(0, LED_COLOR_RED);
    led_set_pixel(1, LED_COLOR_GREEN);
    led_set_pixel(2, LED_COLOR_BLUE);
    led_set_pixel(3, LED_COLOR_WHITE);
    led_set_pixel(4, LED_COLOR_YELLOW);
    led_set_pixel(5, led_color_rgb(255, 128, 0)); // Orange

    led_show();
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Test brightness scaling
    ESP_LOGI(TAG, "Testing brightness scaling...");
    for (int brightness = 255; brightness >= 0; brightness -= 5)
    {
        for (uint16_t i = 0; i < 6; i++)
        {
            led_color_t original = led_get_pixel(i);
            led_set_pixel(i, led_color_brightness(original, brightness));
        }
        led_show();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Clear all LEDs
    led_clear_all();
    led_show();

    ESP_LOGI(TAG, "LED controller test completed successfully!");
    ESP_LOGI(TAG, "Ready for distance sensor integration...");
}