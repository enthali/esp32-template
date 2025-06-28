#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_controller.h"
#include "test/led_running_test.h"
#include "test/led_color_test.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Distance Measurement with LED Strip Display");
    ESP_LOGI(TAG, "Starting LED controller tests...");

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

    // Clear all LEDs first
    led_clear_all();
    led_show();
    vTaskDelay(pdMS_TO_TICKS(500));

    // Test 1: Running light effect (3 cycles)
    ESP_LOGI(TAG, "=== Running Light Test ===");
    led_running_test_multiple_cycles(LED_COLOR_GREEN, 50, 3);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Test 2: Basic color display
    ESP_LOGI(TAG, "=== Basic Colors Test ===");
    led_color_test_basic_colors(2000);
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Test 3: Brightness fade test
    ESP_LOGI(TAG, "=== Brightness Fade Test ===");
    led_color_test_brightness_fade_basic(20);

    // Clear all LEDs
    led_clear_all();
    led_show();

    ESP_LOGI(TAG, "=== All LED Tests Completed Successfully! ===");
    ESP_LOGI(TAG, "Ready for distance sensor integration...");

    // Main loop - keep the program running
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
