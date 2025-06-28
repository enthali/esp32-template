idf_component_register(SRCS "main.c" 
                           "led_controller.c"
                           "test/led_running_test.c"
                           "test/led_color_test.c"
                    INCLUDE_DIRS "." "test")