// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
/* raw deflate (no zlib/gzip header): bytes written, -1 on corrupt data or a too-small out buffer */
long pc_inflate_raw(const void *in, size_t in_len, void *out, size_t out_len);
/* the same with the ~11 KB decompressor state in a caller buffer (PSRAM) instead of on the stack */
size_t pc_inflate_state_size(void);
long pc_inflate_raw_state(const void *in, size_t in_len, void *out, size_t out_len, void *state);
