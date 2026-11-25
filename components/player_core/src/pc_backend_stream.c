// SPDX-License-Identifier: LGPL-2.1-or-later
/* Streamed game audio read straight from the card, three small decoders (no vgmstream): CRI ADX (ADPCM type 3), Sony PlayStation VAG
 * (PS-ADPCM, mono "VAGp"), Nintendo BRSTM (DSP-ADPCM, 1-8 channels). Written for this project from the public format descriptions and
 * checked against ffmpeg's decoders bit for bit on the test files (tools/bench/chip_bench/stream_check.py); no third-party code is vendored.
 * Every number read from a file is untrusted: header fields are range checked, sizes are computed in 64 bits, every read is bounded by the
 * reader's size. The stream plays once (loop points are ignored: a game track's loop is its end here). More than 2 channels are folded to
 * stereo (even channels left, odd right). Resume token (byte_pos) = the PCM frame to continue from: decoding restarts from the start of
 * the stream and drops the frames before it (ADPCM is cheap: well under a second for a 5-minute file); a token further in than EXACT_UNITS jumps to a
 * run-up of SEEK_RUNUP_UNITS units before it instead, so open() stays bounded on an hours-long file. */
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "pc/pc_backend.h"

#define MAXCH 8
#define CACHE 256                       /* bytes cached per channel (units are read in sequence inside a channel) */
#define UNIT_MAX 32                     /* samples per unit and channel: ADX 32, VAG 28, DSP 14 */

#define EXACT_UNITS 16384u             /* a resume up to here decodes from the start (bit-exact; ADX: 11 s at 48 kHz, well under a second of decode) */
#define SEEK_RUNUP_UNITS 256u           /* beyond it: jump to this many units before the target and let the (stable) ADPCM filters settle (the history is zero there) */

typedef enum { K_ADX, K_VAG, K_DSP } kind_t;

typedef struct {
    pc_reader_t r;
    kind_t kind;
    uint32_t rate, total;               /* frames */
    int ch;
    uint32_t pos;                       /* frames handed out */
    uint32_t unit;                      /* next unit to decode */
    int16_t u[MAXCH][UNIT_MAX];
    int un, upos;                       /* samples in the decoded unit / consumed */
    int32_t hist1[MAXCH], hist2[MAXCH];
    uint32_t data_off, data_end;
    bool done;
    /* ADX */
    int adx_bsz, adx_c1, adx_c2;
    /* DSP (BRSTM) */
    uint32_t nblocks, bsz, bsamples, fsz, fpad;
    int16_t coef[MAXCH][16];
    struct { uint32_t off; uint16_t len; uint8_t b[CACHE]; } cache[MAXCH];
} st_t;

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint16_t be16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }
static int16_t clamp16(int64_t v) { return v > 32767 ? 32767 : v < -32768 ? -32768 : (int16_t)v; }
static int snib(int v) { return v >= 8 ? v - 16 : v; }

/* n bytes at absolute offset `off` for channel c through its small cache; NULL when outside the file */
static const uint8_t *bytes(st_t *s, int c, uint32_t off, uint32_t n)
{
    if ((uint64_t)off + n > s->r.size || n > CACHE) return NULL;
    if (!(s->cache[c].len && off >= s->cache[c].off && off + n <= (uint64_t)s->cache[c].off + s->cache[c].len)) {
        uint32_t want = s->r.size - off < CACHE ? s->r.size - off : CACHE;
        int got = s->r.read(s->r.u, off, s->cache[c].b, want);
        if (got < (int)n) { s->cache[c].len = 0; return NULL; }
        s->cache[c].off = off;
        s->cache[c].len = (uint16_t)got;
    }
    return s->cache[c].b + (off - s->cache[c].off);
}

/* ---- ADX ------------------------------------------------------------------------------------------------------------------- */

