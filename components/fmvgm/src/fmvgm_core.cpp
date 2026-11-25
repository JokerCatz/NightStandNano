// SPDX-License-Identifier: LGPL-2.1-or-later
/* fmvgm core: the chips (ymfm), the song clock, rate groups with their integer resampler, the producer, the ring and the worker, the pc_backend_t.
 * Layout of the data flow:
 *   Src (VGM / S98 commands) --writes, waits--> chips (native rate) --raw int32 L/R--> group (chips of one rate summed, OPN decimated 3:1, clamped to int16)
 *   --FIFO--> polyphase resampler --> sum of the groups, clamp, fade --> int16 frames --> ring --> render() --> float.
 * All integer: the output is the same on any platform that runs ymfm's integer code. */
#include "fmvgm.h"
#include "fmvgm_int.h"
#include "fmvgm_resamp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <new>
#include "ymfm_opm.h"
#include "ymfm_opn.h"

/* ---- configuration ---------------------------------------------------------------------------------------------------------------------------------------------- */
static void *def_alloc(size_t n, fmvgm_mem_t) { return malloc(n); }
static void def_free(void *p) { free(p); }
static const fmvgm_port_t k_def_port = { def_alloc, def_free, NULL, NULL, NULL, NULL, NULL, NULL };
static fmvgm_port_t g_port = k_def_port;
static volatile int g_place = FMVGM_PLACE_INLINE, g_prio = 12, g_taps = 24, g_ring_ms = 340, g_loops = 2, g_csum = 0;
static volatile uint32_t g_out_rate = 0, g_ov_window = 6000, g_ov_thr = 0;
static char g_err[48];

void fmvgm_set_port(const fmvgm_port_t *p) { g_port = p ? *p : k_def_port; if (!g_port.alloc) g_port.alloc = def_alloc; if (!g_port.free) g_port.free = def_free; }
void fmvgm_set_placement(int p) { g_place = p < 0 || p > 2 ? 0 : p; }
int  fmvgm_placement(void) { return g_place; }
void fmvgm_set_worker_priority(int p);
void fmvgm_set_out_rate(uint32_t hz) { g_out_rate = hz; }
void fmvgm_set_taps(int t) { g_taps = t < 8 ? 8 : t > FMRS_MAX_TAPS ? FMRS_MAX_TAPS : (t & ~1); }
void fmvgm_set_ring_ms(int ms) { g_ring_ms = ms < 40 ? 40 : ms > 2000 ? 2000 : ms; }
void fmvgm_set_loops(int n) { g_loops = n < 1 ? 1 : n > 20 ? 20 : n; }
void fmvgm_set_checksum(int on) { g_csum = on != 0; }
void fmvgm_set_overload(uint32_t window_ms, uint32_t thr_ms) { g_ov_window = window_ms < 500 ? 500 : window_ms; g_ov_thr = thr_ms; }
void fmvgm_get_overload(uint32_t *w, uint32_t *t) { if (w) *w = g_ov_window; if (t) *t = g_ov_thr; }
const char *fmvgm_error(void) { return g_err; }

/* ---- allocation hook for ymfm's per-operator objects (patch 1): a bump arena that the chip's constructor fills, freed as one block ------------------------------------ */
namespace {
struct Arena { uint8_t *base; size_t cap, used; void *spill[16]; int nspill; };
Arena *g_arena;                               /* set while a chip is constructed (open is not re-entrant) */
}
namespace ymfm {
void *ymfm_alloc(size_t n)
{
    Arena *a = g_arena;
    n = (n + 7) & ~(size_t)7;
    if (a && a->used + n <= a->cap) { void *p = a->base + a->used; a->used += n; return p; }
    void *p = g_port.alloc(n, FMVGM_MEM_FAST);          /* the arena was too small: should not happen (sizes are checked by the host test), but never return NULL */
    if (a && a->nspill < 16) a->spill[a->nspill++] = p;
    return p;
}
void ymfm_free(void *) {}
}

/* ---- chips ------------------------------------------------------------------------------------------------------------------------------------------------------ */
namespace {
struct Host : ymfm::ymfm_interface {};
const int BATCH = 128;

struct Chip {
    uint8_t type; uint32_t raw_rate; uint8_t dec;
    virtual ~Chip() {}
    virtual void write(uint8_t reg, uint8_t val) = 0;
    virtual void gen(int32_t *lr, uint32_t n) = 0;       /* n raw frames, interleaved L R */
};
static void fidelity(ymfm::ym2151 &) {}
static void fidelity(ymfm::ym2203 &c) { c.set_fidelity(ymfm::OPN_FIDELITY_MIN); }   /* FM at clock/72 repeated 3 times, SSG resampled: the lowest rate ymfm offers */

template<class C, int KIND> struct ChipT : Chip {
    Host host; C chip; typename C::output_data buf[BATCH];
    ChipT(uint32_t clock) : chip(host) {
        chip.reset();
        fidelity(chip);
        raw_rate = chip.sample_rate(clock);
        type = KIND == 1 ? CT_OPM : CT_OPN; dec = KIND == 2 ? 3 : 1;
    }
    void write(uint8_t reg, uint8_t val) override { chip.write(0, reg); chip.write(1, val); }
    void gen(int32_t *lr, uint32_t n) override {
        while (n) {
            uint32_t m = n > (uint32_t)BATCH ? BATCH : n;
            chip.generate(buf, m);
            for (uint32_t i = 0; i < m; i++) {
                const int32_t *d = buf[i].data;
                if (KIND == 1) { lr[0] = d[0]; lr[1] = d[1]; }
                else { int32_t s = d[0] + d[1] + d[2] + d[3]; lr[0] = s; lr[1] = s; }
                lr += 2;
            }
            n -= m;
        }
    }
};
typedef ChipT<ymfm::ym2151, 1> Opm;
typedef ChipT<ymfm::ym2203, 2> Opn;
const size_t ARENA_OPM = 4096, ARENA_OPN = 2048;      /* room for the objects ymfm makes while constructing (measured 3.3 KB and 1.3 KB; anything beyond goes to the port allocator and shows in fmvgm_info as chip_spill, which the host test requires to be 0) */
}

