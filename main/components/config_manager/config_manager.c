/**
 * @file config_manager.c
 * @brief Configuration Management Implementation for ESP32 Distance Sensor Project
 * 
 * Implementation of runtime configuration management with NVS persistence,
 * parameter validation, and thread-safe access.
 * 
 * @author ESP32 Distance Project Team
 * @date 2025
 * @version 1.0
 */

#include "config_manager.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "config_manager";

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

/**
 * @brief NVS namespace for configuration storage
 */
#define NVS_NAMESPACE "esp32_config"

/**
 * @brief NVS key for configuration blob
 */
#define NVS_CONFIG_KEY "config"

/**
 * @brief Mutex for thread-safe access to configuration
 */
static SemaphoreHandle_t config_mutex = NULL;

/**
 * @brief Current runtime configuration
 */
static system_config_t current_config;

/**
 * @brief Initialization flag
 */
static bool config_initialized = false;

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

/**
 * @brief Initialize configuration structure with factory defaults
 * @param[out] config Configuration structure to initialize
 */
static void config_init_defaults(system_config_t* config);

/**
 * @brief Validate inter-parameter relationships
 * @param[in] config Configuration to validate
 * @return ESP_OK if relationships are valid, ESP_ERR_INVALID_SIZE otherwise
 */
static esp_err_t config_validate_relationships(const system_config_t* config);

// =============================================================================
// PUBLIC API IMPLEMENTATION
// =============================================================================

esp_err_t config_init(void)
{
    if (config_initialized) {
        ESP_LOGW(TAG, "Configuration manager already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing configuration management subsystem");

    // Create mutex for thread safety
    config_mutex = xSemaphoreCreateMutex();
    if (config_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create configuration mutex");
        return ESP_ERR_NO_MEM;
    }

    // Initialize NVS if not already done
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS requires format, erasing and reinitializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        vSemaphoreDelete(config_mutex);
        config_mutex = NULL;
        return ret;
    }

    // Load configuration from NVS (will fall back to defaults if not found)
    ret = config_load(&current_config);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to load initial configuration, using factory defaults: %s", esp_err_to_name(ret));
        // Don't fail initialization - use factory defaults instead
        config_init_defaults(&current_config);
        
        // Try to save the factory defaults to establish a good NVS state
        esp_err_t save_ret = config_save(&current_config);
        if (save_ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to save factory defaults to NVS: %s", esp_err_to_name(save_ret));
            // Continue anyway - we have valid defaults in memory
        }
    }

    config_initialized = true;
    ESP_LOGI(TAG, "Configuration management initialized successfully");
    ESP_LOGI(TAG, "Configuration version: %lu, save count: %lu", 
             current_config.config_version, current_config.save_count);

    return ESP_OK;
}

esp_err_t config_load(system_config_t* config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Take mutex for thread safety
    if (config_mutex != NULL && xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire configuration mutex");
        return ESP_ERR_TIMEOUT;
    }

    ESP_LOGD(TAG, "Loading configuration from NVS");

    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(ret));
        goto factory_reset;
    }

    // Get required size for configuration blob
    size_t required_size = sizeof(system_config_t);
    ret = nvs_get_blob(nvs_handle, NVS_CONFIG_KEY, config, &required_size);
    nvs_close(nvs_handle);

    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "No configuration found in NVS, using factory defaults");
        goto factory_reset;
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read configuration from NVS: %s", esp_err_to_name(ret));
        goto factory_reset;
    }

    // Validate loaded configuration
    ret = config_validate_range(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Loaded configuration failed validation");
        goto factory_reset;
    }

    // Check configuration version compatibility
    if (config->config_version != CONFIG_VERSION) {
        ESP_LOGW(TAG, "Configuration version mismatch (loaded: %lu, current: %d)", 
                 config->config_version, CONFIG_VERSION);
        goto factory_reset;
    }

    if (config_mutex != NULL) {
        xSemaphoreGive(config_mutex);
    }

    ESP_LOGI(TAG, "Configuration loaded successfully from NVS");
    return ESP_OK;

factory_reset:
    if (config_mutex != NULL) {
        xSemaphoreGive(config_mutex);
    }
    
    ESP_LOGW(TAG, "Performing factory reset due to load failure");
    ret = config_factory_reset();
    if (ret == ESP_OK) {
        // Reload the factory defaults
        return config_load(config);
    }
    return ret;
}

