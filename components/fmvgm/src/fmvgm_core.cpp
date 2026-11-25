// SPDX-License-Identifier: LGPL-2.1-or-later
/* fmvgm core: the chips (ymfm), the song clock, rate groups with their integer resampler, the producer, the ring and the worker, the pc_backend_t.
 * Layout of the data flow (docs/design.md, "Split-core rendering"):
 *   one LANE per core that renders chips (a worker on core 0, a worker on core 1, or the consumer inline); a lane owns its chips, its own cursor in the command stream, and its own ring.
 *   lane: Src (VGM / S98 commands, every lane reads the whole stream and acts on its own chips only; waits are shared by construction) --writes, waits--> chips (native rate)
 *         --raw int32 L/R, gain--> one group per chip (decimated 3:1 for OPN / OPNA, clamped to int16) --FIFO--> polyphase resampler --> sum of the lane's chips --> lane ring (int32)
 *   consumer (render): sum of the lanes' rings, fade, clamp --> int16 --> float.
 * Every chip is resampled on its own, so the output does not depend on which chips share a lane: bit-identical whatever the assignment. All integer. */
#include "fmvgm.h"
#include "fmvgm_int.h"
#include "fmvgm_resamp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <new>

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
static uint8_t g_plan[FM_MAX_CHIPS]; static volatile int g_plan_n;
void fmvgm_set_plan(const uint8_t *place, int n)
{
    if (n < 0 || n > FM_MAX_CHIPS || !place) n = 0;
    for (int i = 0; i < n; i++) g_plan[i] = place[i] > 2 ? 0 : place[i];
    g_plan_n = n;
}
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

/* the port's allocator, for the chip layer and the PCM cores (small things in internal RAM where there is a choice, ROM images in bulk memory) */
void *fm_alloc(size_t n, fmvgm_mem_t kind) { return g_port.alloc(n, kind); }
void fm_free(void *p) { if (p) g_port.free(p); }
extern "C" {
void *fmvgm_pcm_calloc(size_t n, size_t sz)
{
    size_t b = n * sz + 8;
    uint8_t *p = (uint8_t *)g_port.alloc(b, b >= 4096 ? FMVGM_MEM_BULK : FMVGM_MEM_FAST);
    if (!p) return NULL;
    memset(p, 0, b); *(size_t *)p = b - 8;
    return p + 8;
}
void fmvgm_pcm_free(void *q) { if (q) g_port.free((uint8_t *)q - 8); }
void *fmvgm_pcm_realloc(void *q, size_t n)
{
    if (!n) { fmvgm_pcm_free(q); return NULL; }
    uint8_t *p = (uint8_t *)g_port.alloc(n + 8, n + 8 >= 4096 ? FMVGM_MEM_BULK : FMVGM_MEM_FAST);
    if (!p) return NULL;                                   /* like realloc: the old block stays valid */
    *(size_t *)p = n;
    if (q) { size_t o = *(size_t *)((uint8_t *)q - 8); memcpy(p + 8, q, o < n ? o : n); fmvgm_pcm_free(q); }
    return p + 8;
}
}

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

/* ---- per-chip resampling group, lane, handle ------------------------------------------------------------------------------------------------------------------------- */
namespace {
struct Group {
    uint32_t raw_rate, max_raw; uint8_t dec;
    int16_t *buf; uint32_t cap, n; uint64_t base;      /* FIFO of int16 stereo frames [base, base + n) in absolute indices (real frame 0 is index H - 1) */
    uint64_t pos, step;                                /* Q32 absolute position of the next output (its centre), input frames per output frame */
    bool pass, flushed; uint64_t real_end;
    int32_t acc_l, acc_r; int acc_n;                   /* the decimator */
    uint32_t slice_max;                                /* most frames a slice can push */
    int16_t *coef; bool own_coef;                      /* this chip's polyphase table (NULL when pass; shared between chips of the same rates) */
};
const uint32_t CHUNK = 256;                            /* output frames per producer step */
const int MAX_LANES = 3;
}

struct Handle;
struct Lane {
    Handle *h; int place;                              /* FMVGM_PLACE_* of this lane */
    Src src;                                           /* this lane's own cursor in the command stream */
    int nchips; uint8_t cidx[FM_MAX_CHIPS]; uint32_t smask;   /* handle chip indices on this lane, and a mask of them */
    Group grp[FM_MAX_CHIPS];
    uint64_t posq[FM_MAX_CHIPS], cstep[FM_MAX_CHIPS]; uint32_t gen_n[FM_MAX_CHIPS];
    int32_t *tmp; uint32_t tmp_n;                      /* raw batch buffer of one chip */
    uint32_t wait_left, slice_units; bool sched_eof;
    uint64_t out_idx; bool out_done;
    int64_t sum_l, sum_r; uint64_t sum_n;              /* checksum of the chips' raw output (this lane's chips) */
    /* ring of int32 stereo frames: head written by the producer, tail by the consumer */
    int32_t *ring; volatile uint32_t head, tail; volatile int prod_done;
    /* worker ownership */
    volatile int owned, req_park, parked; void *wk;
    uint32_t busy_us, frames_made, full_waits;
};

