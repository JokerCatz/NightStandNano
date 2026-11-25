// SPDX-License-Identifier: LGPL-2.1-or-later
/* Settings persistence: one NVS key per setting (namespace "nsn"), written only when the value changed.
 * Older builds stored the whole struct as one blob under key "set"; that is migrated on first load. */
#include "settings.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "nvs.h"

#define NS          "nsn"
#define KEY_VER     "ver"       /* i32 marker: 2 = per-key layout is present */
#define KEY_BLOB    "set"       /* legacy single-blob layout */
#define LAYOUT_VER  2

#define SET_DEFAULTS { .vol = 70, .out = 0, .led = 12, .autoplay = 0, .alarm_min = -1, .alarm_ramp_s = 60, .alarm_src = "", \
                       .bt_mode = 0, .bt_have = 0, .bt_addr = { 0 }, .last = "", .play_mode = 0 }

nsn_settings_t g_set = SET_DEFAULTS;

static nsn_settings_t s_shadow;         /* what NVS holds (valid when s_have_shadow) */
static bool s_have_shadow;              /* false: unknown NVS content, the next save writes every key */
static SemaphoreHandle_t s_mtx;

static void lock(void)
{
    static portMUX_TYPE init_mux = portMUX_INITIALIZER_UNLOCKED;
    if (!s_mtx) {
        SemaphoreHandle_t m = xSemaphoreCreateMutex();
        portENTER_CRITICAL(&init_mux);
        bool won = (s_mtx == NULL);
        if (won) s_mtx = m;
        portEXIT_CRITICAL(&init_mux);
        if (!won && m) vSemaphoreDelete(m);
    }
    if (s_mtx) xSemaphoreTake(s_mtx, portMAX_DELAY);
}
static void unlock(void) { if (s_mtx) xSemaphoreGive(s_mtx); }

static void validate(nsn_settings_t *s)
{
    s->last[sizeof s->last - 1] = 0; s->alarm_src[sizeof s->alarm_src - 1] = 0;
    if (s->alarm_min < -1 || s->alarm_min >= 1440) s->alarm_min = -1;
    if (s->alarm_ramp_s < 0 || s->alarm_ramp_s > 1800) s->alarm_ramp_s = 60;
    if (s->vol < 0 || s->vol > 100) s->vol = 70;
    if (s->out < 0 || s->out > 3) s->out = 0;
    if (s->led < 0 || s->led > 255) s->led = 12;
    if (s->play_mode < 0 || s->play_mode > 3) s->play_mode = 0;
}

/* ---- reading ---- */
static void rd_i32(nvs_handle_t h, const char *k, int *v)
{
    int32_t t;
    if (nvs_get_i32(h, k, &t) == ESP_OK) *v = (int)t;
}
static void rd_str(nvs_handle_t h, const char *k, char *buf, size_t size)
{
    size_t len = size;
    if (nvs_get_str(h, k, buf, &len) != ESP_OK) return;
    buf[size - 1] = 0;
}
static void rd_all(nvs_handle_t h, nsn_settings_t *s)
{
    rd_i32(h, "vol", &s->vol);
    rd_i32(h, "out", &s->out);
    rd_i32(h, "led", &s->led);
    rd_i32(h, "autoplay", &s->autoplay);
    rd_i32(h, "alarm_min", &s->alarm_min);
    rd_i32(h, "alarm_ramp", &s->alarm_ramp_s);
    rd_str(h, "alarm_src", s->alarm_src, sizeof s->alarm_src);
    rd_i32(h, "bt_mode", &s->bt_mode);
    rd_i32(h, "bt_have", &s->bt_have);
    uint8_t a[sizeof s->bt_addr]; size_t len = sizeof a;
    if (nvs_get_blob(h, "bt_addr", a, &len) == ESP_OK && len == sizeof a) memcpy(s->bt_addr, a, sizeof a);
    rd_str(h, "last", s->last, sizeof s->last);
    rd_i32(h, "play_mode", &s->play_mode);
}

/* Legacy blob: current size, or the older size without the trailing play_mode int. */
static bool rd_legacy_blob(nvs_handle_t h, nsn_settings_t *s)
{
    nsn_settings_t b = *s; size_t len = sizeof b;
    if (nvs_get_blob(h, KEY_BLOB, &b, &len) != ESP_OK) return false;
    if (len != sizeof b && len != sizeof b - sizeof(int)) return false;
    *s = b;
    return true;
}

/* ---- writing (caller holds the lock) ---- */
typedef struct { nvs_handle_t h; bool force; esp_err_t err; } wr_t;

static void wr_i32(wr_t *w, const char *k, int v, int old)
{
    if (w->force || v != old) { esp_err_t e = nvs_set_i32(w->h, k, v); if (e != ESP_OK) w->err = e; }
}
static void wr_str(wr_t *w, const char *k, const char *v, const char *old)
{
    if (w->force || strcmp(v, old) != 0) { esp_err_t e = nvs_set_str(w->h, k, v); if (e != ESP_OK) w->err = e; }
}
static void wr_blob(wr_t *w, const char *k, const void *v, const void *old, size_t n)
{
    if (w->force || memcmp(v, old, n) != 0) { esp_err_t e = nvs_set_blob(w->h, k, v, n); if (e != ESP_OK) w->err = e; }
}