esp_err_t config_save(const system_config_t* config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Validate configuration before saving
    esp_err_t ret = config_validate_range(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuration validation failed, not saving");
        return ret;
    }

    // Take mutex for thread safety
    if (config_mutex != NULL && xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire configuration mutex");
        return ESP_ERR_TIMEOUT;
    }

    ESP_LOGD(TAG, "Saving configuration to NVS");

    nvs_handle_t nvs_handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace for write: %s", esp_err_to_name(ret));
        if (config_mutex != NULL) {
            xSemaphoreGive(config_mutex);
        }
        return ret;
    }

    // Create a copy with updated save count
    system_config_t config_to_save = *config;
    config_to_save.save_count++;

    // Write configuration blob atomically
    ret = nvs_set_blob(nvs_handle, NVS_CONFIG_KEY, &config_to_save, sizeof(system_config_t));
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);

    if (ret == ESP_OK) {
        // Update current configuration
        current_config = config_to_save;
        ESP_LOGI(TAG, "Configuration saved successfully (save count: %lu)", config_to_save.save_count);
    } else {
        ESP_LOGE(TAG, "Failed to save configuration to NVS: %s", esp_err_to_name(ret));
    }

    if (config_mutex != NULL) {
        xSemaphoreGive(config_mutex);
    }

    return ret;
}

