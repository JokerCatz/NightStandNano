// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_mp4.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MOOV_MAX (8u << 20)          /* a long album rip has a few hundred KB; refuse nonsense */

typedef struct { uint32_t vstart, src, len; bool mem; } seg_t;
struct pc_mp4_fs {
    pc_reader_t src;
    uint8_t *moov;
    seg_t seg[4];
    int nseg;
};

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static void put32(uint8_t *p, uint32_t v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

/* adds `delta` to every chunk offset in [lo, hi) of the stco / co64 boxes under `b` (n bytes of box bodies) */
static void patch(uint8_t *b, uint32_t n, uint32_t lo, uint32_t hi, uint32_t delta)
{
    for (uint32_t p = 0; p + 8 <= n;) {
        uint32_t sz = be32(b + p);
        if (sz < 8 || sz > n - p) return;
        uint8_t *t = b + p + 4, *body = b + p + 8;
        uint32_t bl = sz - 8;
        if (!memcmp(t, "trak", 4) || !memcmp(t, "mdia", 4) || !memcmp(t, "minf", 4) || !memcmp(t, "stbl", 4))
            patch(body, bl, lo, hi, delta);
        else if (!memcmp(t, "stco", 4) && bl >= 8) {
            uint32_t cnt = be32(body + 4);
            for (uint32_t i = 0; i < cnt && 8 + 4 * i + 4 <= bl; i++) {
                uint32_t o = be32(body + 8 + 4 * i);
                if (o >= lo && o < hi) put32(body + 8 + 4 * i, o + delta);
            }
        } else if (!memcmp(t, "co64", 4) && bl >= 8) {
            uint32_t cnt = be32(body + 4);
            for (uint32_t i = 0; i < cnt && 8 + 8 * i + 8 <= bl; i++) {
                uint8_t *e = body + 8 + 8 * i;
                uint32_t o = be32(e + 4);
                if (!be32(e) && o >= lo && o < hi) put32(e + 4, o + delta);   /* files here are < 4 GB */
            }
        }
        p += sz;
    }
}

static int vread(void *u, uint32_t off, void *buf, uint32_t n)
{
    pc_mp4_fs_t *s = u;
    uint32_t done = 0;
    for (int i = 0; i < s->nseg && done < n; i++) {
        const seg_t *g = &s->seg[i];
        uint32_t at = off + done;
        if (at < g->vstart || at >= g->vstart + g->len) continue;
        uint32_t k = g->vstart + g->len - at;
        if (k > n - done) k = n - done;
        if (g->mem) memcpy((uint8_t *)buf + done, s->moov + (at - g->vstart), k);
        else {
            int got = s->src.read(s->src.u, g->src + (at - g->vstart), (uint8_t *)buf + done, k);
            if (got < 0) return got;
            done += (uint32_t)got;
            if ((uint32_t)got < k) break;
            continue;
        }
        done += k;
    }
    return (int)done;
}

int pc_mp4_faststart(const pc_reader_t *src, pc_reader_t *out, pc_mp4_fs_t **st)
{
    *st = NULL;
    *out = *src;
    uint8_t h[8];
    uint32_t ftyp_len = 0, moov_off = 0, moov_len = 0, mdat_off = 0;
    bool mdat_seen = false;
    for (uint32_t p = 0; p + 8 <= src->size;) {
        if (src->read(src->u, p, h, 8) != 8) return 0;
        uint32_t sz = be32(h);
        if (sz == 1 || sz == 0) { if (!memcmp(h + 4, "mdat", 4)) mdat_seen = true; break; }   /* 64-bit / to-end box: the rest */
        if (sz < 8 || sz > src->size - p) return p ? -1 : 0;
        if (!p && memcmp(h + 4, "ftyp", 4)) return 0;                  /* not an MP4 */
        if (!p) ftyp_len = sz;
        if (!memcmp(h + 4, "mdat", 4) && !mdat_seen) { mdat_seen = true; mdat_off = p; }
        if (!memcmp(h + 4, "moov", 4)) {
            if (!mdat_seen) return 0;                                   /* already moov first */
            moov_off = p;
            moov_len = sz;
            break;
        }
        p += sz;
    }
    if (!moov_len || !ftyp_len) return 0;
    if (moov_len > MOOV_MAX) return -1;
    pc_mp4_fs_t *s = calloc(1, sizeof *s);
    if (!s) return -1;
    s->src = *src;
    if (s->src.u == src) s->src.u = &s->src;                           /* memory readers point at themselves */
    s->moov = malloc(moov_len);
    if (!s->moov || src->read(src->u, moov_off, s->moov, moov_len) != (int)moov_len) { pc_mp4_faststart_free(s); return -1; }
    /* ftyp | moov | [ftyp_len, moov_off) | [moov_off + moov_len, end): the middle part moves up by moov_len */
    (void)mdat_off;
    patch(s->moov + 8, moov_len - 8, ftyp_len, moov_off, moov_len);
    uint32_t tail = moov_off + moov_len;
    s->seg[0] = (seg_t){ 0, 0, ftyp_len, false };
    s->seg[1] = (seg_t){ ftyp_len, 0, moov_len, true };
    s->seg[2] = (seg_t){ ftyp_len + moov_len, ftyp_len, moov_off - ftyp_len, false };
    s->seg[3] = (seg_t){ tail, tail, src->size - tail, false };
    s->nseg = 4;
    out->read = vread;
    out->u = s;
    out->size = src->size;
    *st = s;
    return 1;
}

void pc_mp4_faststart_free(pc_mp4_fs_t *st)
{
    if (!st) return;
    free(st->moov);
    free(st);
}
