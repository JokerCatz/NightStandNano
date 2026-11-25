// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_id3.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "pc/pc_utf.h"

#define TEXT_MAX 4096     /* bytes of a text frame body we look at */
#define DESC_MAX 2048

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint32_t syncsafe(const uint8_t *p) { return (uint32_t)(p[0] & 0x7f) << 21 | (uint32_t)(p[1] & 0x7f) << 14 | (uint32_t)(p[2] & 0x7f) << 7 | (p[3] & 0x7f); }

/* removes unsynchronisation (FF 00 -> FF) in place; returns new length */
static uint32_t unsync(uint8_t *b, uint32_t n)
{
    uint32_t w = 0;
    for (uint32_t i = 0; i < n; i++) {
        b[w++] = b[i];
        if (b[i] == 0xFF && i + 1 < n && b[i + 1] == 0x00) i++;
    }
    return w;
}

/* length of an encoded string (up to its terminator) within n bytes */
static uint32_t str_len(uint8_t enc, const uint8_t *p, uint32_t n)
{
    if (enc == 1 || enc == 2) {
        uint32_t i = 0;
        while (i + 1 < n && (p[i] || p[i + 1])) i += 2;
        return i > n ? n : i;
    }
    uint32_t i = 0;
    while (i < n && p[i]) i++;
    return i;
}
static uint32_t term_len(uint8_t enc) { return (enc == 1 || enc == 2) ? 2 : 1; }

static void decode(uint8_t enc, const uint8_t *p, uint32_t n, char *dst, size_t cap)
{
    switch (enc) {
    case 1:
        if (n >= 2 && p[0] == 0xFF && p[1] == 0xFE) pc_utf16_to_utf8(p + 2, n - 2, false, dst, cap);
        else if (n >= 2 && p[0] == 0xFE && p[1] == 0xFF) pc_utf16_to_utf8(p + 2, n - 2, true, dst, cap);
        else pc_utf16_to_utf8(p, n, false, dst, cap);    /* no BOM: assume little-endian */
        break;
    case 2: pc_utf16_to_utf8(p, n, true, dst, cap); break;
    case 3: {
        size_t k = str_len(3, p, n);
        if (k > cap - 1) {                         /* clip on a character boundary */
            k = cap - 1;
            while (k > 0 && (p[k] & 0xC0) == 0x80) k--;
        }
        memcpy(dst, p, k);
        dst[k] = 0;
        break;
    }
    default: pc_latin1_to_utf8(p, n, dst, cap); break;
    }
}

/* text frame body (encoding byte + string) */
static void text(const uint8_t *b, uint32_t n, char *dst, size_t cap)
{
    if (n < 1) return;
    decode(b[0], b + 1, n - 1, dst, cap);
}

static int digits(const char *s, int count)
{
    int v = 0;
    for (int i = 0; i < count; i++) {
        if (s[i] < '0' || s[i] > '9') return -1;
        v = v * 10 + (s[i] - '0');
    }
    return v;
}

static uint32_t ymd(int y, int m, int d)
{
    if (y < 1000 || y > 2999) return 0;
    if (m < 1 || m > 12) m = 1, d = 1;
    if (d < 1 || d > 31) d = 1;
    return (uint32_t)(y * 10000 + m * 100 + d);
}

void pc_id3_free(pc_id3_t *t)
{
    free(t->desc);
    t->desc = NULL;
}

