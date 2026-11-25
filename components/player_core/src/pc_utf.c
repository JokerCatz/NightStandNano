// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_utf.h"
#include <string.h>

/* Appends code point cp if it fits; false when full. */
static bool put(uint32_t cp, char *dst, size_t cap, size_t *n)
{
    char b[4];
    size_t k;
    if (cp < 0x80) { b[0] = (char)cp; k = 1; }
    else if (cp < 0x800) { b[0] = (char)(0xC0 | (cp >> 6)); b[1] = (char)(0x80 | (cp & 0x3F)); k = 2; }
    else if (cp < 0x10000) { b[0] = (char)(0xE0 | (cp >> 12)); b[1] = (char)(0x80 | ((cp >> 6) & 0x3F)); b[2] = (char)(0x80 | (cp & 0x3F)); k = 3; }
    else { b[0] = (char)(0xF0 | (cp >> 18)); b[1] = (char)(0x80 | ((cp >> 12) & 0x3F)); b[2] = (char)(0x80 | ((cp >> 6) & 0x3F)); b[3] = (char)(0x80 | (cp & 0x3F)); k = 4; }
    if (*n + k + 1 > cap) return false;
    memcpy(dst + *n, b, k);
    *n += k;
    return true;
}

size_t pc_utf16_to_utf8(const uint8_t *src, size_t src_len, bool be, char *dst, size_t cap)
{
    size_t n = 0;
    if (!cap) return 0;
    for (size_t i = 0; i + 1 < src_len;) {
        uint32_t u = be ? (uint32_t)(src[i] << 8 | src[i + 1]) : (uint32_t)(src[i + 1] << 8 | src[i]);
        i += 2;
        if (u == 0) break;
        uint32_t cp = u;
        if (u >= 0xD800 && u <= 0xDBFF) {
            uint32_t lo = 0;
            if (i + 1 < src_len) lo = be ? (uint32_t)(src[i] << 8 | src[i + 1]) : (uint32_t)(src[i + 1] << 8 | src[i]);
            if (lo >= 0xDC00 && lo <= 0xDFFF) { cp = 0x10000 + ((u - 0xD800) << 10) + (lo - 0xDC00); i += 2; }
            else cp = 0xFFFD;
        } else if (u >= 0xDC00 && u <= 0xDFFF) {
            cp = 0xFFFD;
        }
        if (!put(cp, dst, cap, &n)) break;
    }
    dst[n] = 0;
    return n;
}

size_t pc_latin1_to_utf8(const uint8_t *src, size_t len, char *dst, size_t cap)
{
    size_t n = 0;
    if (!cap) return 0;
    for (size_t i = 0; i < len && src[i]; i++)
        if (!put(src[i], dst, cap, &n)) break;
    dst[n] = 0;
    return n;
}

/* longest prefix of p[0..n) that fits in cap-1 bytes and ends on a character boundary */
static size_t clip(const char *p, size_t n, size_t cap)
{
    if (n <= cap - 1) return n;
    n = cap - 1;
    while (n > 0 && ((unsigned char)p[n] & 0xC0) == 0x80) n--;
    return n;
}

size_t pc_utf8_copy(char *dst, size_t cap, const char *src)
{
    if (!cap) return 0;
    size_t n = clip(src, strlen(src), cap);
    memmove(dst, src, n);
    dst[n] = 0;
    return n;
}

size_t pc_utf8_truncate(char *s, size_t cap)
{
    size_t n = strlen(s);
    if (!cap) return 0;
    if (n < cap) return n;
    n = cap - 1;
    while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80) n--;   /* s[n] is a continuation byte: back up to its lead */
    s[n] = 0;
    return n;
}

size_t pc_title_common_prefix(const char *const *titles, int n)
{
    static const char OPEN[] = "\xE3\x80\x90", CLOSE[] = "\xE3\x80\x91";   /* 【 】 */
    if (n < 2 || strncmp(titles[0], OPEN, 3)) return 0;
    const char *e = strstr(titles[0] + 3, CLOSE);
    if (!e) return 0;
    size_t len = (size_t)(e - titles[0]) + 3;
    for (int i = 0; i < n; i++)
        if (strncmp(titles[i], titles[0], len) || !pc_title_strip(titles[i], len)) return 0;
    return len;
}

size_t pc_title_strip(const char *title, size_t prefix)
{
    if (!prefix) return 0;
    size_t i = prefix;
    for (;;) {
        if (title[i] == ' ') i++;
        else if (!strncmp(title + i, "\xE3\x80\x80", 3)) i += 3;   /* U+3000, the ideographic space */
        else break;
    }
    return title[i] ? i : 0;
}
