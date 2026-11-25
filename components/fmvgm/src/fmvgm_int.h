// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* fmvgm internals: command sources (VGM, S98), the chip layer, shared by fmvgm_src.cpp, fmvgm_chips.cpp and fmvgm_core.cpp. */
#include <stddef.h>
#include <stdint.h>
#include "fmvgm.h"

#define FM_MAX_CHIPS 4
/* chip types. OPN = YM2203 (FM + SSG), OPNA = YM2608, OPNB = YM2610 / YM2610B, OPN2 = YM2612 / YM3438, SSG = AY8910 / YM2149 alone, SN = SN76489, OKI = MSM6295 */
enum { CT_NONE = 0, CT_OPM, CT_OPN, CT_OPNA, CT_OPNB, CT_OPN2, CT_SSG, CT_SN, CT_OKI, CT_N };
enum { SRC_VGM = 0, SRC_S98 = 1 };
enum { EV_WRITE = 0, EV_WAIT = 1, EV_END = 2, EV_DATA = 3, EV_DAC = 4, EV_SEEK = 5 };
/* ROM / memory classes of a data block (the numbers are ymfm's access_class: 1 ADPCM-A, 2 ADPCM-B / DELTA-T; 3 = a PCM bank or ROM that is not ymfm's) */
enum { MC_ADPCM_A = 1, MC_ADPCM_B = 2, MC_PCM = 3 };

struct Ev {
    uint8_t kind, slot, port, reg, val, cls;
    uint32_t units;                                      /* EV_WAIT / EV_DAC: time units */
    uint32_t total, start, len; const uint8_t *p;        /* EV_DATA: the block (total = size of the whole ROM the file declares, start = where this piece goes, 0xFFFFFFFF = append) */
};

struct ChipDesc { uint8_t type; uint32_t clock; uint32_t variant; };

struct Src {
    const uint8_t *d; uint32_t size;
    uint8_t fmt;
    uint32_t pos, data_off, loop_off;                    /* absolute; loop_off 0 = none */
    uint32_t stream_end;                                 /* the command stream stops here (never past size) */
    uint8_t slotmap[256];                                /* VGM: command byte -> chip slot (0xFF none); S98: device index -> slot (first 128 entries) */
    uint8_t cmdkind[256];                                /* VGM: how the command's operands read: 1 = reg, val; 2 = one data byte (SN76489); 3 = aa dd with the chip in bit 7 of aa (AY8910, OKIM6295: slot is the first chip, +1 for bit 7) */
    uint8_t cmdport[256];                                /* VGM: chip port of the command (YM2612 / YM2608 / YM2610: 0 or 1) */
    uint8_t tslot[CT_N][2];                              /* chip slot by type and chip number (0xFF none): for data blocks and the YM2612 DAC */
    uint8_t sports[FM_MAX_CHIPS];                        /* S98: ports a slot accepts (bit 0 = port 0, bit 1 = port 1) */
    uint8_t n_dev;                                       /* S98 devices in the table */
    uint32_t no_wait_run;                                /* commands since the last wait (guard against a loop that never waits) */
    uint8_t err;                                         /* 1: ended by something the stream should not contain (unknown or cut-off command) */
    uint32_t unit_p, unit_q;                             /* time units per second = p / q (VGM 44100 / 1, S98 den / num) */
    uint8_t scan, intro_set; uint64_t acc, intro;        /* length scan: units walked so far, and how many of them lie before the loop point */
    uint32_t pend_wait; uint8_t pend_slot, pend_kind;    /* a command that yields two events (YM2612 DAC write + wait) */
    uint8_t dac_slot;
};

struct SrcInfo {
    int format;
    int n_chips; ChipDesc chip[FM_MAX_CHIPS];
    uint32_t unit_p, unit_q;
    uint32_t intro_units, loop_units, total_units;       /* counted from the stream itself */
    int has_loop;
    uint64_t dropped;                                    /* PC_VGM_BIT mask of chips the file writes that are not played (the host allowed it: fmvgm_set_drop) */
    char dropped_names[40];
    char title[96], author[96], game[96], system[48];
    char why[48];                                        /* when open fails: the reason (chip names, "bad header" ...) */
};

/* 0 ok, <0 not playable (info.why says why). `s` is filled to start at the first command. */
int vgm_open(const uint8_t *d, uint32_t n, Src &s, SrcInfo &i, uint64_t drop);
int s98_open(const uint8_t *d, uint32_t n, Src &s, SrcInfo &i, uint64_t drop);
int src_next(Src &s, Ev &e);
void src_rewind(Src &s);
extern uint64_t g_drop_mask;                             /* fmvgm_set_drop: what open() uses */

/* ---- the chip layer (fmvgm_chips.cpp) ----------------------------------------------------------------------------------------------------------------------------------- */
const char *fm_chip_name(int type);
bool fm_rom2608_ready(void);
bool fm_clock_ok(int type, uint32_t clock);
void fm_chip_rate(int type, uint32_t clock, uint32_t variant, uint32_t *raw, uint8_t *dec, uint32_t *max_raw);   /* the rate the chip delivers (before the decimation), the decimation, the highest rate it may switch to */
void *fm_alloc(size_t n, fmvgm_mem_t kind);              /* the port's allocator (fmvgm_core.cpp), for chips that need memory of their own */
void fm_free(void *p);

struct Chip {
    uint8_t type; uint8_t dec; uint16_t gain;            /* gain in 1/256 */
    uint32_t raw_rate; bool rate_dirty;
    uint8_t flags;                                       /* CF_* */
    virtual ~Chip() {}
    virtual void write(uint8_t port, uint8_t reg, uint8_t val) = 0;
    virtual void gen(int32_t *lr, uint32_t n) = 0;       /* n raw frames, interleaved L R */
    virtual int  block(uint8_t cls, uint32_t total, uint32_t start, const uint8_t *p, uint32_t len) { return 0; }   /* a data block; 0 ok, <0 refused */
    virtual void dac() {}                                /* YM2612: write the next byte of the PCM bank to register 0x2A */
    virtual void dac_seek(uint32_t pos) {}
    virtual bool ok() { return true; }                   /* false when a buffer the chip needs could not be allocated */
    virtual void start_s98() {}                          /* the state an S98 dump expects before its first write */
};
enum { CF_NEEDS_ROM = 1, CF_ROM_MISSING = 2, CF_ADPCM = 4 };
size_t fm_chip_size(int type);
size_t fm_chip_arena(int type);                          /* bytes of ymfm's operator objects the constructor makes (allocation hook) */
Chip *fm_chip_make(void *place, int type, uint32_t clock, uint32_t variant);
