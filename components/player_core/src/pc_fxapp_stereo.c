// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx apps of the stereo-difference family: balance, widener, crossfeed.
 * Same DSP as the old nodes (removed in T3); gains and the width ramp over one block,
 * the crossfeed low-pass slews to its new cutoff with its history kept (fx_bqs_t). */
#include "pc_fxapp_parts.h"

/* ---------- balance: > 0 turns the left down (dB of cut on one side, as the old node: range -1..1 = +-1 dB) ---------- */
enum { B_BAL, B_NP };
static const pc_fxparam_t BAL_P[B_NP] = {
    { "balance", "平衡", "Balance", -1, 1, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
};
typedef struct { pc_fx_ramp_t l, r; } bal_t;

static size_t bal_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(bal_t); }
static void bal_gains(float bal, float *l, float *r) { *l = bal > 0 ? fx_db2lin(-bal) : 1.0f; *r = bal < 0 ? fx_db2lin(bal) : 1.0f; }
static void bal_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    FX_CLEAN_V(BAL_P, B_NP);
    bal_t *n = st;
    float l, r;
    bal_gains(v[B_BAL], &l, &r);
    pc_fx_ramp_set(&n->l, l);
    pc_fx_ramp_set(&n->r, r);
}
static void bal_init(void *st, uint32_t rate, const float *v)
{
    (void)rate;
    FX_CLEAN_V(BAL_P, B_NP);
    bal_t *n = st;
    float l, r;
    bal_gains(v[B_BAL], &l, &r);
    fx_ramp_jump(&n->l, l);
    fx_ramp_jump(&n->r, r);
}
static void bal_reset(void *st) { bal_t *n = st; pc_fx_ramp_end(&n->l); pc_fx_ramp_end(&n->r); }
static void bal_rate(void *st, uint32_t rate) { (void)rate; bal_reset(st); }
static void bal_process(void *st, float *x, int frames) { bal_t *n = st; fx_gain2_run(x, frames, &n->l, &n->r); }

const pc_fxapp_t pc_fxapp_balance = {
    "balance", "左右平衡", "Balance",
    "把聲音往左或往右偏一點。負數偏左，正數偏右。",
    "Shifts the sound a little to the left or the right. Negative leans left, positive leans right.",
    PC_FXFAM_STEREO, PC_FXC_LIGHT, 0, B_NP, BAL_P,
    bal_size, bal_init, fx_fits_any, bal_set, bal_rate, bal_reset, bal_process, NULL,
};

/* ---------- widener: mid/side, the side scaled by width ---------- */
enum { W_WIDTH, W_NP };
static const pc_fxparam_t WIDE_P[W_NP] = {
    { "width", "寬度", "Width", 0, 2, 1, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
};
typedef struct { pc_fx_ramp_t w; } wide_t;

static size_t wide_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(wide_t); }
static void wide_set(void *st, const float *v, uint32_t changed) { (void)changed; FX_CLEAN_V(WIDE_P, W_NP); pc_fx_ramp_set(&((wide_t *)st)->w, v[W_WIDTH]); }
static void wide_init(void *st, uint32_t rate, const float *v) { (void)rate; FX_CLEAN_V(WIDE_P, W_NP); fx_ramp_jump(&((wide_t *)st)->w, v[W_WIDTH]); }
static void wide_reset(void *st) { pc_fx_ramp_end(&((wide_t *)st)->w); }
static void wide_rate(void *st, uint32_t rate) { (void)rate; wide_reset(st); }
static void wide_process(void *st, float *x, int frames)
{
    wide_t *n = st;
    bool still = !fx_ramping(&n->w);
    for (int i = 0; i < frames; i++) {
        float a = still ? n->w.cur : pc_fx_ramp_at(&n->w, i, frames);
        float l = x[2 * i], r = x[2 * i + 1], m = 0.5f * (l + r), s = 0.5f * (l - r) * a;
        x[2 * i] = m + s;
        x[2 * i + 1] = m - s;
    }
    pc_fx_ramp_end(&n->w);
}

const pc_fxapp_t pc_fxapp_widener = {
    "widener", "立體聲寬度", "Stereo width",
    "讓左右聲道分得更開或更靠攏。1 不變，0 變成單聲道，大於 1 更寬。",
    "Spreads the left and right channels apart or draws them together. 1 leaves the sound as it is, 0 makes it mono, above 1 is wider.",
    PC_FXFAM_STEREO, PC_FXC_LIGHT, 0, W_NP, WIDE_P,
    wide_size, wide_init, fx_fits_any, wide_set, wide_rate, wide_reset, wide_process, NULL,
};

