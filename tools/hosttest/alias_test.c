// SPDX-License-Identifier: LGPL-2.1-or-later
// Spectrum measurement of the chip-rate resamplers (host only): fmvgm's integer polyphase (taps 16 / 24 / 32 / 48) against the player's pc_rsq (68 taps, float) and a long reference filter.
//   alias_test sweep IN_RATE OUT_RATE            full-scale-ish sine at many input frequencies: level of the wanted tone, of the alias (tones above the output Nyquist) and the worst spur
//   alias_test file IN.i16 IN_RATE OUT_RATE      a recording of the chip (mono or left channel of interleaved stereo int16; arg 4: channels 1|2): band powers 0-16 / 16-20 / 20-22 / 22-24 kHz per method
// Build: see run_fm.sh (alias target). Exit 0.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "fmvgm_resamp.h"
#include "pc/pc_rsq.h"

static void fft(double *re, double *im, int n)
{
    for (int i = 1, j = 0; i < n; i++) { int bit = n >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) { double t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; } }
    for (int len = 2; len <= n; len <<= 1) {
        double a = -2 * M_PI / len, wr = cos(a), wi = sin(a);
        for (int i = 0; i < n; i += len) { double cr = 1, ci = 0;
            for (int j = 0; j < len / 2; j++) {
                double ur = re[i + j], ui = im[i + j], vr = re[i + j + len / 2] * cr - im[i + j + len / 2] * ci, vi = re[i + j + len / 2] * ci + im[i + j + len / 2] * cr;
                re[i + j] = ur + vr; im[i + j] = ui + vi; re[i + j + len / 2] = ur - vr; im[i + j + len / 2] = ui - vi;
                double t = cr * wr - ci * wi; ci = cr * wi + ci * wr; cr = t; } }
    }
}
#define N 65536
// power spectrum (linear power per bin, Blackman-Harris window, normalised so that a full-scale sine of amplitude A has A^2/4 * ... : use as relative power), x in [-1, 1]
static void spectrum(const float *x, double *pw)
{
    static double re[N], im[N];
    double wsum = 0;
    for (int i = 0; i < N; i++) { double a = 2 * M_PI * i / (N - 1), w = 0.35875 - 0.48829 * cos(a) + 0.14128 * cos(2 * a) - 0.01168 * cos(3 * a); re[i] = x[i] * w; im[i] = 0; wsum += w; }
    fft(re, im, N);
    for (int k = 0; k <= N / 2; k++) pw[k] = (re[k] * re[k] + im[k] * im[k]) / (wsum * wsum / 4);   // a sine of amplitude A: peak bin (spread over ~7 bins) sums to A^2 / 2 * ...
}
static double band(const double *pw, double lo, double hi, double rate)
{
    double s = 0; int a = (int)(lo / rate * N), b = (int)(hi / rate * N);
    for (int k = a; k <= b && k <= N / 2; k++) s += pw[k];
    return s;
}
static double db(double p) { return 10 * log10(p + 1e-30); }

