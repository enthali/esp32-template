/**
 * @file dns_server.c
 * @brief Simple DNS server implementation for WiFi captive portal detection
 * 
 * This module provides a lightweight DNS server that redirects all DNS queries
 * to the ESP32's access point IP address. This is essential for captive portal
 * functionality, as mobile devices detect internet connectivity by making DNS
 * queries to known domains.
 * 
 * When devices connect to the ESP32's WiFi access point, they automatically
 * attempt to reach the internet. This DNS server ensures all their requests
 * are redirected to the ESP32's web interface, triggering the captive portal
 * workflow on most devices.
 */

#include "dns_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>

static const char *TAG = "dns_server";

// DNS server state
static int dns_socket = -1;
static TaskHandle_t dns_task_handle = NULL;
static dns_server_config_t current_config;

/**
 * @brief DNS server background task
 * 
 * Listens for UDP packets on port 53 and responds to all DNS queries
 * with the configured AP IP address. The response format follows the
 * basic DNS protocol structure.
 * 
 * @param pvParameters Task parameters (unused)
 */
static void dns_server_task(void *pvParameters)
{
    ESP_LOGI(TAG, "DNS server task started on port %d", current_config.port);

    // Create UDP socket for DNS server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(current_config.port);

    dns_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (dns_socket < 0) {
        ESP_LOGE(TAG, "Failed to create DNS socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    // Bind socket to DNS port
    if (bind(dns_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind DNS socket: errno %d", errno);
        close(dns_socket);
        dns_socket = -1;
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "DNS server listening on port %d, redirecting to %d.%d.%d.%d", 
             current_config.port,
             (current_config.ap_ip >> 24) & 0xFF,
             (current_config.ap_ip >> 16) & 0xFF,
             (current_config.ap_ip >> 8) & 0xFF,
             current_config.ap_ip & 0xFF);

    uint8_t rx_buffer[512];
    
    // Main DNS server loop
    while (dns_socket >= 0) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Receive DNS query
        int len = recvfrom(dns_socket, rx_buffer, sizeof(rx_buffer), 0,
                          (struct sockaddr *)&client_addr, &client_addr_len);

        if (len > 0 && len >= 12) {  // Minimum DNS header size
            ESP_LOGD(TAG, "Received DNS query (%d bytes) from %s", 
                     len, inet_ntoa(client_addr.sin_addr));

            // Prepare DNS response by copying the query
            uint8_t response[512];
            memcpy(response, rx_buffer, len);

            // Set DNS response flags
            response[2] = 0x81;  // Standard query response, no error
            response[3] = 0x80;  // Recursion available

            // Add answer section pointing to our AP IP
            int response_len = len;
            if (response_len + 16 < sizeof(response)) {
                // Answer RR: compressed name pointer
                response[response_len++] = 0xc0;  // Pointer to query name
                response[response_len++] = 0x0c;
                
                // Answer RR: type A (IPv4 address)
                response[response_len++] = 0x00;  // Type A
                response[response_len++] = 0x01;
                
                // Answer RR: class IN (Internet)
                response[response_len++] = 0x00;  // Class IN
                response[response_len++] = 0x01;
                
                // Answer RR: TTL (4 bytes) - 60 seconds
                response[response_len++] = 0x00;
                response[response_len++] = 0x00;
                response[response_len++] = 0x00;
                response[response_len++] = 0x3c;
                
                // Answer RR: data length (4 bytes for IPv4)
                response[response_len++] = 0x00;
                response[response_len++] = 0x04;
                
                // Answer RR: IP address (4 bytes)
                response[response_len++] = (current_config.ap_ip >> 24) & 0xFF;  // 192
                response[response_len++] = (current_config.ap_ip >> 16) & 0xFF;  // 168
                response[response_len++] = (current_config.ap_ip >> 8) & 0xFF;   // 4
                response[response_len++] = current_config.ap_ip & 0xFF;          // 1

                // Update answer count in DNS header
                response[6] = 0x00;  // Answer count high byte
                response[7] = 0x01;  // Answer count low byte (1 answer)
            }

            // Send DNS response back to client
            int sent = sendto(dns_socket, response, response_len, 0,
                             (struct sockaddr *)&client_addr, client_addr_len);
            
            if (sent < 0) {
                ESP_LOGW(TAG, "Failed to send DNS response: errno %d", errno);
            } else {
                ESP_LOGD(TAG, "Sent DNS response (%d bytes) to %s", 
                         sent, inet_ntoa(client_addr.sin_addr));
            }
        } else if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            ESP_LOGW(TAG, "DNS receive error: errno %d", errno);
        }
    }

    ESP_LOGI(TAG, "DNS server task ended");
    vTaskDelete(NULL);
}

esp_err_t dns_server_start(const dns_server_config_t *config)
{
    // Check if already running
    if (dns_task_handle != NULL) {
        ESP_LOGW(TAG, "DNS server already running");
        return ESP_OK;
    }

    // Use provided config or defaults
    if (config != NULL) {
        current_config = *config;
    } else {
        dns_server_config_t default_config = DNS_SERVER_DEFAULT_CONFIG();
        current_config = default_config;
    }

    // Create DNS server task
    BaseType_t result = xTaskCreate(
        dns_server_task,        // Task function
        "dns_server",           // Task name
        4096,                   // Stack size
        NULL,                   // Task parameters
        5,                      // Task priority
        &dns_task_handle        // Task handle
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create DNS server task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "DNS server started successfully");
    return ESP_OK;
}

esp_err_t dns_server_stop(void)
{
    ESP_LOGI(TAG, "Stopping DNS server");

    // Close socket to break the task loop
    if (dns_socket >= 0) {
        close(dns_socket);
        dns_socket = -1;
    }

    // Wait for task to clean up and delete itself
    if (dns_task_handle != NULL) {
        // Give the task time to exit gracefully
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // If task is still running, force delete it
        if (eTaskGetState(dns_task_handle) != eDeleted) {
            vTaskDelete(dns_task_handle);
        }
        
        dns_task_handle = NULL;
    }

    ESP_LOGI(TAG, "DNS server stopped");
    return ESP_OK;
}

bool dns_server_is_running(void)
{
    return (dns_task_handle != NULL && eTaskGetState(dns_task_handle) != eDeleted);
}
