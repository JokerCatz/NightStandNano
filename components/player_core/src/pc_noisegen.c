// SPDX-License-Identifier: LGPL-2.1-or-later
/* Ported from RP2350_player (JukeChip) src/player/player_backend_noise.c - same integer arithmetic and
 * calibration (see that file for how each constant was derived); this version keeps its state in a struct. */
#include "pc/pc_noisegen.h"
#include "pc/pc_i18n.h"
#include <string.h>

static const uint32_t SEED[PC_NOISE_COLORS] = { 0x9E3779B1u, 0x85EBCA77u, 0xC2B2AE3Du, 0x27D4EB2Fu,
                                                0x165667B1u, 0xD3A2646Cu, 0xFD7046C5u, 0xB55A4F09u };
#define CH1_SEED_XOR 0xA5A5A5A5u
static const int32_t GAIN_Q8[PC_NOISE_COLORS] = { 67, 19, 12, 47, 47, 53, 344, 183 };   /* RMS ~4915 of int16 */
#define DC_R_Q15 32760            /* DC blocker pole ~1.7 Hz */
#define BROWN_LEAK_SHIFT 6        /* leaky integrator corner ~110 Hz */
#define GREEN_F_Q15 2334          /* SVF band-pass 500 Hz, Q 1 */
#define GREEN_Q_Q15 32768
#define GREY_F_Q15 13900          /* SVF notch 3 kHz, Q 0.6: a rough inverse equal-loudness */
#define GREY_Q_Q15 54613
#define VELVET_AVG_GAP 22         /* ~2000 impulses / s */

static uint32_t xorshift32(uint32_t *s) { uint32_t x = *s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; *s = x; return x; }
static int32_t white(pc_noise_ch_t *c) { return (int32_t)(xorshift32(&c->rng) >> 16) - 32768; }
static int32_t mulq15(int32_t a, int32_t b)
{
    int64_t p = (int64_t)a * b;
    return p >= 0 ? (int32_t)((p + 16384) >> 15) : (int32_t)(-((-p + 16384) >> 15));
}
static int ctz(uint32_t x) { if (!x) return 0; int n = 0; while (!(x & 1u)) { x >>= 1; n++; } return n; }
static int32_t pink(pc_noise_ch_t *c)
{
    c->pink_counter++;
    int row = ctz(c->pink_counter) % 16;
    int32_t v = white(c);
    c->pink_sum += v - c->pink_rows[row];
    c->pink_rows[row] = v;
    return c->pink_sum;
}
static void svf(pc_noise_ch_t *c, int32_t x, int32_t f, int32_t q, int32_t *band, int32_t *notch)
{
    int32_t low = c->svf_low + mulq15(c->svf_band, f);
    int32_t high = x - low - mulq15(c->svf_band, q);
    int32_t b = c->svf_band + mulq15(high, f);
    c->svf_low = low;
    c->svf_band = b;
    *band = b;
    *notch = high + low;
}

static int16_t next(pc_noise_ch_t *c, int color)
{
    int32_t raw = 0, band, notch;
    switch (color) {
    case PC_NOISE_WHITE: raw = white(c); break;
    case PC_NOISE_PINK: raw = pink(c); break;
    case PC_NOISE_BROWN: { int32_t v = white(c); c->brown_y = c->brown_y - (c->brown_y >> BROWN_LEAK_SHIFT) + v; raw = c->brown_y; break; }
    case PC_NOISE_BLUE: { int32_t p = pink(c); raw = p - c->prev_raw; c->prev_raw = p; break; }     /* +3 dB/oct */
    case PC_NOISE_VIOLET: { int32_t w = white(c); raw = w - c->prev_raw; c->prev_raw = w; break; }  /* +6 dB/oct */
    case PC_NOISE_GREY: svf(c, white(c), GREY_F_Q15, GREY_Q_Q15, &band, &notch); raw = notch; break;
    case PC_NOISE_GREEN: svf(c, white(c), GREEN_F_Q15, GREEN_Q_Q15, &band, &notch); raw = band; break;
    case PC_NOISE_VELVET:
        if (--c->velvet_gap <= 0) {
            raw = (white(c) & 1) ? 32767 : -32767;
            c->velvet_gap = 1 + (int32_t)((uint32_t)white(c) % (2u * VELVET_AVG_GAP));
        }
        break;
    default: break;
    }
    int32_t y = raw - c->dc_prev_x + mulq15(c->dc_prev_y, DC_R_Q15);
    c->dc_prev_x = raw;
    c->dc_prev_y = y;
    int32_t g = (int32_t)(((int64_t)y * GAIN_Q8[color]) >> 8);
    return (int16_t)(g > 32767 ? 32767 : g < -32768 ? -32768 : g);
}

void pc_noisegen_start(pc_noisegen_t *g, int color)
{
    memset(g, 0, sizeof *g);
    g->color = color >= 0 && color < PC_NOISE_COLORS ? color : 0;
    g->ch[0].rng = SEED[g->color];
    g->ch[1].rng = SEED[g->color] ^ CH1_SEED_XOR;
}

void pc_noisegen_render(pc_noisegen_t *g, float *out, int frames)
{
    for (int i = 0; i < frames; i++) {
        out[2 * i] = next(&g->ch[0], g->color) * (1.0f / 32768);
        out[2 * i + 1] = next(&g->ch[1], g->color) * (1.0f / 32768);
    }
}

const char *pc_noise_name(int color, int zh)
{
    static const char *const EN[PC_NOISE_COLORS] = { "White Noise", "Pink Noise", "Brown Noise", "Blue Noise",
                                                     "Violet Noise", "Grey Noise", "Green Noise", "Velvet Noise" };
    static const char *const ZH[PC_NOISE_COLORS] = { "白噪音", "粉紅噪音", "棕噪音", "藍噪音", "紫噪音", "灰噪音", "綠噪音", "絲絨噪音" };
    if (color < 0 || color >= PC_NOISE_COLORS) return "";
    return zh ? TZ(ZH[color]) : EN[color];
}
