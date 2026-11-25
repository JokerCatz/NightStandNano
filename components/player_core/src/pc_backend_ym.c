// SPDX-License-Identifier: LGPL-2.1-or-later
/* Atari ST YM register dumps (.ym): YM2! / YM3! / YM3b! / YM5! / YM6!, plain or packed with LHA -lh5- (pc_lh5.c). The file is a recording of the
 * 16 registers of the YM2149 sound chip, one set per 50 Hz frame (the YM5/6 header gives the rate, the chip clock and a loop frame), played here
 * through a small YM2149 model of our own: three square oscillators, the 17-bit noise generator, the 16 envelope shapes in the chip's 32 steps,
 * the logarithmic level table (1.5 dB steps), run at the chip's 250 kHz internal rate (clock / 8) and averaged down to the output sample rate.
 * YM5 / YM6 also carry the Atari ST's tricks, which are played as the original drivers did, driven by the MFP timers (2457600 Hz): the SID voice
 * (volume square-wave at timer speed), digidrums (a sample written to a volume register at timer speed) and the sync-buzzer (envelope restarts
 * at timer speed). The driver logic follows the public descriptions of the format and Arnaud Carre's AtariAudio (MIT) as a reference for the
 * register meanings. Not played, refused with a reason: the digital-mix (MIX1) and tracker (YMT1 / YMT2) variants. YM2's built-in digidrum bank is
 * left out (25 KB): those drum hits are silent.
 * The song plays once (the loop frame is where it would restart). byte_pos = the frame being played. Voices: A B C, level from the chip's level
 * index, pitch from the tone period and the clock, noise-only voices flagged; mute removes a voice from the mix. */
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"
#include "pc/pc_extvoice.h"
#include "pc/pc_lh5.h"
#include "pc/pc_vmute.h"

#define RATE 44100u
#define MAX_FILE (2u << 20)
#define MAX_PACKED (1u << 20)
#define MAX_DRUMS 40
#define MFP_CLOCK 2457600u

typedef struct { const uint8_t *data; uint32_t len; } drum_t;
typedef struct { int type; int voice; uint8_t sid_vol, shape; const uint8_t *smp; uint32_t smp_len, phase; uint32_t period; uint64_t acc; bool on; } fx_t;   /* type: 0 none, 1 sid, 2 digidrum, 3 sync buzzer */

typedef struct {
    uint8_t *buf;                       /* the whole (unpacked) file */
    uint32_t size;
    int kind;                           /* 2, 3, 5 (YM5), 6 (YM6) */
    const uint8_t *frames;              /* register data */
    uint32_t nframes, loop;
    uint32_t stride;                    /* bytes per frame: 14 or 16 */
    bool interleaved;
    uint32_t clock, tick_hz;
    drum_t drums[MAX_DRUMS];
    int ndrums;
    /* chip */
    uint8_t reg[16];
    uint32_t tcnt[3], tper[3];
    uint8_t tedge[3];
    uint32_t ncnt, nper, nrack;
    uint8_t nhalf, nbit;
    uint32_t ecnt, eper, estep, eramp;
    uint8_t eshape;
    int elevel;
    bool ehold;
    uint32_t inner, clk8;
    uint32_t T;                         /* the chip tick clock (250 kHz); event times below are on it, compared as signed differences */
    uint32_t tnext[3], enext, nnext;    /* when each oscillator, the envelope and the noise generator change next */
    uint32_t tlast[3], elast, nlast;    /* when they changed last (a period write moves the next change relative to this) */
    uint32_t tacc[3], eacc, nacc;       /* a parked source has been caught up to this tick */
    uint32_t val;                       /* cached chip_value(); valid while !dirty */
    bool dirty;
    bool tpark[3], epark, npark;        /* a source that cannot be heard is not scheduled; its state is caught up (exactly) when it is needed again */
    float dc;
    fx_t fx[2];
    uint32_t frame;                     /* next frame to play */
    uint32_t in_frame;                  /* output samples left in the current frame */
    uint32_t spf_num, spf_acc;          /* samples per frame as a fraction: RATE / tick_hz */
    uint32_t played;
    int32_t length_ms;
    bool ended, started;
    uint8_t armed;
    uint32_t mute;                      /* pc_vmute mask, read once per output sample */
} ym_t;

