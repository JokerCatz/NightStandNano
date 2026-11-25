// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx app: compressor (feed-forward, peak detector, soft knee). Same DSP as the old node (removed in T3);
 * set() keeps the gain reduction, the makeup gain (incl. the auto part) ramps in dB over one block. */
#include "pc_fxapp_parts.h"

enum { C_THR, C_RATIO, C_KNEE, C_ATT, C_REL, C_MAKEUP, C_AUTO, C_NP };
static const pc_fxparam_t COMP_P[C_NP] = {
    { "threshold_db", "門檻", "Threshold", -60, 0, -24, 1, PC_FXU_DB, 0, NULL, NULL },
    { "ratio", "比例", "Ratio", 1, 20, 2.5f, 1.12f, PC_FXU_RATIO, PC_FXP_LOG, NULL, NULL },
    { "knee_db", "柔和度", "Knee", 0, 24, 12, 1, PC_FXU_DB, 0, NULL, NULL },
    { "attack_ms", "起音", "Attack", 0.1f, 100, 10, 1.19f, PC_FXU_MS, PC_FXP_LOG, NULL, NULL },
    { "release_ms", "釋放", "Release", 10, 1000, 200, 1.123f, PC_FXU_MS, PC_FXP_LOG, NULL, NULL },
    { "makeup_db", "補償", "Makeup", -12, 24, 0, 0.5f, PC_FXU_DB, 0, NULL, NULL },
    { "auto_makeup", "自動補償", "Auto makeup", 0, 1, 1, 1, PC_FXU_BOOL, 0, NULL, NULL },
};
typedef struct { uint32_t rate; float v[C_NP]; float thr, ratio, knee, att, rel, gr; pc_fx_ramp_t mk; } comp_t;

static float comp_tune(comp_t *n)   /* returns the makeup in dB */
{
    const float *v = n->v;
    n->thr = v[C_THR];
    n->ratio = v[C_RATIO];
    if (n->ratio < 1) n->ratio = 1;
    n->knee = v[C_KNEE];
    float mk = v[C_MAKEUP];
    if (v[C_AUTO] > 0.5f) mk += -n->thr * (1.0f - 1.0f / n->ratio) * 0.5f;
    n->att = expf(-1.0f / (v[C_ATT] * 0.001f * (float)n->rate));
    n->rel = expf(-1.0f / (v[C_REL] * 0.001f * (float)n->rate));
    return mk;
}
static size_t comp_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(comp_t); }
static void comp_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    comp_t *n = st;
    fx_clean(n->v, v, COMP_P, C_NP);
    pc_fx_ramp_set(&n->mk, comp_tune(n));
}
static void comp_rate(void *st, uint32_t rate)
{
    comp_t *n = st;
    n->rate = rate;
    fx_ramp_jump(&n->mk, comp_tune(n));
    n->gr = 0;
}
static void comp_init(void *st, uint32_t rate, const float *v)
{
    comp_t *n = st;
    memset(n, 0, sizeof *n);
    fx_clean(n->v, v, COMP_P, C_NP);
    comp_rate(n, rate);
}
static void comp_reset(void *st) { comp_t *n = st; n->gr = 0; pc_fx_ramp_end(&n->mk); }
static void comp_process(void *st, float *x, int frames)
{
    comp_t *n = st;
    bool still = !fx_ramping(&n->mk);
    for (int i = 0; i < frames; i++) {
        float pk = fmaxf(fabsf(x[2 * i]), fabsf(x[2 * i + 1]));
        float lv = pk > 1e-6f ? 20.0f * log10f(pk) : -120.0f, over = lv - n->thr, want;
        if (2 * over < -n->knee) want = 0;
        else if (2 * fabsf(over) <= n->knee && n->knee > 0) want = (1 / n->ratio - 1) * (over + n->knee / 2) * (over + n->knee / 2) / (2 * n->knee);
        else want = (1 / n->ratio - 1) * over;                       /* <= 0 dB */
        float c = want < n->gr ? n->att : n->rel;
        n->gr = want + c * (n->gr - want);
        float g = fx_db2lin(n->gr + (still ? n->mk.cur : pc_fx_ramp_at(&n->mk, i, frames)));
        x[2 * i] *= g;
        x[2 * i + 1] *= g;
    }
    pc_fx_ramp_end(&n->mk);
}

const pc_fxapp_t pc_fxapp_compressor = {
    "compressor", "壓縮", "Compressor",
    "縮小大聲和小聲的差距，讓小聲的地方比較聽得清楚。適合吵雜的地方，或大小聲起伏很大的音樂。",
    "Narrows the gap between loud and quiet parts so that quiet passages are easier to hear. Good for noisy places or music with big swings in loudness.",
    PC_FXFAM_DYN, PC_FXC_HEAVY, PC_FXA_MIX, C_NP, COMP_P,
    comp_size, comp_init, fx_fits_any, comp_set, comp_rate, comp_reset, comp_process, NULL,
};
