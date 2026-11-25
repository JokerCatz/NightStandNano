// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Shared parts of the fx app family files (private to components/player_core/src). */
#include <math.h>
#include <string.h>
#include "pc/pc_dsp.h"
#include "pc/pc_fxapp.h"

static inline float fx_db2lin(float db) { return powf(10.0f, db / 20.0f); }

/* the values an app works from: a non-finite value is the parameter's default (an import or a kv file may hold
 * "nan"), a finite one is clamped to [lo, hi] (q = 0 or a cutoff of 1e30 would make NaN coefficients) */
static inline void fx_clean(float *dst, const float *v, const pc_fxparam_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        float x = isfinite(v[i]) ? v[i] : p[i].def;
        dst[i] = x < p[i].lo ? p[i].lo : x > p[i].hi ? p[i].hi : x;
    }
}

/* first line of a set()/init() that reads `v`: from here on `v` is the cleaned copy */
#define FX_CLEAN_V(params, np) float fx_v_[PC_FXP_MAX]; fx_clean(fx_v_, v, params, np); v = fx_v_

static inline void fx_bq_clear(pc_biquad_t *bq)
{
    memset(bq->s1, 0, sizeof bq->s1);
    memset(bq->s2, 0, sizeof bq->s2);
}

/* A 0 dB biquad's history only rings out the old content (plus float rounding of about 1e-7 of the signal while it
 * plays, so it never reaches 0): below -120 dB it may stop and be cleared (a step of at most 1e-6). */
static inline bool fx_bq_quiet(const pc_biquad_t *bq)
{
    const float e = 1e-6f;
    return fabsf(bq->s1[0]) < e && fabsf(bq->s1[1]) < e && fabsf(bq->s2[0]) < e && fabsf(bq->s2[1]) < e;
}

static inline bool fx_ramping(const pc_fx_ramp_t *r) { return r->cur != r->tgt; }
static inline void fx_ramp_jump(pc_fx_ramp_t *r, float v) { r->cur = r->tgt = v; }

/* left *= gl, right *= gr, each ramping over the block (in place, interleaved stereo); a plain multiply when still */
static inline void fx_gain2_run(float *x, int frames, pc_fx_ramp_t *gl, pc_fx_ramp_t *gr)
{
    if (!fx_ramping(gl) && !fx_ramping(gr)) {
        float a = gl->cur, b = gr->cur;
        for (int i = 0; i < frames; i++) { x[2 * i] *= a; x[2 * i + 1] *= b; }
    } else {
        for (int i = 0; i < frames; i++) { x[2 * i] *= pc_fx_ramp_at(gl, i, frames); x[2 * i + 1] *= pc_fx_ramp_at(gr, i, frames); }
    }
    pc_fx_ramp_end(gl);
    pc_fx_ramp_end(gr);
}
static inline void fx_gain_run(float *x, int frames, pc_fx_ramp_t *g) { fx_gain2_run(x, frames, g, g); }

/* the fits() of an app with no rate-sized memory */
static inline bool fx_fits_any(const void *st, uint32_t rate, const float *v) { (void)st; (void)v; return rate > 0; }

/* interleaved stereo frames processed in chunks of this many by apps that need a scratch copy (in their state block) */
#define FX_CHUNK 256

/* A biquad whose values (type, fc, gain, q) slew towards a target: at each block start the values move by at most one
 * bounded step and the coefficients are recomputed with the history kept (one filter, no cross-fade). A big jump thus
 * becomes a run of small steps; a type change goes through 0 dB (where every type is the same 0 dB filter). Within the
 * block after a step the coefficients glide sample by sample (fx_bqs_run), so no step lands on one sample.
 * Steps (chosen against the level-hold and click tests, test_fxapps): 1 dB, x1.16 in frequency, x1.1 in Q per block,
 * so every full-range change arrives within 50 blocks (48 dB: 48; 20 Hz..20 kHz: 47; Q 0.1..10: 49). */
#define FX_SLEW_DB 1.0f
#define FX_SLEW_FC 1.16f
#define FX_SLEW_Q  1.1f
#ifndef FX_DRAIN_BLOCKS
#define FX_DRAIN_BLOCKS 200   /* see fx_bqs_drained */
#endif
#ifndef FX_DRAIN_FADE
#define FX_DRAIN_FADE 0.95f   /* per sample in the fade block: 0.95^480 = 2e-11; the rounding it lets in stays below 1e-5 */
#endif
typedef struct { pc_biquad_t bq; float c0[5]; float fc, g, q, tfc, tg, tq; uint16_t idle; char type, ttype; bool glide, fade, faded; } fx_bqs_t;
/* idle / fade / faded: the forced stop of a LEAVING filter (fx_bqs_drained). They exist only while the filter sits at
 * 0 dB with nothing pending: a new target, a step, design/rest (init, rate, reset, a join) and fx_bqs_stay (the app
 * says "not leaving": a band below the count, a tone word that is on) all clear them. */