/* FLAC files keep their tags in metadata blocks (Vorbis comments, PICTURE); same fields as ID3 */
static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void date_from(const char *s, pc_id3_t *out)
{
    int y = digits(s, 4);
    if (y <= 0) return;
    int mm = s[4] == '-' ? digits(s + 5, 2) : -1;
    int dd = mm > 0 && s[7] == '-' ? digits(s + 8, 2) : -1;
    out->date = ymd(y, mm > 0 ? mm : 1, dd > 0 ? dd : 1);
}
static void vorbis_comments(const uint8_t *b, uint32_t n, pc_id3_t *out)
{
    if (n < 8) return;
    uint32_t p = 4 + le32(b);                        /* vendor string */
    if (p + 4 > n) return;
    uint32_t cnt = le32(b + p);
    p += 4;
    char v[256];
    for (uint32_t i = 0; i < cnt && p + 4 <= n; i++) {
        uint32_t l = le32(b + p);
        p += 4;
        if (l > n - p) break;
        const char *e = memchr(b + p, '=', l);
        if (e) {
            size_t kl = (size_t)(e - (const char *)(b + p)), vl = l - kl - 1;
            if (vl >= sizeof v) vl = sizeof v - 1;
            memcpy(v, e + 1, vl);
            v[vl] = 0;
            const char *k = (const char *)(b + p);
#define KEY(name) (kl == sizeof(name) - 1 && !strncasecmp(k, name, kl))
            if (KEY("TITLE") && !out->title[0]) pc_utf8_copy(out->title, sizeof out->title, v);
            else if (KEY("ARTIST") && !out->artist[0]) pc_utf8_copy(out->artist, sizeof out->artist, v);
            else if (KEY("ALBUM") && !out->album[0]) pc_utf8_copy(out->album, sizeof out->album, v);
            else if (KEY("COPYRIGHT") && !out->copyright[0]) pc_utf8_copy(out->copyright, sizeof out->copyright, v);
            else if ((KEY("PURL") || KEY("URL")) && !out->purl[0]) pc_utf8_copy(out->purl, sizeof out->purl, v);
            else if (KEY("TRACKNUMBER") && !out->track) out->track = (uint16_t)atoi(v);
            else if (KEY("DATE") && !out->date) date_from(v, out);
            else if ((KEY("DESCRIPTION") || KEY("COMMENT")) && !out->desc && (out->desc = malloc(DESC_MAX)))
                pc_utf8_copy(out->desc, DESC_MAX, v);
            else if (kl > 11 && kl < 24 && !strncasecmp(k, "REPLAYGAIN_", 11)) {
                char key[24];
                memcpy(key, k, kl);
                key[kl] = 0;
                pc_rg_tag(&out->rg, key, v);
            }
#undef KEY
        }
        p += l;
    }
}
static void flac_picture(const pc_reader_t *r, uint32_t at, uint32_t n, pc_id3_t *out)
{
    uint8_t h[8];
    if (out->apic_len || n < 32 || r->read(r->u, at, h, 8) != 8) return;
    uint32_t ml = be32(h + 4), p = 8;
    char mime[24] = "";
    if (ml >= sizeof mime || p + ml + 4 > n || r->read(r->u, at + p, mime, ml) != (int)ml) return;
    mime[ml] = 0;
    p += ml;
    if (r->read(r->u, at + p, h, 4) != 4) return;
    p += 4 + be32(h) + 16;                           /* description, width, height, depth, colours */
    if (p + 4 > n || r->read(r->u, at + p, h, 4) != 4) return;
    uint32_t dl = be32(h);
    p += 4;
    if ((strcasecmp(mime, "image/jpeg") && strcasecmp(mime, "image/jpg")) || dl > n - p) return;
    out->apic_off = at + p;
    out->apic_len = dl;
}
static int parse_flac(const pc_reader_t *r, pc_id3_t *out)
{
    uint32_t pos = 4;
    for (int guard = 0; guard < 64; guard++) {
        uint8_t bh[4];
        if (r->read(r->u, pos, bh, 4) != 4) return 0;
        uint32_t type = bh[0] & 0x7f, len = (uint32_t)bh[1] << 16 | (uint32_t)bh[2] << 8 | bh[3];
        pos += 4;
        if (pos + len > r->size) return 0;
        if (type == 4 && len <= 65536) {             /* VORBIS_COMMENT */
            uint8_t *b = malloc(len);
            if (b && r->read(r->u, pos, b, len) == (int)len) vorbis_comments(b, len, out);
            free(b);
        } else if (type == 6) flac_picture(r, pos, len, out);
        pos += len;
        if (bh[0] & 0x80) break;                     /* last metadata block */
    }
    return 0;
}

