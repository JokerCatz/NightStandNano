// SPDX-License-Identifier: LGPL-2.1-or-later
/* PICO-8 cartridges (.p8 text and .p8.png) as music: the sound effects and the music patterns of the cart are played, nothing else
 * (the Lua code of a cart is never read, let alone run). The synthesiser follows zepto8's (Sam Hocevar, WTFPL, github.com/samhocevar/
 * zepto8 @b2929a4, src/synth.cpp and src/pico8/sfx.cpp): the eight waveforms with the "buzz" / "noiz" / detune filter bits, the seven
 * note effects, custom instruments (an sfx used as an instrument), the small cross-fade on abrupt changes, reverb. Left out: the
 * dampen (low-pass) filter and the hardware registers a cart can poke at run time (they only exist while code runs).
 * Differences by design: the engine runs at its native 22050 Hz and is doubled to 44100 Hz by linear interpolation; the note state
 * (offsets, effects) is updated every 8 samples (2.7 kHz) instead of every sample, the phase still advances every sample; the noise
 * uses a fixed-seed generator so a render is reproducible. The cart plays once: the music ends at a stop flag, at a loop flag (where
 * the cart would jump back) or after pattern 63. Sub-tracks = the places a music sequence starts. byte_pos = the pattern being played.
 * A .p8.png hides the cart in the two low bits of every colour channel (160x205 RGBA, one cart byte per pixel); only the music and
 * sfx part (0x3100..0x42ff) is decoded. Every number read from the file is untrusted. */
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"
#include "pc/pc_extvoice.h"
#include "pc/pc_vmute.h"
#include "pc_inflate.h"

#define MAX_FILE (1u << 20)
#define STEP 8                          /* samples (at 22050 Hz) between two updates of the note state */
#define ENGINE_HZ 22050.0
#define OUT_RATE 44100u
#define TICK_SAMPLES 183.0              /* one speed unit of a note is 183 samples at 22050 Hz */
#define SFX_BYTES 68
#define ROM_MUSIC 0x3100u
#define ROM_SFX 0x3200u
#define ROM_END 0x4300u
#define PNG_W 160
#define PNG_H 205

typedef struct { uint8_t key, inst, vol, fx, custom; } note_t;
typedef struct { note_t n[32]; uint8_t filters, speed, ls, le; } sfx_t;
typedef struct { uint8_t d[4]; } song_t;

/* One oscillator state. Phases are Q32 fixed point (one cycle = 2^32), so the per-sample work is integer adds: the board has a single-precision FPU only and a double
 * costs a library call. ph keeps 33 bits (bit 32 = which half of a 2-cycle period, for the buzzing saw); ph2 / dph / dph2 are the extra oscillators
 * zepto8 derives from phi (phaser's second triangle at 109/110, the detuned second wave at a factor, and its own phaser triangle). */
typedef struct {
    uint64_t ph;
    uint32_t ph2, dph, dph2;
    uint32_t step, step2, dstep, dstep2;
    float last_sample, nscale, dfac;
    float freq, volume;
    uint8_t key, inst, custom, filters;
} syn_t;

typedef struct { int sfx; float offset, time, prev_vol; uint8_t prev_key; } sst_t;

typedef struct {
    sst_t main, custom;
    int sfx_music;
    float length, fade;
    bool can_loop, is_music;
    uint8_t last_inst, last_key;
    syn_t last, fade_synth;
    float rev2[366], rev4[732];
    uint32_t ridx;
    int last_note;                      /* for the strike counter */
    int last_sfx;
} chan_t;

typedef struct {
    sfx_t sfx[64];
    song_t song[64];
    chan_t ch[4];
    int pattern, count;
    float moff, mlen;
    int starts[64], nstarts, sub;
    int32_t length_ms;
    uint32_t played;                    /* output frames */
    uint32_t rng;
    float buf[STEP];                    /* the last generated engine samples */
    int bpos;                           /* consumed, STEP = empty */
    float prev;                         /* last engine sample (the interpolation) */
    int phase;                          /* 0: next output is the midpoint, 1: the sample itself */
    bool ended;
    uint32_t start_pat;
    uint32_t resume;      /* resume pattern handed to open(): applied by the start_track that follows it, cleared by the first render */
    float kf[64];                       /* key -> Hz */
} p8_t;

enum { FX_NONE, FX_SLIDE, FX_VIB, FX_DROP, FX_FADE_IN, FX_FADE_OUT, FX_ARP_FAST, FX_ARP_SLOW };

/* key -> Hz, 440 * 2^((key - 33) / 12) for keys 0..63, a fixed table: exp2f is a libm call whose last bit can differ between the host (glibc) and the S31
 * (newlib; the polynomial is fused there), so the board's cart renders could differ from the host's. Values = the host's exp2f, evaluated without fusing (the same numbers the host build always used). */
