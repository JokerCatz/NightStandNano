// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
/* Crash breadcrumbs in RTC memory + a log line on the card after a panic / watchdog reset (round's pc_crash line format).
 * Boot-loop guard: three crashes in a row, each within 60 s of boot, start the box in safe mode (no autoplay, no fx). */
void nsn_crash_boot(void);                    /* first thing in app_main: read the last run's breadcrumbs, count crash streak */
void nsn_crash_crumb(const char *what);       /* what is happening now (track / state); also stamps uptime and memory low-water mark */
void nsn_crash_flush(void);                   /* once the card is mounted: write the pending line to /sdcard/_player/logs/crash.txt */
bool nsn_crash_safe_mode(void);               /* a boot loop was detected: skip the risky parts */
