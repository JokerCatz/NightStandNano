// SPDX-License-Identifier: LGPL-2.1-or-later
#include "fmvgm_resamp.h"

/* sin(pi * x) for any float x, from a polynomial (no libm, so no platform differences). Range reduction to [-0.5, 0.5] then an odd minimax polynomial (error < 3e-7). */
static float sinpi_f(float x)
{
    float k = (float)(int32_t)(x + (x >= 0 ? 0.5f : -0.5f));      /* nearest integer */
    float r = x - k;                                              /* -0.5 .. 0.5 */
    int odd = ((int32_t)k) & 1;
    float z = r * 3.14159265f;
    float z2 = z * z;
    float s = z * (1.0f + z2 * (-0.16666667f + z2 * (0.0083333310f + z2 * (-0.00019840874f + z2 * 2.7525562e-06f))));
    return odd ? -s : s;
}

/* Kaiser window I0 by its power series */
static float i0_f(float x)
{
    float t = 1.0f, sum = 1.0f, q = x * x * 0.25f;
    for (int k = 1; k < 40; k++) { t *= q / (float)(k * k); sum += t; if (t < 1e-9f * sum) break; }
    return sum;
}

static float sqrt_f(float x)                                       /* Newton, a fixed number of steps (no libm sqrt) */
{
    if (x <= 0.0f) return 0.0f;
    float g = x > 1.0f ? x * 0.5f : 1.0f;
    for (int i = 0; i < 12; i++) g = 0.5f * (g + x / g);
    return g;
}

float fmrs_beta(int taps) { return taps >= 32 ? 7.5f : taps >= 24 ? 6.5f : 5.5f; }

float fmrs_cutoff(uint32_t in_rate, uint32_t out_rate)
{
    float r = in_rate ? (float)out_rate / (float)in_rate : 1.0f;
    float edge = 0.93f;                                           /* the passband ends at 93 % of the narrower Nyquist: the rest of the way is the transition */
    float fc = 0.5f * (r < 1.0f ? r : 1.0f) * edge;
    return fc;
}

void fmrs_build(int16_t *coef, int taps, float fc, float beta)
{
    int H = taps / 2;
    float i0b = i0_f(beta);
    for (int p = 0; p <= FMRS_PHASES; p++) {
        float frac = (float)p / (float)FMRS_PHASES;
        float h[FMRS_MAX_TAPS], sum = 0.0f;
        for (int j = 0; j < taps; j++) {
            float d = (float)(j - H + 1) - frac;                  /* input sample minus output instant, in input frames */
            float a = 2.0f * fc * d;
            float sinc = (a > -1e-6f && a < 1e-6f) ? 1.0f : sinpi_f(a) / (3.14159265f * a);
            float u = d / (float)H;
            float w = 0.0f;
            if (u > -1.0f && u < 1.0f) w = i0_f(beta * sqrt_f(1.0f - u * u)) / i0b;
            h[j] = 2.0f * fc * sinc * w;
            sum += h[j];
        }
        int32_t q[FMRS_MAX_TAPS], qs = 0, big = 0;
        for (int j = 0; j < taps; j++) {
            float v = h[j] / sum * 32768.0f;
            q[j] = (int32_t)(v + (v >= 0 ? 0.5f : -0.5f));
            qs += q[j];
            if (q[j] > q[big]) big = j;
        }
        q[big] += 32768 - qs;                                     /* exact DC gain: the rounding error goes into the largest tap */
        for (int j = 0; j < taps; j++) coef[p * taps + j] = (int16_t)q[j];
    }
}

void fmrs_frame(const int16_t *coef, int taps, const int16_t *in, uint32_t frac, int16_t *out_lr)
{
    const int16_t *c0 = coef + (frac >> 24) * taps, *c1 = c0 + taps;
    int32_t f = (int32_t)((frac >> 16) & 0xFF);
    int32_t l0 = 0, r0 = 0, l1 = 0, r1 = 0;
    for (int j = 0; j < taps; j++) {
        int32_t a = c0[j], b = c1[j], xl = in[2 * j], xr = in[2 * j + 1];
        l0 += a * xl; r0 += a * xr; l1 += b * xl; r1 += b * xr;
    }
    int32_t l = l0 + ((((l1 - l0) >> 6) * f) >> 2), r = r0 + ((((r1 - r0) >> 6) * f) >> 2);     /* the difference of the two sums is small: no overflow */
    l = (l + 16384) >> 15; r = (r + 16384) >> 15;
    out_lr[0] = (int16_t)(l > 32767 ? 32767 : l < -32768 ? -32768 : l);
    out_lr[1] = (int16_t)(r > 32767 ? 32767 : r < -32768 ? -32768 : r);
}