/* ---- rate group ------------------------------------------------------------------------------------------------------------------------------------------------- */
namespace {
struct Group {
    uint32_t raw_rate; uint8_t dec;
    uint8_t chips[FM_MAX_CHIPS]; int nchips;
    int16_t *buf; uint32_t cap, n; uint64_t base;      /* FIFO of int16 stereo frames [base, base + n) in absolute indices (real frame 0 is index H - 1) */
    uint64_t pos, step;                                /* Q32 absolute position of the next output (its centre), input frames per output frame */
    bool pass, flushed; uint64_t real_end;
    int32_t acc_l, acc_r; int acc_n;                   /* the decimator */
    uint32_t slice_max;                                /* most frames a slice can push */
    int16_t *coef;                                     /* this group's polyphase table (NULL when pass) */
};

const uint32_t CHUNK = 256;                            /* output frames per producer step */
}

struct Handle {
    Src src; SrcInfo info;
    int nchips; Chip *chip[FM_MAX_CHIPS]; uint64_t posq[FM_MAX_CHIPS]; uint64_t cstep[FM_MAX_CHIPS]; uint32_t gen_n[FM_MAX_CHIPS];
    uint8_t *arena; size_t arena_cap; Arena ar;
    Group grp[FM_MAX_CHIPS]; int ngrp;
    int32_t *tmp[2]; uint32_t tmp_n;                   /* raw batch buffers, 2 chips at a time */
    uint32_t wait_left, slice_units; bool sched_eof;
    int taps, H; uint32_t table_hash;
    uint32_t out_rate, native_rate;
    uint64_t out_idx, len_frames, end_frames, fade_frames; uint32_t fade_cur, fade_step; bool out_done;
    int32_t len_ms;
    /* checksums */
    bool csum; int64_t sum_l, sum_r; uint64_t sum_n; uint32_t hash; uint64_t out_frames;
    /* ring: head written by the producer, tail by the consumer */
    int16_t *ring; uint32_t ring_cap; volatile uint32_t head, tail; volatile int prod_done;
    /* placement */
    volatile int place, owned, req_park, parked; void *wk;
    int primed; uint32_t win_out, win_sil;
    fmvgm_stats_t st;
    volatile int overloaded;
    uint32_t snap_us;
};

/* statistics counters have one writer at a time (the producer's, or the consumer's); the other side only reads them: relaxed loads and stores, no read-modify-write (the handle may sit in PSRAM) */
#define ST_ADD(f, v) __atomic_store_n(&(f), (f) + (uint32_t)(v), __ATOMIC_RELAXED)
#define ST_GET(f) __atomic_load_n(&(f), __ATOMIC_RELAXED)

/* ---- worker threads ----------------------------------------------------------------------------------------------------------------------------------------------- */
namespace {
/* One per core, created on first use, never deleted. `job` is the handle it works on (NULL: idle); `gen` is bumped by the consumer each time it changes `job`. */
struct Worker { void *task; Handle *volatile job; volatile uint32_t gen; };
Worker g_wk[2];
fmvgm_stats_t g_last;
}

static void produce_fill_ring(Handle *h);
/* The worker's whole protocol with the consumer. It touches a handle only between reading it from `job` and acknowledging a park request; after the acknowledgement it only compares
 * the generation (no pointer to the handle is dereferenced again), so the consumer may free the handle the moment `parked` is set and `job` cleared. */
static void worker_main(void *arg)
{
    Worker *w = (Worker *)arg;
    uint32_t w_chunks = 0;
    for (;;) {
        Handle *h = __atomic_load_n(&w->job, __ATOMIC_ACQUIRE);
        if (!h) { g_port.task_wait(100); continue; }
        if (__atomic_load_n(&h->req_park, __ATOMIC_ACQUIRE)) {
            uint32_t g0 = __atomic_load_n(&w->gen, __ATOMIC_ACQUIRE);
            __atomic_store_n(&h->parked, 1, __ATOMIC_RELEASE);            /* from here on h is not ours any more */
            while (__atomic_load_n(&w->gen, __ATOMIC_ACQUIRE) == g0) g_port.task_wait(20);
            continue;
        }
        if (__atomic_load_n(&h->prod_done, __ATOMIC_ACQUIRE)) { g_port.task_wait(30); continue; }
        uint32_t free_ = h->ring_cap - (h->head - __atomic_load_n(&h->tail, __ATOMIC_ACQUIRE));
        if (free_ < CHUNK) { ST_ADD(h->st.full_waits, 1); g_port.task_wait(6); continue; }
        produce_fill_ring(h);
        if ((++w_chunks & 3) == 0) g_port.sleep_ms(1);               /* never CPU-bound: a real sleep (a pending wake-up from the consumer would end a task_wait at once), so the idle task (and its watchdog) and every lower-priority task get a tick in about 20 ms, also when the producer is behind. Measured: without it, YM2151 dense + Auracast on core 0 starved IDLE0 (task watchdog) */
    }
}