struct Handle {
    SrcInfo info; Src src;
    int nchips; Chip *chip[FM_MAX_CHIPS]; uint8_t lane_of[FM_MAX_CHIPS];
    uint8_t *arena; size_t arena_cap; Arena ar;
    Lane lane[MAX_LANES]; int nlanes;
    int taps, H; uint32_t table_hash;
    uint32_t out_rate, native_rate, ring_cap;
    uint64_t out_idx, len_frames, end_frames, fade_frames; uint32_t fade_cur, fade_step;
    int32_t len_ms;
    bool csum; uint32_t hash; uint64_t out_frames;
    int primed; uint32_t win_out, win_sil;
    fmvgm_stats_t st;
    volatile int overloaded;
    uint32_t snap_us;
    int split;                                         /* nlanes > 1: placements are fixed for this handle's life */
};

/* statistics counters have one writer at a time (the producer's, or the consumer's); the other side only reads them: relaxed loads and stores, no read-modify-write (the handle may sit in PSRAM) */
#define ST_ADD(f, v) __atomic_store_n(&(f), (f) + (uint32_t)(v), __ATOMIC_RELAXED)
#define ST_GET(f) __atomic_load_n(&(f), __ATOMIC_RELAXED)

/* ---- worker threads ----------------------------------------------------------------------------------------------------------------------------------------------- */
namespace {
/* One per core, created on first use, never deleted. `job` is the lane it works on (NULL: idle); `gen` is bumped by the consumer each time it changes `job`. */
struct Worker { void *task; Lane *volatile job; volatile uint32_t gen; };
Worker g_wk[2];
fmvgm_stats_t g_last;
}

static void produce_fill_ring(Lane *L);
/* The worker's whole protocol with the consumer. It touches a lane (and the handle behind it) only between reading it from `job` and acknowledging a park request; after the acknowledgement it only
 * compares the generation (no pointer to the lane is dereferenced again), so the consumer may free the handle the moment `parked` is set and `job` cleared. */
