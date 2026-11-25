# The I2S output template is compiled only when asked for (Kconfig NSN_SINK_I2S). The drivers are listed as dependencies either way: REQUIRES cannot depend on
# CONFIG_ values (the requirement scan runs before the configuration is known), and a component nothing refers to adds nothing to the image.
set(NSN_EXTRA_SRCS "")
if(CONFIG_NSN_SINK_I2S)
    list(APPEND NSN_EXTRA_SRCS "board/sink_i2s.c")
endif()
idf_component_register(SRCS "main.c" "storage.c" "exfat_selftest.c" "uac_host.c" "player.c" "bt_audio.c" "bt_ctl.c" "cmd.c" "led.c" "settings.c" "clock.c" "backend_gme.cpp" "radio.c" "wifi.c" "spectrum.c" "backend_sdec.c" "scene.c" "fx.c" "crashlog.c" "night.c" "cast.c" "keys.c" "events.c" "card_test.c" "card_svc.c" "cardfmt.c" "outcheck.c" "zipfs.c" "ratetest.c" "trim.c" "sink.c" "hw.c" "board/board_default.c" ${NSN_EXTRA_SRCS} INCLUDE_DIRS "." "board" REQUIRES app_update driver esp_driver_gpio esp_driver_uart fatfs sdmmc esp_timer nvs_flash vfs esp_ringbuf espressif__esp_audio_codec esp_http_client esp-tls mbedtls esp_driver_rmt esp_driver_spi esp_driver_sdspi esp_wifi esp_netif esp_event bt player_core gme libldac esp_driver_i2s esp_driver_i2c)
