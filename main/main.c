#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "led_controller.h"
#include "test/test_task.h"
#include "test/led_running_test.h"
#include "distance_sensor.h"
#include "wifi_manager.h"
#include "display_logic.h"


static const char *TAG = "main";

// =============================================
// HARDWARE CONFIGURATION
// =============================================
#define LED_DATA_PIN GPIO_NUM_12
#define LED_COUNT 40
#define LED_RMT_CHANNEL RMT_CHANNEL_0

#define DISTANCE_TRIGGER GPIO_NUM_14
#define DISTANCE_ECHO GPIO_NUM_13
#define DISTANCE_INTERVAL 100  // Measurement interval in ms (100ms = 10Hz for responsive updates)
#define DISTANCE_TIMEOUT 30    // Echo timeout in ms
#define TEMPERATURE_C 20.0f    // Room temperature for speed of sound

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Distance Measurement with LED Strip Display");

    // Configure LED strip
    led_config_t led_config = {
        .gpio_pin = LED_DATA_PIN,
        .led_count = LED_COUNT,
        .rmt_channel = LED_RMT_CHANNEL};

    // Initialize LED controller
    esp_err_t ret = led_controller_init(&led_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize LED controller: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "LED controller initialized successfully");
    ESP_LOGI(TAG, "LED count: %d", led_get_count());

    // Clear all LEDs first
    led_clear_all();
    led_show();

    // Run hardware test once at startup (not continuous background task)
    ESP_LOGI(TAG, "Running one-time LED hardware test...");
    led_running_test_single_cycle(LED_COLOR_GREEN, 50);
    ESP_LOGI(TAG, "Hardware test completed");

    // Clear LEDs after test
    led_clear_all();
    led_show();

    // Configure and initialize distance sensor
    distance_sensor_config_t distance_config = {
        .trigger_pin = DISTANCE_TRIGGER,
        .echo_pin = DISTANCE_ECHO,
        .measurement_interval_ms = DISTANCE_INTERVAL,
        .timeout_ms = DISTANCE_TIMEOUT,
        .temperature_celsius = TEMPERATURE_C,
        .smoothing_alpha = 0.3f};  // EMA smoothing factor

    ret = distance_sensor_init(&distance_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize distance sensor: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ret = distance_sensor_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start distance sensor: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "Distance sensor initialized and started");
    ESP_LOGI(TAG, "Hardware: LED=GPIO%d, Trigger=GPIO%d, Echo=GPIO%d", LED_DATA_PIN, DISTANCE_TRIGGER, DISTANCE_ECHO);


    // Initialize and start WiFi manager with smart boot logic
    ret = wifi_manager_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize WiFi manager: %s", esp_err_to_name(ret));
        return;
    }

    ret = wifi_manager_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start WiFi manager: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "WiFi manager initialized and started");
    ESP_LOGI(TAG, "Ready for distance measurement, LED display, and web interface...");


    // Configure and initialize display logic
    display_config_t display_config = {
        .min_distance_cm = 10.0f,
        .max_distance_cm = 50.0f
    };

    ret = display_logic_init(&display_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize display logic: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ret = display_logic_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start display logic: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "Display logic initialized and started");
    ESP_LOGI(TAG, "Ready for distance measurement and LED display...");

    // Main application loop - Coordination and monitoring only
    while (1)
    {
        // Check for queue overflows (indicates system overload)
        uint32_t overflows = distance_sensor_get_queue_overflows();
        static uint32_t last_overflow_count = 0;
        if (overflows > last_overflow_count)
        {
            ESP_LOGW(TAG, "Distance sensor queue overflows: %lu", overflows);
            last_overflow_count = overflows;
        }

        // Periodic WiFi status logging (every 30 seconds)
        static uint32_t wifi_status_counter = 0;
        if (++wifi_status_counter >= 3000) // 3000 * 10ms = 30 seconds
        {
            wifi_status_t wifi_status;
            if (wifi_manager_get_status(&wifi_status) == ESP_OK)
            {
                const char *mode_str = "";
                switch (wifi_status.mode)
                {
                case WIFI_MODE_DISCONNECTED:
                    mode_str = "Disconnected";
                    break;
                case WIFI_MODE_STA_CONNECTING:
                    mode_str = "Connecting";
                    break;
                case WIFI_MODE_STA_CONNECTED:
                    mode_str = "Connected (STA)";
                    break;
                case WIFI_MODE_AP_ACTIVE:
                    mode_str = "Access Point";
                    break;
                case WIFI_MODE_SWITCHING:
                    mode_str = "Switching";
                    break;
                default:
                    mode_str = "Unknown";
                    break;
                }
                
                char ip_str[16] = "N/A";
                wifi_manager_get_ip_address(ip_str, sizeof(ip_str));
                
                ESP_LOGI(TAG, "WiFi Status: %s | IP: %s | SSID: %s", 
                         mode_str, ip_str, 
                         wifi_status.connected_ssid[0] ? wifi_status.connected_ssid : "N/A");
            }
            wifi_status_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Short yield to other tasks
    }
}
