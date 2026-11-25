// SPDX-License-Identifier: LGPL-2.1-or-later
#include <math.h>
#include <string.h>
#include "pc/pc_dsp.h"

#define PI_F 3.14159265f

void pc_biquad_design(pc_biquad_t *bq, char type, float fc, float gain_db, float q, uint32_t rate)
{
    float w0 = 2.0f * PI_F * fc / (float)rate;
    float cw = cosf(w0), sw = sinf(w0);
    float omc = 2.0f * sinf(w0 * 0.5f) * sinf(w0 * 0.5f);   /* 1 - cos(w0) without cancellation at low fc */
    float A = powf(10.0f, gain_db / 40.0f), alpha = sw / (2.0f * q), sa = 2.0f * sqrtf(A) * alpha;
    float b0, b1, b2, a0, a1, a2;
    switch (type) {
    case 'L':
        b0 = A * ((A + 1) - (A - 1) * cw + sa); b1 = 2 * A * ((A - 1) - (A + 1) * cw); b2 = A * ((A + 1) - (A - 1) * cw - sa);
        a0 = (A + 1) + (A - 1) * cw + sa; a1 = -2 * ((A - 1) + (A + 1) * cw); a2 = (A + 1) + (A - 1) * cw - sa;
        break;
    case 'H':
        b0 = A * ((A + 1) + (A - 1) * cw + sa); b1 = -2 * A * ((A - 1) + (A + 1) * cw); b2 = A * ((A + 1) + (A - 1) * cw - sa);
        a0 = (A + 1) - (A - 1) * cw + sa; a1 = 2 * ((A - 1) - (A + 1) * cw); a2 = (A + 1) - (A - 1) * cw - sa;
        break;
    case 'l':
        b0 = omc / 2; b1 = omc; b2 = omc / 2; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha;
        break;
    case 'h':
        b0 = (2 - omc) / 2; b1 = -(2 - omc); b2 = (2 - omc) / 2; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha;
        break;
    default:   /* 'P' */
        b0 = 1 + alpha * A; b1 = -2 * cw; b2 = 1 - alpha * A; a0 = 1 + alpha / A; a1 = -2 * cw; a2 = 1 - alpha / A;
        break;
    }
    bq->b0 = b0 / a0; bq->b1 = b1 / a0; bq->b2 = b2 / a0; bq->a1 = a1 / a0; bq->a2 = a2 / a0;
    memset(bq->s1, 0, sizeof bq->s1);
    memset(bq->s2, 0, sizeof bq->s2);
}

void pc_biquad_run(pc_biquad_t *bq, float *x, int frames)
{
    for (int c = 0; c < 2; c++) {
        float s1 = bq->s1[c], s2 = bq->s2[c];
        for (int i = 0; i < frames; i++) {
            float in = x[2 * i + c], y = bq->b0 * in + s1;
            s1 = bq->b1 * in - bq->a1 * y + s2;
            s2 = bq->b2 * in - bq->a2 * y;
            x[2 * i + c] = y;
        }
        bq->s1[c] = s1;
        bq->s2[c] = s2;
    }
}

float pc_biquad_gain_db(const pc_biquad_t *bq, float f, uint32_t rate)
{
    float w = 2.0f * PI_F * f / (float)rate, c1 = cosf(w), s1 = sinf(w), c2 = cosf(2 * w), s2 = sinf(2 * w);
    float nr = bq->b0 + bq->b1 * c1 + bq->b2 * c2, ni = -(bq->b1 * s1 + bq->b2 * s2);
    float dr = 1.0f + bq->a1 * c1 + bq->a2 * c2, di = -(bq->a1 * s1 + bq->a2 * s2);
    return 10.0f * log10f((nr * nr + ni * ni) / (dr * dr + di * di));
}
