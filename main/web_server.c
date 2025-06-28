/**
 * @file web_server.c
 * @brief Basic HTTP server for WiFi captive portal and configuration
 */

#include "web_server.h"
#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "cJSON.h"

static const char *TAG = "web_server";

// Server handles
static httpd_handle_t server = NULL;
static int dns_socket = -1;
static TaskHandle_t dns_task_handle = NULL;

// Configuration
static web_server_config_t current_config;
static bool server_running = false;

// Forward declarations
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t config_handler(httpd_req_t *req);
static esp_err_t scan_handler(httpd_req_t *req);
static esp_err_t connect_handler(httpd_req_t *req);
static esp_err_t status_handler(httpd_req_t *req);
static esp_err_t reset_handler(httpd_req_t *req);
static void dns_server_task(void *pvParameters);
static esp_err_t start_dns_server(void);
static void stop_dns_server(void);

// HTML pages
static const char *html_config_page =
    "<!DOCTYPE html>"
    "<html><head>"
    "<title>ESP32 Distance Sensor - WiFi Setup</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:40px;background:#f0f0f0}"
    ".container{background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);max-width:500px;margin:0 auto}"
    "h1{color:#333;text-align:center;margin-bottom:30px}"
    ".form-group{margin-bottom:20px}"
    "label{display:block;margin-bottom:5px;font-weight:bold;color:#555}"
    "input,select{width:100%;padding:10px;border:1px solid #ddd;border-radius:5px;font-size:16px;box-sizing:border-box}"
    "button{background:#007bff;color:white;padding:12px 30px;border:none;border-radius:5px;cursor:pointer;font-size:16px;width:100%;margin-top:10px}"
    "button:hover{background:#0056b3}"
    ".status{padding:10px;margin:10px 0;border-radius:5px;text-align:center}"
    ".success{background:#d4edda;color:#155724;border:1px solid #c3e6cb}"
    ".error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}"
    ".info{background:#d1ecf1;color:#0c5460;border:1px solid #bee5eb}"
    "</style>"
    "</head><body>"
    "<div class='container'>"
    "<h1>ESP32 Distance Sensor</h1>"
    "<h2>WiFi Configuration</h2>"
    "<div id='status' class='status info'>Select your WiFi network and enter the password</div>"
    "<form id='wifiForm'>"
    "<div class='form-group'>"
    "<label for='ssid'>WiFi Network:</label>"
    "<select id='ssid' name='ssid' required>"
    "<option value=''>Scanning for networks...</option>"
    "</select>"
    "<button type='button' onclick='scanNetworks()'>Refresh Networks</button>"
    "</div>"
    "<div class='form-group'>"
    "<label for='password'>Password:</label>"
    "<input type='password' id='password' name='password' placeholder='WiFi password'>"
    "</div>"
    "<button type='submit'>Connect to WiFi</button>"
    "</form>"
    "<div style='margin-top:30px;padding-top:20px;border-top:1px solid #ddd'>"
    "<h3>Device Management</h3>"
    "<button type='button' onclick='resetDevice()' style='background:#dc3545'>Reset WiFi & Restart</button>"
    "</div>"
    "<script>"
    "function scanNetworks(){"
    "document.getElementById('ssid').innerHTML='<option value=\"\">Scanning...</option>';"
    "fetch('/scan').then(r=>r.json()).then(data=>{"
    "let select=document.getElementById('ssid');"
    "select.innerHTML='';"
    "if(data.networks && data.networks.length>0){"
    "data.networks.forEach(n=>{"
    "let option=document.createElement('option');"
    "option.value=n.ssid;"
    "option.textContent=n.ssid+' ('+n.rssi+' dBm)';"
    "select.appendChild(option);"
    "});"
    "}else{"
    "select.innerHTML='<option value=\"\">No networks found</option>';"
    "}"
    "}).catch(e=>{"
    "document.getElementById('ssid').innerHTML='<option value=\"\">Scan failed</option>';"
    "console.error('Scan error:',e);"
    "});}"
    "function showStatus(msg,type){"
    "let status=document.getElementById('status');"
    "status.textContent=msg;"
    "status.className='status '+type;"
    "}"
    "document.getElementById('wifiForm').onsubmit=function(e){"
    "e.preventDefault();"
    "let ssid=document.getElementById('ssid').value;"
    "let password=document.getElementById('password').value;"
    "if(!ssid){showStatus('Please select a network','error');return;}"
    "showStatus('Connecting to '+ssid+'...','info');"
    "fetch('/connect',{"
    "method:'POST',"
    "headers:{'Content-Type':'application/json'},"
    "body:JSON.stringify({ssid:ssid,password:password})"
    "}).then(r=>r.json()).then(data=>{"
    "if(data.success){"
    "showStatus('Connected successfully! The device will now connect to your WiFi network.','success');"
    "setTimeout(()=>window.location.reload(),3000);"
    "}else{"
    "showStatus('Connection failed: '+(data.error||'Unknown error'),'error');"
    "}"
    "}).catch(e=>{"
    "showStatus('Connection request failed','error');"
    "console.error('Connection error:',e);"
    "});"
    "};"
    "function resetDevice(){"
    "if(confirm('This will clear WiFi credentials and restart the device. Continue?')){"
    "fetch('/reset',{method:'POST'})"
    ".then(r=>r.json())"
    ".then(data=>{"
    "if(data.success){"
    "showStatus('Device will restart in AP mode...','info');"
    "setTimeout(()=>window.location.href='http://192.168.4.1',5000);"
    "}else{"
    "showStatus('Reset failed: '+(data.error||'Unknown error'),'error');"
    "}"
    "}).catch(e=>{"
    "showStatus('Reset request failed','error');"
    "console.error('Reset error:',e);"
    "});"
    "}"
    "}"
    "scanNetworks();"
    "</script>"
    "</div></body></html>";

