// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* ICY (Shoutcast/Icecast) metadata: every `metaint` audio bytes the server inserts a length byte (x16)
 * and that many bytes of "StreamTitle='...';". pc_icy_feed removes them in place. */
typedef struct {
    uint32_t metaint, left;        /* audio bytes until the next length byte; metaint 0 = no metadata */
    int meta_len, meta_got;        /* -1: expecting the length byte */
    char meta[4081];
    char title[160];
    bool title_new;
} pc_icy_t;
void   pc_icy_init(pc_icy_t *s, uint32_t metaint);
size_t pc_icy_feed(pc_icy_t *s, uint8_t *buf, size_t n);    /* audio bytes now at the front of buf */
bool   pc_icy_take_title(pc_icy_t *s, char *out, size_t cap);   /* true once per new title */
#ifdef __cplusplus
}
#endif
