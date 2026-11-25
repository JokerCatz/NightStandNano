// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Debug output check (UART `dbg oc ...`): watches the PCM the sink pulls from the player and counts what a listener would hear as a break.
 * Meant for steady-tone test files (a sine): there the next sample is predictable, so a jump in phase, a dropout or an inserted silence stands out.
 *   J  jump: the sample differs from the sine prediction x[n] = c * x[n-1] - x[n-2] (c fitted on the fly) by more than max(24, 3 % of the recent peak)
 *   Z  zero run: at least ZMIN frames of exact digital silence in the middle of a signal (length reported when the signal returns)
 * Frames the sink asked for and the player could not give count as zeros too (and as `starved`). */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define NSN_OC_ZMIN 24                 /* frames of silence that count as a break (0.5 ms at 48 kHz; a sine has at most 1-2 exact zeros in a row) */
void nsn_oc_arm(bool on);              /* on: reset and count; off: stop counting */
bool nsn_oc_armed(void);
void nsn_oc_idle(void);                /* the stream is not running (stopped, paused): the next signal is a fresh start, not a jump */
void nsn_oc_feed(const int16_t *stereo, size_t frames);
void nsn_oc_starve(size_t frames);     /* frames the sink asked for and did not get */
void nsn_oc_join(uint32_t frame);      /* player: a track boundary at this output frame (for the report) */
void nsn_oc_report(void (*line)(void *ctx, const char *s), void *ctx);
