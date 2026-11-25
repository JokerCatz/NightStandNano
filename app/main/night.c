// SPDX-License-Identifier: LGPL-2.1-or-later
#include "night.h"
#include "clock.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fx.h"
#include "led.h"
#include "player.h"
#include "settings.h"
#include "pc/pc_fxapp.h"

static const char *TAG = "night";
#define LED_CAP 2
static volatile bool s_on;
static int s_from = -1, s_to = -1;
static bool s_auto_state;      /* what the schedule last decided (so a manual override lasts until the next edge) */

static void nv_load(void)
{
    s_from = nsn_settings_get_i32("ngt_from", s_from);
    s_to = nsn_settings_get_i32("ngt_to", s_to);
}
static void nv_save(void)
{
    nsn_settings_set_i32("ngt_from", s_from);
    nsn_settings_set_i32("ngt_to", s_to);
}

static int app_of(const char *id) { return pc_fxapp_find(id); }
bool nsn_night_on(void) { return s_on; }
void nsn_night_set(bool on)
{
    s_on = on;
    static const char *const apps[] = { "loudness", "compressor", "limiter" };
    for (int i = 0; i < 3; i++) { int a = app_of(apps[i]); if (a >= 0) fx_set_on(a, on); }
    nsn_led_set_cap(on ? LED_CAP : -1);
    ESP_LOGI(TAG, "night mode %s", on ? "on" : "off");
}

void nsn_night_auto(int from_min, int to_min)
{
    s_from = from_min; s_to = to_min; nv_save();
    s_auto_state = false;
}

static bool in_window(int m)
{
    if (s_from < 0 || s_to < 0 || s_from == s_to) return false;
    return s_from < s_to ? (m >= s_from && m < s_to) : (m >= s_from || m < s_to);
}

static void night_task(void *arg)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(20000));
        time_t now = time(NULL);
        if (!nsn_clock_valid() || s_from < 0) continue;
        struct tm tm; localtime_r(&now, &tm);
        bool want = in_window(tm.tm_hour * 60 + tm.tm_min);
        if (want != s_auto_state) { s_auto_state = want; nsn_night_set(want); }   /* only on the edges: a manual `night off` lasts until the next one */
    }
}

void nsn_night_start(void)
{
    nv_load();
    xTaskCreatePinnedToCoreWithCaps(night_task, "night", 4096, NULL, 2, NULL, 0, MALLOC_CAP_SPIRAM);
}

void nsn_night_status(char *out, int n)
{
    if (s_from < 0) snprintf(out, n, "night: %s, schedule off", s_on ? "on" : "off");
    else snprintf(out, n, "night: %s, schedule %02d:%02d-%02d:%02d", s_on ? "on" : "off", s_from / 60, s_from % 60, s_to / 60, s_to % 60);
}

void nsn_bedtime(int minutes)
{
    if (minutes <= 0) minutes = 30;
    nsn_night_set(true);
    const char *src = (!strncmp(g_set.last, "noise:", 6) || !strncmp(g_set.last, "scene:", 6)) ? g_set.last : "noise:brown";
    nsn_player_play(src);
    nsn_player_sleep(minutes);
}