void fmvgm_set_worker_priority(int p)
{
    g_prio = p < 1 ? 1 : p > 24 ? 24 : p;
    if (g_port.task_prio) for (int c = 0; c < 2; c++) if (g_wk[c].task) g_port.task_prio(g_wk[c].task, g_prio);
}

static bool worker_for(int core)
{
    if (!g_port.task_start || !g_port.task_wake || !g_port.task_wait || !g_port.sleep_ms) return false;      /* no threads on this platform: inline only */
    if (!g_wk[core].task) {
        g_wk[core].job = NULL; g_wk[core].gen = 0;
        g_wk[core].task = g_port.task_start(worker_main, &g_wk[core], core, g_prio, 12 * 1024);
    }
    return g_wk[core].task != NULL;
}

/* the consumer side of a worker: stop it touching h (waits for the end of its current chunk). false: the worker did not answer (the caller must not free h) */
static bool park(Handle *h)
{
    if (!h->owned) return true;
    Worker *w = &g_wk[h->wk == &g_wk[1] ? 1 : 0];
    __atomic_store_n(&h->req_park, 1, __ATOMIC_RELEASE);
    for (int i = 0; i < 3000; i++) {
        if (__atomic_load_n(&h->parked, __ATOMIC_ACQUIRE)) break;
        g_port.task_wake(w->task);
        if (g_port.sleep_ms) g_port.sleep_ms(1);
    }
    if (!__atomic_load_n(&h->parked, __ATOMIC_ACQUIRE)) return false;
    __atomic_store_n(&w->job, (Handle *)NULL, __ATOMIC_RELEASE);
    __atomic_store_n(&w->gen, w->gen + 1, __ATOMIC_RELEASE);               /* lets the worker out of its wait */
    g_port.task_wake(w->task);
    h->owned = 0; h->wk = NULL;
    return true;
}

static void assign(Handle *h, int core)
{
    Worker *w = &g_wk[core];
    h->req_park = 0; h->parked = 0; h->wk = w; h->owned = 1;
    __atomic_store_n(&w->job, h, __ATOMIC_RELEASE);
    g_port.task_wake(w->task);
}

/* ---- the producer: song clock, chips, groups, resampler ----------------------------------------------------------------------------------------------------------------- */
static inline int16_t clamp16(int32_t v) { return (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v); }

static void group_compact(Group &g, int H)
{
    uint64_t keep = (g.pos >> 32);
    keep = keep >= (uint64_t)H ? keep - H + 1 : 0;
    if (g.pass) keep = g.pos >> 32;
    if (keep <= g.base) return;
    uint32_t drop = (uint32_t)(keep - g.base);
    if (drop > g.n) drop = g.n;
    memmove(g.buf, g.buf + 2 * (size_t)drop, (size_t)(g.n - drop) * 4);
    g.n -= drop; g.base += drop;
}

static inline void group_push(Group &g, int32_t l, int32_t r)
{
    g.buf[2 * (size_t)g.n] = clamp16(l); g.buf[2 * (size_t)g.n + 1] = clamp16(r); g.n++;
}

/* advances the song by one slice: runs commands up to the next wait, then generates the chips for at most `slice_units` of it */
static void run_slice(Handle *h)
{
    for (int k = 0; k < h->ngrp; k++) { Group &g = h->grp[k]; if (g.cap - g.n < g.slice_max) group_compact(g, h->H); }
    while (!h->wait_left && !h->sched_eof) {
        Ev e;
        int k = src_next(h->src, e);
        if (k == EV_WRITE) { if (e.slot < h->nchips) h->chip[e.slot]->write(e.reg, e.val); }
        else if (k == EV_WAIT) { if (e.units) h->wait_left = e.units; }
        else h->sched_eof = true;
    }
    if (!h->wait_left) return;
    uint32_t u = h->wait_left < h->slice_units ? h->wait_left : h->slice_units;
    h->wait_left -= u;
    uint32_t need[FM_MAX_CHIPS];
    for (int c = 0; c < h->nchips; c++) {
        h->posq[c] += (uint64_t)u * h->cstep[c];
        need[c] = (uint32_t)(h->posq[c] >> 32) - h->gen_n[c];
        h->gen_n[c] += need[c];
    }
    for (int k = 0; k < h->ngrp; k++) {
        Group &g = h->grp[k];
        uint32_t left = need[g.chips[0]];                 /* the chips of one group have the same rate, so the same count */
        while (left) {
            uint32_t m = left > h->tmp_n ? h->tmp_n : left;
            for (int q = 0; q < g.nchips; q++) h->chip[g.chips[q]]->gen(h->tmp[q & 1] + (q >= 2 ? 0 : 0), m);
            /* sum the chips of the group (at most 2 are summed through two buffers; more chips of one rate accumulate into tmp[0]) */
            if (g.nchips > 1) {
                for (int q = 1; q < g.nchips; q++) {
                    if (q > 1) h->chip[g.chips[q]]->gen(h->tmp[1], m);
                    const int32_t *s = h->tmp[1]; int32_t *d = h->tmp[0];
                    for (uint32_t i = 0; i < 2 * m; i++) d[i] += s[i];
                }
            }
            if (h->csum) {
                /* the chips' own raw sums: per chip, before they are added (one chip: tmp[0]); with two chips the sum of both is recorded, which is what the PC does too */
                int64_t sl = 0, sr = 0; const int32_t *p = h->tmp[0];
                for (uint32_t i = 0; i < m; i++) { sl += p[2 * i]; sr += p[2 * i + 1]; }
                h->sum_l += sl; h->sum_r += sr; h->sum_n += (uint64_t)m * g.nchips;
            }
            const int32_t *p = h->tmp[0];
            if (g.dec == 1) for (uint32_t i = 0; i < m; i++) group_push(g, p[2 * i], p[2 * i + 1]);
            else for (uint32_t i = 0; i < m; i++) {
                g.acc_l += p[2 * i]; g.acc_r += p[2 * i + 1];
                if (++g.acc_n == g.dec) { group_push(g, g.acc_l / g.dec, g.acc_r / g.dec); g.acc_l = g.acc_r = 0; g.acc_n = 0; }
            }
            left -= m;
        }
    }
}

