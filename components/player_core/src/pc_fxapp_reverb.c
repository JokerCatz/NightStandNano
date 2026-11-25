// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx app: freeverb — Jezar's Freeverb (public domain), float. Same DSP as the old send node (removed in T3),
 * now inline at its slot: x = x + level * wet (spec §15 call 13). Delay lengths are the 44.1 kHz tunings scaled to
 * the rate; all lines live in the app's own block (sized at init for its rate, re-laid out by rate() when they fit). */
#include "pc_fxapp_parts.h"

#define NC 8
#define NA 4
#define SPREAD 23
static const int COMB[NC] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
static const int ALLP[NA] = { 556, 441, 341, 225 };

enum { R_ROOM, R_DAMP, R_WIDTH, R_LEVEL, R_NP };
static const pc_fxparam_t VERB_P[R_NP] = {
    { "room", "空間大小", "Room size", 0, 1, 0.5f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "damp", "悶度", "Damping", 0, 1, 0.5f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "width", "寬度", "Width", 0, 1, 1, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "level", "殘響量", "Level", 0, 1, 0.3f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
};
typedef struct { float *buf; int len, pos; float store; } line_t;
typedef struct {
    uint32_t rate;
    int cap;                          /* floats of line memory after the struct */
    line_t comb[2][NC], ap[2][NA];
    float fb, damp1, damp2;
    pc_fx_ramp_t wet1, wet2, level;
    float mem[];
} fv_t;

static int line_len(int base, int c, uint32_t rate) { return (int)((base + c * SPREAD) * ((float)rate / 44100.0f)) + 1; }
static int fv_floats(uint32_t rate)
{
    int total = 0;
    for (int c = 0; c < 2; c++) {
        for (int i = 0; i < NC; i++) total += line_len(COMB[i], c, rate);
        for (int i = 0; i < NA; i++) total += line_len(ALLP[i], c, rate);
    }
    return total;
}
static size_t fv_size(uint32_t rate, const float *v) { (void)v; return sizeof(fv_t) + (size_t)fv_floats(rate) * sizeof(float); }
static bool fv_fits(const void *st, uint32_t rate, const float *v) { (void)v; return rate > 0 && fv_floats(rate) <= ((const fv_t *)st)->cap; }
static void fv_tune(fv_t *n, const float *v, bool jump)
{
    n->fb = v[R_ROOM] * 0.28f + 0.7f;
    n->damp1 = v[R_DAMP] * 0.4f;
    n->damp2 = 1.0f - n->damp1;
    float wet = 3.0f, width = v[R_WIDTH];
    void (*to)(pc_fx_ramp_t *, float) = jump ? fx_ramp_jump : pc_fx_ramp_set;
    to(&n->wet1, wet * (width / 2.0f + 0.5f));
    to(&n->wet2, wet * ((1.0f - width) / 2.0f));
    to(&n->level, v[R_LEVEL]);
}
static void fv_rate(void *st, uint32_t rate)
{
    fv_t *n = st;
    n->rate = rate;
    float *p = n->mem;
    for (int c = 0; c < 2; c++) {
        for (int i = 0; i < NC; i++) { line_t *l = &n->comb[c][i]; l->len = line_len(COMB[i], c, rate); l->buf = p; p += l->len; l->pos = 0; l->store = 0; }
        for (int i = 0; i < NA; i++) { line_t *l = &n->ap[c][i]; l->len = line_len(ALLP[i], c, rate); l->buf = p; p += l->len; l->pos = 0; l->store = 0; }
    }
    memset(n->mem, 0, (size_t)n->cap * sizeof(float));
    pc_fx_ramp_end(&n->wet1);
    pc_fx_ramp_end(&n->wet2);
    pc_fx_ramp_end(&n->level);
}
static void fv_init(void *st, uint32_t rate, const float *v)
{
    FX_CLEAN_V(VERB_P, R_NP);
    fv_t *n = st;
    memset(n, 0, sizeof *n);
    n->cap = fv_floats(rate);
    fv_tune(n, v, true);
    fv_rate(n, rate);
}
static void fv_set(void *st, const float *v, uint32_t changed) { (void)changed; FX_CLEAN_V(VERB_P, R_NP); fv_tune(st, v, false); }
static void fv_reset(void *st) { fv_rate(st, ((fv_t *)st)->rate); }
static void fv_process(void *st, float *x, int frames)
{
    fv_t *n = st;
    bool still = !fx_ramping(&n->wet1) && !fx_ramping(&n->wet2) && !fx_ramping(&n->level);
    for (int i = 0; i < frames; i++) {
        float in = (x[2 * i] + x[2 * i + 1]) * 0.015f, out[2] = { 0, 0 };
        for (int c = 0; c < 2; c++) {
            for (int k = 0; k < NC; k++) {
                line_t *l = &n->comb[c][k];
                float y = l->buf[l->pos];
                l->store = y * n->damp2 + l->store * n->damp1;
                l->buf[l->pos] = in + l->store * n->fb;
                if (++l->pos >= l->len) l->pos = 0;
                out[c] += y;
            }
            for (int k = 0; k < NA; k++) {
                line_t *l = &n->ap[c][k];
                float bo = l->buf[l->pos];
                l->buf[l->pos] = out[c] + bo * 0.5f;
                out[c] = bo - out[c];
                if (++l->pos >= l->len) l->pos = 0;
            }
        }
        float w1 = still ? n->wet1.cur : pc_fx_ramp_at(&n->wet1, i, frames), w2 = still ? n->wet2.cur : pc_fx_ramp_at(&n->wet2, i, frames);
        float lv = still ? n->level.cur : pc_fx_ramp_at(&n->level, i, frames);
        float wl = out[0] * w1 + out[1] * w2, wr = out[1] * w1 + out[0] * w2;
        x[2 * i] += lv * wl;
        x[2 * i + 1] += lv * wr;
    }
    pc_fx_ramp_end(&n->wet1);
    pc_fx_ramp_end(&n->wet2);
    pc_fx_ramp_end(&n->level);
}

const pc_fxapp_t pc_fxapp_freeverb = {
    "freeverb", "殘響", "Reverb",
    "加上房間或大廳的回音，讓聲音有空間感。",
    "Adds the echo of a room or a hall for a sense of space.",
    PC_FXFAM_REVERB, PC_FXC_HEAVY, 0, R_NP, VERB_P,
    fv_size, fv_init, fv_fits, fv_set, fv_rate, fv_reset, fv_process, NULL,
};

#if CONFIG_FX_VELVET_ROOM
/* fx app: velvet_room. Ported from the fx-lab project, crates/fx-permissive/src/spaces/velvet_fdn.rs (lab_velvet_fdn).
 * SPDX-License-Identifier: LicenseRef-own (JC), written from Fagerstrom, Alary, Schlecht and Valimaki, "Velvet-noise
 * feedback delay network", DAFx 2020 (entry citation: spaces/mod.rs:124-138).
 *
 * Eight delay lines feed each other through an orthogonal 8x8 sum-and-difference (Hadamard) matrix, so the loop itself
 * loses nothing; what each line gives back is turned down for the decay time and, through a one-pole low-pass, for the
 * damping. The input does not enter at one point: each line is fed from the mono input through its own velvet-noise
 * filter, up to 12 clicks of random sign spread over 24 ms (the "density" sets how many are heard), so the first
 * echoes are already a dense wash. The clicks are laid out once, in reset(), from a fixed seed. The arithmetic follows
 * the source operation by operation in float (host_test/test_fxref.c compares with the fx-lab renders).
 *
 * The state is one block: this header, then the nine delay lines (the eight paths and the input), each a power of two
 * long, sized for the rate and the room size the block is made for. `size` is a REBUILD value for that reason. */

#define VR_LINES 8
#define VR_TAPS  12
#define VR_SPREAD_MS 24.0f
#define VR_TREBLE_HZ 6000.0f
#define VR_TAU 6.2831855f
#define VR_EVERY 8u            /* coefficients are redone every this many samples while something moves (counted from reset) */
static const float VR_LEN_MS[VR_LINES] = { 23.1f, 29.9f, 36.7f, 41.3f, 47.9f, 55.1f, 62.3f, 71.7f };

enum { V_DECAY, V_SIZE, V_DAMPING, V_DENSITY, V_MIX, V_NP };
static const pc_fxparam_t VR_P[V_NP] = {
    { "decay_s", "殘響時間", "Decay", 0.2f, 20, 2, 1.125f, PC_FXU_S, PC_FXP_LOG, NULL, NULL },
    { "size", "房間大小", "Room size", 0.5f, 2, 1, 0.05f, PC_FXU_NONE, PC_FXP_REBUILD, NULL, NULL },
    { "damping", "悶度", "Damping", 0, 1, 0.4f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "density", "密度", "Density", 0, 1, 0.7f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "mix", "濃度", "Mix", 0, 1, 0.3f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },   /* its own row, not the chain's */
};

typedef struct {
    uint32_t rate;
    int cap;                                 /* floats of line memory after the struct */
    int used;                                /* floats the lines take at the current rate */
    float size;
    pc_fxp_line_t in, line[VR_LINES];
    uint32_t late[VR_LINES];                 /* each path's delay in samples */
    float held[VR_LINES], gain[VR_LINES], damp, weight[VR_TAPS];
    struct { uint32_t late; float sign; } click[VR_LINES][VR_TAPS];
    pc_fxp_sm_t sm[4];                       /* decay, damping, density, mix: 40 ms, straight lines */
    float tv[4];                             /* the values reset() snaps to */
    uint64_t tick;
    float mem[];
} vr_t;

static float vr_ms(uint32_t rate) { return 0.001f * (float)rate; }
static uint32_t vr_len(int k, float size, uint32_t rate) { return (uint32_t)(VR_LEN_MS[k] * size * vr_ms(rate)); }
static uint32_t vr_spread(float size, uint32_t rate) { return (uint32_t)(VR_SPREAD_MS * size * vr_ms(rate)) + 1; }
static int vr_floats(uint32_t rate, float size)
{
    size_t t = pc_fxp_line_floats(vr_spread(size, rate));
    for (int k = 0; k < VR_LINES; k++) t += pc_fxp_line_floats(vr_len(k, size, rate));
    return (int)t;
}
static float vr_size_of(const float *v)
{
    float s = isfinite(v[V_SIZE]) ? v[V_SIZE] : VR_P[V_SIZE].def;
    return s < VR_P[V_SIZE].lo ? VR_P[V_SIZE].lo : s > VR_P[V_SIZE].hi ? VR_P[V_SIZE].hi : s;
}
static size_t vr_size(uint32_t rate, const float *v) { return sizeof(vr_t) + (size_t)vr_floats(rate, vr_size_of(v)) * sizeof(float); }
static bool vr_fits(const void *st, uint32_t rate, const float *v)
{
    const vr_t *n = st;
    return rate > 0 && vr_size_of(v) == n->size && vr_floats(rate, n->size) <= n->cap;
}

/* the coefficients the smoothed values give right now */
static void vr_configure(vr_t *n)
{
    float rate = (float)n->rate;
    for (int k = 0; k < VR_LINES; k++) n->gain[k] = pc_fxp_rt60_gain((float)n->late[k] / rate, n->sm[0].cur);
    float hz = VR_TREBLE_HZ * exp2f(2.0f - n->sm[1].cur * 4.0f);   /* the corner: from 20 kHz down to about 1.5 kHz */
    float top = rate * 0.45f;
    n->damp = expf(-VR_TAU * (hz < top ? hz : top) / rate);
    float c = 2.0f + n->sm[2].cur * (float)(VR_TAPS - 2);          /* from two clicks to twelve, each fading in whole */
    for (int j = 0; j < VR_TAPS; j++) {
        float w = c - (float)j;
        n->weight[j] = (w < 0.0f ? 0.0f : w > 1.0f ? 1.0f : w) / sqrtf(c);
    }
}

/* 8-point sum-and-difference transform, scaled by 1/sqrt(8) so that it loses nothing */
static void vr_hadamard(float *x)
{
    for (int h = 1; h < VR_LINES; h *= 2)
        for (int i = 0; i < VR_LINES; i += h * 2)
            for (int j = i; j < i + h; j++) {
                float a = x[j], b = x[j + h];
                x[j] = a + b;
                x[j + h] = a - b;
            }
    float k = 1.0f / sqrtf((float)VR_LINES);
    for (int i = 0; i < VR_LINES; i++) x[i] *= k;
}

static void vr_layout(vr_t *n, uint32_t rate)
{
    float *p = n->mem;
    n->rate = rate;
    n->in.buf = p; n->in.mask = (uint32_t)pc_fxp_line_floats(vr_spread(n->size, rate)) - 1; p += n->in.mask + 1;
    for (int k = 0; k < VR_LINES; k++) {
        n->line[k].buf = p;
        n->line[k].mask = (uint32_t)pc_fxp_line_floats(vr_len(k, n->size, rate)) - 1;
        p += n->line[k].mask + 1;
    }
    n->used = (int)(p - n->mem);
    uint32_t ramp = (uint32_t)((float)rate * 40.0f / 1000.0f);
    for (int i = 0; i < 4; i++) { n->sm[i].ramp = ramp > 1 ? ramp : 1; n->sm[i].geo = false; }
}

static void vr_reset(void *st)
{
    vr_t *n = st;
    float ms = vr_ms(n->rate), size = n->size;
    memset(n->mem, 0, (size_t)n->used * sizeof(float));
    n->in.at = 0;
    for (int k = 0; k < VR_LINES; k++) {
        n->line[k].at = 0;
        n->late[k] = vr_len(k, size, n->rate);
        if (n->late[k] < 2) n->late[k] = 2;
        n->held[k] = 0;
    }
    float span = VR_SPREAD_MS * size * ms / (float)VR_TAPS;       /* one click in each of twelve equal spans */
    pc_fxp_rng_t rng;
    pc_fxp_rng_init(&rng, 0x7E17E7ull);
    for (int k = 0; k < VR_LINES; k++)
        for (int j = 0; j < VR_TAPS; j++) {
            float at = (float)j * span + pc_fxp_rng_next_unit(&rng) * (span - 1.0f);
            n->click[k][j].late = (uint32_t)at;
            n->click[k][j].sign = pc_fxp_rng_next_unit(&rng) < 0.5f ? -1.0f : 1.0f;
        }
    for (int i = 0; i < 4; i++) pc_fxp_sm_snap(&n->sm[i], n->tv[i]);
    vr_configure(n);
    n->tick = 0;
}

static void vr_rate(void *st, uint32_t rate)      /* only when fits() */
{
    vr_t *n = st;
    vr_layout(n, rate);
    vr_reset(n);
}

static void vr_values(vr_t *n, const float *v)    /* v: cleaned */
{
    n->tv[0] = v[V_DECAY]; n->tv[1] = v[V_DAMPING]; n->tv[2] = v[V_DENSITY]; n->tv[3] = v[V_MIX];
}
static void vr_init(void *st, uint32_t rate, const float *v)
{
    FX_CLEAN_V(VR_P, V_NP);
    vr_t *n = st;
    memset(n, 0, sizeof *n);
    n->size = v[V_SIZE];
    n->cap = vr_floats(rate, n->size);
    vr_values(n, v);
    vr_layout(n, rate);
    vr_reset(n);
}
static void vr_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    FX_CLEAN_V(VR_P, V_NP);
    vr_t *n = st;
    vr_values(n, v);
    for (int i = 0; i < 4; i++) pc_fxp_sm_target(&n->sm[i], n->tv[i]);
}

