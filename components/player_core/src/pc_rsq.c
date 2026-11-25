// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_rsq.h"
#include <math.h>
#include <string.h>

#define T PC_RSQ_TAPS
#define P PC_RSQ_PHASES
#define KAISER_BETA 8.6f

struct pc_rsq {
    uint32_t in_rate, out_rate;
    uint32_t rem;               /* output position inside the current input frame, in 1/out_rate units (< out_rate) */
    int w;                      /* start of the window in (history ++ input): window = T frames, w <= in_frames to compute */
    float inv_out;
    float tab[(P + 1) * T];     /* row p: the kernel at fraction p / P; row P = fraction 1 (the neighbour of row P - 1) */
    float hist[2 * T];          /* the last T input frames (zeros at the start) */
};

size_t pc_rsq_size(void) { return (sizeof(struct pc_rsq) + 7) & ~(size_t)7; }

static uint32_t gcd(uint32_t a, uint32_t b) { while (b) { uint32_t t = a % b; a = b; b = t; } return a; }

/* Kaiser window I0(beta sqrt(1 - u^2)) by its power series (at most 17 terms to 1e-7 at beta 8.6) */
static float i0_kaiser(float u)
{
    static const float inv_k2[24] = {                         /* 1 / k^2: no division in the loop (init cost on the board) */
        0, 1.0f, 1.0f / 4, 1.0f / 9, 1.0f / 16, 1.0f / 25, 1.0f / 36, 1.0f / 49, 1.0f / 64, 1.0f / 81, 1.0f / 100, 1.0f / 121,
        1.0f / 144, 1.0f / 169, 1.0f / 196, 1.0f / 225, 1.0f / 256, 1.0f / 289, 1.0f / 324, 1.0f / 361, 1.0f / 400, 1.0f / 441,
        1.0f / 484, 1.0f / 529 };
    float y = KAISER_BETA * KAISER_BETA * 0.25f * (1.0f - u * u), term = 1.0f, sum = 1.0f;
    for (int k = 1; k < 24; k++) {                            /* y <= 18.5: the term is below 1e-7 of the sum by k = 17 */
        term *= y * inv_k2[k];
        sum += term;
        if (term < sum * 1e-7f) break;
    }
    return sum;
}

int pc_rsq_init(pc_rsq_t *q, uint32_t in_rate, uint32_t out_rate)
{
    if (!in_rate || !out_rate || (uint64_t)in_rate > 4ull * out_rate || (uint64_t)out_rate > 4ull * in_rate) return -1;
    uint32_t g = gcd(in_rate, out_rate), lo = in_rate < out_rate ? in_rate : out_rate;
    uint32_t num = lo / g, den = in_rate / g;                 /* r = min(in, out) / in = num / den: the cutoff in input-Nyquist units */
    uint64_t period = 512ull * den;                           /* one turn of sin(pi r tau), tau = m / 256: m * num / (512 den) turns */
    uint32_t step = (uint32_t)(period - (256ull * num) % period);   /* tau -> tau - 1 */
    const float two_pi_over = 6.2831853071795864f / (float)period, inv_i0b = 1.0f / i0_kaiser(0.0f);
    memset(q, 0, sizeof *q);
    q->in_rate = in_rate;
    q->out_rate = out_rate;
    q->inv_out = 1.0f / (float)out_rate;
    q->w = 1;
    /* rows 0..P/2 are computed; row P - p is row p reversed (the kernel is even) */
    for (int p = 0; p <= P / 2; p++) {
        float *row = q->tab + p * T, sum = 0;
        int m0 = p + P * (T / 2 - 1);                         /* tau of tap 0 = m0 / P */
        uint64_t x = ((uint64_t)m0 * num) % period;           /* its angle in period units; each following tap is one frame earlier */
        for (int j = 0; j < T; j++) {
            float tau = (float)(m0 - P * j) * (1.0f / P), h;      /* exact: P is a power of two */
            if (m0 - P * j == 0) h = (float)num / (float)den;
            else h = sinf((float)x * two_pi_over) / (3.14159265358979f * tau);
            row[j] = h * (i0_kaiser(tau * (1.0f / (T / 2))) * inv_i0b);
            sum += row[j];
            x += step;
            if (x >= period) x -= period;
        }
        float k = 1.0f / sum;
        for (int j = 0; j < T; j++) row[j] *= k;
    }
    for (int p = 0; p < P / 2; p++)
        for (int j = 0; j < T; j++) q->tab[(P - p) * T + (T - 1 - j)] = q->tab[p * T + j];
    return 0;
}

