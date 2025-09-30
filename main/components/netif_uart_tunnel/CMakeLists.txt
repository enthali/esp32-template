idf_component_register(
    SRCS "netif_uart_tunnel_sim.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_netif lwip
)
