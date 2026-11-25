// SPDX-License-Identifier: LGPL-2.1-or-later
#include "fmglue.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fmvgm_port_idf.h"
#include "pc/pc_io.h"
#include "pc/pc_zip.h"
#include "sink.h"
#include "player.h"

static const char *TAG = "fm";

/* ---- defaults ----------------------------------------------------------------------------------------------------------------------------------------------------------- */
static int s_check = 1;                    /* dbg fm check 0: the static table is not consulted (to see what a heavy file does) */
static int s_out_auto = 1;                 /* the rate comes from the output; dbg fm rate N / native overrides */
static uint32_t s_out_fixed;
static uint32_t s_skips_static, s_skips_dyn;

void nsn_fm_init(void)
{
    fmvgm_port_install_idf();
    fmvgm_set_placement(FMVGM_PLACE_CORE0);             /* `dbg fm core auto` (the default) picks the assignment per file in nsn_fm_check; core 0 first */
    fmvgm_set_worker_priority(12);                     /* measured: docs/measurements.md (priority sweep), reasons: docs/design.md */
    fmvgm_set_taps(24);
    fmvgm_set_overload(4000, 400);                      /* more than 0.4 s of inserted silence (worker) or of an output ring running dry (inline) within 4 s of audio: the producer cannot keep up. Measured only after 2.5 s of playing, which gives the fx shedding (2 s at 90 %) its chance first */
}

void nsn_fm_note_skip(bool dynamic) { if (dynamic) s_skips_dyn++; else s_skips_static++; }

uint32_t nsn_fm_out_rate(int sink)
{
    if (!s_out_auto) return s_out_fixed;
    const nsn_sink_t *k = nsn_sink_get(sink);
    if (!k) return 48000;
    uint32_t r = k->link_rate(48000);
    return r ? r : 48000;
}

/* ---- the budget -------------------------------------------------------------------------------------------------------------------------------------------------------
 * Static pre-check: "is this chip set known to be too heavy for this output, and which chip goes to which core?". An assignment problem: the chips of the file are put on core 0 or core 1
 * (each core's chips become one lane of the backend: a worker on that core, the lanes are mixed by the consumer, bit-identical for every assignment: docs/design.md). Predicted load of a core in percent:
 *     base(output, core) + sum over the chips on that core of w(chip, core)         valid when both cores are at or under LIMIT (90)
 * base: what the output pipeline alone costs on that core; w: one chip on a "typical" synthetic stream on that core, including its own resampler. All of it was measured on the board
 * (synthetic register streams, 48 kHz output, 24-tap resampler in the producer; `docs/measurements.md` has the matrix: totals of `st` "cpu" per core). The dense stream is 3-8 % more; a real
 * song is about 0.4-0.7 of the synthetic one (study 4.5). The table is the only place these numbers live; to add a chip, measure it alone on core 0 and on core 1 on the USB DAC and add a row.
 *
 *   base                                                         core 0   core 1
 *     USB DAC                                                      10       8
 *     Bluetooth AAC (A2DP to JukeDial)                             12      25        SBC: not measured, taken as AAC
 *     Bluetooth LDAC                                               12      58        ESTIMATE: no LDAC sink here; AAC's 25 + half a core for the encoder (docs/measurements.md, round's figure)
 *     Auracast (LC3 x 2 BIS, Wi-Fi on)                             31      57        (19 for the output; +12 from the cast task's priority over the worker, measured)
 *
 * Choice among the valid assignments: everything on core 0 if that is valid (core 1 stays free for the resampler, the effects and the encoders, and one worker is the simplest arrangement);
 * else the assignment with the lowest of the two predicted peaks. No valid assignment: skip the file ("too heavy for <output>"). Nothing is remembered between files.
 * `dbg fm core 0|1|inline` forces every chip onto one core (the prediction is then only checked), `dbg fm plan 12` forces a plan, `dbg fm core auto` (the default) lets this table choose.
 * The dynamic rule (fmvgm_set_overload, player.c) catches the rest: a real song heavier than its synthetic estimate, a Wi-Fi burst, effects. */
