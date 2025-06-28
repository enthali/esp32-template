idf_component_register(SRCS "main.c" 
                           "test/led_running_test.c"
                           "test/led_color_test.c"
                           "test/test_task.c"
                           "wifi_manager.c"
                           "web_server.c"
                    INCLUDE_DIRS "." "test"
                    REQUIRES esp_wifi esp_http_server esp_netif nvs_flash)