/* Ogg (Vorbis / Opus) files: the second packet holds the same Vorbis comments */
#define OGG_TAGS_MAX 65536
static int parse_ogg(const pc_reader_t *r, pc_id3_t *out)
{
    uint8_t *pkt = malloc(OGG_TAGS_MAX);
    if (!pkt) return 0;
    uint32_t pos = 0, plen = 0, serial = 0;
    int npkt = 0;
    bool first = true, done = false;
    for (int pages = 0; !done && pages < 64; pages++) {
        uint8_t h[27 + 255];
        if (r->read(r->u, pos, h, 27) != 27 || memcmp(h, "OggS", 4)) break;
        int segs = h[26];
        if (r->read(r->u, pos + 27, h + 27, (uint32_t)segs) != segs) break;
        uint32_t ser = le32(h + 14), at = pos + 27 + (uint32_t)segs, body = 0;
        if (first) serial = ser, first = false;
        for (int i = 0; i < segs; i++) body += h[27 + i];
        if (ser == serial) {
            uint32_t off = at;
            for (int i = 0; i < segs && !done; i++) {
                uint32_t sl = h[27 + i];
                if (npkt == 1 && sl) {                   /* collect the comment packet */
                    uint32_t k = plen + sl > OGG_TAGS_MAX ? OGG_TAGS_MAX - plen : sl;
                    if (k && r->read(r->u, off, pkt + plen, k) == (int)k) plen += k;
                }
                off += sl;
                if (sl < 255) { if (npkt == 1) done = true; npkt++; }   /* a lacing value < 255 ends the packet */
            }
        }
        pos = at + body;
    }
    if (plen > 7 && !memcmp(pkt, "\x03vorbis", 7)) vorbis_comments(pkt + 7, plen - 7, out);
    else if (plen > 8 && !memcmp(pkt, "OpusTags", 8)) vorbis_comments(pkt + 8, plen - 8, out);
    free(pkt);
    return 0;
}

/* MP4 / M4A: moov/udta/meta/ilst items, each with a 'data' box (iTunes tags) */
static int mp4_find(const pc_reader_t *r, uint32_t from, uint32_t to, const char *type, uint32_t *body, uint32_t *blen)
{
    uint8_t h[8];
    for (uint32_t p = from; p + 8 <= to;) {
        if (r->read(r->u, p, h, 8) != 8) return -1;
        uint32_t sz = be32(h);
        if (sz < 8 || sz > to - p) return -1;        /* 64-bit sizes (mdat only, in practice) are not walked into */
        if (!memcmp(h + 4, type, 4)) { *body = p + 8; *blen = sz - 8; return 0; }
        p += sz;
    }
    return -1;
}
static void mp4_item(const pc_reader_t *r, const uint8_t *name, uint32_t at, uint32_t n, pc_id3_t *out)
{
    uint32_t d, dl;
    char ff[40] = "";                                /* '----' freeform item: its 'name' (iTunes ReplayGain keys) */
    if (!memcmp(name, "----", 4)) {
        uint32_t nm, nl;
        if (mp4_find(r, at, at + n, "name", &nm, &nl) || nl <= 4 || nl - 4 >= sizeof ff || r->read(r->u, nm + 4, ff, nl - 4) != (int)(nl - 4)) return;
        ff[nl - 4] = 0;
    }
    if (mp4_find(r, at, at + n, "data", &d, &dl) || dl < 8) return;
    uint8_t hd[8];
    if (r->read(r->u, d, hd, 8) != 8) return;
    uint32_t type = be32(hd) & 0xffffff, vo = d + 8, vl = dl - 8;
    if (!memcmp(name, "covr", 4)) {
        if (type == 13 && !out->apic_len) { out->apic_off = vo; out->apic_len = vl; }   /* 13 = JPEG */
        return;
    }
    if (!memcmp(name, "trkn", 4)) {
        uint8_t t[4];
        if (vl >= 4 && r->read(r->u, vo, t, 4) == 4) out->track = (uint16_t)(t[2] << 8 | t[3]);
        return;
    }
    char v[256];
    if (type != 1 || !vl) return;                    /* 1 = UTF-8 text */
    if (vl >= sizeof v) vl = sizeof v - 1;
    if (r->read(r->u, vo, v, vl) != (int)vl) return;
    v[vl] = 0;
    if (ff[0]) { pc_rg_tag(&out->rg, ff, v); return; }
    if (!memcmp(name, "\251nam", 4)) pc_utf8_copy(out->title, sizeof out->title, v);
    else if (!memcmp(name, "\251ART", 4) || (!memcmp(name, "aART", 4) && !out->artist[0])) pc_utf8_copy(out->artist, sizeof out->artist, v);
    else if (!memcmp(name, "\251alb", 4)) pc_utf8_copy(out->album, sizeof out->album, v);
    else if (!memcmp(name, "cprt", 4)) pc_utf8_copy(out->copyright, sizeof out->copyright, v);
    else if (!memcmp(name, "\251day", 4)) date_from(v, out);
    else if ((!memcmp(name, "desc", 4) || !memcmp(name, "\251cmt", 4)) && !out->desc && (out->desc = malloc(DESC_MAX)))
        pc_utf8_copy(out->desc, DESC_MAX, v);
}
static int parse_mp4(const pc_reader_t *r, pc_id3_t *out)
{
    uint32_t mv, mvl, ud, udl, me, mel, il, ill;
    if (mp4_find(r, 0, r->size, "moov", &mv, &mvl) || mp4_find(r, mv, mv + mvl, "udta", &ud, &udl) ||
        mp4_find(r, ud, ud + udl, "meta", &me, &mel) || mel < 4 || mp4_find(r, me + 4, me + mel, "ilst", &il, &ill)) return 0;
    uint8_t h[8];
    for (uint32_t p = il; p + 8 <= il + ill;) {
        if (r->read(r->u, p, h, 8) != 8) break;
        uint32_t sz = be32(h);
        if (sz < 8 || sz > il + ill - p) break;
        mp4_item(r, h + 4, p + 8, sz - 8, out);
        p += sz;
    }
    return 0;
}