#define LIMIT 90
typedef struct { uint8_t c0, c1; } load_t;
static load_t base_load(int sink, const char *codec)
{
    if (sink == 1) return (load_t){ 10, 8 };
    if (sink == 2) return codec && !strcmp(codec, "LDAC") ? (load_t){ 12, 58 } : (load_t){ 12, 25 };
    if (sink == 3) return (load_t){ 31, 57 };                  /* core 0: 19 for the output alone, +12 because the cast task (priority 21) takes the CPU from a worker on that core: YM2608 / YM2610 typical on core 0 starved (measured 2026-11-26) */
    return (load_t){ 15, 30 };                         /* an output nobody measured (board sink): a guess */
}
/* w(chip): typical stream, percent of one core, core 0 / core 1 (core 0 includes what the radio stacks take from it). Measured 2026-11-25 (YM2151, YM2203) and 2026-11-26 (the rest: `st` cpu per core with the chip alone on that core, the larger of two runs, less the output's base; docs/measurements.md). The `st` figure is noisy (+-10 points between runs of one file), the producer's own wall time (dbg fm stat) is higher because preemption counts in it. */
typedef struct { const char *name; uint8_t w0, w1; } chipw_t;
static const chipw_t k_w[] = {                                                  /* % of one core, typical stream, USB DAC; the dense stream of OPNA is 40 % above it (measurements.md) */
    { "YM2151", 76, 69 }, { "YM2203", 55, 50 }, { "YM2608", 60, 60 }, { "YM2610", 62, 62 }, { "YM2610B", 72, 72 }, { "YM2612", 55, 55 }, { "YM3438", 55, 55 },
    { "AY8910", 25, 25 }, { "SN76489", 19, 19 }, { "OKIM6295", 11, 11 },
};
static load_t chip_w(const char *name)
{
    for (unsigned i = 0; i < sizeof k_w / sizeof k_w[0]; i++) if (!strcmp(k_w[i].name, name)) return (load_t){ k_w[i].w0, k_w[i].w1 };
    return (load_t){ 60, 55 };                          /* an unknown chip: assume a heavy one */
}
static int s_place_auto = 1; static uint8_t s_plan_fixed[4]; static int s_plan_n;
static void predict(const fmvgm_info_t *info, load_t b, unsigned mask1, int *p0, int *p1)     /* mask1: bit i set = chip i on core 1 */
{
    int v0 = b.c0, v1 = b.c1;
    for (int i = 0; i < info->n_chips && i < 4; i++) { load_t w = chip_w(info->chip[i].name); if (mask1 >> i & 1) v1 += w.c1; else v0 += w.c0; }
    *p0 = v0; *p1 = v1;
}
static void out_label(int sink, const char *codec, char *out, size_t n)
{
    const nsn_sink_t *k = nsn_sink_get(sink);
    if (sink == 2) snprintf(out, n, "Bluetooth%s%s", codec && codec[0] ? " " : "", codec ? codec : "");
    else snprintf(out, n, "%s", k ? k->desc : "output");
}
int nsn_fm_check(const fmvgm_info_t *info, int sink, const char *codec, bool apply, char *why, size_t n)
{
    out_label(sink, codec, why, n);
    if (apply) fmvgm_set_plan(NULL, 0);
    if (!info->n_chips) return 0;
    const bool enforce = s_check != 0;                                       /* dbg fm check 0: the assignment is still made, nothing is refused */
    load_t b = base_load(sink, codec);
    int nc = info->n_chips < 4 ? info->n_chips : 4;
    int p0, p1;
    if (!s_place_auto) {
        if (s_plan_n) {                                                     /* a plan typed in: taken as it is, only checked */
            unsigned m1 = 0; for (int i = 0; i < nc && i < s_plan_n; i++) if (s_plan_fixed[i] != FMVGM_PLACE_CORE0) m1 |= 1u << i;
            predict(info, b, m1, &p0, &p1);
            if (enforce && (p0 > LIMIT || p1 > LIMIT)) { ESP_LOGW(TAG, "%s: plan predicted %d %% of core 0, %d %% of core 1 (limit %d)", why, p0, p1, LIMIT); return -1; }
            if (apply) fmvgm_set_plan(s_plan_fixed, s_plan_n);
            return 0;
        }
        int cur = fmvgm_placement(), core = cur == FMVGM_PLACE_CORE0 ? 0 : 1;
        predict(info, b, core ? (1u << nc) - 1 : 0, &p0, &p1);
        if (enforce && (core ? p1 : p0) > LIMIT) { ESP_LOGW(TAG, "%s: predicted %d %% of core %d (limit %d)", why, core ? p1 : p0, core, LIMIT); return -1; }
        return 0;
    }
    /* the assignment problem: 2^n assignments of n <= 4 chips, the first valid one wins (all on core 0), else the lowest peak */
    int best = -1, best_peak = 1000;
    for (unsigned m1 = 0; m1 < (1u << nc); m1++) {
        predict(info, b, m1, &p0, &p1);
        if (enforce && (p0 > LIMIT || p1 > LIMIT)) continue;
        if (m1 == 0) { best = 0; break; }
        int peak = p0 > p1 ? p0 : p1;
        if (peak < best_peak) { best_peak = peak; best = (int)m1; }
    }
    if (best < 0) {
        predict(info, b, 0, &p0, &p1);
        int a0 = p0, a1 = p1; predict(info, b, (1u << nc) - 1, &p0, &p1);
        ESP_LOGW(TAG, "%s: no assignment fits (all on core 0: %d %%, all on core 1: %d %%, limit %d)", why, a0, p1, LIMIT); (void)a1;
        return -1;
    }
    if (apply) {
        uint8_t pl[4];
        for (int i = 0; i < nc; i++) pl[i] = (best >> i & 1) ? FMVGM_PLACE_CORE1 : FMVGM_PLACE_CORE0;
        fmvgm_set_plan(pl, nc);
        if (best) { predict(info, b, (unsigned)best, &p0, &p1); ESP_LOGI(TAG, "%s: chips split over both cores (predicted %d %% / %d %%)", why, p0, p1); }
    }
    return 0;
}