static const float lvl_amp[32] = {      /* 32767 * 2^((i - 31) / 4): the YM2149's 1.5 dB steps */
    152, 181, 215, 255, 304, 362, 430, 511, 608, 724, 861, 1023, 1217, 1448, 1722, 2047,
    2435, 2896, 3444, 4095, 4870, 5792, 6888, 8191, 9741, 11584, 13776, 16383, 19483, 23169, 27553, 32767 };

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint32_t be16(const uint8_t *p) { return (uint32_t)p[0] << 8 | p[1]; }

/* ---- the chip -------------------------------------------------------------------------------------------------------------- */

static void env_restart(ym_t *y)
{
    y->estep = 0; y->eramp = 0; y->ecnt = 0; y->ehold = false;
    y->elevel = (y->eshape & 4) ? 0 : 31;
}

static void env_advance(ym_t *y)
{
    if (y->ehold) return;
    const int attack = (y->eshape & 4) != 0, cont = (y->eshape & 8) != 0, hold = (y->eshape & 1) != 0, alt = (y->eshape & 2) != 0;
    if (++y->estep >= 32) {
        y->estep = 0;
        y->eramp++;
        if (!cont) { y->ehold = true; y->elevel = 0; return; }            /* shapes 0..7: one ramp, then silence */
        if (hold) { y->ehold = true; y->elevel = (attack ^ alt) ? 31 : 0; return; }
    }
    int up = attack ^ ((alt && (y->eramp & 1)) ? 1 : 0);
    y->elevel = up ? (int)y->estep : 31 - (int)y->estep;
}

/* the tick at which an oscillator / the envelope / the noise generator changes next (a period write applies to the running count: a period at or below the
 * count changes it on the very next tick) */
static uint32_t later(uint32_t t, uint32_t floor_t) { return (int32_t)(t - floor_t) < 0 ? floor_t : t; }
static uint32_t eff(uint32_t p) { return p ? p : 1; }
#define FAR 0x40000000u                                                      /* a parked source's "next change" lies this far ahead */
static uint32_t tone_next(const ym_t *y, int c) { return later(y->tlast[c] + eff(y->tper[c]), y->T + 1); }
static uint32_t env_next(const ym_t *y) { return later(y->elast + eff(y->eper), y->T + 1); }
static uint32_t noise_next(const ym_t *y)                                   /* shifts happen on odd ticks, 2 ticks per count */
{
    uint32_t cnt = (y->T - y->nlast) / 2, need = eff(y->nper);
    return y->nlast + 2 * (need > cnt ? need : cnt + 1);
}

static void noise_shift(ym_t *y)
{
    y->nbit = (uint8_t)((y->nrack ^ (y->nrack >> 2)) & 1);
    y->nrack = (y->nrack >> 1) | ((uint32_t)y->nbit << 16);
    y->ncnt = 0;
}

/* What a parked source has done since it was last caught up, in one go. The period in force applies from tacc on; a count already past the period
 * toggles on the first tick after tacc (the same rule as a running source). Called before every period write and when the source matters again, so a
 * write never rewrites history. */
static void tone_catch(ym_t *y, int c)
{
    const uint32_t per = eff(y->tper[c]);
    const uint32_t first = later(y->tlast[c] + per, y->tacc[c] + 1);
    if ((int32_t)(first - y->T) <= 0) {
        const uint32_t k = 1 + (y->T - first) / per;
        if (k & 1) y->tedge[c] ^= 1;
        y->tlast[c] = first + (k - 1) * per;
    }
    y->tacc[c] = y->T;
}
static void noise_catch(ym_t *y)
{
    const uint32_t step = 2 * eff(y->nper), cnt = (y->nacc - y->nlast) / 2;
    const uint32_t first = y->nlast + 2 * (eff(y->nper) > cnt ? eff(y->nper) : cnt + 1);
    if ((int32_t)(first - y->T) <= 0) {
        const uint32_t k = 1 + (y->T - first) / step;
        for (uint32_t i = 0, kk = k % 131071u; i < kk; i++) noise_shift(y);  /* the 17-bit generator repeats every 131071 shifts */
        y->nlast = first + (k - 1) * step;
    }
    y->nacc = y->T;
}
static void env_env_advance(ym_t *y);
static void env_catch(ym_t *y)
{
    const uint32_t per = eff(y->eper);
    const uint32_t first = later(y->elast + per, y->eacc + 1);
    if ((int32_t)(first - y->T) <= 0) {
        const uint32_t k = 1 + (y->T - first) / per;
        for (uint32_t i = 0, kk = k <= 128 ? k : 64 + k % 64; i < kk; i++) env_env_advance(y);   /* past 64 steps every shape is periodic (or held) with a period dividing 64 */
        y->elast = first + (k - 1) * per;
    }
    y->eacc = y->T;
}

