// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
/* CRC-32 as zlib computes it (start with 0; pass the previous result to continue). Byte table in flash. */
uint32_t pc_crc32(uint32_t crc, const void *data, uint32_t n);
