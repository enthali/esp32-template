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
     * @brief Test task configuration
     */
    typedef struct
    {
        uint32_t stack_size;  ///< Task stack size in bytes
        UBaseType_t priority; ///< Task priority (1-10 recommended)
        BaseType_t core_id;   ///< CPU core to run on (0, 1, or tskNO_AFFINITY)
    } test_task_config_t;

    /**
     * @brief Create and start the LED test task
     *
     * @param config Task configuration, or NULL for defaults
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t test_task_start(const test_task_config_t *config);

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