/* which sources matter for the sound right now: park the others, catch up the ones that matter again */
static void sched_refresh(ym_t *y)
{
    for (int c = 0; c < 3; c++) {
        const bool need = !(y->reg[7] >> c & 1);
        if (need && y->tpark[c]) { tone_catch(y, c); y->tpark[c] = false; y->tnext[c] = tone_next(y, c); }
        else if (!need && !y->tpark[c]) { y->tpark[c] = true; y->tacc[c] = y->T; y->tnext[c] = y->T + FAR; }
    }
    const bool nneed = (y->reg[7] & 0x38) != 0x38;
    if (nneed && y->npark) { noise_catch(y); y->npark = false; y->nnext = noise_next(y); }
    else if (!nneed && !y->npark) { y->npark = true; y->nacc = y->T; y->nnext = y->T + FAR; }
    const bool eneed = ((y->reg[8] | y->reg[9] | y->reg[10]) & 0x10) != 0;
    if (eneed && y->epark) { env_catch(y); y->epark = false; y->enext = env_next(y); }
    else if (!eneed && !y->epark) { y->epark = true; y->eacc = y->T; y->enext = y->T + FAR; }
}

static void env_env_advance(ym_t *y) { env_advance(y); }

static void wr(ym_t *y, int r, uint8_t v)
{
    static const uint8_t mask[14] = { 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x1f, 0x3f, 0x1f, 0x1f, 0x1f, 0xff, 0xff, 0x0f };
    if (r < 0 || r > 13) return;
    if (r <= 5 && y->tpark[r >> 1]) tone_catch(y, r >> 1);                   /* a parked source is brought up to date before its period changes */
    else if (r == 6 && y->npark) noise_catch(y);
    else if ((r == 11 || r == 12) && y->epark) env_catch(y);
    y->reg[r] = (uint8_t)(v & mask[r]);
    y->dirty = true;
    if (r <= 5) {
        int c = r >> 1;
        y->tper[c] = (uint32_t)(y->reg[c * 2 + 1] << 8 | y->reg[c * 2]);
        if (!y->tpark[c]) y->tnext[c] = tone_next(y, c);
    } else if (r == 6) {
        y->nper = y->reg[6];
        if (!y->npark) y->nnext = noise_next(y);
    } else if (r == 11 || r == 12) {
        y->eper = (uint32_t)(y->reg[12] << 8 | y->reg[11]);
        if (!y->epark) y->enext = env_next(y);
    }
    else if (r == 13) { y->eshape = y->reg[13]; env_restart(y); y->elast = y->T; y->eacc = y->T; if (!y->epark) y->enext = env_next(y); }
    if (r == 7 || (r >= 8 && r <= 10)) sched_refresh(y);
}