static inline bool group_can(const Group &g, int H)
{
    uint64_t i = g.pos >> 32;
    if (g.pass) return i < g.base + g.n && (!g.flushed || i < g.real_end);
    if (g.flushed && i >= g.real_end) return false;
    return i + H < g.base + g.n;
}

static inline void fnv_i16(uint32_t &h, int16_t v)
{
    h = (h ^ ((uint16_t)v & 0xFF)) * 16777619u;
    h = (h ^ ((uint16_t)v >> 8)) * 16777619u;
}

/* make up to `want` output frames; returns the number made, fewer than asked only at the end of the song (with its fade) */
static uint32_t produce(Handle *h, int16_t *out, uint32_t want)
{
    uint32_t made = 0;
    const int H = h->H, taps = h->taps;
    while (made < want) {
        if (h->out_done) break;
        if (h->end_frames && h->out_idx >= h->end_frames) { h->out_done = true; break; }
        bool can = true;
        for (int k = 0; k < h->ngrp; k++) if (!group_can(h->grp[k], H)) { can = false; break; }
        if (!can) {
            if (h->sched_eof && !h->wait_left) {
                bool fl = true;
                for (int k = 0; k < h->ngrp; k++) {
                    Group &g = h->grp[k];
                    if (g.flushed) continue;
                    fl = false;
                    if (g.acc_n) { group_push(g, g.acc_l / g.acc_n, g.acc_r / g.acc_n); g.acc_n = 0; g.acc_l = g.acc_r = 0; }
                    g.real_end = g.base + g.n;
                    if (!g.pass) { memset(g.buf + 2 * (size_t)g.n, 0, (size_t)H * 4); g.n += H; }
                    g.flushed = true;
                }
                if (!fl) continue;
                h->out_done = true; break;                       /* flushed and still cannot: the data is used up */
            }
            run_slice(h);
            continue;
        }
        int32_t sl = 0, sr = 0;
        for (int k = 0; k < h->ngrp; k++) {
            Group &g = h->grp[k];
            int16_t f[2];
            if (g.pass) { const int16_t *p = g.buf + 2 * (size_t)((g.pos >> 32) - g.base); f[0] = p[0]; f[1] = p[1]; }
            else {
                uint64_t i = g.pos >> 32;
                fmrs_frame(g.coef, taps, g.buf + 2 * (size_t)(i + 1 - H - g.base), (uint32_t)g.pos, f);
            }
            sl += f[0]; sr += f[1];
            g.pos += g.step;
        }
        if (h->end_frames && h->out_idx >= h->len_frames) {       /* fade out: linear gain from 1 to 0 */
            int32_t gn = (int32_t)(h->fade_cur >> 16);
            sl = (int32_t)(((int64_t)sl * gn) >> 15); sr = (int32_t)(((int64_t)sr * gn) >> 15);
            h->fade_cur = h->fade_cur > h->fade_step ? h->fade_cur - h->fade_step : 0;
        }
        int16_t a = clamp16(sl), b = clamp16(sr);
        out[2 * made] = a; out[2 * made + 1] = b;
        if (h->csum) { fnv_i16(h->hash, a); fnv_i16(h->hash, b); h->out_frames++; }
        made++; h->out_idx++;
    }
    return made;
}

static inline uint32_t now_us(void) { return g_port.now_us ? g_port.now_us() : 0; }

/* one producer step: CHUNK frames into the ring (the caller has checked that CHUNK frames of room exist) */
static void produce_fill_ring(Handle *h)
{
    int16_t tmp[2 * CHUNK];
    uint32_t t0 = now_us();
    uint32_t n = produce(h, tmp, CHUNK);
    ST_ADD(h->st.busy_us, now_us() - t0);
    ST_ADD(h->st.frames_made, n);
    uint32_t hd = h->head, mask = h->ring_cap - 1;
    for (uint32_t i = 0; i < n; i++) { uint32_t k = (hd + i) & mask; h->ring[2 * k] = tmp[2 * i]; h->ring[2 * k + 1] = tmp[2 * i + 1]; }
    __atomic_store_n(&h->head, hd + n, __ATOMIC_RELEASE);
    if (n < CHUNK) __atomic_store_n(&h->prod_done, 1, __ATOMIC_RELEASE);
}

/* ---- building and tearing down a handle ----------------------------------------------------------------------------------------------------------------------------- */
static void chips_destroy(Handle *h)
{
    for (int c = 0; c < h->nchips; c++) if (h->chip[c]) { h->chip[c]->~Chip(); h->chip[c] = NULL; }
    for (int i = 0; i < h->ar.nspill; i++) g_port.free(h->ar.spill[i]);
    h->ar.nspill = 0; h->ar.used = 0;
}

static void *arena_take(Handle *h, size_t n) { n = (n + 7) & ~(size_t)7; void *p = h->ar.base + h->ar.used; h->ar.used += n; return p; }

