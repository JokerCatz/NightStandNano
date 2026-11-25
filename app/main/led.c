// SPDX-License-Identifier: LGPL-2.1-or-later
#include "led.h"
#include <string.h>
#include "crashlog.h"
#include "radio.h"
#include <math.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "player.h"
#include "settings.h"
#include "uac_host.h"
#include "bt_audio.h"
#include "storage.h"
#include "cardfmt.h"
#include "card_svc.h"
#include "hw.h"
#include "board/board.h"

static const char *TAG = "led";
static led_strip_handle_t s_strip;
static volatile int s_level = 12;
static volatile int s_cap = -1;           /* night cap: the effective peak never exceeds it (-1 = none) */
static volatile int64_t s_test_until_us;
static volatile int s_tr, s_tg, s_tb;

void nsn_led_set_level(int l) { s_level = l < 0 ? 0 : l > 255 ? 255 : l; g_set.led = s_level; nsn_settings_save(); }
void nsn_led_set_cap(int cap) { s_cap = cap; }
int nsn_led_level(void) { return s_level; }
void nsn_led_test(int r, int g, int b) { s_tr = r; s_tg = g; s_tb = b; s_test_until_us = esp_timer_get_time() + 3000000; }

static void put(int r, int g, int b)
{
    led_strip_set_pixel(s_strip, 0, r, g, b);
    led_strip_refresh(s_strip);
}

/* What the LED says (always on, so the box can be read without the page; all scaled by the level, level 0 = off, night mode caps it):
 *   colour = what is going on
 *     green   playing to the USB DAC            cyan    playing to Bluetooth headphones
 *     violet  a phone is streaming in           amber   paused
 *     blue    idle and ready (slow breath); quick blue blinks = searching for / pairing Bluetooth devices
 *   motion = how
 *     steady  playing                           quick pulse  radio connecting / buffering
 *     slow breath while playing = sleep timer running; it fades out with the sound in the last 30 s
 *   red blinks every 4 s = something is missing (count the blinks)
 *     1  no output (no USB DAC and no Bluetooth device)      2  no TF card
 *     3  radio cannot connect                                4  safe mode after repeated crashes
 *   While something plays, a fault shows as one short red blip every 4 s instead of taking the LED over.
 *   Card jobs take the LED over (checked before everything above):
 *     red / white alternation  a card format is waiting for its confirmation code (cardfmt): 6 Hz while the first code is pending, 12 Hz for the second;
 *                              unmistakable on purpose, and shown even at LED level 0 / night mode (it only lasts 30 s per step)
 *     white slow pulse         a card speed test is running (cardtest) */
static int fault_code(const nsn_player_info_t *p, bool have_out)
{
    if (nsn_crash_safe_mode()) return 4;
    const char *rs = nsn_radio_state();
    if (p->active && rs[0] && strcmp(rs, "connecting") && strcmp(rs, "buffering") && strcmp(rs, "reconnecting")) return 3;
    if (!nsn_storage_mounted()) return 2;
    if (!have_out) return 1;
    return 0;
}

