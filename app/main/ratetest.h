// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* DAC sample-rate test: for every sample rate the USB DAC advertises, set the clock and stream ~1.5 s of a quiet 1 kHz sine, and record whether the
 * clock accepted it, what it read back, how many ISO transfers completed or failed and the data rate the transfer completions add up to.
 * The verdicts are kept per DAC (USB VID:PID) in NVS and decide which rates the player may use (player.c: dac_rate_ok). */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cmd.h"

enum { NSN_RT_UNTESTED = 0, NSN_RT_OK = 1, NSN_RT_BAD = 2 };

void nsn_ratetest_attach(uint16_t vid, uint16_t pid);     /* a DAC was attached (uac task): load its stored verdicts into RAM */
int  nsn_ratetest_verdict(uint32_t rate);                  /* NSN_RT_* for the attached DAC; untested for rates the test does not know */
void nsn_ratetest_debug_set(uint32_t rate, int verdict);   /* debug (UART `dbg rt`): change a verdict in RAM only (a later test run stores it) */
bool nsn_ratetest_any(void);                               /* the attached DAC has been tested (at least one verdict) */
bool nsn_ratetest_running(void);                           /* the player must not start while it runs */

/* NULL: started (a worker with an internal stack does it, the reply comes at once); else the refusal reason */
const char *nsn_ratetest_start(void);
bool        nsn_ratetest_cancel(void);                     /* false: not running */
/* state, one line per advertised rate, the final max= line: format documented at the `ratetest` handler in cmd.c */
void        nsn_ratetest_report(nsn_out_t out, void *ctx);