esp_err_t config_validate_range(const system_config_t* config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGD(TAG, "Validating configuration parameters");

    // Distance sensor parameters
    if (!config_is_valid_int_range("distance_min_mm", config->distance_min_mm, 
                               CONFIG_DISTANCE_MIN_MM_MIN, CONFIG_DISTANCE_MIN_MM_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("distance_max_mm", config->distance_max_mm,
                               CONFIG_DISTANCE_MAX_MM_MIN, CONFIG_DISTANCE_MAX_MM_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("measurement_interval_ms", (int32_t)config->measurement_interval_ms,
                               CONFIG_MEASUREMENT_INTERVAL_MS_MIN, CONFIG_MEASUREMENT_INTERVAL_MS_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("sensor_timeout_ms", (int32_t)config->sensor_timeout_ms,
                               CONFIG_SENSOR_TIMEOUT_MS_MIN, CONFIG_SENSOR_TIMEOUT_MS_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("temperature_c_x10", config->temperature_c_x10,
                               CONFIG_TEMPERATURE_C_X10_MIN, CONFIG_TEMPERATURE_C_X10_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("smoothing_factor", config->smoothing_factor,
                               CONFIG_SMOOTHING_FACTOR_MIN, CONFIG_SMOOTHING_FACTOR_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    // LED parameters
    if (!config_is_valid_int_range("led_count", (int32_t)config->led_count,
                               CONFIG_LED_COUNT_MIN, CONFIG_LED_COUNT_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("led_brightness", (int32_t)config->led_brightness,
                               CONFIG_LED_BRIGHTNESS_MIN, CONFIG_LED_BRIGHTNESS_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    // WiFi parameters
    if (!config_is_valid_int_range("wifi_ap_channel", (int32_t)config->wifi_ap_channel,
                               CONFIG_WIFI_AP_CHANNEL_MIN, CONFIG_WIFI_AP_CHANNEL_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("wifi_ap_max_conn", (int32_t)config->wifi_ap_max_conn,
                               CONFIG_WIFI_AP_MAX_CONN_MIN, CONFIG_WIFI_AP_MAX_CONN_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("wifi_sta_max_retry", (int32_t)config->wifi_sta_max_retry,
                               CONFIG_WIFI_STA_MAX_RETRY_MIN, CONFIG_WIFI_STA_MAX_RETRY_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (!config_is_valid_int_range("wifi_sta_timeout_ms", (int32_t)config->wifi_sta_timeout_ms,
                               CONFIG_WIFI_STA_TIMEOUT_MS_MIN, CONFIG_WIFI_STA_TIMEOUT_MS_MAX)) {
        return ESP_ERR_INVALID_SIZE;
    }

    // Validate inter-parameter relationships
    return config_validate_relationships(config);
}

esp_err_t config_factory_reset(void)
{
    ESP_LOGI(TAG, "Performing factory reset to default configuration");

    system_config_t default_config;
    config_init_defaults(&default_config);

    // Save defaults to NVS
    esp_err_t ret = config_save(&default_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Factory reset completed successfully");
    } else {
        ESP_LOGE(TAG, "Factory reset failed: %s", esp_err_to_name(ret));
    }

    return ret;
}

bool config_is_valid_int_range(const char* param_name, int32_t value, int32_t min_val, int32_t max_val)
{
    if (value < min_val || value > max_val) {
        ESP_LOGE(TAG, "Parameter %s value %ld is out of range [%ld, %ld]", 
                 param_name, (long)value, (long)min_val, (long)max_val);
        return false;
    }
    return true;
}

esp_err_t config_get_current(system_config_t* config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Take mutex for thread safety
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire configuration mutex");
        return ESP_ERR_TIMEOUT;
    }

    *config = current_config;

    xSemaphoreGive(config_mutex);
    return ESP_OK;
}

esp_err_t config_set_current(const system_config_t* config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Validate configuration
    esp_err_t ret = config_validate_range(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuration validation failed");
        return ret;
    }

    // Take mutex for thread safety
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire configuration mutex");
        return ESP_ERR_TIMEOUT;
    }

    current_config = *config;

    xSemaphoreGive(config_mutex);
    ESP_LOGD(TAG, "Current configuration updated");
    return ESP_OK;
}

// =============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// =============================================================================

static void config_init_defaults(system_config_t* config)
{
    memset(config, 0, sizeof(system_config_t));

    // Configuration metadata
    config->config_version = CONFIG_VERSION;
    config->save_count = 0;

    // Template: Distance sensor defaults (example values - customize for your hardware)
    config->distance_min_mm = 100;      // 10 cm minimum
    config->distance_max_mm = 4000;     // 4 m maximum  
    config->measurement_interval_ms = 100;
    config->sensor_timeout_ms = 30000;
    config->temperature_c_x10 = 200;    // 20.0°C
    config->smoothing_factor = 300;     // 0.3 alpha

    // Template: LED defaults (example values - customize for your hardware)
    config->led_count = 30;
    config->led_brightness = 128;

    // WiFi defaults (empty SSID/password initially for AP mode)
    memset(config->wifi_ssid, 0, CONFIG_WIFI_SSID_MAX_LEN);
    memset(config->wifi_password, 0, CONFIG_WIFI_PASSWORD_MAX_LEN);
    config->wifi_ap_channel = 1;
    config->wifi_ap_max_conn = 4;
    config->wifi_sta_max_retry = 5;
    config->wifi_sta_timeout_ms = 10000;

    ESP_LOGD(TAG, "Initialized configuration with template defaults");
}

static esp_err_t config_validate_relationships(const system_config_t* config)
{
    // Validate that distance_max_mm > distance_min_mm
    if (config->distance_max_mm <= config->distance_min_mm) {
        ESP_LOGE(TAG, "distance_max_mm (%u) must be greater than distance_min_mm (%u)",
                 config->distance_max_mm, config->distance_min_mm);
        return ESP_ERR_INVALID_SIZE;
    }

    // Validate that sensor_timeout_ms < measurement_interval_ms
    if (config->sensor_timeout_ms >= config->measurement_interval_ms) {
        ESP_LOGE(TAG, "sensor_timeout_ms (%lu) must be less than measurement_interval_ms (%u)",
                 config->sensor_timeout_ms, config->measurement_interval_ms);
        return ESP_ERR_INVALID_SIZE;
    }

    return ESP_OK;
}

esp_err_t config_nvs_health_check(size_t* free_entries, size_t* total_entries)
{
    ESP_LOGD(TAG, "Performing NVS health check");

    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS namespace for health check: %s", esp_err_to_name(ret));
        return ret;
    }

    // Get NVS statistics
    nvs_stats_t nvs_stats;
    ret = nvs_get_stats(NULL, &nvs_stats);
    if (ret == ESP_OK) {
        if (free_entries) *free_entries = nvs_stats.free_entries;
        if (total_entries) *total_entries = nvs_stats.total_entries;
        
        ESP_LOGI(TAG, "NVS Health: %zu/%zu entries used, %zu KB available space",
                 nvs_stats.used_entries, nvs_stats.total_entries,
                 (nvs_stats.free_entries * 32) / 1024); // Rough estimate
        
        // Check for low space condition
        if (nvs_stats.free_entries < 10) {
            ESP_LOGW(TAG, "NVS space is running low (%zu free entries)", nvs_stats.free_entries);
        }
    } else {
        ESP_LOGW(TAG, "Failed to get NVS statistics: %s", esp_err_to_name(ret));
    }

    // Try to read our configuration to verify integrity
    size_t required_size = sizeof(system_config_t);
    system_config_t test_config;
    esp_err_t read_ret = nvs_get_blob(nvs_handle, NVS_CONFIG_KEY, &test_config, &required_size);
    
    nvs_close(nvs_handle);

    if (read_ret == ESP_OK) {
        // Verify the configuration is valid
        esp_err_t validate_ret = config_validate_range(&test_config);
        if (validate_ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS configuration is corrupted (validation failed)");
            return ESP_ERR_INVALID_STATE;
        }
        ESP_LOGD(TAG, "NVS configuration integrity verified");
    } else if (read_ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "No configuration found in NVS (first boot)");
    } else {
        ESP_LOGE(TAG, "Failed to read configuration for health check: %s", esp_err_to_name(read_ret));
        return read_ret;
    }

    ESP_LOGI(TAG, "NVS health check completed successfully");
    return ESP_OK;
}