// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* A minimal stand-in for libvgm's stdtype.h / snddef.h / EmuStructs.h / EmuHelper.h / logging.h, just enough for the vendored MAME-derived cores in emu/cores/ (BSD-3-Clause, see SOURCE.md)
 * to compile unchanged. Written for this repository; it implements the names those cores use and nothing else. The directory layout (pcm/stdtype.h, pcm/emu/*.h, pcm/emu/cores/*.c)
 * mirrors libvgm's so the cores' own #include lines resolve without edits. */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef uint8_t UINT8; typedef int8_t INT8; typedef uint16_t UINT16; typedef int16_t INT16; typedef uint32_t UINT32; typedef int32_t INT32; typedef uint64_t UINT64; typedef int64_t INT64;
#ifndef INLINE
#define INLINE static inline
#endif
/* memory: the cores call calloc / realloc / free; the host's allocator is the one the firmware chooses (fmvgm_pcm.h) */
void *fmvgm_pcm_calloc(size_t n, size_t sz);
void *fmvgm_pcm_realloc(void *p, size_t n);
void fmvgm_pcm_free(void *p);
#define calloc fmvgm_pcm_calloc
#define realloc fmvgm_pcm_realloc
#define free fmvgm_pcm_free