// HTTP request handlers
static esp_err_t root_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serving captive portal page");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");

    return httpd_resp_send(req, html_config_page, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t config_handler(httpd_req_t *req)
{
    return root_handler(req); // Same page for config
}

static esp_err_t scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WiFi scan request");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    // Start WiFi scan
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };

    esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi scan failed: %s", esp_err_to_name(ret));

        // If we're in AP mode, we might need to switch to APSTA mode for scanning
        wifi_mode_t current_mode;
        if (esp_wifi_get_mode(&current_mode) == ESP_OK && current_mode == WIFI_MODE_AP)
        {
            ESP_LOGW(TAG, "Switching to APSTA mode for WiFi scanning");
            esp_wifi_set_mode(WIFI_MODE_APSTA);
            vTaskDelay(pdMS_TO_TICKS(100)); // Give time for mode switch
            ret = esp_wifi_scan_start(&scan_config, true);
        }

        if (ret != ESP_OK)
        {
            return httpd_resp_send(req, "{\"error\":\"Scan failed\"}", HTTPD_RESP_USE_STRLEN);
        }
    }

    // Get scan results
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    if (ap_count == 0)
    {
        return httpd_resp_send(req, "{\"networks\":[]}", HTTPD_RESP_USE_STRLEN);
    }

    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (ap_records == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for scan results");
        return httpd_resp_send(req, "{\"error\":\"Memory allocation failed\"}", HTTPD_RESP_USE_STRLEN);
    }

    esp_wifi_scan_get_ap_records(&ap_count, ap_records);

    // Build JSON response
    cJSON *root = cJSON_CreateObject();
    cJSON *networks = cJSON_CreateArray();

    for (int i = 0; i < ap_count; i++)
    {
        cJSON *network = cJSON_CreateObject();
        cJSON_AddStringToObject(network, "ssid", (char *)ap_records[i].ssid);
        cJSON_AddNumberToObject(network, "rssi", ap_records[i].rssi);
        cJSON_AddNumberToObject(network, "authmode", ap_records[i].authmode);
        cJSON_AddItemToArray(networks, network);
    }

    cJSON_AddItemToObject(root, "networks", networks);

    char *json_string = cJSON_Print(root);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    free(json_string);
    cJSON_Delete(root);
    free(ap_records);

    return ESP_OK;
}

