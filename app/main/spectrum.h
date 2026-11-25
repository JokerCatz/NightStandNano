// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define NSN_BANDS 32
/* What is being played (stereo S16) is pushed from the pull callbacks; a task turns the newest 1024 samples into NSN_BANDS log-spaced bands 0..255 at ~30 Hz. */
void nsn_spec_push(const int16_t *stereo, size_t frames);
void nsn_spec_set_rate(uint32_t hz);
void nsn_spec_start(void (*on_frame)(const uint8_t bands[NSN_BANDS]));    /* the callback runs in the spectrum task */
/* A second (third) consumer, e.g. a display: called in the spectrum task right after the first one, same ~30 Hz; keep it short and do not block. false = no free slot (2). */
bool nsn_spec_observe(void (*on_frame)(const uint8_t bands[NSN_BANDS]));
void nsn_spec_last(uint8_t out[NSN_BANDS]);
