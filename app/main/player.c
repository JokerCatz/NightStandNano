// SPDX-License-Identifier: LGPL-2.1-or-later
#include "player.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/ringbuf.h"
#include "pc/pc_backend.h"
#include "pc/pc_io.h"
#include "uac_host.h"
#include "bt_audio.h"
#include "pc/pc_rsq.h"
#include "spectrum.h"
#include "settings.h"
#include "radio.h"
#include "pc/pc_noisegen.h"
#include "pc/pc_chipseq.h"
#include "pc/pc_zip.h"
#include "backend_gme.h"
#include "backend_sdec.h"
#include "pc/pc_id3.h"
#include "pc/pc_rg.h"
#include "cast.h"
#include "sink.h"
#include "board/board.h"
#include "scene.h"
#include "fx.h"
#include "pc/pc_volsel.h"
#include "outcheck.h"
#include "zipfs.h"
#include "trim.h"
#include "ratetest.h"
#include "pc/pc_lowrate.h"
#include "pc/pc_dacmax.h"

static const char *TAG = "player";
#define RING_BYTES (96 * 1024)           /* ~0.5 s of 48 kHz stereo S16, in PSRAM */
#define BLOCK      1024                  /* frames per decode call */
#define MAX_TRACKS 300
#define NAME_MAX_  160                   /* a playlist entry; a path inside a zip archive is "<zip path>/<entry path>" */
#define FADE_S     30

static RingbufHandle_t s_ring;
static TaskHandle_t s_task;
static char s_path[NAME_MAX_];
static volatile uint32_t s_dacmax;           /* user ceiling for the USB DAC in Hz, 0 = auto (whatever the DAC takes); NVS "dacmax" */
static volatile bool s_stop, s_active, s_eof, s_paused;
static uint32_t s_rate;
static volatile uint32_t s_underruns;
static volatile bool s_gap = true;           /* between tracks (opening, after a skip): an empty ring is expected, not an underrun */
static volatile int32_t s_pos_ms, s_len_ms;
static const char *s_fmt = "";
static volatile int s_vol = 70;              /* the volume in force (the playing output's, below the limit) */
static volatile float s_gain = 0.343f;       /* 0.7^3 */
static char (*s_list)[NAME_MAX_];
static int s_n, s_idx;
static bool s_repeat;
static volatile int s_mode;                  /* g_set.play_mode copy */
static int s_hist[20], s_hn;                 /* shuffle history (indices), for "previous" */
static volatile int s_sub, s_subn;           /* tune N of M inside a multi-tune file (NSF, SID, ...) */
static volatile int s_skip;                  /* +1 next, -1 previous, 0 none */
static volatile int64_t s_sleep_deadline_us; /* 0 = no timer */
static volatile int s_out_pref;              /* NSN_OUT_AUTO, or the id of the wanted sink (sink.h) */
static int s_sink;                           /* id of the sink consuming the ring now, 0 = none */
static uint32_t s_link_rate;                 /* rate of that link; tracks are resampled to it when they differ */
static pc_rsq_t *s_rsq;                      /* polyphase resampler state, PSRAM (JukeDial's pc_rsq) */

/* ---- volume rules -------------------------------------------------------------------------------------------------------------
 * One volume per output (the sinks of sink.h), a limit that no path may exceed, a soft start, a safe first volume for headphones.
 * s_ovol[o] is the stored volume of sink id o+1, -1 = never stored. The DAC's is g_set.vol itself (what older settings hold). The decode task must not
 * write NVS: setters only mark s_dirty, nsn_player_persist() (command task, main loop) writes. */
#define SAFE_START 40                        /* headphones / broadcast that never had a volume of their own start at most here */
#define SOFT_MS 400
static volatile int s_ovol[NSN_SINK_MAX] = { 70, -1, -1, -1, -1, -1 };
static volatile int s_vmax = 100;
static volatile bool s_dirty;
static volatile int s_vout;                  /* id of the sink whose volume is in force while a session runs; 0 = idle */
static volatile bool s_gapless = true;
static volatile bool s_soft_pending;         /* a session just started: the first sink start arms the ramp */
static volatile uint32_t s_soft_left, s_soft_total;   /* soft start: output frames still to ramp / the ramp's length */
static volatile uint32_t s_wr_frames;        /* frames written into the ring this session (output units): track boundaries are reported in them */
static int16_t s_ramp_pk[12]; static volatile uint32_t s_ramp_n;   /* debug: output peak per 1/8 ramp, from the ramp's start */

/* "noise:<colour>" (white pink brown blue violet grey green velvet) as a source: JukeChip's generators, endless, 44.1 kHz */
static int noise_open(void **h, const pc_reader_t *r, uint32_t off)
{
    pc_noisegen_t *g = heap_caps_calloc(1, sizeof *g + 8, MALLOC_CAP_SPIRAM);
    if (!g) return -1;
    int c = (int)(intptr_t)r->u;
    pc_noisegen_start(g, c);
    *h = g;
    return 0;
}
static uint32_t s_noise_frames;
static int noise_render(void *h, float *out, int frames) { pc_noisegen_render(h, out, frames); s_noise_frames += frames; return frames; }
static uint32_t noise_rate(void *h) { return PC_NOISE_RATE; }
static int32_t noise_len(void *h) { return -1; }
static int32_t noise_pos(void *h) { return (int32_t)((uint64_t)s_noise_frames * 1000 / PC_NOISE_RATE); }
static void noise_close(void *h) { heap_caps_free(h); }
static const pc_backend_t noise_backend = { .open = noise_open, .render = noise_render, .rate = noise_rate, .length_ms = noise_len, .position_ms = noise_pos,
                                            .close = noise_close, .name = "Noise" };
/* "bt:in": what a phone streams to us (speaker mode). Live: paced by the phone, ~100 ms cushion, drift trimmed by dropping the excess. */
static bool s_btin_primed;
static int btin_open(void **h, const pc_reader_t *r, uint32_t off) { nsn_bt_in_flush(); s_btin_primed = false; *h = (void *)1; return 0; }
static int btin_render(void *h, float *out, int frames)
{
    static int16_t tmp[1024 * 2];
    uint32_t rate = nsn_bt_in_rate();
    size_t cushion = rate / 10;                                      /* 100 ms */
    if (frames > 1024) frames = 1024;
    if (!s_btin_primed) {
        for (int i = 0; i < 100 && nsn_bt_in_fill() < cushion && nsn_bt_in_streaming(); i++) vTaskDelay(pdMS_TO_TICKS(5));
        if (nsn_bt_in_fill() >= cushion) s_btin_primed = true;
        else { memset(out, 0, sizeof(float) * 2 * frames); return frames; }
    }
    while (nsn_bt_in_fill() > rate / 4) nsn_bt_in_read(tmp, 256);     /* clock drift: more than 250 ms waiting, drop down to the cushion */
    size_t got = 0;
    for (int i = 0; i < 10 && got < (size_t)frames; i++) {            /* wait up to ~20 ms for a full block, then pad (a real underrun) */
        got += nsn_bt_in_read(tmp + got * 2, frames - got);
        if (got < (size_t)frames) vTaskDelay(1);
    }
    for (size_t i = 0; i < got * 2; i++) out[i] = tmp[i] / 32768.0f;
    if (got < (size_t)frames) memset(out + got * 2, 0, sizeof(float) * 2 * (frames - got));
    if (!nsn_bt_in_streaming() && got == 0) return 0;                 /* the phone paused / left: end the track */
    return frames;
}
static uint32_t btin_rate(void *h) { return nsn_bt_in_rate(); }
static int32_t btin_len(void *h) { return -1; }
static int32_t btin_pos(void *h) { return (int32_t)(esp_timer_get_time() / 1000); }
static void btin_close(void *h) { }
static const pc_backend_t btin_backend = { .open = btin_open, .render = btin_render, .rate = btin_rate, .length_ms = btin_len, .position_ms = btin_pos, .close = btin_close, .name = "Phone" };

