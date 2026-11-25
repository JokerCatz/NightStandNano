// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_icy.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "pc/pc_utf.h"

void pc_icy_init(pc_icy_t *s, uint32_t metaint)
{
    memset(s, 0, sizeof *s);
    s->metaint = s->left = metaint;
    s->meta_len = -1;
}

static const char *find_ci(const char *h, const char *n)
{
    size_t k = strlen(n);
    for (; *h; h++) if (!strncasecmp(h, n, k)) return h;
    return NULL;
}

static bool utf8_valid(const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n;) {
        uint8_t c = p[i];
        size_t len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 0;
        if (!len || i + len > n) return false;
        for (size_t j = 1; j < len; j++) if ((p[i + j] & 0xC0) != 0x80) return false;
        i += len;
    }
    return true;
}

static void parse_meta(pc_icy_t *s)
{
    s->meta[s->meta_got] = 0;                          /* the block is zero-padded */
    const char *p = find_ci(s->meta, "StreamTitle=");
    if (!p) return;                                    /* e.g. only StreamUrl: the title stays */
    p += 12;
    const char *e;
    if (*p == '\'' || *p == '"') {
        const char end[3] = { *p, ';', 0 };
        p++;
        e = strstr(p, end);                            /* "';", so "Guns N' Roses" survives */
        if (!e) { e = p + strlen(p); if (e > p && e[-1] == end[0]) e--; }
    } else {
        e = strchr(p, ';');
        if (!e) e = p + strlen(p);
    }
    char raw[sizeof s->title], u8[sizeof s->title * 2];
    size_t n = (size_t)(e - p) < sizeof raw - 1 ? (size_t)(e - p) : sizeof raw - 1;
    memcpy(raw, p, n);
    raw[n] = 0;
    if (!utf8_valid((const uint8_t *)raw, n)) pc_latin1_to_utf8((const uint8_t *)raw, n, u8, sizeof u8);
    else memcpy(u8, raw, n + 1);
    for (char *c = u8; *c; c++) if ((unsigned char)*c < 0x20 || *c == 0x7F) *c = ' ';
    char *b = u8, *z = u8 + strlen(u8);
    while (*b == ' ') b++;
    while (z > b && z[-1] == ' ') *--z = 0;
    char t[sizeof s->title];
    pc_utf8_copy(t, sizeof t, b);
    if (!strcmp(t, s->title)) return;                  /* the same song again */
    memcpy(s->title, t, sizeof t);
    s->title_new = true;
}

size_t pc_icy_feed(pc_icy_t *s, uint8_t *buf, size_t n)
{
    if (!s->metaint) return n;
    size_t in = 0, out = 0;
    while (in < n) {
        if (s->left) {                                 /* audio */
            size_t k = n - in < s->left ? n - in : s->left;
            memmove(buf + out, buf + in, k);
            in += k; out += k; s->left -= (uint32_t)k;
        } else if (s->meta_len < 0) {                  /* the length byte */
            s->meta_len = buf[in++] * 16;
            s->meta_got = 0;
            if (!s->meta_len) { s->meta_len = -1; s->left = s->metaint; }
        } else {                                       /* metadata text */
            size_t k = n - in < (size_t)(s->meta_len - s->meta_got) ? n - in : (size_t)(s->meta_len - s->meta_got);
            memcpy(s->meta + s->meta_got, buf + in, k);
            in += k; s->meta_got += (int)k;
            if (s->meta_got == s->meta_len) { parse_meta(s); s->meta_len = -1; s->left = s->metaint; }
        }
    }
    return out;
}

bool pc_icy_take_title(pc_icy_t *s, char *out, size_t cap)
{
    if (!s->title_new) return false;
    s->title_new = false;
    snprintf(out, cap, "%s", s->title);
    return true;
}
