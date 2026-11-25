// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "pc/pc_crc32.h"
#include <stdint.h>
#include "pc/pc_io.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Zip reader for chipdig system zips (deflate) and sleepmixer's library.muk (stored). No zip64. */
typedef struct { char *name; uint32_t local_off, csize, usize, crc; uint16_t method; } pc_zip_entry_t;

int  pc_zip_open(const pc_reader_t *r, pc_zip_entry_t **out, int *n);   /* central directory, sorted by name; -EINVAL not a zip */
void pc_zip_free(pc_zip_entry_t *e, int n);
int  pc_zip_data_offset(const pc_reader_t *r, const pc_zip_entry_t *e, uint32_t *off);
#define PC_ZIP_MAX_EXTRACT (4u << 20)
/* malloc'd copy of the entry; -E2BIG over 4 MB, -EBADMSG corrupt/CRC, -ENOTSUP other methods */
int  pc_zip_extract(const pc_reader_t *r, const pc_zip_entry_t *e, uint8_t **out, uint32_t *len);
int  pc_gunzip(const uint8_t *in, uint32_t n, uint8_t **out, uint32_t *len);   /* .vgz, up to PC_ZIP_MAX_EXTRACT */
int  pc_gunzip_max(const uint8_t *in, uint32_t n, uint8_t **out, uint32_t *len, uint32_t max_out);   /* the same with the caller's limit on the uncompressed size; -E2BIG above it */
/* In-place gunzip for large files (Neo Geo VGZ of 10-20 MB): read the gzip file into the END of a buffer of isize + pc_gunzip_margin(isize) bytes, call pc_gunzip_inplace; the result is at
 * buf[0..len). pc_gunzip_isize reads the uncompressed size from the last 4 bytes of the file (give it those bytes and the file's size). Added after the copy from JukeDial (ORIGIN.md). */
uint32_t pc_gunzip_margin(uint32_t isize);
int  pc_gunzip_isize(const uint8_t *tail, uint32_t n, uint32_t *isize);
int  pc_gunzip_inplace(uint8_t *buf, uint32_t off, uint32_t n, uint32_t *len);
/* Every function here returns -EINVAL for NULL or inconsistent arguments (entry offsets past the end of the file, impossible counts) and reads nothing past the file or the directory. */
/* No heap at all (a ~600 B stack buffer): finds names[0..n-1] in one pass over the central directory. out[i].name is
 * names[i] when found, NULL when missing. Returns how many were found, -EINVAL not a zip, -EIO. Names over 255 bytes
 * never match. For packs with a few entries, where pc_zip_open's per-zip allocations would be wasted. */
int  pc_zip_lookup(const pc_reader_t *r, const char *const *names, int n, pc_zip_entry_t *out);
/* The idx-th entry of a pc_zip_open result, or NULL when idx is outside [0, n): never index the array with an untrusted number. */
const pc_zip_entry_t *pc_zip_at(const pc_zip_entry_t *e, int n, int idx);
const pc_zip_entry_t *pc_zip_find(const pc_zip_entry_t *e, int n, const char *name);

#ifdef __cplusplus
}
#endif