/* the mixed amplitude (0..3 * 32767) of the chip's current state */
static const float inv3[32] = { 0, 1.0f / 3.0f, 1.0f / 6.0f, 1.0f / 9.0f, 1.0f / 12.0f, 1.0f / 15.0f, 1.0f / 18.0f, 1.0f / 21.0f, 1.0f / 24.0f, 1.0f / 27.0f, 1.0f / 30.0f, 1.0f / 33.0f, 1.0f / 36.0f, 1.0f / 39.0f, 1.0f / 42.0f, 1.0f / 45.0f, 1.0f / 48.0f, 1.0f / 51.0f, 1.0f / 54.0f, 1.0f / 57.0f, 1.0f / 60.0f, 1.0f / 63.0f, 1.0f / 66.0f, 1.0f / 69.0f, 1.0f / 72.0f, 1.0f / 75.0f, 1.0f / 78.0f, 1.0f / 81.0f, 1.0f / 84.0f, 1.0f / 87.0f, 1.0f / 90.0f, 1.0f / 93.0f };   /* 1 / (3 * ticks): an output sample is 5 or 6 chip ticks (up to 23 for the highest clock allowed) */

static uint32_t chip_value(const ym_t *y)
{
    uint32_t sum = 0;
    for (int c = 0; c < 3; c++) {
        const bool tone = y->tedge[c] || (y->reg[7] >> c & 1), noise = y->nbit || (y->reg[7] >> (c + 3) & 1);
        if (tone && noise && !(y->mute >> c & 1)) sum += (uint32_t)lvl_amp[(y->reg[8 + c] & 0x10) ? y->elevel : (y->reg[8 + c] & 15) * 2 + 1];
    }
    return sum;
}

#ifdef PC_YM_REFERENCE
/* register writes of the straightforward model: the registers and the periods only (its counters run on their own) */
static void wr_ref(ym_t *y, int r, uint8_t v)
{
    static const uint8_t mask[14] = { 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x1f, 0x3f, 0x1f, 0x1f, 0x1f, 0xff, 0xff, 0x0f };
    if (r < 0 || r > 13) return;
    y->reg[r] = (uint8_t)(v & mask[r]);
    if (r <= 5) { int c = r >> 1; y->tper[c] = (uint32_t)(y->reg[c * 2 + 1] << 8 | y->reg[c * 2]); }
    else if (r == 6) y->nper = y->reg[6];
    else if (r == 11 || r == 12) y->eper = (uint32_t)(y->reg[12] << 8 | y->reg[11]);
    else if (r == 13) { y->eshape = y->reg[13]; env_restart(y); }
}

/* the straightforward model: one tick of the chip's internal 250 kHz clock at a time (the reference the fast path is checked against) */
static uint32_t chip_tick(ym_t *y)
{
    for (int c = 0; c < 3; c++)
        if (++y->tcnt[c] >= y->tper[c]) { y->tedge[c] ^= 1; y->tcnt[c] = 0; }
    if (++y->ecnt >= y->eper) { y->ecnt = 0; env_advance(y); }
    y->nhalf ^= 1;
    if (y->nhalf && ++y->ncnt >= y->nper) noise_shift(y);                    /* the noise generator runs at half the speed */
    return chip_value(y);
}

static float chip_sample_ref(ym_t *y)
{
    uint32_t acc = 0, n = 0;
    do {
        acc += chip_tick(y);
        n++;
        y->inner += RATE;
    } while (y->inner < y->clk8);
    y->inner -= y->clk8;
    float v = (float)acc * inv3[n];
    y->dc += (v - y->dc) * (1.0f / 2048.0f);
    return (v - y->dc) * (1.0f / 32768.0f);
}
#endif

/* The same model, event by event: between two changes of the oscillators, the noise or the envelope the mix is constant, so a run of ticks is added as
 * value x length. The next change of each source is kept as a time on the chip's tick clock, so a sample with no event (the usual case: the tones are far
 * below 125 kHz) is a few compares and one multiply; checked bit for bit against the tick-by-tick model (test_ym_ref). */