int pc_rsq_process(pc_rsq_t *q, const float *in, int in_frames, float *out, int out_cap, int *consumed)
{
    int n = 0, w = q->w;
    uint32_t rem = q->rem, out_rate = q->out_rate, in_rate = q->in_rate;
    float win[2 * T];
    while (n < out_cap && w <= in_frames) {
        const float *x;
        if (w >= T) {
            x = in + 2 * (w - T);
        } else {                                              /* the window starts in the history */
            memcpy(win, q->hist + 2 * w, sizeof(float) * 2 * (size_t)(T - w));
            memcpy(win + 2 * (T - w), in, sizeof(float) * 2 * (size_t)w);
            x = win;
        }
        uint32_t num = rem << 8, ph = num / out_rate;
        float fr = (float)(num - ph * out_rate) * q->inv_out;
        const float *a = q->tab + ph * T, *b = a + T;
        float l0 = 0, r0 = 0, l1 = 0, r1 = 0;
        for (int k = 0; k < T; k++) {
            float l = x[2 * k], r = x[2 * k + 1];
            l0 += a[k] * l; r0 += a[k] * r;
            l1 += b[k] * l; r1 += b[k] * r;
        }
        out[2 * n] = l0 + fr * (l1 - l0);
        out[2 * n + 1] = r0 + fr * (r1 - r0);
        n++;
        rem += in_rate;
        while (rem >= out_rate) { rem -= out_rate; w++; }
    }
    int c = w < in_frames ? w : in_frames;                    /* take only what the next window still needs when out_cap ran out */
    if (c >= T) {
        memcpy(q->hist, in + 2 * (c - T), sizeof(float) * 2 * T);
    } else if (c > 0) {
        memmove(q->hist, q->hist + 2 * c, sizeof(float) * 2 * (size_t)(T - c));
        memcpy(q->hist + 2 * (T - c), in, sizeof(float) * 2 * (size_t)c);
    }
    q->w = w - c;
    q->rem = rem;
    *consumed = c;
    return n;
}

void pc_rsq_reset(pc_rsq_t *q)
{
    q->rem = 0;
    q->w = 1;                                                 /* as pc_rsq_init */
    memset(q->hist, 0, sizeof q->hist);
}

/* ---- hires T10b: live table + spare, the spare built off the audio task (pc_rsq.h has the ownership rules) ---- */
static int pair_eq(pc_rsq_pair_t a, pc_rsq_pair_t b) { return a.in == b.in && a.out == b.out; }
static int pair_needs_table(pc_rsq_pair_t p) { return p.in && p.out && p.in != p.out; }
/* the hand-off's loads and stores (pc_rsq.h: "the hand-off, step by step"): acquire / release on aligned 32-bit words, plain lw / sw plus fences
 * on RISC-V (allowed in PSRAM; no read-modify-write). test_rsq_order builds this file with PC_RSQ_LOAD / PC_RSQ_STORE recording each one. */
#ifndef PC_RSQ_LOAD
#define PC_RSQ_LOAD(x, mo) __atomic_load_n(&(x), (mo))
#define PC_RSQ_STORE(x, v, mo) __atomic_store_n(&(x), (v), (mo))
#endif
#define LD(x) PC_RSQ_LOAD(x, __ATOMIC_ACQUIRE)
#define ST(x, v) PC_RSQ_STORE(x, v, __ATOMIC_RELEASE)

static uint32_t rq_tag(uint32_t gen, uint32_t swaps) { return gen * 65536u + (swaps & 0xffffu); }   /* 10c fix 1: a reclaim names the publish AND the swap count */

pc_rsq_pick_t pc_rsq_pick(pc_rsq_pair_t want, pc_rsq_pair_t live, pc_rsq_pair_t ready)
{
    if (want.in == want.out) return PC_RSQ_PASS;
    if (pair_eq(want, live)) return PC_RSQ_USE;
    if (pair_needs_table(ready) && pair_eq(want, ready)) return PC_RSQ_SWAP;
    return PC_RSQ_WAIT;
}

pc_rsq_pair_t pc_rsq_build_pick(pc_rsq_pair_t req, pc_rsq_pair_t now, pc_rsq_pair_t next, pc_rsq_pair_t live)
{
    if (pair_needs_table(req) && !pair_eq(req, live)) return req;
    if (pair_needs_table(now) && !pair_eq(now, live)) return now;
    if (pair_needs_table(next) && !pair_eq(next, live)) return next;
    pc_rsq_pair_t none = { 0, 0 };
    return none;
}

int pc_rsq_spare_unwanted(pc_rsq_pair_t spare, pc_rsq_pair_t req, pc_rsq_pair_t now, pc_rsq_pair_t next)
{
    return !pair_eq(spare, req) && !pair_eq(spare, now) && !pair_eq(spare, next);
}

void pc_rsq_swap_setup(pc_rsq_swap_t *s, pc_rsq_t *a, pc_rsq_t *b)
{
    memset(s, 0, sizeof *s);
    if (!a || !b) return;                                     /* no blocks: every resampled block waits (pc_rsq_swap_spare never offers one) */
    s->blk[0] = a;
    s->blk[1] = b;
}

