/**
 * @file config_manager.h
 * @brief Generic Configuration Management API
 * 
 * This module provides a metadata-driven configuration system with:
 * - Type-specific parameter tables (uint16 and string)
 * - Persistent NVS storage with individual parameter access
 * - Metadata-driven validation (no hardcoded parameter logic)
 * - Thread-safe access with mutex protection
 * - Automatic fallback to factory defaults
 * 
 * ARCHITECTURE:
 * The config manager is completely generic and has NO knowledge of application
 * parameters (WiFi, LED, sensors, etc.). All parameter definitions live in
 * the application's config.h file.
 * 
 * THREAD SAFETY:
 * All functions are thread-safe and can be called from multiple tasks simultaneously.
 * Internal mutex protection ensures data consistency.
 * 
 * MEMORY EFFICIENCY:
 * Dual-table design (uint16 + string) saves ~73% RAM vs union approach:
 * - Union: 20 params × 65 bytes = 1300 bytes
 * - Dual-table: (15 × 2) + (5 × 65) = 355 bytes
 * 
 * ERROR HANDLING:
 * All functions return esp_err_t codes for proper error handling integration
 * with ESP-IDF error handling patterns.
 * 
 * @author ESP32 Template Project
 * @date 2025
 * @version 2.0
 * 
 * Requirements Traceability:
 * - REQ_CFG_3: Configuration Data Structure
 * - REQ_CFG_4: Non-Volatile Storage (NVS)
 * - REQ_CFG_5: Configuration API
 * - REQ_CFG_6: Parameter Validation
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PARAMETER TYPE DEFINITIONS (REQ_CFG_3)
// =============================================================================

/**
 * @brief Maximum string length (excluding null terminator)
 * 
 * Balances memory efficiency with typical use cases:
 * - WiFi SSID: max 32 chars (IEEE 802.11)
 * - WiFi password: max 63 chars (WPA2)
 * - Device names, URLs: typically < 64 chars
 */
#define CONFIG_STRING_MAX_LEN 64

/**
 * @brief UINT16 parameter metadata
 * 
 * Defines constraints for a numeric parameter.
 * Stored in Flash ROM (const), zero RAM cost.
 */
typedef struct {
    uint16_t min;          ///< Minimum allowed value (inclusive)
    uint16_t max;          ///< Maximum allowed value (inclusive)
    uint16_t default_val;  ///< Factory default value
} config_uint16_param_t;

/**
 * @brief STRING parameter metadata
 * 
 * Defines constraints for a string parameter.
 * Stored in Flash ROM (const), zero RAM cost.
 * Default value pointer references Flash ROM string.
 */
typedef struct {
    uint8_t min_len;           ///< Minimum string length (0 = empty string allowed)
    uint8_t max_len;           ///< Maximum string length (max CONFIG_STRING_MAX_LEN)
    const char* default_val;   ///< Pointer to default string in Flash ROM
} config_string_param_t;

// =============================================================================
// CONFIGURATION API (REQ_CFG_5)
// =============================================================================

/**
 * @brief Initialize configuration management subsystem
 * 
 * Initializes the configuration manager, creates mutex protection,
 * opens NVS namespace "config", and loads all parameters from NVS into
 * runtime caches. If NVS is empty or corrupted, loads default values
 * from parameter tables and persists them.
 * 
 * Must be called once during system startup before any other config functions.
 * 
 * @return ESP_OK on success
 * @return ESP_ERR_NO_MEM if memory allocation fails
 * @return ESP_ERR_NVS_* on NVS initialization failure
 * 
 * @requirement REQ_CFG_5 AC-1
 */
esp_err_t config_init(void);

/**
 * @brief Reset all parameters to factory defaults
 * 
 * Loads default values from CONFIG_UINT16_PARAMS and CONFIG_STRING_PARAMS
 * metadata tables (defined in config.h), erases all NVS entries in "config"
 * namespace, and persists defaults to NVS.
 * 
 * @return ESP_OK on success
 * @return ESP_ERR_NVS_* on NVS operation failure
 * 
 * @requirement REQ_CFG_5 AC-6, AC-9
 */
esp_err_t config_factory_reset(void);

// =============================================================================
// UINT16 PARAMETER ACCESS (REQ_CFG_5)
// =============================================================================

/**
 * @brief Get uint16 parameter value
 * 
 * Reads parameter from runtime cache (RAM) without NVS access.
 * Fast operation (<10 CPU cycles). Thread-safe with mutex protection.
 * 
 * @param[in] id Parameter identifier from config_uint16_id_t enum (defined in config.h)
 * @param[out] value Pointer to store parameter value
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if id out of range or value is NULL
 * @return ESP_ERR_INVALID_STATE if config manager not initialized
 * 
 * @requirement REQ_CFG_5 AC-2
 */
esp_err_t config_get_uint16(uint32_t id, uint16_t* value);

/**
 * @brief Set uint16 parameter value
 * 
 * Validates parameter using metadata table (min, max constraints from config.h),
 * updates runtime cache, and persists to NVS with key "u<id>".
 * Thread-safe with mutex protection.
 * 
 * @param[in] id Parameter identifier from config_uint16_id_t enum (defined in config.h)
 * @param[in] value New parameter value
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if id out of range or value violates constraints
 * @return ESP_ERR_INVALID_STATE if config manager not initialized
 * @return ESP_ERR_NVS_* on NVS write failure
 * 
 * @requirement REQ_CFG_5 AC-4, REQ_CFG_6 AC-1
 */
esp_err_t config_set_uint16(uint32_t id, uint16_t value);

// =============================================================================
// STRING PARAMETER ACCESS (REQ_CFG_5)
// =============================================================================

/**
 * @brief Get string parameter value
 * 
 * Copies string from runtime cache to user buffer.
 * Thread-safe with mutex protection.
 * 
 * @param[in] id Parameter identifier from config_string_id_t enum (defined in config.h)
 * @param[out] buffer Buffer to store string (must be at least buf_len bytes)
 * @param[in] buf_len Maximum buffer size (including null terminator)
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if id out of range, buffer is NULL, or buf_len too small
 * @return ESP_ERR_INVALID_STATE if config manager not initialized
 * 
 * @requirement REQ_CFG_5 AC-2
 */
esp_err_t config_get_string(uint32_t id, char* buffer, size_t buf_len);

/**
 * @brief Set string parameter value
 * 
 * Validates string length using metadata table (min_len, max_len from config.h),
 * copies to runtime cache, and persists to NVS with key "s<id>".
 * Thread-safe with mutex protection.
 * 
 * @param[in] id Parameter identifier from config_string_id_t enum (defined in config.h)
 * @param[in] value Null-terminated string value (must not be NULL)
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if id out of range, value is NULL, or length violates constraints
 * @return ESP_ERR_INVALID_STATE if config manager not initialized
 * @return ESP_ERR_NVS_* on NVS write failure
 * 
 * @requirement REQ_CFG_5 AC-4, REQ_CFG_6 AC-2
 */
esp_err_t config_set_string(uint32_t id, const char* value);

#ifdef __cplusplus
}
#endif
