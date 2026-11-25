// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_stream.h"
#include <string.h>

/* the decoder's reader: offsets only grow, except a decoder may re-read the first bytes (ID3 check) */
static int stream_read(void *u, uint32_t off, void *buf, uint32_t n)
{
    pc_stream_t *s = u;
    uint32_t done = 0;
    while (off < s->consumed && done < n) {           /* re-read of the start */
        if (off >= s->head_n) return (int)done;       /* older than what we kept: nothing */
        ((uint8_t *)buf)[done++] = s->head[off++];
    }
    uint32_t got = pc_ring_read(s->ring, (uint8_t *)buf + done, n - done);
    if (s->head_n < sizeof s->head) {
        uint32_t k = got < sizeof s->head - s->head_n ? got : (uint32_t)sizeof s->head - s->head_n;
        memcpy(s->head + s->head_n, (uint8_t *)buf + done, k);
        s->head_n += k;
    }
    s->consumed += got;
    return (int)(done + got);
}

static int st_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    (void)start_off;
    pc_stream_t *s = r->u;
    if (!s || !s->ring || !s->inner) return -1;
    s->rd = (pc_reader_t){ .read = stream_read, .size = UINT32_MAX, .u = s };
    s->head_n = s->consumed = 0;
    s->buffering = false;
    s->underruns = 0;
    if (s->inner->open(&s->ih, &s->rd, 0)) return -1;
    *h = s;
    return 0;
}

static int st_render(void *h, float *out, int frames)
{
    pc_stream_t *s = h;
    bool eof = s->eof;
    uint32_t used = pc_ring_used(s->ring);
    if (s->buffering && (eof || used >= s->prebuffer)) s->buffering = false;
    if (!s->buffering && !eof && used < PC_STREAM_LOW) {   /* running dry: pause instead of decoding a torn frame */
        s->buffering = true;
        s->underruns++;
    }
    if (s->buffering) {
        memset(out, 0, sizeof(float) * 2 * (size_t)frames);
        return frames;
    }
    int n = s->inner->render(s->ih, out, frames);
    if (n >= 0 && n < frames && !s->eof) {            /* the decoder ran dry anyway: still a pause */
        memset(out + 2 * n, 0, sizeof(float) * 2 * (size_t)(frames - n));
        s->buffering = true;
        s->underruns++;
        return frames;
    }
    return n;
}

static uint32_t st_rate(void *h) { pc_stream_t *s = h; return s->inner->rate(s->ih); }
static int32_t st_len(void *h) { (void)h; return -1; }
static int32_t st_pos(void *h) { pc_stream_t *s = h; return s->inner->position_ms ? s->inner->position_ms(s->ih) : 0; }
static void st_close(void *h) { pc_stream_t *s = h; s->inner->close(s->ih); s->ih = NULL; }

const pc_backend_t pc_backend_stream = {
    .open = st_open, .render = st_render, .rate = st_rate, .length_ms = st_len, .position_ms = st_pos, .close = st_close,
    .name = "Radio",
};