/* ---- the optional YM2608 rhythm ROM -------------------------------------------------------------------------------------------------------------------------------------- */
#define ROM2608_PATH "/sdcard/_player/roms/ym2608_adpcm_rom.bin"
static uint8_t *s_rom2608; static bool s_rom_event;
void nsn_fm_session_start(void) { s_rom_event = false; }
bool nsn_fm_rom_event_once(void) { if (s_rom_event) return false; s_rom_event = true; return true; }
void nsn_fm_load_rom(void)
{
    if (s_rom2608) return;
    FILE *f = fopen(ROM2608_PATH, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz != 8192) ESP_LOGW(TAG, "%s is %ld bytes (ymfm's image is 8192): used as it is", ROM2608_PATH, sz);
    if (sz > 0 && sz <= 65536) {
        uint8_t *b = heap_caps_malloc((size_t)sz, MALLOC_CAP_SPIRAM);
        if (b && fread(b, 1, (size_t)sz, f) == (size_t)sz) { s_rom2608 = b; fmvgm_set_rom(FMVGM_ROM_YM2608_RHYTHM, b, (uint32_t)sz); ESP_LOGI(TAG, "YM2608 rhythm ROM loaded (%ld bytes)", sz); }
        else heap_caps_free(b);
    }
    fclose(f);
}