static float chip_sample(ym_t *y)
{
    uint32_t n = 0;                                                          /* chip ticks that make up this output sample (5 or 6) */
    do { n++; y->inner += RATE; } while (y->inner < y->clk8);
    y->inner -= y->clk8;
    if (y->dirty) { y->val = chip_value(y); y->dirty = false; }
    const uint32_t end = y->T + n;
    uint32_t cur = y->T, acc = 0, val = y->val;
    for (;;) {
        uint32_t te = y->tnext[0];
        if ((int32_t)(y->tnext[1] - te) < 0) te = y->tnext[1];
        if ((int32_t)(y->tnext[2] - te) < 0) te = y->tnext[2];
        if ((int32_t)(y->enext - te) < 0) te = y->enext;
        if ((int32_t)(y->nnext - te) < 0) te = y->nnext;
        if ((int32_t)(te - end) > 0) break;
        acc += val * (te - 1 - cur);                                         /* the ticks before the event tick see the old state */
        for (int c = 0; c < 3; c++) if (y->tnext[c] == te) {
            if (y->tpark[c]) y->tnext[c] = te + FAR;                         /* (a parked source's time came round after 2^30 ticks: stay parked) */
            else { y->tedge[c] ^= 1; y->tlast[c] = te; y->tnext[c] = te + eff(y->tper[c]); }
        }
        if (y->enext == te) {
            if (y->epark) y->enext = te + FAR; else { y->elast = te; env_advance(y); y->enext = te + eff(y->eper); }
        }
        if (y->nnext == te) {
            if (y->npark) y->nnext = te + FAR; else { noise_shift(y); y->nlast = te; y->nnext = te + 2 * eff(y->nper); }
        }
        val = chip_value(y);
        acc += val;                                                          /* the event tick itself already sees the new state */
        cur = te;
    }
    acc += val * (end - cur);
    y->val = val;
    y->T = end;
    float v = (float)acc * inv3[n];
    y->dc += (v - y->dc) * (1.0f / 2048.0f);                               /* a slow one-pole high-pass takes the DC offset out */
    return (v - y->dc) * (1.0f / 32768.0f);
}

/* ---- the MFP timers behind the YM5/6 tricks -------------------------------------------------------------------------------- */

static void set_timer(fx_t *f, int prediv, int count)
{
    static const uint32_t pre[8] = { 0, 4, 10, 16, 50, 64, 100, 200 };
    const uint32_t per = pre[prediv & 7] * (uint32_t)count;
    if (per == f->period && f->on) return;                                  /* writing the same setting to a running timer does not restart it */
    f->period = per;
    f->on = per != 0;
    f->acc = 0;
}

static void fx_fire(ym_t *y, fx_t *f)
{
    if (f->type == 1) { f->phase++; wr(y, f->voice + 8, (f->phase & 1) ? f->sid_vol : 0); }
    else if (f->type == 3) wr(y, 13, f->shape);
    else if (f->type == 2) {
        if (f->phase < f->smp_len) { wr(y, f->voice + 8, f->smp[f->phase]); f->phase++; }
        else { f->on = false; f->type = 0; }
    }
}

static void timers_run(ym_t *y)                                            /* called once per output sample */
{
    for (int t = 0; t < 2; t++) {
        fx_t *f = &y->fx[t];
        if (!f->on) continue;
        f->acc += (uint64_t)MFP_CLOCK << 16;
        const uint64_t per = (uint64_t)f->period * RATE << 16;
        while (f->on && f->acc >= per) { f->acc -= per; fx_fire(y, f); }
    }
}

/* ---- the 50 Hz driver ------------------------------------------------------------------------------------------------------ */

static uint8_t rd(const ym_t *y, uint32_t frame, int r)
{
    return y->interleaved ? y->frames[(uint64_t)y->nframes * (uint32_t)r + frame] : y->frames[(uint64_t)frame * y->stride + (uint32_t)r];
}

static uint32_t fx_decode(ym_t *y, int slot, int rc, int rp, int rn)
{
    fx_t *f = &y->fx[slot];
    uint32_t skip = 0;
    int code = rd(y, y->frame, rc) & 0xf0, prediv = rd(y, y->frame, rp) >> 5 & 7, count = rd(y, y->frame, rn);
    if (y->kind == 5) { code &= 0x30; if (code) code |= slot ? 0x40 : 0x00; }          /* YM5: slot 0 is a SID voice, slot 1 a digidrum */
    if (code & 0x30) {
        f->voice = ((code & 0x30) >> 4) - 1;
        switch (code & 0xc0) {
        case 0x00:
            f->type = 1; f->sid_vol = rd(y, y->frame, f->voice + 8) & 15; skip = 1u << (f->voice + 8); set_timer(f, prediv, count); break;
        case 0x40: {
            int id = rd(y, y->frame, f->voice + 8) & 31;
            if (id < y->ndrums) {
                f->type = 2; f->smp = y->drums[id].data; f->smp_len = y->drums[id].len; f->phase = 0;
                skip = 1u << (f->voice + 8); set_timer(f, prediv, count);
            }
            break;
        }
        case 0xc0:
            f->type = 3; f->shape = rd(y, y->frame, f->voice + 8) & 15; skip = 1u << (f->voice + 8); set_timer(f, prediv, count); break;
        default: break;
        }
    } else if (f->type != 2) { f->on = false; f->type = 0; }                        /* a running digidrum stops by itself */
    if (f->type == 2) skip |= 1u << (f->voice + 8);
    return skip;
}

