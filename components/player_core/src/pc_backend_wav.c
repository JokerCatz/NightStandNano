// SPDX-License-Identifier: LGPL-2.1-or-later
/* WAV decoder backend: PCM 16/24/32-bit int, IEEE float 32-bit, WAVE_FORMAT_EXTENSIBLE (sub-format PCM or FLOAT), 1 or 2 channels.
 * Every number read from the file is untrusted: the header is walked chunk by chunk through the reader, sizes are
 * checked in 64 bits, and the audio length is clipped to the reader's size and to whole frames. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"

#define WAV_BUF 4096              /* read size; one allocation per open (handle + this buffer) */
#define WAV_MIN_RATE 8000u
#define WAV_MAX_RATE 768000u      /* parse limit only: the caller's rate guard decides what plays */
#define WAV_MAX_CHUNKS 256        /* header walk bound (a file made of thousands of tiny chunks is refused) */

typedef struct {
    pc_reader_t r;
    uint32_t rate;
    uint32_t data_off;            /* file offset of the data payload */
    uint32_t data_len;            /* payload bytes, a whole number of frames, inside the reader */
    uint32_t pos;                 /* payload bytes handed out (frame-aligned): the next frame to render */
    uint32_t fpos;                /* payload bytes read into buf */
    uint32_t blen, bpos;          /* valid bytes in buf, consumed bytes of them */
    uint16_t ch, bits, frame;     /* frame = block_align = ch * bits / 8 */
    bool flt;
    uint8_t buf[WAV_BUF];
} wav_t;

static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

static bool rd(const pc_reader_t *r, uint32_t off, void *b, uint32_t n) { return r->read(r->u, off, b, n) == (int)n; }

/* the 14 bytes after the 16-bit sub-format of KSDATAFORMAT_SUBTYPE_PCM / _IEEE_FLOAT */
static const uint8_t GUID_TAIL[14] = { 0, 0, 0, 0, 0x10, 0, 0x80, 0, 0, 0xAA, 0, 0x38, 0x9B, 0x71 };

/* fills the format part of w from the fmt chunk payload f (n bytes, n >= 16 checked by the caller) */
static bool parse_fmt(wav_t *w, const uint8_t *f, uint32_t n)
{
    uint16_t tag = rd16(f), ch = rd16(f + 2), align = rd16(f + 12), bits = rd16(f + 14);
    uint32_t rate = rd32(f + 4);
    if (tag == 0xFFFE) {
        if (n < 40 || rd16(f + 16) < 22) return false;                   /* cbSize covers valid bits, mask and the GUID */
        if (memcmp(f + 26, GUID_TAIL, sizeof GUID_TAIL)) return false;   /* not one of the standard sub-formats */
        tag = rd16(f + 24);
    }
    if (tag == 1) { if (bits != 16 && bits != 24 && bits != 32) return false; }
    else if (tag == 3) { if (bits != 32) return false; }
    else return false;
    if (ch < 1 || ch > 2 || rate < WAV_MIN_RATE || rate > WAV_MAX_RATE) return false;
    if (align != (uint32_t)ch * bits / 8) return false;                   /* also catches bits not a multiple of 8 */
    w->flt = tag == 3;
    w->ch = ch;
    w->bits = bits;
    w->frame = align;
    w->rate = rate;
    return true;
}

/* walks the chunks (RIFF header, odd sizes padded) up to the data chunk; false = not a WAV we play */
static bool parse(wav_t *w)
{
    const pc_reader_t *r = &w->r;
    uint8_t h[12], c[8], f[40];
    if (r->size < 12 || !rd(r, 0, h, 12) || memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4)) return false;
    bool fmt = false;
    uint64_t o = 12;
    for (int k = 0; k < WAV_MAX_CHUNKS && o + 8 <= r->size; k++) {
        if (!rd(r, (uint32_t)o, c, 8)) return false;
        uint32_t sz = rd32(c + 4);
        uint64_t body = o + 8;
        if (!memcmp(c, "fmt ", 4) && !fmt) {
            if (sz < 16 || body + sz > r->size) return false;
            uint32_t n = sz < sizeof f ? sz : (uint32_t)sizeof f;
            if (!rd(r, (uint32_t)body, f, n) || !parse_fmt(w, f, n)) return false;
            fmt = true;
        } else if (!memcmp(c, "data", 4)) {
            if (!fmt || body >= r->size) return false;
            uint64_t len = sz < r->size - body ? sz : r->size - body;       /* a lying size: what the file really holds */
            len -= len % w->frame;
            if (!len) return false;
            w->data_off = (uint32_t)body;
            w->data_len = (uint32_t)len;
            return true;
        }
        o = body + sz + (sz & 1);                                           /* 64-bit: a size near 2^32 cannot wrap */
    }
    return false;
}

