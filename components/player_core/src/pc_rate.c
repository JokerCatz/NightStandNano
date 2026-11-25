// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_rate.h"

bool pc_rate_ok(uint32_t rate) { return rate >= 1 && rate <= PC_AUDIO_MAX_RATE; }

int pc_block_frames(uint32_t rate)
{
    uint32_t n = rate / 100;
    return n < 1 ? 1 : n > PC_AUDIO_MAX_BLOCK ? PC_AUDIO_MAX_BLOCK : (int)n;
}

uint32_t pc_src_rate(const pc_backend_t *be, void *h, uint32_t *raw)
{
    uint32_t r = be && be->rate ? be->rate(h) : 0;
    if (raw) *raw = r;
    return r >= 1 && r <= PC_SRC_MAX_RATE ? r : 0;     /* the audio task plans the pipeline (pc_halve) */
}

uint32_t pc_out_max_rate(pc_out_t out, uint32_t wired_cap)
{
    if (out == PC_OUT_SINK) return 48000;
    return wired_cap < PC_AUDIO_MAX_RATE ? wired_cap : PC_AUDIO_MAX_RATE;
}

pc_pipe_t pc_pipe_plan(uint32_t src, uint32_t out_max, uint32_t burst, uint8_t limit_why)
{
    pc_pipe_t r = { 0, 0, PC_PIPE_NATIVE };
    if (src == 0 || src > PC_SRC_MAX_RATE) return r;
    uint32_t p = src;
    uint8_t n = 0, why = PC_PIPE_NATIVE;
    for (;;) {
        uint8_t w;
        if (p > out_max) w = limit_why;
        else if ((p > 96000 && p <= 192000 && burst > PC_BURST_MAX_192K) || (p > 192000 && burst > PC_BURST_MAX_384K)) w = PC_PIPE_BURST;
        else break;
        if ((p & 1) || n >= 3) return r;
        p /= 2;
        n++;
        why = w;
    }
    r.pipe = p;
    r.halvings = n;
    r.why = why;
    return r;
}

/* H19: BCK = 64 fs. <= 96k stays on the crystal; the APLL tiers need mclk 128 fs with 8 x 511-frame descriptors.
 * DMA shape (H22, hires T8b): <= 48k 6 x 480 (60 ms at 48k, today's); 88.2/96k 8 x 480 (40 ms at 96k: 6 x 480 was only 30 ms, Batch B M-B4),
 * +7,680 B internal only while the output runs in that tier. */
bool pc_i2s_clk_for(uint32_t rate, pc_i2s_clk_t *out)
{
    pc_i2s_clk_t c = { rate, PC_CLK_XTAL, 256, 6, 480 };
    if (rate == 0) return false;
    if (rate <= 48000) { /* today's setup */ }
    else if (rate == 88200 || rate == 96000) { c.mclk_mult = 128; c.desc = 8; }
    else if (rate == 176400 || rate == 192000 || rate == 352800 || rate == 384000) { c.src = PC_CLK_APLL; c.mclk_mult = 128; c.desc = 8; c.frames = 511; }
    else return false;
    if (out) *out = c;
    return true;
}

bool pc_i2s_shape_follows(uint32_t desc, uint32_t frames)
{
    return frames == 480 ? desc == 6 || desc == 8 : frames == 511 && desc == 8;
}

bool pc_i2s_rate_ok(uint32_t want, uint32_t bclk_hz, uint8_t slot_bits)
{
    if (!want || !bclk_hz || !slot_bits) return false;
    uint32_t got = bclk_hz / (2u * slot_bits);
    uint32_t d = got > want ? got - want : want - got;
    return d <= want / 1000;
}

int pc_tap_decim(uint32_t rate)
{
    int d = 1;
    while (d < 8 && rate / (uint32_t)d > 48000) d *= 2;
    return d;
}

bool pc_mixer_rate_ok(uint32_t rate) { return pc_rate_ok(rate) && pc_i2s_clk_for(rate, NULL); }

const char *pc_pipe_why_name(uint8_t why)
{
    static const char *const W[] = { "native", "cap", "sink", "burst", "clock" };
    return why < sizeof W / sizeof *W ? W[why] : "?";
}
