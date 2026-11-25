// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* One shared voice snapshot for the chip-like backends added after GME and libxmp (HVL/AHX, PICO-8, YM). Same rules as the
 * snapshot of the xmp backend: PSRAM, written by the audio task once per block, read by the UI task; `n` is written last when
 * a source takes it over and first (0) when it lets go. A pre-opened next handle leaves it alone: the first block a handle
 * renders takes it over (pc_extvoice_take), closing a handle clears it only if that handle owns it. */
#include "pc/pc_voice.h"

#ifdef __cplusplus
extern "C" {
#endif
/* audio task: `owner` takes the snapshot when it is free: n voices, names "1".."n", everything else cleared, pc_vmute_reset(). Returns the snapshot to fill, or NULL while another handle
 * holds it (a crossfade renders the next handle beside the playing one; it asks again each block until the old one is closed and has released it). */
pc_voices_t *pc_extvoice_take(const void *owner, int n);
pc_voices_t *pc_extvoice_get(const void *owner);          /* the snapshot when `owner` holds it, else NULL */
void pc_extvoice_release(const void *owner);              /* clears it (n = 0) when `owner` holds it */
int pc_extvoice_read(pc_voices_t *out);                   /* UI task: copies the snapshot; returns n, 0 = none */
#ifdef __cplusplus
}
#endif