/* fresh chips, fresh song state: used at open and for a restart */
static void build_runtime(Handle *h)
{
    const SrcInfo &si = h->info;
    g_arena = &h->ar;
    for (int c = 0; c < h->nchips; c++) {
        if (si.chip[c].type == CT_OPM) { void *m = arena_take(h, sizeof(Opm)); h->chip[c] = new (m) Opm(si.chip[c].clock); }
        else { void *m = arena_take(h, sizeof(Opn)); h->chip[c] = new (m) Opn(si.chip[c].clock); }
        h->posq[c] = 0; h->gen_n[c] = 0;
    }
    g_arena = NULL;
    for (int k = 0; k < h->ngrp; k++) {
        Group &g = h->grp[k];
        g.flushed = false; g.real_end = 0; g.acc_l = g.acc_r = 0; g.acc_n = 0; g.base = 0;
        if (g.pass) { g.n = 0; g.pos = 0; }
        else { g.n = (uint32_t)(h->H - 1); memset(g.buf, 0, (size_t)g.n * 4); g.pos = (uint64_t)(h->H - 1) << 32; }
    }
    src_rewind(h->src);
    h->wait_left = 0; h->sched_eof = false;
    h->out_idx = 0; h->out_done = false; h->fade_cur = 32768u << 16;
    h->head = h->tail = 0; h->prod_done = 0; h->primed = 0; h->win_out = h->win_sil = 0; h->overloaded = 0;
    h->sum_l = h->sum_r = 0; h->sum_n = 0; h->hash = 2166136261u; h->out_frames = 0;
    memset(&h->st, 0, sizeof h->st);
    h->st.low_water = h->ring_cap; h->st.ring_frames = h->ring_cap; h->st.placement = h->place;
}

static void handle_free(Handle *h)
{
    chips_destroy(h);
    for (int k = 0; k < h->ngrp; k++) { g_port.free(h->grp[k].buf); g_port.free(h->grp[k].coef); }
    g_port.free(h->tmp[0]); g_port.free(h->ring); g_port.free(h->arena);
    g_port.free(h);
}

static int fail(const char *why) { snprintf(g_err, sizeof g_err, "%s", why); return -1; }

static uint32_t fnv_bytes(const void *p, size_t n)
{
    uint32_t h = 2166136261u; const uint8_t *b = (const uint8_t *)p;
    for (size_t i = 0; i < n; i++) h = (h ^ b[i]) * 16777619u;
    return h;
}