static esp_err_t connect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WiFi connect request");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    // Read POST data
    char *buf = malloc(req->content_len + 1);
    if (buf == NULL)
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Memory allocation failed\"}", HTTPD_RESP_USE_STRLEN);
    }

    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0)
    {
        free(buf);
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Failed to read request\"}", HTTPD_RESP_USE_STRLEN);
    }
    buf[ret] = '\0';

    // Parse JSON
    cJSON *root = cJSON_Parse(buf);
    free(buf);

    if (root == NULL)
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Invalid JSON\"}", HTTPD_RESP_USE_STRLEN);
    }

    cJSON *ssid_json = cJSON_GetObjectItem(root, "ssid");
    cJSON *password_json = cJSON_GetObjectItem(root, "password");

    if (!cJSON_IsString(ssid_json))
    {
        cJSON_Delete(root);
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"SSID required\"}", HTTPD_RESP_USE_STRLEN);
    }

    wifi_credentials_t credentials = {0};
    strncpy(credentials.ssid, ssid_json->valuestring, sizeof(credentials.ssid) - 1);

    if (cJSON_IsString(password_json))
    {
        strncpy(credentials.password, password_json->valuestring, sizeof(credentials.password) - 1);
    }

    cJSON_Delete(root);

    ESP_LOGI(TAG, "Attempting to connect to SSID: %s", credentials.ssid);
    ESP_LOGI(TAG, "Password: '%s' (length: %d)", credentials.password, strlen(credentials.password));

    // Attempt to set credentials and connect
    esp_err_t wifi_ret = wifi_manager_set_credentials(&credentials);
    if (wifi_ret == ESP_OK)
    {
        return httpd_resp_send(req, "{\"success\":true}", HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Connection failed\"}", HTTPD_RESP_USE_STRLEN);
    }
}

static esp_err_t status_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    wifi_status_t status;
    if (wifi_manager_get_status(&status) != ESP_OK)
    {
        return httpd_resp_send(req, "{\"error\":\"Failed to get status\"}", HTTPD_RESP_USE_STRLEN);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mode", status.mode);
    cJSON_AddStringToObject(root, "ssid", status.connected_ssid);
    cJSON_AddNumberToObject(root, "rssi", status.rssi);
    cJSON_AddBoolToObject(root, "has_credentials", status.has_credentials);

    char ip_str[16];
    if (wifi_manager_get_ip_address(ip_str, sizeof(ip_str)) == ESP_OK)
    {
        cJSON_AddStringToObject(root, "ip", ip_str);
    }

    char *json_string = cJSON_Print(root);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    free(json_string);
    cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t reset_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WiFi reset request");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    // Clear WiFi credentials and restart in AP mode
    esp_err_t ret = wifi_manager_clear_credentials();
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi credentials cleared, device will restart in AP mode");

        // Send success response
        httpd_resp_send(req, "{\"success\":true,\"message\":\"Device will restart in AP mode\"}", HTTPD_RESP_USE_STRLEN);

        // Give time for response to be sent, then restart
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();

        return ESP_OK;
    }
    else
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Failed to clear credentials\"}", HTTPD_RESP_USE_STRLEN);
    }
}

// DNS server for captive portal detection
static void dns_server_task(void *pvParameters)
{
    ESP_LOGI(TAG, "DNS server task started");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(current_config.dns_port);

    dns_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (dns_socket < 0)
    {
        ESP_LOGE(TAG, "Failed to create DNS socket");
        vTaskDelete(NULL);
        return;
    }

    if (bind(dns_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        ESP_LOGE(TAG, "Failed to bind DNS socket");
        close(dns_socket);
        dns_socket = -1;
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "DNS server listening on port %d", current_config.dns_port);

    uint8_t rx_buffer[512];

    while (dns_socket >= 0)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int len = recvfrom(dns_socket, rx_buffer, sizeof(rx_buffer), 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);

        if (len > 0 && len >= 12)
        { // Minimum DNS header size
            // Simple DNS response: redirect all queries to 192.168.4.1
            uint8_t response[512];
            memcpy(response, rx_buffer, len);

            // Set response flags
            response[2] = 0x81; // Standard query response, no error
            response[3] = 0x80; // Recursion available

            // Add answer section pointing to 192.168.4.1
            int response_len = len;
            if (response_len + 16 < sizeof(response))
            {
                response[response_len++] = 0xc0; // Pointer to query name
                response[response_len++] = 0x0c;
                response[response_len++] = 0x00; // Type A
                response[response_len++] = 0x01;
                response[response_len++] = 0x00; // Class IN
                response[response_len++] = 0x01;
                response[response_len++] = 0x00; // TTL (4 bytes)
                response[response_len++] = 0x00;
                response[response_len++] = 0x00;
                response[response_len++] = 0x3c;
                response[response_len++] = 0x00; // Data length
                response[response_len++] = 0x04;
                response[response_len++] = 192; // IP: 192.168.4.1
                response[response_len++] = 168;
                response[response_len++] = 4;
                response[response_len++] = 1;

                // Update answer count
                response[6] = 0x00;
                response[7] = 0x01;
            }

            sendto(dns_socket, response, response_len, 0,
                   (struct sockaddr *)&client_addr, client_addr_len);
        }
    }

    ESP_LOGI(TAG, "DNS server task ended");
    vTaskDelete(NULL);
}