static void frame_tick(ym_t *y)
{
    if (y->kind == 2) {                                                      /* YM2 (Mad Max): registers 0..10, and r13 != 0xff = the "buzzer" envelope; its built-in drums are left out */
        if (rd(y, y->frame, 13) != 0xff) { wr(y, 11, rd(y, y->frame, 11)); wr(y, 12, 0); wr(y, 13, 10); }
        for (int r = 0; r <= 10; r++) wr(y, r, rd(y, y->frame, r));
        return;
    }
    uint32_t skip = 0;
    if (y->kind >= 5) { skip |= fx_decode(y, 0, 1, 6, 14); skip |= fx_decode(y, 1, 3, 8, 15); }
    uint8_t r7 = rd(y, y->frame, 7);
    for (int t = 0; t < 2; t++) if (y->fx[t].type == 2) r7 |= (uint8_t)((1 | 8) << y->fx[t].voice);     /* a digidrum silences tone and noise of its voice */
    wr(y, 7, r7);
    skip |= 1u << 7;
    int eper = rd(y, y->frame, 12) << 8 | rd(y, y->frame, 11);
    if (eper == 0) { wr(y, 11, 0xff); wr(y, 12, 0xff); skip |= 1u << 11 | 1u << 12; }   /* some files set the envelope one frame late: a very long period meanwhile */
    for (int r = 0; r <= 12; r++) if (!(skip >> r & 1)) wr(y, r, rd(y, y->frame, r));
    uint8_t r13 = rd(y, y->frame, 13);
    if (r13 != 0xff) wr(y, 13, r13);
}

/* ---- parsing --------------------------------------------------------------------------------------------------------------- */

