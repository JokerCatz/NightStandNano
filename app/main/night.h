// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Night mode and bedtime scene.
 *  night on|off      loudness + compressor + limiter (small volumes stay balanced, sudden loud parts are tamed), LED capped dark
 *  night auto HH:MM HH:MM | night auto off   switch night mode on and off by itself between those times (needs NTP time)
 *  bedtime [minutes] night on, play noise/scene (the last one used, else brown noise) and set the sleep timer (default 30)
 *  wake              night off
 * The schedule, LED cap and last bedtime source live in their own NVS keys (the settings blob stays compatible). */
#include <stdbool.h>
void nsn_night_start(void);
void nsn_night_set(bool on);
bool nsn_night_on(void);
void nsn_night_auto(int from_min, int to_min);   /* -1 = off */
void nsn_night_status(char *out, int n);
void nsn_bedtime(int minutes);