static const char *const k_colors[PC_NOISE_COLORS] = { "white", "pink", "brown", "blue", "violet", "grey", "green", "velvet" };
static int noise_color(const char *path)
{
    if (strncmp(path, "noise:", 6)) return -1;
    for (int i = 0; i < PC_NOISE_COLORS; i++) if (!strcasecmp(path + 6, k_colors[i])) return i;
    return -1;
}

static const pc_backend_t *backend_for(const char *path)
{
    if (!strcmp(path, "bt:in")) return &btin_backend;
    if (!strncmp(path, "scene:", 6)) return &nsn_scene_backend;
    if (!strncmp(path, "noise:", 6)) return noise_color(path) >= 0 ? &noise_backend : NULL;
    const char *e = strrchr(path, '.');
    if (!e) return NULL;
    if (!strcasecmp(e, ".ym")) return &pc_backend_ym;
    if (!strcasecmp(e, ".p8")) return &pc_backend_p8;
    if (pc_opl_ext(e) || pc_xmp_ext(e)) return &pc_backend_chain;      /* tracker modules and DOS AdLib music: libxmp / AdPlug, in the order pc_chipseq.c says */
    if (!strcasecmp(e, ".nsf") || !strcasecmp(e, ".nsfe") || !strcasecmp(e, ".gbs") || !strcasecmp(e, ".gym") || !strcasecmp(e, ".hes") || !strcasecmp(e, ".kss") ||
        !strcasecmp(e, ".sap") || !strcasecmp(e, ".spc") || !strcasecmp(e, ".vgm") || !strcasecmp(e, ".vgz") || !strcasecmp(e, ".ay")) return &backend_gme;
    if (!strcasecmp(e, ".adx")) return &pc_backend_adx;                 /* game audio: CRI ADX, PlayStation VAG, Nintendo BRSTM (round's backends) */
    if (!strcasecmp(e, ".vag")) return &pc_backend_vag;
    if (!strcasecmp(e, ".brstm")) return &pc_backend_brstm;
    if (!strcasecmp(e, ".mp3")) return &pc_backend_mp3;
    if (!strcasecmp(e, ".aac")) return &backend_aac;
    if (!strcasecmp(e, ".ogg") || !strcasecmp(e, ".oga") || !strcasecmp(e, ".opus")) return &backend_ogg_trim;     /* exact length from the container (trim.c) */
    if (!strcasecmp(e, ".m4a") || !strcasecmp(e, ".mp4")) return &backend_m4a_trim;
    if (!strcasecmp(e, ".flac")) return &pc_backend_flac;
    if (!strcasecmp(e, ".wav")) return &pc_backend_wav;
    if (!strcasecmp(e, ".aif") || !strcasecmp(e, ".aiff")) return &pc_backend_aiff;
    if (!strcasecmp(e, ".mpc")) return &pc_backend_mpc;
    if (!strcasecmp(e, ".tta")) return &pc_backend_tta;
    if (!strcasecmp(e, ".ahx") || !strcasecmp(e, ".hvl")) return &pc_backend_hvl;
    if (!strcasecmp(e, ".sid")) return &pc_backend_sid;
    return NULL;
}

/* The rates the player used on the DAC before the rate test existed: kept as is for a DAC that was never tested. */
static bool legacy_rate(uint32_t r)
{
    return r == 44100 || r == 48000 || r == 96000 || r == 192000 || r == 32000 || r == 16000 || r == 8000 || r == 384000;
}

/* May the DAC be run at r? Untested DAC: today's list. Tested DAC (ratetest): the rate must be advertised, not known bad, and either measured good or on
 * today's list (a rate the test measured good is allowed even when it is not on the list: 88.2 / 176.4 kHz). */
static bool dac_rate_ok(uint32_t r)
{
    if (!nsn_ratetest_any()) return legacy_rate(r);
    int v = nsn_ratetest_verdict(r);
    if (v == NSN_RT_BAD || !nsn_uac_rate_advertised(r)) return false;
    return v == NSN_RT_OK || legacy_rate(r);
}

/* The rate the DAC runs at for a source at `src`: the source's own when the DAC may run at it and it is under the user's ceiling (dacmax);
 * otherwise JukeDial's rules: an exact halving first (pc_pipe_plan: 192 -> 96 kHz), then pc_dac_pick_rate over the rates the DAC may run at (the highest at or
 * below the source, the same 44.1 / 48 family first). A source that was never a DAC rate here (22.05 kHz, ...) keeps the old 48 kHz target. */
static uint32_t dac_link_rate(uint32_t src)
{
    uint32_t ceil = s_dacmax ? s_dacmax : 384000;
    if (src <= ceil && dac_rate_ok(src)) return src;
    if (src <= ceil && !legacy_rate(src)) return 48000;
    uint32_t lim = src <= ceil ? src - 1 : ceil;                   /* the ceiling, or just below a source the DAC may not run at */
    for (int k = 0; k < 2; k++) {                                  /* the exact halving (at most 4:1, the resampler's ratio) */
        pc_pipe_t p = pc_pipe_plan(src, lim, 0, PC_PIPE_CAP);
        if (!p.pipe || (uint64_t)src > 4ull * p.pipe) break;
        if (dac_rate_ok(p.pipe)) return p.pipe;
        lim = p.pipe - 1;
    }
    static const uint32_t std[] = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000 };
    uint32_t list[sizeof std / sizeof std[0]]; uint8_t n = 0;
    for (unsigned i = 0; i < sizeof std / sizeof std[0]; i++) if (std[i] <= ceil && dac_rate_ok(std[i])) list[n++] = (uint32_t)std[i];
    const pc_dac_caps_t caps = { .list = list, .n = n, .max = 384000 };
    uint32_t t = pc_dac_pick_rate(&caps, src, ceil);
    return t ? t : 48000;
}

static void soft_arm(uint32_t link_rate)
{
    s_soft_total = link_rate / 1000 * SOFT_MS; s_soft_left = s_soft_total;
    s_ramp_n = 0; memset(s_ramp_pk, 0, sizeof s_ramp_pk);
}

/* Soft start: the first SOFT_MS of what the sink takes rise linearly from silence (a restart from pause, a new session). Done on the output side
 * because the ring already holds audio at full gain when playback resumes. */