static void worker_main(void *arg)
{
    Worker *w = (Worker *)arg;
    uint32_t w_chunks = 0;
    for (;;) {
        Lane *L = __atomic_load_n(&w->job, __ATOMIC_ACQUIRE);
        if (!L) { g_port.task_wait(100); continue; }
        if (__atomic_load_n(&L->req_park, __ATOMIC_ACQUIRE)) {
            uint32_t g0 = __atomic_load_n(&w->gen, __ATOMIC_ACQUIRE);
            __atomic_store_n(&L->parked, 1, __ATOMIC_RELEASE);            /* from here on L is not ours any more */
            while (__atomic_load_n(&w->gen, __ATOMIC_ACQUIRE) == g0) g_port.task_wait(20);
            continue;
        }
        if (__atomic_load_n(&L->prod_done, __ATOMIC_ACQUIRE)) { g_port.task_wait(30); continue; }
        uint32_t free_ = L->h->ring_cap - (L->head - __atomic_load_n(&L->tail, __ATOMIC_ACQUIRE));
        if (free_ < CHUNK) { ST_ADD(L->full_waits, 1); g_port.task_wait(6); continue; }
        produce_fill_ring(L);
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

/* the consumer side of a worker: stop it touching the lane (waits for the end of its current chunk). false: the worker did not answer (the caller must not free the handle) */
static bool park_lane(Lane *L)
{
    if (!L->owned) return true;
    Worker *w = &g_wk[L->wk == &g_wk[1] ? 1 : 0];
    __atomic_store_n(&L->req_park, 1, __ATOMIC_RELEASE);
    for (int i = 0; i < 3000; i++) {
        if (__atomic_load_n(&L->parked, __ATOMIC_ACQUIRE)) break;
        g_port.task_wake(w->task);
        if (g_port.sleep_ms) g_port.sleep_ms(1);
    }
    if (!__atomic_load_n(&L->parked, __ATOMIC_ACQUIRE)) return false;
    __atomic_store_n(&w->job, (Lane *)NULL, __ATOMIC_RELEASE);
    __atomic_store_n(&w->gen, w->gen + 1, __ATOMIC_RELEASE);               /* lets the worker out of its wait */
    g_port.task_wake(w->task);
    L->owned = 0; L->wk = NULL;
    return true;
}
static bool park_all(Handle *h) { bool ok = true; for (int i = 0; i < h->nlanes; i++) if (!park_lane(&h->lane[i])) ok = false; return ok; }

static void assign(Lane *L, int core)
{
    Worker *w = &g_wk[core];
    L->req_park = 0; L->parked = 0; L->wk = w; L->owned = 1;
    __atomic_store_n(&w->job, L, __ATOMIC_RELEASE);
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

static void lane_rate_update(Lane *L, int k)
{
    Handle *h = L->h; Group &g = L->grp[k]; Chip *c = h->chip[L->cidx[k]];
    uint32_t raw = c->raw_rate;
    if (raw > g.max_raw) raw = g.max_raw;                                   /* the FIFO is sized for the highest rate the chip may switch to */
    if (g.pass || !raw) return;
    uint64_t step = ((uint64_t)raw << 32) / ((uint64_t)g.dec * h->out_rate);
    if (step < (1ull << 28) || step > (4ull << 32)) return;
    g.raw_rate = raw; g.step = step;
    L->cstep[k] = ((uint64_t)raw * h->src.unit_q << 32) / h->src.unit_p;
}

/* advances the lane by one slice: runs commands up to the next wait, then generates its chips for at most `slice_units` of it */
static void run_slice(Lane *L)
{
    Handle *h = L->h;
    for (int k = 0; k < L->nchips; k++) { Group &g = L->grp[k]; if (g.cap - g.n < g.slice_max) group_compact(g, h->H); }
    while (!L->wait_left && !L->sched_eof) {
        Ev e;
        int kind = src_next(L->src, e);
        if (kind == EV_WAIT) { if (e.units) L->wait_left = e.units; continue; }
        if (kind == EV_END) { L->sched_eof = true; continue; }
        if (kind == EV_DAC) { if (e.slot < h->nchips && (L->smask >> e.slot & 1)) h->chip[e.slot]->dac(); if (e.units) L->wait_left = e.units; continue; }
        if (e.slot >= h->nchips || !(L->smask >> e.slot & 1)) continue;           /* another lane's chip: it acts on it, this lane only keeps time */
        Chip *c = h->chip[e.slot];
        if (kind == EV_WRITE) {
            c->write(e.port, e.reg, e.val);
            if (c->rate_dirty) { c->rate_dirty = false; for (int k = 0; k < L->nchips; k++) if (L->cidx[k] == e.slot) lane_rate_update(L, k); }
        } else if (kind == EV_DATA) {
            if (!(c->type == CT_OPN2 && e.cls == MC_PCM)) c->block(e.cls, e.total, e.start, e.p, e.len);     /* the PCM bank was filled at open; the blocks of a loop are not appended again */
        } else if (kind == EV_SEEK) c->dac_seek(e.start);
    }
    if (!L->wait_left) return;
    uint32_t u = L->wait_left < L->slice_units ? L->wait_left : L->slice_units;
    L->wait_left -= u;
    for (int k = 0; k < L->nchips; k++) {
        L->posq[k] += (uint64_t)u * L->cstep[k];
        uint32_t need = (uint32_t)(L->posq[k] >> 32) - L->gen_n[k];
        L->gen_n[k] += need;
        Group &g = L->grp[k]; Chip *c = h->chip[L->cidx[k]];
        uint32_t left = need;
        while (left) {
            uint32_t m = left > L->tmp_n ? L->tmp_n : left;
            c->gen(L->tmp, m);
            if (h->csum) {                                                          /* the chip's raw output, before gain, decimation and resampling */
                int64_t sl = 0, sr = 0; const int32_t *p = L->tmp;
                for (uint32_t i = 0; i < m; i++) { sl += p[2 * i]; sr += p[2 * i + 1]; }
                L->sum_l += sl; L->sum_r += sr; L->sum_n += m;
            }
            int32_t *p = L->tmp;
            if (c->gain != 256) for (uint32_t i = 0; i < 2 * m; i++) p[i] = (int32_t)(((int64_t)p[i] * c->gain) >> 8);
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

/* make up to `want` frames of this lane's chips (the sum of their resampled outputs, int32, before fade and clamp); fewer than asked only at the end of the song */
static uint32_t produce(Lane *L, int32_t *out, uint32_t want)
{
    Handle *h = L->h;
    uint32_t made = 0;
    const int H = h->H, taps = h->taps;
    while (made < want) {
        if (L->out_done) break;
        if (h->end_frames && L->out_idx >= h->end_frames) { L->out_done = true; break; }
        bool can = true;
        for (int k = 0; k < L->nchips; k++) if (!group_can(L->grp[k], H)) { can = false; break; }
        if (!can) {
            if (L->sched_eof && !L->wait_left) {
                bool fl = true;
                for (int k = 0; k < L->nchips; k++) {
                    Group &g = L->grp[k];
                    if (g.flushed) continue;
                    fl = false;
                    if (g.acc_n) { group_push(g, g.acc_l / g.acc_n, g.acc_r / g.acc_n); g.acc_n = 0; g.acc_l = g.acc_r = 0; }
                    g.real_end = g.base + g.n;
                    if (!g.pass) { memset(g.buf + 2 * (size_t)g.n, 0, (size_t)H * 4); g.n += H; }
                    g.flushed = true;
                }
                if (!fl) continue;
                L->out_done = true; break;                       /* flushed and still cannot: the data is used up */
            }
            run_slice(L);
            continue;
        }
        int32_t sl = 0, sr = 0;
        for (int k = 0; k < L->nchips; k++) {
            Group &g = L->grp[k];
            int16_t f[2];
            if (g.pass) { const int16_t *p = g.buf + 2 * (size_t)((g.pos >> 32) - g.base); f[0] = p[0]; f[1] = p[1]; }
            else {
                uint64_t i = g.pos >> 32;
                fmrs_frame(g.coef, taps, g.buf + 2 * (size_t)(i + 1 - H - g.base), (uint32_t)g.pos, f);
            }
            sl += f[0]; sr += f[1];
            g.pos += g.step;
        }
        out[2 * made] = sl; out[2 * made + 1] = sr;
        made++; L->out_idx++;
    }
    return made;
}

static inline uint32_t now_us(void) { return g_port.now_us ? g_port.now_us() : 0; }

/* one producer step: CHUNK frames into the lane's ring (the caller has checked that CHUNK frames of room exist) */
static void produce_fill_ring(Lane *L)
{
    int32_t tmp[2 * CHUNK];
    uint32_t t0 = now_us();
    uint32_t n = produce(L, tmp, CHUNK);
    ST_ADD(L->busy_us, now_us() - t0);
    ST_ADD(L->frames_made, n);
    uint32_t hd = L->head, mask = L->h->ring_cap - 1;
    for (uint32_t i = 0; i < n; i++) { uint32_t k = (hd + i) & mask; L->ring[2 * k] = tmp[2 * i]; L->ring[2 * k + 1] = tmp[2 * i + 1]; }
    __atomic_store_n(&L->head, hd + n, __ATOMIC_RELEASE);
    if (n < CHUNK) __atomic_store_n(&L->prod_done, 1, __ATOMIC_RELEASE);
}

/* ---- building and tearing down a handle ----------------------------------------------------------------------------------------------------------------------------- */
static void chips_destroy(Handle *h)
{
    for (int c = 0; c < h->nchips; c++) if (h->chip[c]) { h->chip[c]->~Chip(); h->chip[c] = NULL; }
    for (int i = 0; i < h->ar.nspill; i++) g_port.free(h->ar.spill[i]);
    h->ar.nspill = 0; h->ar.used = 0;
}

static void *arena_take(Handle *h, size_t n) { n = (n + 7) & ~(size_t)7; void *p = h->ar.base + h->ar.used; h->ar.used += n; return p; }

static int fail(const char *why) { snprintf(g_err, sizeof g_err, "%s", why); return -1; }

/* fresh chips, fresh song state: used at open and for a restart. 0, or -1 with g_err set */
static int build_runtime(Handle *h)
{
    const SrcInfo &si = h->info;
    g_arena = &h->ar;
    for (int c = 0; c < h->nchips; c++) {
        void *m = arena_take(h, fm_chip_size(si.chip[c].type));
        h->chip[c] = fm_chip_make(m, si.chip[c].type, si.chip[c].clock, si.chip[c].variant);
        if (!h->chip[c]->ok()) { g_arena = NULL; return fail("no memory (chip data)"); }
    }
    g_arena = NULL;
    for (int c = 0; c < h->nchips; c++) if (si.format == SRC_S98) h->chip[c]->start_s98();
    for (int li = 0; li < h->nlanes; li++) {
        Lane *L = &h->lane[li];
        for (int k = 0; k < L->nchips; k++) {
            Group &g = L->grp[k];
            g.flushed = false; g.real_end = 0; g.acc_l = g.acc_r = 0; g.acc_n = 0; g.base = 0;
            Chip *c = h->chip[L->cidx[k]];
            c->rate_dirty = false;
            if (g.pass) { g.n = 0; g.pos = 0; }
            else { g.n = (uint32_t)(h->H - 1); memset(g.buf, 0, (size_t)g.n * 4); g.pos = (uint64_t)(h->H - 1) << 32; }
            g.raw_rate = c->raw_rate > g.max_raw ? g.max_raw : c->raw_rate;
            L->posq[k] = 0; L->gen_n[k] = 0;
            L->cstep[k] = ((uint64_t)g.raw_rate * si.unit_q << 32) / si.unit_p;       /* chip frames per time unit, Q32 */
            if (!g.pass) g.step = ((uint64_t)g.raw_rate << 32) / ((uint64_t)g.dec * h->out_rate);
        }
        L->src = h->src; src_rewind(L->src);
        L->wait_left = 0; L->sched_eof = false; L->out_idx = 0; L->out_done = false;
        L->head = L->tail = 0; L->prod_done = 0; L->sum_l = L->sum_r = 0; L->sum_n = 0;
        L->busy_us = L->frames_made = L->full_waits = 0;
        /* ROM / bank data blocks of the whole stream go to the chips now, so that allocation (and any refusal) happens at open; the lane applies them again when its cursor passes them (same
         * ranges: nothing new is stored) */
        Src w = L->src; w.scan = 1; Ev e; int k;
        while ((k = src_next(w, e)) != EV_END) {
            if (k != EV_DATA || e.slot >= h->nchips || !(L->smask >> e.slot & 1)) continue;
            if (h->chip[e.slot]->block(e.cls, e.total, e.start, e.p, e.len) < 0) return fail(e.cls == MC_PCM && h->chip[e.slot]->type != CT_OPN2 ? "sample ROM does not fit" : "ADPCM data does not fit");
        }
    }
    h->out_idx = 0; h->fade_cur = 32768u << 16;
    h->primed = 0; h->win_out = h->win_sil = 0; h->overloaded = 0;
    h->hash = 2166136261u; h->out_frames = 0;
    memset(&h->st, 0, sizeof h->st);
    h->st.low_water = h->ring_cap; h->st.ring_frames = h->ring_cap;
    h->st.placement = h->lane[0].place;
    return 0;
}

static void handle_free(Handle *h)
{
    chips_destroy(h);
    for (int li = 0; li < h->nlanes; li++) {
        Lane *L = &h->lane[li];
        for (int k = 0; k < L->nchips; k++) { g_port.free(L->grp[k].buf); if (L->grp[k].own_coef) g_port.free(L->grp[k].coef); }
        g_port.free(L->tmp); g_port.free(L->ring);
    }
    g_port.free(h->arena);
    g_port.free(h);
}

static uint32_t fnv_bytes(const void *p, size_t n)
{
    uint32_t h = 2166136261u; const uint8_t *b = (const uint8_t *)p;
    for (size_t i = 0; i < n; i++) h = (h ^ b[i]) * 16777619u;
    return h;
}

static int place_core(int place) { return place == FMVGM_PLACE_CORE1 ? 1 : 0; }

static Handle *handle_new(const uint8_t *d, uint32_t n)
{
    g_err[0] = 0;
    Handle *h = (Handle *)g_port.alloc(sizeof(Handle), FMVGM_MEM_FAST);
    if (!h) { fail("no memory"); return NULL; }
    memset(h, 0, sizeof *h);
    int rc;
    if (fmvgm_is_s98(d, n)) rc = s98_open(d, n, h->src, h->info, 0);
    else if (n >= 0x40 && !memcmp(d, "Vgm ", 4)) rc = vgm_open(d, n, h->src, h->info, g_drop_mask);
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

    /* lanes: one per distinct placement among the chips (the plan from the host, else everything where fmvgm_set_placement says); a worker that cannot be had means inline */
    int place[FM_MAX_CHIPS];
    for (int c = 0; c < h->nchips; c++) {
        int pl = c < g_plan_n ? g_plan[c] : g_place;
        if (pl != FMVGM_PLACE_INLINE && !worker_for(place_core(pl))) pl = FMVGM_PLACE_INLINE;
        place[c] = pl;
    }
    for (int c = 0; c < h->nchips; c++) {
        int li = 0;
        for (; li < h->nlanes; li++) if (h->lane[li].place == place[c]) break;
        if (li == h->nlanes) { h->lane[li].place = place[c]; h->lane[li].h = h; h->nlanes++; }
        Lane *L = &h->lane[li];
        L->cidx[L->nchips++] = (uint8_t)c; L->smask |= 1u << c; h->lane_of[c] = (uint8_t)li;
    }
    h->split = h->nlanes > 1 || g_plan_n > 0;           /* a handle built from a plan keeps its placement; only a plain single-lane handle follows fmvgm_set_placement while it plays */

    /* the chips' arena (ymfm's operator objects), then the rates */
    size_t arena_need = 64;
    uint32_t raw[FM_MAX_CHIPS], maxraw[FM_MAX_CHIPS]; uint8_t dec[FM_MAX_CHIPS];
    for (int c = 0; c < h->nchips; c++) {
        fm_chip_rate(si.chip[c].type, si.chip[c].clock, si.chip[c].variant, &raw[c], &dec[c], &maxraw[c]);
        arena_need += ((fm_chip_size(si.chip[c].type) + 7) & ~(size_t)7) + fm_chip_arena(si.chip[c].type);
    }
    h->arena = (uint8_t *)g_port.alloc(arena_need, FMVGM_MEM_FAST);
    if (!h->arena) { fail("no memory (chips)"); handle_free(h); return NULL; }
    h->ar.base = h->arena; h->ar.cap = arena_need; h->ar.used = 0; h->arena_cap = arena_need;
    h->taps = g_taps; h->H = h->taps / 2;
    uint32_t nat = raw[0] / dec[0];
    h->out_rate = g_out_rate ? g_out_rate : nat;
    h->native_rate = nat;
    {
        uint64_t rf = (uint64_t)g_ring_ms * h->out_rate / 1000;
        uint32_t cap = 2048; while (cap < rf && cap < (1u << 20)) cap <<= 1;
        h->ring_cap = cap;
    }
    int16_t *tables[FM_MAX_CHIPS]; uint32_t tabkey[FM_MAX_CHIPS]; int ntab = 0; bool first_table = true;
    for (int li = 0; li < h->nlanes; li++) {
        Lane *L = &h->lane[li];
        uint32_t rmax = 0;
        for (int k = 0; k < L->nchips; k++) if (maxraw[L->cidx[k]] > rmax) rmax = maxraw[L->cidx[k]];
        /* slice: about 256 raw chip frames of the lane's fastest chip, at least one time unit */
        uint64_t su = ((uint64_t)256 * si.unit_p) / ((uint64_t)si.unit_q * rmax);
        L->slice_units = su < 1 ? 1 : su > 0xFFFF ? 0xFFFF : (uint32_t)su;
        L->tmp_n = 256;
        L->tmp = (int32_t *)g_port.alloc(2 * (size_t)L->tmp_n * 4, FMVGM_MEM_FAST);
        L->ring = (int32_t *)g_port.alloc((size_t)h->ring_cap * 8, FMVGM_MEM_BULK);
        if (!L->tmp || !L->ring) { fail("no memory (ring)"); handle_free(h); return NULL; }
        for (int k = 0; k < L->nchips; k++) {
            Group &g = L->grp[k]; int c = L->cidx[k];
            g.raw_rate = raw[c]; g.max_raw = maxraw[c]; g.dec = dec[c];
            uint32_t rate_dec = g.max_raw / g.dec;
            uint64_t sf = ((uint64_t)L->slice_units * rate_dec * si.unit_q + si.unit_p - 1) / si.unit_p + 4;
            if (sf > 120000) { fail("tick too long"); handle_free(h); return NULL; }
            g.slice_max = (uint32_t)sf;
            g.pass = g.dec == 1 && g.raw_rate == h->out_rate && g.max_raw == g.raw_rate;
            g.step = g.pass ? (1ull << 32) : (((uint64_t)g.raw_rate << 32) / ((uint64_t)g.dec * h->out_rate));
            if (!g.pass && (g.step < (1ull << 28) || g.step > (4ull << 32))) { fail("rate ratio out of range"); handle_free(h); return NULL; }
            g.cap = 2 * g.slice_max + 2 * (uint32_t)h->taps + 64;
            size_t bytes = (size_t)g.cap * 4;
            g.buf = (int16_t *)g_port.alloc(bytes, bytes > 8192 ? FMVGM_MEM_BULK : FMVGM_MEM_FAST);
            if (!g.buf) { fail("no memory (fifo)"); handle_free(h); return NULL; }
            g.coef = NULL; g.own_coef = false;
            if (!g.pass) {
                uint32_t key = g.raw_rate / g.dec;
                for (int t = 0; t < ntab; t++) if (tabkey[t] == key) g.coef = tables[t];     /* chips of the same rate share one read-only table */
                if (!g.coef) {
                    g.coef = (int16_t *)g_port.alloc((size_t)(FMRS_PHASES + 1) * h->taps * 2, FMVGM_MEM_BULK);
                    if (!g.coef) { fail("no memory (filter)"); handle_free(h); return NULL; }
                    g.own_coef = true;
                    fmrs_build(g.coef, h->taps, fmrs_cutoff(g.raw_rate / g.dec, h->out_rate), fmrs_beta(h->taps));
                    tables[ntab] = g.coef; tabkey[ntab++] = key;
                    if (first_table) { h->table_hash = fnv_bytes(g.coef, (size_t)(FMRS_PHASES + 1) * h->taps * 2); first_table = false; }
                }
            }
        }
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
    if (build_runtime(h) != 0) { handle_free(h); return NULL; }
    return h;
}

/* ---- the pc_backend_t -------------------------------------------------------------------------------------------------------------------------------------------------- */
static void assign_all(Handle *h)
{
    for (int i = 0; i < h->nlanes; i++) if (h->lane[i].place != FMVGM_PLACE_INLINE) assign(&h->lane[i], place_core(h->lane[i].place));
}

static void migrate(Handle *h)
{
    if (h->split) return;                                      /* chips on several cores: placements are fixed for this handle (a new open follows the plan again) */
    Lane *L = &h->lane[0];
    int want = g_place;
    if (want != FMVGM_PLACE_INLINE && !worker_for(place_core(want))) want = FMVGM_PLACE_INLINE;
    if (want == L->place) return;
    if (L->owned && !park_lane(L)) return;                     /* the old worker has not let go: stay where we are */
    L->place = want; h->st.placement = want; ST_ADD(h->st.migrations, 1);
    if (want != FMVGM_PLACE_INLINE) assign(L, place_core(want));
}

static int be_open(void **hp, const pc_reader_t *r, uint32_t)
{
    *hp = NULL;
    if (!r || !r->mem.p || r->size < 0x20) { fail("no data in memory"); return -1; }
    Handle *h = handle_new(r->mem.p, r->size);
    if (!h) return -1;
    assign_all(h);
    *hp = h;
    return 0;
}

static void be_close(void *hv)
{
    Handle *h = (Handle *)hv;
    if (!h) return;
    if (!park_all(h)) {                                       /* a stuck worker: leak the handle rather than free memory it may still touch */
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
    if (!park_all(h)) return -1;
    chips_destroy(h);
    if (build_runtime(h) != 0) return -1;
    assign_all(h);
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
    uint32_t fill0 = ~0u;
    for (int l = 0; l < h->nlanes; l++) { uint32_t f = __atomic_load_n(&h->lane[l].head, __ATOMIC_ACQUIRE) - h->lane[l].tail; if (f < fill0) fill0 = f; }
    if (fill0 < ST_GET(h->st.low_water) && h->st.frames_read > 10 * h->ring_cap) __atomic_store_n(&h->st.low_water, fill0, __ATOMIC_RELAXED);      /* not counted for the first ten ring-fulls (about 3.4 s): the player's own ring is prefilled faster than real time and a producer that is only 1.2-1.5x faster than real time needs a second or two to build its lead */
    int waited = 0;
    if (!h->primed) {                                          /* first block: let the workers build up some lead */
        h->primed = 1;
        uint32_t lead = h->ring_cap / 2 < 4096 ? h->ring_cap / 2 : 4096;
        for (int l = 0; l < h->nlanes; l++) {
            Lane *L = &h->lane[l];
            if (L->place == FMVGM_PLACE_INLINE) continue;
            for (int i = 0; i < 400 && !L->prod_done && __atomic_load_n(&L->head, __ATOMIC_ACQUIRE) - L->tail < lead; i++) g_port.sleep_ms(1);
        }
    }
    while (got < want) {
        if (h->end_frames && h->out_idx >= h->end_frames) break;                       /* the fade is over */
        uint32_t m = want - got;
        if (h->end_frames && h->end_frames - h->out_idx < m) m = (uint32_t)(h->end_frames - h->out_idx);
        bool ended = false, wait = false;
        for (int l = 0; l < h->nlanes; l++) {
            Lane *L = &h->lane[l];
            uint32_t avail = __atomic_load_n(&L->head, __ATOMIC_ACQUIRE) - L->tail;
            if (!avail && !__atomic_load_n(&L->prod_done, __ATOMIC_ACQUIRE) && L->place == FMVGM_PLACE_INLINE) { produce_fill_ring(L); avail = L->head - L->tail; }
            if (!avail) { if (__atomic_load_n(&L->prod_done, __ATOMIC_ACQUIRE)) ended = true; else wait = true; }
            if (avail < m) m = avail;
        }
        if (ended) break;
        if (wait) {
            if (waited < 25) { waited++; g_port.sleep_ms(1); continue; }
            starved_frames = want - got;                       /* a producer is behind: silence for the rest of this block (the clock goes on) */
            memset(out + 2 * (size_t)got, 0, sizeof(float) * 2 * starved_frames);
            got = want;
            break;
        }
        const int nl = h->nlanes;
        for (uint32_t i = 0; i < m; i++) {
            int32_t sl = 0, sr = 0;
            for (int l = 0; l < nl; l++) { const Lane *L = &h->lane[l]; uint32_t k = (L->tail + i) & mask; sl += L->ring[2 * k]; sr += L->ring[2 * k + 1]; }
            if (h->end_frames && h->out_idx >= h->len_frames) {       /* fade out: linear gain from 1 to 0 */
                int32_t gn = (int32_t)(h->fade_cur >> 16);
                sl = (int32_t)(((int64_t)sl * gn) >> 15); sr = (int32_t)(((int64_t)sr * gn) >> 15);
                h->fade_cur = h->fade_cur > h->fade_step ? h->fade_cur - h->fade_step : 0;
            }
            int16_t a = clamp16(sl), b = clamp16(sr);
            out[2 * (size_t)(got + i)] = a * (1.0f / 32768.0f); out[2 * (size_t)(got + i) + 1] = b * (1.0f / 32768.0f);
            if (h->csum) { fnv_i16(h->hash, a); fnv_i16(h->hash, b); h->out_frames++; }
            h->out_idx++;
        }
        for (int l = 0; l < nl; l++) { Lane *L = &h->lane[l]; __atomic_store_n(&L->tail, L->tail + m, __ATOMIC_RELEASE); }
        got += m;
        ST_ADD(h->st.frames_read, m);
        for (int l = 0; l < nl; l++) if (h->lane[l].place != FMVGM_PLACE_INLINE && h->lane[l].wk) g_port.task_wake(((Worker *)h->lane[l].wk)->task);
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

static const char *chip_label(const ChipDesc &c)
{
    if (c.type == CT_OPNB && (c.variant & 1)) return "YM2610B";
    if (c.type == CT_OPN2 && (c.variant & 1)) return "YM3438";
    return fm_chip_name(c.type);
}
static void fill_chips(fmvgm_info_t *o, const SrcInfo &si)
{
    o->format = si.format; o->n_chips = si.n_chips;
    for (int c = 0; c < si.n_chips && c < 4; c++) {
        snprintf(o->chip[c].name, sizeof o->chip[c].name, "%s", chip_label(si.chip[c]));
        o->chip[c].clock = si.chip[c].clock;
        uint32_t raw, mx; uint8_t dec; fm_chip_rate(si.chip[c].type, si.chip[c].clock, si.chip[c].variant, &raw, &dec, &mx);
        o->chip[c].native_rate = raw / dec;
        o->chip[c].type = si.chip[c].type;
        if (si.chip[c].type == CT_OPNA && !fm_rom2608_ready()) o->rom_missing = 1;
    }
    o->dropped = si.dropped; snprintf(o->dropped_names, sizeof o->dropped_names, "%s", si.dropped_names);
}

int fmvgm_probe(const void *mem, uint32_t size, fmvgm_info_t *o, fmvgm_tags_t *t) { return fmvgm_probe_drop(mem, size, o, t, g_drop_mask); }

int fmvgm_probe_drop(const void *mem, uint32_t size, fmvgm_info_t *o, fmvgm_tags_t *t, uint64_t drop)
{
    g_err[0] = 0;
    memset(o, 0, sizeof *o);
    if (t) memset(t, 0, sizeof *t);
    const uint8_t *d = (const uint8_t *)mem;
    Src src; SrcInfo si; memset(&si, 0, sizeof si);
    int rc;
    if (fmvgm_is_s98(d, size)) rc = s98_open(d, size, src, si, 0);
    else if (size >= 0x40 && !memcmp(d, "Vgm ", 4)) rc = vgm_open(d, size, src, si, drop);
    else { fail("not a VGM / S98 file"); return -1; }
    if (rc < 0) { fail(si.why[0] ? si.why : "cannot open"); return -1; }
    Src w = src; w.scan = 1;
    Ev e;
    uint32_t adpcm = 0;
    while (src_next(w, e) != EV_END) if (e.kind == EV_DATA && e.cls != MC_PCM) adpcm += e.len;
    uint64_t tot = w.acc;
    bool loop = w.intro_set && tot > w.intro;
    fill_chips(o, si);
    o->adpcm_bytes = adpcm;
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
    fill_chips(o, h->info);
    for (int c = 0; c < h->nchips && c < 4; c++) { o->chip[c].place = (uint8_t)h->lane[h->lane_of[c]].place; if (h->chip[c] && (h->chip[c]->flags & CF_ROM_MISSING)) o->rom_missing = 1; }
    o->out_rate = h->out_rate; o->loops = h->info.has_loop; o->length_ms = h->len_ms;
    o->loop_ms = h->info.has_loop ? (int32_t)((uint64_t)h->info.loop_units * 1000 * h->info.unit_q / h->info.unit_p) : 0;
    o->taps = h->lane[0].grp[0].pass ? 0 : h->taps; o->placement = h->lane[0].place; o->lanes = h->nlanes;
    o->chip_bytes = (uint32_t)h->ar.used; o->chip_bytes_cap = (uint32_t)h->ar.cap; o->chip_spill = h->ar.nspill;
}

void fmvgm_stats(void *hv, fmvgm_stats_t *s)
{
    Handle *h = (Handle *)hv;
    if (!h) { *s = g_last; return; }
    const fmvgm_stats_t &t = h->st;
    s->frames_made = ST_GET(h->lane[0].frames_made); s->frames_read = ST_GET(t.frames_read); s->starved = ST_GET(t.starved); s->starved_ms = ST_GET(t.starved_ms);
    s->window_ms = ST_GET(t.window_ms); s->low_water = ST_GET(t.low_water); s->ring_frames = t.ring_frames;
    s->busy_us = 0; s->full_waits = 0; s->lanes = h->nlanes;
    for (int l = 0; l < MAX_LANES; l++) { s->lane_busy_us[l] = 0; s->lane_place[l] = -1; }
    for (int l = 0; l < h->nlanes; l++) {
        s->lane_busy_us[l] = ST_GET(h->lane[l].busy_us); s->lane_place[l] = h->lane[l].place;
        s->busy_us += s->lane_busy_us[l]; s->full_waits += ST_GET(h->lane[l].full_waits);
    }
    s->migrations = ST_GET(t.migrations); s->placement = h->lane[0].place;
    s->made_ms = (uint32_t)((uint64_t)s->frames_made * 1000 / h->out_rate);
    s->overloaded = h->overloaded;
}

int fmvgm_sums(void *hv, fmvgm_sums_t *s)
{
    Handle *h = (Handle *)hv;
    if (!h || !h->csum) return -1;
    s->chip_sum_l = s->chip_sum_r = 0; s->chip_samples = 0;
    for (int l = 0; l < h->nlanes; l++) { s->chip_sum_l += h->lane[l].sum_l; s->chip_sum_r += h->lane[l].sum_r; s->chip_samples += h->lane[l].sum_n; }
    s->out_hash = h->hash; s->out_frames = h->out_frames; s->table_hash = h->table_hash;
    return 0;
}

/* the whole file on the calling thread, no ring: every lane in turn is run to the same frame count and the lanes are mixed exactly as render() does (host tests, `dbg fm bench`) */
uint64_t fmvgm_bench(void *hv, uint32_t seconds, uint32_t *busy_us)
{
    Handle *h = (Handle *)hv;
    int32_t tmp[3][2 * CHUNK];
    uint64_t limit = seconds ? (uint64_t)seconds * h->out_rate : ~0ull, total = 0;
    uint32_t busy = 0;
    while (total < limit) {
        uint32_t n[MAX_LANES];
        for (int l = 0; l < h->nlanes; l++) { uint32_t t0 = now_us(); n[l] = produce(&h->lane[l], tmp[l], CHUNK); busy += now_us() - t0; }
        uint32_t m = n[0];
        for (int l = 1; l < h->nlanes; l++) if (n[l] < m) m = n[l];
        for (uint32_t i = 0; i < m; i++) {
            int32_t sl = 0, sr = 0;
            for (int l = 0; l < h->nlanes; l++) { sl += tmp[l][2 * i]; sr += tmp[l][2 * i + 1]; }
            if (h->end_frames && h->out_idx >= h->len_frames) {
                int32_t gn = (int32_t)(h->fade_cur >> 16);
                sl = (int32_t)(((int64_t)sl * gn) >> 15); sr = (int32_t)(((int64_t)sr * gn) >> 15);
                h->fade_cur = h->fade_cur > h->fade_step ? h->fade_cur - h->fade_step : 0;
            }
            if (h->csum) { fnv_i16(h->hash, clamp16(sl)); fnv_i16(h->hash, clamp16(sr)); h->out_frames++; }
            h->out_idx++;
        }
        total += m;
        if (m < CHUNK) break;
    }
    if (busy_us) *busy_us = busy;
    return total;
}
