// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_ring.h"
#include <string.h>

void pc_ring_init(pc_ring_t *g, void *mem, uint32_t cap) { g->buf = mem; g->cap = cap; g->w = g->r = 0; }

uint32_t pc_ring_used(const pc_ring_t *g)
{
    return __atomic_load_n(&g->w, __ATOMIC_ACQUIRE) - __atomic_load_n(&g->r, __ATOMIC_ACQUIRE);
}
uint32_t pc_ring_free(const pc_ring_t *g) { return g->cap - pc_ring_used(g); }

uint32_t pc_ring_write(pc_ring_t *g, const void *src, uint32_t n)
{
    uint32_t f = pc_ring_free(g);
    if (n > f) n = f;
    uint32_t w = g->w, at = w % g->cap, first = g->cap - at < n ? g->cap - at : n;
    memcpy(g->buf + at, src, first);
    memcpy(g->buf, (const uint8_t *)src + first, n - first);
    __atomic_store_n(&g->w, w + n, __ATOMIC_RELEASE);
    return n;
}

uint32_t pc_ring_read(pc_ring_t *g, void *dst, uint32_t n)
{
    uint32_t u = pc_ring_used(g);
    if (n > u) n = u;
    uint32_t r = g->r, at = r % g->cap, first = g->cap - at < n ? g->cap - at : n;
    memcpy(dst, g->buf + at, first);
    memcpy((uint8_t *)dst + first, g->buf, n - first);
    __atomic_store_n(&g->r, r + n, __ATOMIC_RELEASE);
    return n;
}

uint32_t pc_ring_peek(const pc_ring_t *g, void *dst, uint32_t n)
{
    uint32_t u = pc_ring_used(g);
    if (n > u) n = u;
    uint32_t at = g->r % g->cap, first = g->cap - at < n ? g->cap - at : n;
    memcpy(dst, g->buf + at, first);
    memcpy((uint8_t *)dst + first, g->buf, n - first);
    return n;
}
