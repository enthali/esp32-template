/**
 * @file config_manager.c
 * @brief Generic Configuration Management Implementation
 * 
 * Metadata-driven configuration system with type-specific parameter tables.
 * Completely generic - no hardcoded knowledge of application parameters.
 * 
 * ARCHITECTURE:
 * - Imports parameter tables from config.h (application-specific)
 * - Maintains separate runtime caches for uint16 and string parameters
 * - Uses NVS with type-prefixed keys ("u0", "s0") for storage
 * - Metadata-driven validation (no switch statements on parameter IDs)
 * 
 * @author ESP32 Template Project
 * @date 2025
 * @version 2.0
 * 
 * Requirements Traceability:
 * - REQ_CFG_4: NVS persistent storage
 * - REQ_CFG_5: Configuration API implementation
 * - REQ_CFG_6: Parameter validation
 */

#include "config_manager.h"
#include "../../config.h"  // Import application parameter tables
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

static const char *TAG = "config";

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

/** @brief NVS namespace for configuration storage */
#define NVS_NAMESPACE "config"

/** @brief Mutex for thread-safe access to configuration */
static SemaphoreHandle_t config_mutex = NULL;

/** @brief Initialization flag */
static bool config_initialized = false;

/** @brief Runtime cache for uint16 parameters */
static uint16_t cache_uint16[CONFIG_UINT16_COUNT];

/** @brief Runtime cache for string parameters (max 65 bytes including null) */
static char cache_strings[CONFIG_STRING_COUNT][CONFIG_STRING_MAX_LEN + 1];

// =============================================================================
// PRIVATE HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Generate NVS key for uint16 parameter
 * @param id Parameter ID
 * @param key_buf Buffer to store key (min 4 bytes)
 */
static inline void generate_uint16_key(uint32_t id, char* key_buf) {
    snprintf(key_buf, 4, "u%" PRIu32, id);
}

/**
 * @brief Generate NVS key for string parameter
 * @param id Parameter ID
 * @param key_buf Buffer to store key (min 4 bytes)
 */
static inline void generate_string_key(uint32_t id, char* key_buf) {
    snprintf(key_buf, 4, "s%" PRIu32, id);
}

/**
 * @brief Validate uint16 parameter value against metadata constraints
 * @param id Parameter ID
 * @param value Value to validate
 * @return ESP_OK if valid, ESP_ERR_INVALID_ARG if out of range
 */
