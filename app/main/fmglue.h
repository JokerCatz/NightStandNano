// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* NightStand Nano's side of the FM backend (components/fmvgm): the platform port, the placement default, the budget table (which chip sets are too heavy for which output),
 * and the UART-only `dbg fm ...` switches. Nothing in components/fmvgm refers to this file. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "fmvgm.h"
#ifdef __cplusplus
extern "C" {
#endif
void nsn_fm_init(void);                                         /* boot (nsn_player_init_settings): install the port, set the defaults */
/* Static pre-check: is this file (chips and clocks from fmvgm_info / fmvgm_probe) known to be too heavy for the output `sink` (sink.h id) with the placement now in force?
 * 0 = go ahead; -1 = skip, `why` = "Bluetooth AAC" ("too heavy for <why>"). `codec` is the A2DP codec name ("AAC" "SBC" "LDAC" or ""). The table and the numbers behind it: fmglue.c. */
int  nsn_fm_check(const fmvgm_info_t *info, int sink, const char *codec, char *why, size_t n);
/* the rate render() should deliver for the output now: the output's link rate for a 48 kHz source (so the player does not resample), or 0 = leave the chip's own rate */
uint32_t nsn_fm_out_rate(int sink);
/* UART `dbg fm ...`: args is what follows "fm" (may be ""); every reply line goes to `out` */
void nsn_fm_dbg(const char *args, void (*out)(void *ctx, const char *line), void *ctx);
/* a track was ended or refused for being too heavy: counted here for `dbg fm stat` */
void nsn_fm_note_skip(bool dynamic);
#ifdef __cplusplus
}
#endif