static int adx_parse(st_t *s)
{
    uint8_t h[32];
    if (s->r.size < 36 || s->r.read(s->r.u, 0, h, 32) != 32 || h[0] != 0x80 || h[1] != 0) return -1;
    uint32_t cr = be16(h + 2);
    if (h[4] != 3 || h[6] != 4) return -2;                               /* type 3 ADPCM, 4-bit: the others (fixed coefficients, exponential, AHX) are not played */
    int bsz = h[5], ch = h[7];
    uint32_t rate = be32(h + 8), total = be32(h + 12);
    if (h[19] & 8) return -2;                                            /* encrypted (v4 flags): it would play as noise */
    if (bsz != 18 || ch < 1 || ch > MAXCH || rate < 4000 || rate > 192000 || !total) return -1;
    uint8_t sig[6];
    if ((uint64_t)cr + 4 > s->r.size || cr < 6 || s->r.read(s->r.u, cr - 2, sig, 6) != 6 || memcmp(sig, "(c)CRI", 6)) return -1;
    s->data_off = cr + 4;
    s->adx_bsz = bsz;
    s->ch = ch;
    s->rate = rate;
    double fc = be16(h + 16), a = M_SQRT2 - cos(2.0 * M_PI * fc / rate), b = M_SQRT2 - 1.0, c = (a - sqrt((a + b) * (a - b))) / b;
    s->adx_c1 = (int)lround(c * 8192.0);                                 /* rounded to nearest, as ffmpeg's decoder does (floor and truncation each differ on some rates) */
    s->adx_c2 = (int)lround(-(c * c) * 4096.0);
    uint64_t units = (uint64_t)(s->r.size - s->data_off) / ((uint64_t)bsz * ch), have = units * (uint64_t)((bsz - 2) * 2);
    s->total = total < have ? total : (uint32_t)have;                    /* a file shorter than its header says */
    return s->total ? 0 : -1;
}

static bool adx_unit(st_t *s)
{
    const int spb = (s->adx_bsz - 2) * 2;
    for (int c = 0; c < s->ch; c++) {
        const uint8_t *p = bytes(s, c, s->data_off + (uint32_t)(((uint64_t)s->unit * s->ch + (uint32_t)c) * (uint32_t)s->adx_bsz), (uint32_t)s->adx_bsz);
        if (!p) return false;
        int scale = (p[0] << 8 | p[1]) & 0x1fff;
        int32_t h1 = s->hist1[c], h2 = s->hist2[c];
        for (int i = 0; i < spb && i < UNIT_MAX; i++) {
            int nib = snib(i & 1 ? p[2 + i / 2] & 15 : p[2 + i / 2] >> 4);
            int32_t v = clamp16(nib * scale + ((s->adx_c1 * h1 + s->adx_c2 * h2) >> 12));
            s->u[c][i] = (int16_t)v;
            h2 = h1;
            h1 = v;
        }
        s->hist1[c] = h1;
        s->hist2[c] = h2;
    }
    s->un = spb < UNIT_MAX ? spb : UNIT_MAX;
    return true;
}

/* ---- PlayStation VAG ------------------------------------------------------------------------------------------------------- */

static int vag_parse(st_t *s)
{
    uint8_t h[48];
    if (s->r.size < 64 || s->r.read(s->r.u, 0, h, 48) != 48 || memcmp(h, "VAGp", 4)) return -1;
    uint32_t size = be32(h + 12), rate = be32(h + 16);
    if (rate < 4000 || rate > 192000 || size < 16) return -1;
    s->data_off = 48;
    s->data_end = (uint64_t)48 + size > s->r.size ? s->r.size : 48 + size;   /* the size field is untrusted: never past the file */
    s->ch = 1;
    s->rate = rate;
    s->total = (uint32_t)(((uint64_t)(s->data_end - s->data_off) / 16) * 28);
    return s->total ? 0 : -1;
}

