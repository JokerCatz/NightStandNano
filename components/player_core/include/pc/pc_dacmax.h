// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* dac_max_rate (hires T6, H17): the highest rate the wired DAC is asked to run at. Pure: no RTOS, no BSP, no state. The tiers are
 * {48000, 96000, 192000, 384000} up to PC_AUDIO_MAX_RATE (four in this build); a stored value is normalised, never trusted. */
int pc_dacmax_count(void);                               /* how many tiers this build offers (>= 3: the settings row takes the knob, rule R21) */
/* stored <= 0 (missing, 0, negative) -> def normalised; else the largest offered tier <= stored, at least 48000. def is normalised the same way
 * (a default above the build, or garbage, gives the largest tier it reaches); the result is always an offered tier. */
uint32_t pc_dacmax_norm(int stored, uint32_t def);
uint32_t pc_dacmax_step(uint32_t v, int detents);        /* v normalised, then moved by detents over the offered tiers, wrapping */
const char *pc_dacmax_label(uint32_t v);                 /* "48k" "96k" "192k" "384k"; "?" for anything else */

#ifdef __cplusplus
}
#endif