static esp_err_t validate_uint16(uint32_t id, uint16_t value) {
    if (id >= CONFIG_UINT16_COUNT) {
        ESP_LOGE(TAG, "Invalid uint16 parameter ID: %" PRIu32, id);
        return ESP_ERR_INVALID_ARG;
    }
    
    const config_uint16_param_t* param = &CONFIG_UINT16_PARAMS[id];
    
    if (value < param->min || value > param->max) {
        ESP_LOGE(TAG, "Parameter u%" PRIu32 " out of range: %u (min=%u, max=%u)",
                 id, value, param->min, param->max);
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

/**
 * @brief Validate string parameter value against metadata constraints
 * @param id Parameter ID
 * @param value String value to validate (must not be NULL)
 * @return ESP_OK if valid, ESP_ERR_INVALID_ARG if length out of range
 */
static esp_err_t validate_string(uint32_t id, const char* value) {
    if (id >= CONFIG_STRING_COUNT) {
        ESP_LOGE(TAG, "Invalid string parameter ID: %" PRIu32, id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "String parameter s%" PRIu32 ": NULL value not allowed", id);
        return ESP_ERR_INVALID_ARG;
    }
    
    const config_string_param_t* param = &CONFIG_STRING_PARAMS[id];
    size_t len = strlen(value);
    
    if (len < param->min_len || len > param->max_len) {
        ESP_LOGE(TAG, "String parameter s%" PRIu32 " length invalid: %zu (min=%zu, max=%zu)",
                 id, len, param->min_len, param->max_len);
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

/**
 * @brief Load uint16 parameter from NVS or use default
 * @param handle NVS handle
 * @param id Parameter ID
 * @return ESP_OK on success
 */
static esp_err_t load_uint16_param(nvs_handle_t handle, uint32_t id) {
    char key[4];
    generate_uint16_key(id, key);
    
    uint16_t value;
    esp_err_t err = nvs_get_u16(handle, key, &value);
    
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Use default value
        value = CONFIG_UINT16_PARAMS[id].default_val;
        ESP_LOGD(TAG, "Parameter %s not found in NVS, using default: %u", key, (unsigned int)value);
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read %s from NVS: %s", key, esp_err_to_name(err));
        return err;
    }
    
    // Validate and store in cache
    err = validate_uint16(id, value);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Invalid value for %s in NVS, using default", key);
        value = CONFIG_UINT16_PARAMS[id].default_val;
    }
    
    cache_uint16[id] = value;
    return ESP_OK;
}

/**
 * @brief Load string parameter from NVS or use default
 * @param handle NVS handle
 * @param id Parameter ID
 * @return ESP_OK on success
 */
static esp_err_t load_string_param(nvs_handle_t handle, uint32_t id) {
    char key[4];
    generate_string_key(id, key);
    
    size_t required_size = CONFIG_STRING_MAX_LEN + 1;
    esp_err_t err = nvs_get_str(handle, key, cache_strings[id], &required_size);
    
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Use default value
        const char* default_val = CONFIG_STRING_PARAMS[id].default_val;
        strncpy(cache_strings[id], default_val, CONFIG_STRING_MAX_LEN);
        cache_strings[id][CONFIG_STRING_MAX_LEN] = '\0';
        ESP_LOGD(TAG, "Parameter %s not found in NVS, using default: \"%s\"", key, default_val);
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read %s from NVS: %s", key, esp_err_to_name(err));
        return err;
    }
    
    // Validate
    err = validate_string(id, cache_strings[id]);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Invalid value for %s in NVS, using default", key);
        const char* default_val = CONFIG_STRING_PARAMS[id].default_val;
        strncpy(cache_strings[id], default_val, CONFIG_STRING_MAX_LEN);
        cache_strings[id][CONFIG_STRING_MAX_LEN] = '\0';
    }
    
    return ESP_OK;
}

// =============================================================================
// PUBLIC API IMPLEMENTATION
// =============================================================================

esp_err_t config_init(void) {
    if (config_initialized) {
        ESP_LOGW(TAG, "Config manager already initialized");
        return ESP_OK;
    }
    
    // Create mutex
    config_mutex = xSemaphoreCreateMutex();
    if (config_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create config mutex");
        return ESP_ERR_NO_MEM;
    }
    
    // Open NVS namespace
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        vSemaphoreDelete(config_mutex);
        config_mutex = NULL;
        return err;
    }
    
    // Load all uint16 parameters
    for (uint32_t i = 0; i < CONFIG_UINT16_COUNT; i++) {
        err = load_uint16_param(nvs_handle, i);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to load uint16 parameter %" PRIu32, i);
            // Continue loading other parameters
        }
    }
    
    // Load all string parameters
    for (uint32_t i = 0; i < CONFIG_STRING_COUNT; i++) {
        err = load_string_param(nvs_handle, i);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to load string parameter %" PRIu32, i);
            // Continue loading other parameters
        }
    }
    
    nvs_close(nvs_handle);
    
    config_initialized = true;
    ESP_LOGI(TAG, "Config manager initialized (%" PRIu32 " uint16, %" PRIu32 " string parameters)",
             (uint32_t)CONFIG_UINT16_COUNT, (uint32_t)CONFIG_STRING_COUNT);
    
    return ESP_OK;
}

