// SPDX-License-Identifier: LGPL-2.1-or-later
#include "ratetest.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cardfmt.h"
#include "card_svc.h"
#include "player.h"
#include "settings.h"
#include "uac_host.h"

static const char *TAG = "ratetest";

/* the sample rates the test knows (what any USB DAC is likely to advertise); a stored verdict takes two bits per rate, in this order */
static const uint32_t k_std[] = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000 };
#define NSTD ((int)(sizeof k_std / sizeof k_std[0]))
#define TEST_MS 1500
#define SETTLE_MS 300
#define TONE_HZ 1000
#define TONE_AMP 1036                    /* -30 dBFS (32767 * 0.0316): the loudest a test ever gets, whatever the volume */

typedef struct { uint32_t rate; bool tested, set_ok; uint32_t clock, xok, xbad, fps; uint8_t verdict; } row_t;
enum { ST_IDLE, ST_RUNNING, ST_DONE, ST_CANCELLED, ST_FAILED };
static const char *const k_state[] = { "idle", "running", "done", "cancelled", "failed" };

static row_t s_row[NSTD];
static int s_nrow;
static volatile int s_state = ST_IDLE;
static volatile bool s_cancel;
static volatile uint32_t s_now;          /* the rate being tested */
static uint32_t s_mask;                  /* stored verdicts of the attached DAC: 2 bits per k_std index */
static uint16_t s_vid, s_pid;
static const char *s_why = "";

static int std_index(uint32_t r) { for (int i = 0; i < NSTD; i++) if (k_std[i] == r) return i; return -1; }
static int mask_get(int i) { return (int)((s_mask >> (2 * i)) & 3); }
static void key_of(char *k, size_t n) { snprintf(k, n, "rt_%04x%04x", s_vid, s_pid); }

/* rows = the standard rates the DAC advertises (plus any it has a stored verdict for), with the stored verdicts and no measurements */
static void build_rows(void)
{
    s_nrow = 0;
    for (int i = 0; i < NSTD; i++) {
        if (!nsn_uac_rate_advertised(k_std[i]) && !mask_get(i)) continue;
        s_row[s_nrow++] = (row_t){ .rate = k_std[i], .verdict = (uint8_t)mask_get(i) };
    }
}

void nsn_ratetest_attach(uint16_t vid, uint16_t pid)
{
    char k[16];
    s_vid = vid; s_pid = pid;
    key_of(k, sizeof k);
    s_mask = (uint32_t)nsn_settings_get_i32(k, 0);
    if (s_state != ST_RUNNING) { s_state = ST_IDLE; build_rows(); }
}

int nsn_ratetest_verdict(uint32_t rate) { int i = std_index(rate); return i < 0 ? NSN_RT_UNTESTED : mask_get(i); }
void nsn_ratetest_debug_set(uint32_t rate, int verdict)
{
    int i = std_index(rate);
    if (i >= 0 && verdict >= 0 && verdict <= 2) s_mask = (s_mask & ~(3u << (2 * i))) | ((uint32_t)verdict << (2 * i));
}
bool nsn_ratetest_any(void) { return s_mask != 0; }
bool nsn_ratetest_running(void) { return s_state == ST_RUNNING; }

/* ---- the tone: a 1024-point table, a 32-bit phase, a 20 ms fade-in; runs in the USB task, integer maths only ---- */
static int16_t s_tab[1024];
static struct { uint32_t phase, step, n, ramp; int amp; } s_tone;

static size_t tone_pull(int16_t *dst, size_t frames, void *ctx)
{
    (void)ctx;
    for (size_t i = 0; i < frames; i++) {
        int v = s_tab[s_tone.phase >> 22];
        if (s_tone.n < s_tone.ramp) v = (int)((int64_t)v * s_tone.n / s_tone.ramp);
        s_tone.n++;
        s_tone.phase += s_tone.step;
        dst[2 * i] = dst[2 * i + 1] = (int16_t)v;
    }
    return frames;
}

/* the level: -30 dBFS at most, times the volume the player would use (cubic like the player's own gain, never above its limit) */
static int tone_amp(void)
{
    float x = nsn_player_volume() / 100.0f;
    float g = x * x * x;
    if (g > 1.0f) g = 1.0f;
    return (int)(TONE_AMP * g);
}

