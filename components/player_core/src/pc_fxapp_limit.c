// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx app: limiter (look-ahead, hold, exponential release, final clamp at the ceiling). Same DSP as the old node
 * (removed in T3); the look-ahead line lives in the app's own block, so lookahead_ms is a REBUILD value.
 * set() keeps the gain state and the line; a new ceiling ramps over one block (the final clamp would cut at once). */
#include "pc_fxapp_parts.h"

enum { M_CEIL, M_REL, M_LA, M_NP };
static const pc_fxparam_t LIM_P[M_NP] = {
    { "ceiling_db", "上限", "Ceiling", -12, 0, -1, 0.1f, PC_FXU_DB, 0, NULL, NULL },
    { "release_ms", "釋放", "Release", 10, 1000, 120, 1.123f, PC_FXU_MS, PC_FXP_LOG, NULL, NULL },
    { "lookahead_ms", "預看", "Look-ahead", 0, 20, 5, 0.5f, PC_FXU_MS, PC_FXP_REBUILD, NULL, NULL },
};
typedef struct {
    uint32_t rate;
    float v[M_NP];
    float rel, g, tgt, la_ms;
    pc_fx_ramp_t ceil;
    int la, pos, hold, cap;           /* cap: frames the line holds */
    float dl[];
} lim_t;

static float lim_ms(const float *v) { float m = isfinite(v[M_LA]) ? v[M_LA] : LIM_P[M_LA].def; return m < 0 ? 0 : m > 20 ? 20 : m; }
static int lim_frames(float la_ms, uint32_t rate) { return (int)(la_ms * 0.001f * (float)rate) + 1; }
static size_t lim_size(uint32_t rate, const float *v) { return sizeof(lim_t) + (size_t)lim_frames(lim_ms(v), rate) * 2 * sizeof(float); }
static bool lim_fits(const void *st, uint32_t rate, const float *v)
{
    const lim_t *n = st;
    return rate > 0 && lim_ms(v) == n->la_ms && lim_frames(n->la_ms, rate) <= n->cap;
}
static void lim_tune(lim_t *n, bool jump)
{
    (jump ? fx_ramp_jump : pc_fx_ramp_set)(&n->ceil, fx_db2lin(n->v[M_CEIL]));
    n->rel = 1.0f - expf(-1.0f / (n->v[M_REL] * 0.001f * (float)n->rate));
}
static void lim_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    lim_t *n = st;
    float la = n->v[M_LA];
    fx_clean(n->v, v, LIM_P, M_NP);
    n->v[M_LA] = la;                  /* REBUILD: a new block carries a new look-ahead */
    lim_tune(n, false);
}
static void lim_reset(void *st)
{
    lim_t *n = st;
    memset(n->dl, 0, (size_t)n->la * 2 * sizeof(float));
    n->pos = n->hold = 0;
    n->g = n->tgt = 1.0f;
    pc_fx_ramp_end(&n->ceil);
}
static void lim_rate(void *st, uint32_t rate)
{
    lim_t *n = st;
    n->rate = rate;
    n->la = lim_frames(n->la_ms, rate);
    if (n->la > n->cap) n->la = n->cap;   /* only called when fits() */
    lim_tune(n, true);
    lim_reset(n);
}
static void lim_init(void *st, uint32_t rate, const float *v)
{
    lim_t *n = st;
    memset(n, 0, sizeof *n);
    fx_clean(n->v, v, LIM_P, M_NP);
    n->la_ms = lim_ms(v);
    n->cap = lim_frames(n->la_ms, rate);
    lim_rate(n, rate);
}
static void lim_process(void *st, float *x, int frames)
{
    lim_t *n = st;
    bool still = !fx_ramping(&n->ceil);
    for (int i = 0; i < frames; i++) {
        float ceil = still ? n->ceil.cur : pc_fx_ramp_at(&n->ceil, i, frames);
        float l = x[2 * i], r = x[2 * i + 1], pk = fmaxf(fabsf(l), fabsf(r));
        float req = pk > ceil ? ceil / pk : 1.0f;
        if (req < n->tgt) { n->tgt = req; n->hold = n->la; }
        else if (n->hold > 0) n->hold--;
        else n->tgt += (1.0f - n->tgt) * n->rel;
        if (n->tgt < n->g) n->g += (n->tgt - n->g) / (float)n->la;   /* reach it within the look-ahead */
        else n->g = n->tgt;
        float dlv = n->dl[2 * n->pos], drv = n->dl[2 * n->pos + 1];
        n->dl[2 * n->pos] = l;
        n->dl[2 * n->pos + 1] = r;
        n->pos = (n->pos + 1) % n->la;
        float ol = dlv * n->g, orr = drv * n->g;
        x[2 * i] = ol > ceil ? ceil : ol < -ceil ? -ceil : ol;
        x[2 * i + 1] = orr > ceil ? ceil : orr < -ceil ? -ceil : orr;
    }
    pc_fx_ramp_end(&n->ceil);
}
static int lim_latency(const void *st) { return ((const lim_t *)st)->la; }

const pc_fxapp_t pc_fxapp_limiter = {
    "limiter", "限幅", "Limiter",
    "擋住太大聲的瞬間，防止爆音。永遠在最後一個。",
    "Holds back sudden peaks so that the sound never clips. It always comes last.",
    PC_FXFAM_LIMIT, PC_FXC_LIGHT, PC_FXA_ALWAYS_LAST, M_NP, LIM_P,
    lim_size, lim_init, lim_fits, lim_set, lim_rate, lim_reset, lim_process, lim_latency,
};
