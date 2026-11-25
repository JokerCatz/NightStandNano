// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Integer polyphase resampler for the chip rates (internal to fmvgm). A windowed-sinc (Kaiser) table of `taps` coefficients for each of FMRS_PHASES + 1 phases, Q15, int16, read
 * at the two phases around the exact position and interpolated linearly (a nearest-phase table of the same size leaves timing spurs near -62 dBc at 15 kHz, measured); the
 * signal is int16 stereo. Everything the output depends on is integer (the table is built with float math that has no libm call and no fused multiply-add: this file is
 * built with -ffp-contract=off), so a PC build and the firmware produce the same bytes.
 *   out[k] = sum_j  c[phase][j] * in[i - H + 1 + j]   (H = taps / 2),  position of out[k] = k * in_rate / out_rate (Q32) in input frames. */
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define FMRS_PBITS 8                              /* 256 phases: a timing error of at most 1/512 input frame */
#define FMRS_PHASES (1 << FMRS_PBITS)
#define FMRS_MAX_TAPS 48

/* Builds the table ((FMRS_PHASES + 1) x taps int16, DC gain exactly 1 << 15 per row). fc = cutoff in cycles per INPUT frame (0.5 = the input Nyquist); beta = Kaiser beta. */
void fmrs_build(int16_t *coef, int taps, float fc, float beta);
/* The cutoff / beta this module picks for in_rate -> out_rate: fc = min(0.5, 0.5 * out / in) * edge. */
float fmrs_cutoff(uint32_t in_rate, uint32_t out_rate);
float fmrs_beta(int taps);
/* One stereo output frame: `in` points at the first of `taps` interleaved stereo input frames (index i - H + 1); the position inside input frame i is frac / 2^32 (frac = the low 32 bits of the Q32 position). */
void fmrs_frame(const int16_t *coef, int taps, const int16_t *in, uint32_t frac, int16_t *out_lr);

#ifdef __cplusplus
}
#endif