static const float key_hz[64] = {
	65.4063873f, 69.2956543f, 73.4161987f, 77.7817459f, 82.4068832f, 87.3070602f,
	92.4986038f, 97.998848f, 103.82618f, 110.0f, 116.540947f, 123.470825f,
	130.812775f, 138.591324f, 146.832382f, 155.563492f, 164.813782f, 174.61412f,
	184.997208f, 195.997726f, 207.652344f, 220.0f, 233.081863f, 246.94165f,
	261.625549f, 277.182648f, 293.664764f, 311.126984f, 329.627563f, 349.228241f,
	369.994415f, 391.995422f, 415.304688f, 440.0f, 466.163788f, 493.883301f,
	523.251099f, 554.365295f, 587.329529f, 622.253967f, 659.255127f, 698.456482f,
	739.988831f, 783.990845f, 830.609375f, 880.0f, 932.327576f, 987.766602f,
	1046.5022f, 1108.73059f, 1174.65906f, 1244.50793f, 1318.51025f, 1396.91296f,
	1479.97766f, 1567.98181f, 1661.21875f, 1760.0f, 1864.65491f, 1975.53345f,
	2093.00439f, 2217.46094f, 2349.31836f, 2489.01587f
};

/* ---- cart parsing ---------------------------------------------------------------------------------------------------------- */

static int hexv(int c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1; }

static void set_note(note_t *n, unsigned key, unsigned inst_nibble, unsigned vol, unsigned fx)
{
    n->key = (uint8_t)(key & 63);
    n->inst = (uint8_t)(inst_nibble & 7);
    n->custom = (uint8_t)(inst_nibble >> 3 & 1);
    n->vol = (uint8_t)(vol & 7);
    n->fx = (uint8_t)(fx & 7);
}

/* the text cart: only __sfx__ and __music__ are read */
static void parse_text(p8_t *p, const uint8_t *s, uint32_t len)
{
    int sect = 0, row = 0;
    uint32_t i = 0;
    for (int k = 0; k < 64; k++) for (int q = 0; q < 4; q++) p->song[k].d[q] = (uint8_t)(0x41 + q);   /* patterns the file does not list: all four channels off */
    while (i < len) {
        uint32_t e = i;
        while (e < len && s[e] != '\n') e++;
        uint32_t le = e;
        if (le > i && s[le - 1] == '\r') le--;
        const uint8_t *l = s + i;
        uint32_t n = le - i;
        if (n >= 4 && l[0] == '_' && l[1] == '_' && l[n - 1] == '_' && l[n - 2] == '_') {
            sect = (n == 7 && !memcmp(l, "__sfx__", 7)) ? 1 : (n == 9 && !memcmp(l, "__music__", 9)) ? 2 : 0;
            row = 0;
        } else if (sect == 1 && row < 64) {
            if (n >= 8) {
                int h[4], ok = 1;
                for (int k = 0; k < 8; k++) if (hexv(l[k]) < 0) ok = 0;
                for (int k = 0; k < 4; k++) h[k] = hexv(l[2 * k]) * 16 + hexv(l[2 * k + 1]);
                if (ok) {
                    sfx_t *x = &p->sfx[row];
                    x->filters = (uint8_t)h[0]; x->speed = (uint8_t)h[1]; x->ls = (uint8_t)h[2]; x->le = (uint8_t)h[3];
                    for (int k = 0; k < 32 && 8 + 5 * k + 5 <= (int)n; k++) {
                        int d[5], good = 1;
                        for (int q = 0; q < 5; q++) { d[q] = hexv(l[8 + 5 * k + q]); if (d[q] < 0) good = 0; }
                        if (good) set_note(&x->n[k], (unsigned)(d[0] * 16 + d[1]), (unsigned)d[2], (unsigned)d[3], (unsigned)d[4]);
                    }
                }
            }
            row++;
        } else if (sect == 2 && row < 64) {
            if (n >= 11 && l[2] == ' ') {
                int f = hexv(l[0]) * 16 + hexv(l[1]), ok = f >= 0;
                int b[4];
                for (int k = 0; k < 4; k++) { b[k] = hexv(l[3 + 2 * k]) * 16 + hexv(l[4 + 2 * k]); if (hexv(l[3 + 2 * k]) < 0 || hexv(l[4 + 2 * k]) < 0) ok = 0; }
                if (ok) {
                    for (int k = 0; k < 4; k++) p->song[row].d[k] = (uint8_t)(b[k] & 0x7f);
                    if (f & 1) p->song[row].d[0] |= 0x80;
                    if (f & 2) p->song[row].d[1] |= 0x80;
                    if (f & 4) p->song[row].d[2] |= 0x80;
                }
            }
            row++;
        }
        i = e + 1;
    }
}

