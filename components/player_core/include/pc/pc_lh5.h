// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* LHA (-lh5- and stored -lh0-) depacker for a single-file archive in memory (the Atari ST .ym files come this way). */
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
bool pc_lh5_is_packed(const uint8_t *in, uint32_t n);      /* an LHA level 0/1 header with method -lh5- or -lh0- at offset 2 */
/* malloc'd copy of the file inside (at most max_out bytes): 0, or -1 damaged / unsupported, -2 too big / out of memory. Reads nothing outside in[0..n). */
int pc_lh5_unpack(const uint8_t *in, uint32_t n, uint32_t max_out, uint8_t **out, uint32_t *len);
#ifdef __cplusplus
}
#endif