static bool sleep_or_stop(int ms)             /* false: cancelled or the DAC went away */
{
    for (int t = 0; t < ms; t += 50) {
        if (s_cancel) return false;
        if (!nsn_uac_info_present()) { s_why = "dac gone"; return false; }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    return true;
}

static bool run_one(row_t *w)
{
    uint32_t r = w->rate;
    s_now = r;
    s_tone.phase = 0; s_tone.n = 0; s_tone.ramp = r / 50; s_tone.step = (uint32_t)(((uint64_t)TONE_HZ << 32) / r);
    for (int i = 0; i < 1024; i++) s_tab[i] = (int16_t)(tone_amp() * sinf(6.2831853f * (float)i / 1024.0f));
    *w = (row_t){ .rate = r, .tested = true };
    esp_err_t e = nsn_uac_play(r, tone_pull, NULL);
    nsn_uac_last_clock(&w->set_ok, &w->clock);
    if (e != ESP_OK) { w->verdict = NSN_RT_BAD; ESP_LOGW(TAG, "%lu Hz: start failed (%s)", (unsigned long)r, esp_err_to_name(e)); return nsn_uac_info_present(); }
    bool live = sleep_or_stop(SETTLE_MS);
    uint32_t d0, b0, f0, d1 = 0, b1 = 0, f1 = 0; int64_t t0 = esp_timer_get_time(), t1 = t0;
    nsn_uac_stats(&d0, &b0); f0 = nsn_uac_frames();
    if (live) live = sleep_or_stop(TEST_MS);
    nsn_uac_stats(&d1, &b1); f1 = nsn_uac_frames(); t1 = esp_timer_get_time();
    nsn_uac_stop();
    if (!live) { *w = (row_t){ .rate = r }; return nsn_uac_info_present() && !s_cancel; }
    w->xok = (d1 - d0) - (b1 - b0); w->xbad = b1 - b0;
    w->fps = t1 > t0 ? (uint32_t)((uint64_t)(f1 - f0) * 1000000 / (uint64_t)(t1 - t0)) : 0;
    uint32_t dev = w->fps > r ? w->fps - r : r - w->fps;
    w->verdict = (w->set_ok && w->clock == r && w->xok > 0 && w->xbad == 0 && dev * 100 <= r) ? NSN_RT_OK : NSN_RT_BAD;
    ESP_LOGI(TAG, "%lu Hz: set %d clock %lu xfers %lu ok %lu bad, %lu frames/s -> %s", (unsigned long)r, w->set_ok, (unsigned long)w->clock,
             (unsigned long)w->xok, (unsigned long)w->xbad, (unsigned long)w->fps, w->verdict == NSN_RT_OK ? "ok" : "bad");
    return true;
}

static void worker(void *arg)
{
    (void)arg;
    nsn_player_stop();                                       /* nothing may be streaming to the DAC */
    bool alive = true;
    for (int i = 0; i < s_nrow && alive && !s_cancel; i++) {
        alive = run_one(&s_row[i]);
        if (alive && !sleep_or_stop(150)) alive = false;
    }
    s_now = 0;
    char k[16]; key_of(k, sizeof k);
    for (int i = 0; i < s_nrow; i++) {                       /* keep what was measured (verdicts of rates not reached stay as they were) */
        int x = std_index(s_row[i].rate);
        if (x >= 0 && s_row[i].tested) s_mask = (s_mask & ~(3u << (2 * x))) | ((uint32_t)s_row[i].verdict << (2 * x));
    }
    nsn_settings_set_i32(k, (int)s_mask);                    /* this task has an internal stack: NVS is allowed */
    s_state = s_cancel ? ST_CANCELLED : alive ? ST_DONE : ST_FAILED;
    ESP_LOGI(TAG, "%s", k_state[s_state]);
    vTaskDelete(NULL);
}

const char *nsn_ratetest_start(void)
{
    nsn_uac_info_t u; nsn_uac_info(&u);
    if (!u.present) return "no dac";
    if (s_state == ST_RUNNING) return "already running";
    if (nsn_cardtest_running()) return "card test running";
    if (nsn_cardfmt_state() == NSN_FMT_RUNNING) return "formatting";
    if (!nsn_uac_rates_known()) return "the dac advertises no sample rates";
    s_vid = u.vid; s_pid = u.pid;
    build_rows();
    if (!s_nrow) return "no known rate advertised";
    s_cancel = false; s_why = ""; s_state = ST_RUNNING;
    if (xTaskCreate(worker, "ratetest", 4608, NULL, 4, NULL) != pdPASS) { s_state = ST_IDLE; return "no memory"; }
    return NULL;
}

bool nsn_ratetest_cancel(void)
{
    if (s_state != ST_RUNNING) return false;
    s_cancel = true;
    return true;
}

static const char *vword(const row_t *w) { return w->verdict == NSN_RT_OK ? "ok" : w->verdict == NSN_RT_BAD ? "bad" : "untested"; }

void nsn_ratetest_report(nsn_out_t out, void *ctx)
{
    char b[160];
    nsn_uac_info_t u; nsn_uac_info(&u);
    int n = snprintf(b, sizeof b, "ratetest state=%s dac=%04x:%04x", k_state[s_state], s_vid, s_pid);
    if (s_state == ST_RUNNING) snprintf(b + n, sizeof b - n, " now=%lu", (unsigned long)s_now);
    else if (s_why[0]) snprintf(b + n, sizeof b - n, " reason=\"%s\"", s_why);
    out(ctx, b);
    uint32_t top = 0;
    for (int i = 0; i < s_nrow; i++) {
        const row_t *w = &s_row[i];
        snprintf(b, sizeof b, "ratetest rate=%lu set=%s clock=%lu xfer_ok=%lu xfer_bad=%lu fps=%lu verdict=%s", (unsigned long)w->rate,
                 w->tested ? (w->set_ok ? "ok" : "fail") : "na", (unsigned long)w->clock, (unsigned long)w->xok, (unsigned long)w->xbad, (unsigned long)w->fps, vword(w));
        out(ctx, b);
        if (w->verdict == NSN_RT_OK && w->rate > top) top = w->rate;
    }
    if (s_state != ST_RUNNING && (top || s_state == ST_DONE)) { snprintf(b, sizeof b, "ratetest max=%lu", (unsigned long)top); out(ctx, b); }
}