/* the 0x3100..0x42ff part of a cart's memory: 64 patterns of 4 bytes, 64 sfx of 68 bytes */
static void parse_rom(p8_t *p, const uint8_t *rom)
{
    for (int i = 0; i < 64; i++) memcpy(p->song[i].d, rom + (ROM_MUSIC - ROM_MUSIC) + 4 * i, 4);
    for (int i = 0; i < 64; i++) {
        const uint8_t *b = rom + (ROM_SFX - ROM_MUSIC) + SFX_BYTES * i;
        sfx_t *x = &p->sfx[i];
        for (int k = 0; k < 32; k++) {
            unsigned w = (unsigned)(b[2 * k] | b[2 * k + 1] << 8);
            set_note(&x->n[k], w & 63, (w >> 6 & 7) | (w >> 15 & 1) << 3, w >> 9 & 7, w >> 12 & 7);
        }
        x->filters = b[64]; x->speed = b[65]; x->ls = b[66]; x->le = b[67];
    }
}

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

/* PNG -> the cart bytes up to ROM_END (caller frees *out). 8-bit RGBA 160x205, not interlaced. 0 or <0 */
static int decode_png(const uint8_t *f, uint32_t n, uint8_t **out)
{
    if (n < 33 || memcmp(f, "\x89PNG\r\n\x1a\n", 8)) return -1;
    uint32_t o = 8, idat = 0;
    uint8_t *z = malloc(n);                         /* all IDAT chunks, back to back */
    if (!z) return -1;
    bool hdr = false;
    for (int guard = 0; guard < 4096 && o + 12 <= n; guard++) {
        uint32_t cl = be32(f + o);
        const uint8_t *t = f + o + 4;
        if (cl > n - o - 12) break;
        if (!memcmp(t, "IHDR", 4)) {
            if (cl != 13 || be32(t + 4) != PNG_W || be32(t + 8) != PNG_H || t[12] != 8 || t[13] != 6 || t[16] != 0) { free(z); return -2; }
            hdr = true;
        } else if (!memcmp(t, "IDAT", 4)) { memcpy(z + idat, t + 4, cl); idat += cl; }
        else if (!memcmp(t, "IEND", 4)) break;
        o += 12 + cl;
    }
    const size_t raw_len = (size_t)PNG_H * (1 + PNG_W * 4);
    uint8_t *raw = hdr && idat > 6 ? malloc(raw_len) : NULL;
    void *st = raw ? malloc(pc_inflate_state_size()) : NULL;
    long got = st ? pc_inflate_raw_state(z + 2, idat - 2, raw, raw_len, st) : -1;   /* zlib header skipped, the Adler sum is not checked */
    free(st);
    free(z);
    if (got != (long)raw_len) { free(raw); return -1; }
    const int stride = PNG_W * 4;
    for (int y = 0; y < PNG_H; y++) {               /* undo the PNG row filters in place */
        uint8_t *r = raw + (size_t)y * (stride + 1) + 1, ft = r[-1];
        const uint8_t *u = y ? raw + (size_t)(y - 1) * (stride + 1) + 1 : NULL;
        for (int x = 0; x < stride; x++) {
            int a = x >= 4 ? r[x - 4] : 0, b = u ? u[x] : 0, c = (u && x >= 4) ? u[x - 4] : 0, v = r[x];
            switch (ft) {
            case 0: break;
            case 1: v += a; break;
            case 2: v += b; break;
            case 3: v += (a + b) / 2; break;
            case 4: { int pp = a + b - c, pa = abs(pp - a), pb = abs(pp - b), pc = abs(pp - c); v += (pa <= pb && pa <= pc) ? a : pb <= pc ? b : c; break; }
            default: free(raw); return -1;
            }
            r[x] = (uint8_t)v;
        }
    }
    uint8_t *cart = malloc(ROM_END);
    if (!cart) { free(raw); return -1; }
    for (uint32_t i = 0; i < ROM_END; i++) {        /* one cart byte per pixel: a r g b, two bits each (a = the high pair) */
        const uint8_t *px = raw + (size_t)(i / PNG_W) * (stride + 1) + 1 + (i % PNG_W) * 4;
        cart[i] = (uint8_t)((px[3] & 3) << 6 | (px[0] & 3) << 4 | (px[1] & 3) << 2 | (px[2] & 3));
    }
    free(raw);
    *out = cart;
    return 0;
}

/* ---- sequencing helpers ---------------------------------------------------------------------------------------------------- */

static bool pat_loop(const p8_t *p, int i) { return p->song[i].d[1] & 0x80; }
static bool pat_stop(const p8_t *p, int i) { return p->song[i].d[2] & 0x80; }
static bool pat_empty(const p8_t *p, int i) { return (p->song[i].d[0] & p->song[i].d[1] & p->song[i].d[2] & p->song[i].d[3] & 0x40) != 0; }

