idf_component_register(SRCS "main.c" 
                           "display_logic.c"
                           "test/led_running_test.c"
                           "test/led_color_test.c"
                           "test/test_task.c"
                           "wifi_manager.c"
                           "web_server.c"
                    INCLUDE_DIRS "." "test"
                    REQUIRES led_controller distance_sensor esp_wifi esp_http_server esp_netif nvs_flash json)