// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Conversions stop at a NUL code unit and never split a character; dst is always NUL-terminated.
 * Return the byte length written (without the NUL). */
size_t pc_utf16_to_utf8(const uint8_t *src, size_t src_len, bool big_endian, char *dst, size_t dst_cap);
size_t pc_latin1_to_utf8(const uint8_t *src, size_t n, char *dst, size_t cap);
/* Shortens s so it fits in cap bytes (cap-1 plus NUL) without cutting a multi-byte character. */
size_t pc_utf8_truncate(char *s, size_t cap);
/* Copies a UTF-8 string into cap bytes, never ending inside a character; returns the length. */
size_t pc_utf8_copy(char *dst, size_t cap, const char *src);
/* When every one of n >= 2 titles starts with the same 【...】 (a bracketed artist), the byte length of that prefix; else 0.
 * 0 too when a title is nothing but the prefix (and spaces), so no row would show empty. */
size_t pc_title_common_prefix(const char *const *titles, int n);
/* Where one title's row text starts once that prefix (prefix bytes, from pc_title_common_prefix) is dropped: after it and the spaces
 * (ASCII or U+3000) that follow it. 0 (the whole title) when prefix is 0 or nothing but spaces would be left. */
size_t pc_title_strip(const char *title, size_t prefix);

#ifdef __cplusplus
}
#endif
