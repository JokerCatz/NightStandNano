// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "pc/pc_backend.h"
#ifdef __cplusplus
extern "C" {
#endif

/* The highest pipeline rate: the audio task's block buffers (after the halving stage) hold one 10 ms block at this rate, PC_AUDIO_MAX_BLOCK
 * frames, and the wired DAC runs at most at it (hires T5: 88.2/96 kHz native on the crystal; T12: 176.4/192 kHz on the APLL; T13: 352.8/384 kHz). Sources up to PC_SRC_MAX_RATE are accepted and
 * halved on the audio task (pc_halve) until they fit the output; a mixer is never halved, so it must itself be pc_rate_ok. */
#define PC_AUDIO_MAX_RATE 384000
#define PC_AUDIO_MAX_BLOCK (PC_AUDIO_MAX_RATE / 100)

/* hi-res plan (hires spec H17-H22) */
#ifndef PC_SRC_MAX_RATE
#define PC_SRC_MAX_RATE 384000         /* highest raw source rate accepted (halved down by pc_halve when the output cannot take it); #ifndef so a host test can override it */
#endif
#define PC_SINK_MAX_BLOCK 480          /* a Bluetooth sink never gets more than 10 ms at 48 kHz (out_max SINK = 48000); hp_sink clamps to it */
#define PC_BURST_MAX_192K 16384        /* H22: native at 176.4/192k only if burst <= this */
#define PC_BURST_MAX_384K 32768        /* H22: native at 352.8/384k only if burst <= this (32768 = ffmpeg-default FLAC frame: 85 ms, inside the 250 ms reservoir) */
typedef enum { PC_OUT_WIRED = 0, PC_OUT_SINK = 1 } pc_out_t;   /* plan 2 adds PC_OUT_LDAC96 */
uint32_t pc_out_max_rate(pc_out_t out, uint32_t wired_cap);     /* WIRED: min(wired_cap, PC_AUDIO_MAX_RATE); SINK: 48000 */
typedef enum { PC_PIPE_NATIVE, PC_PIPE_CAP, PC_PIPE_SINK, PC_PIPE_BURST, PC_PIPE_CLOCK } pc_pipe_why_t;
typedef struct { uint32_t pipe; uint8_t halvings; uint8_t why; } pc_pipe_t;   /* pipe 0 = refuse */
/* limit_why: what set out_max (PC_PIPE_CAP, PC_PIPE_SINK or PC_PIPE_CLOCK). why = the reason of the last halving. */
pc_pipe_t pc_pipe_plan(uint32_t src, uint32_t out_max, uint32_t burst, uint8_t limit_why);
typedef enum { PC_CLK_XTAL, PC_CLK_APLL } pc_clk_src_t;
typedef struct { uint32_t rate; uint8_t src; uint16_t mclk_mult; uint8_t desc; uint16_t frames; } pc_i2s_clk_t;
bool pc_i2s_clk_for(uint32_t rate, pc_i2s_clk_t *out);         /* false: no row (refused); desc x frames: the DMA shape of the tier (6 x 480 <= 48k, 8 x 480 at 88.2/96k, 8 x 511 APLL) */
bool pc_i2s_shape_follows(uint32_t desc, uint32_t frames);     /* a table row's shape: such a channel follows the table at a rate change (reshape); any other (the instrument's 3 x 64) keeps its own */
bool pc_i2s_rate_ok(uint32_t want, uint32_t bclk_hz, uint8_t slot_bits);   /* |bclk/(2*slot_bits) - want| <= want/1000 */
int  pc_tap_decim(uint32_t rate);                                /* 1, 2, 4, 8: frames averaged so the tap rate <= 48000 */

bool     pc_rate_ok(uint32_t rate);              /* 1 .. PC_AUDIO_MAX_RATE */
bool     pc_mixer_rate_ok(uint32_t rate);        /* a mixer's own rate (never halved): pc_rate_ok and a clock row (pc_i2s_clk_for) */
const char *pc_pipe_why_name(uint8_t why);       /* "native" "cap" "sink" "burst" "clock"; "?" for anything else */
int      pc_block_frames(uint32_t rate);         /* rate / 100, at least 1 and at most PC_AUDIO_MAX_BLOCK, whatever the rate says */
uint32_t pc_src_rate(const pc_backend_t *be, void *h, uint32_t *raw);   /* the opened source's raw rate (1 .. PC_SRC_MAX_RATE), or 0 = refuse it (no rate(), 0, or too high); rate() is called once, *raw (may be NULL) gets what it said */

#ifdef __cplusplus
}
#endif
