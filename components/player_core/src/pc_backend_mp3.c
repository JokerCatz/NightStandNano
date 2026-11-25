// SPDX-License-Identifier: LGPL-2.1-or-later
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_FLOAT_OUTPUT
#include "minimp3.h"
#pragma GCC diagnostic pop

#define BUF_SIZE (16 * 1024)
#define NO_SYNC_LIMIT (64 * 1024)   /* give up after this many bytes without a decodable frame */

typedef struct {
    pc_reader_t r;
    uint32_t off;                  /* next file byte to read */
    uint8_t buf[BUF_SIZE];
    uint32_t len, pos;             /* valid bytes, consumed bytes */
    mp3dec_t dec;
    float pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    int pcm_frames, pcm_pos, channels;
    uint32_t rate;
    uint64_t played;
    uint32_t junk;                 /* bytes skipped since the last good frame */
    uint32_t skip;                 /* gapless: frames still to drop at the start (encoder + decoder delay) */
    int64_t remain;                /* gapless: frames left to output (-1: no LAME tag, play what decodes) */
    int64_t total;                 /* frames in the file per the Xing/Info tag, -1 unknown */
} mp3_t;

static void refill(mp3_t *m)
{
    if (m->pos) {
        memmove(m->buf, m->buf + m->pos, m->len - m->pos);
        m->len -= m->pos;
        m->pos = 0;
    }
    if (m->len < BUF_SIZE && m->off < m->r.size) {
        int got = m->r.read(m->r.u, m->off, m->buf + m->len, BUF_SIZE - m->len);
        if (got > 0) { m->len += (uint32_t)got; m->off += (uint32_t)got; }
    }
}

/* decodes the next frame into pcm; 1 ok, 0 end, -1 no sync for too long */
static int next_frame(mp3_t *m)
{
    for (;;) {
        if (m->len - m->pos < 4096) refill(m);
        if (m->pos >= m->len) return 0;
        mp3dec_frame_info_t fi;
        int s = mp3dec_decode_frame(&m->dec, m->buf + m->pos, (int)(m->len - m->pos), m->pcm, &fi);
        if (fi.frame_bytes == 0) {                   /* nothing decodable in what is buffered */
            if (m->off >= m->r.size) return 0;
            m->junk += m->len - m->pos;
            m->pos = m->len;
            if (m->junk > NO_SYNC_LIMIT) return -1;
            continue;
        }
        m->pos += (uint32_t)fi.frame_bytes;
        if (s <= 0) {
            m->junk += (uint32_t)fi.frame_bytes;
            if (m->junk > NO_SYNC_LIMIT) return -1;
            continue;
        }
        m->junk = 0;
        m->pcm_frames = s;
        m->pcm_pos = 0;
        m->channels = fi.channels;
        if (!m->rate) m->rate = (uint32_t)fi.hz;
        return 1;
    }
}

/* Xing / Info tag in the first frame (LAME, also ffmpeg's): frame count, and the LAME extension's encoder
 * delay and padding. The decoder adds 528 + 1 samples of its own delay (as minimp3_ex and mpg123 count it).
 * Returns 1 with the counts when the frame is a tag (it carries no audio), 0 otherwise. */
#define DECODER_DELAY 529
static int vbrtag(const uint8_t *frame, int frame_bytes, uint32_t *frames, int *delay, int *padding)
{
    if (frame_bytes < HDR_SIZE + 32 + 8 || HDR_GET_LAYER(frame) != 1) return 0;   /* layer III only */
    bs_t bs[1];
    L3_gr_info_t gr[4];
    bs_init(bs, frame + HDR_SIZE, frame_bytes - HDR_SIZE);
    if (HDR_IS_CRC(frame)) get_bits(bs, 16);
    if (L3_read_side_info(bs, gr, frame) < 0) return 0;
    const uint8_t *t = frame + HDR_SIZE + bs->pos / 8, *end = frame + frame_bytes;
    if (t + 8 > end || (memcmp(t, "Xing", 4) && memcmp(t, "Info", 4))) return 0;
    int flags = t[7];
    t += 8;
    *frames = 0;
    *delay = *padding = -1;
    if (flags & 1) { if (t + 4 > end) return 1; *frames = (uint32_t)t[0] << 24 | (uint32_t)t[1] << 16 | (uint32_t)t[2] << 8 | t[3]; t += 4; }
    if (flags & 2) t += 4;
    if (flags & 4) t += 100;
    if (flags & 8) t += 4;
    if (t + 24 <= end && t[0]) {                       /* encoder string ("LAME3.100", "Lavf"), then the LAME fields */
        *delay = t[21] << 4 | t[22] >> 4;
        *padding = (t[22] & 0xF) << 8 | t[23];
    }
    return 1;
}