/* ---- dbg fm ---------------------------------------------------------------------------------------------------------------------------------------------------------- */
typedef struct { char path[96]; uint32_t secs; int core, prio; void (*out)(void *, const char *); void *ctx; } bench_job_t;
static TaskHandle_t s_bench_task[2]; static bench_job_t s_job[2]; static volatile int s_bench_busy;

static void say(bench_job_t *j, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void say(bench_job_t *j, const char *fmt, ...)
{
    char b[260]; va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    printf("%s\n", b);                                      /* the bench finishes after the command returned: its console context is gone, so the UART it is */
}

static void bench_run(bench_job_t *j)
{
    uint8_t *mem = NULL; uint32_t len = 0;
    FILE *f = fopen(j->path, "rb");
    if (!f) { say(j, "fm bench: cannot open %s", j->path); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 24 * 1024 * 1024 || !(mem = heap_caps_malloc((size_t)sz, MALLOC_CAP_SPIRAM)) || fread(mem, 1, (size_t)sz, f) != (size_t)sz) { say(j, "fm bench: cannot read %s", j->path); fclose(f); heap_caps_free(mem); return; }
    fclose(f); len = (uint32_t)sz;
    if (len > 2 && mem[0] == 0x1f && mem[1] == 0x8b) {
        uint8_t *raw = NULL; uint32_t rl = 0;
        if (pc_gunzip_max(mem, len, &raw, &rl, 24u << 20) != 0 || !raw) { say(j, "fm bench: gunzip failed"); heap_caps_free(mem); return; }
        heap_caps_free(mem); mem = raw; len = rl;
    }
    pc_reader_t r; pc_reader_mem(&r, mem, len);
    void *h = NULL;
    fmvgm_set_checksum(1);
    fmvgm_set_out_rate(s_out_auto ? 48000 : s_out_fixed);   /* 48000: what the DAC and Auracast run at */
    int saved = fmvgm_placement();
    fmvgm_set_placement(FMVGM_PLACE_INLINE);            /* the bench renders on this task whatever the playback placement */
    fmvgm_set_plan(NULL, 0);
    nsn_fm_load_rom();
    if (pc_backend_fmvgm.open(&h, &r, 0) != 0) { fmvgm_set_placement(saved); say(j, "fm bench: open failed: %s", fmvgm_error()); heap_caps_free(mem); return; }
    fmvgm_set_placement(saved);
    fmvgm_info_t in; fmvgm_info(h, &in);
    uint32_t busy = 0;
    int64_t t0 = esp_timer_get_time();
    uint64_t frames = fmvgm_bench(h, j->secs, &busy);
    int64_t wall = esp_timer_get_time() - t0;
    fmvgm_sums_t s; fmvgm_sums(h, &s);
    double audio = (double)frames / in.out_rate;
    for (int c = 1; c < in.n_chips; c++) say(j, "fm bench chip %d: %s@%lu", c, in.chip[c].name, (unsigned long)in.chip[c].clock);
    say(j, "fm bench core=%d prio=%d %s: %s@%lu out=%luHz taps=%d frames=%llu audio=%.2fs busy=%.1fms wall=%.1fms cost=%.2f ms/s (%.1f %% of a core)", j->core, j->prio, j->path, in.chip[0].name,
        (unsigned long)in.chip[0].clock, (unsigned long)in.out_rate, in.taps, (unsigned long long)frames, audio, busy / 1000.0, wall / 1000.0, audio > 0 ? busy / 1000.0 / audio : 0.0, audio > 0 ? busy / 10000.0 / audio : 0.0);
    say(j, "fm bench sums: chipL=%lld chipR=%lld chip_samples=%llu out_hash=%08lx out_frames=%llu table=%08lx", (long long)s.chip_sum_l, (long long)s.chip_sum_r, (unsigned long long)s.chip_samples,
        (unsigned long)s.out_hash, (unsigned long long)s.out_frames, (unsigned long)s.table_hash);
    pc_backend_fmvgm.close(h);
    heap_caps_free(mem);
}

static void bench_task(void *arg)
{
    int core = (int)(intptr_t)arg;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        bench_run(&s_job[core]);
        s_bench_busy = 0;
    }
}