esp_err_t config_factory_reset(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Acquire mutex
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire config mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    // Open NVS namespace
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        xSemaphoreGive(config_mutex);
        return err;
    }
    
    // Erase all entries
    err = nvs_erase_all(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        xSemaphoreGive(config_mutex);
        return err;
    }
    
    // Load defaults into cache and persist
    char key[4];
    
    // Uint16 defaults
    for (uint32_t i = 0; i < CONFIG_UINT16_COUNT; i++) {
        uint16_t default_val = CONFIG_UINT16_PARAMS[i].default_val;
        cache_uint16[i] = default_val;
        
        generate_uint16_key(i, key);
        err = nvs_set_u16(nvs_handle, key, default_val);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write default for %s: %s", key, esp_err_to_name(err));
        }
    }
    
    // String defaults
    for (uint32_t i = 0; i < CONFIG_STRING_COUNT; i++) {
        const char* default_val = CONFIG_STRING_PARAMS[i].default_val;
        strncpy(cache_strings[i], default_val, CONFIG_STRING_MAX_LEN);
        cache_strings[i][CONFIG_STRING_MAX_LEN] = '\0';
        
        generate_string_key(i, key);
        err = nvs_set_str(nvs_handle, key, default_val);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write default for %s: %s", key, esp_err_to_name(err));
        }
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    xSemaphoreGive(config_mutex);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Factory reset completed successfully");
    } else {
        ESP_LOGE(TAG, "Factory reset commit failed: %s", esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t config_get_uint16(uint32_t id, uint16_t* value) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (id >= CONFIG_UINT16_COUNT) {
        ESP_LOGE(TAG, "Invalid uint16 parameter ID: %" PRIu32, id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "NULL value pointer");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Acquire mutex
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire config mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    *value = cache_uint16[id];
    xSemaphoreGive(config_mutex);
    
    return ESP_OK;
}

esp_err_t config_set_uint16(uint32_t id, uint16_t value) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Validate first (before acquiring mutex)
    esp_err_t err = validate_uint16(id, value);
    if (err != ESP_OK) {
        return err;
    }
    
    // Acquire mutex
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire config mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    // Open NVS
    nvs_handle_t nvs_handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        xSemaphoreGive(config_mutex);
        return err;
    }
    
    // Write to NVS
    char key[4];
    generate_uint16_key(id, key);
    err = nvs_set_u16(nvs_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write %s to NVS: %s", key, esp_err_to_name(err));
        nvs_close(nvs_handle);
        xSemaphoreGive(config_mutex);
        return err;
    }
    
    // Commit
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (err == ESP_OK) {
        // Update cache only after successful NVS write
        cache_uint16[id] = value;
        ESP_LOGD(TAG, "Set %s = %u", key, (unsigned int)value);
    } else {
        ESP_LOGE(TAG, "Failed to commit %s to NVS: %s", key, esp_err_to_name(err));
    }
    
    xSemaphoreGive(config_mutex);
    return err;
}

esp_err_t config_get_string(uint32_t id, char* buffer, size_t buf_len) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (id >= CONFIG_STRING_COUNT) {
        ESP_LOGE(TAG, "Invalid string parameter ID: %" PRIu32, id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (buffer == NULL || buf_len == 0) {
        ESP_LOGE(TAG, "Invalid buffer");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Acquire mutex
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire config mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    // Check buffer size
    size_t required_len = strlen(cache_strings[id]) + 1;
    if (buf_len < required_len) {
        ESP_LOGE(TAG, "Buffer too small: need %zu, have %zu", required_len, buf_len);
        xSemaphoreGive(config_mutex);
        return ESP_ERR_INVALID_SIZE;
    }
    
    strncpy(buffer, cache_strings[id], buf_len);
    buffer[buf_len - 1] = '\0';  // Ensure null termination
    
    xSemaphoreGive(config_mutex);
    return ESP_OK;
}

esp_err_t config_set_string(uint32_t id, const char* value) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Validate first (before acquiring mutex)
    esp_err_t err = validate_string(id, value);
    if (err != ESP_OK) {
        return err;
    }
    
    // Acquire mutex
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire config mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    // Open NVS
    nvs_handle_t nvs_handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        xSemaphoreGive(config_mutex);
        return err;
    }
    
    // Write to NVS
    char key[4];
    generate_string_key(id, key);
    err = nvs_set_str(nvs_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write %s to NVS: %s", key, esp_err_to_name(err));
        nvs_close(nvs_handle);
        xSemaphoreGive(config_mutex);
        return err;
    }
    
    // Commit
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (err == ESP_OK) {
        // Update cache only after successful NVS write
        strncpy(cache_strings[id], value, CONFIG_STRING_MAX_LEN);
        cache_strings[id][CONFIG_STRING_MAX_LEN] = '\0';
        ESP_LOGD(TAG, "Set %s = \"%s\"", key, value);
    } else {
        ESP_LOGE(TAG, "Failed to commit %s to NVS: %s", key, esp_err_to_name(err));
    }
    
    xSemaphoreGive(config_mutex);
    return err;
}