static Handle *handle_new(const uint8_t *d, uint32_t n)
{
    g_err[0] = 0;
    Handle *h = (Handle *)g_port.alloc(sizeof(Handle), FMVGM_MEM_FAST);
    if (!h) { fail("no memory"); return NULL; }
    memset(h, 0, sizeof *h);
    int rc;
    if (fmvgm_is_s98(d, n)) rc = s98_open(d, n, h->src, h->info);
    else if (n >= 0x40 && !memcmp(d, "Vgm ", 4)) rc = vgm_open(d, n, h->src, h->info);
    else { fail("not a VGM / S98 file"); g_port.free(h); return NULL; }
    if (rc < 0) { fail(h->info.why[0] ? h->info.why : "cannot open"); g_port.free(h); return NULL; }
    SrcInfo &si = h->info;
    h->nchips = si.n_chips;

    /* the length, counted from the stream itself (headers lie): one walk that does not follow the loop and notes where the loop point lies */
    {
        Src w = h->src; w.scan = 1; w.acc = 0; w.intro_set = 0; w.intro = 0;
        Ev e;
        while (src_next(w, e) != EV_END) {}
        uint64_t tot = w.acc > 0xFFFFFFFFull ? 0xFFFFFFFFull : w.acc;
        si.total_units = (uint32_t)tot;
        si.has_loop = w.intro_set && tot > w.intro;
        si.intro_units = (uint32_t)(w.intro_set ? w.intro : tot);
        si.loop_units = si.has_loop ? (uint32_t)(tot - w.intro) : 0;
        if (!si.has_loop) h->src.loop_off = 0;
    }

    /* the chips: their rates decide the groups */
    size_t arena_need = 64;
    uint32_t raw[FM_MAX_CHIPS]; uint8_t dec[FM_MAX_CHIPS];
    for (int c = 0; c < h->nchips; c++) {
        uint32_t clk = si.chip[c].clock;
        if (si.chip[c].type == CT_OPM) { raw[c] = clk / 64; dec[c] = 1; arena_need += sizeof(Opm) + ARENA_OPM; }
        else { raw[c] = clk / 24; dec[c] = 3; arena_need += sizeof(Opn) + ARENA_OPN; }
    }
    h->arena = (uint8_t *)g_port.alloc(arena_need, FMVGM_MEM_FAST);
    if (!h->arena) { fail("no memory (chips)"); handle_free(h); return NULL; }
    h->ar.base = h->arena; h->ar.cap = arena_need; h->ar.used = 0; h->arena_cap = arena_need;
    /* sample rates: YM2151 clock / 64, YM2203 clock / 24 (ymfm's lowest fidelity) decimated by 3 */
    for (int c = 0; c < h->nchips; c++) {
        int k = 0;
        for (; k < h->ngrp; k++) if (h->grp[k].raw_rate == raw[c] && h->grp[k].dec == dec[c]) break;
        if (k == h->ngrp) { h->grp[k].raw_rate = raw[c]; h->grp[k].dec = dec[c]; h->grp[k].nchips = 0; h->ngrp++; }
        Group &g = h->grp[k]; g.chips[g.nchips++] = (uint8_t)c;
    }
    h->taps = g_taps; h->H = h->taps / 2;
    uint32_t nat = h->grp[0].raw_rate / h->grp[0].dec;
    h->out_rate = g_out_rate ? g_out_rate : nat;
    h->native_rate = nat;
    uint32_t rmax = 0;
    for (int k = 0; k < h->ngrp; k++) if (h->grp[k].raw_rate > rmax) rmax = h->grp[k].raw_rate;
    /* slice: about 256 raw chip frames, at least one time unit */
    uint64_t su = ((uint64_t)256 * si.unit_p) / ((uint64_t)si.unit_q * rmax);
    h->slice_units = su < 1 ? 1 : su > 0xFFFF ? 0xFFFF : (uint32_t)su;
    h->tmp_n = 256;
    h->tmp[0] = (int32_t *)g_port.alloc(2 * (size_t)h->tmp_n * 4 * 2, FMVGM_MEM_FAST);
    if (!h->tmp[0]) { fail("no memory"); handle_free(h); return NULL; }
    h->tmp[1] = h->tmp[0] + 2 * (size_t)h->tmp_n;
    for (int k = 0; k < h->ngrp; k++) {
        Group &g = h->grp[k];
        uint32_t rate_dec = g.raw_rate / g.dec;
        uint64_t sf = ((uint64_t)h->slice_units * rate_dec * si.unit_q + si.unit_p - 1) / si.unit_p + 4;
        if (sf > 120000) { fail("tick too long"); handle_free(h); return NULL; }
        g.slice_max = (uint32_t)sf;
        g.pass = g.dec == 1 && g.raw_rate == h->out_rate;
        g.step = g.pass ? (1ull << 32) : (((uint64_t)g.raw_rate << 32) / ((uint64_t)g.dec * h->out_rate));
        if (!g.pass && (g.step < (1ull << 30) || g.step > (4ull << 32))) { fail("rate ratio out of range"); handle_free(h); return NULL; }
        g.cap = 2 * g.slice_max + 2 * (uint32_t)h->taps + 64;
        size_t bytes = (size_t)g.cap * 4;
        g.buf = (int16_t *)g_port.alloc(bytes, bytes > 8192 ? FMVGM_MEM_BULK : FMVGM_MEM_FAST);
        if (!g.buf) { fail("no memory (fifo)"); handle_free(h); return NULL; }
        if (!g.pass) {
            g.coef = (int16_t *)g_port.alloc((size_t)(FMRS_PHASES + 1) * h->taps * 2, FMVGM_MEM_BULK);
            if (!g.coef) { fail("no memory (filter)"); handle_free(h); return NULL; }
            fmrs_build(g.coef, h->taps, fmrs_cutoff(g.raw_rate / g.dec, h->out_rate), fmrs_beta(h->taps));
            if (k == 0) h->table_hash = fnv_bytes(g.coef, (size_t)(FMRS_PHASES + 1) * h->taps * 2);
        }
    }
    for (int c = 0; c < h->nchips; c++) {
        h->cstep[c] = ((uint64_t)raw[c] * si.unit_q << 32) / si.unit_p;       /* chip frames per time unit, Q32 */
        h->chip[c] = NULL;
    }
    /* length and fade */
    uint64_t ms_per = 1000ull * si.unit_q;
    if (si.has_loop) {
        uint64_t units = (uint64_t)si.intro_units + (uint64_t)g_loops * si.loop_units;
        h->len_ms = (int32_t)(units * ms_per / si.unit_p);
        h->len_frames = (uint64_t)h->len_ms * h->out_rate / 1000;
        h->fade_frames = (uint64_t)FMVGM_FADE_MS * h->out_rate / 1000;
        h->end_frames = h->len_frames + h->fade_frames;
        h->fade_step = h->fade_frames ? (uint32_t)((32768ull << 16) / h->fade_frames) : 0xFFFFFFFFu;
    } else {
        h->len_ms = si.total_units ? (int32_t)((uint64_t)si.total_units * ms_per / si.unit_p) : -1;
    }
    h->csum = g_csum != 0;
    uint64_t rf = (uint64_t)g_ring_ms * h->out_rate / 1000;
    uint32_t cap = 2048; while (cap < rf && cap < (1u << 20)) cap <<= 1;
    h->ring_cap = cap;
    h->ring = (int16_t *)g_port.alloc((size_t)cap * 4, FMVGM_MEM_BULK);
    if (!h->ring) { fail("no memory (ring)"); handle_free(h); return NULL; }
    h->place = FMVGM_PLACE_INLINE;
    build_runtime(h);
    return h;
}

/* ---- the pc_backend_t -------------------------------------------------------------------------------------------------------------------------------------------------- */
static int place_core(int place) { return place == FMVGM_PLACE_CORE1 ? 1 : 0; }

static void migrate(Handle *h)
{
    int want = g_place;
    if (want != FMVGM_PLACE_INLINE && !worker_for(place_core(want))) want = FMVGM_PLACE_INLINE;
    if (want == h->place) return;
    if (h->owned && !park(h)) return;                        /* the old worker has not let go: stay where we are */
    h->place = want; h->st.placement = want; ST_ADD(h->st.migrations, 1);
    if (want != FMVGM_PLACE_INLINE) assign(h, place_core(want));
}

static int be_open(void **hp, const pc_reader_t *r, uint32_t)
{
    *hp = NULL;
    if (!r || !r->mem.p || r->size < 0x20) { fail("no data in memory"); return -1; }
    Handle *h = handle_new(r->mem.p, r->size);
    if (!h) return -1;
    int want = g_place;
    if (want != FMVGM_PLACE_INLINE && !worker_for(place_core(want))) want = FMVGM_PLACE_INLINE;
    h->place = want; h->st.placement = want;
    if (want != FMVGM_PLACE_INLINE) assign(h, place_core(want));
    *hp = h;
    return 0;
}