static void soft_apply(int16_t *dst, size_t frames)
{
    uint32_t total = s_soft_total, left = s_soft_left;
    for (size_t i = 0; i < frames && left; i++, left--) {
        float k = (float)(total - left) / (float)total;
        dst[2 * i] = (int16_t)(dst[2 * i] * k); dst[2 * i + 1] = (int16_t)(dst[2 * i + 1] * k);
        uint32_t b = s_ramp_n++ / (total / 8 + 1);
        if (b < 12) { int a = dst[2 * i] < 0 ? -dst[2 * i] : dst[2 * i]; if (a > s_ramp_pk[b]) s_ramp_pk[b] = (int16_t)a; }
    }
    s_soft_left = left;
}

/* USB task: take what is there, silence for the rest (and count it). A byte ring returns at most up to its end, so a read that
 * wraps needs a second call for the rest; counting that as an underrun (and zero-filling it) was a click every ring revolution. */
static size_t pull(int16_t *dst, size_t frames, void *ctx)
{
    size_t want = frames * 4, got = 0;
    if (!s_ring || s_paused || !s_active) { nsn_oc_idle(); return 0; }
    while (got < want) {
        size_t n = 0;
        uint8_t *p = xRingbufferReceiveUpTo(s_ring, &n, 0, want - got);
        if (!p) break;
        memcpy((uint8_t *)dst + got, p, n);
        vRingbufferReturnItem(s_ring, p);
        got += n;
    }
    if (got < want && s_active && !s_eof && !s_gap) s_underruns++;
    if (s_soft_left && got) soft_apply(dst, got / 4);
    if (got) { nsn_spec_push(dst, got / 4); nsn_oc_feed(dst, got / 4); }
    if (got < want && !s_eof) nsn_oc_starve((want - got) / 4);      /* what the sink pads with silence, whether or not it is an "underrun" by the rules above */
    return got / 4;
}

static void drain_ring(void)
{
    void *junk; size_t n;
    while (s_ring && (junk = xRingbufferReceiveUpTo(s_ring, &n, 0, RING_BYTES))) vRingbufferReturnItem(s_ring, junk);
}

static bool ring_empty(void) { return xRingbufferGetCurFreeSize(s_ring) >= RING_BYTES - 64; }

/* 1 when the sleep timer has run out, else the fade factor 0..1 for now */
static float sleep_fade(bool *over)
{
    *over = false;
    if (!s_sleep_deadline_us) return 1.0f;
    int64_t left = s_sleep_deadline_us - esp_timer_get_time();
    if (left <= 0) { *over = true; return 0.0f; }
    float s = left / 1e6f;
    return s >= FADE_S ? 1.0f : s / FADE_S;
}

/* ---- the three built-in outputs, as sinks (sink.h). Each is a thin adapter to its module; ids 1..3 are fixed ---- */
static bool dac_present(void) { return nsn_uac_info_present(); }
static uint32_t dac_rate_for(uint32_t src) { return dac_link_rate(src); }
static void dac_describe(char *b, size_t n)
{
    if (s_dacmax) snprintf(b, n, ", dacmax %lu", (unsigned long)s_dacmax); else snprintf(b, n, ", dacmax auto");
}
static esp_err_t dac_start(uint32_t rate, nsn_sink_pull_t cb, void *ctx) { return nsn_uac_play(rate, cb, ctx); }
static void dac_stop(void) { nsn_uac_stop(); }

static bool bt_present(void) { return nsn_bt_connected(); }
static uint32_t bt_rate_for(uint32_t src) { return nsn_bt_rate(); }       /* the negotiated A2DP rate, whatever the source */
static esp_err_t bt_start(uint32_t rate, nsn_sink_pull_t cb, void *ctx) { nsn_bt_set_source(cb, ctx); return nsn_bt_start(); }
static void bt_stop(void) { nsn_bt_suspend(); nsn_bt_set_source(NULL, NULL); }

static bool cast_present(void) { return nsn_cast_on(); }
static uint32_t cast_rate_for(uint32_t src) { return nsn_cast_rate(); }
static esp_err_t cast_start(uint32_t rate, nsn_sink_pull_t cb, void *ctx) { nsn_cast_set_source(cb, ctx); return ESP_OK; }
static void cast_stop(void) { nsn_cast_set_source(NULL, NULL); }          /* the broadcast goes on, silent */

static const nsn_sink_t k_sink_dac = { .name = "dac", .label = "DAC", .desc = "USB DAC", .vol_key = NULL, .auto_rank = 10, .gone_msg = "USB DAC gone",
    .present = dac_present, .link_rate = dac_rate_for, .describe = dac_describe, .start = dac_start, .stop = dac_stop };
static const nsn_sink_t k_sink_bt = { .name = "bt", .label = "BT", .desc = "Bluetooth", .vol_key = "vol_bt", .auto_rank = 30, .resume_after_drop = true,
    .gone_msg = "Bluetooth output gone", .present = bt_present, .link_rate = bt_rate_for, .start = bt_start, .stop = bt_stop };
static const nsn_sink_t k_sink_cast = { .name = "cast", .label = "Auracast", .desc = "Auracast", .vol_key = "vol_cast", .auto_rank = 0,
    .gone_msg = "broadcast stopped", .present = cast_present, .link_rate = cast_rate_for, .start = cast_start, .stop = cast_stop };

/* The sink a session would use now: the chosen one if it is present, `auto` = the best present one (sink.h auto_rank). 0 = none. */
static int pick_sink(void)
{
    const nsn_sink_t *k = nsn_sink_get(s_out_pref);
    if (!k) return nsn_sink_auto();
    return k->present() ? s_out_pref : 0;
}

static void stop_sink(void)
{
    const nsn_sink_t *k = nsn_sink_get(s_sink);
    if (k) k->stop();
    s_sink = 0; s_link_rate = 0;
}

static bool start_sink(int sink, uint32_t link_rate)
{
    if (s_soft_pending) { s_soft_pending = false; soft_arm(link_rate); }       /* before the sink can pull its first block */
    if (nsn_sink_get(sink)->start(link_rate, pull, NULL) != ESP_OK) return false;
    s_sink = sink; s_link_rate = link_rate;
    return true;
}

static char s_drop_path[160]; static int64_t s_drop_us;
/* The track that was playing when the Bluetooth output dropped, if that was under 10 minutes ago; cleared when read. */
bool nsn_player_take_dropped(char *out, size_t n)
{
    if (!s_drop_path[0]) return false;
    bool fresh = esp_timer_get_time() - s_drop_us < 600LL * 1000000;
    if (fresh) snprintf(out, n, "%s", s_drop_path);
    s_drop_path[0] = 0;
    return fresh;
}

/* Bookmark: where a card file was when playback was stopped (stop, sleep timer, output gone), so `resume` can go on from there.
 * Kept in RAM here; the main task stores it in NVS (this task's stack is in PSRAM and must not write flash). */
