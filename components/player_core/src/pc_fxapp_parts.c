// SPDX-License-Identifier: LGPL-2.1-or-later
/* Shared parts of the ported fx apps: C versions of the small pieces of the fx-lab project's parts.rs
 * (ln_cosh, DcBlock, hash/unit, Line). Written here from what each does; all arithmetic in float as there. */
#include "pc_fxapp_parts.h"

float pc_fxp_ln_cosh(float x)
{
    float a = fabsf(x);
    return a + log1pf(expf(-2.0f * a)) - 0.69314718056f;
}

void pc_fxp_dc_set(pc_fxp_dc_t *d, float hz, float rate)
{
    d->r = 1.0f - 6.28318530718f * hz / rate;
}

float pc_fxp_dc_tick(pc_fxp_dc_t *d, float x)
{
    float y = x - d->x1 + d->r * d->y1;
    d->x1 = x;
    d->y1 = y;
    return y;
}

uint64_t pc_fxp_hash(uint64_t seed, uint64_t count)
{
    uint64_t z = seed ^ (count * 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

float pc_fxp_unit(uint64_t seed, uint64_t count)
{
    return (float)(pc_fxp_hash(seed, count) >> 40) / 16777216.0f;
}

void pc_fxp_rng_init(pc_fxp_rng_t *r, uint64_t seed)
{
    r->seed = seed;
    r->count = 0;
}

float pc_fxp_rng_next_unit(pc_fxp_rng_t *r)
{
    r->count++;
    return pc_fxp_unit(r->seed, r->count);
}

float pc_fxp_rt60_gain(float period_s, float seconds)
{
    return powf(10.0f, -3.0f * period_s / (seconds > 1e-3f ? seconds : 1e-3f));
}

size_t pc_fxp_line_floats(uint32_t samples)
{
    size_t n = 1;
    while (n < (size_t)samples + 4) n <<= 1;
    return n;
}

void pc_fxp_line_init(pc_fxp_line_t *l, float *mem, uint32_t samples)
{
    size_t n = pc_fxp_line_floats(samples);
    l->buf = mem;
    l->mask = (uint32_t)(n - 1);
    l->at = 0;
    memset(mem, 0, n * sizeof(float));
}

void pc_fxp_line_push(pc_fxp_line_t *l, float x)
{
    l->at = (l->at + 1) & l->mask;
    l->buf[l->at] = x;
}

float pc_fxp_line_at(const pc_fxp_line_t *l, uint32_t k)
{
    return l->buf[(l->at - k) & l->mask];
}

void pc_fxp_sm_snap(pc_fxp_sm_t *s, float v)
{
    s->cur = s->tgt = v;
    s->left = 0;
    s->step = 0;
}

void pc_fxp_sm_target(pc_fxp_sm_t *s, float t)
{
    if (t == s->tgt) return;
    s->tgt = t;
    s->left = s->ramp;
    s->step = s->geo ? powf(t / s->cur, 1.0f / (float)s->ramp) : (t - s->cur) / (float)s->ramp;
}

float pc_fxp_sm_next(pc_fxp_sm_t *s)
{
    if (s->left > 0) {
        s->left--;
        s->cur = s->left == 0 ? s->tgt : s->geo ? s->cur * s->step : s->cur + s->step;
    }
    return s->cur;
}