/* the length of a pattern in speed ticks, as zepto8's set_music_pattern; <= 0: nothing plays */
static int pat_duration(const p8_t *p, int pat)
{
    int dloop = -1, dno = -1;
    for (int i = 0; i < 4; i++) {
        int n = p->song[pat].d[i] & 0x7f;
        if (n & 0x40) continue;
        const sfx_t *s = &p->sfx[n & 63];
        if (s->le > 0 && s->le > s->ls) { int d = 32 * s->speed; if (d > dloop) dloop = d; }
        else {
            int end = 32;
            if (s->le == 0 && s->ls > 0 && s->ls < end) end = s->ls;
            dno = end * s->speed;
            break;
        }
    }
    return dno > 0 ? dno : dloop;
}

static int32_t seq_ms(const p8_t *p, int start)
{
    double ticks = 0;
    for (int pat = start; pat < 64; pat++) {
        int d = pat_duration(p, pat);
        if (d <= 0) break;
        ticks += d;
        if (pat_stop(p, pat) || pat_loop(p, pat)) break;
    }
    return (int32_t)(ticks * TICK_SAMPLES / ENGINE_HZ * 1000.0);
}

static void find_starts(p8_t *p)
{
    bool fresh = true;
    p->nstarts = 0;
    for (int i = 0; i < 64; i++) {
        if (pat_empty(p, i) || pat_duration(p, i) <= 0) { fresh = true; continue; }
        if (fresh) p->starts[p->nstarts++] = i;
        fresh = pat_stop(p, i) || pat_loop(p, i);
    }
}

/* ---- the synthesiser ------------------------------------------------------------------------------------------------------- */

static float rnd(p8_t *p)                       /* -1..1 */
{
    uint32_t x = p->rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    p->rng = x;
    return (float)(int32_t)x * (1.0f / 2147483648.0f);
}

static float absf(float v) { return v < 0 ? -v : v; }
#define Q32F (1.0f / 4294967296.0f)

/* one wave of the oscillator state (ph, ph2); d = true: the detune pair (dph, dph2) instead */
static float wave1(p8_t *p, syn_t *s, int inst, uint64_t ph, uint32_t ph2)
{
    const uint32_t f = (uint32_t)ph;
    const float t = (float)f * Q32F;
    const bool noiz = s->filters & 2, buzz = s->filters & 4;
    float ret;
    switch (inst) {
    case 0:
        ret = 1.f - absf(4.f * t - 2.f);
        if (buzz) {
            const float a = 0.875f;
            float b = t < a ? 2.f * t / a - 1.f : 2.f * (1.f - t) / (1.f - a) - 1.f;
            ret = ret * 0.75f + b * 0.25f;
        }
        return ret * 0.5f;
    case 1: {
        float a = buzz ? 0.975f : 0.875f;
        ret = t < a ? 2.f * t / a - 1.f : 2.f * (1.f - t) / (1.f - a) - 1.f;
        return ret * 0.5f;
    }
    case 2:
        ret = t < 0.5f ? t : t - 1.f;
        if (buzz) { float u = (float)((ph >> 32) & 1) + t; ret = ret * 0.83f - (absf(u - 1.0f) < 0.5f ? 0.085f : 0.0f); }
        return 0.653f * ret;
    case 3: return t < (buzz ? 0.4f : 0.5f) ? 0.25f : -0.25f;
    case 4: return t < (buzz ? 0.255f : 0.316f) ? 0.25f : -0.25f;
    case 5:
        ret = t < 0.5f ? 3.f - absf(24.f * t - 6.f) : 1.f - absf(16.f * t - 12.f);
        if (buzz) {
            ret = t < 0.5f ? ret * 2.0f + 3.0f : ret;
            ret = (t < 0.5f && ret > -1.875f) ? ret * 0.2f - 1.0f : ret + 0.5f;
        }
        return ret / 9.f;
    case 6: {                                   /* noise: a one-pole-smoothed random walk whose speed follows the pitch */
        float scale = s->nscale;
        float ns = (s->last_sample + scale * rnd(p)) / (1.0f + scale);
        float factor = 1.0f - (float)s->key / 63.0f;
        ret = ns * 1.5f * (1.0f + factor * factor);
        if (noiz) ret *= 2.0f * (t < 0.5f ? t : t - 1.f);
        s->last_sample = ns;
        return ret;
    }
    default: {
        float t2 = (float)ph2 * Q32F;
        ret = 2.f - absf(8.f * t - 4.f);
        ret += 1.f - absf(4.f * t2 - 2.f);
        if (buzz) {
            float t3 = (float)(uint32_t)(f * 2u + 0x80000000u) * Q32F, t4 = (float)(uint32_t)(f * 4u) * Q32F;
            ret += 0.25f - absf(t3 - 0.5f);
            ret += 0.125f - absf(0.5f * t4 - 0.25f);
        }
        return ret / 6.f;
    }
    }
}

