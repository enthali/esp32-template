/**
 * @file test_task.c
 * @brief LED Controller Test Task Implementation
 */

#include "test_task.h"
#include "../led_controller.h"
#include "led_running_test.h"
#include "led_color_test.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "test_task";

// Task handle
static TaskHandle_t test_task_handle = NULL;

/**
 * @brief Main test task function
 */
static void test_task_main(void *pvParameters)
{
    ESP_LOGI(TAG, "LED Test Task started (Priority: %d, Core: %d)", 
             uxTaskPriorityGet(NULL), xPortGetCoreID());

    while (1) {
        ESP_LOGI(TAG, "=== Starting LED Hardware Tests ===");

        // Test 1: Running light effect (3 cycles)
        ESP_LOGI(TAG, "Running Light Test (3 cycles)");
        led_running_test_multiple_cycles(LED_COLOR_GREEN, 50, 3);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Test 2: Basic color display
        ESP_LOGI(TAG, "Basic Colors Test");
        led_color_test_basic_colors(2000);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Test 3: Brightness fade test
        ESP_LOGI(TAG, "Brightness Fade Test");
        led_color_test_brightness_fade_basic(20);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Clear all LEDs
        led_clear_all();
        led_show();

        ESP_LOGI(TAG, "=== Test Cycle Complete ===");
        
        // Wait 10 seconds before next test cycle
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

esp_err_t test_task_start(void)
{
    if (test_task_handle != NULL) {
        ESP_LOGW(TAG, "Test task already running");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Creating LED test task...");

    BaseType_t result = xTaskCreatePinnedToCore(
        test_task_main,           // Task function
        "led_test",               // Task name
        4096,                     // Stack size (4KB)
        NULL,                     // Parameters
        2,                        // Priority (low priority background task)
        &test_task_handle,        // Task handle
        1                         // Core ID (run on core 1)
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create test task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "LED test task created and started successfully");
    return ESP_OK;
}

esp_err_t test_task_stop(void)
{
    if (test_task_handle == NULL) {
        ESP_LOGW(TAG, "Test task not running");
        return ESP_ERR_INVALID_STATE;
    }

    vTaskDelete(test_task_handle);
    test_task_handle = NULL;
    
    ESP_LOGI(TAG, "Test task stopped");
    return ESP_OK;
}

bool test_task_is_running(void)
{
    return (test_task_handle != NULL);
}