static char s_bm_path[160]; static uint32_t s_bm_pos; static volatile bool s_bm_dirty;
static char s_tag_title[128], s_tag_artist[96], s_tag_album[128];   /* tags of the playing card file ("" when none) */
static float s_rg = 1.0f;                    /* ReplayGain factor of the playing file (1 = none / off) */
static volatile int s_rg_mode;               /* pc_rg_mode_t: 0 off, 1 track, 2 album */
void nsn_player_set_rg(int m) { s_rg_mode = m < 0 || m > 2 ? 0 : m; }
int nsn_player_rg(void) { return s_rg_mode; }
static uint32_t s_start_off;                 /* one-shot: where the next file opens */
void nsn_player_set_bookmark(const char *path, uint32_t pos) { snprintf(s_bm_path, sizeof s_bm_path, "%s", path ? path : ""); s_bm_pos = pos; }
bool nsn_player_bookmark_dirty(char *path, size_t n, uint32_t *pos)
{
    if (!s_bm_dirty) return false;
    s_bm_dirty = false;
    snprintf(path, n, "%s", s_bm_path); *pos = s_bm_pos;
    return true;
}
const char *nsn_player_bookmark(void) { return s_bm_path; }

/* One open track: the decoder and what it reads from. r must outlive h (the backends keep a pointer to it). */
typedef struct { const pc_backend_t *be; void *h; FILE *f; uint8_t *mem; pc_reader_t r; bool is_url, card; uint32_t rate; } trk_t;   /* card: a file of the card or a zip entry played like one (bookmark) */

static void close_track(trk_t *t)
{
    if (t->h) t->be->close(t->h);
    if (t->f) fclose(t->f);
    if (t->is_url) nsn_radio_end();
    heap_caps_free(t->mem);
    memset(t, 0, sizeof *t);
}

/* Opens the source of `path` (card file, station, scene, noise, phone) and fills the "now playing" fields: tags, ReplayGain, format, length.
 * 0 ok; -1 could not (nothing is left open). */
static volatile int s_dbg_open_delay_ms;    /* debug: pretend card files open slowly (big cover art, slow card) */
void nsn_player_debug_opendelay(int ms) { s_dbg_open_delay_ms = ms > 0 ? ms : 0; }

static int open_track(const char *path, trk_t *t)
{
    memset(t, 0, sizeof *t);
    if (s_dbg_open_delay_ms && s_active) vTaskDelay(pdMS_TO_TICKS(s_dbg_open_delay_ms));
    const pc_backend_t *be = backend_for(path);
    s_tag_title[0] = s_tag_artist[0] = s_tag_album[0] = 0; s_rg = 1.0f; s_fmt = "";
    bool is_url = !strncmp(path, "http://", 7) || !strncmp(path, "https://", 8);
    if (!is_url) nsn_radio_end();                           /* a refusal text of the last station (HLS: encrypted ...) does not outlive it */
    char zpath[NAME_MAX_]; const char *inner = NULL;
    bool zip = !is_url && path[0] == '/' && nsn_zip_split(path, zpath, sizeof zpath, &inner);     /* "<archive>.zip/<entry>" */
    pc_reader_t *r = &t->r;
    void *h = NULL;
    if (is_url) {
        s_fmt = "Radio";
        if (!nsn_radio_begin(path, &be, r)) { ESP_LOGE(TAG, "radio: no stream"); return -1; }
        t->be = be; t->is_url = true;
        if (be->open(&h, r, 0) != 0) { ESP_LOGE(TAG, "radio: decoder open failed"); close_track(t); return -1; }
    } else if (be == &btin_backend) {
        s_fmt = "Phone";
        if (be->open(&h, r, 0) != 0) return -1;
    } else if (be == &nsn_scene_backend) {
        r->u = (void *)(intptr_t)(atoi(path + 6) - 1);
        s_fmt = "Scene";
        if (be->open(&h, r, 0) != 0) { ESP_LOGE(TAG, "scene: cannot open"); return -1; }
    } else if (be == &noise_backend) {
        r->u = (void *)(intptr_t)noise_color(path);
        s_noise_frames = 0; s_fmt = "Noise";
        if (be->open(&h, r, 0) != 0) return -1;
    } else if (be == &backend_gme || be == &pc_backend_chain || be == &pc_backend_ym || be == &pc_backend_p8) {
        uint8_t *mem = NULL; uint32_t len = 0;
        if (zip) {
            int rc = nsn_zip_read(zpath, inner, NSN_ZIP_MAX_ENTRY, &mem, &len);
            if (rc) { ESP_LOGE(TAG, "zip entry %s refused: %s", path, nsn_zip_err(rc)); return -1; }
        } else {
            FILE *f = fopen(path, "rb");
            if (!f) { ESP_LOGE(TAG, "cannot open %s", path); return -1; }
            fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
            if (sz <= 0 || sz > 8 * 1024 * 1024) { ESP_LOGE(TAG, "chip file size %ld refused", sz); fclose(f); return -1; }
            mem = heap_caps_malloc((size_t)sz, MALLOC_CAP_SPIRAM);
            if (!mem || fread(mem, 1, (size_t)sz, f) != (size_t)sz) { ESP_LOGE(TAG, "cannot read %s", path); heap_caps_free(mem); fclose(f); return -1; }
            fclose(f);
            len = (uint32_t)sz;
        }
        const char *ext = strrchr(path, '.');
        if (ext && !strcasecmp(ext, ".vgz")) {                              /* gzip'd VGM */
            uint8_t *raw = NULL; uint32_t rl = 0;
            if (pc_gunzip(mem, len, &raw, &rl) != 0 || !raw) { ESP_LOGE(TAG, "gunzip failed: %s", path); heap_caps_free(mem); return -1; }
            heap_caps_free(mem); mem = raw; len = rl;
        }
        t->mem = mem;
        pc_reader_mem(r, mem, len);
        pc_reader_set_ext(r, path);
        if (be->open(&h, r, 0) != 0) { ESP_LOGE(TAG, "decoder open failed: %s", path); close_track(t); return -1; }
    } else {
        if (!be) { ESP_LOGE(TAG, "no decoder for %s", path); return -1; }
        if (zip) {                                                          /* a zip entry: extracted into PSRAM, read from memory like a chip file */
            uint8_t *mem = NULL; uint32_t len = 0;
            int rc = nsn_zip_read(zpath, inner, NSN_ZIP_MAX_ENTRY, &mem, &len);
            if (rc) { ESP_LOGE(TAG, "zip entry %s refused: %s", path, nsn_zip_err(rc)); return -1; }
            t->mem = mem;
            pc_reader_mem(r, mem, len);
        } else {
            FILE *f = fopen(path, "rb");
            if (!f || pc_reader_file(r, f) != 0) { ESP_LOGE(TAG, "cannot open %s", path); if (f) fclose(f); return -1; }
            t->f = f;
        }
        t->card = true;
        pc_reader_set_ext(r, path);
        {   /* tags: title / artist / album for the page, ReplayGain for the level (round's parser: ID3, FLAC, Ogg, MP4) */
            pc_id3_t *tag = heap_caps_calloc(1, sizeof *tag, MALLOC_CAP_SPIRAM);
            if (tag && pc_id3_parse(r, tag) == 0) {
                strlcpy(s_tag_title, tag->title, sizeof s_tag_title); strlcpy(s_tag_artist, tag->artist, sizeof s_tag_artist); strlcpy(s_tag_album, tag->album, sizeof s_tag_album);
                float db = 0; s_rg = pc_rg_linear(&tag->rg, s_rg_mode, 0.0f, &db);
                if (s_rg_mode && db != 0) ESP_LOGI(TAG, "ReplayGain %+.1f dB", (double)db);
            }
            if (tag) { pc_id3_free(tag); heap_caps_free(tag); }
        }
        uint32_t off = be->byte_pos ? s_start_off : 0;
        s_start_off = 0;
        t->be = be;
        if (be->open(&h, r, off) != 0 && (off == 0 || be->open(&h, r, 0) != 0)) { ESP_LOGE(TAG, "decoder open failed: %s", path); close_track(t); return -1; }
        if (off) ESP_LOGI(TAG, "resuming at token %lu", (unsigned long)off);
    }
    t->be = be; t->h = h;
    const pc_backend_t *real = pc_backend_resolve(be, h);
    if (!is_url && be != &noise_backend && be != &nsn_scene_backend && be != &btin_backend) s_fmt = (real && real->name) ? real->name : "?";
    t->rate = be->rate(h);
    s_len_ms = be->length_ms ? be->length_ms(h) : -1; s_pos_ms = 0;
    return 0;
}

