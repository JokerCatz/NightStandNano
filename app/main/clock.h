// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Wall clock and time zone. The box has no RTC: the time comes from SNTP (Wi-Fi) or from the web page (`time set`), and is lost at reboot.
 * The zone is a POSIX TZ string kept in NVS (key "tz"), default CST-8 (Taiwan). */
#include <stdbool.h>
#include <stddef.h>
#include <time.h>
typedef enum { NSN_CLOCK_NONE, NSN_CLOCK_CLIENT, NSN_CLOCK_NTP } nsn_clock_src_t;
void nsn_clock_boot(void);                      /* load the stored zone (call once, after nsn_settings_load) */
bool nsn_clock_valid(void);                     /* the clock was set by NTP or by a client: local time can be trusted */
nsn_clock_src_t nsn_clock_src(void);
void nsn_clock_ntp_synced(void);                /* called by the SNTP callback: from now on the clock is NTP's */
void nsn_clock_iso(time_t t, char *out, size_t n);    /* local time with offset, e.g. 2026-11-25T14:03:20+08:00 */
void nsn_clock_status(char *out, size_t n);     /* the `time` reply */
/* `time set` / `time tz`: return a reply line (static or in `out`) */
const char *nsn_clock_cmd_set(const char *args, char *out, size_t n);
const char *nsn_clock_cmd_tz(const char *tz, char *out, size_t n);
