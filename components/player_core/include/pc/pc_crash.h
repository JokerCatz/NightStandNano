// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Crash-log line format and file tail trimming (host-testable; no I/O here). */
#define PC_CRASH_MAGIC 0xC4A5E001u
enum { PC_CRASH_MODE_UNKNOWN, PC_CRASH_MODE_NORMAL, PC_CRASH_MODE_INST, PC_CRASH_MODE_USB, PC_CRASH_MODE_CAST };
typedef struct {
    uint32_t magic, wall /* UTC seconds since 1970, 0 = clock not set */, uptime_s, internal_min, pc, ra, cause;
    uint8_t mode, bt_mode, src, has_pc;
    char screen[16];
} pc_crash_t;
/* One line ending in '\n'; returns its length, or 0 if it does not fit in cap (out[0] = 0).
 * screen keeps printable ASCII only and is safe without a terminating 0. */
int pc_crash_format(const pc_crash_t *c, int reset_reason, const char *fw, char *out, size_t cap);
/* Start of the first complete line inside the last keep bytes of buf[0..n); with no complete line, n - min(n, keep). */
size_t pc_crash_tail(const char *buf, size_t n, size_t keep);
#ifdef __cplusplus
}
#endif