static void start_bench(const char *path, uint32_t secs, int core, int prio, void (*out)(void *, const char *), void *ctx)
{
    if (s_bench_busy) { out(ctx, "fm bench: busy"); return; }
    if (!s_bench_task[core] && xTaskCreatePinnedToCoreWithCaps(bench_task, "fmbench", 14 * 1024, (void *)(intptr_t)core, prio, &s_bench_task[core], core, MALLOC_CAP_SPIRAM) != pdPASS) { out(ctx, "fm bench: no task"); return; }
    vTaskPrioritySet(s_bench_task[core], prio);
    bench_job_t *j = &s_job[core];
    snprintf(j->path, sizeof j->path, "%s", path); j->secs = secs; j->core = core; j->prio = prio; j->out = out; j->ctx = ctx;
    s_bench_busy = 1;
    xTaskNotifyGive(s_bench_task[core]);
    out(ctx, "fm bench: started");
}

static void line(void (*out)(void *, const char *), void *ctx, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
static void line(void (*out)(void *, const char *), void *ctx, const char *fmt, ...)
{
    char b[420]; va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    out(ctx, b);
}

void nsn_fm_dbg(const char *args, void (*out)(void *, const char *), void *ctx)
{
    while (*args == ' ') args++;
    int v;
    if (!*args) {
        line(out, ctx, "fm: core auto|0|1|inline (now %s) | plan DIGITS|auto | prio N | taps N | rate auto|native|HZ | ring MS | loops N | thr WINDOW_MS THR_MS | sum on|off | check 0|1 | stat | bench PATH [SECS] [core] [prio]",
             s_place_auto ? "auto" : fmvgm_placement() == FMVGM_PLACE_INLINE ? "inline" : fmvgm_placement() == FMVGM_PLACE_CORE0 ? "0" : "1");
    }
    else if (!strncmp(args, "core ", 5)) {
        const char *a = args + 5;
        if (!strcmp(a, "auto")) { s_place_auto = 1; s_plan_n = 0; line(out, ctx, "fm placement: automatic per file (all on core 0, else the split with the lowest peak)"); return; }
        int p = !strcmp(a, "0") ? FMVGM_PLACE_CORE0 : !strcmp(a, "1") ? FMVGM_PLACE_CORE1 : !strcmp(a, "inline") ? FMVGM_PLACE_INLINE : -1;
        if (p < 0) line(out, ctx, "fm core 0|1|inline");
        else { s_place_auto = 0; s_plan_n = 0; fmvgm_set_placement(p); line(out, ctx, "fm placement %s, fixed (a playing file moves at its next block)", a); }
    }
    else if (!strncmp(args, "plan ", 5)) {                       /* plan 12: chip 0 on core 0 (1), chip 1 on core 1 (2); 0 = inline; applies to the next files */
        const char *a = args + 5; int n = 0;
        if (!strcmp(a, "auto")) { s_place_auto = 1; s_plan_n = 0; line(out, ctx, "fm plan: automatic"); return; }
        for (; a[n] >= '0' && a[n] <= '2' && n < 4; n++) s_plan_fixed[n] = (uint8_t)(a[n] - '0');
        if (!n) line(out, ctx, "fm plan DIGITS (one per chip in header order: 0 inline, 1 core 0, 2 core 1) | auto");
        else { s_place_auto = 0; s_plan_n = n; line(out, ctx, "fm plan %.4s for the next files (chips beyond it follow `core`)", a); }
    }
    else if (!strncmp(args, "prio ", 5)) { v = atoi(args + 5); fmvgm_set_worker_priority(v); line(out, ctx, "fm worker priority %d", v); }
    else if (!strncmp(args, "taps ", 5)) { v = atoi(args + 5); fmvgm_set_taps(v); line(out, ctx, "fm taps %d (next open)", v); }
    else if (!strncmp(args, "ring ", 5)) { v = atoi(args + 5); fmvgm_set_ring_ms(v); line(out, ctx, "fm ring %d ms (next open)", v); }
    else if (!strncmp(args, "loops ", 6)) { v = atoi(args + 6); fmvgm_set_loops(v); line(out, ctx, "fm loops %d (next open)", v); }
    else if (!strncmp(args, "rate ", 5)) {
        const char *a = args + 5;
        if (!strcmp(a, "auto")) { s_out_auto = 1; line(out, ctx, "fm rate: from the output (next open)"); }
        else if (!strcmp(a, "native")) { s_out_auto = 0; s_out_fixed = 0; line(out, ctx, "fm rate: the chip's own (the player's pc_rsq resamples; next open)"); }
        else { s_out_auto = 0; s_out_fixed = (uint32_t)atoi(a); line(out, ctx, "fm rate: %lu Hz (next open)", (unsigned long)s_out_fixed); }
    }
    else if (!strncmp(args, "thr ", 4)) {
        unsigned w = 0, t = 0;
        if (sscanf(args + 4, "%u %u", &w, &t) >= 1) { fmvgm_set_overload(w, t); line(out, ctx, "fm overload: more than %u ms of silence within %u ms (0 = never)", t, w < 500 ? 500 : w); }
        else line(out, ctx, "fm thr WINDOW_MS THR_MS");
    }
    else if (!strncmp(args, "sum ", 4)) { fmvgm_set_checksum(!strcmp(args + 4, "on")); line(out, ctx, "fm checksums %s (next open)", !strcmp(args + 4, "on") ? "on" : "off"); }
    else if (!strncmp(args, "check ", 6)) { s_check = atoi(args + 6) != 0; line(out, ctx, "fm static check %s", s_check ? "on" : "off"); }
    else if (!strcmp(args, "stat")) {
        fmvgm_stats_t s; fmvgm_stats(NULL, &s);
        line(out, ctx, "fm stat: lanes=%d (%d/%d/%d busy %lu/%lu/%lu us) place=%d made=%lu read=%lu starved=%lu (%lu ms) win=%lu ms fullwaits=%lu lowwater=%lu/%lu busy=%lu us over %lu ms audio -> %.2f ms/s migr=%lu overloaded=%d skips static=%lu dynamic=%lu",
             s.lanes, s.lane_place[0], s.lane_place[1], s.lane_place[2], (unsigned long)s.lane_busy_us[0], (unsigned long)s.lane_busy_us[1], (unsigned long)s.lane_busy_us[2],
             s.placement, (unsigned long)s.frames_made, (unsigned long)s.frames_read, (unsigned long)s.starved, (unsigned long)s.starved_ms, (unsigned long)s.window_ms, (unsigned long)s.full_waits,
             (unsigned long)s.low_water, (unsigned long)s.ring_frames, (unsigned long)s.busy_us, (unsigned long)s.made_ms, s.made_ms ? s.busy_us / 1000.0 / (s.made_ms / 1000.0) : 0.0,
             (unsigned long)s.migrations, s.overloaded, (unsigned long)s_skips_static, (unsigned long)s_skips_dyn);
    }
    else if (!strncmp(args, "bench ", 6)) {
        char path[96]; unsigned secs = 0; int core = 1, prio = 5;
        int k = sscanf(args + 6, "%95s %u %d %d", path, &secs, &core, &prio);
        if (k < 1) line(out, ctx, "fm bench PATH [SECS] [core] [prio]");
        else start_bench(path, secs, core ? 1 : 0, prio, out, ctx);
    }
    else line(out, ctx, "fm ?");
    (void)TAG; (void)s_check;
}
