// SPDX-License-Identifier: LGPL-2.1-or-later
/* LHA depacker: the -lh5- method (LZSS with a 8 KiB window and static Huffman codes for the literals / lengths and the positions) and stored -lh0-.
 * Written for this project from the published description of the format (the algorithm of Haruhiko Okumura's public-domain ar002 / LHarc):
 * the bit stream is read most significant bit first; per block a 16-bit symbol count, a small Huffman code for the code lengths, the code
 * lengths of the literal/length alphabet (510 symbols) and of the position alphabet (14 symbols). The output buffer doubles as the dictionary.
 * Every number is untrusted: reads past the input give zero bits (and fail the stream when too many), a bad Huffman table or a match that
 * reaches before the start of the output ends the stream with an error, the output never grows past what the header declares. */
#include <stdlib.h>
#include <string.h>
#include "pc/pc_lh5.h"

#define NC 510                           /* 256 literals + 254 match lengths (3..256) */
#define NP 14                            /* position slots for a 13-bit window */
#define NT 19                            /* symbols of the code-length code */
#define CBIT 9
#define PBIT 4
#define TBIT 5
#define THRESHOLD 3

typedef uint16_t ush;
typedef uint8_t uch;

typedef struct {
    const uch *in;
    uint32_t n, pos;
    uint32_t bitbuf;                    /* the next 16 bits, MSB first, in the low 16 bits */
    unsigned subbit, bitcount;
    uint32_t over;                      /* bytes asked for past the end of the input */
    bool bad;
    ush left[2 * NC - 1], right[2 * NC - 1];
    uch c_len[NC], pt_len[NT > NP ? NT : NP];
    ush c_table[4096], pt_table[256];
    unsigned blocksize;
} lh_t;

static void fillbuf(lh_t *s, unsigned n)
{
    s->bitbuf = (s->bitbuf << n) & 0xFFFF;
    while (n > s->bitcount) {
        n -= s->bitcount;
        s->bitbuf |= s->subbit << n;
        if (s->pos < s->n) s->subbit = s->in[s->pos++]; else { s->subbit = 0; s->over++; }
        s->bitcount = 8;
    }
    s->bitcount -= n;
    s->bitbuf |= s->subbit >> s->bitcount;
    s->bitbuf &= 0xFFFF;
}

static unsigned getbits(lh_t *s, unsigned n)
{
    unsigned x = n ? (s->bitbuf >> (16 - n)) : 0;
    fillbuf(s, n);
    return x;
}

static void init_bits(lh_t *s)
{
    s->bitbuf = 0; s->subbit = 0; s->bitcount = 0;
    fillbuf(s, 16);
}

/* the decoding table of a canonical Huffman code (ar002's make_table): the first tablebits bits index `table`, longer codes continue in left/right */
static void make_table(lh_t *s, unsigned nchar, const uch *bitlen, unsigned tablebits, ush *table)
{
    ush count[17], weight[17], start[18], *p;
    unsigned i, k, len, ch, jutbits, avail, nextcode, mask;
    for (i = 1; i <= 16; i++) count[i] = 0;
    for (i = 0; i < nchar; i++) count[bitlen[i]]++;
    start[1] = 0;
    for (i = 1; i <= 16; i++) start[i + 1] = (ush)(start[i] + (count[i] << (16 - i)));
    if (start[17] != (ush)0) { s->bad = true; return; }            /* the code must fill the whole 16-bit space (1 << 16 wraps to 0) */
    jutbits = 16 - tablebits;
    for (i = 1; i <= tablebits; i++) { start[i] >>= jutbits; weight[i] = (ush)(1u << (tablebits - i)); }
    while (i <= 16) { weight[i] = (ush)(1u << (16 - i)); i++; }
    i = start[tablebits + 1] >> jutbits;
    if (i != 0) { k = 1u << tablebits; while (i != k) table[i++] = 0; }
    avail = nchar;
    mask = 1u << (15 - tablebits);
    for (ch = 0; ch < nchar; ch++) {
        if ((len = bitlen[ch]) == 0) continue;
        nextcode = (unsigned)start[len] + weight[len];
        if (len <= tablebits) {
            if (nextcode > (1u << tablebits)) { s->bad = true; return; }
            for (i = start[len]; i < nextcode; i++) table[i] = (ush)ch;
        } else {
            k = start[len];
            p = &table[k >> jutbits];
            i = len - tablebits;
            while (i != 0) {
                if (*p == 0) {
                    if (avail >= 2 * NC - 1) { s->bad = true; return; }
                    s->right[avail] = s->left[avail] = 0;
                    *p = (ush)avail++;
                }
                p = (k & mask) ? &s->right[*p] : &s->left[*p];
                k <<= 1;
                i--;
            }
            *p = (ush)ch;
        }
        start[len] = (ush)nextcode;
    }
}

