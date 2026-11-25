// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* What plays after a chiptune track ends (spec 4.3, the user's rule, as JukeChip):
 * next sub-track of the same file first (also in random mode); after the last sub-track,
 * LOOP = next file in the system zip (wrapping), RANDOM = another file of the same system; sub 0. */
typedef enum { PC_SEQ_LOOP = 0, PC_SEQ_RANDOM } pc_seq_t;
typedef struct { int file, sub; } pc_chippos_t;
pc_chippos_t pc_chip_next(pc_chippos_t cur, int sub_count, int file_count, pc_seq_t mode, uint32_t rnd);
int32_t pc_chip_length_ms(int32_t gme_play_length);   /* >= 0 as is, else 180000 */
bool pc_chip_supported_ext(const char *name);          /* .nsf .nsfe .gbs .gym .hes .kss .sap .spc .vgm .vgz .ay .ahx .hvl .p8 .p8.png .ym .sid */
bool pc_xmp_ext(const char *ext);                      /* a tracker module extension with its dot (".mod", ".stm", ".okt", any case) libxmp plays (components/libxmp); the ones OPL music also uses (.imf .dtm .ims) belong to pc_opl_ext */
bool pc_opl_ext(const char *ext);                      /* a DOS AdLib / OPL music extension with its dot (".rad", any case) AdPlug plays: not .mid .raw .m (too generic), not .vgm (GME) */
/* Format tiers (owner 2026-11-18). One table in pc_chipseq.c has a cell per backend per suffix: PC_TIER_NONE (the backend does not take it), STABLE, EXPERIMENTAL.
 * The setting 實驗性格式 (default off) is read through the source the app registers (pc_tier_set_source; none = off). */
enum { PC_TIER_NONE = 0, PC_TIER_STABLE = 1, PC_TIER_EXPERIMENTAL = 2 };
void pc_tier_set_source(bool (*experimental_on)(void));
bool pc_tier_experimental_on(void);
int  pc_ext_backends(const char *ext, int *opl_cell, int *xmp_cell);   /* how many backends list the suffix (0, 1, 2); the cells (either pointer may be NULL) */
bool pc_tier_allowed(int cell);                                        /* a cell that may be played now: stable, or experimental with the setting on */
bool pc_ext_has_gated(const char *ext);                                /* some backend of the suffix is experimental and the setting is off (the suffix stays listed) */
bool pc_ext_blocked(const char *ext);                                  /* no backend of the suffix may play it now: it is refused with S_REFUSE_EXPERIMENTAL_OFF */
bool pc_ext_scan_blind(const char *ext);                               /* the library scan must read nothing of such a file (filename only) */
#ifdef __cplusplus
}
#endif
