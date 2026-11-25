// SPDX-License-Identifier: LGPL-2.1-or-later
#include "cmd.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "player.h"
#include "uac_host.h"
#include "bt_audio.h"
#include "storage.h"
#include "spectrum.h"
#include "wifi.h"
#include "clock.h"
#include "led.h"
#include "settings.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "radio.h"
#include "scene.h"
#include "fx.h"
#include "night.h"
#include "cast.h"
#include "keys.h"
#include "events.h"
#include "bt_ctl.h"
#include "card_svc.h"
#include "cardfmt.h"
#include "outcheck.h"
#include "zipfs.h"
#include "ratetest.h"
#include "trim.h"
#include "hw.h"
#include "sink.h"
#include "esp_app_desc.h"

typedef struct {
    nsn_out_t out;
    void *ctx;
    const char *line;   /* the whole trimmed command line (fx/fxs forward it unchanged) */
    bool local;         /* true only for the wired UART console */
} o_t;

#define UNUSED __attribute__((unused))

static void say(o_t *o, const char *fmt, ...)
{
    char b[800]; va_list ap;
    va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    o->out(o->ctx, b);
}

/* mkwav PATH: 5 s test file, 44.1 kHz stereo S16, 440 Hz left / 660 Hz right, with a click every second */
static void mkwav(o_t *o, const char *path, uint32_t rate)
{
    FILE *f = fopen(path, "wb");
    if (!f) { say(o, "mkwav: cannot create"); return; }
    const uint32_t frames = rate * 5, data = frames * 4;
    uint8_t h[44] = { 'R','I','F','F', 0,0,0,0, 'W','A','V','E', 'f','m','t',' ', 16,0,0,0, 1,0, 2,0, 0,0,0,0, 0,0,0,0,
                      4,0, 16,0, 'd','a','t','a', 0,0,0,0 };
    uint32_t v = 36 + data; memcpy(h + 4, &v, 4); memcpy(h + 24, &rate, 4); v = rate * 4; memcpy(h + 28, &v, 4); memcpy(h + 40, &data, 4);
    fwrite(h, 1, 44, f);
    static int16_t blk[1024 * 2];
    for (uint32_t i = 0; i < frames;) {
        int n = frames - i > 1024 ? 1024 : frames - i;
        for (int k = 0; k < n; k++, i++) {
            float t = (float)i / rate;
            bool click = (i % rate) < 200;
            blk[2 * k] = (int16_t)(6000.0f * sinf(6.2831853f * 440.0f * t) + (click ? 8000 : 0));
            blk[2 * k + 1] = (int16_t)(6000.0f * sinf(6.2831853f * 660.0f * t));
        }
        fwrite(blk, 4, n, f);
    }
    fclose(f);
    say(o, "mkwav: wrote %s", path);
}

static size_t tone441_pull(int16_t *dst, size_t frames, void *ctx)
{
    static float ph;
    for (size_t i = 0; i < frames; i++) {
        dst[2 * i] = dst[2 * i + 1] = (int16_t)(3277.0f * sinf(ph));
        ph += 6.2831853f * 1000.0f / 44100.0f;
        if (ph > 6.2831853f) ph -= 6.2831853f;
    }
    return frames;
}

static void bt_cmd(o_t *o, const char *args)
{
    if (!strncmp(args, "mode ", 5)) {
        nsn_bt_mode_t m = !strcmp(args + 5, "headphones") ? NSN_BT_HEADPHONES : NSN_BT_SPEAKER;
        if (m == NSN_BT_SPEAKER && nsn_player_output() == NSN_OUT_BT) nsn_player_set_output(NSN_OUT_AUTO);
        say(o, "bt mode %s: %s", m == NSN_BT_HEADPHONES ? "headphones" : "speaker", esp_err_to_name(nsn_bt_set_mode(m)));
        return;
    }
    if (!strncmp(args, "pair", 4)) {
        int s = atoi(args + 4);
        int secs = args[4] ? (s > 0 ? s : 120) : 120;
        nsn_bt_pairable(secs);
        say(o, "pairing window %d s", secs);
        return;
    }
    if (!strcmp(args, "ldactest")) { char b[400]; nsn_bt_ldac_selftest(b, sizeof b); say(o, "%s", b); return; }
    if (!strcmp(args, "scan")) { say(o, "scan: %s", esp_err_to_name(nsn_bt_scan())); return; }
    if (!strcmp(args, "list")) {
        nsn_bt_dev_t d[12]; int n = nsn_bt_scan_results(d, 12);
        for (int i = 0; i < n; i++)
            say(o, "%02x:%02x:%02x:%02x:%02x:%02x %4d %06lx %c %s", d[i].bda[0], d[i].bda[1], d[i].bda[2], d[i].bda[3], d[i].bda[4],
                d[i].bda[5], d[i].rssi, (unsigned long)d[i].cod, d[i].audio ? 'A' : ' ', d[i].name);
        say(o, "%d device(s)%s", n, nsn_bt_scanning() ? " (scanning)" : "");
        return;
    }
    if (!strncmp(args, "conn ", 5)) {
        unsigned a[6];
        if (sscanf(args + 5, "%x:%x:%x:%x:%x:%x", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]) != 6) { say(o, "bad address"); return; }
        uint8_t b[6]; for (int i = 0; i < 6; i++) b[i] = a[i];
        say(o, "conn: %s", esp_err_to_name(nsn_bt_connect(b)));
        return;
    }
    if (!strcmp(args, "tone")) { nsn_bt_set_source(tone441_pull, NULL); say(o, "tone: %s", esp_err_to_name(nsn_bt_start())); return; }
    if (!strcmp(args, "unpair")) { say(o, "unpaired %d", nsn_bt_forget_all()); return; }
    if (!strcmp(args, "pause")) { say(o, "pause: %s", esp_err_to_name(nsn_bt_suspend())); return; }
    if (!strcmp(args, "disc")) { say(o, "disc: %s", esp_err_to_name(nsn_bt_disconnect())); return; }
    char b[160]; nsn_bt_status(b, sizeof b); say(o, "%s", b);
}

/* JSON string escaping for the st line: backslash-escape quotes and backslashes, drop control characters */
static void json_esc(char *dst, size_t cap, const char *s)
{
    size_t j = 0;
    for (; *s && j < cap - 6; s++) {
        if (*s == '"' || *s == '\\') dst[j++] = '\\';
        if ((unsigned char)*s >= 32) dst[j++] = *s;
    }
    dst[j] = 0;
}