/* the first frame (at m->pos): a Xing/Info tag is skipped; its counts set the trim */
static void gapless_setup(mp3_t *m)
{
    if (m->len - m->pos < 4096) refill(m);
    mp3dec_frame_info_t fi;
    int spf = mp3dec_decode_frame(&m->dec, m->buf + m->pos, (int)(m->len - m->pos), NULL, &fi);   /* NULL: find, no decode */
    mp3dec_init(&m->dec);
    if (spf <= 0 || !fi.frame_bytes) return;
    uint32_t frames;
    int delay, padding;
    const uint8_t *f = m->buf + m->pos + fi.frame_offset;
    if (!vbrtag(f, fi.frame_bytes - fi.frame_offset, &frames, &delay, &padding)) return;
    m->pos += (uint32_t)fi.frame_bytes;                /* the tag frame decodes as silence: not played */
    if (!m->rate) m->rate = (uint32_t)fi.hz;
    if (frames) m->total = (int64_t)frames * spf;
    if (delay >= 0) {
        m->skip = (uint32_t)delay + DECODER_DELAY;
        if (frames) m->remain = m->total = m->total - delay - padding;
        if (m->total < 0) m->total = m->remain = 0;
    }
}

static int mp3_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    mp3_t *m = calloc(1, sizeof *m);
    if (!m) return -1;
    m->r = *r;
    if (r->read == NULL) { free(m); return -1; }
    if (m->r.u == r) m->r.u = &m->r;               /* memory readers point at themselves */
    m->off = start_off;
    uint8_t h10[10];                               /* an ID3v2 tag here (cover art) is skipped, not scanned */
    if (m->r.read(m->r.u, start_off, h10, 10) == 10 && !memcmp(h10, "ID3", 3) && !((h10[6] | h10[7] | h10[8] | h10[9]) & 0x80))
        m->off += 10 + ((uint32_t)h10[6] << 21 | (uint32_t)h10[7] << 14 | (uint32_t)h10[8] << 7 | h10[9]) + ((h10[5] & 0x10) ? 10 : 0);
    mp3dec_init(&m->dec);
    m->remain = m->total = -1;
    if (start_off == 0 || m->off != start_off) gapless_setup(m);   /* only from the file's start (a resume lands mid-stream) */
    if (next_frame(m) != 1) { free(m); return -1; }
    *h = m;
    return 0;
}

static int mp3_render(void *h, float *out, int frames)
{
    mp3_t *m = h;
    int done = 0;
    while (done < frames && m->remain != 0) {
        if (m->pcm_pos >= m->pcm_frames) {
            int rc = next_frame(m);
            if (rc < 0) return done ? done : -1;
            if (rc == 0) break;
        }
        if (m->skip) {                                 /* gapless: encoder + decoder delay */
            int k = m->pcm_frames - m->pcm_pos;
            if ((uint32_t)k > m->skip) k = (int)m->skip;
            m->pcm_pos += k;
            m->skip -= (uint32_t)k;
            continue;
        }
        int n = m->pcm_frames - m->pcm_pos;
        if (n > frames - done) n = frames - done;
        if (m->remain > 0 && n > m->remain) n = (int)m->remain;   /* gapless: the encoder's padding is not played */
        const float *src = m->pcm + m->pcm_pos * m->channels;
        for (int i = 0; i < n; i++) {
            float l = src[i * m->channels], r = m->channels > 1 ? src[i * m->channels + 1] : l;
            out[2 * (done + i)] = l > 1.0f ? 1.0f : l < -1.0f ? -1.0f : l;
            out[2 * (done + i) + 1] = r > 1.0f ? 1.0f : r < -1.0f ? -1.0f : r;
        }
        m->pcm_pos += n;
        done += n;
        if (m->remain > 0) m->remain -= n;
    }
    m->played += (uint64_t)done;
    return done;
}

static uint32_t mp3_rate(void *h) { return ((mp3_t *)h)->rate; }
static int32_t mp3_length(void *h) { mp3_t *m = h; return m->total >= 0 && m->rate ? (int32_t)(m->total * 1000 / m->rate) : -1; }
static int32_t mp3_pos(void *h) { mp3_t *m = h; return m->rate ? (int32_t)(m->played * 1000 / m->rate) : 0; }
static void mp3_close(void *h) { free(h); }
static uint32_t mp3_byte_pos(void *h) { mp3_t *m = h; return m->off - (m->len - m->pos); }   /* read, minus what waits in the buffer */

const pc_backend_t pc_backend_mp3 = {
    .open = mp3_open, .render = mp3_render, .rate = mp3_rate,
    .length_ms = mp3_length, .position_ms = mp3_pos, .close = mp3_close, .byte_pos = mp3_byte_pos,
    .name = "MP3",
};