/* 0 ok, -1 not usable, -2 a variant that is not played (MIX1 / YMT1 / YMT2) */
static int ym_parse(ym_t *y, uint8_t *b, uint32_t n)
{
    if (n < 8) return -1;
    if (!memcmp(b, "MIX1", 4) || !memcmp(b, "YMT1", 4) || !memcmp(b, "YMT2", 4)) return -2;
    y->buf = b; y->size = n;
    if (!memcmp(b, "YM2!", 4) || !memcmp(b, "YM3!", 4) || !memcmp(b, "YM3b", 4)) {
        const bool b3 = !memcmp(b, "YM3b", 4);
        y->kind = b[2] == '2' ? 2 : 3;
        y->stride = 14; y->interleaved = true; y->clock = 2000000; y->tick_hz = 50;
        uint32_t body = n - 4 - (b3 ? 4 : 0);
        y->nframes = body / 14;
        y->frames = b + 4;
        y->loop = b3 && n >= 8 ? (uint32_t)(b[n - 4] | b[n - 3] << 8 | b[n - 2] << 16 | (uint32_t)b[n - 1] << 24) : 0;
    } else if ((!memcmp(b, "YM5!", 4) || !memcmp(b, "YM6!", 4)) && n >= 34 && !memcmp(b + 4, "LeOnArD!", 8)) {
        y->kind = b[2] == '5' ? 5 : 6;
        y->nframes = be32(b + 12);
        uint32_t attr = be32(b + 16);
        int nd = (int)be16(b + 20);
        y->clock = be32(b + 22);
        y->tick_hz = be16(b + 26);
        y->loop = be32(b + 28);
        uint32_t pos = 34 + be16(b + 32);
        if (nd > MAX_DRUMS) return -1;
        for (int i = 0; i < nd; i++) {
            if ((uint64_t)pos + 4 > n) return -1;
            uint32_t l = be32(b + pos);
            pos += 4;
            if (l > n - pos) return -1;
            y->drums[i].data = b + pos; y->drums[i].len = l;
            pos += l;
        }
        y->ndrums = nd;
        if (!(attr & 4)) {                                                   /* 8-bit samples: reduced to the chip's 4-bit volumes (the Mad Max table), in place: the buffer is ours */
            static const uint8_t mm[16] = { 0x0, 0x7, 0x9, 0xa, 0xb, 0xc, 0xc, 0xd, 0xd, 0xd, 0xe, 0xe, 0xe, 0xf, 0xf, 0xf };
            for (int i = 0; i < nd; i++) for (uint32_t k = 0; k < y->drums[i].len; k++) ((uint8_t *)y->drums[i].data)[k] = mm[y->drums[i].data[k] >> 4];
        }
        for (int s = 0; s < 3; s++) {                                        /* three zero-terminated strings: name, author, comment */
            while (pos < n && b[pos]) pos++;
            if (pos >= n) return -1;
            pos++;
        }
        y->frames = b + pos; y->stride = 16; y->interleaved = (attr & 1) != 0;
        if ((uint64_t)pos + (uint64_t)y->nframes * 16 > n) {
            if (y->interleaved) return -1;
            y->nframes = (n - pos) / 16;                                     /* a cut file: the frames that are there */
        }
    } else return -1;
    if (y->clock < 500000 || y->clock > 8000000 || y->tick_hz < 10 || y->tick_hz > 400 || !y->nframes || y->nframes > (1u << 22)) return -1;
    if (y->loop >= y->nframes) y->loop = 0;
    return 0;
}

static int ym_load(ym_t *y, const pc_reader_t *r)
{
    if (r->size < 8 || r->size > MAX_PACKED * 2) return -1;
    uint8_t *f = malloc(r->size);
    if (!f) return -1;
    if (r->read(r->u, 0, f, r->size) != (int)r->size) { free(f); return -1; }
    uint32_t n = r->size;
    if (pc_lh5_is_packed(f, n)) {
        uint8_t *u;
        uint32_t ul;
        if (n > MAX_PACKED || pc_lh5_unpack(f, n, MAX_FILE, &u, &ul)) { free(f); return -1; }
        free(f);
        f = u;
        n = ul;
    } else if (n > MAX_FILE) { free(f); return -1; }
    int rc = ym_parse(y, f, n);
    if (rc) { free(f); y->buf = NULL; return rc; }
    return 0;
}

int pc_ym_check(const pc_reader_t *r)
{
    ym_t *y = calloc(1, sizeof *y < 1100 ? 1100 : sizeof *y);
    if (!y) return -1;
    int rc = ym_load(y, r);
    free(y->buf);
    free(y);
    return rc;
}

/* ---- the backend ----------------------------------------------------------------------------------------------------------- */

static void chip_reset(ym_t *y)
{
    memset(y->reg, 0, sizeof y->reg);
    memset(y->tcnt, 0, sizeof y->tcnt);
    memset(y->tper, 0, sizeof y->tper);
    memset(y->tedge, 0, sizeof y->tedge);
    y->nper = 0; y->ncnt = 0; y->nrack = 1; y->nhalf = 0; y->nbit = 0;
    y->eper = 0; y->ecnt = 0; y->eshape = 0; y->estep = 0; y->eramp = 0; y->elevel = 0; y->ehold = true;
    y->reg[7] = 0x3f;
    y->inner = 0; y->clk8 = y->clock / 8; y->dc = 0;
    y->T = 0; y->elast = 0; y->nlast = (uint32_t)-1; y->dirty = true;
    for (int c = 0; c < 3; c++) { y->tlast[c] = 0; y->tnext[c] = 1; }
    y->enext = 1; y->nnext = 1;
    for (int c = 0; c < 3; c++) y->tpark[c] = false;
    y->epark = y->npark = false;
    sched_refresh(y);
    memset(y->fx, 0, sizeof y->fx);
}

