// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_crash.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Copy the visible ASCII bytes (0x21..0x7e) of src, stopping at a 0 or after max_src bytes, keeping at most cap-1. */
static size_t clean_token(char *dst, size_t cap, const char *src, size_t max_src)
{
    size_t k = 0;
    for (size_t i = 0; i < max_src && src[i] && k + 1 < cap; i++) {
        unsigned char ch = (unsigned char)src[i];
        if (ch > 0x20 && ch < 0x7f) dst[k++] = (char)ch;
    }
    dst[k] = '\0';
    return k;
}

int pc_crash_format(const pc_crash_t *c, int reset_reason, const char *fw, char *out, size_t cap)
{
    static const char *const modes[] = { "unknown", "normal", "inst", "usb", "cast" };
    char when[24] = "-", scr[17], fwc[17];
    if (!out || cap == 0) return 0;
    out[0] = '\0';
    if (!c) return 0;
    if (c->wall) {
        time_t t = (time_t)c->wall;
        struct tm tm;
        if (!gmtime_r(&t, &tm) || strftime(when, sizeof when, "%Y-%m-%dT%H:%M:%SZ", &tm) == 0) strcpy(when, "-");
    }
    size_t k = clean_token(scr, sizeof scr, c->screen, sizeof c->screen);
    size_t kf = fw ? clean_token(fwc, sizeof fwc, fw, SIZE_MAX) : 0;
    const char *mode = modes[c->mode <= PC_CRASH_MODE_CAST ? c->mode : PC_CRASH_MODE_UNKNOWN];
    char tail[64];
    if (c->has_pc)
        snprintf(tail, sizeof tail, "pc=0x%08" PRIx32 " ra=0x%08" PRIx32 " cause=0x%" PRIx32, c->pc, c->ra, c->cause);
    else
        strcpy(tail, "pc=- ra=- cause=-");
    int n = snprintf(out, cap, "%s fw=%s reason=%d mode=%s up=%" PRIu32 " screen=%s bt=%u src=%u imin=%" PRIu32 " %s\n",
                     when, kf ? fwc : "-", reset_reason, mode, c->uptime_s, k ? scr : "-",
                     (unsigned)c->bt_mode, (unsigned)c->src, c->internal_min, tail);
    if (n < 0 || (size_t)n >= cap) { out[0] = '\0'; return 0; }
    return n;
}

size_t pc_crash_tail(const char *buf, size_t n, size_t keep)
{
    if (n == 0 || keep >= n) return 0;
    size_t start = n - keep;
    if (buf[start - 1] == '\n') return start;
    for (size_t i = start; i < n; i++)
        if (buf[i] == '\n') return i + 1 < n ? i + 1 : start;
    return start;
}