static esp_err_t start_dns_server(void)
{
    if (dns_task_handle != NULL)
    {
        ESP_LOGW(TAG, "DNS server already running");
        return ESP_OK;
    }

    BaseType_t result = xTaskCreate(dns_server_task, "dns_server", 4096, NULL, 5, &dns_task_handle);
    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create DNS server task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void stop_dns_server(void)
{
    if (dns_socket >= 0)
    {
        close(dns_socket);
        dns_socket = -1;
    }

    if (dns_task_handle != NULL)
    {
        vTaskDelete(dns_task_handle);
        dns_task_handle = NULL;
    }

    ESP_LOGI(TAG, "DNS server stopped");
}

// Public functions
esp_err_t web_server_init(const web_server_config_t *config)
{
    if (server != NULL)
    {
        ESP_LOGW(TAG, "Web server already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (config != NULL)
    {
        current_config = *config;
    }
    else
    {
        web_server_config_t default_config = WEB_SERVER_DEFAULT_CONFIG();
        current_config = default_config;
    }

    ESP_LOGI(TAG, "Initializing web server on port %d", current_config.port);

    httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
    httpd_config.server_port = current_config.port;
    httpd_config.max_open_sockets = current_config.max_open_sockets;
    httpd_config.lru_purge_enable = true;

    if (httpd_start(&server, &httpd_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }

    // Register URI handlers
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &root_uri);

    httpd_uri_t config_uri = {
        .uri = "/config",
        .method = HTTP_GET,
        .handler = config_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &config_uri);

    httpd_uri_t scan_uri = {
        .uri = "/scan",
        .method = HTTP_GET,
        .handler = scan_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &scan_uri);

    httpd_uri_t connect_uri = {
        .uri = "/connect",
        .method = HTTP_POST,
        .handler = connect_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &connect_uri);

    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &status_uri);

    httpd_uri_t reset_uri = {
        .uri = "/reset",
        .method = HTTP_POST,
        .handler = reset_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &reset_uri);

    ESP_LOGI(TAG, "Web server initialized successfully");
    return ESP_OK;
}

esp_err_t web_server_start(void)
{
    if (server == NULL)
    {
        ESP_LOGE(TAG, "Web server not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (server_running)
    {
        ESP_LOGW(TAG, "Web server already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting web server");

    // Only start DNS server for captive portal if we're in AP mode
    wifi_mode_t wifi_mode;
    if (esp_wifi_get_mode(&wifi_mode) == ESP_OK &&
        (wifi_mode == WIFI_MODE_AP || wifi_mode == WIFI_MODE_APSTA))
    {
        ESP_LOGI(TAG, "Starting DNS server for captive portal (AP mode)");
        esp_err_t ret = start_dns_server();
        if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "Failed to start DNS server, captive portal may not work properly");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Skipping DNS server (STA mode - not needed)");
    }

    server_running = true;
    ESP_LOGI(TAG, "Web server started successfully");

    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (!server_running)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping web server");

    stop_dns_server();

    if (server != NULL)
    {
        httpd_stop(server);
        server = NULL;
    }

    server_running = false;
    ESP_LOGI(TAG, "Web server stopped");

    return ESP_OK;
}

bool web_server_is_running(void)
{
    return server_running;
}

uint16_t web_server_get_port(void)
{
    return current_config.port;
}