/* the playlist's next index (shuffle history included). false: the list is over. */
static bool advance(bool manual, int dir)
{
    int next;
    if (s_n > 1 && s_mode == 1) {                                  /* shuffle: a random other track; previous walks the history back */
        if (dir < 0 && s_hn > 0) next = s_hist[--s_hn];
        else {
            if (s_hn == 20) { memmove(s_hist, s_hist + 1, sizeof s_hist - sizeof s_hist[0]); s_hn--; }
            s_hist[s_hn++] = s_idx;
            do next = (int)(esp_random() % (uint32_t)s_n); while (next == s_idx);
        }
    } else if (s_mode == 2 && !manual) next = s_idx;               /* repeat one */
    else {
        next = s_idx + dir;
        if (next < 0) next = s_n - 1;
        if (next >= s_n) { if (!s_repeat || s_mode == 3) return false; next = 0; }
    }
    if (s_mode == 3 && !manual && next <= s_idx && s_n > 1) return false;
    s_idx = next;
    return true;
}

/* play_one's word to run_session about where the playlist stands when the track ended by itself */
enum { ADV_NONE, ADV_NEXT, ADV_END };      /* NEXT: s_idx already points at the track to play next; END: the list is over */
static int s_adv;

/* The track ended by itself. When the next one can follow on the same output at the same rate, switch to it inside the running session:
 * the ring is still full of the old track's tail (the decoder runs a ring ahead of the output), so opening the next file costs no gap, and
 * the sink, the resampler and the ring stay as they are. Samples are neither dropped nor added; MP3 encoder delay / padding are trimmed by
 * the backend (pc_backend_mp3, LAME/Xing tag). false: not chained (s_adv says what to do next). */
static bool try_chain(trk_t *t, int sink)
{
    bool special = t->is_url || t->be == &btin_backend || t->be == &noise_backend || t->be == &nsn_scene_backend;
    if (!s_gapless || s_stop || s_skip || special || !s_n) return false;
    if (t->card && !strcmp(s_bm_path, s_path)) { s_bm_path[0] = 0; s_bm_pos = 0; s_bm_dirty = true; }     /* that file has now been heard to the end */
    if (!advance(false, 1)) { s_adv = ADV_END; return false; }
    s_adv = ADV_NEXT;
    uint32_t rate = t->rate;
    int64_t t0 = esp_timer_get_time();
    close_track(t);
    if (open_track(s_list[s_idx], t) < 0) return false;
    if (t->rate != rate || pick_sink() != sink) { close_track(t); return false; }       /* another rate or output: the retune path takes it from here */
    strlcpy(s_path, s_list[s_idx], sizeof s_path);
    s_adv = ADV_NONE;
    nsn_oc_join(s_wr_frames);
    size_t used = RING_BYTES - xRingbufferGetCurFreeSize(s_ring);
    ESP_LOGI(TAG, "gapless: %s opened in %d ms, %d ms of audio still queued", s_path, (int)((esp_timer_get_time() - t0) / 1000), (int)(used / 4 * 1000 / (s_link_rate ? s_link_rate : 48000)));
    return true;
}

/* Which output the volume belongs to now (sink id - 1): the running session's, else the one that would be picked. */
static int vol_output(void)
{
    if (s_vout) return s_vout - 1;
    int k = pick_sink();
    if (k) return k - 1;
    return s_out_pref > 0 && s_out_pref < NSN_SINK_MAX ? s_out_pref - 1 : 0;
}

/* the volume output o starts from: its own, else the DAC's setting, else (headphones / broadcast) no louder than SAFE_START; always under the limit */
static int vol_start(int o, int cur)
{
    int v = s_ovol[o];
    if (v < 0) v = cur < SAFE_START ? cur : SAFE_START;
    return pc_volsel_clamp(v, s_vmax);
}

static void set_vol_now(int v) { s_vol = v; float x = v / 100.0f; s_gain = x * x * x; }

/* The session's output is chosen: its own volume applies (an output that never had one gets the safe start and keeps it). */
static void apply_out_volume(int sink)
{
    int o = sink - 1;
    int v = vol_start(o, s_vol);
    if (s_ovol[o] < 0) { s_ovol[o] = v; s_dirty = true; ESP_LOGI(TAG, "first use of %s: volume starts at %d", nsn_sink_get(sink)->desc, v); }
    s_vout = sink;
    if (v != s_vol) ESP_LOGI(TAG, "volume %d -> %d (output %d, limit %d)", s_vol, v, sink, s_vmax);
    set_vol_now(v);
}