static bool vag_unit(st_t *s)
{
    static const int f0[5] = { 0, 60, 115, 98, 122 }, f1[5] = { 0, 0, -52, -55, -60 };
    uint32_t off = s->data_off + s->unit * 16;
    if ((uint64_t)off + 16 > s->data_end) return false;
    const uint8_t *p = bytes(s, 0, off, 16);
    if (!p) return false;
    int shift = p[0] & 15, pred = p[0] >> 4;
    if (pred > 4) pred = 0;
    if (p[1] == 7) {                                         /* flag 7: end and mute: the frame is silence (as ffmpeg's decoder) */
        memset(s->u[0], 0, 28 * sizeof s->u[0][0]);
        s->hist1[0] = s->hist2[0] = 0;
        s->un = 28;
        return true;
    }
    int32_t h1 = s->hist1[0], h2 = s->hist2[0];
    for (int i = 0; i < 28; i++) {
        int nib = snib(i & 1 ? p[2 + i / 2] >> 4 : p[2 + i / 2] & 15);
        int32_t v = clamp16(((nib * 4096) >> shift) + (f0[pred] * h1 + f1[pred] * h2) / 64);   /* nibble << 12 >> shift; the filter term truncates toward zero (as ffmpeg's decoder) */
        s->u[0][i] = (int16_t)v;
        h2 = h1;
        h1 = v;
    }
    s->hist1[0] = h1;
    s->hist2[0] = h2;
    s->un = 28;
    return true;
}

/* ---- Nintendo BRSTM (DSP-ADPCM) --------------------------------------------------------------------------------------------- */

static int dsp_parse(st_t *s)
{
    uint8_t h[0x40];
    if (s->r.size < 0x100 || s->r.read(s->r.u, 0, h, 0x40) != 0x40 || memcmp(h, "RSTM", 4) || h[4] != 0xFE || h[5] != 0xFF) return -1;
    uint32_t ho = be32(h + 0x10), hs = be32(h + 0x14);
    uint8_t c[0x50];
    if (ho < 0x20 || (uint64_t)ho + 0x58 > s->r.size || hs < 0x58 || s->r.read(s->r.u, ho, c, 12) != 12 || memcmp(c, "HEAD", 4)) return -1;
    const uint32_t base = ho + 8;                                         /* offsets inside HEAD count from here */
    uint8_t t[24];
    if (s->r.read(s->r.u, base, t, 24) != 24) return -1;
    uint32_t o1 = be32(t + 4), o3 = be32(t + 20);                         /* [0]: stream info, [1]: tracks, [2]: channels: (marker, offset) pairs */
    uint8_t si[0x30];
    if ((uint64_t)base + o1 + sizeof si > s->r.size || s->r.read(s->r.u, base + o1, si, sizeof si) != (int)sizeof si) return -1;
    int codec = si[0], ch = si[2];
    if (ch < 1 || ch > MAXCH) return -1;
    if (codec != 2) return -2;                                            /* PCM8 / PCM16 BRSTM: not played */
    s->rate = be16(si + 4);
    s->total = be32(si + 12);
    s->data_off = be32(si + 0x10);
    s->nblocks = be32(si + 0x14);
    s->bsz = be32(si + 0x18);
    s->fsz = be32(si + 0x20);
    s->fpad = be32(si + 0x28);
    s->bsamples = be32(si + 0x1c);
    s->ch = ch;
    if (s->rate < 4000 || s->rate > 192000 || !s->total || !s->nblocks || s->nblocks > (1u << 20) || s->bsz < 8 || s->bsz % 8 || s->bsz > (1u << 20) ||
        s->fsz > s->bsz || s->fpad > s->bsz || s->fpad < s->fsz || s->data_off >= s->r.size) return -1;
    uint8_t cn[4];
    if ((uint64_t)base + o3 + 4 > s->r.size || s->r.read(s->r.u, base + o3, cn, 4) != 4 || cn[0] != ch) return -1;
    for (int k = 0; k < ch; k++) {                                        /* channel k: (marker, offset) -> (marker, offset) -> 16 coefficients */
        uint8_t e[8], coef[32];
        if ((uint64_t)base + o3 + 4 + 8 * (uint64_t)k + 8 > s->r.size || s->r.read(s->r.u, base + o3 + 4 + 8 * (uint32_t)k, e, 8) != 8) return -1;
        uint32_t p1 = base + be32(e + 4);
        if ((uint64_t)p1 + 8 > s->r.size || s->r.read(s->r.u, p1, e, 8) != 8) return -1;
        uint32_t p2 = base + be32(e + 4);
        if ((uint64_t)p2 + 32 > s->r.size || s->r.read(s->r.u, p2, coef, 32) != 32) return -1;
        for (int i = 0; i < 16; i++) s->coef[k][i] = (int16_t)be16(coef + 2 * i);
    }
    uint64_t span = (uint64_t)(s->nblocks - 1) * s->bsz * (uint32_t)ch + (uint64_t)s->fpad * (uint32_t)ch;      /* the audio must fit in the file */
    if ((uint64_t)s->data_off + span > s->r.size) {
        uint64_t have = (uint64_t)s->r.size - s->data_off, full = have / ((uint64_t)s->bsz * (uint32_t)ch);
        if (full == 0) return -1;
        s->nblocks = (uint32_t)full + 1;                                  /* a cut file: the whole blocks that are there (the last one counts as full) */
        s->fsz = s->fpad = s->bsz;
    }
    uint64_t avail = (uint64_t)(s->nblocks - 1) * (s->bsz / 8) * 14 + (uint64_t)(s->fsz / 8) * 14;
    if (s->total > avail) s->total = (uint32_t)avail;
    return 0;
}