static float synth_sample(p8_t *p, syn_t *s)
{
    float w = wave1(p, s, s->inst, s->ph, s->ph2);
    if (s->dfac != 0.f)                          /* detune: a second wave at a factor of the pitch */
        w += wave1(p, s, (s->filters / 8) % 3 == 2 && s->inst == 5 ? 0 : s->inst, s->dph, s->dph2) * 0.5f;
    float v = w * s->volume;
    return v > 1.f ? 1.f : v < -1.f ? -1.f : v;
}

static void syn_advance(syn_t *s)
{
    s->ph += s->step;
    s->ph2 += s->step2;
    if (s->dfac != 0.f) { s->dph += s->dstep; s->dph2 += s->dstep2; }
}

/* the phase increments for s->freq, the detune factor, and (on a reset) the derived phases from the main phase */
static void syn_setup(syn_t *s, bool reset_derived)
{
    float fr = s->freq > 11000.f ? 11000.f : s->freq < 0.f ? 0.f : s->freq;
    s->step = (uint32_t)(fr * (4294967296.0f / 22050.0f));
    s->step2 = (uint32_t)((uint64_t)s->step * 109u / 110u);     /* integer: the double quotient truncated to the same value (the fraction is never within 2^-21 of an integer), and no soft-double on the board in a per-channel, 2.7 kHz path */
    s->nscale = (float)s->step * Q32F * 8.858923f;
    int detune = (s->filters / 8) % 3;
    float factor = 0.f;
    if (detune && s->inst != 6) {
        if (s->inst == 0) factor = detune == 1 ? 3.0f / 4.0f : 3.0f / 2.0f;
        else if (s->inst == 5) factor = detune == 1 ? 200.0f / 199.0f : 800.0f / 199.0f;
        else if (s->inst == 7) factor = detune == 1 ? 49.0f / 50.0f : 400.0f / 199.0f;
        else factor = detune == 1 ? 200.0f / 199.0f : 400.0f / 199.0f;
    }
    s->dfac = factor;
    if (factor == 0.f) {                                                /* no detune (most voices): 0 exactly, no double work */
        s->dstep = 0;
        s->dstep2 = 0;
    } else {
        s->dstep = (uint32_t)(uint64_t)((double)s->step * factor);      /* the product can pass 2^32: the phase wraps anyway (a double -> uint32 conversion out of range is undefined) */
        s->dstep2 = (uint32_t)((uint64_t)s->dstep * 109u / 110u);
    }
    if (reset_derived) {
        double cyc = (double)(uint32_t)s->ph * (1.0 / 4294967296.0);
        s->ph2 = (uint32_t)(uint64_t)(cyc * 109.0 / 110.0 * 4294967296.0);
        s->dph = (uint32_t)(uint64_t)(cyc * factor * 4294967296.0);
        s->dph2 = (uint32_t)(uint64_t)(cyc * factor * 109.0 / 110.0 * 4294967296.0);
    }
}

