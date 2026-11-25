// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx app: gain (same DSP as the old "gain" node, removed in T3); the gain ramps over one block. */
#include "pc_fxapp_parts.h"

enum { P_GAIN, NP };
static const pc_fxparam_t PARAM[NP] = {
    { "gain_db", "增益", "Gain", -24, 12, 0, 0.5f, PC_FXU_DB, 0, NULL, NULL },
};
typedef struct { uint32_t rate; pc_fx_ramp_t g; } gain_t;

static size_t gain_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(gain_t); }
static void gain_set(void *st, const float *v, uint32_t changed) { (void)changed; FX_CLEAN_V(PARAM, NP); pc_fx_ramp_set(&((gain_t *)st)->g, fx_db2lin(v[P_GAIN])); }
static void gain_init(void *st, uint32_t rate, const float *v)
{
    FX_CLEAN_V(PARAM, NP);
    gain_t *n = st;
    memset(n, 0, sizeof *n);
    n->rate = rate;
    fx_ramp_jump(&n->g, fx_db2lin(v[P_GAIN]));
}
static void gain_rate(void *st, uint32_t rate) { gain_t *n = st; n->rate = rate; pc_fx_ramp_end(&n->g); }
static void gain_reset(void *st) { pc_fx_ramp_end(&((gain_t *)st)->g); }
static void gain_process(void *st, float *x, int frames) { fx_gain_run(x, frames, &((gain_t *)st)->g); }

const pc_fxapp_t pc_fxapp_gain = {
    "gain", "音量增益", "Gain",
    "把整體聲音調大或調小。歌曲之間或耳機之間大小聲差太多時用。",
    "Turns the whole sound up or down. Use it when songs or headphones differ a lot in loudness.",
    PC_FXFAM_GAIN, PC_FXC_LIGHT, 0, NP, PARAM,
    gain_size, gain_init, fx_fits_any, gain_set, gain_rate, gain_reset, gain_process, NULL,
};
