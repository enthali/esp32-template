/**
 * @file main.c
 * @brief ESP32 Template - Application Entry Point with Web Configuration
 * 
 * This template provides:
 * - WiFi connectivity (STA mode with AP fallback)
 * - Web-based configuration interface (captive portal)
 * - JSON-based configuration management (NVS storage)
 * - QEMU network support (UART tunnel for emulation)
 * 
 * SYSTEM ARCHITECTURE:
 * - Configuration Manager: JSON schema-based settings with NVS persistence
 * - WiFi Manager: Network connectivity with automatic web server lifecycle
 * - Web Server: Configuration interface with captive portal support
 * 
 * Customize the web interface and add your application components below.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

// Template components - Configuration and Connectivity
#include "config_manager.h"
#include "wifi_manager.h"

static const char *TAG = "main";

/**
 * @brief Main application entry point
 * 
 * Initializes the ESP32 template with web-based configuration interface.
 * System initialization follows these steps:
 * 1. Configuration manager (NVS + JSON schema)
 * 2. WiFi manager and web server (automatic lifecycle management)
 * 3. Your custom application components (add here)
 * 
 * The device will:
 * - Try to connect to saved WiFi (STA mode)
 * - Fall back to AP mode with captive portal if connection fails
 * - Provide web interface for configuration and monitoring
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║      ESP32 Project Template - Starting...      ║");
    ESP_LOGI(TAG, "║      WiFi + Web Config + JSON Settings         ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    // Step 1: Initialize configuration manager
    // Internally initializes NVS and loads configuration from storage
    // Uses JSON schema from config_schema.json for all parameters
    ESP_LOGI(TAG, "Initializing configuration manager...");
    ESP_ERROR_CHECK(config_init());
    ESP_LOGI(TAG, "✓ Configuration manager initialized (NVS ready)");
    
    // Step 2: Initialize WiFi manager and web server
    // Handles both STA mode (connect to WiFi) and AP mode (captive portal)
    // WiFi manager automatically starts web server in both modes:
    //   - AP mode: Web server on 192.168.4.1 (captive portal)
    //   - STA mode: Web server on network IP (after connection)
    ESP_LOGI(TAG, "Initializing WiFi manager...");
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(wifi_manager_start());
    ESP_LOGI(TAG, "✓ WiFi manager initialized (web server lifecycle managed automatically)");
    
    // Step 3: Initialize your custom application components
    // Add your hardware initialization, sensors, displays, etc. here
    // Example:
    //   ESP_LOGI(TAG, "Initializing custom hardware...");
    //   ESP_ERROR_CHECK(my_sensor_init());
    //   ESP_ERROR_CHECK(my_display_init());
    //   ESP_LOGI(TAG, "✓ Custom components initialized");
    
    // System initialized successfully
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║          Template Ready!                   ║");
    ESP_LOGI(TAG, "║  Web Interface: http://192.168.4.1         ║");
    ESP_LOGI(TAG, "║  Captive Portal: Auto (AP mode)            ║");
    ESP_LOGI(TAG, "║  QEMU: http://localhost:8080               ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    // Main monitoring loop
    // Lightweight periodic health checks and logging
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 second interval
        
        // Monitor WiFi and system health
        wifi_manager_monitor();  // Check WiFi connection status
        
        // Log system metrics
        uint32_t heap_free = esp_get_free_heap_size();
        uint32_t heap_min = esp_get_minimum_free_heap_size();
        uint32_t uptime_s = (uint32_t)(esp_timer_get_time() / 1000000);
        
        ESP_LOGD(TAG, "Uptime: %lu s | Heap: %lu/%lu bytes",
                 uptime_s, heap_free, heap_min);
    }
}