/* the code lengths of a small alphabet (the code-length code, or the positions) */
static void read_pt_len(lh_t *s, unsigned nn, unsigned nbit, int i_special)
{
    int n = (int)getbits(s, nbit);
    if (n == 0) {
        unsigned c = getbits(s, nbit);
        memset(s->pt_len, 0, nn);
        for (unsigned i = 0; i < 256; i++) s->pt_table[i] = (ush)c;
        return;
    }
    if ((unsigned)n > nn) { s->bad = true; return; }
    int i = 0;
    while (i < n) {
        int c = (int)(s->bitbuf >> 13);
        if (c == 7) {
            unsigned mask = 1u << 12;
            while (mask && (s->bitbuf & mask)) { mask >>= 1; c++; }
            if (c > 16) { s->bad = true; return; }
        }
        fillbuf(s, c < 7 ? 3 : (unsigned)c - 3);
        s->pt_len[i++] = (uch)c;
        if (i == i_special) {
            int z = (int)getbits(s, 2);
            while (--z >= 0 && i < (int)nn) s->pt_len[i++] = 0;
        }
    }
    while (i < (int)nn) s->pt_len[i++] = 0;
    make_table(s, nn, s->pt_len, 8, s->pt_table);
}

static void read_c_len(lh_t *s)
{
    int n = (int)getbits(s, CBIT);
    if (n == 0) {
        unsigned c = getbits(s, CBIT);
        memset(s->c_len, 0, NC);
        for (unsigned i = 0; i < 4096; i++) s->c_table[i] = (ush)c;
        return;
    }
    if (n > NC) { s->bad = true; return; }
    int i = 0;
    while (i < n) {
        int c = s->pt_table[s->bitbuf >> 8];
        if (c >= NT) {
            unsigned mask = 1u << 7;
            do { c = (s->bitbuf & mask) ? s->right[c] : s->left[c]; mask >>= 1; } while (c >= NT && mask);
            if (c >= NT) { s->bad = true; return; }
        }
        fillbuf(s, s->pt_len[c]);
        if (c <= 2) {
            if (c == 0) c = 1;
            else if (c == 1) c = (int)getbits(s, 4) + 3;
            else c = (int)getbits(s, CBIT) + 20;
            while (--c >= 0 && i < NC) s->c_len[i++] = 0;
        } else s->c_len[i++] = (uch)(c - 2);
    }
    while (i < NC) s->c_len[i++] = 0;
    make_table(s, NC, s->c_len, 12, s->c_table);
}

static unsigned decode_c(lh_t *s)
{
    if (s->blocksize == 0) {
        s->blocksize = getbits(s, 16);
        read_pt_len(s, NT, TBIT, 3);
        read_c_len(s);
        read_pt_len(s, NP, PBIT, -1);
        if (s->bad || s->blocksize == 0) { s->bad = true; return NC; }
    }
    s->blocksize--;
    unsigned j = s->c_table[s->bitbuf >> 4];
    if (j >= NC) {
        unsigned mask = 1u << 3;
        do { j = (s->bitbuf & mask) ? s->right[j] : s->left[j]; mask >>= 1; } while (j >= NC && mask);
        if (j >= NC) { s->bad = true; return NC; }
    }
    fillbuf(s, s->c_len[j]);
    return j;
}

