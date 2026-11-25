// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
#include "pc/pc_io.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Title / artist / album of the formats whose decoders have no tag call, read from the file header: C64 PSID / RSID (name, author, released),
 * MOD / XM / S3M / IT (module title). ext = the extension with its dot. Strings come out valid UTF-8 (Latin-1 or CP437 bytes above 127 become '?').
 * Leaves the buffers alone for any other format or an unreadable header. */
void nsn_chip_tags(const pc_reader_t *r, const char *ext, char *title, size_t tn, char *artist, size_t an, char *album, size_t bn);
/* copies src into dst (cap bytes) keeping only valid UTF-8 and printable ASCII (anything else becomes '?'), trimming trailing spaces */
void nsn_tag_clean(char *dst, size_t cap, const char *src, size_t srclen);
#ifdef __cplusplus
}
#endif