/* zepto8 update_sfx_state for STEP samples at once. `ns` receives the note's synth parameters (phi stays: the caller advances it). */
static void update_sfx(const p8_t *p, sst_t *cur, syn_t *ns, float freq_factor, float length, bool is_music, bool can_loop)
{
    if (cur->sfx < 0) return;
    const sfx_t *sfx = &p->sfx[cur->sfx & 63];
    const int speed = sfx->speed > 0 ? sfx->speed : 1;
    const float per_second = (float)(ENGINE_HZ / (TICK_SAMPLES * speed));
    const float per_step = (float)STEP / (float)(TICK_SAMPLES * speed);
    const float offset = cur->offset;
    float next_offset = offset + per_step, next_time = cur->time + per_step;
    const float loop_range = (float)sfx->le - (float)sfx->ls;
    if (loop_range > 0.f && next_offset >= (float)sfx->le && can_loop) next_offset = fmodf(next_offset - (float)sfx->ls, loop_range) + (float)sfx->ls;
    bool has_end = false;
    float end_time = 32.f;
    if (length > 0.f) { has_end = true; end_time = length; }
    if (!is_music && sfx->le == 0 && sfx->ls > 0) { has_end = true; if ((float)sfx->ls < end_time) end_time = (float)sfx->ls; }
    if (loop_range <= 0.f) {
        has_end = true;
        if (!is_music) {
            int last = 0;
            for (int n = 0; n < 32; n++) if (sfx->n[n].vol > 0) last = n + 1 < 32 ? n + 1 : 32;
            if ((float)last < end_time) end_time = (float)last;
        }
    }
    if (offset < 32.f) {
        const int note_id = (int)floorf(offset), next_id = (int)floorf(next_offset);
        const note_t *nt = &sfx->n[note_id & 31];
        uint8_t key = nt->key;
        float volume = (float)nt->vol / 7.f, freq = p->kf[key & 63] * freq_factor;
        if (volume > 0.f) {
            const float fr = offset - floorf(offset);
            switch (nt->fx) {
            case FX_SLIDE:
                freq = p->kf[cur->prev_key & 63] + (freq - p->kf[cur->prev_key & 63]) * fr;
                if (cur->prev_vol > 0.f) volume = cur->prev_vol + (volume - cur->prev_vol) * fr;
                break;
            case FX_VIB: {
                float vx = 7.5f * offset / per_second;
                float tt = absf(vx - floorf(vx) - 0.5f) - 0.25f;
                freq = freq + (freq * 1.059463094359f - freq) * tt;
                break;
            }
            case FX_DROP: freq *= 1.f - fr; break;
            case FX_FADE_IN: volume *= fr; break;
            case FX_FADE_OUT: volume *= 1.f - fr; break;
            case FX_ARP_FAST:
            case FX_ARP_SLOW: {
                int m = (speed <= 8 ? 32 : 16) / (nt->fx == FX_ARP_FAST ? 4 : 8);
                int n = (int)((float)m * 7.5f * offset / per_second);
                freq = p->kf[sfx->n[((note_id & ~3) | (n & 3)) & 31].key & 63];
                break;
            }
            default: break;
            }
            ns->key = key;
            ns->freq = freq;
            ns->inst = nt->inst;
            ns->custom = nt->custom;
            ns->filters = sfx->filters;
            ns->volume = volume;
        }
        if (next_id != note_id) { cur->prev_key = nt->key; cur->prev_vol = (float)nt->vol / 7.f; }
    }
    cur->offset = next_offset;
    cur->time = next_time;
    if (has_end && next_time >= end_time) cur->sfx = -1;
}

static void launch(chan_t *c, int sfx, float offset, bool music)
{
    c->main.sfx = sfx;
    c->main.offset = offset > 0 ? offset : 0;
    c->main.time = 0;
    c->length = 0;
    c->can_loop = true;
    c->is_music = music;
    c->last_inst = 0xff;
    c->last_key = 0xff;
    c->main.prev_key = 24;
    c->main.prev_vol = 0;
}

static void set_pattern(p8_t *p, int pat)
{
    for (int n = 0; n < 4; n++) if (p->ch[n].is_music) { p->ch[n].main.sfx = -1; p->ch[n].sfx_music = -1; }
    int dur = pat >= 0 && pat < 64 ? pat_duration(p, pat) : 0;
    if (dur <= 0) { p->pattern = -1; p->moff = -1; p->mlen = 0; p->ended = true; return; }
    p->pattern = pat;
    p->moff = 0;
    p->mlen = (float)dur;
    for (int i = 0; i < 4; i++) {
        int n = p->song[pat].d[i] & 0x7f;
        if (n & 0x40) continue;
        if (p->ch[i].main.sfx == -1) launch(&p->ch[i], n & 63, 0, true);
        else p->ch[i].sfx_music = n & 63;
    }
}

/* ---- the 4 channels, STEP samples at a time -------------------------------------------------------------------------------- */

