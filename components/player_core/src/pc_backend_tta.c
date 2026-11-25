// SPDX-License-Identifier: LGPL-2.1-or-later
/* True Audio (.tta) decoder backend over libtta (LGPL, components/libtta). 16..24-bit, 1 or 2 channels; the sample rate is the file's.
 * libtta keeps its state in globals: ONE decoder at a time. open() claims it and fails while another TTA handle is open (gapless pre-open
 * of a second TTA file then fails and the file is opened normally when its turn comes, like any failed pre-open).
 * Resume token (byte_pos): the PCM frame (sample) index. open() jumps to the TTA frame (about 1.04 s) that holds it through the seek
 * table and decodes the remainder away; a file without a usable seek table decodes from the start. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "libtta.h"
#include "pc/pc_backend.h"

int  tta_decoder_claim(void);
void tta_decoder_release(void);
int  tta_decoder_frame_init(TTAuint32 frame, bool seek_needed);   /* libtta.c, not in its header */
extern bool dec_seek_allowed;

#define CHUNK 512                              /* PCM frames decoded per libtta call */

typedef struct {
    TTA_io_callback io;                        /* first member: the callbacks get a pointer to it */
    pc_reader_t r;
    uint32_t off;
    TTA_info info;
    uint32_t depth;                            /* bytes per sample in the packed output */
    float scale;
    uint64_t pos;                              /* PCM frames handed out */
    bool ended;
    uint8_t buf[CHUNK * 2 * 3] __attribute__((aligned(4)));   /* libtta stores 16-bit samples with int16 stores: an odd address traps on RISC-V (ASan caught it) */
} tta_t;

static TTAint32 io_read(TTA_io_callback *io, TTAuint8 *dst, TTAuint32 n)
{
    tta_t *t = (tta_t *)io;
    if (t->off >= t->r.size) return 0;
    if (n > t->r.size - t->off) n = t->r.size - t->off;
    int got = t->r.read(t->r.u, t->off, dst, n);
    if (got <= 0) return 0;
    t->off += (uint32_t)got;
    return got;
}
static TTAint32 io_write(TTA_io_callback *io, TTAuint8 *src, TTAuint32 n) { (void)io; (void)src; (void)n; return 0; }
static TTAint64 io_seek(TTA_io_callback *io, TTAint64 offset)
{
    tta_t *t = (tta_t *)io;
    if (offset < 0 || (uint64_t)offset > t->r.size) return -1;
    t->off = (uint32_t)offset;
    return offset;
}

static void t_close(void *h)
{
    tta_t *t = h;
    if (!t) return;
    tta_decoder_done();
    tta_decoder_release();
    free(t);
}

/* decodes up to `want` PCM frames into t->buf; returns frames, 0 at the end, <0 on error */
static int decode(tta_t *t, int want)
{
    if (t->ended) return 0;
    if (want > CHUNK) want = CHUNK;
    int n = tta_decoder_process_stream(t->buf, (TTAuint32)want * t->info.nch * t->depth, NULL);
    if (n < 0) return -1;
    if (n == 0) t->ended = true;
    return n;
}

#define RUNUP_MAX_S 60u                    /* no seek table: the longest run-up (seconds of audio) open() will decode and drop */

static int t_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    if (!r->read || r->size < 32) return -1;
    if (!tta_decoder_claim()) return -1;
    tta_t *t = calloc(1, sizeof *t);
    if (!t) { tta_decoder_release(); return -1; }
    t->r = *r;
    if (t->r.u == r) t->r.u = &t->r;           /* memory readers point at themselves */
    t->io = (TTA_io_callback){ .read = io_read, .write = io_write, .seek = io_seek };
    tta_decoder_new(&t->io);
    if (tta_decoder_init_get_info(&t->info)) goto fail;
    uint32_t sps = t->info.sps;
    if (t->info.format != TTA_FORMAT_SIMPLE || t->info.nch < 1 || t->info.nch > 2 || sps < 8000 || sps > 384000 || !t->info.samples) goto fail;
    t->depth = (t->info.bps + 7) / 8;
    t->scale = 1.0f / (float)(1u << (t->info.bps - 1));
    if (start_off && start_off < t->info.samples) {
        uint32_t flen = 256u * sps / 245u;     /* samples in one TTA frame */
        uint32_t frame = start_off / flen;
        if (dec_seek_allowed) {
            if (tta_decoder_frame_init(frame, true) < 0) goto fail;
            t->pos = (uint64_t)frame * flen;
        } else if (start_off > RUNUP_MAX_S * sps) {
            start_off = 0;                      /* no usable seek table: the run-up decodes from the start inside open(); past RUNUP_MAX_S the resume starts the track over instead (wave-6 #19) */
        }                                       /* else: decoded from the start, everything before start_off dropped below */
        while (t->pos < start_off) {
            uint32_t want = start_off - (uint32_t)t->pos;
            if (want > CHUNK) want = CHUNK;
            int n = decode(t, (int)want);
            if (n <= 0) goto fail;
            t->pos += (uint64_t)n;
        }
    }
    *h = t;
    return 0;
fail:
    t_close(t);
    return -1;
}

static int t_render(void *h, float *out, int frames)
{
    tta_t *t = h;
    int done = 0;
    while (done < frames) {
        int n = decode(t, frames - done);
        if (n < 0) return done ? done : -1;
        if (n == 0) break;
        const uint8_t *p = t->buf;
        for (int i = 0; i < n; i++) {
            float v[2] = { 0.0f, 0.0f };
            for (uint32_t c = 0; c < t->info.nch; c++, p += t->depth) {
                int32_t s = t->depth == 2 ? (int16_t)(p[0] | p[1] << 8)
                                          : (int32_t)((uint32_t)p[0] << 8 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 24) >> 8;
                v[c] = (float)s * t->scale;
            }
            out[2 * (done + i)] = v[0];
            out[2 * (done + i) + 1] = t->info.nch > 1 ? v[1] : v[0];
        }
        t->pos += (uint64_t)n;
        done += n;
    }
    return done;
}

static uint32_t t_rate(void *h) { return ((tta_t *)h)->info.sps; }
static int32_t t_length(void *h) { tta_t *t = h; return (int32_t)((uint64_t)t->info.samples * 1000 / t->info.sps); }
static int32_t t_position(void *h) { tta_t *t = h; return (int32_t)(t->pos * 1000 / t->info.sps); }
static uint32_t t_byte_pos(void *h) { tta_t *t = h; return t->ended ? 0 : (uint32_t)t->pos; }
static uint8_t t_bits(void *h) { return (uint8_t)((tta_t *)h)->info.bps; }

const pc_backend_t pc_backend_tta = {
    .open = t_open, .render = t_render, .rate = t_rate, .length_ms = t_length, .position_ms = t_position, .close = t_close,
    .byte_pos = t_byte_pos, .name = "TTA", .bits = t_bits,
};
