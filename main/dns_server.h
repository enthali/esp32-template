/**
 * @file dns_server.h
 * @brief Simple DNS server for WiFi captive portal detection
 * 
 * This module implements a basic DNS server that redirects all queries to the ESP32's
 * AP mode IP address (192.168.4.1). This enables captive portal detection on mobile
 * devices, automatically prompting users to configure WiFi when they connect to the
 * ESP32's access point.
 * 
 * The DNS server runs as a background FreeRTOS task and listens on UDP port 53.
 * All DNS queries are answered with the ESP32's AP IP address, regardless of the
 * requested domain name.
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DNS server configuration structure
 */
typedef struct {
    uint16_t port;          /**< DNS server port (default: 53) */
    uint32_t ap_ip;         /**< AP mode IP address to redirect to (default: 192.168.4.1) */
} dns_server_config_t;

/**
 * @brief Default DNS server configuration
 */
#define DNS_SERVER_DEFAULT_CONFIG() { \
    .port = 53, \
    .ap_ip = 0xC0A80401  /* 192.168.4.1 in network byte order */ \
}

/**
 * @brief Initialize and start the DNS server
 * 
 * Creates a background FreeRTOS task that listens for DNS queries on UDP port 53.
 * All queries are answered with the configured AP IP address to enable captive
 * portal detection.
 * 
 * @param config DNS server configuration, or NULL for default settings
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t dns_server_start(const dns_server_config_t *config);

/**
 * @brief Stop the DNS server
 * 
 * Closes the DNS socket and terminates the background task.
 * Safe to call multiple times or when the server is not running.
 * 
 * @return ESP_OK on success
 */
esp_err_t dns_server_stop(void);

/**
 * @brief Check if the DNS server is currently running
 * 
 * @return true if the DNS server task is active, false otherwise
 */
bool dns_server_is_running(void);

#ifdef __cplusplus
}
#endif