static void step(p8_t *p, float *out, pc_voices_t *v)
{
    for (int i = 0; i < STEP; i++) out[i] = 0.f;
    float amp[4] = { 0, 0, 0, 0 };
    const uint32_t mask = v ? pc_vmute_mask() : 0;
    for (int chn = 0; chn < 4; chn++) {
        chan_t *c = &p->ch[chn];
        if (chn == 0 && p->pattern != -1) {
            p->moff += (float)STEP / (float)TICK_SAMPLES;
            if (p->moff >= p->mlen) {
                int next = p->pattern + 1;
                if (pat_stop(p, p->pattern)) next = -1;
                else if (pat_loop(p, p->pattern)) next = -1;          /* where the cart would jump back: the song has been played once */
                p->count++;
                set_pattern(p, next);
            }
        }
        if (c->main.sfx == -1 && c->sfx_music != -1) {
            const sfx_t *sfx = &p->sfx[c->sfx_music];
            int speed = sfx->speed > 0 ? sfx->speed : 1;
            float no = p->moff / (float)speed;
            const float lr = (float)sfx->le - (float)sfx->ls;
            bool want = true;
            if (lr > 0.f && c->can_loop) { if (no > (float)sfx->ls) no = fmodf(no - (float)sfx->ls, lr) + (float)sfx->ls; }
            else if (no > 32.f) want = false;
            if (want) launch(c, c->sfx_music, no, true);
            c->sfx_music = -1;
        }
        syn_t ns;
        memset(&ns, 0, sizeof ns);
        ns.ph = c->last.ph; ns.ph2 = c->last.ph2; ns.dph = c->last.dph; ns.dph2 = c->last.dph2;
        ns.last_sample = c->last.last_sample;
        const float base_off = c->main.offset;
        update_sfx(p, &c->main, &ns, 1.0f, c->length, c->is_music, c->can_loop);
        bool restart = ns.inst != c->last_inst || ns.key != c->last_key;
        c->last_inst = ns.inst;
        c->last_key = ns.key;
        if (ns.volume > 0.f && ns.custom) {
            if (c->main.offset < base_off) restart = true;
            if (c->custom.sfx == -1 && floorf(base_off) != floorf(c->main.offset)) restart = true;
            if (restart) { c->custom.sfx = ns.inst; c->custom.offset = 0; c->custom.time = 0; c->custom.prev_key = 24; c->custom.prev_vol = 0; }
            float fac = ns.freq / p->kf[24];
            float mv = ns.volume;
            update_sfx(p, &c->custom, &ns, fac, 0.f, false, true);
            ns.volume *= mv;
        }
        {                                                             /* a strike: a new note (or sfx) that sounds */
            int nid = c->main.sfx >= 0 ? (int)floorf(base_off) : -1;
            if (ns.volume > 0.f && (nid != c->last_note || c->main.sfx != c->last_sfx) && v) v->strikes[chn]++;
            if (ns.volume > 0.f) { c->last_note = nid; c->last_sfx = c->main.sfx; }
            else if (c->main.sfx < 0) c->last_note = -1;
        }
        bool reset = false;                                           /* an abrupt change cross-fades from the previous parameters for a few milliseconds */
        {
            float lo = ns.freq < c->last.freq ? ns.freq : c->last.freq;
            if (absf(ns.volume - c->last.volume) > 0.1f || absf(ns.freq - c->last.freq) > lo * 0.01f || ns.inst != c->last.inst) {
                if (c->fade <= 0.f) c->fade_synth = c->last;
                c->fade = 1.f;
                ns.ph &= 0xFFFFFFFFull;                               /* the cycle count is reset between notes */
                reset = true;
            }
        }
        syn_setup(&ns, reset);
        const int reverb = (ns.filters / 24) % 3;
        for (int i = 0; i < STEP; i++) {
            float value = 0.f;
            if (ns.volume > 0.f) {
                syn_advance(&ns);
                value = synth_sample(p, &ns);
            }
            float r1 = reverb == 1 ? 1.f : 0.f;
            if (c->fade > 0.f) {
                syn_advance(&c->fade_synth);
                float vf = synth_sample(p, &c->fade_synth);
                value = value + (vf - value) * c->fade;
                const int fr = (c->fade_synth.filters / 24) % 3;
                r1 = r1 + ((fr == 1 ? 1.f : 0.f) - r1) * c->fade;
                c->fade -= (float)(130.0 / ENGINE_HZ);
            }
            if (r1 > 0.f) {
                value += r1 * c->rev2[c->ridx % 366] * 0.5f;
                value += r1 * c->rev4[c->ridx % 732] * 0.5f;          /* (zepto8 also weights the second tap by the first one's amount) */
            }
            c->rev2[c->ridx % 366] = value;
            c->rev4[c->ridx % 732] = value;
            c->ridx++;
            float cl = value > 0.99f ? 0.99f : value < -0.99f ? -0.99f : value;
            if (!(mask >> chn & 1)) { out[i] += cl; if (absf(cl) > amp[chn]) amp[chn] = absf(cl); }
        }
        c->last = ns;
        if (v) {
            unsigned lv = (unsigned)(amp[chn] * 2.0f * 255.f);
            v->level[chn] = pc_voice_decay(v->level[chn], lv > 255 ? 255 : (uint8_t)lv);
            if (ns.volume > 0.f && ns.inst == 6) { v->cents[chn] = PC_VOICE_NOPITCH; v->flags[chn] = PC_VF_NOISE; }
            else if (ns.volume > 0.f && ns.freq > 1.f) {
                v->cents[chn] = (int16_t)pc_voice_cents_of_hz_x100((long)(ns.freq * 100.f));
                v->flags[chn] = v->cents[chn] == PC_VOICE_NOPITCH ? 0 : PC_VF_PITCH;
            } else { v->cents[chn] = PC_VOICE_NOPITCH; v->flags[chn] = 0; }
        }
    }
    for (int i = 0; i < STEP; i++) { float m = out[i]; out[i] = m > 1.f ? 1.f : m < -1.f ? -1.f : m; }
}

/* ---- the backend ----------------------------------------------------------------------------------------------------------- */