/* Tracker modules carry only a title (MOD/XM/S3M/IT: CP437/Latin-1, space or NUL padded). True when the
 * header is one; the artist stays empty so the library falls back to the folder name. */
static bool mod_sig(const uint8_t *s)
{
    static const char *const sigs[] = { "M.K.", "M!K!", "M&K!", "N.T.", "FLT4", "FLT8", "CD81", "OKTA", "OCTA" };
    for (unsigned i = 0; i < sizeof sigs / sizeof *sigs; i++) if (!memcmp(s, sigs[i], 4)) return true;
    bool d0 = s[0] >= '0' && s[0] <= '9', d1 = s[1] >= '0' && s[1] <= '9';
    return (d0 && !memcmp(s + 1, "CHN", 3)) || (d0 && d1 && !memcmp(s + 2, "CH", 2));
}
static bool parse_module(const pc_reader_t *r, pc_id3_t *out)
{
    uint8_t h[48], sig[4];
    int got = r->read(r->u, 0, h, sizeof h);
    if (got < 0) return false;
    const uint8_t *t = NULL;
    uint32_t n = 0;
    if (got >= 30 && !memcmp(h, "IMPM", 4)) t = h + 4, n = 26;
    else if (got >= 37 && !strncasecmp((const char *)h, "Extended Module: ", 17)) t = h + 17, n = 20;
    else if (got >= 48 && !memcmp(h + 44, "SCRM", 4)) t = h, n = 28;
    else if (got >= 20 && r->size >= 1084 && r->read(r->u, 1080, sig, 4) == 4 && mod_sig(sig)) t = h, n = 20;
    if (!t) return false;
    uint8_t b[32];
    uint32_t k = 0;
    while (k < n && t[k]) { b[k] = t[k] < 0x20 ? ' ' : t[k]; k++; }
    while (k && b[k - 1] == ' ') k--;
    uint32_t s = 0;
    while (s < k && b[s] == ' ') s++;
    pc_latin1_to_utf8(b + s, k - s, out->title, sizeof out->title);
    return true;
}