// ---- the methods: each turns `in` (float, mono, in_rate) into `out` (float, mono, out_rate), returns the number of output samples ----
static int m_fmvgm(const float *in, int n, uint32_t ir, uint32_t orate, int taps, float *out, int cap)
{
    int H = taps / 2;
    int16_t *c = malloc((size_t)(FMRS_PHASES + 1) * taps * 2); fmrs_build(c, taps, fmrs_cutoff(ir, orate), fmrs_beta(taps));
    int16_t *buf = calloc((size_t)(n + 2 * taps + 4) * 2, 2);                      // stereo (L = R), H-1 leading zeros
    for (int i = 0; i < n; i++) { int v = (int)lrintf(in[i] * 32767.0f); buf[2 * (i + H - 1)] = buf[2 * (i + H - 1) + 1] = (int16_t)v; }
    uint64_t step = ((uint64_t)ir << 32) / orate, pos = (uint64_t)(H - 1) << 32; int k = 0;
    for (; k < cap; k++, pos += step) {
        uint64_t i = pos >> 32;
        if (i + H >= (uint64_t)n + H - 1) break;
        int16_t f[2]; fmrs_frame(c, taps, buf + 2 * (i + 1 - H), (uint32_t)pos, f); out[k] = f[0] / 32768.0f;
    }
    free(c); free(buf); return k;
}
static int m_rsq(const float *in, int n, uint32_t ir, uint32_t orate, int taps, float *out, int cap)
{
    pc_rsq_t *q = calloc(1, pc_rsq_size()); pc_rsq_init(q, ir, orate);
    float *st = malloc((size_t)n * 8), *so = malloc((size_t)cap * 8);
    for (int i = 0; i < n; i++) st[2 * i] = st[2 * i + 1] = in[i];
    int used = 0, k = pc_rsq_process(q, st, n, so, cap, &used);
    for (int i = 0; i < k; i++) out[i] = so[2 * i];
    free(q); free(st); free(so); return k;
}
static double i0d(double x) { double t = 1, s = 1, q = x * x / 4; for (int k = 1; k < 60; k++) { t *= q / ((double)k * k); s += t; } return s; }
static int m_ref(const float *in, int n, uint32_t ir, uint32_t orate, int taps, float *out, int cap)   // 192 taps Kaiser(9.5), cutoff 0.46 * min(rate): double, exact positions
{
    int H = 96; double fc = 0.5 * (orate < ir ? (double)orate / ir : 1.0) * 0.92, beta = 9.5, ib = i0d(beta); int k = 0;
    for (; k < cap; k++) {
        double t = (double)k * ir / orate; long i = (long)floor(t); double fr = t - i;
        if (i + H >= n) break;
        double acc = 0;
        for (int j = -H + 1; j <= H; j++) {
            long idx = i + j; if (idx < 0 || idx >= n) continue;
            double d = j - fr, a = 2 * fc * d, sinc = fabs(a) < 1e-9 ? 1 : sin(M_PI * a) / (M_PI * a), u = d / H, w = fabs(u) < 1 ? i0d(beta * sqrt(1 - u * u)) / ib : 0;
            acc += in[idx] * 2 * fc * sinc * w;
        }
        out[k] = (float)acc;
    }
    return k;
}
typedef int (*meth_t)(const float *, int, uint32_t, uint32_t, int, float *, int);
static const struct { const char *name; meth_t f; int taps; } M[] = { {"fmvgm16", m_fmvgm, 16}, {"fmvgm24", m_fmvgm, 24}, {"fmvgm32", m_fmvgm, 32}, {"fmvgm48", m_fmvgm, 48}, {"pc_rsq68", m_rsq, 68}, {"ref192", m_ref, 192} };
#define NM (int)(sizeof M / sizeof M[0])

static void sweep(uint32_t ir, uint32_t orate)
{
    static const double fl[] = { 1000, 5000, 10000, 15000, 18000, 20000, 21000, 22000, 23000, 24500, 25500, 26500, 27500 };
    int n = (int)((N + 4000) * (double)ir / orate) + 400; if (n < N + 6000) n = N + 6000;
    float *in = malloc(n * 4), *out = malloc((size_t)(n + 100) * 4), seg[N];
    printf("sine %.1f dBFS (peak 0.9) at %u Hz -> %u Hz.  For f below the output Nyquist: gain at f and the worst spur (dBc); above it: the alias product (dB re the input tone)\n", db(0.81), ir, orate);
    printf("%8s", "f Hz"); for (int m = 0; m < NM; m++) printf(" | %-16s", M[m].name); printf("\n");
    for (unsigned q = 0; q < sizeof fl / sizeof fl[0]; q++) {
        double f = fl[q]; if (f >= ir / 2.0) continue;
        for (int i = 0; i < n; i++) in[i] = (float)(0.9 * sin(2 * M_PI * f * i / ir));
        printf("%8.0f", f);
        for (int m = 0; m < NM; m++) {
            int k = M[m].f(in, n, ir, orate, M[m].taps, out, n + 100);
            if (k < N + 2000) { printf(" | %-16s", "short"); continue; }
            memcpy(seg, out + 1500, sizeof seg);
            static double pw[N / 2 + 1]; spectrum(seg, pw);
            double fo = f < orate / 2.0 ? f : orate - f;                                    // where the tone (or its alias) lands
            int kb = (int)(fo / orate * N + 0.5);
            double tone = 0; for (int k2 = kb - 5; k2 <= kb + 5; k2++) if (k2 >= 0 && k2 <= N / 2) tone += pw[k2];
            double worst = 0; for (int k2 = 1; k2 <= N / 2; k2++) if (abs(k2 - kb) > 8 && pw[k2] > worst) worst = pw[k2];
            double ref_in = 0.81 / 2;                                                       // power of the input tone in the same units (A^2 / 2)
            if (f < orate / 2.0 * 0.99) printf(" | %+6.2f dB %7.1f dBc", db(tone) - db(ref_in), db(worst) - db(tone));
            else printf(" | alias %+7.1f dB      ", db(tone) - db(ref_in));
        }
        printf("\n");
    }
    free(in); free(out);
}