static void be_close(void *hv)
{
    Handle *h = (Handle *)hv;
    if (!h) return;
    if (h->owned && !park(h)) {                               /* a stuck worker: leak the handle rather than free memory it may still touch */
        g_last = h->st;
        return;
    }
    fmvgm_stats(h, &g_last);
    handle_free(h);
}

static uint32_t be_rate(void *hv) { return ((Handle *)hv)->out_rate; }
static int32_t be_len(void *hv) { return ((Handle *)hv)->len_ms; }
static int32_t be_pos(void *hv) { Handle *h = (Handle *)hv; return (int32_t)((uint64_t)h->st.frames_read * 1000 / h->out_rate); }

static int be_start_track(void *hv, int idx)
{
    Handle *h = (Handle *)hv;
    if (idx != 0) return -1;
    int pl = h->place;
    if (h->owned && !park(h)) return -1;
    chips_destroy(h);
    build_runtime(h);
    h->place = pl;
    if (pl != FMVGM_PLACE_INLINE) assign(h, place_core(pl));
    return 0;
}

static int be_render(void *hv, float *out, int frames)
{
    Handle *h = (Handle *)hv;
    if (frames <= 0) return 0;
    migrate(h);
    if (h->overloaded) return FMVGM_E_OVERLOAD;
    const uint32_t mask = h->ring_cap - 1;
    uint32_t got = 0, want = (uint32_t)frames, starved_frames = 0;
    uint32_t fill0 = __atomic_load_n(&h->head, __ATOMIC_ACQUIRE) - h->tail;
    if (fill0 < ST_GET(h->st.low_water) && h->st.frames_read > 10 * h->ring_cap) __atomic_store_n(&h->st.low_water, fill0, __ATOMIC_RELAXED);      /* not counted for the first ten ring-fulls (about 3.4 s): the player's own ring is prefilled faster than real time and a producer that is only 1.2-1.5x faster than real time needs a second or two to build its lead */
    int waited = 0;
    if (!h->primed) {                                          /* first block: let a worker build up some lead */
        h->primed = 1;
        if (h->place != FMVGM_PLACE_INLINE) {
            uint32_t lead = h->ring_cap / 2 < 4096 ? h->ring_cap / 2 : 4096;
            for (int i = 0; i < 400 && !h->prod_done && __atomic_load_n(&h->head, __ATOMIC_ACQUIRE) - h->tail < lead; i++) g_port.sleep_ms(1);
        }
    }
    while (got < want) {
        uint32_t hd = __atomic_load_n(&h->head, __ATOMIC_ACQUIRE), avail = hd - h->tail;
        if (!avail) {
            if (__atomic_load_n(&h->prod_done, __ATOMIC_ACQUIRE)) break;
            if (h->place == FMVGM_PLACE_INLINE) { produce_fill_ring(h); continue; }
            if (waited < 25) { waited++; g_port.sleep_ms(1); continue; }
            starved_frames = want - got;                       /* the producer is behind: silence for the rest of this block (the clock goes on) */
            memset(out + 2 * (size_t)got, 0, sizeof(float) * 2 * starved_frames);
            got = want;
            break;
        }
        uint32_t m = avail < want - got ? avail : want - got;
        uint32_t t = h->tail;
        for (uint32_t i = 0; i < m; i++) { uint32_t k = (t + i) & mask; out[2 * (size_t)(got + i)] = h->ring[2 * k] * (1.0f / 32768.0f); out[2 * (size_t)(got + i) + 1] = h->ring[2 * k + 1] * (1.0f / 32768.0f); }
        __atomic_store_n(&h->tail, t + m, __ATOMIC_RELEASE);
        got += m;
        ST_ADD(h->st.frames_read, m);
        if (h->place != FMVGM_PLACE_INLINE) g_port.task_wake(((Worker *)h->wk)->task);
    }
    if (starved_frames) {
        uint32_t ms = (uint32_t)((uint64_t)starved_frames * 1000 / h->out_rate);
        ST_ADD(h->st.starved, 1); ST_ADD(h->st.starved_ms, ms); h->win_sil += starved_frames;
    }
    h->win_out += got;
    if ((uint64_t)h->win_out * 1000 >= (uint64_t)g_ov_window * h->out_rate) {
        __atomic_store_n(&h->st.window_ms, (uint32_t)((uint64_t)h->win_sil * 1000 / h->out_rate), __ATOMIC_RELAXED);
        if (g_ov_thr && h->st.window_ms > g_ov_thr) { h->overloaded = 1; h->st.overloaded = 1; }
        h->win_out = h->win_sil = 0;
    }
    if ((uint32_t)(now_us() - h->snap_us) > 250000u) { h->snap_us = now_us(); fmvgm_stats_t t; fmvgm_stats(h, &t); g_last = t; }
    if (h->overloaded) return FMVGM_E_OVERLOAD;
    return (int)got;
}

const pc_backend_t pc_backend_fmvgm = {
    .open = be_open, .render = be_render, .rate = be_rate, .track_count = NULL, .start_track = be_start_track, .length_ms = be_len, .position_ms = be_pos,
    .close = be_close, .byte_pos = NULL, .name = "FM", .bits = NULL, .burst = NULL };

/* ---- the rest of the public surface ------------------------------------------------------------------------------------------------------------------------------------ */
void fmvgm_tags(void *hv, fmvgm_tags_t *t)
{
    Handle *h = (Handle *)hv;
    memset(t, 0, sizeof *t);
    if (!h) return;
    snprintf(t->title, sizeof t->title, "%s", h->info.title); snprintf(t->author, sizeof t->author, "%s", h->info.author);
    snprintf(t->game, sizeof t->game, "%s", h->info.game); snprintf(t->system, sizeof t->system, "%s", h->info.system);
}