/* APEv2 tag (Musepack, TTA: at the end of the file, before an optional 128-byte ID3v1): text items, same fields as the Vorbis comments */
static void parse_ape(const pc_reader_t *r, pc_id3_t *out)
{
    uint32_t end = r->size;
    uint8_t f[32], t[3];
    if (end >= 128 + 32 && r->read(r->u, end - 128, t, 3) == 3 && !memcmp(t, "TAG", 3)) end -= 128;
    if (end < 32 || r->read(r->u, end - 32, f, 32) != 32 || memcmp(f, "APETAGEX", 8)) return;
    uint32_t size = le32(f + 12), count = le32(f + 16);      /* size counts the footer, not the header */
    if (size < 32 || size > end || size - 32 > 16384) return;
    uint32_t n = size - 32;
    uint8_t *b = malloc(n + 1);
    if (!b) return;
    if (n && r->read(r->u, end - size, b, n) != (int)n) { free(b); return; }
    uint32_t p = 0;
    char v[256];
    for (uint32_t i = 0; i < count && p + 9 <= n; i++) {
        uint32_t vl = le32(b + p), flags = le32(b + p + 4);
        const char *k = (const char *)b + p + 8;
        size_t kl = strnlen(k, n - p - 8);
        if (p + 8 + kl + 1 > n || vl > n - (p + 8 + kl + 1)) break;
        const uint8_t *val = b + p + 8 + kl + 1;
        p += 8 + (uint32_t)kl + 1 + vl;
        if (flags & 6) continue;                             /* binary or external: not text */
        size_t c = vl < sizeof v - 1 ? vl : sizeof v - 1;
        memcpy(v, val, c);
        v[c] = 0;
#define KEY(name) (kl == sizeof(name) - 1 && !strncasecmp(k, name, kl))
        if (KEY("TITLE") && !out->title[0]) pc_utf8_copy(out->title, sizeof out->title, v);
        else if (KEY("ARTIST") && !out->artist[0]) pc_utf8_copy(out->artist, sizeof out->artist, v);
        else if (KEY("ALBUM") && !out->album[0]) pc_utf8_copy(out->album, sizeof out->album, v);
        else if (KEY("COPYRIGHT") && !out->copyright[0]) pc_utf8_copy(out->copyright, sizeof out->copyright, v);
        else if (KEY("TRACK") && !out->track) out->track = (uint16_t)atoi(v);
        else if (KEY("YEAR") && !out->date) date_from(v, out);
        else if (KEY("COMMENT") && !out->desc && (out->desc = malloc(DESC_MAX))) pc_utf8_copy(out->desc, DESC_MAX, v);
        else if (kl > 11 && kl < 24 && !strncasecmp(k, "REPLAYGAIN_", 11)) {
            char key[24];
            memcpy(key, k, kl);
            key[kl] = 0;
            pc_rg_tag(&out->rg, key, v);
        }
#undef KEY
    }
    free(b);
}

/* AIFF / AIFF-C: NAME and AUTH text chunks, and an "ID3 " chunk holding a whole ID3v2 tag (parsed through a window onto the file). */
typedef struct { const pc_reader_t *base; uint32_t off; } window_t;
static int window_read(void *u, uint32_t off, void *buf, uint32_t n) { const window_t *w = u; return w->base->read(w->base->u, w->off + off, buf, n); }
static int parse_main(const pc_reader_t *r, pc_id3_t *out, int depth);   /* depth: 1 inside an AIFF "ID3 " chunk, where a nested FORM is not followed (stack, 20 bytes of file per level) */
static int parse_aiff(const pc_reader_t *r, pc_id3_t *out)
{
    uint8_t h[12], c[8];
    if (r->size < 12 || r->read(r->u, 0, h, 12) != 12 || (memcmp(h + 8, "AIFF", 4) && memcmp(h + 8, "AIFC", 4))) return 0;
    uint64_t o = 12;
    for (int k = 0; k < 64 && o + 8 <= r->size; k++) {
        if (r->read(r->u, (uint32_t)o, c, 8) != 8) break;
        uint32_t sz = be32(c + 4);
        uint32_t body = (uint32_t)o + 8;
        char v[128];
        if ((!memcmp(c, "NAME", 4) || !memcmp(c, "AUTH", 4)) && sz && (uint64_t)body + sz <= r->size) {
            uint32_t m = sz < sizeof v - 1 ? sz : (uint32_t)sizeof v - 1;
            if (r->read(r->u, body, v, m) == (int)m) {
                v[m] = 0;
                if (!memcmp(c, "NAME", 4) && !out->title[0]) pc_utf8_copy(out->title, sizeof out->title, v);
                else if (!memcmp(c, "AUTH", 4) && !out->artist[0]) pc_utf8_copy(out->artist, sizeof out->artist, v);
            }
        } else if (!memcmp(c, "ID3 ", 4) && sz > 10 && (uint64_t)body + sz <= r->size) {
            window_t w = { r, body };
            pc_reader_t sub = { .read = window_read, .size = sz, .u = &w };
            pc_id3_t t;
            parse_main(&sub, &t, 1);
            if (t.title[0]) {                                /* the ID3 tag wins over the plain chunks */
                free(out->desc);
                *out = t;
                if (out->apic_len) out->apic_off += body;
            } else pc_id3_free(&t);
        }
        o = (uint64_t)body + sz + (sz & 1);
    }
    return 0;
}