/* CPU load per core in percent since the previous call (idle task run time against wall time); -1 until there are two samples */
static void cpu_load(int out[2])
{
    static uint32_t last_idle[2]; static int64_t last_us;
    int64_t now = esp_timer_get_time();
    for (int c = 0; c < 2; c++) {
        uint32_t idle = (uint32_t)ulTaskGetIdleRunTimeCounterForCore(c);
        int64_t dt = now - last_us;
        out[c] = last_us && dt > 100000 ? 100 - (int)((uint64_t)(uint32_t)(idle - last_idle[c]) * 100 / (uint64_t)dt) : -1;
        if (out[c] < -1) out[c] = 0;
        if (out[c] > 100) out[c] = 100;
        last_idle[c] = idle;
    }
    last_us = now;
}

/* lrc: the lyrics file next to the playing track (same name, .lrc), raw bytes as base64 lines; the page parses and keeps time.
 *   lrc none   |   lrc begin <bytes>  /  lrc+ <base64> ...  /  lrc end */
static void h_lrc(o_t *o, char *args)
{
    (void)args;
    nsn_player_info_t p; nsn_player_info(&p);
    char path[176]; strlcpy(path, p.path, sizeof path);
    char *dot = strrchr(path, '.'), *slash = strrchr(path, '/');
    if (!p.active || path[0] != '/' || !dot || (slash && dot < slash) || strlen(path) + 5 > sizeof path) { say(o, "lrc none"); return; }
    strcpy(dot, ".lrc");
    uint8_t *buf = NULL; size_t n = 0;
    char zip[176]; const char *inner;
    if (nsn_zip_split(path, zip, sizeof zip, &inner)) {                    /* a track inside an archive: its lyrics are an entry of the same archive */
        uint32_t len = 0;
        if (nsn_zip_read(zip, inner, 65536, &buf, &len) != 0) { say(o, "lrc none"); return; }
        n = len;
    } else {
        FILE *f = fopen(path, "rb");
        if (!f) { say(o, "lrc none"); return; }
        buf = heap_caps_malloc(65536, MALLOC_CAP_SPIRAM);
        n = buf ? fread(buf, 1, 65536, f) : 0;
        fclose(f);
    }
    if (!n) { heap_caps_free(buf); say(o, "lrc none"); return; }
    static const char T[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    say(o, "lrc begin %u", (unsigned)n);
    for (size_t i = 0; i < n; i += 180) {                 /* 180 bytes -> 240 characters per line */
        char line[250]; size_t k = 0, m = n - i < 180 ? n - i : 180;
        for (size_t j = 0; j < m; j += 3) {
            uint32_t v = (uint32_t)buf[i + j] << 16 | (j + 1 < m ? buf[i + j + 1] << 8 : 0) | (j + 2 < m ? buf[i + j + 2] : 0);
            line[k++] = T[v >> 18]; line[k++] = T[(v >> 12) & 63];
            line[k++] = j + 1 < m ? T[(v >> 6) & 63] : '=';
            line[k++] = j + 2 < m ? T[v & 63] : '=';
        }
        line[k] = 0;
        say(o, "lrc+ %s", line);
    }
    say(o, "lrc end");
    heap_caps_free(buf);
}

/* memwatch [seconds]: sample free internal RAM every tick and report the lowest moment (finds short dips that `st` cannot see) */
static void memwatch_task(void *arg)
{
    int ms = (int)(intptr_t)arg; size_t lo = SIZE_MAX, lo_big = SIZE_MAX; int at = 0;
    for (int i = 0; i < ms / (int)portTICK_PERIOD_MS; i++) {
        size_t f = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        if (f < lo) { lo = f; at = i * (int)portTICK_PERIOD_MS; lo_big = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL); }
        vTaskDelay(1);
    }
    printf("memwatch: lowest free %u at %d ms (largest block then %u)\n", (unsigned)lo, at, (unsigned)lo_big);
    vTaskDelete(NULL);
}
static void h_memwatch(o_t *o, char *args)
{
    int s = args ? atoi(args) : 4;
    if (s < 1 || s > 60) s = 4;
    xTaskCreate(memwatch_task, "memwatch", 3072, (void *)(intptr_t)(s * 1000), configMAX_PRIORITIES - 2, NULL);
    say(o, "memwatch: %d s", s);
}

/* rg [off|track|album]: ReplayGain from the files' tags (applies from the next track) */
static void h_rg(o_t *o, char *args)
{
    static const char *const nm[] = { "off", "track", "album" };
    if (args && args[0]) {
        int m = -1;
        for (int i = 0; i < 3; i++) if (!strcmp(args, nm[i])) m = i;
        if (m < 0) { say(o, "rg off|track|album"); return; }
        nsn_player_set_rg(m); nsn_settings_set_i32("rg", m);
    }
    say(o, "rg: %s", nm[nsn_player_rg()]);
}

/* "card" in st: ok = mounted, bad = a card that answers but has no readable file system (the page may offer cardfmt), none = no card */
static const char *card_word(void)
{
    nsn_card_state_t c = nsn_storage_state();
    return c == NSN_CARD_OK ? "ok" : c == NSN_CARD_BAD ? "bad" : "none";
}