/* Writes the keys that differ from the shadow (all keys when force or no shadow). Returns true when NVS is up to date. */
static bool save_locked(bool force)
{
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return false;
    nsn_settings_t cur = g_set;                 /* snapshot: other tasks may write g_set meanwhile */
    nsn_settings_t *o = &s_shadow;
    wr_t w = { .h = h, .force = force || !s_have_shadow, .err = ESP_OK };
    wr_i32(&w, "vol", cur.vol, o->vol);
    wr_i32(&w, "out", cur.out, o->out);
    wr_i32(&w, "led", cur.led, o->led);
    wr_i32(&w, "autoplay", cur.autoplay, o->autoplay);
    wr_i32(&w, "alarm_min", cur.alarm_min, o->alarm_min);
    wr_i32(&w, "alarm_ramp", cur.alarm_ramp_s, o->alarm_ramp_s);
    wr_str(&w, "alarm_src", cur.alarm_src, o->alarm_src);
    wr_i32(&w, "bt_mode", cur.bt_mode, o->bt_mode);
    wr_i32(&w, "bt_have", cur.bt_have, o->bt_have);
    wr_blob(&w, "bt_addr", cur.bt_addr, o->bt_addr, sizeof cur.bt_addr);
    wr_str(&w, "last", cur.last, o->last);
    wr_i32(&w, "play_mode", cur.play_mode, o->play_mode);
    if (w.force) wr_i32(&w, KEY_VER, LAYOUT_VER, 0);    /* marker last: only complete data carries it */
    bool ok = (w.err == ESP_OK);
    if (ok && nvs_commit(h) != ESP_OK) ok = false;      /* nothing pending: commit is a cheap no-op */
    nvs_close(h);
    if (ok) { s_shadow = cur; s_have_shadow = true; }
    else s_have_shadow = false;                         /* unknown state: rewrite everything next time */
    return ok;
}

void nsn_settings_load(void)
{
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { nvs_flash_erase(); nvs_flash_init(); }
    lock();
    static const nsn_settings_t def = SET_DEFAULTS;
    nsn_settings_t s = def;
    g_set = s;
    s_have_shadow = false;
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) { unlock(); return; }
    int32_t ver = 0;
    bool have_ver = (nvs_get_i32(h, KEY_VER, &ver) == ESP_OK && ver >= LAYOUT_VER);
    bool migrate = false;
    if (have_ver) rd_all(h, &s);
    else migrate = rd_legacy_blob(h, &s);
    nvs_close(h);
    validate(&s);
    g_set = s;
    if (have_ver) { s_shadow = s; s_have_shadow = true; }
    else if (migrate && save_locked(true) && nvs_open(NS, NVS_READWRITE, &h) == ESP_OK) {
        /* per-key data (incl. bt_addr) is committed: the blob can go */
        nvs_erase_key(h, KEY_BLOB);
        nvs_commit(h);
        nvs_close(h);
    }
    unlock();
}

void nsn_settings_save(void)
{
    lock();
    save_locked(false);
    unlock();
}

int nsn_settings_get_i32(const char *key, int def)
{
    int v = def;
    lock();
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) == ESP_OK) {
        rd_i32(h, key, &v);
        nvs_close(h);
    }
    unlock();
    return v;
}

void nsn_settings_set_i32(const char *key, int v)
{
    lock();
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) == ESP_OK) {
        int32_t cur;
        if (nvs_get_i32(h, key, &cur) != ESP_OK || cur != (int32_t)v) {
            if (nvs_set_i32(h, key, v) == ESP_OK) nvs_commit(h);
        }
        nvs_close(h);
    }
    unlock();
}

bool nsn_settings_get_str(const char *key, char *buf, size_t cap)
{
    bool ok = false;
    lock();
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) == ESP_OK) {
        size_t len = cap;
        ok = nvs_get_str(h, key, buf, &len) == ESP_OK;
        nvs_close(h);
    }
    unlock();
    if (!ok && cap) buf[0] = 0;
    return ok;
}

bool nsn_settings_set_str(const char *key, const char *v)
{
    bool ok = false;
    lock();
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) == ESP_OK) {
        char cur[130]; size_t len = sizeof cur;
        if (nvs_get_str(h, key, cur, &len) == ESP_OK && !strcmp(cur, v)) ok = true;
        else ok = nvs_set_str(h, key, v) == ESP_OK && nvs_commit(h) == ESP_OK;
        nvs_close(h);
    }
    unlock();
    return ok;
}

void nsn_settings_erase(const char *key)
{
    lock();
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_erase_key(h, key) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
    unlock();
}
