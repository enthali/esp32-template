idf_component_register(
    SRCS "display_logic.c"
    INCLUDE_DIRS "."
    REQUIRES 
        led_controller 
        distance_sensor 
        freertos
    PRIV_REQUIRES 
        main
)
