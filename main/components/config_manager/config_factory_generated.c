// Auto-generated from config_schema.json - DO NOT EDIT MANUALLY
// Generator: tools/generate_config_factory.py

#include "config_manager.h"
#include "esp_log.h"

static const char *TAG = "config_factory";

/**
 * @brief Write factory default values to NVS
 * 
 * This function is auto-generated from config_schema.json.
 * It writes all default configuration values to NVS storage.
 */
void config_write_factory_defaults(void)
{
    ESP_LOGI(TAG, "Writing factory defaults to NVS...");

    config_set_string("wifi_ssid", "");
    config_set_string("wifi_pass", "");
    config_set_int16("led_count", 60);
    config_set_int16("led_bright", 128);
    config_set_string("device_name", "ESP32-Device");

    ESP_LOGI(TAG, "Factory defaults written successfully");
}