static int parse_main(const pc_reader_t *r, pc_id3_t *out, int depth)
{
    memset(out, 0, sizeof *out);
    pc_rg_clear(&out->rg);
    uint8_t h[10];
    int got = r->read(r->u, 0, h, 10);
    if (got < 0) return -1;
    if (got >= 4 && !memcmp(h, "fLaC", 4)) return parse_flac(r, out);
    if (got >= 4 && !memcmp(h, "FORM", 4)) return depth ? 0 : parse_aiff(r, out);
    if (got >= 4 && !memcmp(h, "OggS", 4)) return parse_ogg(r, out);
    if (got >= 8 && !memcmp(h + 4, "ftyp", 4)) return parse_mp4(r, out);
    if ((got < 3 || memcmp(h, "ID3", 3)) && parse_module(r, out)) return 0;
    if (got < 10 || memcmp(h, "ID3", 3) || h[3] < 2 || h[3] > 4 || (h[6] | h[7] | h[8] | h[9]) & 0x80) return 0;
    int ver = h[3];
    if (ver == 2) {                                  /* v2.2 uses 3-byte ids: only skip it */
        uint32_t end = 10 + syncsafe(h + 6);
        out->audio_off = end < r->size ? end : r->size;
        return 0;
    }
    bool tag_unsync = h[5] & 0x80;
    uint32_t end = 10 + syncsafe(h + 6) + ((ver == 4 && (h[5] & 0x10)) ? 10 : 0);
    if (end > r->size) end = r->size;
    out->audio_off = end;
    uint32_t pos = 10;
    if (h[5] & 0x40) {                               /* extended header */
        uint8_t e[4];
        if (r->read(r->u, pos, e, 4) != 4) return 0;
        pos += ver == 4 ? syncsafe(e) : 4 + be32(e);
    }
    char tyer[8] = "", tdat[8] = "", tdrc[16] = "", trck[16] = "";
    uint8_t *buf = malloc(TEXT_MAX);
    if (!buf) return 0;
    while (pos + 10 <= end) {
        uint8_t fh[10];
        if (r->read(r->u, pos, fh, 10) != 10) break;
        bool idok = true;
        for (int i = 0; i < 4; i++)
            if (!((fh[i] >= 'A' && fh[i] <= 'Z') || (fh[i] >= '0' && fh[i] <= '9'))) idok = false;
        if (!idok) break;                            /* padding or garbage */
        uint32_t size = ver == 4 ? syncsafe(fh + 4) : be32(fh + 4);
        if (size > end - pos - 10) break;            /* frame runs past the tag */
        uint32_t body = pos + 10;
        pos = body + size;
        uint8_t fmt = fh[9];
        bool f_unsync = tag_unsync || (ver == 4 && (fmt & 0x02));
        bool skip = ver == 4 ? (fmt & 0x0C) : (fmt & 0xC0);   /* compressed / encrypted */
        if (skip || size == 0) continue;
        if (ver == 4 && (fmt & 0x01)) {              /* data length indicator */
            if (size < 4) continue;
            body += 4;
            size -= 4;
        }
        if (!memcmp(fh, "APIC", 4)) {
            uint32_t n = size < 300 ? size : 300;
            if (f_unsync || (uint32_t)r->read(r->u, body, buf, n) != n || n < 4) continue;
            uint8_t enc = buf[0];
            uint32_t i = 1 + str_len(0, buf + 1, n - 1) + 1;          /* MIME, Latin-1 */
            if (i + 1 > n) continue;
            bool jpeg = !strncmp((char *)buf + 1, "image/jpeg", 10) || !strncmp((char *)buf + 1, "image/jpg", 9);
            i += 1;                                                    /* picture type */
            i += str_len(enc, buf + i, n - i) + term_len(enc);         /* description */
            if (i >= n || !jpeg) continue;
            if (!out->apic_len) { out->apic_off = body + i; out->apic_len = size - i; }
            continue;
        }
        bool want = !memcmp(fh, "TIT2", 4) || !memcmp(fh, "TPE1", 4) || !memcmp(fh, "TALB", 4) || !memcmp(fh, "TCOP", 4) ||
                    !memcmp(fh, "TRCK", 4) || !memcmp(fh, "TYER", 4) || !memcmp(fh, "TDAT", 4) || !memcmp(fh, "TDRC", 4) ||
                    !memcmp(fh, "TXXX", 4);
        if (!want) continue;
        uint32_t n = size < TEXT_MAX ? size : TEXT_MAX;
        if ((uint32_t)r->read(r->u, body, buf, n) != n) break;
        if (f_unsync) n = unsync(buf, n);
        if (!memcmp(fh, "TIT2", 4)) text(buf, n, out->title, sizeof out->title);
        else if (!memcmp(fh, "TPE1", 4)) text(buf, n, out->artist, sizeof out->artist);
        else if (!memcmp(fh, "TALB", 4)) text(buf, n, out->album, sizeof out->album);
        else if (!memcmp(fh, "TCOP", 4)) text(buf, n, out->copyright, sizeof out->copyright);
        else if (!memcmp(fh, "TRCK", 4)) text(buf, n, trck, sizeof trck);
        else if (!memcmp(fh, "TYER", 4)) text(buf, n, tyer, sizeof tyer);
        else if (!memcmp(fh, "TDAT", 4)) text(buf, n, tdat, sizeof tdat);
        else if (!memcmp(fh, "TDRC", 4)) text(buf, n, tdrc, sizeof tdrc);
        else if (n > 1) {                                              /* TXXX: desc + value */
            uint8_t enc = buf[0];
            uint32_t dl = str_len(enc, buf + 1, n - 1);
            char key[32];
            decode(enc, buf + 1, dl, key, sizeof key);
            uint32_t v = 1 + dl + term_len(enc);
            if (v > n) continue;
            if (!strcmp(key, "purl")) decode(enc, buf + v, n - v, out->purl, sizeof out->purl);
            else if (!strncasecmp(key, "replaygain_", 11)) {
                char val[32];
                decode(enc, buf + v, n - v, val, sizeof val);
                pc_rg_tag(&out->rg, key, val);
            }
            else if (!strcmp(key, "description") && !out->desc && (out->desc = malloc(DESC_MAX)))
                decode(enc, buf + v, n - v, out->desc, DESC_MAX);
        }
    }
    free(buf);
    if (trck[0]) out->track = (uint16_t)atoi(trck);
    int y = digits(tyer, 4);
    if (y > 0) {
        int dd = strlen(tdat) == 4 ? digits(tdat, 2) : -1, mm = strlen(tdat) == 4 ? digits(tdat + 2, 2) : -1;
        out->date = dd > 0 && mm > 0 ? ymd(y, mm, dd) : ymd(y, 1, 1);
    } else if ((y = digits(tdrc, 4)) > 0) {
        int mm = tdrc[4] == '-' ? digits(tdrc + 5, 2) : -1;
        int dd = mm > 0 && tdrc[7] == '-' ? digits(tdrc + 8, 2) : -1;
        out->date = ymd(y, mm > 0 ? mm : 1, dd > 0 ? dd : 1);
    }
    return 0;
}

