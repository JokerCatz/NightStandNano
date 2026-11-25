// SPDX-License-Identifier: LGPL-2.1-or-later
#include "trim.h"
#include <string.h>
#include <stdlib.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "backend_sdec.h"

static const char *TAG = "trim";

typedef struct {
    const pc_backend_t *in;
    void *h;
    uint32_t rate;
    bool active;                           /* the container said and trimming is on; otherwise the decoder's output passes through */
    uint64_t skip_left, keep;              /* frames still to drop at the start; frames to play in all (0 = pass through) */
    uint64_t skip, ckeep, got, played;     /* what the container says (skip, length); delivered by the decoder, handed on */
    int last;                              /* what the decoder said when it ended: 0 end of stream, <0 error */
    char what[8];                          /* "" = the container did not say */
} trim_t;

static volatile bool s_on = true;
void nsn_trim_enable(bool on) { s_on = on; }
bool nsn_trim_enabled(void) { return s_on; }

static uint32_t le16(const uint8_t *p) { return p[0] | p[1] << 8; }
static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static uint64_t le64(const uint8_t *p) { return le32(p) | (uint64_t)le32(p + 4) << 32; }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static uint64_t be64(const uint8_t *p) { return (uint64_t)be32(p) << 32 | be32(p + 4); }

/* ---- Ogg: Vorbis and Opus ---- */
#define OGG_TAIL 70000                     /* a page is at most 65307 bytes */

/* frames in the stream (granule position of the last complete page) and the number to skip at the start; false: not told. *opus: it was an Opus stream (48 kHz units) */
static bool ogg_info(const pc_reader_t *r, uint64_t *skip, uint64_t *keep, bool *opus)
{
    uint8_t head[512];
    int n = r->read(r->u, 0, head, sizeof head < r->size ? sizeof head : r->size);
    if (n < 64 || memcmp(head, "OggS", 4)) return false;
    uint32_t pre = 0;
    *opus = false;
    for (int i = 27; i + 19 <= n; i++)
        if (!memcmp(head + i, "OpusHead", 8)) { *opus = true; pre = le16(head + i + 10); break; }
    if (!*opus) { bool v = false; for (int i = 27; i + 7 <= n; i++) if (!memcmp(head + i, "\x01vorbis", 7)) { v = true; break; } if (!v) return false; }
    uint32_t len = r->size < OGG_TAIL ? r->size : OGG_TAIL;
    uint8_t *b = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    if (!b) return false;
    bool ok = false;
    if ((uint32_t)r->read(r->u, r->size - len, b, len) == len) {
        for (int p = (int)len - 27; p >= 0; p--) {
            if (memcmp(b + p, "OggS", 4) || b[p + 4] != 0) continue;
            uint32_t nseg = b[p + 26], psz = 27 + nseg;
            if (p + psz > len) continue;
            for (uint32_t k = 0; k < nseg; k++) psz += b[p + 27 + k];
            if (p + psz > len) continue;                        /* a cut-off page: look at the one before */
            int64_t g = (int64_t)le64(b + p + 6);
            if (g < 0) continue;                                /* no packet ends on this page */
            if ((uint64_t)g > pre) { *skip = pre; *keep = (uint64_t)g - pre; ok = true; }
            break;
        }
    }
    heap_caps_free(b);
    return ok;
}

/* ---- MP4 / M4A ---- */
#define MOOV_MAX (4u << 20)

static const uint8_t *find_box(const uint8_t *p, const uint8_t *end, const char *type, uint32_t *size)
{
    while (p + 8 <= end) {
        uint32_t sz = be32(p);
        if (sz < 8 || p + sz > end) return NULL;
        if (!memcmp(p + 4, type, 4)) { *size = sz; return p; }
        p += sz;
    }
    return NULL;
}

