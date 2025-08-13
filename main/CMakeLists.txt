idf_component_register(SRCS "main.c" 
                           "display_logic.c"
                           "config_manager.c"
                           "wifi_manager.c"
                    INCLUDE_DIRS "."
                    REQUIRES led_controller distance_sensor cert_handler startup_tests web_server esp_wifi esp_http_server esp_https_server esp_netif nvs_flash json)