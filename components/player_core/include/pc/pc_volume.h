// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/* Software volume 0..100, 2 steps per knob detent, -60 dB..0 dB curve (spec 5.5). */
#define PC_VOL_DEFAULT 40
#define PC_VOL_PER_DETENT 2

int   pc_volume_step(int vol, int detents, int max);
float pc_volume_gain(int vol);   /* 0 -> silence */

#ifdef __cplusplus
}
#endif
