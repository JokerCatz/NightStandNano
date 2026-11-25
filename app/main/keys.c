// SPDX-License-Identifier: LGPL-2.1-or-later
#include "keys.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "player.h"
#include "events.h"
#include "settings.h"

static const char *TAG = "keys";
enum { A_NONE, A_PLAY, A_NEXT, A_PREV, A_VOLUP, A_VOLDN, A_STOP, A_N };
static const char *const ANAME[A_N] = { "none", "play", "next", "prev", "vol+", "vol-", "stop" };
static const char BTN[NSN_KEY_N] = { 'c', 'p', 'm' };
static const char *const NVKEY[NSN_KEY_N] = { "key_c", "key_p", "key_m" };
enum { G1, G2, G3, GH, G_N };
#define CLICK_MS 400        /* how long a click waits for the next one */
#define HOLD_MS 600
#define REPEAT_MS 250

static uint8_t s_map[NSN_KEY_N][G_N];
static const uint8_t PRESET[3][NSN_KEY_N][G_N] = {
    /* one button: the usual single-key headset */
    { { A_PLAY, A_NEXT, A_PREV, A_NONE }, { 0 }, { 0 } },
    /* two buttons (+ and -): volume on a click or hold, track on a double click */
    { { 0 }, { A_VOLUP, A_NEXT, A_NONE, A_VOLUP }, { A_VOLDN, A_PREV, A_NONE, A_VOLDN } },
    /* three buttons: the usual +, centre, - */
    { { A_PLAY, A_NEXT, A_PREV, A_NONE }, { A_VOLUP, A_NONE, A_NONE, A_VOLUP }, { A_VOLDN, A_NONE, A_NONE, A_VOLDN } },
};
typedef struct { uint8_t key, down; int64_t us; } ev_t;
static QueueHandle_t s_q;
static struct { bool down, held; int clicks; int64_t t_down, t_up, t_rep; } s_k[NSN_KEY_N];
static char s_last[8]; static uint8_t s_last_act; static int64_t s_last_us;

static void save(int k) { nsn_settings_set_i32(NVKEY[k], s_map[k][0] | s_map[k][1] << 4 | s_map[k][2] << 8 | s_map[k][3] << 12); }
static void load(void)
{
    memcpy(s_map, PRESET[2], sizeof s_map);
    for (int k = 0; k < NSN_KEY_N; k++) {
        int v = nsn_settings_get_i32(NVKEY[k], -1);
        if (v < 0) continue;
        for (int g = 0; g < G_N; g++) { int a = (v >> (4 * g)) & 15; s_map[k][g] = a < A_N ? (uint8_t)a : A_NONE; }
    }
}

static void act(int k, int g)
{
    int a = s_map[k][g];
    snprintf(s_last, sizeof s_last, "%c%c", BTN[k], "123h"[g]); s_last_act = (uint8_t)a; s_last_us = esp_timer_get_time();
    ESP_LOGI(TAG, "button %c gesture %c -> %s", BTN[k], "123h"[g], ANAME[a]);
    nsn_event("wired %c%c %s", BTN[k], "123h"[g], ANAME[a]);
    nsn_player_info_t p;
    switch (a) {
    case A_PLAY:
        nsn_player_info(&p);
        if (!p.active) nsn_player_resume(); else nsn_player_pause(!p.paused);
        break;
    case A_NEXT: nsn_player_skip(1); break;
    case A_PREV: nsn_player_skip(-1); break;
    case A_VOLUP: { int v = nsn_player_volume() + 5; nsn_player_set_volume(v > 100 ? 100 : v); break; }
    case A_VOLDN: { int v = nsn_player_volume() - 5; nsn_player_set_volume(v < 0 ? 0 : v); break; }
    case A_STOP: nsn_player_stop(); break;
    default: break;
    }
}
static bool multi(int k) { return s_map[k][G2] != A_NONE || s_map[k][G3] != A_NONE; }
static bool is_vol(int a) { return a == A_VOLUP || a == A_VOLDN; }

static void keys_task(void *arg)
{
    ev_t e;
    for (;;) {
        bool got = xQueueReceive(s_q, &e, pdMS_TO_TICKS(20)) == pdTRUE;
        int64_t now = esp_timer_get_time();
        if (got && e.key < NSN_KEY_N) {
            int k = e.key;
            if (e.down && !s_k[k].down) {
                s_k[k].down = true; s_k[k].held = false; s_k[k].t_down = e.us;
                if (!multi(k)) act(k, G1);                       /* nothing to wait for: act on the press */
            } else if (!e.down && s_k[k].down) {
                s_k[k].down = false; s_k[k].t_up = e.us;
                if (!s_k[k].held && multi(k)) s_k[k].clicks++;
            }
        }
        for (int k = 0; k < NSN_KEY_N; k++) {
            if (s_k[k].down) {
                if (!s_k[k].held && now - s_k[k].t_down >= HOLD_MS * 1000LL) {
                    s_k[k].held = true; s_k[k].clicks = 0; s_k[k].t_rep = now;
                    act(k, GH);
                } else if (s_k[k].held && is_vol(s_map[k][GH]) && now - s_k[k].t_rep >= REPEAT_MS * 1000LL) {
                    s_k[k].t_rep = now; act(k, GH);
                }
            } else if (s_k[k].clicks && now - s_k[k].t_up >= CLICK_MS * 1000LL) {
                int n = s_k[k].clicks > 3 ? 3 : s_k[k].clicks;
                s_k[k].clicks = 0;
                act(k, n - 1);
            }
        }
    }
}

void nsn_keys_start(void)
{
    load();
    s_q = xQueueCreate(16, sizeof(ev_t));
    xTaskCreatePinnedToCore(keys_task, "keys", 4096, NULL, 4, NULL, 0);     /* internal stack: actions may save settings (flash) */
}
void nsn_keys_event(int key, bool down)
{
    if (!s_q) return;
    ev_t e = { (uint8_t)key, down, esp_timer_get_time() };
    xQueueSend(s_q, &e, 0);
}
static int idx(const char *set, char c) { const char *p = strchr(set, c); return p && c ? (int)(p - set) : -1; }
bool nsn_keys_set(char button, char gesture, const char *action)
{
    int k = idx("cpm", button), g = idx("123h", gesture), a = -1;
    for (int i = 0; i < A_N; i++) if (!strcmp(action, ANAME[i])) a = i;
    if (k < 0 || g < 0 || a < 0) return false;
    s_map[k][g] = (uint8_t)a; save(k);
    return true;
}
void nsn_keys_preset(int buttons)
{
    if (buttons < 1 || buttons > 3) return;
    memcpy(s_map, PRESET[buttons - 1], sizeof s_map);
    for (int k = 0; k < NSN_KEY_N; k++) save(k);
}
void nsn_keys_line(int k, char *out, int n)
{
    snprintf(out, n, "keys %c 1=%s 2=%s 3=%s h=%s", BTN[k], ANAME[s_map[k][0]], ANAME[s_map[k][1]], ANAME[s_map[k][2]], ANAME[s_map[k][3]]);
}
void nsn_keys_last(char *out, int n)
{
    if (!s_last[0]) snprintf(out, n, "keys last none");
    else snprintf(out, n, "keys last %s %s %d s ago", s_last, ANAME[s_last_act], (int)((esp_timer_get_time() - s_last_us) / 1000000));
}