/* ---------- crossfeed (bs2b-like): the far ear gets the other side, low-passed and quieter ---------- */
enum { X_FEED, X_CUT, X_NP };
static const pc_fxparam_t XF_P[X_NP] = {
    { "feed_db", "饋送（越小越強）", "Feed (lower = stronger)", 0, 15, 4.5f, 0.5f, PC_FXU_DB, 0, NULL, NULL },
    { "cutoff_hz", "頻率", "Cutoff", 300, 2000, 700, 1.05f, PC_FXU_HZ, PC_FXP_LOG, NULL, NULL },
};
typedef struct { uint32_t rate; float cut; fx_bqs_t lp; pc_fx_ramp_t g; float c[2 * FX_CHUNK]; } xf_t;

static size_t xf_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(xf_t); }
static void xf_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    FX_CLEAN_V(XF_P, X_NP);
    xf_t *n = st;
    pc_fx_ramp_set(&n->g, fx_db2lin(-v[X_FEED]));
    n->cut = v[X_CUT];
    fx_bqs_target(&n->lp, 'l', n->cut, 0, 0.5f);
}
static void xf_rate(void *st, uint32_t rate)
{
    xf_t *n = st;
    n->rate = rate;
    fx_bqs_design(&n->lp, 'l', n->cut, 0, 0.5f, rate);
    pc_fx_ramp_end(&n->g);
}
static void xf_init(void *st, uint32_t rate, const float *v)
{
    FX_CLEAN_V(XF_P, X_NP);
    xf_t *n = st;
    memset(n, 0, sizeof *n);
    n->cut = v[X_CUT];
    fx_ramp_jump(&n->g, fx_db2lin(-v[X_FEED]));
    xf_rate(n, rate);
}
static void xf_reset(void *st) { xf_t *n = st; fx_bqs_design(&n->lp, 'l', n->cut, 0, 0.5f, n->rate); pc_fx_ramp_end(&n->g); }
static void xf_process(void *st, float *x, int frames)
{
    xf_t *n = st;
    bool still = !fx_ramping(&n->g);
    fx_bqs_step(&n->lp, n->rate);
    for (int off = 0; off < frames; off += FX_CHUNK) {
        int k = frames - off < FX_CHUNK ? frames - off : FX_CHUNK;
        float *xs = x + 2 * off, *c = n->c;
        for (int i = 0; i < k; i++) { c[2 * i] = xs[2 * i + 1]; c[2 * i + 1] = xs[2 * i]; }   /* swapped */
        fx_bqs_run(&n->lp, c, k, off, frames);
        if (still) {
            float g = n->g.cur, norm = 1.0f / (1.0f + g);
            for (int i = 0; i < 2 * k; i++) xs[i] = (xs[i] + g * c[i]) * norm;
        } else {
            for (int i = 0; i < k; i++) {
                float g = pc_fx_ramp_at(&n->g, off + i, frames), norm = 1.0f / (1.0f + g);
                xs[2 * i] = (xs[2 * i] + g * c[2 * i]) * norm;
                xs[2 * i + 1] = (xs[2 * i + 1] + g * c[2 * i + 1]) * norm;
            }
        }
    }
    pc_fx_ramp_end(&n->g);
    fx_bqs_end(&n->lp);
}

const pc_fxapp_t pc_fxapp_crossfeed = {
    "crossfeed", "耳機交叉饋送", "Crossfeed",
    "把一點另一邊的聲音送進這一邊，像用喇叭聽一樣。戴耳機久聽比較不累。",
    "Feeds a little of each side into the other, as when listening to speakers. Long listening on headphones gets less tiring.",
    PC_FXFAM_STEREO, PC_FXC_LIGHT, PC_FXA_MIX, X_NP, XF_P,
    xf_size, xf_init, fx_fits_any, xf_set, xf_rate, xf_reset, xf_process, NULL,
};