pc_rsq_pick_t pc_rsq_swap_take(pc_rsq_swap_t *s, uint32_t in, uint32_t out, pc_rsq_t **q)
{
    pc_rsq_pair_t want = { in, out }, none = { 0, 0 };
    int live = s->live, ready = LD(s->ready);                 /* live: only this task writes it */
    pc_rsq_pick_t k = pc_rsq_pick(want, s->blk[live] ? s->pair[live] : none, ready ? s->pair[!live] : none);
    *q = NULL;
    if (k == PC_RSQ_PASS || k == PC_RSQ_USE) {
        if (LD(s->req.in) || LD(s->req.out)) { ST(s->req.in, 0u); ST(s->req.out, 0u); }   /* fix 2: this block does not wait: nothing is asked for */
        if (ready && LD(s->reclaim) == rq_tag(s->gen, LD(s->swaps))) ST(s->ready, 0);   /* fix 2: the builder wants this ready spare back (nobody wants its pair): no flip.
                                                                   * gen is read after ready's acquire; a reclaim of an older gen never matches */
    }
    if (k == PC_RSQ_PASS) {
        ST(s->fresh, 1);                                      /* the next resampled block does not blend in the tail from before this stretch */
    } else if (k == PC_RSQ_USE) {
        if (LD(s->fresh)) { ST(s->fresh, 0); pc_rsq_reset(s->blk[live]); }   /* cleared before the reset: a restart asked meanwhile is not lost */
        *q = s->blk[live];
    } else if (k == PC_RSQ_SWAP) {
        pc_rsq_reset(s->blk[!live]);                          /* 10c: a table swapped back in was used before: silent history (a fresh build is already) */
        ST(s->live, !live);
        ST(s->swaps, s->swaps + 1);                           /* after the flip (audio task is the only writer): a reclaim decided before it carries the old count and never matches */
        ST(s->fresh, 0);
        if (LD(s->req.in) || LD(s->req.out)) { ST(s->req.in, 0u); ST(s->req.out, 0u); }   /* served: nothing to build until the next wait */
        ST(s->reclaim, 0u);                                   /* 10c: a reclaim aimed at the block that just became live must not take the cached one */
        int old_valid = pair_needs_table(s->pair[live]);      /* the table that was live: a real pair (cache) or a never-built block (free) */
        /* 10c: ready stays 1 - the old live table is now the ready spare, a cache for the way back (44.1k <-> 48k source is a pointer swap).
         * The builder evicts it only when it has another pair to build (pc_rsq_swap_spare). A never-built old block is simply free: ready 0. */
        if (!old_valid) ST(s->ready, 0);                      /* after the flip, last (step 4a) */
        *q = s->blk[!live];
    } else {
        if (LD(s->req.in) != in || LD(s->req.out) != out) { ST(s->req.in, in); ST(s->req.out, out); }
        if (ready) ST(s->ready, 0);                           /* the spare holds a pair nobody wants now: back to the builder, unused */
    }
    return k;
}

pc_rsq_t *pc_rsq_swap_spare(pc_rsq_swap_t *s, pc_rsq_pair_t now, pc_rsq_pair_t next, pc_rsq_pair_t *build)
{
    if (!s->blk[0]) return NULL;
    uint32_t swaps = LD(s->swaps);                            /* before live: if this is the new count, the flip is already visible */
    int ready = LD(s->ready);
    int live = LD(s->live);                                   /* stable while ready is 0 (the audio task flips it only on a ready spare) */
    pc_rsq_pair_t req = { LD(s->req.in), LD(s->req.out) };
    if (ready) {                                              /* the audio task decides; nobody writes the spare. Fix 2: a spare nobody wants is reclaimed */
        pc_rsq_pair_t spare = s->pair[!live], want = pc_rsq_build_pick(req, now, next, s->pair[live]);
        if (pair_needs_table(want) && !pair_eq(want, spare) && pc_rsq_spare_unwanted(spare, req, now, next))   /* 10c: a cached spare stays until something else needs the room */
            ST(s->reclaim, rq_tag(s->gen, swaps));                           /* gen, pair: this task's own fields. A flip meanwhile */
        return NULL;                                          /* only makes this reclaim name a gen that is no longer ready: ignored */
    }
    *build = pc_rsq_build_pick(req, now, next, s->pair[live]);
    if (!pair_needs_table(*build)) return NULL;
    s->pair[!live].in = s->pair[!live].out = 0;               /* being rebuilt: no pair until it is published */
    return s->blk[!live];
}

void pc_rsq_swap_publish(pc_rsq_swap_t *s, pc_rsq_pair_t built)
{
    int live = LD(s->live);
    s->pair[!live] = built;
    s->gen++;                                                 /* this task's own count; read by the audio task only after ready 1 */
    ST(s->ready, 1);                                          /* release, last (step 2): the table, its pair and gen before the flag */
}

void pc_rsq_swap_restart(pc_rsq_swap_t *s) { ST(s->fresh, 1); }

int pc_rsq_swap_has(pc_rsq_swap_t *s, pc_rsq_pair_t p)
{
    if (!s->blk[0] || !pair_needs_table(p)) return 0;
    int ready = LD(s->ready), live = LD(s->live);
    return pair_eq(s->pair[live], p) || (ready && pair_eq(s->pair[!live], p));
}

int pc_rsq_swap_spare_busy(pc_rsq_swap_t *s) { return s->blk[0] && LD(s->ready); }