static bool dsp_unit(st_t *s)
{
    const uint32_t fpb = s->bsz / 8;
    uint32_t b = s->unit / fpb, fi = s->unit % fpb;
    if (b >= s->nblocks) return false;
    const bool last = b == s->nblocks - 1;
    if (last && fi >= s->fsz / 8) return false;
    for (int c = 0; c < s->ch; c++) {
        uint64_t off = (uint64_t)s->data_off + (uint64_t)b * s->bsz * (uint32_t)s->ch + (uint64_t)c * (last ? s->fpad : s->bsz) + (uint64_t)fi * 8;
        if (off + 8 > 0xFFFFFFFFull) return false;
        const uint8_t *p = bytes(s, c, (uint32_t)off, 8);
        if (!p) return false;
        int scale = 1 << (p[0] & 15), idx = (p[0] >> 4) & 7;
        int c1 = s->coef[c][idx * 2], c2 = s->coef[c][idx * 2 + 1];
        int32_t h1 = s->hist1[c], h2 = s->hist2[c];
        for (int i = 0; i < 14; i++) {
            int nib = snib(i & 1 ? p[1 + i / 2] & 15 : p[1 + i / 2] >> 4);
            int32_t v = clamp16((((int64_t)nib * scale * 2048) + 1024 + (int64_t)c1 * h1 + (int64_t)c2 * h2) >> 11);   /* 64-bit: hostile coefficients overflowed int32 */
            s->u[c][i] = (int16_t)v;
            h2 = h1;
            h1 = v;
        }
        s->hist1[c] = h1;
        s->hist2[c] = h2;
    }
    s->un = 14;
    return true;
}

/* ---- the backend ------------------------------------------------------------------------------------------------------------ */

static int st_open(st_t **out, const pc_reader_t *r, uint32_t start_off, kind_t k)
{
    st_t *s = calloc(1, sizeof *s);                         /* ~3 KB: not a small (internal-RAM) block */
    if (!s) return -1;
    s->r = *r;
    if (s->r.u == r) s->r.u = &s->r;                         /* memory readers point at themselves */
    s->kind = k;
    int rc = k == K_ADX ? adx_parse(s) : k == K_VAG ? vag_parse(s) : dsp_parse(s);
    if (rc) { free(s); return -1; }
    *out = s;
    uint32_t skip = start_off < s->total ? start_off : 0;     /* resume: decode and drop (ADPCM history needs the run-up) */
    const uint32_t ups = k == K_ADX ? (uint32_t)((s->adx_bsz - 2) * 2) : k == K_VAG ? 28u : 14u;   /* samples per unit and channel */
    if (skip / ups > EXACT_UNITS) {                           /* wave-6 #19: far into a long file the run-up from the start is too slow for open(): start a few units before it, history zero */
        const uint32_t u0 = skip / ups - SEEK_RUNUP_UNITS;
        s->unit = u0;
        s->pos = u0 * ups;
        skip -= u0 * ups;
    }
    while (skip && !s->done) {
        if (s->upos >= s->un) {
            bool ok = k == K_ADX ? adx_unit(s) : k == K_VAG ? vag_unit(s) : dsp_unit(s);
            if (!ok) { s->done = true; break; }
            s->unit++;
            s->upos = 0;
        }
        uint32_t n = (uint32_t)(s->un - s->upos);
        if (n > skip) n = skip;
        s->upos += (int)n;
        s->pos += n;
        skip -= n;
    }
    return 0;
}

