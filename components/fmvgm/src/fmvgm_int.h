// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* fmvgm internals: command sources (VGM, S98), shared by fmvgm_src.cpp and fmvgm_core.cpp. */
#include <stddef.h>
#include <stdint.h>

#define FM_MAX_CHIPS 4
enum { CT_NONE = 0, CT_OPM = 1, CT_OPN = 2 };           /* YM2151, YM2203 */
enum { SRC_VGM = 0, SRC_S98 = 1 };
enum { EV_WRITE = 0, EV_WAIT = 1, EV_END = 2 };

struct Ev { uint8_t kind, slot, reg, val; uint32_t units; };

struct ChipDesc { uint8_t type; uint32_t clock; };

struct Src {
    const uint8_t *d; uint32_t size;
    uint8_t fmt;
    uint32_t pos, data_off, loop_off;                    /* absolute; loop_off 0 = none */
    uint32_t stream_end;                                 /* the command stream stops here (never past size) */
    uint8_t slotmap[256];                                /* VGM: command byte -> chip slot (0xFF none); S98: device index -> slot (first 128 entries) */
    uint8_t n_dev;                                       /* S98 devices in the table */
    uint32_t no_wait_run;                                /* commands since the last wait (guard against a loop that never waits) */
    uint8_t err;                                         /* 1: ended by something the stream should not contain (unknown or cut-off command) */
    uint32_t unit_p, unit_q;                             /* time units per second = p / q (VGM 44100 / 1, S98 den / num) */
    uint8_t scan, intro_set; uint64_t acc, intro;        /* length scan: units walked so far, and how many of them lie before the loop point */
};

struct SrcInfo {
    int format;
    int n_chips; ChipDesc chip[FM_MAX_CHIPS];
    uint32_t unit_p, unit_q;
    uint32_t intro_units, loop_units, total_units;       /* counted from the stream itself */
    int has_loop;
    char title[96], author[96], game[96], system[48];
    char why[48];                                        /* when open fails: the reason (chip names, "bad header" ...) */
};

/* 0 ok, <0 not playable (info.why says why). `s` is filled to start at the first command. */
int vgm_open(const uint8_t *d, uint32_t n, Src &s, SrcInfo &i);
int s98_open(const uint8_t *d, uint32_t n, Src &s, SrcInfo &i);
int src_next(Src &s, Ev &e);
void src_rewind(Src &s);

/* routing helpers (fmvgm_route / fmvgm_s98_missing are in the public header) */
const char *fm_chip_name(int type);
