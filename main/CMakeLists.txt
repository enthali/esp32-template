idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash
        # Optional example components (uncomment as needed):
        # config_manager
        # web_server
        # cert_handler
        # netif_uart_tunnel
)