// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Music inside zip archives. A path "<dir>/<name>.zip/<entry path>" names an entry of the archive on the card; "<name>.zip" and
 * "<name>.zip/<folder>" are folders of it. Built on JukeDial's pc_zip (central directory, sorted by name, stored + deflated entries).
 * Entries are extracted whole into PSRAM and played from memory, so one entry is limited to NSN_ZIP_MAX_ENTRY (pc_zip_extract's own 4 MB
 * limit is lower, hence our own extraction on top of pc_zip_data_offset / pc_inflate / pc_crc32). */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NSN_ZIP_MAX_ENTRY (8u << 20)

typedef enum { NSN_ZIP_NONE, NSN_ZIP_FILE, NSN_ZIP_DIR } nsn_zip_kind_t;

/* true: `path` is "<existing regular file ending in .zip>[/<inner>]". zip gets the archive's path, *inner points into `path` just after the
 * slash ("" for the archive itself; a trailing '/' may remain: the functions below ignore it). A folder that happens to be named x.zip is not an archive. */
bool nsn_zip_split(const char *path, char *zip, size_t zcap, const char **inner);
bool nsn_zip_is_archive_name(const char *name);                  /* ends in .zip (any case) */

/* what `inner` is inside the archive; *size = the uncompressed size of a file. 0 ok (kind may be NSN_ZIP_NONE), <0 -errno (not a zip, no memory) */
int nsn_zip_stat(const char *zip, const char *inner, nsn_zip_kind_t *kind, uint32_t *size);

/* the entries directly inside folder `dir` ("" = root): playable files (ok(name) true) and sub-folders that hold at least one playable file, in name order.
 * Entries over the size limit are listed (play refuses them). Returns the number emitted, <0 -errno. */
int nsn_zip_ls(const char *zip, const char *dir, bool (*ok)(const char *name), void (*emit)(void *ctx, const char *name, bool is_dir), void *ctx);

/* every playable entry at or below `dir` (recursive), in name order, by entry path; entries over the limit or with an unsupported method are left out
 * (*skipped counts them, may be NULL). emit returns false to stop. Returns the number emitted, <0 -errno. */
int nsn_zip_collect(const char *zip, const char *dir, bool (*ok)(const char *name), bool (*emit)(void *ctx, const char *entry), void *ctx, int *skipped);

/* the entry's bytes in a new PSRAM block (heap_caps_free it); CRC checked. <0: -ENOENT, -E2BIG (over `max`), -ENOTSUP (method), -EBADMSG (corrupt),
 * -ENOMEM, -EINVAL (not a zip). */
int nsn_zip_read(const char *zip, const char *inner, uint32_t max, uint8_t **out, uint32_t *len);
const char *nsn_zip_err(int rc);                                  /* short words for the log */
