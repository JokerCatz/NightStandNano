// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* VGM header inspector: pure C, no allocation, no dependency beyond <stdint.h>. Given the first bytes of an already un-gzipped VGM it says which
 * version it is, which chips it declares (non-zero clock), where the data / loop / GD3 are, and (with the whole file) which chips the command
 * stream really writes. Layout: the published VGM specification (1.71; vgmrips.net/wiki/VGM_Specification, cross-checked with the field order of
 * vgmplay's VGMFile.h). Chip ids are the spec's chip-ID order (the 0x90 stream chip types).
 *
 * Rules that matter (the classic bugs):
 *  - a clock field exists only when the file's version has it AND it lies inside the header (before the data offset). Version 1.50 files have a
 *    0x40 byte header: whatever follows is command data, never a clock;
 *  - the data offset (0x34, v1.50+) is relative to 0x34, 0 = 0x40; a header shorter than a field means the field is zero;
 *  - versions below 1.10 have no YM2612 / YM2151 field: the YM2413 field is shared by the three (PC_VGM_CF_LEGACY_FM); pc_vgm_scan resolves it by the
 *    first FM command, as game-music-emu does.
 *  - bit 30 of a clock = a second chip, bit 31 = a chip variant (T6W28, YM2610B, VRC7, ...); both are stripped from `clock`. */
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PC_VGM_SN76489 = 0, PC_VGM_YM2413, PC_VGM_YM2612, PC_VGM_YM2151, PC_VGM_SEGAPCM, PC_VGM_RF5C68, PC_VGM_YM2203, PC_VGM_YM2608, PC_VGM_YM2610,
    PC_VGM_YM3812, PC_VGM_YM3526, PC_VGM_Y8950, PC_VGM_YMF262, PC_VGM_YMF278B, PC_VGM_YMF271, PC_VGM_YMZ280B, PC_VGM_RF5C164, PC_VGM_PWM,
    PC_VGM_AY8910, PC_VGM_GBDMG, PC_VGM_NESAPU, PC_VGM_MULTIPCM, PC_VGM_UPD7759, PC_VGM_OKIM6258, PC_VGM_OKIM6295, PC_VGM_K051649, PC_VGM_K054539,
    PC_VGM_HUC6280, PC_VGM_C140, PC_VGM_K053260, PC_VGM_POKEY, PC_VGM_QSOUND, PC_VGM_SCSP, PC_VGM_WSWAN, PC_VGM_VSU, PC_VGM_SAA1099, PC_VGM_ES5503,
    PC_VGM_ES5506, PC_VGM_X1010, PC_VGM_C352, PC_VGM_GA20,
    PC_VGM_CHIP_N
} pc_vgm_chip_t;

#define PC_VGM_BIT(id) ((uint64_t)1 << (id))

enum { PC_VGM_CF_DUAL = 1,            /* a second chip (clock bit 30, or extra header id bit 7) */
       PC_VGM_CF_VARIANT = 2,         /* clock bit 31: SN76489 T6W28, YM2610B, YM2413 VRC7, ... (pc_vgm_chip_label) */
       PC_VGM_CF_LEGACY_FM = 4 };     /* version < 1.10: this YM2413 field is shared with YM2612 / YM2151 until pc_vgm_scan resolves it */

typedef struct { uint8_t id; uint8_t flags; uint32_t clock; } pc_vgm_chipinfo_t;   /* clock in Hz, bits 30 / 31 stripped */

enum { PC_VGM_I_TRUNC = 1,            /* fewer bytes were given than the header (data offset) needs: chips beyond them are unknown */
       PC_VGM_I_UNKNOWN_CHIP = 2 };   /* a v1.72+ header has non-zero bytes where this module knows no chip */

typedef struct {
    uint32_t version;                 /* BCD: 0x171 = 1.71 */
    uint32_t eof_size;                /* file size the header claims (EOF offset + 4) */
    uint32_t data_off;                /* absolute offset of the command stream */
    uint32_t gd3_off;                 /* absolute offset of the "Gd3 " block, 0 = none */
    uint32_t loop_off;                /* absolute offset of the loop point, 0 = no loop */
    uint32_t total_samples, loop_samples;   /* at 44100 Hz */
    uint32_t rate;                    /* recording rate in Hz (v1.01+), 0 = unknown */
    uint32_t flags;                   /* PC_VGM_I_* */
    uint64_t chip_mask;               /* PC_VGM_BIT of every declared chip */
    uint64_t dual_mask;               /* ... of those with a second chip */
    uint8_t  n_chips;
    pc_vgm_chipinfo_t chips[PC_VGM_CHIP_N];
} pc_vgm_info_t;

enum { PC_VGM_OK = 0, PC_VGM_E_SHORT = -1, PC_VGM_E_IDENT = -2, PC_VGM_E_VERSION = -3, PC_VGM_E_DATAOFF = -4 };

/* Reads the header from the first n bytes. 0 or a PC_VGM_E_* code (info is then undefined). n >= 0x40 is needed; give up to 0x100 + the extra header
 * (or the whole file) so that every chip field is visible. */
int pc_vgm_parse(const uint8_t *p, size_t n, pc_vgm_info_t *info);

/* Command stream scan (the whole file, or at least from data_off to the end): which declared chips are written at all. */
typedef struct {
    uint64_t used;                    /* chips whose write commands occur */
    uint32_t commands;                /* commands walked */
    uint8_t  valid;                   /* 1: the walk reached the end command / end of data in step; 0: an unknown / cut-off command, `used` is a lower bound */
    uint8_t  dac_stream;              /* a 0x90..0x95 stream command occurs */
} pc_vgm_use_t;
/* Fills `use`; also resolves a PC_VGM_CF_LEGACY_FM chip in `info` by the first FM command (YM2413 / YM2612 / YM2151; none found: left as YM2413). */
void pc_vgm_scan(const uint8_t *vgm, size_t len, pc_vgm_info_t *info, pc_vgm_use_t *use);

/* Chips declared and not in `supported` (a mask of PC_VGM_BIT). When `use` is given and valid, a declared chip whose commands never occur does not count. */
uint64_t pc_vgm_unsupported(const pc_vgm_info_t *info, uint64_t supported, const pc_vgm_use_t *use);

const char *pc_vgm_chip_name(int id);                                   /* "YM2151"; "?" for an id out of range */
const char *pc_vgm_chip_label(const pc_vgm_chipinfo_t *c);              /* the name, or the variant's ("YM2610B", "T6W28", "VRC7") when the variant flag is set */
int pc_vgm_names(const pc_vgm_info_t *info, uint64_t mask, char *buf, size_t cap);   /* "YM2151, OKIM6295" (variants by label; "chip?" for an unknown one); returns the length written, buf is always terminated */

#ifdef __cplusplus
}
#endif