static void file_test(const char *path, uint32_t ir, uint32_t orate, int ch)
{
    FILE *f = fopen(path, "rb"); if (!f) { perror(path); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    int n = (int)(sz / 2 / ch); int16_t *raw = malloc(sz); if (fread(raw, 1, sz, f) != (size_t)sz) return; fclose(f);
    if (n > ir * 20) n = ir * 20;
    float *in = malloc(n * 4); for (int i = 0; i < n; i++) in[i] = raw[i * ch] / 32768.0f;
    printf("%s: %d samples at %u Hz -> %u Hz; power in dB (relative to the reference's 0-16 kHz band):\n", path, n, ir, orate);
    printf("%-10s %9s %9s %9s %9s   %s\n", "method", "0-16k", "16-20k", "20-22.05k", "22.05-24k", "error vs ref, 0-20k (dB re ref 0-20k)");
    float *outs[NM]; int ks[NM];
    for (int m = 0; m < NM; m++) { outs[m] = malloc((size_t)(n + 100) * 4); ks[m] = M[m].f(in, n, ir, orate, M[m].taps, outs[m], n + 100); }
    int r = NM - 1; double p0[4];
    for (int m = 0; m < NM; m++) {
        double tot[4] = {0, 0, 0, 0}; int blocks = 0;
        for (int off = 3000; off + N < ks[m] && blocks < 6; off += N, blocks++) {
            static double pw[N / 2 + 1]; spectrum(outs[m] + off, pw);
            tot[0] += band(pw, 20, 16000, orate); tot[1] += band(pw, 16000, 20000, orate); tot[2] += band(pw, 20000, 22050, orate); tot[3] += band(pw, 22050, 24000, orate);
        }
        if (m == 0) {} if (m == r) memcpy(p0, tot, sizeof tot);
        for (int b = 0; b < 4; b++) tot[b] = tot[b];
        // error: difference signal against the reference (the same length, the same time axis), band-limited to 0-20 kHz
        double ep = 0, rp = 0; int kk = ks[m] < ks[r] ? ks[m] : ks[r]; static float dif[N];
        for (int off = 3000, b = 0; off + N < kk && b < 6; off += N, b++) {
            for (int i = 0; i < N; i++) dif[i] = outs[m][off + i] - outs[r][off + i];
            static double pw[N / 2 + 1], pr[N / 2 + 1]; spectrum(dif, pw); spectrum(outs[r] + off, pr);
            ep += band(pw, 20, 20000, orate); rp += band(pr, 20, 20000, orate);
        }
        printf("%-10s %9.1f %9.1f %9.1f %9.1f   %9.1f\n", M[m].name, db(tot[0] / (blocks ? blocks : 1)), db(tot[1] / (blocks ? blocks : 1)), db(tot[2] / (blocks ? blocks : 1)), db(tot[3] / (blocks ? blocks : 1)), db(ep) - db(rp));
    }
    (void)p0;
}

int main(int argc, char **argv)
{
    if (argc >= 4 && !strcmp(argv[1], "sweep")) { sweep((uint32_t)atoi(argv[2]), (uint32_t)atoi(argv[3])); return 0; }
    if (argc >= 5 && !strcmp(argv[1], "file")) { file_test(argv[2], (uint32_t)atoi(argv[3]), (uint32_t)atoi(argv[4]), argc > 5 ? atoi(argv[5]) : 2); return 0; }
    fprintf(stderr, "usage: alias_test sweep IN OUT | file IN.i16 IN OUT [channels]\n"); return 2;
}