static int wav_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    wav_t *w = calloc(1, sizeof *w);
    if (!w) return -1;
    w->r = *r;
    if (!parse(w)) { free(w); return -1; }
    uint32_t at = start_off - start_off % w->frame;
    if (at > w->data_len) at = w->data_len;  /* a resume token past the end: plays nothing */
    w->pos = w->fpos = at;
    *h = w;
    return 0;
}

static float to_float(const wav_t *w, const uint8_t *p)
{
    if (w->flt) {
        float v;
        memcpy(&v, p, 4);
        if (v != v) return 0.0f;             /* NaN */
        return v > 1.0f ? 1.0f : v < -1.0f ? -1.0f : v;
    }
    if (w->bits == 16) return (float)(int16_t)rd16(p) * (1.0f / 32768.0f);
    if (w->bits == 24) return (float)((int32_t)((uint32_t)p[0] << 8 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 24) >> 8) * (1.0f / 8388608.0f);
    return (float)(int32_t)rd32(p) * (1.0f / 2147483648.0f);
}

/* makes at least one whole frame available at buf[bpos]; false = end of data or read error */
static bool refill(wav_t *w, bool *err)
{
    uint32_t rem = w->blen - w->bpos;
    memmove(w->buf, w->buf + w->bpos, rem);
    w->bpos = 0;
    w->blen = rem;
    uint32_t want = WAV_BUF - rem;
    if (want > w->data_len - w->fpos) want = w->data_len - w->fpos;
    if (want) {
        int got = w->r.read(w->r.u, w->data_off + w->fpos, w->buf + rem, want);
        if (got < 0) { *err = true; return false; }
        w->fpos += (uint32_t)got;
        w->blen += (uint32_t)got;
    }
    return w->blen >= w->frame;
}

static int wav_render(void *h, float *out, int frames)
{
    wav_t *w = h;
    int done = 0;
    bool err = false;
    while (done < frames) {
        if (w->blen - w->bpos < w->frame && !refill(w, &err)) break;
        int n = (int)((w->blen - w->bpos) / w->frame);
        if (n > frames - done) n = frames - done;
        const uint8_t *p = w->buf + w->bpos;
        uint32_t step = w->bits / 8;
        for (int i = 0; i < n; i++, p += w->frame) {
            float l = to_float(w, p);
            out[2 * (done + i)] = l;
            out[2 * (done + i) + 1] = w->ch > 1 ? to_float(w, p + step) : l;
        }
        w->bpos += (uint32_t)n * w->frame;
        w->pos += (uint32_t)n * w->frame;
        done += n;
    }
    return done ? done : err ? -1 : 0;
}

static uint32_t wav_rate(void *h) { return ((wav_t *)h)->rate; }
static int32_t wav_len(void *h) { wav_t *w = h; return (int32_t)((uint64_t)(w->data_len / w->frame) * 1000 / w->rate); }
static int32_t wav_pos(void *h) { wav_t *w = h; return (int32_t)((uint64_t)(w->pos / w->frame) * 1000 / w->rate); }
static uint32_t wav_bytepos(void *h) { return ((wav_t *)h)->pos; }
static void wav_close(void *h) { free(h); }
static uint8_t wav_bits(void *h) { return (uint8_t)((wav_t *)h)->bits; }

const pc_backend_t pc_backend_wav = {
    .open = wav_open, .render = wav_render, .rate = wav_rate, .length_ms = wav_len, .position_ms = wav_pos,
    .close = wav_close, .byte_pos = wav_bytepos,
    .name = "WAV", .bits = wav_bits,
};