/* skip / keep in media-timescale frames (the decoder's own rate when the timescale is the sample rate); *ts = that timescale */
static bool mp4_info(const pc_reader_t *r, uint64_t *skip, uint64_t *keep, uint32_t *ts_out)
{
    uint8_t h[16];
    uint64_t off = 0, moov_off = 0, moov_sz = 0;
    for (int i = 0; i < 256 && off + 8 <= r->size; i++) {                  /* the top-level boxes: moov may follow a big mdat */
        if (r->read(r->u, (uint32_t)off, h, 16) < 8) return false;
        uint64_t sz = be32(h);
        if (sz == 1) sz = be64(h + 8); else if (sz == 0) sz = r->size - off;
        if (sz < 8) return false;
        if (!memcmp(h + 4, "moov", 4)) { moov_off = off; moov_sz = sz; break; }
        off += sz;
    }
    if (!moov_sz || moov_sz > MOOV_MAX || moov_off + moov_sz > r->size) return false;
    uint8_t *m = heap_caps_malloc((size_t)moov_sz + 1, MALLOC_CAP_SPIRAM);
    if (!m) return false;
    bool ok = false;
    if ((uint64_t)r->read(r->u, (uint32_t)moov_off, m, (uint32_t)moov_sz) != moov_sz) goto out;
    const uint8_t *end = m + moov_sz, *body = m + 8;
    uint32_t sz;
    /* iTunSMPB: " 00000000 <delay> <padding> <sample count> ..." in an ilst "data" atom (iTunes, many taggers, faac, newer ffmpeg with use_metadata_tags) */
    for (const uint8_t *q = body; q + 8 < end; q++) {
        if (memcmp(q, "iTunSMPB", 8)) continue;
        const uint8_t *d = q + 8;
        for (int k = 0; k < 64 && d + 4 < end; k++, d++) if (!memcmp(d, "data", 4)) break;
        if (d + 4 >= end || memcmp(d, "data", 4)) break;
        char txt[128] = { 0 };
        size_t tl = (size_t)(end - (d + 12)) < sizeof txt - 1 ? (size_t)(end - (d + 12)) : sizeof txt - 1;
        memcpy(txt, d + 12, tl);                                          /* skip "data", version / flags, locale */
        unsigned z, delay, pad; unsigned long long total;
        if (sscanf(txt, " %x %x %x %llx", &z, &delay, &pad, &total) == 4 && total > 0 && total < (1ull << 40)) { *skip = delay; *keep = total; *ts_out = 0; ok = true; goto out; }
        break;
    }
    uint32_t tsz, vsz;
    const uint8_t *trak = find_box(body, end, "trak", &tsz);
    const uint8_t *mvhd = find_box(body, end, "mvhd", &vsz);
    if (!trak || !mvhd) goto out;
    const uint8_t *mdia = find_box(trak + 8, trak + tsz, "mdia", &sz);
    if (!mdia) goto out;
    uint32_t mvts = mvhd[8] == 1 ? be32(mvhd + 28) : be32(mvhd + 20);
    const uint8_t *mdhd = find_box(mdia + 8, mdia + sz, "mdhd", &sz);
    if (!mdhd) goto out;
    uint32_t ts; uint64_t mdur;
    if (mdhd[8] == 1) { ts = be32(mdhd + 28); mdur = be64(mdhd + 32); } else { ts = be32(mdhd + 20); mdur = be32(mdhd + 24); }
    const uint8_t *edts = find_box(trak + 8, trak + tsz, "edts", &sz);
    const uint8_t *elst = edts ? find_box(edts + 8, edts + sz, "elst", &sz) : NULL;
    if (!elst || !mvts || !ts) goto out;
    uint32_t ver = elst[8], cnt = be32(elst + 12);
    if (cnt != 1) goto out;                                               /* several edits (gaps, repeats): not a plain encoder-delay list */
    uint64_t seg; int64_t mt;
    if (ver == 1) { seg = be64(elst + 16); mt = (int64_t)be64(elst + 24); } else { seg = be32(elst + 16); mt = (int32_t)be32(elst + 20); }
    if (mt < 0 || seg == 0) goto out;
    uint64_t k = (seg * ts + mvts / 2) / mvts;                            /* the edit's length in media frames, as exact as the movie timescale is */
    uint64_t alt = mdur > (uint64_t)mt ? mdur - (uint64_t)mt : 0;         /* the whole media minus the priming: exact, but may still hold padding */
    uint64_t tick = ts / mvts + 1;
    if (alt && (alt > k ? alt - k : k - alt) <= tick) k = alt;
    *skip = (uint64_t)mt; *keep = k; *ts_out = ts; ok = true;
out:
    heap_caps_free(m);
    return ok;
}