static int ym_open(void **hp, const pc_reader_t *r, uint32_t start_off)
{
    ym_t *y = calloc(1, sizeof *y < 1100 ? 1100 : sizeof *y);   /* never a small (internal-RAM) block */
    if (!y) return -1;
    if (ym_load(y, r)) { free(y); return -1; }
    chip_reset(y);
    y->frame = start_off < y->nframes ? start_off : 0;
    y->played = (uint32_t)((uint64_t)y->frame * RATE / y->tick_hz);
    y->length_ms = (int32_t)((uint64_t)y->nframes * 1000 / y->tick_hz);
    y->armed = 7;
    *hp = y;
    return 0;
}

static void voices_update(ym_t *y, pc_voices_t *v)
{
    for (int c = 0; c < 3; c++) {
        int idx = (y->reg[8 + c] & 0x10) ? y->elevel : (y->reg[8 + c] & 15) * 2 + 1;
        bool sounding = ((y->reg[7] >> c & 1) == 0 || (y->reg[7] >> (c + 3) & 1) == 0) && idx > 0;
        uint8_t now = sounding && !(y->mute >> c & 1) ? (uint8_t)(idx * 255 / 31) : 0;
        uint8_t arm = (uint8_t)(y->armed >> c & 1);
        if (pc_voice_level_strike(now, &arm)) v->strikes[c]++;
        y->armed = (uint8_t)((y->armed & ~(1u << c)) | (unsigned)arm << c);
        v->level[c] = pc_voice_decay(v->level[c], now);
        const uint32_t per = y->tper[c];
        if (sounding && !(y->reg[7] >> c & 1) && per > 1) {
            v->cents[c] = (int16_t)pc_voice_cents_of_hz_x100((long)((int64_t)y->clock * 100 / (16 * (int64_t)per)));
            v->flags[c] = v->cents[c] == PC_VOICE_NOPITCH ? 0 : PC_VF_PITCH;
        } else if (sounding) { v->cents[c] = PC_VOICE_NOPITCH; v->flags[c] = PC_VF_NOISE; }
        else { v->cents[c] = PC_VOICE_NOPITCH; v->flags[c] = 0; }
    }
}

static int ym_render(void *h, float *out, int frames)
{
    ym_t *y = h;
    pc_voices_t *v = pc_extvoice_get(y);
    if (!v && !y->ended) {
        v = pc_extvoice_take(y, 3);
        if (v) for (int i = 0; i < 3; i++) { v->name[i][0] = (char)('A' + i); v->name[i][1] = 0; }
    }
    int done = 0;
    while (done < frames) {
        if (y->in_frame == 0) {
            if (y->frame >= y->nframes) { y->ended = true; break; }                 /* the loop frame is where the song would start again: it has been played once */
            frame_tick(y);
            y->frame++;
            y->spf_acc += RATE;
            y->in_frame = y->spf_acc / y->tick_hz;                                   /* samples in this frame, the remainder carried over */
            y->spf_acc %= y->tick_hz;
            if (v) voices_update(y, v);
        }
        { uint32_t m = v ? pc_vmute_mask() : 0; if (m != y->mute) { y->mute = m; y->dirty = true; } }
        timers_run(y);
        float s = chip_sample(y);
        out[2 * done] = out[2 * done + 1] = s;
        y->in_frame--;
        y->played++;
        done++;
    }
    return done;
}

static uint32_t ym_rate(void *h) { (void)h; return RATE; }
static int32_t ym_len(void *h) { return ((ym_t *)h)->length_ms; }
static int32_t ym_pos(void *h) { ym_t *y = h; return (int32_t)((uint64_t)y->played * 1000 / RATE); }
static uint32_t ym_bytepos(void *h) { return ((ym_t *)h)->frame; }
static void ym_close(void *h)
{
    ym_t *y = h;
    pc_extvoice_release(y);
    free(y->buf);
    free(y);
}

const pc_backend_t pc_backend_ym = {
    .open = ym_open, .render = ym_render, .rate = ym_rate, .length_ms = ym_len, .position_ms = ym_pos, .close = ym_close, .byte_pos = ym_bytepos, .name = "YM",
};