/* st: one JSON line for apps and the web page */
static void json_status(o_t *o)
{
    nsn_player_info_t p; nsn_player_info(&p);
    nsn_uac_info_t u; nsn_uac_info(&u);
    const char *name = strrchr(p.path, '/'); name = name ? name + 1 : p.path;
    char esc[96], tesc[96], aesc[64], lesc[64];
    json_esc(esc, sizeof esc, name);
    json_esc(tesc, sizeof tesc, nsn_radio_title()[0] ? nsn_radio_title() : p.title);   /* radio: the ICY title; card file: its tag */
    json_esc(aesc, sizeof aesc, p.artist); json_esc(lesc, sizeof lesc, p.album);
    int cpu[2]; cpu_load(cpu);
    say(o, "{\"st\":\"%s\",\"card\":\"%s\",\"fmt\":\"%s\",\"rate\":%lu,\"pos\":%ld,\"len\":%ld,\"vol\":%d,\"trk\":%d,\"n\":%d,\"sub\":%d,\"subn\":%d,"
           "\"sleep\":%d,\"cpu\":[%d,%d],\"mode\":%d,\"und\":%lu,\"out\":\"%s\",\"dac\":%d,\"bt\":%d,\"wifi\":%d,\"mem\":%u,"
           "\"title\":\"%s\",\"rstate\":\"%s\",\"file\":\"%s\",\"bm\":%d,\"artist\":\"%s\",\"album\":\"%s\",\"rg\":%d,\"fxshed\":%d,\"big\":%u,\"psram\":%u,\"gapless\":%d,\"vmax\":%d,\"dacmax\":%lu}",
        !p.active ? "idle" : p.paused ? "paused" : "playing", card_word(), p.fmt ? p.fmt : "", (unsigned long)p.rate, (long)p.pos_ms, (long)p.len_ms,
        p.volume, p.index, p.count, p.sub, p.subn, p.sleep_left_s, cpu[0], cpu[1], nsn_player_mode(), (unsigned long)p.underruns,
        nsn_sink_name(nsn_player_sink()), u.present, nsn_bt_connected(), nsn_wifi_connected(),
        (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), tesc, nsn_radio_state(), esc, nsn_player_bookmark()[0] ? 1 : 0, aesc, lesc, nsn_player_rg(), nsn_fx_shed() ? 1 : 0,
        (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        nsn_player_gapless() ? 1 : 0, nsn_player_vollimit(), (unsigned long)nsn_player_dacmax());
}

/* ---- command handlers ----------------------------------------------------------------------------------------------------------
 * args: the text after the first space, or NULL when the line had no space at all ("vol" -> NULL, "vol " -> "").
 * Handlers for commands that need an argument, or that take none, answer "?" themselves when the shape is wrong, exactly as an
 * unknown command would. o->local is true only on the wired console. */

static bool need_args(o_t *o, const char *args)
{
    if (args) return true;
    say(o, "?");
    return false;
}

static bool need_noargs(o_t *o, const char *args)
{
    if (!args) return true;
    say(o, "?");
    return false;
}

static void h_reboot(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    esp_restart();
}

static void h_st(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    json_status(o);
}

/* putx[!] PATH HEX  (! = start the file over). Content is never echoed or logged. */
static void putx_do(o_t *o, char *args, bool fresh)
{
    if (!need_args(o, args)) return;
    char *path = args, *hex = strchr(path, ' ');
    if (!hex) { say(o, "putx: bad line"); return; }
    *hex++ = 0;
    FILE *f = fopen(path, fresh ? "wb" : "ab");
    if (!f) { say(o, "putx: cannot open"); return; }
    size_t n = 0;
    for (; hex[0] && hex[1]; hex += 2, n++) {
        unsigned v;
        if (sscanf(hex, "%2x", &v) != 1) break;
        fputc(v, f);
    }
    fclose(f);
    say(o, "ok %u", (unsigned)n);
}

static void h_putx(o_t *o, char *args)  { putx_do(o, args, false); }
static void h_putxf(o_t *o, char *args) { putx_do(o, args, true); }

static void h_play(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    strlcpy(g_set.last, args, sizeof g_set.last);
    nsn_settings_save();
    char why[40];
    esp_err_t e = nsn_player_play_why(args, why, sizeof why);
    if (e == ESP_ERR_NOT_SUPPORTED && why[0]) say(o, "play: ESP_ERR_NOT_SUPPORTED unsupported chip %s", why);   /* the page reads the first word as the error name and shows the text it knows for it */
    else say(o, "play: %s", esp_err_to_name(e));
}

static void h_radio(o_t *o, char *args)
{
    if (!args || !strcmp(args, "list")) {
        int n = nsn_radio_load();
        if (!n) { say(o, "radio: no stations (put name|url lines in /sdcard/_player/config/radio.txt)"); return; }
        for (int i = 0; i < n; i++) {
            const char *nm;
            const char *u = nsn_radio_station(i, &nm);
            say(o, "%d %s %s", i + 1, nm, u);
        }
        return;
    }
    if (!strncmp(args, "add ", 4)) {                      /* radio add <name>|<url>: a favourite, kept on the card */
        char *bar = strrchr(args + 4, '|');
        if (!bar) { say(o, "radio add: name|url"); return; }
        *bar = 0;
        int k = nsn_radio_add(args + 4, bar + 1);
        if (k > 0) say(o, "radio add: %d", k);
        else say(o, "radio add: %s", k == -2 ? "list full" : k == -3 ? "cannot write the card" : "refused");
        return;
    }
    if (!strncmp(args, "del ", 4)) {
        say(o, "radio del: %s", nsn_radio_del(atoi(args + 4) - 1) == 0 ? "ok" : "failed");
        return;
    }
    if (args[0] >= '0' && args[0] <= '9' && !strchr(args, ':')) {
        nsn_radio_load();
        const char *u = nsn_radio_station(atoi(args) - 1, NULL);
        if (!u) { say(o, "radio: no such station"); return; }
        strlcpy(g_set.last, u, sizeof g_set.last);
        nsn_settings_save();
        say(o, "radio: %s", esp_err_to_name(nsn_player_play(u)));
        return;
    }
    strlcpy(g_set.last, args, sizeof g_set.last);
    nsn_settings_save();
    say(o, "radio: %s", esp_err_to_name(nsn_player_play(args)));
}

/* fx / fxs go to fx_console() with the full original line */
static void fx_forward(o_t *o)
{
    fx_set_out(o->out, o->ctx);
    bool h = fx_console(o->line);
    fx_set_out(NULL, NULL);
    if (!h) say(o, "fx: unknown");
}

static void h_fx(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    fx_forward(o);
}

static void h_fxs(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    fx_forward(o);
}

static void h_night(o_t *o, char *args)
{
    const char *rest = args ? args : "";
    int h1, m1, h2, m2;
    if (!strcmp(rest, "on")) nsn_night_set(true);
    else if (!strcmp(rest, "off")) nsn_night_set(false);
    else if (!strcmp(rest, "auto off")) nsn_night_auto(-1, -1);
    else if (sscanf(rest, " auto %d:%d %d:%d", &h1, &m1, &h2, &m2) == 4 &&
             h1 >= 0 && h1 < 24 && h2 >= 0 && h2 < 24 && m1 >= 0 && m1 < 60 && m2 >= 0 && m2 < 60)
        nsn_night_auto(h1 * 60 + m1, h2 * 60 + m2);
    else if (args) { say(o, "night on|off|auto HH:MM HH:MM|auto off"); return; }
    char b[80]; nsn_night_status(b, sizeof b); say(o, "%s", b);
}

static void h_bedtime(o_t *o, char *args)
{
    int min = args ? atoi(args) : 30;
    nsn_bedtime(min);
    say(o, "bedtime: night mode on, sleep timer %d min", min);
}

static void h_wake(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    nsn_night_set(false);
    say(o, "wake: night mode off");
}

static void h_mode(o_t *o, char *args)
{
    static const char *const nm[] = { "folder", "shuffle", "one", "once" };
    if (args) {
        int m = -1;
        for (int i = 0; i < 4; i++) if (!strcmp(args, nm[i])) m = i;
        if (m < 0) { say(o, "mode: folder|shuffle|one|once"); return; }
        g_set.play_mode = m;
        nsn_player_set_mode(m);
        nsn_settings_save();
    }
    say(o, "mode: %s", nm[nsn_player_mode()]);
}

static void h_scene(o_t *o, char *args)
{
    int n = nsn_scene_load();
    if (!args) {
        if (!n) { say(o, "scene: none (tools/make_scenes.py builds /sdcard/_player/scenes/)"); return; }
        for (int i = 0; i < n; i++) say(o, "%d %s", i + 1, nsn_scene_name(i));
        return;
    }
    int k = atoi(args);
    if (k < 1 || k > n) { say(o, "scene: no such scene"); return; }
    char src[24]; snprintf(src, sizeof src, "scene:%d", k % 1000);
    esp_err_t e = nsn_player_play(src);
    if (e == ESP_OK) { strlcpy(g_set.last, src, sizeof g_set.last); nsn_settings_save(); }
    say(o, "scene: %s", esp_err_to_name(e));
}

static void h_noise(o_t *o, char *args)
{
    if (!args) { say(o, "noise colours: white pink brown blue violet grey green velvet  (noise <colour>, stop to end)"); return; }
    char src[48]; strlcpy(src, "noise:", sizeof src); strlcat(src, args, sizeof src);
    esp_err_t e = nsn_player_play(src);
    if (e == ESP_OK) { strlcpy(g_set.last, src, sizeof g_set.last); nsn_settings_save(); }
    say(o, "noise: %s", esp_err_to_name(e));
}

static void h_stop(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    nsn_player_stop();
    say(o, "stopped");
}

static void h_autoplay(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    g_set.autoplay = atoi(args) != 0;
    nsn_settings_save();
    say(o, "autoplay %d (last: %s)", g_set.autoplay, g_set.last);
}


static void h_next(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    nsn_player_skip(1);
    say(o, "next");
}

static void h_prev(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    nsn_player_skip(-1);
    say(o, "prev");
}

static void h_timer(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    nsn_player_sleep(atoi(args));
    say(o, "sleep timer %d min", atoi(args));
}

static void h_out(o_t *o, char *args)
{
    const char *rest = args ? args : "";
    int id = nsn_sink_by_name(rest);                   /* dac bt cast, and whatever the board registered (i2s) */
    if (id == NSN_OUT_CAST) {                          /* Auracast broadcast: exclusive with Bluetooth headphone output */
        nsn_player_stop();
        if (nsn_ble_connected() && !o->local) { say(o, "out cast: starting, the Bluetooth control link reconnects in a moment"); vTaskDelay(pdMS_TO_TICKS(400)); }
        esp_err_t e = nsn_cast_start();
        if (e != ESP_OK) { say(o, "out cast: %s", esp_err_to_name(e)); return; }
        nsn_player_set_output(NSN_OUT_CAST);
    }
    else if (id || !strcmp(rest, "auto")) {
        if (nsn_cast_on()) { nsn_player_stop(); nsn_cast_stop(); }
        nsn_player_set_output(id);                     /* 0 = auto */
        if (id == NSN_OUT_BT && nsn_bt_mode() != NSN_BT_HEADPHONES) nsn_bt_set_mode(NSN_BT_HEADPHONES);
    }
    int pref = nsn_player_output();
    say(o, "out %s (playing on %s)", pref ? nsn_sink_name(pref) : "auto", nsn_sink_name(nsn_player_sink()));
}

/* keys | keys preset 1|2|3 | keys set <c|p|m> <1|2|3|h> <action> : headset line-control buttons (keys.h) */
static void h_keys(o_t *o, char *args)
{
    char b, g, a[12]; int n;
    if (args && o->local && sscanf(args, "sim %c %c", &b, &g) == 2) {          /* wired console: act out a gesture without a headset */
        const char *bs = "cpm", *bp = strchr(bs, b);
        if (!bp) { say(o, "keys sim <c|p|m> <1|2|3|h>"); return; }
        int k = (int)(bp - bs);
        if (g == 'h') { nsn_keys_event(k, true); vTaskDelay(pdMS_TO_TICKS(1500)); nsn_keys_event(k, false); }
        else for (int i = 0; i < g - '0' && i < 3; i++) { nsn_keys_event(k, true); vTaskDelay(pdMS_TO_TICKS(60)); nsn_keys_event(k, false); vTaskDelay(pdMS_TO_TICKS(120)); }
        vTaskDelay(pdMS_TO_TICKS(600));
    }
    else if (args && sscanf(args, "preset %d", &n) == 1) nsn_keys_preset(n);
    else if (args && sscanf(args, "set %c %c %11s", &b, &g, a) == 3) { if (!nsn_keys_set(b, g, a)) { say(o, "keys set <c|p|m> <1|2|3|h> <none|play|next|prev|vol+|vol-|stop>"); return; } }
    else if (args && args[0]) { say(o, "keys | keys preset 1|2|3 | keys set <c|p|m> <1|2|3|h> <action>"); return; }
    char l[96];
    for (int k = 0; k < NSN_KEY_N; k++) { nsn_keys_line(k, l, sizeof l); say(o, "%s", l); }
    nsn_keys_last(l, sizeof l); say(o, "%s", l);
}

/* events [from_seq]: recent things the box noticed (events.h) */
static void h_events(o_t *o, char *args)
{
    unsigned from = args ? (unsigned)strtoul(args, NULL, 10) : 0, seq; int age; char t[64];
    for (int i = 0; nsn_events_get(from, i, &seq, &age, t, sizeof t); i++) say(o, "ev %u %d %s", seq, age, t);
    say(o, "ev end %u", nsn_events_next_seq());
}

static void h_cast(o_t *o, char *args)
{
    (void)args;
    char b[200]; nsn_cast_status(b, sizeof b); say(o, "%s", b);
}

static void h_pause(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    nsn_player_pause(true);
    say(o, "paused");
}

static void h_resume(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    nsn_player_info_t p; nsn_player_info(&p);
    if (p.active) { nsn_player_pause(false); say(o, "resumed"); return; }
    if (nsn_player_resume() == ESP_OK) say(o, "resumed %s", nsn_player_bookmark());     /* idle: go on from the bookmark */
    else say(o, "nothing to resume");
}

/* vol [N]: the volume of the output that plays now (idle: of the one that would be picked), never above `vollimit` */
static void h_vol(o_t *o, char *args)
{
    if (args && args[0]) { nsn_player_set_volume(atoi(args)); nsn_player_persist(); }
    say(o, "vol %d", nsn_player_volume());
}

/* vollimit [N]: 0..100, default 100; no volume above it is applied or shown, on any path */
static void h_vollimit(o_t *o, char *args)
{
    if (args && args[0]) { nsn_player_set_vollimit(atoi(args)); nsn_player_persist(); }
    say(o, "vollimit %d", nsn_player_vollimit());
}

/* gapless [on|off]: track ends flow into the next track (same rate and output) with no gap; default on */
static void h_gapless(o_t *o, char *args)
{
    if (args && args[0]) {
        if (!strcmp(args, "on") || !strcmp(args, "off")) { nsn_player_set_gapless(args[1] == 'n'); nsn_player_persist(); }
        else { say(o, "gapless on|off"); return; }
    }
    say(o, "gapless %s", nsn_player_gapless() ? "on" : "off");
}

/* expfmt [on|off]: the AdLib / tracker readers of pc_chipseq's experimental tier (a2m a2t d00 sa2 ...): on by default, applies from the next file */
static void h_expfmt(o_t *o, char *args)
{
    if (args && args[0]) {
        if (!strcmp(args, "on") || !strcmp(args, "off")) nsn_player_set_expfmt(args[1] == 'n');
        else { say(o, "expfmt on|off"); return; }
    }
    say(o, "expfmt %s", nsn_player_expfmt() ? "on" : "off");
}

/* dacmax [auto|<rate>]: the highest sample rate the USB DAC is asked to run at. Tracks above it are resampled down (an exact halving where there is one,
 * else the highest rate the DAC may run at in the same 44.1 / 48 kHz family); applies from the next track. auto = whatever the DAC takes (default).
 * A rate is rounded down to 48000 / 96000 / 192000 / 384000 (JukeDial's pc_dacmax tiers); "96k" is accepted. Reply "dacmax auto" / "dacmax 96000". */
static void h_dacmax(o_t *o, char *args)
{
    if (args && args[0]) {
        if (!strcmp(args, "auto")) nsn_player_set_dacmax(0);
        else {
            char *e; double v = strtod(args, &e);
            if (*e == 'k' || *e == 'K') { v *= 1000; e++; }
            if (*e || v < 1000) { say(o, "dacmax auto|48000|96000|192000|384000"); return; }
            nsn_player_set_dacmax((uint32_t)v);
        }
        nsn_player_persist();
    }
    if (nsn_player_dacmax()) say(o, "dacmax %lu", (unsigned long)nsn_player_dacmax()); else say(o, "dacmax auto");
}

/* ratetest | ratetest status | ratetest cancel: the USB DAC's sample-rate test (ratetest.c). For each rate the DAC advertises: the clock is set, ~1.5 s of a
 * quiet 1 kHz sine (-30 dBFS at most, scaled by the volume, under the volume limit) is streamed, and the ISO transfers are counted. Stops playback first.
 *   ratetest         ->  "ratetest: started"  or  "ratetest: refused <reason>" (no dac, card test running, formatting, already running; while it runs: the status)
 *   ratetest status  ->  ratetest state=idle|running|done|cancelled|failed dac=<vid>:<pid> [now=<rate>] [reason="..."]
 *                        ratetest rate=<Hz> set=ok|fail|na clock=<read-back Hz> xfer_ok=<n> xfer_bad=<n> fps=<measured frames per second> verdict=ok|bad|untested   (one per rate)
 *                        ratetest max=<highest rate with verdict ok>                 (when finished)
 *   ratetest cancel  ->  "ratetest: cancelled" or "ratetest: refused not running"
 * Verdicts are stored per DAC (USB VID:PID) in NVS and decide which rates the player uses. */
static void h_ratetest(o_t *o, char *args)
{
    if (args && !strcmp(args, "cancel")) {
        if (nsn_ratetest_cancel()) say(o, "ratetest: cancelled"); else say(o, "ratetest: refused not running");
        return;
    }
    if (args && strcmp(args, "status") && strcmp(args, "start")) { say(o, "ratetest [status|cancel]"); return; }
    if (!args || !strcmp(args, "start")) {
        if (!nsn_ratetest_running()) {
            const char *why = nsn_ratetest_start();
            if (why) say(o, "ratetest: refused %s", why); else say(o, "ratetest: started");
            return;
        }
    }
    nsn_ratetest_report(o->out, o->ctx);
}

static void oc_line(void *ctx, const char *s) { say((o_t *)ctx, "%s", s); }

/* dbg ...: UART-only test helpers
 *   dbg oc arm|off|show       output check (outcheck.h): counts jumps / dropouts / inserted silence in the PCM the sink pulls
 *   dbg vol                   per-output volumes, limit, effective          dbg vol forget bt|cast   forget that output's own volume
 *   dbg ramp                  the soft start's output peaks                 dbg looplen MS   scene layers loop after MS ms (0 = off)
 *   dbg scene NAME|file.ogg:vol|...   adds a test scene (RAM only, until reboot) after those of scenes.txt */
static void h_dbg(o_t *o, char *args)
{
    if (!args) { say(o, "dbg oc|vol|ramp|looplen|scene|trim"); return; }
    char b[240];
    if (!strcmp(args, "oc arm")) { nsn_oc_arm(true); say(o, "oc armed"); }
    else if (!strcmp(args, "oc off")) { nsn_oc_arm(false); say(o, "oc off"); }
    else if (!strcmp(args, "oc show") || !strcmp(args, "oc")) nsn_oc_report(oc_line, o);
    else if (!strcmp(args, "vol")) { nsn_player_vol_dump(b, sizeof b); say(o, "%s", b); }
    else if (!strcmp(args, "vol forget bt")) { nsn_player_vol_forget(1); say(o, "bt volume forgotten"); }
    else if (!strcmp(args, "vol forget cast")) { nsn_player_vol_forget(2); say(o, "cast volume forgotten"); }
    else if (!strcmp(args, "trim on") || !strcmp(args, "trim off")) { nsn_trim_enable(args[6] == 'n'); say(o, "trim %s (applies to the next track)", nsn_trim_enabled() ? "on" : "off"); }
    else if (!strncmp(args, "rt ", 3)) {                                        /* dbg rt <rate> ok|bad|untested: a verdict for the attached DAC, RAM only */
        unsigned r = (unsigned)atoi(args + 3); const char *w = strrchr(args, ' ');
        nsn_ratetest_debug_set(r, w && !strcmp(w + 1, "ok") ? NSN_RT_OK : w && !strcmp(w + 1, "bad") ? NSN_RT_BAD : NSN_RT_UNTESTED);
        say(o, "rt %u -> %d", r, nsn_ratetest_verdict(r));
    }
    else if (!strncmp(args, "hlsreuse ", 9)) { nsn_radio_debug_reuse(atoi(args + 9)); say(o, "hlsreuse %d (connections so far %u)", atoi(args + 9), nsn_radio_connections()); }
    else if (!strcmp(args, "conns")) say(o, "connections %u", nsn_radio_connections());
    else if (!strncmp(args, "opendelay ", 10)) { nsn_player_debug_opendelay(atoi(args + 10)); say(o, "opendelay %d ms", atoi(args + 10)); }
    else if (!strncmp(args, "xfade ", 6)) { nsn_scene_debug_xfade(atoi(args + 6)); say(o, "scene xfade %d ms", atoi(args + 6)); }
    else if (!strcmp(args, "ramp")) { nsn_player_ramp_dump(b, sizeof b); say(o, "%s", b); }
    else if (!strncmp(args, "looplen ", 8)) { nsn_scene_debug_looplen(atoi(args + 8)); say(o, "looplen %d ms", atoi(args + 8)); }
    else if (!strcmp(args, "spare 0") || !strcmp(args, "spare 1")) { nsn_scene_debug_spare(args[6] == '1'); say(o, "scene loop: %s", args[6] == '1' ? "spare decoder" : "rewind"); }
    else if (!strncmp(args, "scene ", 6)) { nsn_scene_debug_add(args + 6); say(o, "scenes: %d", nsn_scene_load()); }
    else say(o, "dbg oc arm|off|show | vol [forget bt|cast] | ramp | looplen MS | scene NAME|file:vol");
}

static void h_spec(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    uint8_t b[NSN_BANDS]; char t[NSN_BANDS * 4 + 1];
    nsn_spec_last(b);
    for (int i = 0; i < NSN_BANDS; i++) sprintf(t + i * 3, "%02x ", b[i]);
    say(o, "spec %s", t);
}

/* BLE is unencrypted and open by the owner's design, so `wifi set` is allowed there and the password crosses the air in clear: accepted trade-off.
 * The line is never echoed or logged, and no reply contains the password. */
static void h_wifi(o_t *o, char *args)
{
    if (!args) { char b[260]; nsn_wifi_status(b, sizeof b); say(o, "%s", b); }
    else if (!strcmp(args, "on")) say(o, "wifi on: %s", esp_err_to_name(nsn_wifi_start()));
    else if (!strcmp(args, "off")) { nsn_wifi_stop(); say(o, "wifi off"); }
    else if (!strncmp(args, "set ", 4)) {
        char *sep = strchr(args + 4, '|');           /* the FIRST '|' separates: a password may contain '|' */
        if (!sep) { say(o, "wifi set: refused usage: wifi set <ssid>|<password>"); return; }
        *sep = 0;
        const char *why = nsn_wifi_set(args + 4, sep + 1);
        memset(sep + 1, 0, strlen(sep + 1));          /* the password was in the caller's buffer */
        if (why) say(o, "wifi set: refused %s", why); else say(o, "wifi set: ok, connecting");
    }
    else if (!strcmp(args, "forget")) { nsn_wifi_forget(); say(o, "wifi forget: ok"); }
    else if (!strcmp(args, "scan")) {
        nsn_ap_t ap[20];
        int n = nsn_wifi_scan(ap, 20);
        if (n == -1) { say(o, "wifi scan: busy"); return; }
        if (n < 0) { say(o, "wifi scan: failed"); return; }
        for (int i = 0; i < n; i++) {
            char e[72]; json_esc(e, sizeof e, ap[i].ssid);
            say(o, "wifi ap %d %s \"%s\"", ap[i].rssi, ap[i].open ? "open" : "secured", e);
        }
        say(o, "wifi scan: %d found", n);
    }
    else say(o, "?");
}

static void h_time(o_t *o, char *args)
{
    char b[160];
    if (!args) { nsn_clock_status(b, sizeof b); say(o, "%s", b); }
    else if (!strncmp(args, "set ", 4)) say(o, "%s", nsn_clock_cmd_set(args + 4, b, sizeof b));
    else if (!strncmp(args, "tz ", 3)) say(o, "%s", nsn_clock_cmd_tz(args + 3, b, sizeof b));
    else say(o, "?");
}

static void h_led(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    int r, g, b;
    if (sscanf(args, "%d %d %d", &r, &g, &b) == 3) {
        nsn_led_test(r, g, b);
        say(o, "led %d %d %d for 3 s", r, g, b);
    } else {
        nsn_led_set_level(atoi(args));
        say(o, "led level %d", nsn_led_level());
    }
}

static void h_status(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    char b[160]; nsn_player_status(b, sizeof b); say(o, "%s", b);
}

static void h_crash(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    say(o, "crashing on purpose");
    vTaskDelay(pdMS_TO_TICKS(200));
    *(volatile int *)0 = 1;
}

static void h_mkwav(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    char *sp = strchr(args, ' '); uint32_t rate = 44100;
    if (sp) { *sp = 0; rate = atoi(sp + 1); }
    mkwav(o, args, rate ? rate : 44100);
}

static void h_rm(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    say(o, "rm: %d", remove(args));
}

static void h_mkdir(o_t *o, char *args)
{
    if (!need_args(o, args)) return;
    say(o, "mkdir: %d", mkdir(args, 0777));
}

static void h_bt(o_t *o, char *args)
{
    bt_cmd(o, args ? args : "");
}

static void zip_ls_emit(void *ctx, const char *name, bool is_dir) { say((o_t *)ctx, "%s%s", name, is_dir ? "/" : ""); }
static bool zip_ls_ok(const char *name) { return nsn_player_can_play(name); }

/* ls [path]: a folder's entries, "/" after folders. A .zip file is listed as a folder ("x.zip/") and can be entered: "ls /sdcard/x.zip",
 * "ls /sdcard/x.zip/sub"; inside, only what the player can play, plus the sub-folders that hold something playable. */
static void h_ls(o_t *o, char *args)
{
    const char *path = args ? args : "/sdcard";
    char zip[176]; const char *inner;
    if (nsn_zip_split(path, zip, sizeof zip, &inner)) {
        nsn_zip_kind_t kind; uint32_t size;
        int rc = nsn_zip_stat(zip, inner, &kind, &size);
        if (rc || kind != NSN_ZIP_DIR) { say(o, "ls: cannot open"); return; }
        if (nsn_zip_ls(zip, inner, zip_ls_ok, zip_ls_emit, o) < 0) say(o, "ls: cannot open");
        return;
    }
    DIR *d = opendir(path);
    if (!d) { say(o, "ls: cannot open"); return; }
    for (struct dirent *de; (de = readdir(d));)
        say(o, "%s%s", de->d_name, de->d_type == DT_DIR || nsn_zip_is_archive_name(de->d_name) ? "/" : "");
    closedir(d);
}

/* hw: the hardware settings (hw.c). `hw` reads for everyone; every setter is wired-console only and answers "?" over BLE, checked inside. sdpins: the older spelling */
static void h_hw(o_t *o, char *args) { nsn_hw_exec(args, o->local, o->out, o->ctx); }
static void h_sdpins(o_t *o, char *args) { nsn_hw_sdpins_legacy(args, o->local, o->out, o->ctx); }

/* ver: ver <project version> idf=<IDF version> chip=esp32s31 built=<first 9 hex digits of the ELF SHA-256>. The build is reproducible, so there is no
 * date: the ELF hash is the stamp that tells two builds of the same version apart. */
static void h_ver(o_t *o, char *args)
{
    if (args) { say(o, "?"); return; }
    const esp_app_desc_t *d = esp_app_get_description();
    char sha[12] = "";
    esp_app_get_elf_sha256(sha, 10);                 /* 9 hex digits + NUL */
    say(o, "ver %s idf=%s chip=esp32s31 built=%s", d->version, d->idf_ver, sha);
}

/* card: one line with the card's facts (BLE and UART), stable and parseable:
 *   card state=mounted size=<bytes> free=<bytes|?> fs=FAT32|exFAT|FAT16|FAT12|? name="<CID name>" serial=<8 hex digits> fmt=idle|pending|running|done|failed
 *   card state=unreadable size=<bytes> free=? fs=? name="..." serial=...  fmt=...   (the card answers, no file system mounts: cardfmt may be offered)
 *   card state=none fmt=...
 * free / fs are "?" until the card service has computed them once after the mount (f_getfree can take seconds on a big FAT32 card). */
static void h_card(o_t *o, char *args)
{
    if (!need_noargs(o, args)) return;
    char b[200]; nsn_card_line(b, sizeof b); say(o, "%s", b);
}

/* cardtest: the card speed test (JukeDial's 記憶卡診斷). Nothing on the card is changed except the one temporary file of `file` (written, read back, removed)
 * and _player/cardtest.txt (the latest results per card).
 *   cardtest raw|file   ->  "cardtest: started raw|file"  or  "cardtest: refused <reason>"   (reason: no card, card unreadable, already running, formatting,
 *                           small (card under 64 MB), nospace (file test needs 8 MB free), io, nomem)
 *   cardtest cancel     ->  "cardtest: cancelled"  or  "cardtest: refused not running"
 *   cardtest            ->  status and the latest results, one line each, in this order (a line is absent when it has nothing to say):
 *     cardtest state idle | cardtest state running raw|file <pct>%
 *     cardtest fail raw|file <reason>                       the last run of that kind failed
 *     cardtest raw avg16=<MB/s> slow16=<MB/s> max64=<MB/s> worst_us=<us> time=<YYYY-MM-DDTHH:MM|->
 *     cardtest file write=<MB/s> read=<MB/s> verify=1|0 time=<...>
 *     cardtest basis file|raw speed=<MB/s>                   which result the grades follow (file read-back when there is one, else raw)
 *     then the ladder, heaviest format first, 7 formats and 2 marker rows (raw, file) at the place their speed reaches:
 *     cardtest fmt "<format>" need=<MB/s> grade=ok|marginal|slow|unknown      (unknown: no result yet)
 *     cardtest mark raw|file speed=<MB/s>                    speed=0.00: that test has no result
 *   All speeds are MB/s (1 MB = 1,000,000 bytes) with two decimals. Results of this card survive a reboot (saved on the card). */
static void h_cardtest(o_t *o, char *args)
{
    if (!args || !args[0]) { nsn_cardtest_report(o->out, o->ctx); return; }
    if (!strcmp(args, "cancel")) {
        if (nsn_cardtest_cancel()) say(o, "cardtest: cancelled"); else say(o, "cardtest: refused not running");
        return;
    }
    if (strcmp(args, "raw") && strcmp(args, "file")) { say(o, "?"); return; }
    const char *why = nsn_cardtest_start(args);
    if (why) say(o, "cardtest: refused %s", why); else say(o, "cardtest: started %s", args);
}

/* cardfmt: format the TF card. ONLY for a card that answers but whose file system cannot be mounted (card state=unreadable), through three steps with two codes.
 *   cardfmt                 ->  cardfmt plan fs=exFAT|FAT32|FAT16 label="NIGHTSTAND" size=<bytes>
 *                               cardfmt warn everything on the card is erased, including the Wi-Fi settings file
 *                               cardfmt: confirm <CODE1> within 30 s            (LED: red / white alternation)
 *   cardfmt <CODE1>         ->  cardfmt: confirm again <CODE2> within 30 s      (a different code; LED: faster alternation)
 *   cardfmt <CODE2>         ->  cardfmt: started
 *   a wrong, late or misplaced code, or a card that is readable / absent at any step  ->  cardfmt: refused <reason>   and the request is gone
 *   cardfmt cancel          ->  cardfmt: cancelled
 *   cardfmt status          ->  cardfmt state idle|pending|running|done|failed [<reason>]    ("done sim" after the simulation; failed <outcome> <esp error>)
 * Refusals for a card that mounts: "cardfmt: refused the card is readable; formatting is only for a card that cannot be read"; no card: "cardfmt: refused no card".
 * UART-only: `cardfmt selftest` (RAM-disk format tests), `cardfmt sim on|off` (pretend an unreadable 8 MiB card; a confirmed format goes to the RAM disk). */
static void h_cardfmt(o_t *o, char *args)
{
    if (!args || !args[0]) {
        char plan[96]; int code = 0;
        const char *why = nsn_cardfmt_request(plan, sizeof plan, &code);
        if (why) { say(o, "cardfmt: refused %s", why); return; }
        say(o, "cardfmt plan %s", plan);
        say(o, "cardfmt warn everything on the card is erased, including the Wi-Fi settings file");
        say(o, "cardfmt: confirm %04d within 30 s", code);
        return;
    }
    if (!strcmp(args, "status")) { char b[120]; nsn_cardfmt_status(b, sizeof b); say(o, "%s", b); return; }
    if (!strcmp(args, "cancel")) { nsn_cardfmt_cancel(); say(o, "cardfmt: cancelled"); return; }
    if (!strcmp(args, "selftest") || !strncmp(args, "sim", 3)) {
        if (!o->local) { say(o, "?"); return; }
        if (args[0] == 's' && args[1] == 'e') { say(o, "cardfmt selftest %s", nsn_cardfmt_selftest(o->out, o->ctx) ? "ALL PASS" : "FAILED"); return; }
        if (!strcmp(args, "sim on") || !strcmp(args, "sim off")) { nsn_cardfmt_sim(args[5] == 'n'); say(o, "cardfmt sim %s", nsn_cardfmt_sim_on() ? "on" : "off"); return; }
        say(o, "cardfmt sim %s", nsn_cardfmt_sim_on() ? "on" : "off");
        return;
    }
    for (const char *c = args; *c; c++) if (*c < '0' || *c > '9') { say(o, "?"); return; }        /* not a code: not ours */
    int next = 0;
    const char *why = strlen(args) == 4 ? nsn_cardfmt_confirm(atoi(args), &next) : "wrong code";
    if (strlen(args) != 4) nsn_cardfmt_cancel();
    if (why) say(o, "cardfmt: refused %s", why);
    else if (next) say(o, "cardfmt: confirm again %04d within 30 s", next);
    else say(o, "cardfmt: started");
}

/* ---- command table ----------------------------------------------------------------------------------------------------------------
 * uart_only: file / config changes and anything that can reboot or crash the box are for the wired console only: a radio link must
 * never be able to delete or write card files. The dispatcher enforces this in one place. */

typedef struct {
    const char *name;                       /* first word of the line */
    void (*fn)(o_t *o, char *args);
    bool uart_only;
} cmd_t;

static const cmd_t k_cmds[] = {
    { "reboot",  h_reboot,  true  },
    { "st",      h_st,      false },
    { "putx",    h_putx,    true  },
    { "putx!",   h_putxf,   true  },
    { "play",    h_play,    false },
    { "radio",   h_radio,   false },
    { "fx",      h_fx,      false },
    { "fxs",     h_fxs,     false },
    { "night",   h_night,   false },
    { "bedtime", h_bedtime, false },
    { "wake",    h_wake,    false },
    { "mode",    h_mode,    false },
    { "scene",   h_scene,   false },
    { "noise",   h_noise,   false },
    { "stop",    h_stop,    false },
    { "autoplay", h_autoplay, false },
    { "lrc",     h_lrc,     false },
    { "cast",    h_cast,    false },
    { "keys",    h_keys,    false },
    { "events",  h_events,  false },
    { "memwatch", h_memwatch, true },
    { "rg",      h_rg,      false },
    { "next",    h_next,    false },
    { "prev",    h_prev,    false },
    { "timer",   h_timer,   false },
    { "out",     h_out,     false },
    { "pause",   h_pause,   false },
    { "resume",  h_resume,  false },
    { "vol",     h_vol,     false },
    { "vollimit", h_vollimit, false },
    { "gapless", h_gapless, false },
    { "expfmt",  h_expfmt,  false },
    { "dacmax",  h_dacmax,  false },
    { "ratetest", h_ratetest, false },
    { "dbg",     h_dbg,     true  },
    { "spec",    h_spec,    false },
    { "wifi",    h_wifi,    false },
    { "time",    h_time,    false },
    { "led",     h_led,     false },
    { "status",  h_status,  false },
    { "crash",   h_crash,   true  },
    { "mkwav",   h_mkwav,   true  },
    { "rm",      h_rm,      true  },
    { "mkdir",   h_mkdir,   true  },
    { "bt",      h_bt,      false },
    { "ls",      h_ls,      false },
    { "card",    h_card,    false },
    { "cardtest", h_cardtest, false },
    { "cardfmt", h_cardfmt, false },     /* only for an unreadable card, two codes; selftest / sim are UART-only inside the handler */
    { "sdpins",  h_sdpins,  false },     /* read: everyone; setters checked inside the handler */
    { "hw",      h_hw,      false },     /* the same */
    { "ver",     h_ver,     false },
};

static void exec(const char *in, nsn_out_t out, void *ctx, bool local)
{
    char line[400];
    strlcpy(line, in, sizeof line);
    line[strcspn(line, "\r\n")] = 0;
    if (!line[0]) return;

    o_t o = { out, ctx, line, local };
    char *sp = strchr(line, ' ');
    size_t wlen = sp ? (size_t)(sp - line) : strlen(line);
    char *args = sp ? sp + 1 : NULL;

    for (size_t i = 0; i < sizeof k_cmds / sizeof k_cmds[0]; i++) {
        const cmd_t *c = &k_cmds[i];
        if (strlen(c->name) != wlen || strncmp(line, c->name, wlen)) continue;
        if (c->uart_only && !local) break;
        c->fn(&o, args);
        return;
    }
    say(&o, "?");
}

void nsn_cmd_exec(const char *line, nsn_out_t out, void *ctx) { exec(line, out, ctx, false); }
void nsn_cmd_exec_local(const char *line, nsn_out_t out, void *ctx) { exec(line, out, ctx, true); }
