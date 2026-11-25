// SPDX-License-Identifier: LGPL-2.1-or-later
#include <string.h>
#include "pc/pc_lowrate.h"

static const uint32_t ROUND_LIST[] = { 88200, 96000, 176400, 192000, 352800, 384000 };
static const uint32_t FAKEPOD_LIST[] = { 32000, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000 };
const pc_dac_caps_t pc_dac_caps_round = { 1, 48000, ROUND_LIST, sizeof ROUND_LIST / sizeof *ROUND_LIST, 384000, 0, 0 };
const pc_dac_caps_t pc_dac_caps_fakepod = { 0, 0, FAKEPOD_LIST, sizeof FAKEPOD_LIST / sizeof *FAKEPOD_LIST, 384000, 32000, 16000 };   /* DACFS: >= 32 kHz native by reprogramming the DAC, 16 .. 32 kHz upsampled by the stage (owner 2026-11-17), < 16 kHz refused */

bool pc_dac_has(const pc_dac_caps_t *c, uint32_t rate)
{
    if (!rate || rate > c->max) return false;
    if (rate >= c->any_lo && rate <= c->any_hi && c->any_hi) return true;
    for (int i = 0; i < c->n; i++) if (c->list[i] == rate) return true;
    return false;
}

static const uint32_t STD[] = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000 };
static int fam(uint32_t r) { return r % 11025 == 0 ? 44 : r % 4000 == 0 ? 48 : 0; }
static bool ratio_ok(uint32_t src, uint32_t r) { return r <= PC_UP_MAX_OUT && (uint64_t)r * 4 >= src && (uint64_t)src * 4 >= r; }   /* the resampler's 1/4 .. 4 and its CPU budget */

uint32_t pc_dac_pick_rate(const pc_dac_caps_t *c, uint32_t src, uint32_t out_max)
{
    const int N = (int)(sizeof STD / sizeof *STD);
    if (!src) return 0;
    const int f = fam(src);
    if (c->native_from && src < c->native_from) {          /* FakePod, 16 .. 32 kHz: the nearest integer multiple the DAC takes (22.05 -> 44.1, 24 -> 48, 16 -> 32): the same family, an exact 1:n stage */
        for (int i = 0; i < N; i++) {
            uint32_t r = STD[i];
            if (r > src && r % src == 0 && r <= out_max && pc_dac_has(c, r) && ratio_ok(src, r)) return r;
        }
    }
    const uint32_t top = src < out_max ? src : out_max;   /* the target never above the source (a source above the ceiling: the ceiling) */
    for (int pass = 0; pass < 2; pass++) {              /* pass 0: the same family, pass 1: any */
        for (int i = N - 1; i >= 0; i--) {              /* the highest rate at or below the source: nothing is made up above what the file holds */
            uint32_t r = STD[i];
            if (r > top || !pc_dac_has(c, r) || !ratio_ok(src, r) || (pass == 0 && (!f || fam(r) != f))) continue;
            return r;
        }
        for (int i = 0; i < N; i++) {                   /* none at or below: the lowest above (22.05 kHz -> 44.1 kHz on the CS43131) */
            uint32_t r = STD[i];
            if (r < src || r > out_max || !pc_dac_has(c, r) || !ratio_ok(src, r) || (pass == 0 && (!f || fam(r) != f))) continue;
            return r;
        }
    }
    return 0;
}

uint32_t pc_dac_fix_max(const pc_dac_caps_t *c, bool sink, uint32_t cap, uint32_t hw_cap)
{
    uint32_t m = pc_out_max_rate(sink ? PC_OUT_SINK : PC_OUT_WIRED, hw_cap < cap ? hw_cap : cap);
    return m > c->max ? c->max : m;
}

int pc_up_want(int carry, int frames, uint32_t in, uint32_t out)
{
    int want = pc_up_need(frames, in, out) - carry, room = PC_UP_SBUF_FRAMES - carry;
    if (want > room) want = room;
    return want < 0 ? 0 : want;
}

pc_pipe_t pc_dac_fix(const pc_dac_caps_t *c, uint32_t raw, uint32_t out_max, bool sink, pc_pipe_t p, uint32_t *from)
{
    if (from) *from = 0;
    if (sink || !raw || (c->native_from && (raw >= c->native_from || raw < c->resample_floor)) || (p.pipe && pc_dac_has(c, p.pipe))) return p;   /* FakePod: >= 32 kHz never gets a stage, < 16 kHz neither (both stay as they are: native, or refused with the reason) */
    uint32_t t = pc_dac_pick_rate(c, raw, out_max);
    if (!t) return p;
    if (from) *from = raw;
    return (pc_pipe_t){ t, 0, PC_PIPE_NATIVE };
}

int pc_up_need(int frames, uint32_t in, uint32_t out)
{
    return (int)(((uint64_t)frames * in + out - 1) / out) + 2;
}

int pc_up_run(pc_rsq_t *q, float *sb, int carry, int ns, float *out, int frames, int *new_carry)
{
    int total = carry + ns, used = 0;
    int n = pc_rsq_process(q, sb, total, out, frames, &used);
    int left = total - used;
    if (left > 0 && used > 0) memmove(sb, sb + 2 * used, (size_t)left * 2 * sizeof(float));
    *new_carry = left > 0 ? left : 0;
    return n;
}
