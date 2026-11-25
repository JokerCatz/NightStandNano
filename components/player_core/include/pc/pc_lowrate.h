// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "pc/pc_pipe_rules.h"
#include "pc/pc_rsq.h"
#ifdef __cplusplus
extern "C" {
#endif

/* LOWRATE (owner 2026-11-16), one rule for both boards: each board declares the rates its wired DAC takes (pc_dac_caps_t, a const table), and a source
 * whose plan lands on a rate outside it (22.05 kHz on the CS43131, an odd 99.999 kHz anywhere) is resampled with pc_rsq (rational, up and down) to the best
 * supported rate by one stage between the source and the pipeline (the place the halving stage has): everything after it (post hook, taps, toy sounds,
 * the gain stage, pc_rv_block, the I2S write) sees an ordinary block at the DAC rate. The exact power-of-two halving (pc_pipe_plan) stays first: it is cheaper
 * and already reaches a supported rate (384 -> 192, 352.8 -> 176.4, 64 -> 32 kHz). */
/* The stage makes at most 96 kHz (960 frames per 10 ms block, about 35 % of one core on the S31 by the measured 3.6 us per output frame): a 384 -> 192 resample would take about 70 %, so a
 * source above 192 kHz that the halving cannot bring down (an odd one) is refused with its reason; the exact halving covers 384 / 352.8 / 192 / 176.4. */
#define PC_UP_MAX_OUT 96000
typedef struct {
    uint32_t any_lo, any_hi;     /* every rate in this range is taken (the round board's I2S: 1 .. 48000); 0, 0: none */
    const uint32_t *list;        /* and these rates */
    uint8_t n;
    uint32_t max;                /* the clock ceiling of the DAC path (a plan never goes above it) */
    uint32_t native_from;        /* DACFS (FakePod, 0 = off): a source at or above this rate is never resampled (the DAC is reprogrammed to it; no code = refused) */
    uint32_t resample_floor;     /* with native_from: a source below native_from but at least this (16 kHz) is upsampled by the stage to the same family; below it: refused */
} pc_dac_caps_t;
extern const pc_dac_caps_t pc_dac_caps_round;     /* JC3636F118: <= 48k any (crystal), 88.2/96 (crystal x128), 176.4/192/352.8/384 (APLL) = pc_i2s_clk_for */
extern const pc_dac_caps_t pc_dac_caps_fakepod;   /* CS43131 ASP slave, DACFS: 32 .. 384 kHz (pc_cs_rate_code, DS 7.1.7 p.114), native only (below 32 kHz the part has no code: refused, never resampled) */

bool pc_dac_has(const pc_dac_caps_t *c, uint32_t rate);
/* The rate a source at src is resampled to: the standard rates the DAC takes, at most out_max and within 1/4 .. 4 of src (the resampler's ratio range, the 96k output cap).
 * The highest of them at or below the source (and at most out_max): 99,999 Hz -> 96k, 50,000 Hz -> 48k at every ceiling; none at or below (22.05 kHz on the CS43131):
 * the lowest above it. The same family (44.1 kHz multiples / 4 kHz multiples) first, the other family when the family has none. 0: nothing fits (refused). */
uint32_t pc_dac_pick_rate(const pc_dac_caps_t *c, uint32_t src, uint32_t out_max);
/* The ceiling the rate pick works within: the user's ceiling and this boot's clock, at most the DAC's own (a sink: 48k). It is NOT pc_plan_args' out_max: that one drops to
 * 48k for a pipe without a clock row, which would pull an odd 99,999 Hz source down to 48k at every ceiling above 96k. */
uint32_t pc_dac_fix_max(const pc_dac_caps_t *c, bool sink, uint32_t cap, uint32_t hw_cap);
/* The plan arguments' ceiling never above the DAC's own */
static inline void pc_dac_clamp_args(const pc_dac_caps_t *c, pc_plan_args_t *a) { if (a->out_max > c->max) a->out_max = c->max; }
/* The halving plan p for raw, made supported: a rate the DAC takes stays (*from = 0); otherwise the resample target (*from = raw, halvings 0), or p as it is
 * when nothing fits (pipe 0 = refused by the caller; a nonzero unsupported pipe = pc_dac_unsupported, refused with its reason). A sink takes any rate: p as it is. */
pc_pipe_t pc_dac_fix(const pc_dac_caps_t *c, uint32_t raw, uint32_t out_max, bool sink, pc_pipe_t p, uint32_t *from);
/* DACFS (owner 2026-11-17): the FakePod's hard limit (dac_hard_rate): a wired source above it is refused before it is opened (a sink and the round board: never). */
static inline bool pc_dac_over_hard(const pc_dac_caps_t *c, bool sink, uint32_t raw, uint32_t hard) { return c->native_from && !sink && hard && raw > hard; }
static inline bool pc_dac_unsupported(const pc_dac_caps_t *c, uint32_t pipe, bool sink) { return pipe && !sink && !pc_dac_has(c, pipe); }

/* Source frames the stage keeps buffered (carry + new) for one block of `frames` output frames at in -> out: the most one block can consume, plus two. */
int pc_up_need(int frames, uint32_t in, uint32_t out);
/* The stage's source buffer: one 10 ms block at the highest source rate plus 8 frames (the ceil, the 2 spare frames of pc_up_need and the carry: a 383,999 Hz header needs 3842 + carry). */
#define PC_UP_SBUF_FRAMES (PC_SRC_MAX_RATE / 100 + 8)
/* Source frames to read now: what tops carry up to pc_up_need, never beyond the buffer (carry + result <= PC_UP_SBUF_FRAMES, always). */
int pc_up_want(int carry, int frames, uint32_t in, uint32_t out);
/* One block. sb holds `carry` frames left from the last block followed by `ns` new source frames (interleaved stereo float, room for carry + ns);
 * writes exactly `frames` output frames to out when carry + ns >= pc_up_need (fewer only when the source ended), moves what the resampler did not
 * take to the front of sb and returns it in *new_carry. Returns the output frames. */
int pc_up_run(pc_rsq_t *q, float *sb, int carry, int ns, float *out, int frames, int *new_carry);

#ifdef __cplusplus
}
#endif
