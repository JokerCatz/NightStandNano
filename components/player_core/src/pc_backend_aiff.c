// SPDX-License-Identifier: LGPL-2.1-or-later
/* AIFF / AIFF-C decoder backend: big-endian integer PCM 8/16/24/32-bit (AIFF-C 'NONE', and 'sowt' = the same samples little-endian),
 * 1 or 2 channels (mono to both sides), the 80-bit extended sample rate. Same shape as pc_backend_wav.c: every number read from the
 * file is untrusted, the header is walked chunk by chunk through the reader, sizes are checked in 64 bits, the audio length is clipped
 * to the reader's size and to whole frames. byte_pos = payload bytes to the next frame (open takes the same), as for WAV. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"

#define AIFF_BUF 4096             /* read size; one allocation per open (handle + this buffer) */
#define AIFF_MIN_RATE 8000u
#define AIFF_MAX_RATE 768000u     /* parse limit only: the caller's rate guard decides what plays */
#define AIFF_MAX_CHUNKS 256       /* header walk bound */

typedef struct {
    pc_reader_t r;
    uint32_t rate;
    uint32_t data_off;            /* file offset of the sample data */
    uint32_t data_len;            /* payload bytes, a whole number of frames, inside the reader */
    uint32_t pos;                 /* payload bytes handed out (frame-aligned) */
    uint32_t fpos;                /* payload bytes read into buf */
    uint32_t blen, bpos;
    uint16_t ch, bytes, frame;    /* bytes per sample, frame = ch * bytes */
    uint8_t bits;                 /* sample size from the file (the source depth shown) */
    bool le;                      /* 'sowt' */
    uint8_t buf[AIFF_BUF];
} aiff_t;

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint16_t be16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }
static bool rd(const pc_reader_t *r, uint32_t off, void *b, uint32_t n) { return r->read(r->u, off, b, n) == (int)n; }

/* IEEE 754 80-bit extended, big-endian: sign + 15-bit exponent, 64-bit mantissa with an explicit integer bit. 0 = not a usable rate. */
static uint32_t rate80(const uint8_t *p)
{
    uint32_t e = be16(p) & 0x7FFF;
    if (p[0] & 0x80) return 0;                                   /* negative */
    uint64_t m = (uint64_t)be32(p + 2) << 32 | be32(p + 6);
    if (!m || e < 0x3FFF || e > 0x3FFF + 31) return 0;           /* 1 .. 2^32 */
    int sh = 63 - (int)(e - 0x3FFF);                             /* the value is m >> sh */
    uint64_t v = (m + (1ull << (sh - 1))) >> sh;                 /* sh >= 32 here: rounded */
    return v > 0xFFFFFFFFu ? 0 : (uint32_t)v;
}

static bool parse(aiff_t *a)
{
    const pc_reader_t *r = &a->r;
    uint8_t h[12], c[8], f[26];
    if (r->size < 12 || !rd(r, 0, h, 12) || memcmp(h, "FORM", 4)) return false;
    bool aifc = !memcmp(h + 8, "AIFC", 4);
    if (!aifc && memcmp(h + 8, "AIFF", 4)) return false;
    bool comm = false;
    uint32_t ssnd_sz = 0;
    uint64_t ssnd_body = 0;
    uint32_t frames = 0;
    uint64_t o = 12;
    for (int k = 0; k < AIFF_MAX_CHUNKS && o + 8 <= r->size; k++) {
        if (!rd(r, (uint32_t)o, c, 8)) return false;
        uint32_t sz = be32(c + 4);
        uint64_t body = o + 8;
        if (!memcmp(c, "COMM", 4) && !comm) {
            if (sz < 18 || body + sz > r->size) return false;
            uint32_t need = aifc ? 22u : 18u;
            if (sz < need || !rd(r, (uint32_t)body, f, need)) return false;
            uint16_t ch = be16(f), bits = be16(f + 6);
            frames = be32(f + 2);
            uint32_t rate = rate80(f + 8);
            if (aifc) {
                if (!memcmp(f + 18, "sowt", 4)) a->le = true;
                else if (memcmp(f + 18, "NONE", 4)) return false;   /* compressed or float: not played */
            }
            if (ch < 1 || ch > 2 || bits < 8 || bits > 32 || rate < AIFF_MIN_RATE || rate > AIFF_MAX_RATE) return false;
            a->ch = ch;
            a->bits = (uint8_t)bits;
            a->bytes = (uint16_t)((bits + 7) / 8);
            a->frame = (uint16_t)(a->ch * a->bytes);
            a->rate = rate;
            comm = true;
        } else if (!memcmp(c, "SSND", 4) && !ssnd_sz) {
            if (sz < 8 || body + 8 > r->size) return false;        /* SSND may come before COMM (the spec allows it): remember it */
            ssnd_body = body;
            ssnd_sz = sz;
        }
        if (comm && ssnd_sz) {
            uint32_t sz = ssnd_sz;
            uint64_t body = ssnd_body;
            uint8_t s[8];
            if (!rd(r, (uint32_t)body, s, 8)) return false;
            uint64_t start = body + 8 + be32(s);                    /* the offset field skips padding before the first frame */
            if (start >= r->size) return false;
            uint64_t len = sz - 8u > be32(s) ? sz - 8u - be32(s) : 0;
            if (len > r->size - start) len = r->size - start;       /* a lying size: what the file really holds */
            if ((uint64_t)frames * a->frame < len) len = (uint64_t)frames * a->frame;
            len -= len % a->frame;
            if (!len) return false;
            a->data_off = (uint32_t)start;
            a->data_len = (uint32_t)len;
            return true;
        }
        o = body + sz + (sz & 1);                                   /* 64-bit: a size near 2^32 cannot wrap */
    }
    return false;
}