/* Chip-style formats that carry their own text (CHIP2A): a song name from the file header. Printable ASCII only (the Amiga / Atari
 * texts are Latin-1 or worse): anything else becomes '?'. Returns false when the file is not one of them. */
static void chip_text(char *dst, size_t cap, const uint8_t *s, uint32_t n)
{
    char t[128];
    uint32_t k = 0;
    while (k < n && k < sizeof t - 1 && s[k]) { t[k] = s[k] >= 0x20 && s[k] < 0x7f ? (char)s[k] : '?'; k++; }
    while (k && t[k - 1] == ' ') k--;
    t[k] = 0;
    if (k) pc_utf8_copy(dst, cap, t);
}

static void parse_chip(const pc_reader_t *r, pc_id3_t *out)
{
    uint8_t h[16], nm[128];
    if (r->size < 16 || r->read(r->u, 0, h, 16) != 16) return;
    if ((!memcmp(h, "THX", 3) && h[3] < 3) || (!memcmp(h, "HVL", 3) && h[3] < 2)) {          /* HivelyTracker / AHX: the song name */
        uint32_t off = (uint32_t)(h[4] << 8 | h[5]);
        if (off < r->size) {
            uint32_t n = r->size - off < sizeof nm ? r->size - off : (uint32_t)sizeof nm;
            if ((uint32_t)r->read(r->u, off, nm, n) == n) chip_text(out->title, sizeof out->title, nm, n);
        }
    } else if ((!memcmp(h, "YM5!", 4) || !memcmp(h, "YM6!", 4)) && r->size > 40) {     /* Atari ST YM: name and author after the digidrums (a plain file; a packed one is not unpacked for a tag) */
        uint8_t hd[34];
        if (r->read(r->u, 0, hd, 34) == 34 && !memcmp(hd + 4, "LeOnArD!", 8)) {
            uint32_t pos = 34 + (uint32_t)(hd[32] << 8 | hd[33]);
            uint32_t nd = (uint32_t)(hd[20] << 8 | hd[21]);
            for (uint32_t i = 0; i < nd && i < 40 && pos + 4 < r->size; i++) {
                uint8_t l4[4];
                if (r->read(r->u, pos, l4, 4) != 4) { pos = r->size; break; }
                uint64_t np = (uint64_t)pos + 4 + ((uint32_t)l4[0] << 24 | (uint32_t)l4[1] << 16 | (uint32_t)l4[2] << 8 | l4[3]);
                pos = np > r->size ? r->size : (uint32_t)np;
            }
            if (pos < r->size) {
                uint32_t n = r->size - pos < sizeof nm ? r->size - pos : (uint32_t)sizeof nm;
                if ((uint32_t)r->read(r->u, pos, nm, n) == n) {
                    uint32_t a = 0;
                    while (a < n && nm[a]) a++;
                    chip_text(out->title, sizeof out->title, nm, a);
                    if (a + 1 < n) chip_text(out->artist, sizeof out->artist, nm + a + 1, n - a - 1);
                }
            }
        }
    } else if (!memcmp(h, "VAGp", 4)) {                       /* PlayStation VAG: a 16-byte name at 0x20 */
        if (r->size >= 0x30 && r->read(r->u, 0x20, nm, 16) == 16) chip_text(out->title, sizeof out->title, nm, 16);
    } else if (!memcmp(h, "pico-8 cartridge", 16)) {         /* a text cart: by convention the first two comment lines of the code are the title and "by <author>" */
        uint32_t n = r->size < 4096 ? r->size : 4096;
        char *b = malloc(n + 1);
        if (!b) return;
        if ((uint32_t)r->read(r->u, 0, b, n) == n) {
            b[n] = 0;
            char *c = strstr(b, "__lua__\n");
            for (int line = 0; c && line < 2; line++) {
                c = strchr(c, '\n');
                if (!c) break;
                c++;
                if (c[0] != '-' || c[1] != '-') break;
                const char *t = c + 2;
                while (*t == ' ') t++;
                const char *e = strchr(t, '\n');
                uint32_t tl = e ? (uint32_t)(e - t) : (uint32_t)strlen(t);
                if (tl && t[tl - 1] == '\r') tl--;
                if (line == 0) chip_text(out->title, sizeof out->title, (const uint8_t *)t, tl);
                else if (!strncasecmp(t, "by ", 3)) chip_text(out->artist, sizeof out->artist, (const uint8_t *)t + 3, tl - 3);
            }
        }
        free(b);
    }
}

int pc_id3_parse(const pc_reader_t *r, pc_id3_t *out)
{
    int rc = parse_main(r, out, 0);
    if (rc >= 0 && !out->title[0] && !out->artist[0]) parse_chip(r, out);
    if (rc >= 0 && !out->title[0] && !out->artist[0]) parse_ape(r, out);   /* Musepack / TTA keep an APEv2 tag at the end */
    return rc;
}
