/* fx app: tube. Ported from the fx-lab project, crates/fx-permissive/src/small/tube_adaa.rs (lab_tube_adaa).
 * SPDX-License-Identifier: LicenseRef-own (JC), written from Parker, Zavalishin and Le Bivic, DAFx 2016
 * (entry citation: small/mod.rs:41-44, "Reducing the aliasing of nonlinear waveshaping using continuous-time convolution").
 *
 * Shape: tanh(v + b) - tanh(b), a lopsided curve; b (from "even") shifts where the signal sits on it, which adds the
 * even harmonics. Aliasing is kept down by first-order antiderivative antialiasing: each output is the average of the
 * curve between the previous and the current input, (F(u) - F(u')) / (u - u'), where F is the curve's antiderivative,
 * ln cosh(v + b) - v tanh(b). A 10 Hz DC blocker follows, then a gain that makes small signals come out at the level
 * setting: level / (drive * slope of the curve at b). The arithmetic follows the fx-lab code operation by operation in
 * float, so the renders match (host_test/test_fxref.c). The four values ramp over 20 ms and snap on reset; the gains
 * ramp geometrically (see pc_fxp_sm_t). */
#include "pc_fxapp_parts.h"

enum { T_DRIVE, T_EVEN, T_LEVEL, T_MIX, T_NP };
static const pc_fxparam_t TUBE_P[T_NP] = {
    { "drive_db", "推力", "Drive", -12, 24, 6, 0.5f, PC_FXU_DB, 0, NULL, NULL },
    { "even", "偶次泛音", "Even harmonics", 0, 1, 0.5f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "level_db", "輸出", "Level", -24, 6, 0, 0.5f, PC_FXU_DB, 0, NULL, NULL },
    { "mix", "濃度", "Mix", 0, 1, 1, 0.05f, PC_FXU_PCT, 0, NULL, NULL },   /* its own row, not the chain's */
};

#define FULL_SCALE 0.25f   /* input scale into the curve at 0 dB drive */
#define FURTHEST   0.8f    /* bias b at even = 1 */

/* the four values ramp over 20 ms with the shared pc_fxp_sm_t: straight for the bias and mix, geometric for the two gains.
 * The source ramps the gains in a straight line too; near the end of a big drive cut that changes the gain by several percent
 * per sample, which the curve's stored last point turns into a step (test_fxapps click bound), so the gains move by a constant
 * ratio. Constant values never move, so renders of constant parameters do not depend on the law. */
typedef struct {
    uint32_t rate;
    float t[4];                          /* targets: drive gain, bias, level gain, mix */
    pc_fxp_sm_t sm[4];
    float last[2], last_f[2];            /* per side: previous curve input and its F, valid for the bias in held_bias */
    float held_bias, tanh_b, slope;
    pc_fxp_dc_t dc[2];
} tube_t;

static void tube_targets(tube_t *n, const float *v)
{
    FX_CLEAN_V(TUBE_P, T_NP);
    n->t[0] = fx_db2lin(v[T_DRIVE]) * FULL_SCALE;
    n->t[1] = v[T_EVEN] * FURTHEST;
    n->t[2] = fx_db2lin(v[T_LEVEL]);
    n->t[3] = v[T_MIX];
}
static void tube_clear(tube_t *n)
{
    for (int i = 0; i < 4; i++) pc_fxp_sm_snap(&n->sm[i], n->t[i]);
    n->last[0] = n->last[1] = 0;
    n->held_bias = NAN;
    for (int c = 0; c < 2; c++) n->dc[c].x1 = n->dc[c].y1 = 0;
}
static void tube_rate(void *st, uint32_t rate)
{
    tube_t *n = st;
    n->rate = rate;
    uint32_t ramp = (uint32_t)((float)rate * 20.0f / 1000.0f);
    for (int i = 0; i < 4; i++) { n->sm[i].ramp = ramp > 1 ? ramp : 1; n->sm[i].geo = i == 0 || i == 2; }
    for (int c = 0; c < 2; c++) pc_fxp_dc_set(&n->dc[c], 10.0f, (float)rate);
    tube_clear(n);
}
static size_t tube_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(tube_t); }
static void tube_init(void *st, uint32_t rate, const float *v)
{
    tube_t *n = st;
    memset(n, 0, sizeof *n);
    tube_targets(n, v);
    tube_rate(n, rate);
}
static void tube_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    tube_t *n = st;
    tube_targets(n, v);
    for (int i = 0; i < 4; i++) pc_fxp_sm_target(&n->sm[i], n->t[i]);
}
static void tube_reset(void *st)
{
    tube_t *n = st;
    tube_clear(n);
}

static inline float tube_f(const tube_t *n, float v, float b) { return pc_fxp_ln_cosh(v + b) - v * n->tanh_b; }

static void tube_process(void *st, float *x, int frames)
{
    tube_t *n = st;
    for (int i = 0; i < frames; i++) {
        float g = pc_fxp_sm_next(&n->sm[0]);
        float b = pc_fxp_sm_next(&n->sm[1]);
        if (b != n->held_bias) {   /* bias changed: redo tanh(b), the slope, and F of the stored previous inputs */
            n->held_bias = b;
            n->tanh_b = tanhf(b);
            n->slope = 1.0f - n->tanh_b * n->tanh_b;
            for (int c = 0; c < 2; c++) n->last_f[c] = tube_f(n, n->last[c], b);
        }
        float back = pc_fxp_sm_next(&n->sm[2]) / (g * n->slope);
        float mix = pc_fxp_sm_next(&n->sm[3]);
        for (int c = 0; c < 2; c++) {
            float xs = x[2 * i + c];
            xs = isfinite(xs) ? xs : 0.0f;
            float u = xs * g;
            float fu = tube_f(n, u, b);
            float d = u - n->last[c];
            float bent = fabsf(d) < 1e-4f ? tanhf((u + n->last[c]) * 0.5f + b) - n->tanh_b : (fu - n->last_f[c]) / d;
            n->last[c] = u;
            n->last_f[c] = fu;
            float y = pc_fxp_dc_tick(&n->dc[c], bent) * back;
            x[2 * i + c] = xs + (y - xs) * mix;
        }
    }
}

const pc_fxapp_t pc_fxapp_tube = {
    "tube", "真空管", "Tube",
    "像真空管擴大器那樣把聲音稍微壓圓，加一點偶次泛音，聽起來更暖、更厚。晶片音樂的方波只有奇次泛音，加了偶次之後特別明顯。推力大一點更有味道，音量不會因此變大。",
    "Bends the sound a little, as a valve amplifier does, and adds even harmonics: warmer and thicker. On chip music, whose square waves have odd harmonics only, the difference is easy to hear. More drive bends more, and the sound does not get louder for it.",
    PC_FXFAM_TUBE, PC_FXC_HEAVY, 0, T_NP, TUBE_P,
    tube_size, tube_init, fx_fits_any, tube_set, tube_rate, tube_reset, tube_process, NULL,
};
