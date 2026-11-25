// SPDX-License-Identifier: LGPL-2.1-or-later
#include "chiptags.h"
#include <string.h>
#include <strings.h>

void nsn_tag_clean(char *dst, size_t cap, const char *src, size_t srclen)
{
    size_t o = 0;
    if (!cap) return;
    for (size_t i = 0; i < srclen && src[i] && o + 1 < cap;) {
        unsigned char c = (unsigned char)src[i];
        size_t l = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 0;
        int ok = l > 0 && i + l <= srclen;
        for (size_t k = 1; ok && k < l; k++) if (((unsigned char)src[i + k] & 0xC0) != 0x80) ok = 0;
        if (l == 1 && c < 0x20) { dst[o++] = ' '; i++; continue; }
        if (!ok) { dst[o++] = '?'; i++; continue; }
        if (o + l >= cap) break;
        memcpy(dst + o, src + i, l); o += l; i += l;
    }
    while (o && dst[o - 1] == ' ') o--;
    dst[o] = 0;
}

static void field(const pc_reader_t *r, uint32_t off, size_t len, char *dst, size_t cap)
{
    char b[64];
    if (len > sizeof b) len = sizeof b;
    if (!dst || !cap || r->read(r->u, off, b, (uint32_t)len) != (int)len) return;
    nsn_tag_clean(dst, cap, b, len);
}

void nsn_chip_tags(const pc_reader_t *r, const char *ext, char *title, size_t tn, char *artist, size_t an, char *album, size_t bn)
{
    uint8_t h[4];
    if (!ext || r->read(r->u, 0, h, 4) != 4) return;
    if (!strcasecmp(ext, ".sid")) {
        if (memcmp(h, "PSID", 4) && memcmp(h, "RSID", 4)) return;
        field(r, 0x16, 32, title, tn); field(r, 0x36, 32, artist, an); field(r, 0x56, 32, album, bn);
    } else if (!strcasecmp(ext, ".xm")) {
        if (r->size > 0x3C) field(r, 17, 20, title, tn);
    } else if (!strcasecmp(ext, ".it")) {
        if (!memcmp(h, "IMPM", 4)) field(r, 4, 26, title, tn);
    } else if (!strcasecmp(ext, ".s3m")) {
        field(r, 0, 28, title, tn);
    } else if (!strcasecmp(ext, ".mod")) {
        field(r, 0, 20, title, tn);
    }
}