static inline void fx_bqs_undrain(fx_bqs_t *s) { s->idle = 0; s->fade = s->faded = false; }
static inline void fx_bqs_stay(fx_bqs_t *s) { fx_bqs_undrain(s); }

static inline void fx_bqs_design(fx_bqs_t *s, char type, float fc, float g, float q, uint32_t rate)   /* there now, at rest */
{
    s->type = s->ttype = type;
    s->fc = s->tfc = fc;
    s->g = s->tg = g;
    s->q = s->tq = q;
    s->glide = false;
    fx_bqs_undrain(s);
    pc_biquad_design(&s->bq, type, fc, g, q, rate);
}
/* a peak/shelf at 0 dB and at rest (a 0 dB biquad has no history of its own): a band or word that joins starts here */
static inline void fx_bqs_rest(fx_bqs_t *s, char type, float fc, float q, uint32_t rate) { fx_bqs_design(s, type, fc, 0, q, rate); }
static inline void fx_bqs_target(fx_bqs_t *s, char type, float fc, float g, float q)
{
    if (s->ttype != type || s->tfc != fc || s->tg != g || s->tq != q) fx_bqs_undrain(s);   /* a new target: not draining */
    s->ttype = type; s->tfc = fc; s->tg = g; s->tq = q;
}
static inline bool fx_bqs_moving(const fx_bqs_t *s) { return s->type != s->ttype || s->fc != s->tfc || s->g != s->tg || s->q != s->tq; }

static inline float fx_slew_lin(float v, float t, float d) { return t > v + d ? v + d : t < v - d ? v - d : t; }
static inline float fx_slew_log(float v, float t, float r)
{
    if (!(v > 0) || !(t > 0)) return t;
    return t > v * r ? v * r : t < v / r ? v / r : t;
}
/* at the block start: one step towards the target, coefficients recomputed, history kept; nothing when there */
static inline void fx_bqs_step(fx_bqs_t *s, uint32_t rate)
{
    if (!fx_bqs_moving(s)) return;
    fx_bqs_undrain(s);
    s->g = fx_slew_lin(s->g, s->type != s->ttype ? 0.0f : s->tg, FX_SLEW_DB);
    if (s->type != s->ttype && s->g == 0.0f) s->type = s->ttype;
    s->fc = fx_slew_log(s->fc, s->tfc, FX_SLEW_FC);
    s->q = fx_slew_log(s->q, s->tq, FX_SLEW_Q);
    pc_biquad_t n = s->bq;
    pc_biquad_design(&n, s->type, s->fc, s->g, s->q, rate);
    s->c0[0] = s->bq.b0; s->c0[1] = s->bq.b1; s->c0[2] = s->bq.b2; s->c0[3] = s->bq.a1; s->c0[4] = s->bq.a2;
    s->bq.b0 = n.b0; s->bq.b1 = n.b1; s->bq.b2 = n.b2; s->bq.a1 = n.a1; s->bq.a2 = n.a2;
    s->glide = true;
}

/* run k frames at frame `off` of a block of `frames`. In the block after a step the coefficients glide linearly from
 * the old set to the new one, sample by sample (a straight line between two stable biquads stays stable: the
 * (a1, a2) stability region is a triangle); otherwise the plain biquad (bit-exact with a fixed filter). */
static inline void fx_bqs_run(fx_bqs_t *s, float *x, int k, int off, int frames)
{
    if (s->fade) {                                 /* the last block of a leaving 0 dB filter: its history fades out */
        pc_biquad_t *bq = &s->bq;
        for (int c = 0; c < 2; c++) {
            float s1 = bq->s1[c], s2 = bq->s2[c];
            for (int i = 0; i < k; i++) {
                float in = x[2 * i + c], y = bq->b0 * in + s1;
                s1 = (bq->b1 * in - bq->a1 * y + s2) * FX_DRAIN_FADE;
                s2 = (bq->b2 * in - bq->a2 * y) * FX_DRAIN_FADE;
                x[2 * i + c] = y;
            }
            bq->s1[c] = s1;
            bq->s2[c] = s2;
        }
        if (off + k >= frames) { s->fade = false; s->faded = true; }   /* one block only, whoever runs it */
        return;
    }
    if (!s->glide) { pc_biquad_run(&s->bq, x, k); return; }
    pc_biquad_t *bq = &s->bq;
    const float *c0 = s->c0;
    const float d0 = bq->b0 - c0[0], d1 = bq->b1 - c0[1], d2 = bq->b2 - c0[2], d3 = bq->a1 - c0[3], d4 = bq->a2 - c0[4];
    const float inv = 1.0f / (float)frames;   /* one divide per call */
    float s1[2] = { bq->s1[0], bq->s1[1] }, s2[2] = { bq->s2[0], bq->s2[1] };
    for (int i = 0; i < k; i++) {
        float b0, b1, b2, a1, a2;
        if (off + i + 1 >= frames) { b0 = bq->b0; b1 = bq->b1; b2 = bq->b2; a1 = bq->a1; a2 = bq->a2; }   /* the last frame: exactly the new set */
        else {
            float t = (float)(off + i + 1) * inv;
            b0 = c0[0] + d0 * t; b1 = c0[1] + d1 * t; b2 = c0[2] + d2 * t; a1 = c0[3] + d3 * t; a2 = c0[4] + d4 * t;
        }
        for (int c = 0; c < 2; c++) {                                     /* one coefficient set for both sides */
            float in = x[2 * i + c], y = b0 * in + s1[c];
            s1[c] = b1 * in - a1 * y + s2[c];
            s2[c] = b2 * in - a2 * y;
            x[2 * i + c] = y;
        }
    }
    bq->s1[0] = s1[0]; bq->s1[1] = s1[1];
    bq->s2[0] = s2[0]; bq->s2[1] = s2[1];
}
static inline void fx_bqs_end(fx_bqs_t *s) { s->glide = false; }   /* after the block */

