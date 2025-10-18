/**
 * @file main.c
 * @brief ESP32 Template - Minimal Application Entry Point
 * 
 * This is a minimal template for ESP32 applications. It demonstrates:
 * - Basic initialization and logging
 * - NVS flash initialization
 * - Main application loop structure
 * 
 * Add your application components and logic here.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

static const char *TAG = "main";

/**
 * @brief Main application entry point
 * 
 * This is where your ESP32 application starts. Initialize your components
 * and start your application logic here.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Template Starting...");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    // Initialize NVS (Non-Volatile Storage)
    // This is required for WiFi, Bluetooth, and other system components
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Flash initialized successfully");
    
    // TODO: Initialize your components here
    // Example: config_manager_init();
    // Example: wifi_manager_init();
    // Example: web_server_start();
    
    ESP_LOGI(TAG, "Template initialized successfully");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "  ESP32 Project Template");
    ESP_LOGI(TAG, "  Add your application code in main/main.c");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");
    
    // Main application loop
    // Replace this with your application logic
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Template running... (uptime: %lu seconds)", (unsigned long)(esp_timer_get_time() / 1000000));
    }
}