static void led_task(void *arg)
{
    int64_t t0 = esp_timer_get_time();
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(40));
        int64_t now = esp_timer_get_time();
        int eff = s_cap >= 0 && s_cap < s_level ? s_cap : s_level;
        float lv = eff / 255.0f;
        if (now < s_test_until_us) { put(s_tr, s_tg, s_tb); continue; }
        int warn = nsn_cardfmt_led();
        bool ctest = nsn_cardtest_running();
        static int last_pat = -1;
        int pat = warn ? warn : ctest ? 3 : 0;
        if (pat != last_pat) { last_pat = pat; if (pat) ESP_LOGI(TAG, "card pattern %s", pat == 1 ? "format warning (code 1)" : pat == 2 ? "format urgent (code 2)" : "test pulse"); else ESP_LOGI(TAG, "card pattern off"); }
        if (warn) {                                                          /* format waiting for its code: red / white, never dim to nothing */
            float w = lv < 0.12f ? 0.12f : lv;
            bool red = ((now / (warn == 2 ? 40000 : 80000)) & 1) == 0;
            put((int)(w * 255), red ? 0 : (int)(w * 255), red ? 0 : (int)(w * 255));
            continue;
        }
        if (eff == 0) { put(0, 0, 0); continue; }
        if (ctest) {                                        /* card speed test: white pulse */
            float k = 0.25f + 0.75f * (0.5f + 0.5f * sinf((now - t0) / 1e6f * 2.0f * 3.14159265f * 1.5f));
            put((int)(k * lv * 255), (int)(k * lv * 255), (int)(k * lv * 255));
            continue;
        }
        nsn_player_info_t p; nsn_player_info(&p);
        nsn_uac_info_t u; nsn_uac_info(&u);
        float t = (now - t0) / 1e6f;
        float r = 0, g = 0, b = 0;
        bool playing = p.active && !p.paused;
        int fault = fault_code(&p, u.present || nsn_bt_connected());
        float ph = fmodf(t, 4.0f);
        if (fault && !playing) {                                             /* count the red blinks */
            for (int i = 0; i < fault; i++) if (ph >= i * 0.4f && ph < i * 0.4f + 0.15f) r = 1.0f;
        } else if (fault && ph < 0.12f) {
            r = 1.0f;                                                        /* playing with a fault: one red blip */
        } else if (playing) {
            const char *rs = nsn_radio_state();
            float k = 1.0f;
            if (rs[0]) k = 0.35f + 0.65f * (0.5f + 0.5f * sinf(t * 2.0f * 3.14159265f * 2.0f));          /* radio connecting / buffering: quick pulse */
            else if (p.sleep_left_s > 0 && p.sleep_left_s < 30) k = p.sleep_left_s / 30.0f;             /* fading out with the sound */
            else if (p.sleep_left_s > 0) k = 0.45f + 0.55f * (0.5f + 0.5f * sinf(t * 2.0f * 3.14159265f / 6.0f));   /* sleep timer running */
            if (p.fmt && !strcmp(p.fmt, "Phone")) { r = 0.55f * k; g = 0.2f * k; b = 1.0f * k; }         /* violet */
            else if (nsn_player_sink() == 2) { g = 0.75f * k; b = 1.0f * k; }                            /* cyan */
            else g = k;                                                                                  /* green */
        } else if (p.active) {
            r = 1.0f; g = 0.45f;                                             /* paused: amber */
        } else if (nsn_bt_scanning()) {
            b = fmodf(t, 0.5f) < 0.2f ? 1.0f : 0.0f;                         /* looking for Bluetooth devices */
        } else {
            b = 0.5f + 0.5f * sinf(t * 2.0f * 3.14159265f / 4.0f); b = b * b * 0.5f;   /* idle: slow blue breath */
        }
        put((int)(r * lv * 255), (int)(g * lv * 255), (int)(b * lv * 255));
    }
}

/* Position of each colour in the bytes the LED expects (its order string "grb" = first byte green, then red, then blue) */
static led_color_component_format_t wire_format(const char *order)
{
    led_color_component_format_t f = LED_STRIP_COLOR_COMPONENT_FMT_RGB;
    f.format.r_pos = (uint32_t)(strchr(order, 'r') - order);
    f.format.g_pos = (uint32_t)(strchr(order, 'g') - order);
    f.format.b_pos = (uint32_t)(strchr(order, 'b') - order);
    return f;
}

esp_err_t nsn_led_start(void)
{
    s_level = g_set.led;
    const nsn_hw_led_t *hl = nsn_hw_led();            /* GPIO and colour order: the stored hardware setting of this boot (hw.c), validated there */
    if (hl->gpio < 0) { ESP_LOGI(TAG, "led disabled (hw led off): no task, pin untouched"); return ESP_OK; }
    led_strip_config_t sc = { .strip_gpio_num = hl->gpio, .max_leds = 1, .led_model = board_led_model(), .color_component_format = wire_format(hl->order) };
    led_strip_rmt_config_t rc = { .clk_src = RMT_CLK_SRC_DEFAULT, .resolution_hz = 10 * 1000 * 1000 };
    esp_err_t e = led_strip_new_rmt_device(&sc, &rc, &s_strip);
    if (e != ESP_OK) { ESP_LOGE(TAG, "led init failed: %s", esp_err_to_name(e)); return e; }
    led_strip_clear(s_strip);
    xTaskCreatePinnedToCoreWithCaps(led_task, "led", 3072, NULL, 2, NULL, 0, MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "led on gpio %d, colour order %s", hl->gpio, hl->order);
    return ESP_OK;
}
