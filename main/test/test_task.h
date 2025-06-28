/**
 * @file test_task.h
 * @brief LED Controller Test Task
 *
 * Background task for running LED hardware tests
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Create and start the LED test task with default configuration
     *
     * Creates a low-priority background task that runs continuous LED tests.
     * The task runs on core 1 with 4KB stack at priority 2.
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t test_task_start(void);

    /**
     * @brief Stop the test task
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t test_task_stop(void);

    /**
     * @brief Check if test task is running
     *
     * @return true if running, false otherwise
     */
    bool test_task_is_running(void);

#ifdef __cplusplus
}
#endif