static void begin(p8_t *p, int pattern)
{
    memset(p->ch, 0, sizeof p->ch);
    for (int i = 0; i < 4; i++) { p->ch[i].main.sfx = p->ch[i].custom.sfx = p->ch[i].sfx_music = -1; p->ch[i].last_note = -1; p->ch[i].last_sfx = -2; }
    p->rng = 0x9E3779B9u;
    for (int i = 0; i < 64; i++) p->kf[i] = key_hz[i];
    p->pattern = -1;
    p->count = 0;
    p->ended = false;
    p->bpos = STEP;
    p->prev = 0;
    p->phase = 0;
    p->played = 0;
    set_pattern(p, pattern);
}

/* 0, or why not: -1 not a usable cart, -2 a PNG that is not a PICO-8 one (160x205 8-bit RGBA), -3 a cart without music */
static int p8_parse(p8_t *p, const pc_reader_t *r)
{
    if (r->size < 16 || r->size > MAX_FILE) return -1;
    uint8_t *f = malloc(r->size);
    if (!f) return -1;
    if (r->read(r->u, 0, f, r->size) != (int)r->size) { free(f); return -1; }
    int rc = 0;
    if (!memcmp(f, "\x89PNG", 4)) {
        uint8_t *cart = NULL;
        rc = decode_png(f, r->size, &cart);
        if (!rc) { parse_rom(p, cart + ROM_MUSIC); free(cart); }
    } else if (!memcmp(f, "pico-8 cartridge", 16)) parse_text(p, f, r->size);
    else rc = -1;
    free(f);
    if (rc) return rc;
    find_starts(p);
    return p->nstarts < 1 ? -3 : 0;
}

static int p8_open(void **hp, const pc_reader_t *r, uint32_t start_off)
{
    p8_t *p = calloc(1, sizeof *p);
    if (!p) return -1;
    if (p8_parse(p, r)) { free(p); return -1; }
    p->start_pat = start_off < 64 && start_off > 0 && !pat_empty(p, (int)start_off) ? start_off : (uint32_t)p->starts[0];
    p->resume = start_off < 64 && start_off > 0 && !pat_empty(p, (int)start_off) ? start_off : 0;
    p->length_ms = seq_ms(p, (int)p->start_pat);
    begin(p, (int)p->start_pat);
    *hp = p;
    return 0;
}

int pc_p8_check(const pc_reader_t *r)
{
    p8_t *p = calloc(1, sizeof *p);
    if (!p) return -1;
    int rc = p8_parse(p, r);
    free(p);
    return rc;
}

static int p8_tracks(void *h) { return ((p8_t *)h)->nstarts; }

static int p8_start_track(void *h, int idx)
{
    p8_t *p = h;
    if (idx < 0 || idx >= p->nstarts) return -1;
    p->sub = idx;
    p->start_pat = p->resume ? p->resume : (uint32_t)p->starts[idx];   /* a resume inside a sub-track */
    p->resume = 0;
    p->length_ms = seq_ms(p, (int)p->start_pat);
    begin(p, (int)p->start_pat);
    return 0;
}

static int p8_render(void *h, float *out, int frames)
{
    p8_t *p = h;
    p->resume = 0;
    pc_voices_t *v = pc_extvoice_get(p);
    if (!v && !p->ended) v = pc_extvoice_take(p, 4);
    int done = 0;
    while (done < frames) {
        if (p->bpos >= STEP) {
            if (p->ended && p->ch[0].main.sfx < 0 && p->ch[1].main.sfx < 0 && p->ch[2].main.sfx < 0 && p->ch[3].main.sfx < 0) break;
            step(p, p->buf, v);
            p->bpos = 0;
        }
        float cur = p->buf[p->bpos];
        float o = p->phase == 0 ? 0.5f * (p->prev + cur) : cur;
        out[2 * done] = out[2 * done + 1] = o;
        done++;
        if (p->phase == 0) p->phase = 1;
        else { p->phase = 0; p->prev = cur; p->bpos++; }
    }
    p->played += (uint32_t)done;
    return done;
}

static uint32_t p8_rate(void *h) { (void)h; return OUT_RATE; }
static int32_t p8_len(void *h) { return ((p8_t *)h)->length_ms; }
static int32_t p8_pos(void *h) { return (int32_t)((uint64_t)((p8_t *)h)->played * 1000 / OUT_RATE); }
static uint32_t p8_bytepos(void *h) { p8_t *p = h; return p->pattern >= 0 ? (uint32_t)p->pattern : p->start_pat; }
static void p8_close(void *h)
{
    pc_extvoice_release(h);
    free(h);
}

const pc_backend_t pc_backend_p8 = {
    .open = p8_open, .render = p8_render, .rate = p8_rate, .track_count = p8_tracks, .start_track = p8_start_track,
    .length_ms = p8_len, .position_ms = p8_pos, .close = p8_close, .byte_pos = p8_bytepos, .name = "PICO-8",
};
