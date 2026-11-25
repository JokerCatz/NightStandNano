// SPDX-License-Identifier: LGPL-2.1-or-later
#include "spectrum.h"
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define N 1024
static int16_t s_win[N];                    /* mono ring, newest samples */
static volatile uint32_t s_wr;
static volatile uint32_t s_rate = 44100;
static void (*s_cb)(const uint8_t *);
#define N_OBS 2
static void (*s_obs[N_OBS])(const uint8_t *);       /* more consumers (board_display_init): same frame, same task */
static uint8_t s_last[NSN_BANDS];
void nsn_spec_last(uint8_t out[NSN_BANDS]) { memcpy(out, s_last, NSN_BANDS); }
static float s_cos[N / 2], s_sin[N / 2], s_hann[N];

bool nsn_spec_observe(void (*on_frame)(const uint8_t bands[NSN_BANDS]))
{
    for (int i = 0; i < N_OBS; i++) if (!s_obs[i]) { s_obs[i] = on_frame; return true; }
    return false;
}

static void emit(const uint8_t *bands)
{
    if (s_cb) s_cb(bands);
    for (int i = 0; i < N_OBS; i++) if (s_obs[i]) s_obs[i](bands);
}

void nsn_spec_set_rate(uint32_t hz) { s_rate = hz ? hz : 44100; }

void nsn_spec_push(const int16_t *st, size_t frames)
{
    uint32_t w = s_wr;
    for (size_t i = 0; i < frames; i++) { s_win[w & (N - 1)] = (int16_t)((st[2 * i] + st[2 * i + 1]) >> 1); w++; }
    s_wr = w;
}

static void fft(float *re, float *im)
{
    for (int i = 1, j = 0; i < N; i++) {                      /* bit reversal */
        int bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { float t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; }
    }
    for (int len = 2; len <= N; len <<= 1) {
        int step = N / len;
        for (int i = 0; i < N; i += len)
            for (int k = 0; k < len / 2; k++) {
                float wr = s_cos[k * step], wi = -s_sin[k * step];
                int a = i + k, b = i + k + len / 2;
                float xr = re[b] * wr - im[b] * wi, xi = re[b] * wi + im[b] * wr;
                re[b] = re[a] - xr; im[b] = im[a] - xi; re[a] += xr; im[a] += xi;
            }
    }
}

static void spec_task(void *arg)
{
    static float re[N], im[N];
    uint8_t bands[NSN_BANDS], held[NSN_BANDS] = { 0 };
    for (int i = 0; i < N / 2; i++) { s_cos[i] = cosf(6.2831853f * i / N); s_sin[i] = sinf(6.2831853f * i / N); }
    for (int i = 0; i < N; i++) s_hann[i] = 0.5f - 0.5f * cosf(6.2831853f * i / (N - 1));
    TickType_t last = xTaskGetTickCount();
    uint32_t prev_wr = 0;
    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(33));
        uint32_t w = s_wr;
        if (w == prev_wr) {                                     /* nothing new: fall to silence */
            for (int b = 0; b < NSN_BANDS; b++) held[b] = held[b] > 12 ? held[b] - 12 : 0;
            memcpy(bands, held, NSN_BANDS); memcpy(s_last, held, NSN_BANDS);
            emit(bands);
            continue;
        }
        prev_wr = w;
        for (int i = 0; i < N; i++) { re[i] = s_win[(w - N + i) & (N - 1)] * s_hann[i] / 32768.0f; im[i] = 0; }
        fft(re, im);
        float binhz = (float)s_rate / N, lo = 40.0f, hi = s_rate * 0.45f > 16000.0f ? 16000.0f : s_rate * 0.45f;
        for (int b = 0; b < NSN_BANDS; b++) {
            float f0 = lo * powf(hi / lo, (float)b / NSN_BANDS), f1 = lo * powf(hi / lo, (float)(b + 1) / NSN_BANDS);
            int k0 = (int)(f0 / binhz), k1 = (int)(f1 / binhz);
            if (k1 <= k0) k1 = k0 + 1;
            if (k1 > N / 2) k1 = N / 2;
            float m = 0;
            for (int k = k0; k < k1; k++) { float a = re[k] * re[k] + im[k] * im[k]; if (a > m) m = a; }
            float db = 10.0f * log10f(m * 4.0f / ((float)N * N / 4.0f) + 1e-12f);        /* ~ dBFS of a full-scale sine = 0 */
            int v = (int)((db + 70.0f) * (255.0f / 70.0f));
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            held[b] = v >= held[b] ? v : (held[b] > 10 ? held[b] - 10 : 0);              /* fast attack, slow release */
        }
        memcpy(bands, held, NSN_BANDS); memcpy(s_last, held, NSN_BANDS);
        emit(bands);
    }
}

void nsn_spec_start(void (*on_frame)(const uint8_t *)) { s_cb = on_frame; xTaskCreatePinnedToCoreWithCaps(spec_task, "spec", 6144, NULL, 3, NULL, 1, MALLOC_CAP_SPIRAM); }