static int aiff_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    aiff_t *a = calloc(1, sizeof *a);
    if (!a) return -1;
    a->r = *r;
    if (a->r.u == r) a->r.u = &a->r;         /* memory readers point at themselves: the copy must not keep the caller's address */
    if (!parse(a)) { free(a); return -1; }
    uint32_t at = start_off - start_off % a->frame;
    if (at >= a->data_len) at = 0;           /* a resume token past the end: from the start (as MPC and TTA do) */
    a->pos = a->fpos = at;
    *h = a;
    return 0;
}

static float to_float(const aiff_t *a, const uint8_t *p)
{
    int32_t v = 0;
    if (a->le) { for (int i = a->bytes - 1; i >= 0; i--) v = (int32_t)((uint32_t)v << 8 | p[i]); }
    else { for (int i = 0; i < a->bytes; i++) v = (int32_t)((uint32_t)v << 8 | p[i]); }
    int top = 8 * a->bytes;                                         /* sign-extend from the container width */
    if (top < 32) v = (int32_t)((uint32_t)v << (32 - top)) >> (32 - top);
    return (float)v * (1.0f / (float)(1ull << (top - 1)));
}

static bool refill(aiff_t *a, bool *err)
{
    uint32_t rem = a->blen - a->bpos;
    memmove(a->buf, a->buf + a->bpos, rem);
    a->bpos = 0;
    a->blen = rem;
    uint32_t want = AIFF_BUF - rem;
    if (want > a->data_len - a->fpos) want = a->data_len - a->fpos;
    if (want) {
        int got = a->r.read(a->r.u, a->data_off + a->fpos, a->buf + rem, want);
        if (got < 0) { *err = true; return false; }
        a->fpos += (uint32_t)got;
        a->blen += (uint32_t)got;
    }
    return a->blen >= a->frame;
}

static int aiff_render(void *h, float *out, int frames)
{
    aiff_t *a = h;
    int done = 0;
    bool err = false;
    while (done < frames) {
        if (a->blen - a->bpos < a->frame && !refill(a, &err)) break;
        int n = (int)((a->blen - a->bpos) / a->frame);
        if (n > frames - done) n = frames - done;
        const uint8_t *p = a->buf + a->bpos;
        for (int i = 0; i < n; i++, p += a->frame) {
            float l = to_float(a, p);
            out[2 * (done + i)] = l;
            out[2 * (done + i) + 1] = a->ch > 1 ? to_float(a, p + a->bytes) : l;
        }
        a->bpos += (uint32_t)n * a->frame;
        a->pos += (uint32_t)n * a->frame;
        done += n;
    }
    return done ? done : err ? -1 : 0;
}

static uint32_t aiff_rate(void *h) { return ((aiff_t *)h)->rate; }
static int32_t aiff_len(void *h) { aiff_t *a = h; return (int32_t)((uint64_t)(a->data_len / a->frame) * 1000 / a->rate); }
static int32_t aiff_pos(void *h) { aiff_t *a = h; return (int32_t)((uint64_t)(a->pos / a->frame) * 1000 / a->rate); }
static uint32_t aiff_bytepos(void *h) { return ((aiff_t *)h)->pos; }
static void aiff_close(void *h) { free(h); }
static uint8_t aiff_bits(void *h) { return ((aiff_t *)h)->bits; }

const pc_backend_t pc_backend_aiff = {
    .open = aiff_open, .render = aiff_render, .rate = aiff_rate, .length_ms = aiff_len, .position_ms = aiff_pos,
    .close = aiff_close, .byte_pos = aiff_bytepos,
    .name = "AIFF", .bits = aiff_bits,
};
