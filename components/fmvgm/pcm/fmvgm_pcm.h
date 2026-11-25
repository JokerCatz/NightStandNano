// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The sample / PSG chip interface of fmvgm. A chip core that is not ymfm is wrapped by one small adapter file (pcm/fmvgm_pcm_<chip>.c) that fills one of these tables;
 * the backend (src/fmvgm_chips.cpp) treats every table the same way: it creates the chip at open, forwards the VGM / S98 register writes and ROM data blocks, and pulls raw frames.
 * Adding a chip = one adapter (about 40 lines around the core's own entry points), one row in the chip table in fmvgm_chips.cpp, and the VGM command / data-block type in fmvgm_src.cpp.
 *
 * Contract: create() runs at open (or restart) on the consumer thread and may allocate through fmvgm_pcm_calloc / realloc / free (the port allocator); nothing allocates afterwards except
 * rom() before the first sample (a ROM image is copied into the core's own buffer). write() / render() run on the producer thread, one thread at a time per chip. The chip's rate may change
 * when a register write changes its clock: rate() is asked again after every write. Integer in, integer out: the same writes give the same frames on every platform (no libm in the sample path). */
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct fmvgm_pcm_ops {
    const char *name;
    void *(*create)(uint32_t clock, uint32_t variant);   /* variant: chip-specific bits from the file (OKIM6295: pin 7; SN76489: feedback | width << 16 | flags << 24); NULL when out of memory */
    uint32_t (*rate)(void *chip);                        /* native sample rate in Hz (render() makes this many frames per second) */
    void (*reset)(void *chip);
    void (*write)(void *chip, uint32_t reg, uint32_t val);   /* the register / offset byte and the data byte of the VGM command (S98: the S98 reg and data) */
    void (*render)(void *chip, int32_t *lr, uint32_t n);     /* n frames, interleaved left / right, raw (the backend applies the chip gain) */
    int  (*rom)(void *chip, uint32_t total, uint32_t start, const uint8_t *data, uint32_t len);   /* a ROM data block: 0 ok, <0 refused (too large, no memory); NULL for chips without ROM */
    void (*destroy)(void *chip);
} fmvgm_pcm_ops;
extern const fmvgm_pcm_ops fmvgm_pcm_sn76496, fmvgm_pcm_okim6295;
#ifdef __cplusplus
}
#endif