/* ---- the wrapper ---- */
static int trim_open_with(void **h, const pc_reader_t *r, uint32_t off, const pc_backend_t *in, bool m4a)
{
    trim_t *t = heap_caps_calloc(1, sizeof *t, MALLOC_CAP_SPIRAM);
    if (!t) return -1;
    if (in->open(&t->h, r, off) != 0) { heap_caps_free(t); return -1; }
    t->in = in;
    t->rate = in->rate(t->h);
    uint64_t skip = 0, keep = 0; uint32_t ts = 0; bool opus = false;
    bool ok = m4a ? mp4_info(r, &skip, &keep, &ts) : ogg_info(r, &skip, &keep, &opus);
    if (ok && t->rate) {
        uint32_t from = m4a ? ts : opus ? 48000 : t->rate;                 /* the unit the container counts in; 0 (iTunSMPB): the decoder's own */
        if (from && from != t->rate) { skip = skip * t->rate / from; keep = keep * t->rate / from; }
        t->skip = skip; t->ckeep = keep;
        t->active = s_on;
        if (t->active) { t->skip_left = skip; t->keep = keep; }
        snprintf(t->what, sizeof t->what, "%s", m4a ? "m4a" : opus ? "opus" : "vorbis");
    }
    *h = t;
    return 0;
}
static int ogg_open(void **h, const pc_reader_t *r, uint32_t o) { return trim_open_with(h, r, o, &backend_ogg, false); }
static int m4a_open(void **h, const pc_reader_t *r, uint32_t o) { return trim_open_with(h, r, o, &backend_m4a, true); }

static int trim_render(void *h, float *out, int frames)
{
    trim_t *t = h;
    if (!t->active) { int n = t->in->render(t->h, out, frames); if (n > 0) t->got += (uint64_t)n; else t->last = n; return n; }
    while (t->skip_left) {                                                  /* the priming: decoded and dropped (out is a big enough scratch block) */
        int want = t->skip_left < (uint64_t)frames ? (int)t->skip_left : frames;
        int n = t->in->render(t->h, out, want);
        if (n <= 0) return n;
        t->got += (uint64_t)n; t->skip_left -= (uint64_t)n;
    }
    if (t->played >= t->keep) return 0;                                     /* the padding after the last real frame is never asked for */
    uint64_t left = t->keep - t->played;
    int n = t->in->render(t->h, out, left < (uint64_t)frames ? (int)left : frames);
    if (n <= 0) t->last = n;
    if (n > 0) { t->got += (uint64_t)n; t->played += (uint64_t)n; }
    return n;
}

static uint32_t trim_rate(void *h) { return ((trim_t *)h)->rate; }
static int32_t trim_len(void *h) { trim_t *t = h; return t->keep && t->rate ? (int32_t)(t->keep * 1000 / t->rate) : -1; }
static int32_t trim_pos(void *h)
{
    trim_t *t = h;
    if (!t->active) return t->in->position_ms(t->h);
    return t->rate ? (int32_t)(t->played * 1000 / t->rate) : 0;
}
static void trim_close(void *h)
{
    trim_t *t = h;
    if (t->what[0]) ESP_LOGI(TAG, "%s: container says skip %llu + play %llu frames; trimming %s; decoder gave %llu (ended with %d), handed on %llu", t->what,
                             (unsigned long long)t->skip, (unsigned long long)t->ckeep, t->active ? "on" : "off", (unsigned long long)t->got, t->last,
                             (unsigned long long)(t->active ? t->played : t->got));
    t->in->close(t->h);
    heap_caps_free(t);
}

#define OPS .render = trim_render, .rate = trim_rate, .length_ms = trim_len, .position_ms = trim_pos, .close = trim_close
const pc_backend_t backend_ogg_trim = { .open = ogg_open, OPS, .name = "OGG" };
const pc_backend_t backend_m4a_trim = { .open = m4a_open, OPS, .name = "M4A" };
