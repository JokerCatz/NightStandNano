// SPDX-License-Identifier: LGPL-2.1-or-later
/* NightStand Nano: headless bedside audio box on an ESP32-S31 (TF card -> USB DAC / A2DP, Bluetooth control). Bring-up stage. */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/uart_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bt_audio.h"
#include "bt_ctl.h"
#include "cmd.h"
#include "spectrum.h"
#include "storage.h"
#include "uac_host.h"
#include "wifi.h"
#include "clock.h"
#include "nvs.h"
#include "esp_heap_caps.h"
#include "fx.h"
#include "crashlog.h"
#include "night.h"
#include "cast.h"
#include "keys.h"
#include "led.h"
#include "card_svc.h"
#include "settings.h"
#include "player.h"
#include "hw.h"
#include "board/board.h"
#include "esp_app_desc.h"

static const char *TAG = "nsn";
bool nsn_exfat_selftest(void);

static void uart_out(void *ctx, const char *t) { puts(t); }

/* Serial console (UART0), same commands as the BLE control characteristic */
static void console_task(void *arg)
{
    char line[400];
    uart_driver_install(UART_NUM_0, 512, 0, 0, NULL, 0);     /* blocking stdin: without the driver fgets() sees EOF at once */
    uart_vfs_dev_use_driver(UART_NUM_0);
    while (fgets(line, sizeof line, stdin)) nsn_cmd_exec_local(line, uart_out, NULL);
    vTaskDelete(NULL);
}

/* boot log: free internal RAM and its low-water mark after each stage (internal RAM is the scarce resource) */
static void memlog(const char *stage)
{
    ESP_LOGI(TAG, "mem after %-8s free %6u  min %6u  largest %6u", stage, (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

void app_main(void)
{
    ESP_LOGI(TAG, "NightStand Nano %s (IDF %s)", esp_app_get_description()->version, esp_app_get_description()->idf_ver);
    nsn_crash_boot();
    nsn_settings_load();
    nsn_hw_load();                                    /* TF card wiring and LED pin / order from NVS: before storage and led use them */
    nsn_clock_boot();
    nsn_wifi_boot();
    nsn_player_init_settings();
    nsn_player_set_mode(g_set.play_mode);
    nsn_player_set_rg(nsn_settings_get_i32("rg", 0));
    {   /* the bookmark: its own NVS keys */
        nvs_handle_t nh; char bp[160] = ""; size_t bl = sizeof bp; uint32_t pos = 0;
        if (nvs_open("nsn", NVS_READONLY, &nh) == ESP_OK) {
            if (nvs_get_str(nh, "bm_path", bp, &bl) != ESP_OK) bp[0] = 0;
            nvs_get_u32(nh, "bm_pos", &pos); nvs_close(nh);
        }
        nsn_player_set_bookmark(bp, pos);
    }
    nsn_player_set_output(g_set.out);
    xTaskCreate(console_task, "console", 6144, NULL, 3, NULL);
    memlog("start");
    nsn_exfat_selftest();
    memlog("exfat");
    ESP_LOGI(TAG, "storage: %s", nsn_storage_mount() == ESP_OK ? "mounted" : nsn_storage_why());
    memlog("storage");
    nsn_card_svc_start();
    nsn_player_follow_phone();
    nsn_keys_start();
    ESP_LOGI(TAG, "bt_init: %s", esp_err_to_name(nsn_bt_init()));
    memlog("bt");
    ESP_LOGI(TAG, "uac_start: %s", esp_err_to_name(nsn_uac_start()));
    memlog("uac");
    ESP_LOGI(TAG, "led: %s", esp_err_to_name(nsn_led_start()));
    nsn_crash_flush();
    if (!nsn_crash_safe_mode()) { nsn_fx_start(); nsn_night_start(); }
    nsn_wifi_start();
    ESP_LOGI(TAG, "ble: %s", esp_err_to_name(nsn_ble_start()));
    ESP_LOGI(TAG, "board display: %s", esp_err_to_name(board_display_init()));      /* extension points (board/board.h): empty on the stock board */
    ESP_LOGI(TAG, "board buttons: %s", esp_err_to_name(board_buttons_init()));
    if (g_set.out == NSN_OUT_CAST) {                 /* the broadcast was the chosen output: bring it back, after Wi-Fi had its start (round: Wi-Fi first) */
        vTaskDelay(pdMS_TO_TICKS(6000));
        if (nsn_cast_start() != ESP_OK) nsn_player_set_output(NSN_OUT_AUTO);
    }
    if (g_set.autoplay && g_set.last[0] && !nsn_crash_safe_mode()) { vTaskDelay(pdMS_TO_TICKS(4000)); nsn_player_play(g_set.last); }   /* after USB / Wi-Fi had a moment */
    for (;;) {                                   /* a card inserted later is picked up (0027: the box must not stay blind) */
        vTaskDelay(pdMS_TO_TICKS(5000));
        nsn_player_persist();                       /* volume changes made from the keys / headset callbacks */
        {   char bp[160]; uint32_t pos; nvs_handle_t nh;
            if (nsn_player_bookmark_dirty(bp, sizeof bp, &pos) && nvs_open("nsn", NVS_READWRITE, &nh) == ESP_OK) {
                nvs_set_str(nh, "bm_path", bp); nvs_set_u32(nh, "bm_pos", pos); nvs_commit(nh); nvs_close(nh);
            }
        }
        {   nsn_player_info_t pi; nsn_player_info(&pi);
            size_t l = strlen(pi.path); nsn_crash_crumb(pi.active ? pi.path + (l > 15 ? l - 15 : 0) : "idle"); }
        {   /* remember the A2DP device that connected; reconnect to it when Bluetooth is the wanted output and it is not connected */
            static int quiet;
            uint8_t peer[6];
            if (nsn_bt_connected()) {
                nsn_bt_peer(peer);
                if (!g_set.bt_have || memcmp(peer, g_set.bt_addr, 6)) { memcpy(g_set.bt_addr, peer, 6); g_set.bt_have = 1; nsn_settings_save(); ESP_LOGI(TAG, "remembered BT device"); }
                quiet = 0;
                char dp[160];
                if (nsn_player_take_dropped(dp, sizeof dp)) { ESP_LOGI(TAG, "Bluetooth is back, resuming %s", dp); nsn_player_play(dp); }
            } else if (g_set.bt_have && !nsn_bt_scanning()) {
                nsn_uac_info_t u; nsn_uac_info(&u);
                bool want = nsn_player_output() == NSN_OUT_BT || (nsn_player_output() == NSN_OUT_AUTO && !u.present);
                if (want && ++quiet >= 3) { quiet = 0; ESP_LOGI(TAG, "reconnecting to the last BT device"); nsn_bt_connect(g_set.bt_addr); }
            }
        }
        if (!nsn_storage_mounted() && nsn_storage_mount() == ESP_OK) { ESP_LOGI(TAG, "card mounted"); nsn_wifi_start(); }
    }
}
