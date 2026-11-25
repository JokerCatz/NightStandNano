// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_zip.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "pc_inflate.h"

static uint16_t le16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }
static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

static int cmp(const void *a, const void *b) { return strcmp(((const pc_zip_entry_t *)a)->name, ((const pc_zip_entry_t *)b)->name); }

/* entries and their names share one allocation (large -> PSRAM on the device, not thousands of small blocks) */
void pc_zip_free(pc_zip_entry_t *e, int n)
{
    (void)n;
    free(e);
}

int pc_zip_open(const pc_reader_t *r, pc_zip_entry_t **out, int *n)
{
    if (!out || !n) return -EINVAL;
    *out = NULL;
    *n = 0;
    if (!r || !r->read || r->size < 22) return -EINVAL;
    uint32_t tail = r->size < 65557 ? r->size : 65557;   /* EOCD + longest comment */
    uint8_t *b = malloc(tail);
    if (!b) return -ENOMEM;
    if ((uint32_t)r->read(r->u, r->size - tail, b, tail) != tail) { free(b); return -EIO; }
    int at = -1;
    for (int i = (int)tail - 22; i >= 0; i--)
        if (b[i] == 'P' && b[i + 1] == 'K' && b[i + 2] == 5 && b[i + 3] == 6) { at = i; break; }
    if (at < 0) { free(b); return -EINVAL; }
    uint32_t count = le16(b + at + 10), cd_size = le32(b + at + 12), cd_off = le32(b + at + 16);
    free(b);
    if ((uint64_t)cd_off + cd_size > r->size || cd_size > 64u << 20) return -EINVAL;
    if ((uint64_t)count * 46 > cd_size) return -EINVAL;           /* every entry is at least 46 bytes: a bigger count is a lie */
    uint8_t *cd = malloc(cd_size ? cd_size : 1);
    if (!cd) return -ENOMEM;
    if ((uint32_t)r->read(r->u, cd_off, cd, cd_size) != cd_size) { free(cd); return -EIO; }
    pc_zip_entry_t *e = calloc(1, (size_t)(count ? count : 1) * sizeof *e + cd_size + 1);
    if (!e) { free(cd); return -ENOMEM; }
    char *names = (char *)(e + (count ? count : 1));   /* names are shorter than the directory they came from */
    uint32_t p = 0;
    int k = 0, rc = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (p + 46 > cd_size || le32(cd + p) != 0x02014b50) { rc = -EINVAL; break; }
        uint16_t nl = le16(cd + p + 28), xl = le16(cd + p + 30), cl = le16(cd + p + 32);
        if (p + 46u + nl + xl + cl > cd_size) { rc = -EINVAL; break; }
        uint32_t usize = le32(cd + p + 24);
        if (nl && cd[p + 46 + nl - 1] != '/') {                  /* skip directory entries */
            pc_zip_entry_t *z = &e[k];
            z->name = names;
            memcpy(names, cd + p + 46, nl);
            names[nl] = 0;
            names += nl + 1u;
            z->method = le16(cd + p + 10);
            z->crc = le32(cd + p + 16);
            z->csize = le32(cd + p + 20);
            z->usize = usize;
            z->local_off = le32(cd + p + 42);
            if ((uint64_t)z->local_off + 30 > r->size || z->csize > r->size) { rc = -EINVAL; break; }   /* data that cannot lie inside the file */
            k++;
        }
        p += 46u + nl + xl + cl;
    }
    free(cd);
    if (rc) { pc_zip_free(e, k); return rc; }
    qsort(e, (size_t)k, sizeof *e, cmp);
    *out = e;
    *n = k;
    return 0;
}

const pc_zip_entry_t *pc_zip_find(const pc_zip_entry_t *e, int n, const char *name)
{
    if (!e || n <= 0 || !name) return NULL;
    pc_zip_entry_t key = { .name = (char *)name };
    return bsearch(&key, e, (size_t)n, sizeof *e, cmp);
}

const pc_zip_entry_t *pc_zip_at(const pc_zip_entry_t *e, int n, int idx)
{
    return e && idx >= 0 && idx < n ? &e[idx] : NULL;
}

int pc_zip_data_offset(const pc_reader_t *r, const pc_zip_entry_t *e, uint32_t *off)
{
    uint8_t h[30];
    if (!r || !r->read || !e || !off) return -EINVAL;
    if ((uint64_t)e->local_off + 30 > r->size) return -EINVAL;
    if (r->read(r->u, e->local_off, h, 30) != 30 || le32(h) != 0x04034b50) return -EINVAL;
    uint64_t o = (uint64_t)e->local_off + 30 + le16(h + 26) + le16(h + 28);
    if (o + e->csize > r->size) return -EINVAL;
    *off = (uint32_t)o;
    return 0;
}