int fmvgm_probe(const void *mem, uint32_t size, fmvgm_info_t *o, fmvgm_tags_t *t)
{
    g_err[0] = 0;
    memset(o, 0, sizeof *o);
    if (t) memset(t, 0, sizeof *t);
    const uint8_t *d = (const uint8_t *)mem;
    Src src; SrcInfo si; memset(&si, 0, sizeof si);
    int rc;
    if (fmvgm_is_s98(d, size)) rc = s98_open(d, size, src, si);
    else if (size >= 0x40 && !memcmp(d, "Vgm ", 4)) rc = vgm_open(d, size, src, si);
    else { fail("not a VGM / S98 file"); return -1; }
    if (rc < 0) { fail(si.why[0] ? si.why : "cannot open"); return -1; }
    Src w = src; w.scan = 1;
    Ev e;
    while (src_next(w, e) != EV_END) {}
    uint64_t tot = w.acc;
    bool loop = w.intro_set && tot > w.intro;
    o->format = si.format; o->n_chips = si.n_chips;
    for (int c = 0; c < si.n_chips && c < 4; c++) {
        snprintf(o->chip[c].name, sizeof o->chip[c].name, "%s", fm_chip_name(si.chip[c].type));
        o->chip[c].clock = si.chip[c].clock;
        o->chip[c].native_rate = si.chip[c].type == CT_OPM ? si.chip[c].clock / 64 : si.chip[c].clock / 72;
    }
    o->out_rate = g_out_rate ? g_out_rate : o->chip[0].native_rate; o->loops = loop; o->taps = g_taps; o->placement = g_place;
    uint64_t ms_per = 1000ull * si.unit_q;
    o->length_ms = loop ? (int32_t)(((uint64_t)w.intro + (uint64_t)g_loops * (tot - w.intro)) * ms_per / si.unit_p) : tot ? (int32_t)(tot * ms_per / si.unit_p) : -1;
    o->loop_ms = loop ? (int32_t)((tot - w.intro) * ms_per / si.unit_p) : 0;
    if (t) { snprintf(t->title, sizeof t->title, "%s", si.title); snprintf(t->author, sizeof t->author, "%s", si.author); snprintf(t->game, sizeof t->game, "%s", si.game); snprintf(t->system, sizeof t->system, "%s", si.system); }
    return 0;
}

void fmvgm_info(void *hv, fmvgm_info_t *o)
{
    Handle *h = (Handle *)hv;
    memset(o, 0, sizeof *o);
    if (!h) return;
    o->format = h->info.format; o->n_chips = h->nchips;
    for (int c = 0; c < h->nchips && c < 4; c++) {
        snprintf(o->chip[c].name, sizeof o->chip[c].name, "%s", fm_chip_name(h->info.chip[c].type));
        o->chip[c].clock = h->info.chip[c].clock;
        o->chip[c].native_rate = h->info.chip[c].type == CT_OPM ? h->info.chip[c].clock / 64 : h->info.chip[c].clock / 72;
    }
    o->out_rate = h->out_rate; o->loops = h->info.has_loop; o->length_ms = h->len_ms;
    o->loop_ms = h->info.has_loop ? (int32_t)((uint64_t)h->info.loop_units * 1000 * h->info.unit_q / h->info.unit_p) : 0;
    o->taps = h->grp[0].pass ? 0 : h->taps; o->placement = h->place;
    o->chip_bytes = (uint32_t)h->ar.used; o->chip_bytes_cap = (uint32_t)h->ar.cap; o->chip_spill = h->ar.nspill;
}

void fmvgm_stats(void *hv, fmvgm_stats_t *s)
{
    Handle *h = (Handle *)hv;
    if (!h) { *s = g_last; return; }
    const fmvgm_stats_t &t = h->st;
    s->frames_made = ST_GET(t.frames_made); s->frames_read = ST_GET(t.frames_read); s->starved = ST_GET(t.starved); s->starved_ms = ST_GET(t.starved_ms);
    s->window_ms = ST_GET(t.window_ms); s->full_waits = ST_GET(t.full_waits); s->low_water = ST_GET(t.low_water); s->ring_frames = t.ring_frames;
    s->busy_us = ST_GET(t.busy_us); s->migrations = ST_GET(t.migrations); s->placement = h->place;
    s->made_ms = (uint32_t)((uint64_t)s->frames_made * 1000 / h->out_rate);
    s->overloaded = h->overloaded;
}

int fmvgm_sums(void *hv, fmvgm_sums_t *s)
{
    Handle *h = (Handle *)hv;
    if (!h || !h->csum) return -1;
    s->chip_sum_l = h->sum_l; s->chip_sum_r = h->sum_r; s->chip_samples = h->sum_n;
    s->out_hash = h->hash; s->out_frames = h->out_frames; s->table_hash = h->table_hash;
    return 0;
}

uint64_t fmvgm_bench(void *hv, uint32_t seconds, uint32_t *busy_us)
{
    Handle *h = (Handle *)hv;
    int16_t tmp[2 * CHUNK];
    uint64_t limit = seconds ? (uint64_t)seconds * h->out_rate : ~0ull, total = 0;
    uint32_t busy = 0;
    while (total < limit) {
        uint32_t t0 = now_us();
        uint32_t n = produce(h, tmp, CHUNK);
        busy += now_us() - t0;
        total += n;
        if (n < CHUNK) break;
    }
    if (busy_us) *busy_us = busy;
    return total;
}
