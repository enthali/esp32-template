idf_component_register(SRCS "main.c" 
                           "display_logic.c"
                           "test/led_running_test.c"
                           "test/led_color_test.c"
                           "test/test_task.c"
                    INCLUDE_DIRS "." "test")