int pc_zip_extract(const pc_reader_t *r, const pc_zip_entry_t *e, uint8_t **out, uint32_t *len)
{
    if (!out || !len) return -EINVAL;
    *out = NULL;
    *len = 0;
    if (!r || !e) return -EINVAL;
    if (e->usize > PC_ZIP_MAX_EXTRACT) return -E2BIG;
    if (e->csize > PC_ZIP_MAX_EXTRACT + (64u << 10)) return -E2BIG;   /* a stored or deflated entry is never much bigger than what it holds */
    if (e->method != 0 && e->method != 8) return -ENOTSUP;
    uint32_t off;
    int rc = pc_zip_data_offset(r, e, &off);
    if (rc) return rc;
    uint8_t *dst = malloc(e->usize + 1u);
    if (!dst) return -ENOMEM;
    if (e->method == 0) {
        if (e->csize != e->usize || (uint32_t)r->read(r->u, off, dst, e->usize) != e->usize) { free(dst); return -EBADMSG; }
    } else {
        uint8_t *src = malloc(e->csize ? e->csize : 1);
        if (!src) { free(dst); return -ENOMEM; }
        long got = (uint32_t)r->read(r->u, off, src, e->csize) == e->csize ? pc_inflate_raw(src, e->csize, dst, e->usize) : -1;
        free(src);
        if (got != (long)e->usize) { free(dst); return -EBADMSG; }
    }
    if (pc_crc32(0, dst, e->usize) != e->crc) { free(dst); return -EBADMSG; }
    dst[e->usize] = 0;
    *out = dst;
    *len = e->usize;
    return 0;
}

int pc_gunzip(const uint8_t *in, uint32_t n, uint8_t **out, uint32_t *len)
{
    if (!out || !len) return -EINVAL;
    *out = NULL;
    *len = 0;
    if (!in || n < 18 || in[0] != 0x1f || in[1] != 0x8b || in[2] != 8) return -EINVAL;
    uint8_t fl = in[3];
    uint32_t p = 10;
    if (fl & 4) { if (p + 2 > n) return -EINVAL; p += 2u + le16(in + p); }
    if (fl & 8) { while (p < n && in[p]) p++; p++; }
    if (fl & 16) { while (p < n && in[p]) p++; p++; }
    if (fl & 2) p += 2;
    if (p + 8 > n) return -EINVAL;
    uint32_t crc = le32(in + n - 8), isize = le32(in + n - 4);
    if (isize > PC_ZIP_MAX_EXTRACT) return -E2BIG;
    uint8_t *dst = malloc(isize + 1u);
    if (!dst) return -ENOMEM;
    long got = pc_inflate_raw(in + p, n - 8 - p, dst, isize);
    if (got != (long)isize || pc_crc32(0, dst, isize) != crc) { free(dst); return -EBADMSG; }
    *out = dst;
    *len = isize;
    return 0;
}

/* the end record, searched backwards in windows (it may be followed by a comment up to 64 KB) */
static int find_eocd(const pc_reader_t *r, uint8_t eocd[22])
{
    uint8_t b[512];
    if (r->size < 22) return -EINVAL;
    uint32_t lo = r->size > 65557 ? r->size - 65557 : 0, end = r->size;   /* search [lo, end) for a record start */
    while (end - lo >= 22) {
        uint32_t start = end - lo > sizeof b ? end - (uint32_t)sizeof b : lo;
        uint32_t n = end - start;
        if ((uint32_t)r->read(r->u, start, b, n) != n) return -EIO;
        for (int i = (int)n - 22; i >= 0; i--)
            if (b[i] == 'P' && b[i + 1] == 'K' && b[i + 2] == 5 && b[i + 3] == 6) { memcpy(eocd, b + i, 22); return 0; }
        if (start == lo) break;
        end = start + 21;                          /* overlap: a record across the window edge */
    }
    return -EINVAL;
}

int pc_zip_lookup(const pc_reader_t *r, const char *const *names, int n, pc_zip_entry_t *out)
{
    if (!r || !r->read || !names || !out || n <= 0) return -EINVAL;
    for (int i = 0; i < n; i++) memset(&out[i], 0, sizeof out[i]);
    uint8_t eocd[22], h[46 + 255];
    int rc = find_eocd(r, eocd);
    if (rc) return rc;
    uint32_t count = le16(eocd + 10), cd_size = le32(eocd + 12), cd_off = le32(eocd + 16);
    if ((uint64_t)cd_off + cd_size > r->size || (uint64_t)count * 46 > cd_size) return -EINVAL;
    uint32_t p = 0;
    int found = 0;
    for (uint32_t k = 0; k < count && found < n; k++) {
        if (p + 46 > cd_size) return -EINVAL;
        uint32_t want = cd_size - p < sizeof h ? cd_size - p : (uint32_t)sizeof h;
        if ((uint32_t)r->read(r->u, cd_off + p, h, want) != want || le32(h) != 0x02014b50) return -EINVAL;
        uint32_t nl = le16(h + 28), xl = le16(h + 30), cl = le16(h + 32);
        if (p + 46u + nl + xl + cl > cd_size) return -EINVAL;
        for (int i = 0; nl <= 255 && i < n; i++) {
            if (out[i].name || strlen(names[i]) != nl || memcmp(h + 46, names[i], nl)) continue;
            out[i] = (pc_zip_entry_t){ .name = (char *)names[i], .method = le16(h + 10), .crc = le32(h + 16),
                                       .csize = le32(h + 20), .usize = le32(h + 24), .local_off = le32(h + 42) };
            found++;
            break;
        }
        p += 46u + nl + xl + cl;
    }
    return found;
}