/* Call after the block of a leaving filter (target 0 dB). True: stop it now (and clear it). It stops when its
 * history is quiet, or else after FX_DRAIN_BLOCKS blocks at 0 dB, through one fade block that multiplies the history
 * by FX_DRAIN_FADE per sample (a time constant of 20 samples). Why the second way: a loud steady tone at the band's own frequency keeps float
 * rounding in a 0 dB filter's history for good (coherent at the resonance, up to about 1e-3 of the signal for 50 Hz
 * Q10), so "quiet" may never come and a plain clear would step by that much. FX_DRAIN_BLOCKS: the slowest pole the
 * tables allow (20 Hz Q10 at 0 dB, r = 1 - 1.31e-4 per sample at 48 kHz) has a time constant of 16 blocks; 200 blocks
 * (2 s) = 12.5 of them, so the old content has rung out (e^-12.5 = 4e-6) and only the rounding residue is left,
 * which the fade block takes down smoothly (measured: task-1-report.md, fix round 2). */
static inline bool fx_bqs_drained(fx_bqs_t *s)
{
    if (fx_bqs_moving(s)) { fx_bqs_undrain(s); return false; }
    if (s->faded || fx_bq_quiet(&s->bq)) { fx_bqs_undrain(s); return true; }
    if (s->idle < 0xFFFF) s->idle++;
    if (s->idle >= FX_DRAIN_BLOCKS) s->fade = true;     /* the next block fades the history, then it stops */
    return false;
}

/* test hooks (host_test/test_fxapps.c): biquads that cost work right now */
int fx_tone_running(const void *st);
int fx_eq_running(const void *st);

/* ---- parts shared by the ported apps (pc_fxapp_parts.c): the fx-lab project's parts.rs, in C ---- */
float pc_fxp_ln_cosh(float x);                          /* ln(cosh(x)), safe for large |x| */
typedef struct { float r, x1, y1; } pc_fxp_dc_t;       /* one-pole high-pass that takes out what does not move */
void  pc_fxp_dc_set(pc_fxp_dc_t *d, float hz, float rate);   /* the coefficient only; history untouched */
float pc_fxp_dc_tick(pc_fxp_dc_t *d, float x);
uint64_t pc_fxp_hash(uint64_t seed, uint64_t count);    /* SplitMix64 finaliser of (seed, count): no state, same answer after a seek */
float pc_fxp_unit(uint64_t seed, uint64_t count);       /* [0, 1) from the top 24 bits of the hash */
/* a stream of random numbers from a counter run through pc_fxp_hash: restart with the same seed and it says the same again */
typedef struct { uint64_t seed, count; } pc_fxp_rng_t;
void  pc_fxp_rng_init(pc_fxp_rng_t *r, uint64_t seed);
float pc_fxp_rng_next_unit(pc_fxp_rng_t *r);            /* [0, 1): the counter goes up first, so the first draw is unit(seed, 1) */
float pc_fxp_rt60_gain(float period_s, float seconds);  /* the feedback that makes a loop of `period_s` fall 60 dB in `seconds` (held at >= 1 ms) */
/* a power-of-two delay line inside the app's own block: next_pow2(samples + 4) floats, at(0) = newest */
typedef struct { float *buf; uint32_t mask, at; } pc_fxp_line_t;
size_t pc_fxp_line_floats(uint32_t samples);
void  pc_fxp_line_init(pc_fxp_line_t *l, float *mem, uint32_t samples);   /* mem: pc_fxp_line_floats(samples) floats, zeroed here */
void  pc_fxp_line_push(pc_fxp_line_t *l, float x);
float pc_fxp_line_at(const pc_fxp_line_t *l, uint32_t k);

/* a value that goes to its target over a fixed number of samples: a straight line, or a constant ratio per sample (geo).
 * Used by the ported apps for their 20 ms smoothing (tube gains, widener amount); constant values never move. */
typedef struct { float cur, tgt, step; uint32_t left, ramp; bool geo; } pc_fxp_sm_t;
void  pc_fxp_sm_snap(pc_fxp_sm_t *s, float v);
void  pc_fxp_sm_target(pc_fxp_sm_t *s, float t);
float pc_fxp_sm_next(pc_fxp_sm_t *s);