static void vr_process(void *st, float *x, int frames)
{
    vr_t *n = st;
    for (int i = 0; i < frames; i++) {
        bool moving = n->sm[0].left || n->sm[1].left || n->sm[2].left;   /* the mix moves without new coefficients */
        for (int s = 0; s < 4; s++) pc_fxp_sm_next(&n->sm[s]);
        if (moving && n->tick % VR_EVERY == 0) vr_configure(n);
        n->tick++;
        float l = isfinite(x[2 * i]) ? x[2 * i] : 0.0f, r = isfinite(x[2 * i + 1]) ? x[2 * i + 1] : 0.0f;
        pc_fxp_line_push(&n->in, (l + r) * 0.5f);
        float outs[VR_LINES], wl = 0.0f, wr = 0.0f;
        for (int k = 0; k < VR_LINES; k++) {
            float y = pc_fxp_line_at(&n->line[k], n->late[k] - 1);
            n->held[k] = y * (1.0f - n->damp) + n->held[k] * n->damp;   /* the loss for the decay time, the treble a little more */
            outs[k] = n->held[k] * n->gain[k];
        }
        for (int k = 0; k < VR_LINES; k++) {
            float sign = (k & 2) == 0 ? 1.0f : -1.0f;
            if ((k & 1) == 0) wl += outs[k] * sign; else wr += outs[k] * sign;
        }
        vr_hadamard(outs);
        for (int k = 0; k < VR_LINES; k++) {
            float fed = 0.0f;
            for (int j = 0; j < VR_TAPS; j++) fed += pc_fxp_line_at(&n->in, n->click[k][j].late) * n->click[k][j].sign * n->weight[j];
            float u = outs[k] + fed * 0.5f;
            pc_fxp_line_push(&n->line[k], isfinite(u) ? u : 0.0f);
        }
        float mix = n->sm[3].cur;
        x[2 * i] = l + (wl * 0.5f - l) * mix;
        x[2 * i + 1] = r + (wr * 0.5f - r) * mix;
    }
}

const pc_fxapp_t pc_fxapp_velvet_room = {
    "velvet_room", "絲絨房間", "Velvet room",
    "另一種殘響：一開始就很密、很滑順，像一間聲音均勻的房間。殘響時間決定餘音多久消失，房間大小改變回音的間隔，悶度讓高音先消失。吃的運算比較多。",
    "A different reverb: dense and smooth from the very first moment, like a room that sounds even. Decay sets how long the tail lasts, room size changes the spacing of the echoes, and damping lets the treble die first. It takes more processing.",
    PC_FXFAM_REVERB, PC_FXC_HEAVY, PC_FXA_WIRED_ONLY, V_NP, VR_P,
    vr_size, vr_init, vr_fits, vr_set, vr_rate, vr_reset, vr_process, NULL,
};
#endif
