#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_controller.h"
#include "test/test_task.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Distance Measurement with LED Strip Display");

    // Configure LED strip
    led_config_t led_config = {
        .gpio_pin = GPIO_NUM_12,     // WS2812 data pin
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

    // Start background test task
    ret = test_task_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start test task");
        return;
    }

    ESP_LOGI(TAG, "Background LED test task started");
    ESP_LOGI(TAG, "Ready for distance sensor integration...");

    // Main application loop - this is where distance sensor code will go
    while (1)
    {
        // Future: Distance sensor reading and LED display logic
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
