#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_controller.h"
#include "test/test_task.h"
#include "distance_sensor.h"

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

    // Configure and initialize distance sensor
    distance_sensor_config_t distance_config = {
        .trigger_pin = GPIO_NUM_14,     // Trigger pin
        .echo_pin = GPIO_NUM_13,        // Echo pin
        .measurement_interval_ms = 1000, // Measure every 1 second (prevents queue overflow)
        .timeout_ms = 30,               // 30ms timeout (for max 400cm range)
        .temperature_celsius = 20.0     // Room temperature for speed of sound
    };

    ret = distance_sensor_init(&distance_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize distance sensor: %s", esp_err_to_name(ret));
        return;
    }

    ret = distance_sensor_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start distance sensor: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Distance sensor initialized and started");
    ESP_LOGI(TAG, "Hardware: LED=GPIO12, Trigger=GPIO14, Echo=GPIO13");
    ESP_LOGI(TAG, "Ready for distance measurement and LED display...");

    // Main application loop - Read distance and display on LED strip
    while (1)
    {
        distance_measurement_t measurement;

        // Check for new distance measurement
        if (distance_sensor_get_latest(&measurement) == ESP_OK)
        {
            if (measurement.status == DISTANCE_SENSOR_OK)
            {
                ESP_LOGI(TAG, "Distance: %.2f cm", measurement.distance_cm);

                // TODO: Convert distance to LED visualization
                // For now, just log the successful measurement
            }
            else
            {
                // Handle measurement errors
                const char *error_msg = "";
                switch (measurement.status)
                {
                case DISTANCE_SENSOR_TIMEOUT:
                    error_msg = "Timeout";
                    break;
                case DISTANCE_SENSOR_OUT_OF_RANGE:
                    error_msg = "Out of range";
                    break;
                case DISTANCE_SENSOR_NO_ECHO:
                    error_msg = "No echo";
                    break;
                case DISTANCE_SENSOR_INVALID_READING:
                    error_msg = "Invalid reading";
                    break;
                default:
                    error_msg = "Unknown error";
                    break;
                }
                ESP_LOGW(TAG, "Distance measurement error: %s", error_msg);
            }
        }

        // Check for queue overflows (indicates system overload)
        uint32_t overflows = distance_sensor_get_queue_overflows();
        static uint32_t last_overflow_count = 0;
        if (overflows > last_overflow_count)
        {
            ESP_LOGW(TAG, "Distance sensor queue overflows: %lu", overflows);
            last_overflow_count = overflows;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Short yield to other tasks
    }
}
