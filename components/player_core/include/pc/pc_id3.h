// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#include "pc/pc_io.h"
#include "pc/pc_rg.h"
#ifdef __cplusplus
extern "C" {
#endif

/* ID3v2.3 / v2.4 (and FLAC / Ogg / MP4 / module) tag fields the player shows or indexes (spec 4.4). All strings UTF-8. */
typedef struct {
    char title[128], artist[96], album[128], copyright[96], purl[192];
    char *desc;              /* TXXX:description, malloc'd, at most 2048 bytes, may be NULL */
    uint32_t date;           /* yyyymmdd from TYER+TDAT (DDMM) or TDRC; year only -> yyyy0101; unknown -> 0 */
    uint16_t track;          /* leading number of TRCK */
    uint32_t apic_off, apic_len;   /* JPEG cover inside the file; 0 when absent or not usable */
    uint32_t audio_off;      /* first byte after the tag; 0 without a tag */
    pc_rg_t rg;              /* ReplayGain: ID3 TXXX, Vorbis comments, MP4 '----' items (pc_rg.h) */
} pc_id3_t;

/* Returns 0 (also for no tag or a damaged tag, keeping what was parsed); <0 only when read fails. */
int  pc_id3_parse(const pc_reader_t *r, pc_id3_t *out);
void pc_id3_free(pc_id3_t *t);

#ifdef __cplusplus
}
#endif