static int st_render(void *h, float *out, int frames)
{
    st_t *s = h;
    int done = 0;
    while (done < frames && s->pos < s->total) {
        if (s->upos >= s->un) {
            bool ok = s->kind == K_ADX ? adx_unit(s) : s->kind == K_VAG ? vag_unit(s) : dsp_unit(s);
            if (!ok) { s->total = s->pos; break; }           /* a short read ends the stream */
            s->unit++;
            s->upos = 0;
        }
        int n = s->un - s->upos;
        if (n > frames - done) n = frames - done;
        if ((uint32_t)n > s->total - s->pos) n = (int)(s->total - s->pos);
        for (int i = 0; i < n; i++) {
            int32_t l, rr;
            if (s->ch == 1) l = rr = s->u[0][s->upos + i];
            else if (s->ch == 2) { l = s->u[0][s->upos + i]; rr = s->u[1][s->upos + i]; }
            else {
                int sl = 0, sr = 0, nl = 0, nr = 0;
                for (int c = 0; c < s->ch; c++) { if (c & 1) { sr += s->u[c][s->upos + i]; nr++; } else { sl += s->u[c][s->upos + i]; nl++; } }
                l = sl / nl;
                rr = sr / nr;
            }
            out[2 * (done + i)] = (float)l * (1.0f / 32768.0f);
            out[2 * (done + i) + 1] = (float)rr * (1.0f / 32768.0f);
        }
        s->upos += n;
        s->pos += (uint32_t)n;
        done += n;
    }
    return done;
}

static uint32_t st_rate(void *h) { return ((st_t *)h)->rate; }
static int32_t st_len(void *h) { st_t *s = h; return (int32_t)((uint64_t)s->total * 1000 / s->rate); }
static int32_t st_pos(void *h) { st_t *s = h; return (int32_t)((uint64_t)s->pos * 1000 / s->rate); }
static uint32_t st_bytepos(void *h) { return ((st_t *)h)->pos; }
static void st_close(void *h) { free(h); }
static int adx_open(void **h, const pc_reader_t *r, uint32_t o) { return st_open((st_t **)h, r, o, K_ADX); }
static int vag_open(void **h, const pc_reader_t *r, uint32_t o) { return st_open((st_t **)h, r, o, K_VAG); }
static int dsp_open(void **h, const pc_reader_t *r, uint32_t o) { return st_open((st_t **)h, r, o, K_DSP); }

const pc_backend_t pc_backend_adx = { .open = adx_open, .render = st_render, .rate = st_rate, .length_ms = st_len, .position_ms = st_pos,
                                      .close = st_close, .byte_pos = st_bytepos, .name = "ADX" };
const pc_backend_t pc_backend_vag = { .open = vag_open, .render = st_render, .rate = st_rate, .length_ms = st_len, .position_ms = st_pos,
                                      .close = st_close, .byte_pos = st_bytepos, .name = "VAG" };
const pc_backend_t pc_backend_brstm = { .open = dsp_open, .render = st_render, .rate = st_rate, .length_ms = st_len, .position_ms = st_pos,
                                        .close = st_close, .byte_pos = st_bytepos, .name = "BRSTM" };

/* why a stream file is refused (0 = no known reason): 1 an ADX that is not type 3 ADPCM, 2 a BRSTM that is not DSP-ADPCM */
int pc_stream_check(const pc_reader_t *r, const char *ext)
{
    st_t *s = calloc(1, sizeof *s);
    if (!s) return 0;
    s->r = *r;
    if (s->r.u == r) s->r.u = &s->r;
    int rc = !strcasecmp(ext, ".adx") ? adx_parse(s) : !strcasecmp(ext, ".brstm") ? dsp_parse(s) : 0;
    free(s);
    return rc == -2 ? (!strcasecmp(ext, ".adx") ? 1 : 2) : 0;
}
