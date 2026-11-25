// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "pc/pc_rate.h"
#include "pc/pc_rsq.h"
#ifdef __cplusplus
extern "C" {
#endif

/* The audio task's hi-res decisions (hires T5b), pure: no RTOS, no BSP, no allocation, no state. audio_svc.c calls them where it decides
 * and the simulator's fake shares the plan arguments, so a skipped halving, a missing re-plan or a block written at the wrong rate fails a host test. */

/* out_max and the reason that set it, for the output of the moment. cap = the user's ceiling (PC_AUDIO_MAX_RATE until T6), hw_cap = this boot's
 * clock ceiling (lowered by a refused clock). A wired pipeline above 48k needs a clock row (pc_i2s_clk_for): any other rate there (64 kHz, say)
 * is planned as for a crystal-only DAC (48k, why clock). raw and burst are the source's (the no-row rule looks at the plan they give). */
typedef struct { uint32_t out_max; uint8_t limit_why; } pc_plan_args_t;
pc_plan_args_t pc_plan_args(uint32_t raw, uint32_t burst, bool sink, uint32_t cap, uint32_t hw_cap);
/* The wired DAC rates a board takes (pc_dac_caps_t) and what is done with a rate outside them: pc/pc_lowrate.h. */

/* The clock ceiling after the user changed the wired ceiling (dac_max_rate, hires T6 fix 1): raising it re-arms the DAC (PC_AUDIO_MAX_RATE: tried once more
 * at the new rate; a repeated clock failure lowers it again with the usual line); lowering it or no change leaves hw_cap alone. */
uint32_t pc_hw_cap_after_cap_change(uint32_t old_cap, uint32_t new_cap, uint32_t hw_cap);

/* The ceiling after a refused clock at rate: the tier below it (96000 -> 48000, 192000 -> 96000, ...), never below 48000. */
uint32_t pc_hires_tier_down(uint32_t rate);

/* Error codes as the caller's platform numbers them (ESP-IDF's values; audio_svc.c asserts that they match esp_err.h). */
#define PC_ERR_NO_MEM           0x101
#define PC_ERR_INVALID_ARG      0x102
#define PC_ERR_NOT_SUPPORTED    0x106
#define PC_ERR_INVALID_RESPONSE 0x108
/* Only a clock-type failure lowers the ceiling: no clock (NOT_SUPPORTED), a set-up the driver refuses (INVALID_ARG), a wrong read-back
 * (the BSP's INVALID_RESPONSE, or readback_mismatch: either one is enough, so a caller that forgets the flag cannot keep a bad clock). Anything else (NO_MEM for the DMA, a disable / enable error) is retried at the same rate. */
bool pc_hires_failure_lowers_cap(int err, bool readback_mismatch);

/* How the next source follows the current one (a = current, b = next, each its raw rate and its plan; a's pipe = the rate the channel runs):
 * GAPLESS: same raw rate and same plan (pipe, halvings), it carries on in the same block; SEAMLESS: the pipeline rate stays (no clock change:
 * no mute, no ramp); RATE_CHANGE: the clock changes (mute, switch, ramp in). */
typedef enum { PC_JOIN_GAPLESS, PC_JOIN_SEAMLESS, PC_JOIN_RATE_CHANGE } pc_join_t;
pc_join_t pc_join_kind(uint32_t raw_a, pc_pipe_t plan_a, uint32_t raw_b, pc_pipe_t plan_b);
const char *pc_join_name(pc_join_t k);           /* "gapless" "seamless" "rate change"; "?" for anything else */

/* hires T10b fix 1, the look-ahead of the Bluetooth resampler: the rate a source of raw rate `raw` (and its burst) is planned at for a sink
 * (pc_plan_args(raw, burst, sink, ...) then pc_pipe_plan, as audio_svc.c plans it once the sink is on), i.e. the input rate of the sink's resampler;
 * 0 when raw is 0 or the plan refuses it. The builder pairs it with the link rate for the queued next track (audio_svc_info().next_rate). */
uint32_t pc_sink_pipe_rate(uint32_t raw, uint32_t burst);
/* fix 2 (review m4): the builder's two hints from audio_svc_info, one rule for bt_svc.c and the simulator's model. on = a sink is on (no hints
 * otherwise); now = (pipe_rate, link) while a source is loaded and its pipeline is a rate a sink takes as is (<= 48k); next = the queued next
 * track's sink rate (pc_sink_pipe_rate) with the link; {0, 0} where there is none. */
void pc_rsq_hints(bool on, uint32_t src_rate, uint32_t pipe_rate, uint32_t next_rate, uint32_t link, pc_rsq_pair_t *now, pc_rsq_pair_t *next);

/* Where one block goes (spec 4.5, never at the wrong rate): a sink takes it at its own rate; the wired DAC only when its channel is up and the
 * block was made at the rate the channel runs (brate == s_rate); otherwise it is dropped and the task waits (10 ms). */
typedef enum { PC_BLOCK_SINK, PC_BLOCK_WRITE, PC_BLOCK_DROP_WAIT } pc_block_out_t;
pc_block_out_t pc_block_route(bool sink, bool channel_up, uint32_t brate, uint32_t s_rate);

#ifdef __cplusplus
}
#endif