/* One track, and any that follow it gaplessly. 0 = ended, 1 = skip requested, -1 = could not play, 2 = sleep timer over / stop / output gone */
static int play_one(void)
{
    s_gap = true; s_adv = ADV_NONE;
    if (s_wr_frames) nsn_oc_join(s_wr_frames);                  /* a boundary of the playlist (for the output check's report) */
    trk_t t;
    float *fl = NULL, *rs = NULL; int16_t *pcm = NULL; int res = 0;
    if (open_track(s_path, &t) < 0) return -1;
    uint32_t rate = t.rate;
    int sink = pick_sink();
    if (!sink) { ESP_LOGE(TAG, "no output (USB DAC / Bluetooth)"); res = -1; goto out; }
    apply_out_volume(sink);
    const nsn_sink_t *sk = nsn_sink_get(sink);
    uint32_t link = sk->link_rate(rate);
    char dm[24] = "";
    if (sk->describe) sk->describe(dm, sizeof dm);
    ESP_LOGI(TAG, "%s: %s %lu Hz, %ld ms -> %s %lu Hz%s", s_path, s_fmt, (unsigned long)rate, (long)s_len_ms, sk->label, (unsigned long)link, dm);
    if (s_sink && (s_sink != sink || s_link_rate != link)) {      /* other output or rate: let the old audio play out, then retune */
        for (int i = 0; i < 100 && !s_stop && !ring_empty(); i++) vTaskDelay(pdMS_TO_TICKS(30));
        stop_sink();
    }
    int cap = BLOCK;
    if (link != rate) {
        if (!s_rsq) s_rsq = heap_caps_calloc(1, pc_rsq_size(), MALLOC_CAP_SPIRAM);
        if (!s_rsq || pc_rsq_init(s_rsq, rate, link) != 0) { ESP_LOGE(TAG, "cannot resample %lu -> %lu", (unsigned long)rate, (unsigned long)link); res = -1; goto out; }
        cap = (int)((uint64_t)BLOCK * link / rate) + 32;
        rs = heap_caps_malloc((size_t)cap * 2 * sizeof(float), MALLOC_CAP_SPIRAM);
    }
    s_rate = rate; nsn_spec_set_rate(link);
    fl = heap_caps_malloc(BLOCK * 2 * sizeof(float), MALLOC_CAP_SPIRAM);
    pcm = heap_caps_malloc((size_t)cap * 2 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    if (!fl || !pcm || (link != rate && !rs)) { res = -1; goto out; }
    int sub = 0, subn = (t.be->track_count && t.be->start_track) ? t.be->track_count(t.h) : 1;
    if (subn < 1) subn = 1;
    s_sub = 1; s_subn = subn;
    bool live = t.be == &btin_backend;
    while (!s_stop) {
        if (s_skip && subn > 1 && sub + s_skip >= 0 && sub + s_skip < subn) {          /* next / previous tune inside the file */
            sub += s_skip; s_skip = 0; s_sub = sub + 1;
            if (t.be->start_track(t.h, sub) == 0) { s_len_ms = t.be->length_ms ? t.be->length_ms(t.h) : -1; s_pos_ms = 0; drain_ring(); continue; }
        }
        if (s_skip) { res = 1; s_gap = true; drain_ring(); break; }
        const nsn_sink_t *cur = nsn_sink_get(s_sink);
        if (cur && !cur->present()) {                                                   /* the output went away under the session */
            ESP_LOGW(TAG, "%s", cur->gone_msg);
            if (cur->resume_after_drop) { snprintf(s_drop_path, sizeof s_drop_path, "%s", s_path); s_drop_us = esp_timer_get_time(); }
            res = 2; break;
        }
        bool over; float fade = sleep_fade(&over);
        if (over) { res = 2; break; }
        int n = t.be->render(t.h, fl, BLOCK);
        if (n <= 0) {
            if (subn > 1 && sub + 1 < subn && t.be->start_track(t.h, ++sub) == 0) { s_sub = sub + 1; s_len_ms = t.be->length_ms ? t.be->length_ms(t.h) : -1; continue; }   /* next tune of the same file */
            if (try_chain(&t, sink)) {                                                  /* the next track carries on in the same session */
                sub = 0; subn = (t.be->track_count && t.be->start_track) ? t.be->track_count(t.h) : 1;
                if (subn < 1) subn = 1;
                s_sub = 1; s_subn = subn;
                continue;
            }
            break;
        }
        nsn_fx_process(fl, n, rate);
        const float *src_f = fl;
        if (link != rate) { int used = 0; n = pc_rsq_process(s_rsq, fl, n, rs, cap, &used); src_f = rs; }
        float g = s_gain * fade * s_rg * 32767.0f;
        for (int i = 0; i < n * 2; i++) {
            float v = src_f[i] * g;
            pcm[i] = v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : (int16_t)v;
        }
        size_t bytes = n * 4; const uint8_t *src = (const uint8_t *)pcm;
        while (bytes && !s_stop && !s_skip) {                    /* block while the ring is full */
            size_t chunk = bytes > 2048 ? 2048 : bytes;
            if (xRingbufferSend(s_ring, src, chunk, pdMS_TO_TICKS(100)) == pdTRUE) { src += chunk; bytes -= chunk; s_gap = false; s_wr_frames += (uint32_t)(chunk / 4); }
        }
        s_pos_ms = t.be->position_ms ? t.be->position_ms(t.h) : 0;
        if (!s_sink && xRingbufferGetCurFreeSize(s_ring) < (live ? RING_BYTES - 8192 : RING_BYTES / 2)) start_sink(sink, link);
    }
    if (!s_sink && !s_stop && res != 1) start_sink(sink, link);  /* a clip shorter than half the ring */
out:
    if (t.h && t.card && t.be->byte_pos && (s_stop || res == 2)) {      /* stopped in the middle of a card file: remember where */
        snprintf(s_bm_path, sizeof s_bm_path, "%s", s_path); s_bm_pos = t.be->byte_pos(t.h); s_bm_dirty = true;
    } else if (t.h && t.card && res == 0 && !strcmp(s_bm_path, s_path)) {     /* that file has now been heard to the end */
        s_bm_path[0] = 0; s_bm_pos = 0; s_bm_dirty = true;
    }
    close_track(&t);
    heap_caps_free(fl); heap_caps_free(pcm); heap_caps_free(rs);
    return res;
}

/* the phone started / stopped streaming (BTC task): follow it from a task of our own, never block the stack */
static QueueHandle_t s_phone_q;
static void phone_task(void *arg)
{
    uint8_t on;
    for (;;) {
        if (xQueueReceive(s_phone_q, &on, portMAX_DELAY) != pdTRUE) continue;
        if (on) { if (strcmp(s_path, "bt:in") || !s_active) nsn_player_play("bt:in"); }
        else if (s_active && !strcmp(s_path, "bt:in")) nsn_player_stop();
    }
}
static void on_phone(bool streaming) { uint8_t v = streaming; if (s_phone_q) xQueueSend(s_phone_q, &v, 0); }
void nsn_player_follow_phone(void)
{
    if (!s_phone_q) { s_phone_q = xQueueCreate(4, 1); xTaskCreateWithCaps(phone_task, "phone", 4096, NULL, 4, NULL, MALLOC_CAP_SPIRAM); }
    nsn_bt_set_in_callback(on_phone);
}

void nsn_player_set_mode(int m) { s_mode = m < 0 || m > 3 ? 0 : m; }
int nsn_player_mode(void) { return s_mode; }
static int cmp_name(const void *a, const void *b) { return strcasecmp((const char *)a, (const char *)b); }

static void run_session(void)
{
    int fails = 0;
    s_eof = false; s_underruns = 0; s_wr_frames = 0;
    s_soft_pending = s_n > 0 && strcmp(s_list[s_idx], "bt:in") != 0;     /* a phone stream has its own volume in front of ours and must not be delayed */
    while (!s_stop && s_n > 0) {
        strlcpy(s_path, s_list[s_idx], sizeof s_path);
        int r = play_one();
        if (s_stop || r == 2) break;
        if (r < 0) { if (++fails >= s_n) { ESP_LOGE(TAG, "nothing playable"); break; } } else fails = 0;
        if (s_adv == ADV_END) break;                                  /* play_one found the list over, or already moved on to the next track */
        if (s_adv == ADV_NEXT) continue;
        int dir = s_skip < 0 ? -1 : 1;
        bool manual = s_skip != 0;
        s_skip = 0;
        if (!advance(manual, dir)) break;
    }
    s_eof = true;
    s_sleep_deadline_us = 0;                      /* a timer belongs to one session: it must not cut tomorrow's music */
    if (!s_stop) for (int i = 0; i < 200 && !ring_empty(); i++) vTaskDelay(pdMS_TO_TICKS(50));   /* let the tail play */
    ESP_LOGI(TAG, "stopped (underruns %lu)", (unsigned long)s_underruns);
    stop_sink();
    s_vout = 0;
    s_active = false;
}

/* persistent worker with its stack in PSRAM (internal RAM is the scarce one; a task created WithCaps must not delete itself) */
static void decode_task(void *arg)
{
    for (;;) { ulTaskNotifyTake(pdTRUE, portMAX_DELAY); run_session(); }
}

static bool is_dir(const char *p) { struct stat st; return stat(p, &st) == 0 && S_ISDIR(st.st_mode); }

/* Entry points (play, resume, stop) come from several tasks (console, BLE commands, the phone follower, night mode, the main loop):
 * one recursive lock makes them take turns. Rules for the flags: only these entry points set s_stop / clear it and touch the list; the decode
 * task owns everything else of a session and clears s_active as its very last step; a new session never starts while s_active is set. */
static SemaphoreHandle_t s_api;
static void api_lock(void)
{
    static portMUX_TYPE mx = portMUX_INITIALIZER_UNLOCKED;
    if (!s_api) { SemaphoreHandle_t m = xSemaphoreCreateRecursiveMutex(); portENTER_CRITICAL(&mx); if (!s_api) { s_api = m; m = NULL; } portEXIT_CRITICAL(&mx); if (m) vSemaphoreDelete(m); }
    xSemaphoreTakeRecursive(s_api, portMAX_DELAY);
}
static void api_unlock(void) { xSemaphoreGiveRecursive(s_api); }

/* the zip archive's playlist: every playable entry at or below the folder, "<zip>/<entry>" in name order */
typedef struct { const char *zip; int n; } zlist_t;
static bool zlist_add(void *ctx, const char *entry)
{
    zlist_t *z = ctx;
    if (z->n >= MAX_TRACKS) return false;
    if (snprintf(s_list[z->n], NAME_MAX_, "%s/%s", z->zip, entry) < NAME_MAX_) z->n++;
    else ESP_LOGW(TAG, "%s: path too long, left out", entry);
    return true;
}
static bool playable_name(const char *name) { return backend_for(name) != NULL; }
bool nsn_player_can_play(const char *name) { return playable_name(name); }

static esp_err_t start_u(const char *path, const char *pick, uint32_t off)
{
    if (nsn_ratetest_running()) { ESP_LOGE(TAG, "the DAC rate test is running"); return ESP_ERR_INVALID_STATE; }
    nsn_player_stop();
    if (s_active) { ESP_LOGE(TAG, "the previous session did not end; not starting %s", path); return ESP_ERR_TIMEOUT; }
    if (!s_ring) { s_ring = xRingbufferCreateWithCaps(RING_BYTES, RINGBUF_TYPE_BYTEBUF, MALLOC_CAP_SPIRAM); if (!s_ring) return ESP_ERR_NO_MEM; }
    if (!s_list) { s_list = heap_caps_calloc(MAX_TRACKS, NAME_MAX_, MALLOC_CAP_SPIRAM); if (!s_list) return ESP_ERR_NO_MEM; }
    drain_ring();
    s_n = 0; s_idx = 0; s_skip = 0; s_hn = 0;
    char zpath[NAME_MAX_]; const char *inner = NULL;
    bool is_http = !strncmp(path, "http://", 7) || !strncmp(path, "https://", 8);
    if (!is_http && path[0] == '/' && nsn_zip_split(path, zpath, sizeof zpath, &inner)) {     /* inside a zip archive */
        nsn_zip_kind_t kind; uint32_t size;
        int rc = nsn_zip_stat(zpath, inner, &kind, &size);
        if (rc) { ESP_LOGE(TAG, "%s: %s", path, nsn_zip_err(rc)); return ESP_ERR_NOT_FOUND; }
        if (kind == NSN_ZIP_NONE) return ESP_ERR_NOT_FOUND;
        if (kind == NSN_ZIP_FILE) {
            if (size > NSN_ZIP_MAX_ENTRY) { ESP_LOGE(TAG, "zip entry %s is %lu bytes: over the %u MB limit for entries (they are played from memory), refused", path, (unsigned long)size, (unsigned)(NSN_ZIP_MAX_ENTRY >> 20)); return ESP_ERR_INVALID_SIZE; }
            if (!backend_for(path)) { ESP_LOGE(TAG, "no decoder for %s", path); return ESP_ERR_NOT_SUPPORTED; }
            size_t pl = strlen(path); while (pl > 1 && path[pl - 1] == '/') pl--;
            snprintf(s_list[0], NAME_MAX_, "%.*s", (int)pl, path); s_n = 1; s_repeat = false;
        } else {
            zlist_t z = { zpath, 0 };
            int skipped = 0;
            rc = nsn_zip_collect(zpath, inner, playable_name, zlist_add, &z, &skipped);
            if (rc < 0) { ESP_LOGE(TAG, "%s: %s", path, nsn_zip_err(rc)); return ESP_ERR_NOT_FOUND; }
            s_n = z.n;
            if (skipped) ESP_LOGW(TAG, "%d entries over the size limit or with an unsupported method are not in the playlist", skipped);
            qsort(s_list, s_n, NAME_MAX_, cmp_name);
            s_repeat = true;
        }
    } else if (!is_http && is_dir(path)) {
        DIR *d = opendir(path);
        if (!d) return ESP_ERR_NOT_FOUND;
        char base[NAME_MAX_]; strlcpy(base, path, sizeof base);
        size_t bl = strlen(base); while (bl > 1 && base[bl - 1] == '/') base[--bl] = 0;
        for (struct dirent *de; (de = readdir(d)) && s_n < MAX_TRACKS;) {
            if (de->d_type == DT_DIR || !backend_for(de->d_name)) continue;
            if (snprintf(s_list[s_n], NAME_MAX_, "%s/%s", base, de->d_name) < NAME_MAX_) s_n++;
        }
        closedir(d);
        qsort(s_list, s_n, NAME_MAX_, cmp_name);
        s_repeat = true;
    } else {
        strlcpy(s_list[0], path, NAME_MAX_); s_n = 1; s_repeat = false;
    }
    if (s_n == 0) return ESP_ERR_NOT_FOUND;
    s_start_off = 0;
    if (pick) {
        int k = -1;
        for (int i = 0; i < s_n; i++) if (!strcmp(s_list[i], pick)) k = i;
        if (k < 0) return ESP_ERR_NOT_FOUND;
        s_idx = k; s_start_off = off;
    }
    s_stop = false; s_paused = false; s_active = true; s_pos_ms = 0; s_len_ms = -1;
    if (!s_task && xTaskCreatePinnedToCoreWithCaps(decode_task, "decode", 24 * 1024, NULL, 6, &s_task, 1, MALLOC_CAP_SPIRAM) != pdPASS) { s_active = false; return ESP_ERR_NO_MEM; }
    xTaskNotifyGive(s_task);
    return ESP_OK;
}

static esp_err_t start(const char *path, const char *pick, uint32_t off) { api_lock(); esp_err_t e = start_u(path, pick, off); api_unlock(); return e; }
esp_err_t nsn_player_play(const char *path) { return start(path, NULL, 0); }

/* go on from the bookmark: its folder as the playlist, that file first, from the remembered place */
esp_err_t nsn_player_resume(void)
{
    if (!s_bm_path[0]) return ESP_ERR_NOT_FOUND;
    char file[160], dir[160]; uint32_t off = s_bm_pos;
    strlcpy(file, s_bm_path, sizeof file); strlcpy(dir, s_bm_path, sizeof dir);
    char *sl = strrchr(dir, '/');
    if (!sl || sl == dir) return ESP_ERR_NOT_FOUND;
    *sl = 0;
    api_lock();
    esp_err_t e = start(dir, file, off);
    if (e != ESP_OK) e = start(file, NULL, 0);
    api_unlock();
    return e;
}

void nsn_player_stop(void)
{
    api_lock();
    s_stop = true;
    nsn_radio_cancel();                                    /* a station still connecting gives up now instead of after its 30 s */
    for (int i = 0; i < 800 && s_active; i++) vTaskDelay(pdMS_TO_TICKS(10));
    api_unlock();
}

void nsn_player_set_output(int pref) { s_out_pref = pref; g_set.out = pref; nsn_settings_save(); }
int nsn_player_output(void) { return s_out_pref; }
int nsn_player_sink(void) { return s_sink; }
void nsn_player_skip(int dir) { if (s_active) s_skip = dir < 0 ? -1 : 1; }
bool nsn_player_active(void) { return s_active; }

void nsn_player_sleep(int minutes)
{
    s_sleep_deadline_us = minutes > 0 ? esp_timer_get_time() + (int64_t)minutes * 60 * 1000000 : 0;
}

void nsn_player_status(char *out, int n)
{
    snprintf(out, n, "%s %s %lu Hz %ld/%ld ms track %d/%d underruns=%lu", s_active ? "playing" : "idle", s_fmt, (unsigned long)s_rate, (long)s_pos_ms, (long)s_len_ms,
             s_n ? s_idx + 1 : 0, s_n, (unsigned long)s_underruns);
}

void nsn_player_info(nsn_player_info_t *o)
{
    o->active = s_active; o->paused = s_paused; o->fmt = s_fmt; o->rate = s_rate; o->pos_ms = s_pos_ms; o->len_ms = s_len_ms;
    o->underruns = s_underruns; o->volume = nsn_player_volume(); strlcpy(o->path, s_path, sizeof o->path);
    strlcpy(o->title, s_tag_title, sizeof o->title); strlcpy(o->artist, s_tag_artist, sizeof o->artist); strlcpy(o->album, s_tag_album, sizeof o->album);
    o->index = s_n ? s_idx + 1 : 0; o->count = s_n; o->sub = s_sub; o->subn = s_subn;
    int64_t left = s_sleep_deadline_us ? (s_sleep_deadline_us - esp_timer_get_time()) / 1000000 : 0;
    o->sleep_left_s = left > 0 ? (int)left : 0;
}

void nsn_player_pause(bool on)
{
    if (!on && s_paused && s_sink) soft_arm(s_link_rate ? s_link_rate : 48000);      /* the ring holds full-level audio: ramp it at the output */
    s_paused = on;
}

/* what `vol` shows and sets: the playing output's volume (or, idle, the one of the output that would be picked), below the limit */
int nsn_player_volume(void)
{
    if (s_vout) return s_vol;
    return vol_start(vol_output(), s_vol);
}

void nsn_player_set_volume(int pct)
{
    int o = vol_output();
    pct = pc_volsel_clamp(pct, s_vmax);
    s_ovol[o] = pct;
    if (o == 0) g_set.vol = pct;
    set_vol_now(pct);
    s_dirty = true;
}

void nsn_player_set_volume_temp(int pct)       /* the alarm's ramp: moves the gain only, the saved volume stays */
{
    float x = pc_volsel_clamp(pct, s_vmax) / 100.0f;
    s_gain = x * x * x;
}

/* The limit: nothing above it is applied or shown; stored values stay as they are (raising the limit brings them back). */
void nsn_player_set_vollimit(int n)
{
    s_vmax = pc_volsel_clamp(n, 100);
    if (s_vol > s_vmax) set_vol_now(s_vmax);
    s_dirty = true;
}
int nsn_player_vollimit(void) { return s_vmax; }

void nsn_player_set_dacmax(uint32_t hz) { s_dacmax = hz ? pc_dacmax_norm((int)hz, 384000) : 0; s_dirty = true; }
uint32_t nsn_player_dacmax(void) { return s_dacmax; }

void nsn_player_set_gapless(bool on) { s_gapless = on; s_dirty = true; }
bool nsn_player_gapless(void) { return s_gapless; }

/* boot: load what this module keeps in NVS (the DAC's volume is g_set.vol) */
void nsn_player_init_settings(void)
{
    if (!nsn_sink_count()) {                       /* once: the built-ins take ids 1..3, then whatever the board has (board_sinks_register) */
        nsn_sink_register(&k_sink_dac); nsn_sink_register(&k_sink_bt); nsn_sink_register(&k_sink_cast);
        board_sinks_register();
    }
    s_gapless = nsn_settings_get_i32("gapless", 1) != 0;
    s_dacmax = nsn_settings_get_i32("dacmax", 0) > 0 ? pc_dacmax_norm(nsn_settings_get_i32("dacmax", 0), 384000) : 0;
    s_vmax = pc_volsel_clamp(nsn_settings_get_i32("vollimit", 100), 100);
    s_ovol[0] = pc_volsel_clamp(g_set.vol, 100);
    for (int o = 1; o < nsn_sink_count(); o++) { int v = nsn_settings_get_i32(nsn_sink_get(o + 1)->vol_key, -1); s_ovol[o] = v < 0 ? -1 : pc_volsel_clamp(v, 100); }
    set_vol_now(pc_volsel_clamp(s_ovol[0], s_vmax));
    s_dirty = false;
}

/* write what changed, from a task with an internal stack (the command task, the main loop): the decode task and the BT / keys callbacks only mark it */
void nsn_player_persist(void)
{
    if (!s_dirty) return;
    s_dirty = false;
    nsn_settings_save();
    for (int o = 1; o < nsn_sink_count(); o++) if (s_ovol[o] >= 0) nsn_settings_set_i32(nsn_sink_get(o + 1)->vol_key, s_ovol[o]);
    nsn_settings_set_i32("vollimit", s_vmax);
    nsn_settings_set_i32("gapless", s_gapless ? 1 : 0);
    nsn_settings_set_i32("dacmax", (int)s_dacmax);
}

/* debug: forget an output's own volume (1 Bluetooth, 2 Auracast) so its next first use takes the safe start again */
void nsn_player_vol_forget(int o)
{
    if (o < 1 || o >= nsn_sink_count()) return;
    s_ovol[o] = -1; nsn_settings_erase(nsn_sink_get(o + 1)->vol_key);
}
void nsn_player_vol_dump(char *out, int n)
{
    snprintf(out, n, "vol dac=%d bt=%d cast=%d (-1 = never stored) limit=%d effective=%d playing_output=%d", s_ovol[0], s_ovol[1], s_ovol[2], s_vmax, nsn_player_volume(), s_vout);
}
void nsn_player_ramp_dump(char *out, int n)
{
    int k = snprintf(out, n, "soft start %lu/%lu frames left, peak per 1/8 ramp:", (unsigned long)s_soft_left, (unsigned long)s_soft_total);
    for (int i = 0; i < 12 && k < n; i++) k += snprintf(out + k, n - k, " %d", s_ramp_pk[i]);
}