/* ---------- velvet_widener: a short sparse random filter that makes the two ears hear different things ----------
 * Ported from the fx-lab project, crates/fx-permissive/src/small/vnd.rs (lab_vnd).
 * SPDX-License-Identifier: LicenseRef-own (JC), written from Alary, Politis and Valimaki, "Velvet-noise decorrelator",
 * DAFx 2017 (entry citation: small/mod.rs:63-66); the mono-safe way is the fx-lab author's own.
 *
 * A "sequence" is a list of n taps spread over the length: tap k lies somewhere inside the k-th equal slice (random place,
 * random sign), and the tap gains drop in a straight line in dB so the last is 18 dB under the first; all gains are then
 * scaled so their squares add up to 1. Reading the past of the signal at those taps gives a copy with the same content but
 * a different fine structure.
 * Mono-safe mode filters the middle (l + r) / 2 with the left sequence and adds the result to the left, subtracts it from the
 * right: the sum l + r is untouched, whatever the amount. Two sequences mode filters each side with its own sequence and
 * mixes it with the dry side along a quarter circle in amount, which keeps the level about the same across the range.
 * The arithmetic follows the source in float, tap by tap (host_test/test_fxref.c compares renders). Tables and both delay
 * lines live in the app's block; length_ms, mode and seed are REBUILD values (a new block through the chain's swap). */
enum { V_AMOUNT, V_LENGTH, V_MODE, V_SEED, V_NP };
static const char *const VMODE_ZH[] = { "單聲道安全", "兩組" };
static const char *const VMODE_EN[] = { "Mono-safe", "Two sequences" };
static const pc_fxparam_t VND_P[V_NP] = {
    { "amount", "寬度", "Width", 0, 1, 0.7f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "length_ms", "長度", "Length", 5, 50, 25, 1.12f, PC_FXU_MS, PC_FXP_LOG | PC_FXP_REBUILD, NULL, NULL },
    { "mode", "模式", "Mode", 0, 1, 0, 1, PC_FXU_CHOICE, PC_FXP_REBUILD, VMODE_ZH, VMODE_EN },
    { "seed", "種子", "Seed", 0, 99, 0, 1, PC_FXU_NONE, PC_FXP_REBUILD, NULL, NULL },
};

#define VND_PER_MS 1.0f     /* clicks a millisecond */
#define VND_DECAY_DB 18.0f  /* the last click against the first */
#define VND_MOST 64         /* the most clicks (50 ms at one a millisecond fits) */

typedef struct {
    uint32_t rate, cap;                      /* cap: floats of one line */
    float ms;
    uint32_t seed;
    bool two;
    int n;
    uint32_t late[2][VND_MOST];
    float gain[2][VND_MOST];
    pc_fxp_sm_t amt;
    float held, fade_s, fade_c;              /* the quarter-circle pair last worked out, for the amount held */
    pc_fxp_line_t line[2];
    float mem[];
} vnd_t;

