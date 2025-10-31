/**
 * @file config_manager.c
 * @brief JSON Schema-Driven Configuration Manager Implementation
 * 
 * Simple NVS wrapper with no validation logic. Browser performs validation.
 * Direct key-value storage - no enums, no metadata tables, no runtime caches.
 * 
 * ARCHITECTURE:
 * - JSON schema (config_schema.json) defines all parameters
 * - Python script generates factory defaults at build time
 * - config_get/set functions are thin wrappers around NVS API
 * - Zero RAM overhead (direct NVS access, no caches)
 * 
 * @author ESP32 Template Project
 * @date 2025
 * @version 3.0 (JSON-based)
 * 
 * Requirements Traceability:
 * - REQ_CFG_JSON_6: Key-Based NVS Storage
 * - REQ_CFG_JSON_7: Type-Safe Configuration API
 * - REQ_CFG_JSON_8: Persistent Configuration Storage
 * - REQ_CFG_JSON_11: NVS Error Graceful Handling
 * - REQ_CFG_JSON_12: Configuration Initialization on Boot
 */

#include "config_manager.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

static const char *TAG = "config";

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

/** @brief NVS namespace for configuration storage */
#define NVS_NAMESPACE "config"

/** @brief NVS handle for config namespace */
static nvs_handle_t config_nvs_handle = 0;

/** @brief Initialization flag */
static bool config_initialized = false;

// =============================================================================
// PRIVATE HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Open NVS handle if not already open
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t ensure_nvs_open(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized - call config_init() first");
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t config_commit(void) {
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit NVS changes: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "NVS changes committed successfully");
    return ESP_OK;
}

// =============================================================================
// LIFECYCLE FUNCTIONS (REQ_CFG_JSON_12)
// =============================================================================

esp_err_t config_init(void) {
    if (config_initialized) {
        ESP_LOGW(TAG, "Config manager already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing configuration manager...");
    
    // Initialize NVS if not already done
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated, erase and re-initialize
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Open NVS namespace "config"
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s': %s", 
                 NVS_NAMESPACE, esp_err_to_name(ret));
        return ret;
    }
    
    config_initialized = true;
    ESP_LOGI(TAG, "Configuration manager initialized successfully");
    
    // Check if NVS is empty (first boot) - check for a known key
    char test_buffer[64];
    size_t test_len = sizeof(test_buffer);
    ret = nvs_get_str(config_nvs_handle, "device_name", test_buffer, &test_len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        // First boot - write factory defaults
        ESP_LOGI(TAG, "First boot detected, writing factory defaults...");
        config_factory_reset();
    }
    
    return ESP_OK;
}

esp_err_t config_factory_reset(void) {
    ESP_LOGI(TAG, "Resetting configuration to factory defaults...");
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Erase all keys in "config" namespace
    ret = nvs_erase_all(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase NVS namespace: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Write factory defaults (auto-generated function)
    config_write_factory_defaults();
    
    // Commit changes
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit factory defaults to NVS: %s", 
                 esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Factory defaults written successfully");
    return ESP_OK;
}

// =============================================================================
// STRING PARAMETER ACCESS (REQ_CFG_JSON_7)
// =============================================================================

esp_err_t config_get_string(const char* key, char* buffer, size_t buf_len) {
    if (key == NULL || buffer == NULL) {
        ESP_LOGE(TAG, "config_get_string: key or buffer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    size_t required_size = buf_len;
    ret = nvs_get_str(config_nvs_handle, key, buffer, &required_size);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        buffer[0] = '\0';  // Return empty string
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read string key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t config_set_string_no_commit(const char* key, const char* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_set_string_no_commit: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS without commit
    ret = nvs_set_str(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write string key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set string '%s' = '%s' (no commit)", key, value);
    return ESP_OK;
}

esp_err_t config_set_string(const char* key, const char* value) {
    esp_err_t ret = config_set_string_no_commit(key, value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Commit immediately for persistence
    return config_commit();
}

// =============================================================================
// INTEGER PARAMETER ACCESS (REQ_CFG_JSON_7)
// =============================================================================

esp_err_t config_get_int32(const char* key, int32_t* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_get_int32: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_get_i32(config_nvs_handle, key, value);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        *value = 0;  // Return 0 as default
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t config_set_int32(const char* key, int32_t value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_int32: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS
    ret = nvs_set_i32(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    // Commit immediately for persistence
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set int32 '%s' = %ld", key, (long)value);
    return ESP_OK;
}

esp_err_t config_get_int16(const char* key, int16_t* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_get_int16: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_get_i16(config_nvs_handle, key, value);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        *value = 0;  // Return 0 as default
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read int16 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t config_set_int16_no_commit(const char* key, int16_t value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_int16_no_commit: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS without commit
    ret = nvs_set_i16(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write int16 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set int16 '%s' = %d (no commit)", key, value);
    return ESP_OK;
}

esp_err_t config_set_int16(const char* key, int16_t value) {
    esp_err_t ret = config_set_int16_no_commit(key, value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Commit immediately for persistence
    return config_commit();
}

// =============================================================================
// BOOLEAN PARAMETER ACCESS (REQ_CFG_JSON_7)
// =============================================================================

esp_err_t config_get_bool(const char* key, bool* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_get_bool: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t u8_value;
    ret = nvs_get_u8(config_nvs_handle, key, &u8_value);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        *value = false;  // Return false as default
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read bool key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    *value = (u8_value != 0);
    return ESP_OK;
}

esp_err_t config_set_bool(const char* key, bool value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_bool: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Store bool as uint8
    uint8_t u8_value = value ? 1 : 0;
    
    // Write to NVS
    ret = nvs_set_u8(config_nvs_handle, key, u8_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write bool key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    // Commit immediately for persistence
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit bool key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set bool '%s' = %s", key, value ? "true" : "false");
    return ESP_OK;
}
