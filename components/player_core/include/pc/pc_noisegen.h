// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Coloured noise, ported from RP2350_player (JukeChip) src/player/player_backend_noise.c: fixed-point generators
 * calibrated at 44.1 kHz to ~15 % of full scale RMS; L and R run independent copies (decorrelated stereo). */
#define PC_NOISE_COLORS 8
#define PC_NOISE_RATE 44100
typedef enum { PC_NOISE_WHITE, PC_NOISE_PINK, PC_NOISE_BROWN, PC_NOISE_BLUE, PC_NOISE_VIOLET, PC_NOISE_GREY,
               PC_NOISE_GREEN, PC_NOISE_VELVET } pc_noise_color_t;
typedef struct {
    uint32_t rng;
    int32_t pink_rows[16], pink_sum;
    uint32_t pink_counter;
    int32_t brown_y, prev_raw, svf_low, svf_band, velvet_gap, dc_prev_x, dc_prev_y;
} pc_noise_ch_t;
typedef struct { int color; pc_noise_ch_t ch[2]; } pc_noisegen_t;
void pc_noisegen_start(pc_noisegen_t *g, int color);                /* resets: the same colour replays the same samples */
void pc_noisegen_render(pc_noisegen_t *g, float *stereo, int frames);   /* interleaved, [-1, 1] */
const char *pc_noise_name(int color, int zh);                       /* "Pink Noise" / "粉紅噪音" */
#ifdef __cplusplus
}
#endif
