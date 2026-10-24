idf_component_register(
    SRCS "main.c" "display.c" "move_square_game.c" "buttons.c" "audio.c" "move_scream_game.c" "cpn_game.c"
    PRIV_REQUIRES esp_lcd esp_driver_spi esp_driver_i2s esp_driver_gpio esp_wifi esp_netif esp_event nvs_flash esp_http_server esp_driver_gpio
)
