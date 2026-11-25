// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Byte ring for one writer task and one reader task (no locks: each side owns its counter).
 * Counters run free; cap must be a power of two for streams longer than 4 GB. */
typedef struct { uint8_t *buf; uint32_t cap; uint32_t w, r; } pc_ring_t;
void     pc_ring_init(pc_ring_t *g, void *mem, uint32_t cap);
uint32_t pc_ring_used(const pc_ring_t *g);
uint32_t pc_ring_free(const pc_ring_t *g);
uint32_t pc_ring_write(pc_ring_t *g, const void *src, uint32_t n);   /* bytes taken (<= free) */
uint32_t pc_ring_read(pc_ring_t *g, void *dst, uint32_t n);          /* bytes given (<= used) */
uint32_t pc_ring_peek(const pc_ring_t *g, void *dst, uint32_t n);    /* like read, but leaves them in */
#ifdef __cplusplus
}
#endif