static float vnd_ms(const float *v) { float m = isfinite(v[V_LENGTH]) ? v[V_LENGTH] : VND_P[V_LENGTH].def; return m < 5 ? 5 : m > 50 ? 50 : m; }
static float vnd_seedv(const float *v) { float s = isfinite(v[V_SEED]) ? v[V_SEED] : VND_P[V_SEED].def; return s < 0 ? 0 : s > 99 ? 99 : s; }
static bool vnd_two(const float *v) { return isfinite(v[V_MODE]) && v[V_MODE] >= 0.5f; }
static uint32_t vnd_length(float ms, uint32_t rate) { uint32_t l = (uint32_t)(ms * 0.001f * (float)rate); return l < 2 ? 2 : l; }
static size_t vnd_size(uint32_t rate, const float *v) { return sizeof(vnd_t) + 2 * pc_fxp_line_floats(vnd_length(vnd_ms(v), rate)) * sizeof(float); }
static bool vnd_fits(const void *st, uint32_t rate, const float *v)
{
    const vnd_t *n = st;
    return rate > 0 && vnd_ms(v) == n->ms && (uint32_t)vnd_seedv(v) == n->seed && vnd_two(v) == n->two
        && pc_fxp_line_floats(vnd_length(n->ms, rate)) <= n->cap;
}
static void vnd_velvet(vnd_t *n, int side, uint64_t seed, uint32_t length)
{
    const int cnt = n->n;
    float grid = (float)length / (float)cnt, energy = 0;
    for (int k = 0; k < cnt; k++) {
        float at = ((float)k + pc_fxp_unit(seed, 2 * (uint64_t)k) * 0.999f) * grid;
        uint32_t p = (uint32_t)at;
        n->late[side][k] = p > length - 1 ? length - 1 : p;
        float sign = pc_fxp_unit(seed, 2 * (uint64_t)k + 1) < 0.5f ? -1.0f : 1.0f;
        n->gain[side][k] = sign * powf(10.0f, -VND_DECAY_DB / 20.0f * (float)k / (float)cnt);
        energy += n->gain[side][k] * n->gain[side][k];
    }
    float norm = 1.0f / sqrtf(energy);
    for (int k = 0; k < cnt; k++) n->gain[side][k] *= norm;
}
static void vnd_clear(vnd_t *n)
{
    pc_fxp_sm_snap(&n->amt, n->amt.tgt);
    pc_fxp_line_init(&n->line[0], n->mem, vnd_length(n->ms, n->rate));
    pc_fxp_line_init(&n->line[1], n->mem + n->cap, vnd_length(n->ms, n->rate));
}
static void vnd_rate(void *st, uint32_t rate)
{
    vnd_t *n = st;
    n->rate = rate;
    uint32_t length = vnd_length(n->ms, rate);
    int cnt = (int)(n->ms * VND_PER_MS);
    n->n = cnt < 2 ? 2 : cnt > VND_MOST ? VND_MOST : cnt;
    vnd_velvet(n, 0, 0x5eed0001ull + (uint64_t)n->seed * 2, length);
    vnd_velvet(n, 1, 0x5eed0002ull + (uint64_t)n->seed * 2, length);
    uint32_t ramp = (uint32_t)((float)rate * 20.0f / 1000.0f);
    n->amt.ramp = ramp > 1 ? ramp : 1;
    vnd_clear(n);
}
static void vnd_init(void *st, uint32_t rate, const float *v)
{
    FX_CLEAN_V(VND_P, V_NP);
    vnd_t *n = st;
    memset(n, 0, sizeof *n);
    n->ms = vnd_ms(v);
    n->seed = (uint32_t)vnd_seedv(v);
    n->two = vnd_two(v);
    n->cap = (uint32_t)pc_fxp_line_floats(vnd_length(n->ms, rate));
    n->held = NAN;
    pc_fxp_sm_snap(&n->amt, v[V_AMOUNT]);
    vnd_rate(n, rate);
}
static void vnd_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    FX_CLEAN_V(VND_P, V_NP);
    pc_fxp_sm_target(&((vnd_t *)st)->amt, v[V_AMOUNT]);   /* the REBUILD values belong to this block */
}
static void vnd_reset(void *st) { vnd_clear(st); }
static inline float vnd_tick(const vnd_t *n, int side)
{
    float s = 0;
    for (int k = 0; k < n->n; k++) s += pc_fxp_line_at(&n->line[side], n->late[side][k]) * n->gain[side][k];
    return s;
}
static void vnd_process(void *st, float *x, int frames)
{
    vnd_t *n = st;
    for (int i = 0; i < frames; i++) {
        float l = isfinite(x[2 * i]) ? x[2 * i] : 0.0f, r = isfinite(x[2 * i + 1]) ? x[2 * i + 1] : 0.0f;
        float a = pc_fxp_sm_next(&n->amt);
        if (n->two) {
            pc_fxp_line_push(&n->line[0], l);
            pc_fxp_line_push(&n->line[1], r);
            float vl = vnd_tick(n, 0), vr = vnd_tick(n, 1);
            if (a != n->held) {
                n->held = a;
                float ang = a * 1.57079637f;   /* a quarter circle, kept in float */
                n->fade_s = sinf(ang);
                n->fade_c = cosf(ang);
            }
            x[2 * i] = l * n->fade_c + vl * n->fade_s;
            x[2 * i + 1] = r * n->fade_c + vr * n->fade_s;
        } else {
            pc_fxp_line_push(&n->line[0], (l + r) * 0.5f);
            float d = vnd_tick(n, 0) * a;
            x[2 * i] = l + d;
            x[2 * i + 1] = r - d;
        }
    }
}

const pc_fxapp_t pc_fxapp_velvet_widener = {
    "velvet_widener", "絲絨加寬", "Velvet widener",
    "讓聽起來偏單聲道的音樂左右分開、變寬，又不會像單純延遲一邊那樣產生空洞的梳狀聲。晶片音樂多半是單聲道，效果最明顯。單聲道安全模式下，左右加起來和原來完全一樣，所以接到只會合成單聲道的喇叭時聲音不變；兩組模式更寬，但合成單聲道時會有點改變。",
    "Spreads music that is nearly mono across the two sides without the hollow, comb-like colour that simply delaying one side gives. Chip music is mostly mono, so the effect is easy to hear. In mono-safe mode left plus right is exactly what it was, so a speaker that only plays the sum hears no change. Two sequences is wider, but the mono sum changes a little.",
    PC_FXFAM_STEREO, PC_FXC_HEAVY, 0, V_NP, VND_P,
    vnd_size, vnd_init, vnd_fits, vnd_set, vnd_rate, vnd_reset, vnd_process, NULL,
};
