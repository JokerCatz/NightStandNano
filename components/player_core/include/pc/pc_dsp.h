// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* The biquad the fx apps (pc_fxapp_*.c) are built from; the chain itself is pc_fxchain.h. */
typedef struct { float b0, b1, b2, a1, a2; float s1[2], s2[2]; } pc_biquad_t;   /* transposed direct form II */
void  pc_biquad_design(pc_biquad_t *bq, char type, float fc, float gain_db, float q, uint32_t rate);  /* RBJ: 'P' 'L' 'H', 'l' low-pass, 'h' high-pass */
void  pc_biquad_run(pc_biquad_t *bq, float *stereo, int frames);
float pc_biquad_gain_db(const pc_biquad_t *bq, float f, uint32_t rate);
#ifdef __cplusplus
}
#endif