static unsigned decode_p(lh_t *s)
{
    unsigned j = s->pt_table[s->bitbuf >> 8];
    if (j >= NP) {
        unsigned mask = 1u << 7;
        do { j = (s->bitbuf & mask) ? s->right[j] : s->left[j]; mask >>= 1; } while (j >= NP && mask);
        if (j >= NP) { s->bad = true; return 0; }
    }
    fillbuf(s, s->pt_len[j]);
    if (j) j = (1u << (j - 1)) + getbits(s, j - 1);
    return j;
}

static uint32_t le32(const uch *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static uint32_t le16(const uch *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }

/* the header of the first entry: where its data starts, how long the packed data is, the unpacked size, the method (5 or 0); false when not usable */
static bool header(const uch *in, uint32_t n, uint32_t *off, uint32_t *packed, uint32_t *orig, int *method)
{
    if (n < 24 || in[2] != '-' || in[3] != 'l' || in[4] != 'h' || in[6] != '-') return false;
    if (in[5] != '5' && in[5] != '0') return false;
    uint32_t hs = in[0], level = in[20];
    if (level > 1 || hs < 20 || (uint64_t)hs + 2 > n) return false;
    uint32_t pos = hs + 2, pk = le32(in + 7), skipped = 0;
    if (level == 1) {                                                  /* extended headers follow: their sizes are part of the packed size */
        for (int guard = 0; guard < 64; guard++) {
            uint32_t ext = le16(in + pos - 2);
            if (!ext) break;
            if ((uint64_t)pos + ext > n) return false;
            pos += ext;
            skipped += ext;
        }
        if (skipped > pk) return false;
        pk -= skipped;
    }
    if (pos > n) return false;
    *off = pos; *packed = pk; *orig = le32(in + 11); *method = in[5] == '5' ? 5 : 0;
    return true;
}

bool pc_lh5_is_packed(const uint8_t *in, uint32_t n)
{
    uint32_t a, b, c;
    int m;
    return in && header(in, n, &a, &b, &c, &m);
}

int pc_lh5_unpack(const uint8_t *in, uint32_t n, uint32_t max_out, uint8_t **out, uint32_t *len)
{
    uint32_t off, packed, orig;
    int method;
    if (!in || !header(in, n, &off, &packed, &orig, &method)) return -1;
    if (orig > max_out || !orig) return -2;
    if (packed > n - off) packed = n - off;                            /* a lying size: what is there */
    uint8_t *o = malloc(orig);
    if (!o) return -2;
    if (method == 0) {
        if (packed < orig) { free(o); return -1; }
        memcpy(o, in + off, orig);
        *out = o; *len = orig;
        return 0;
    }
    lh_t *s = calloc(1, sizeof *s);                                    /* ~16 KB: a PSRAM block on the target */
    if (!s) { free(o); return -2; }
    s->in = in + off; s->n = packed;
    init_bits(s);
    uint32_t r = 0;
    while (r < orig && !s->bad) {
        unsigned c = decode_c(s);
        if (s->bad || s->over > 8) break;
        if (c < 256) o[r++] = (uint8_t)c;
        else if (c == NC) break;
        else {
            unsigned cnt = c - (256 - THRESHOLD);
            unsigned dist = decode_p(s) + 1;
            if (s->bad || dist > r) { s->bad = true; break; }
            for (unsigned k = 0; k < cnt && r < orig; k++, r++) o[r] = o[r - dist];
        }
    }
    bool ok = !s->bad && r == orig;
    free(s);
    if (!ok) { free(o); return -1; }
    *out = o; *len = orig;
    return 0;
}
