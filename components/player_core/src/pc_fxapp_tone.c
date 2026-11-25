// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx apps of the tone family: equalizer, tone, bass, loudness.
 * Same DSP as the old nodes (removed in T3). set() keeps every filter's history: a biquad's values
 * slew to their targets in bounded steps per block with its history kept (fx_bqs_t), gains ramp over one block;
 * an equalizer band or tone word that joins starts at 0 dB at rest and slews up, one that leaves slews to 0 dB and
 * then stops; the bass cut fades in or out over one block. */
#include <assert.h>
#include "pc_fxapp_parts.h"

/* ---------- equalizer: preamp + up to ten bands (AutoEQ letters P/L/H) ---------- */
#define EQ_BANDS 10
enum { E_PRE, E_COUNT, E_B0 };   /* band b: E_B0 + 4b + {0 type, 1 fc, 2 gain, 3 q} */
#define E_NP (E_B0 + 4 * EQ_BANDS)
static const char *const TY_ZH[3] = { "峰", "低架", "高架" };
static const char *const TY_EN[3] = { "Peak", "Low shelf", "High shelf" };
static const char TY_CH[3] = { 'P', 'L', 'H' };
#define BAND(b, N) \
    { "b" #b "_type", "頻段 " #N " 類型", "Band " #N " type", 0, 2, 0, 1, PC_FXU_CHOICE, PC_FXP_BAND, TY_ZH, TY_EN }, \
    { "b" #b "_fc", "頻段 " #N " 頻率", "Band " #N " freq", 20, 20000, 1000, 1.19f, PC_FXU_HZ, PC_FXP_BAND | PC_FXP_LOG, NULL, NULL }, \
    { "b" #b "_gain_db", "頻段 " #N " 增益", "Band " #N " gain", -24, 24, 0, 0.5f, PC_FXU_DB, PC_FXP_BAND, NULL, NULL }, \
    { "b" #b "_q", "頻段 " #N " Q", "Band " #N " Q", 0.1f, 10, 0.71f, 1.125f, PC_FXU_NONE, PC_FXP_BAND | PC_FXP_LOG, NULL, NULL }
static const pc_fxparam_t EQ_P[E_NP] = {
    { "preamp_db", "前級增益", "Preamp", -24, 6, 0, 0.5f, PC_FXU_DB, 0, NULL, NULL },
    { "count", "頻段數", "Bands", 0, EQ_BANDS, 0, 1, PC_FXU_NONE, 0, NULL, NULL },
    BAND(0, 1), BAND(1, 2), BAND(2, 3), BAND(3, 4), BAND(4, 5), BAND(5, 6), BAND(6, 7), BAND(7, 8), BAND(8, 9), BAND(9, 10),
};
_Static_assert(E_NP <= PC_FXP_MAX, "equalizer values");
typedef struct {
    uint32_t rate;
    float v[E_NP];
    pc_fx_ramp_t pre;
    fx_bqs_t bq[EQ_BANDS];
    bool on[EQ_BANDS];                /* running: below the count, or leaving (slewing to 0 dB) */
    int count;
} eq_t;

static int eq_count(const float *v) { float c = v[E_COUNT]; return c <= 0 ? 0 : c >= EQ_BANDS ? EQ_BANDS : (int)lroundf(c); }
static char eq_type(const float *v, int b) { float t = v[E_B0 + 4 * b]; return TY_CH[t <= 0 ? 0 : t >= 2 ? 2 : (int)lroundf(t)]; }
static void eq_targets(eq_t *n)
{
    n->count = eq_count(n->v);
    for (int b = 0; b < EQ_BANDS; b++) {
        const float *w = n->v + E_B0 + 4 * b;
        fx_bqs_t *s = &n->bq[b];
        if (b < n->count) {
            if (!n->on[b]) { fx_bqs_rest(s, eq_type(n->v, b), w[1], w[3], n->rate); n->on[b] = true; }
            fx_bqs_stay(s);                                               /* back below the count: not leaving */
            fx_bqs_target(s, eq_type(n->v, b), w[1], w[2], w[3]);
        } else if (n->on[b]) {
            fx_bqs_target(s, s->ttype, s->tfc, 0, s->tq);                 /* leaving: to 0 dB, then off */
        }
    }
}
static size_t eq_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(eq_t); }
static void eq_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    eq_t *n = st;
    fx_clean(n->v, v, EQ_P, E_NP);
    pc_fx_ramp_set(&n->pre, fx_db2lin(n->v[E_PRE]));
    eq_targets(n);
}
static void eq_rate(void *st, uint32_t rate)
{
    eq_t *n = st;
    n->rate = rate;
    n->count = eq_count(n->v);
    for (int b = 0; b < EQ_BANDS; b++) {
        const float *w = n->v + E_B0 + 4 * b;
        fx_bqs_design(&n->bq[b], eq_type(n->v, b), w[1], w[2], w[3], rate);
        n->on[b] = b < n->count;
    }
    pc_fx_ramp_end(&n->pre);
}
static void eq_init(void *st, uint32_t rate, const float *v)
{
    eq_t *n = st;
    memset(n, 0, sizeof *n);
    fx_clean(n->v, v, EQ_P, E_NP);
    fx_ramp_jump(&n->pre, fx_db2lin(n->v[E_PRE]));
    eq_rate(n, rate);
}
static void eq_reset(void *st) { eq_rate(st, ((eq_t *)st)->rate); }
static void eq_process(void *st, float *x, int frames)
{
    eq_t *n = st;
    assert(frames <= PC_FX_BLOCK);
    if (fx_ramping(&n->pre) || n->pre.cur != 1.0f) fx_gain_run(x, frames, &n->pre);
    for (int b = 0; b < EQ_BANDS; b++) {
        if (!n->on[b]) continue;
        fx_bqs_step(&n->bq[b], n->rate);
        fx_bqs_run(&n->bq[b], x, frames, 0, frames);
        fx_bqs_end(&n->bq[b]);
        if (b >= n->count && fx_bqs_drained(&n->bq[b])) { n->on[b] = false; fx_bq_clear(&n->bq[b].bq); }
    }
}
int fx_eq_running(const void *st)
{
    const eq_t *n = st;
    int k = 0;
    for (int b = 0; b < EQ_BANDS; b++) k += n->on[b];
    return k;
}

const pc_fxapp_t pc_fxapp_equalizer = {
    "equalizer", "等化器", "Equalizer",
    "用幾個頻段分別調大或調小某些頻率。常用來套用耳機的校正曲線（AutoEQ）。",
    "Raises or lowers chosen frequency bands. Often used for a headphone correction curve (AutoEQ).",
    PC_FXFAM_TONE, PC_FXC_HEAVY, PC_FXA_MIX | PC_FXA_BANDS, E_NP, EQ_P,
    eq_size, eq_init, fx_fits_any, eq_set, eq_rate, eq_reset, eq_process, NULL,
};

/* ---------- tone: chipchill tone.rs WORDS, each a slider -1.5..1.5 scaling up to two filters ---------- */
static const struct { const char *id; struct { char t; float hz, q, db; } f[2]; int nf; } WORDS[] = {   /* id = TONE_P[i].id */
    { "warmth", { { 'L', 250, 0.707f, 6 }, { 'H', 4000, 0.707f, -6 } }, 2 },
    { "depth", { { 'L', 70, 0.707f, 12 } }, 1 },
    { "punch", { { 'P', 100, 1.0f, 10 } }, 1 },
    { "body", { { 'P', 220, 0.7f, 8 } }, 1 },
    { "voice", { { 'P', 800, 0.5f, 8 } }, 1 },
    { "presence", { { 'P', 3000, 1.4f, 8 } }, 1 },
    { "bite", { { 'P', 5500, 0.6f, 8 } }, 1 },
    { "sibilance", { { 'P', 7500, 2.0f, 10 } }, 1 },
    { "air", { { 'H', 11000, 0.707f, 12 } }, 1 },
};
#define T_NP 9
#define T_NBQ 10
_Static_assert(sizeof WORDS / sizeof *WORDS == T_NP, "tone words");
static const pc_fxparam_t TONE_P[T_NP] = {
    { "warmth", "溫暖", "Warmth", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "depth", "低沉", "Depth", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "punch", "打擊感", "Punch", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "body", "厚度", "Body", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "voice", "人聲", "Voice", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "presence", "臨場感", "Presence", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "bite", "清脆", "Bite", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "sibilance", "齒音", "Sibilance", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
    { "air", "空氣感", "Air", -1.5f, 1.5f, 0, 0.05f, PC_FXU_NONE, 0, NULL, NULL },
};
/* one fixed slot per word filter, in WORDS order (= the old node's order of the non-zero words). The word values
 * themselves slew (TONE_SLEW per block: a full -1.5..1.5 change in 50 blocks, one 0.05 detent in one block; the
 * 12 dB words then move 0.72 dB per block) and every filter of a word follows its current value. */
#define TONE_SLEW 0.06f
typedef struct { fx_bqs_t f; bool on; } slot_t;
typedef struct { uint32_t rate; float v[T_NP], cur[T_NP]; slot_t s[T_NBQ]; } tone_t;

static float tone_word(const float *v, int w) { float x = v[w]; return x > 1.5f ? 1.5f : x < -1.5f ? -1.5f : x; }
/* keep: the filters take the current word values as targets (a word joins at 0 dB at rest); else: there now */
static void tone_tune(tone_t *n, bool keep)
{
    int k = 0;
    for (int w = 0; w < T_NP; w++) {
        float x = n->cur[w];
        bool want = x != 0.0f || tone_word(n->v, w) != 0.0f;
        for (int f = 0; f < WORDS[w].nf; f++, k++) {
            slot_t *s = &n->s[k];
            char t = WORDS[w].f[f].t;
            float hz = WORDS[w].f[f].hz, q = WORDS[w].f[f].q, g = x * WORDS[w].f[f].db;
            if (!keep) {                                   /* init / rate / reset: a 0 word runs no filter */
                s->on = x != 0.0f;
                fx_bqs_design(&s->f, t, hz, g, q, n->rate);
                continue;
            }
            if (want && !s->on) { fx_bqs_rest(&s->f, t, hz, q, n->rate); s->on = true; }
            if (want) fx_bqs_stay(&s->f);                  /* the word is on again: not leaving */
            if (s->on) fx_bqs_target(&s->f, t, hz, g, q);
        }
    }
}
static size_t tone_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(tone_t); }
static void tone_set(void *st, const float *v, uint32_t changed) { (void)changed; fx_clean(((tone_t *)st)->v, v, TONE_P, T_NP); }
static void tone_rate(void *st, uint32_t rate)
{
    tone_t *n = st;
    n->rate = rate;
    for (int w = 0; w < T_NP; w++) n->cur[w] = tone_word(n->v, w);
    tone_tune(n, false);
}
static void tone_init(void *st, uint32_t rate, const float *v)
{
    tone_t *n = st;
    memset(n, 0, sizeof *n);
    fx_clean(n->v, v, TONE_P, T_NP);
    tone_rate(n, rate);
}
static void tone_reset(void *st) { tone_rate(st, ((tone_t *)st)->rate); }
static void tone_process(void *st, float *x, int frames)
{
    tone_t *n = st;
    bool moved = false;
    for (int w = 0; w < T_NP; w++) {
        float t = tone_word(n->v, w);
        if (n->cur[w] != t) { n->cur[w] = fx_slew_lin(n->cur[w], t, TONE_SLEW); moved = true; }
    }
    if (moved) tone_tune(n, true);
    for (int k = 0; k < T_NBQ; k++) {
        slot_t *s = &n->s[k];
        if (!s->on) continue;
        fx_bqs_step(&s->f, n->rate);
        fx_bqs_run(&s->f, x, frames, 0, frames);
        fx_bqs_end(&s->f);
        if (s->f.tg == 0.0f && fx_bqs_drained(&s->f)) { s->on = false; fx_bq_clear(&s->f.bq); }
    }
}
int fx_tone_running(const void *st)
{
    const tone_t *n = st;
    int k = 0;
    for (int i = 0; i < T_NBQ; i++) k += n->s[i].on;
    return k;
}

const pc_fxapp_t pc_fxapp_tone = {
    "tone", "音色", "Tone",
    "用形容詞調整聲音的個性，例如溫暖、低沉、空氣感。0 是不變。",
    "Shapes the character of the sound with words such as warmth, depth or air. 0 leaves it unchanged.",
    PC_FXFAM_TONE, PC_FXC_HEAVY, PC_FXA_MIX, T_NP, TONE_P,
    tone_size, tone_init, fx_fits_any, tone_set, tone_rate, tone_reset, tone_process, NULL,
};

/* ---------- bass: chipchill makes 2nd/3rd harmonics of the bass below reach_hz ---------- */
enum { S_AMOUNT, S_REACH, S_CUT, S_NP };
static const pc_fxparam_t BASS_P[S_NP] = {
    { "amount", "強度", "Amount", 0, 1, 0.5f, 0.05f, PC_FXU_PCT, 0, NULL, NULL },
    { "reach_hz", "延伸到", "Reach", 40, 160, 90, 1.05f, PC_FXU_HZ, PC_FXP_LOG, NULL, NULL },
    { "cut", "切掉更低的", "Cut below", 0, 1, 0, 1, PC_FXU_BOOL, 0, NULL, NULL },
};
typedef struct {
    uint32_t rate;
    float reach, treach, env, env_fall;   /* reach slews to treach by BASS_SLEW per block (its own detent; 40..160 in 29 blocks) */
    fx_bqs_t lp, hp_band, lp_band, cut;
    pc_fx_ramp_t amount, cutmix;      /* cutmix: 1 = the cut high-pass fully in */
    float lo[2 * FX_CHUNK], tmp[2 * FX_CHUNK];
} bass_t;

#define BASS_SLEW 1.05f
static float bass_reach(const float *v) { float r = v[S_REACH]; return r < 40 ? 40 : r > 160 ? 160 : r; }
static void bass_tune(bass_t *n, bool keep)
{
    if (!keep) {
        fx_bqs_design(&n->lp, 'l', n->reach, 0, 0.707f, n->rate);              /* the bass harmonics are made from */
        fx_bqs_design(&n->hp_band, 'h', n->reach, 0, 0.707f, n->rate);         /* what is made stays between reach .. 3 reach */
        fx_bqs_design(&n->lp_band, 'l', n->reach * 3.0f, 0, 0.707f, n->rate);
        fx_bqs_design(&n->cut, 'h', n->reach, 0, 0.707f, n->rate);
        return;
    }
    fx_bqs_target(&n->lp, 'l', n->reach, 0, 0.707f);
    fx_bqs_target(&n->hp_band, 'h', n->reach, 0, 0.707f);
    fx_bqs_target(&n->lp_band, 'l', n->reach * 3.0f, 0, 0.707f);
    if (n->cutmix.cur > 0 || fx_ramping(&n->cutmix)) fx_bqs_target(&n->cut, 'h', n->reach, 0, 0.707f);
    else fx_bqs_design(&n->cut, 'h', n->reach, 0, 0.707f, n->rate);   /* not heard: there now, from rest */
}
static size_t bass_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(bass_t); }
static void bass_set(void *st, const float *v, uint32_t changed)
{
    (void)changed;
    bass_t *n = st;
    float w[S_NP];
    fx_clean(w, v, BASS_P, S_NP);
    n->treach = bass_reach(w);
    pc_fx_ramp_set(&n->amount, w[S_AMOUNT]);
    pc_fx_ramp_set(&n->cutmix, w[S_CUT] > 0.5f ? 1.0f : 0.0f);
}
static void bass_rate(void *st, uint32_t rate)
{
    bass_t *n = st;
    n->rate = rate;
    n->reach = n->treach;
    bass_tune(n, false);
    n->env = 0;
    n->env_fall = expf(-1.0f / (0.05f * (float)rate));               /* held about one 20 Hz wave */
    pc_fx_ramp_end(&n->amount);
    pc_fx_ramp_end(&n->cutmix);
}
static void bass_init(void *st, uint32_t rate, const float *v)
{
    bass_t *n = st;
    float w[S_NP];
    fx_clean(w, v, BASS_P, S_NP);
    memset(n, 0, sizeof *n);
    n->reach = n->treach = bass_reach(w);
    fx_ramp_jump(&n->amount, w[S_AMOUNT]);
    fx_ramp_jump(&n->cutmix, w[S_CUT] > 0.5f ? 1.0f : 0.0f);
    bass_rate(n, rate);
}
static void bass_reset(void *st) { bass_rate(st, ((bass_t *)st)->rate); }
static void bass_process(void *st, float *x, int frames)
{
    bass_t *n = st;
    bool amt_still = !fx_ramping(&n->amount), cut_still = !fx_ramping(&n->cutmix);
    float *lo = n->lo, *t = n->tmp;
    if (n->reach != n->treach) { n->reach = fx_slew_log(n->reach, n->treach, BASS_SLEW); bass_tune(n, true); }
    fx_bqs_step(&n->lp, n->rate);
    fx_bqs_step(&n->hp_band, n->rate);
    fx_bqs_step(&n->lp_band, n->rate);
    fx_bqs_step(&n->cut, n->rate);
    for (int off = 0; off < frames; off += FX_CHUNK) {
        int k = frames - off < FX_CHUNK ? frames - off : FX_CHUNK;
        float *xs = x + 2 * off;
        memcpy(lo, xs, (size_t)k * 2 * sizeof(float));
        fx_bqs_run(&n->lp, lo, k, off, frames);
        for (int i = 0; i < k; i++) {
            float m = 0.5f * (lo[2 * i] + lo[2 * i + 1]), a = fabsf(m);
            n->env = a > n->env ? a : n->env * n->env_fall;
            float u = n->env > 1e-6f ? m / n->env : 0.0f;                /* -1..1 */
            float h = 0.6f * (2.0f * u * u - 1.0f) + 0.4f * (4.0f * u * u * u - 3.0f * u);   /* Chebyshev T2, T3 */
            lo[2 * i] = lo[2 * i + 1] = h * n->env;
        }
        fx_bqs_run(&n->hp_band, lo, k, off, frames);
        fx_bqs_run(&n->lp_band, lo, k, off, frames);
        if (cut_still) {
            if (n->cutmix.cur == 1.0f) fx_bqs_run(&n->cut, xs, k, off, frames);
        } else {                                                       /* the cut fades in or out across the block */
            memcpy(t, xs, (size_t)k * 2 * sizeof(float));
            fx_bqs_run(&n->cut, t, k, off, frames);
            for (int i = 0; i < k; i++) {
                float c = pc_fx_ramp_at(&n->cutmix, off + i, frames);
                xs[2 * i] += c * (t[2 * i] - xs[2 * i]);
                xs[2 * i + 1] += c * (t[2 * i + 1] - xs[2 * i + 1]);
            }
        }
        if (amt_still) {
            float am = n->amount.cur;
            for (int i = 0; i < 2 * k; i++) xs[i] += am * lo[i];
        } else {
            for (int i = 0; i < k; i++) {
                float am = pc_fx_ramp_at(&n->amount, off + i, frames);
                xs[2 * i] += am * lo[2 * i];
                xs[2 * i + 1] += am * lo[2 * i + 1];
            }
        }
    }
    pc_fx_ramp_end(&n->amount);
    fx_bqs_end(&n->lp);
    fx_bqs_end(&n->hp_band);
    fx_bqs_end(&n->lp_band);
    fx_bqs_end(&n->cut);
    if (!cut_still && n->cutmix.tgt == 0.0f) fx_bq_clear(&n->cut.bq);   /* faded out: starts from rest next time */
    pc_fx_ramp_end(&n->cutmix);
}

const pc_fxapp_t pc_fxapp_bass = {
    "bass", "低音加強", "Bass boost",
    "在低音上加出泛音，讓小耳機也聽得出低音。也可以切掉耳機放不出來的更低頻。",
    "Adds overtones to the bass so that small headphones carry it too. It can also cut the lowest notes they cannot play.",
    PC_FXFAM_TONE, PC_FXC_HEAVY, PC_FXA_MIX, S_NP, BASS_P,
    bass_size, bass_init, fx_fits_any, bass_set, bass_rate, bass_reset, bass_process, NULL,
};

/* ---------- loudness: chipchill fits ISO 226 contours; here two shelves that follow their shape ---------- */
enum { L_Q, L_NP };
static const pc_fxparam_t LOUD_P[L_NP] = {
    { "quieter_db", "補償量", "Amount", 0, 40, 20, 1, PC_FXU_DB, 0, NULL, NULL },
};
/* the amount itself slews (1 dB per block, so the shelves and the level cut move together), both shelves follow it */
typedef struct { uint32_t rate; float q, tq; fx_bqs_t lo, hi; pc_fx_ramp_t pre; } loud_t;

static float loud_q(const float *v) { float q = isfinite(v[L_Q]) ? v[L_Q] : LOUD_P[L_Q].def; return q < 0 ? 0 : q > 40 ? 40 : q; }
static void loud_design(loud_t *n)   /* there now, at rest */
{
    n->q = n->tq;
    fx_bqs_design(&n->lo, 'L', 100, 0.33f * n->q, 0.6f, n->rate);
    fx_bqs_design(&n->hi, 'H', 10000, 0.12f * n->q, 0.6f, n->rate);
    fx_ramp_jump(&n->pre, fx_db2lin(-0.1f * n->q));
}
static size_t loud_size(uint32_t rate, const float *v) { (void)rate; (void)v; return sizeof(loud_t); }
static void loud_set(void *st, const float *v, uint32_t changed) { (void)changed; ((loud_t *)st)->tq = loud_q(v); }
static void loud_rate(void *st, uint32_t rate) { loud_t *n = st; n->rate = rate; loud_design(n); }
static void loud_init(void *st, uint32_t rate, const float *v)
{
    loud_t *n = st;
    memset(n, 0, sizeof *n);
    n->tq = loud_q(v);
    loud_rate(n, rate);
}
static void loud_reset(void *st) { loud_rate(st, ((loud_t *)st)->rate); }
static void loud_process(void *st, float *x, int frames)
{
    loud_t *n = st;
    if (n->q != n->tq) {
        n->q = fx_slew_lin(n->q, n->tq, FX_SLEW_DB);
        fx_bqs_target(&n->lo, 'L', 100, 0.33f * n->q, 0.6f);      /* steps below FX_SLEW_DB: there in this block */
        fx_bqs_target(&n->hi, 'H', 10000, 0.12f * n->q, 0.6f);
        pc_fx_ramp_set(&n->pre, fx_db2lin(-0.1f * n->q));
        fx_bqs_step(&n->lo, n->rate);
        fx_bqs_step(&n->hi, n->rate);
    }
    if (fx_ramping(&n->pre) || n->pre.cur != 1.0f) fx_gain_run(x, frames, &n->pre);
    fx_bqs_run(&n->lo, x, frames, 0, frames);
    fx_bqs_run(&n->hi, x, frames, 0, frames);
    fx_bqs_end(&n->lo);
    fx_bqs_end(&n->hi);
}

const pc_fxapp_t pc_fxapp_loudness = {
    "loudness", "等響度", "Loudness",
    "小聲聽時，補回聽不太到的低音和高音。越小聲聽，補償量可以越大。",
    "Brings back the bass and treble that fade away when you listen quietly. The quieter you listen, the more you can add.",
    PC_FXFAM_TONE, PC_FXC_LIGHT, PC_FXA_MIX, L_NP, LOUD_P,
    loud_size, loud_init, fx_fits_any, loud_set, loud_rate, loud_reset, loud_process, NULL,
};
