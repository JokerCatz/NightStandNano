// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* 記憶卡診斷 (cardtest, owner 2026-11-20): the pure half. No RTOS, no BSP, no LVGL.
 *  - MB/s arithmetic (1 MB = 1,000,000 bytes, as card makers count; rates are carried as MB/s x 100)
 *  - the verdict table: which formats a measured 16 KB read speed can feed
 *  - the fixed pseudo-random pattern of the file write test (position-independent: any chunking writes / checks the same bytes) */

#define PC_CT_SAFETY_X100      150   /* the card must deliver 1.5 x the stream's byte rate (seeks, FAT walks, a busy card) */
#define PC_CT_MARGINAL_PCT      75   /* "marginal": at least 75 % of that requirement (1.125 x the stream); below: not enough */
#define PC_CT_FLAC_RATIO_PCT    75   /* worst-case FLAC size as a share of the PCM it decodes to (measured 384k/24 content: 1.3-2.25 MB/s of 2.3 PCM) */

typedef enum { PC_CT_NO, PC_CT_MARGINAL, PC_CT_YES } pc_ct_grade_t;

typedef struct {
    const char *label;       /* a format name, the same in every language */
    uint32_t rate;           /* sample rate */
    uint8_t bits;            /* bits per sample */
    bool flac;               /* compressed (PC_CT_FLAC_RATIO_PCT of the PCM); false: PCM as stored (WAV) */
    uint32_t fixed_bps;      /* nonzero: a fixed stream (bytes per second) instead of rate x bits x 2 channels */
} pc_ct_fmt_t;

uint32_t pc_ct_x100(uint64_t bytes, uint64_t us);            /* MB/s x 100; 0 for no time */
int pc_ct_format_count(void);
const pc_ct_fmt_t *pc_ct_format(int i);                      /* NULL out of range */
uint32_t pc_ct_stream_bps(const pc_ct_fmt_t *f);             /* the stream's worst-case bytes per second */
uint32_t pc_ct_need_x100(const pc_ct_fmt_t *f);              /* what the card must read: MB/s x 100, rounded up */
pc_ct_grade_t pc_ct_grade(uint32_t speed_x100, const pc_ct_fmt_t *f);

/* the ladder (天梯): the formats sorted by what they need, heaviest first, with TWO marker rows (the raw read and the file read-back are separate scores, never merged),
 * each at its own place: a marker sits above the first format whose need is at most its speed. A mode without a result has speed 0: its marker is at the bottom
 * ("not tested"), raw before file. The formats' grades follow the file read-back marker when it has a result (closest to real playback), else the raw one
 * (PC_CT_NO for all and unmarked when neither has a result; see pc_ct_follow). Always pc_ct_format_count() + 2 rungs; returns that count (0 when max is smaller).
 * Everything at or below the followed marker is OK, the rows just above it within PC_CT_MARGINAL_PCT are marginal, the rest too slow. */
#define PC_CT_RUNG_RAW  (-1)       /* pc_ct_rung_t.fmt of the raw marker */
#define PC_CT_RUNG_FILE (-2)       /* ... of the file read-back marker */
typedef struct { int8_t fmt; uint8_t grade; uint32_t need_x100; } pc_ct_rung_t;   /* a marker: need_x100 = its speed (0: no result) */
int pc_ct_ladder(uint32_t raw_x100, uint32_t file_x100, pc_ct_rung_t *out, int max);
uint32_t pc_ct_follow(uint32_t raw_x100, uint32_t file_x100, bool *file);          /* the speed the grades follow (0: none) and whether it is the file one */

/* The result file /sdcard/_player/cardtest.txt: one line per mode, the latest only, with the card's CID serial and size (another card's lines are ignored):
 *   raw  serial=00005D01 size=31914983424 time=2026-11-20T12:30 avg16=350 slow16=350 max64=800 worst_us=4681
 *   file serial=00005D01 size=31914983424 time=- write=400 read=380 verify=1
 * time is "-" when the clock is not set. The numbers are MB/s x 100 (worst_us: microseconds). */
enum { PC_CT_RAW, PC_CT_FILE, PC_CT_MODES };
#define PC_CT_NV 7
typedef struct {
    uint32_t serial;
    uint64_t size;
    char when[20];            /* "YYYY-MM-DDTHH:MM" or "-" */
    uint32_t v[PC_CT_NV];     /* raw: avg16, slow16, max64, worst_us, then the 16 KB speed of the start / middle / end region (reg0..reg2, 0 = not measured, written only when measured); file: write, read, verify (0 / 1) */
} pc_ct_rec_t;
int pc_ct_rec_format(int mode, const pc_ct_rec_t *r, char *buf, size_t n);          /* the line with its newline; its length, 0 when it does not fit */
bool pc_ct_rec_parse(const char *line, int *mode, pc_ct_rec_t *r);                  /* false: not a line of ours (a garbled line is skipped, never half-read) */
/* the file's new text: `old` (NUL-terminated, may be NULL) with the line of `mode` replaced by `r`; the other mode's line stays only when it is the same card. Returns the length, 0 when it does not fit. */
size_t pc_ct_file_update(const char *old, int mode, const pc_ct_rec_t *r, char *out, size_t cap);
/* the saved record of `mode` in `text` for the card `serial` / `size`; false: none, or another card */
bool pc_ct_file_find(const char *text, int mode, uint32_t serial, uint64_t size, pc_ct_rec_t *r);

/* night3 G: the raw test reads each region TWICE at the same place, 16 KB chunks: the first pass (warm) wakes the card / controller and is only reported, the SECOND pass is the region's speed.
 * (FakePod card, three runs in a row: the start region read 2.76 / 3.36 / 4.44 MB/s while the others held 9-11, so the verdict flipped at the low end.) Per region: pass 0 = warm 16 KB,
 * pass 1 = measured 16 KB (same sectors), pass 2 = 64 KB (REGION_ALIGN sectors later, so no card cache helps). */
#define PC_CT_PASSES_PER_REGION 3
enum { PC_CT_PASS_WARM16, PC_CT_PASS_MEAS16, PC_CT_PASS_MEAS64 };
int      pc_ct_pass_role(uint32_t pass);                                  /* PC_CT_PASS_* of pass number `pass` */
int      pc_ct_pass_region(uint32_t pass);
uint32_t pc_ct_pass_sector(uint32_t base, int role, uint32_t region_align);   /* the first sector of a pass: the region's base, the 64 KB pass region_align later */
#define PC_CT_RAW_METHOD "second_pass"   /* the console's disclosure word: the speed is the second 16 KB pass of each region, the first (warm) pass is printed as warm= */

/* the raw verdict is conservative: the speed of the SLOWEST region (a region with 0 = not measured is skipped; 0 when none was). The page shows all three (start / middle / end) and says
 * that where the files sit decides: the file test shows that. */
uint32_t pc_ct_slowest(const uint32_t *reg, int n);

/* the write test's pattern: byte i of the file is a pure function of i */
void pc_ct_fill(uint8_t *buf, uint32_t len, uint64_t offset);
/* true: buf equals the pattern at `offset`; else *bad = the file offset of the first byte that differs (bad may be NULL) */
bool pc_ct_check(const uint8_t *buf, uint32_t len, uint64_t offset, uint64_t *bad);

#ifdef __cplusplus
}
#endif
