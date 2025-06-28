/**
 * @file web_server.h
 * @brief Basic HTTP server for WiFi captive portal and configuration
 *
 * Provides:
 * - Captive portal for WiFi configuration
 * - DNS server to redirect all queries to ESP32 IP
 * - Configuration endpoints for WiFi setup
 * - Simple HTML interface for network selection
 */

#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Web server configuration
 */
typedef struct {
    uint16_t port;              ///< HTTP server port (default: 80)
    uint16_t dns_port;          ///< DNS server port (default: 53)
    size_t max_open_sockets;    ///< Maximum concurrent connections (default: 7)
} web_server_config_t;

/**
 * @brief Default web server configuration
 */
#define WEB_SERVER_DEFAULT_CONFIG() { \
    .port = 80, \
    .dns_port = 53, \
    .max_open_sockets = 7 \
}

/**
 * @brief Initialize web server for captive portal
 * 
 * Sets up HTTP server and DNS server for captive portal detection.
 * 
 * @param config Server configuration (NULL for default)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_init(const web_server_config_t *config);

/**
 * @brief Start web server
 * 
 * Starts both HTTP and DNS servers.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_start(void);

/**
 * @brief Stop web server
 * 
 * Stops both HTTP and DNS servers.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_stop(void);

/**
 * @brief Check if web server is running
 * 
 * @return true if server is running, false otherwise
 */
bool web_server_is_running(void);

/**
 * @brief Get current server port
 * 
 * @return Server port number
 */
uint16_t web_server_get_port(void);

#ifdef __cplusplus
}
#endif