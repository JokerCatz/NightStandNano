// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* 記憶卡格式化 (cardfmt, owner 2026-11-14): formatting a TF card on the device. Pure: no RTOS, no BSP, no LVGL; the caller feeds the clock in
 * milliseconds. Two halves:
 *  - the guard: the steps of one session of the format page, in this order only: WARN (the warning page, its 繼續 row) -> CODE (the Konami code
 *    上上下下左右左右BA by taps; a wrong tap clears the entry, PC_FG_TIMEOUT_MS without a tap goes back to WARN) -> ARM (the final armed tap) ->
 *    GO (pc_fmtguard_take: true once, the format may start) -> OFF. A session starts only for a card whose last mount failed with "gpt" or
 *    "nofs", while it is not mounted and the USB drive mode is not on (pc_fmt_entry_allowed). Back anywhere: OFF (nothing formats).
 *  - the plan: what the card becomes (MBR, one partition aligned to the SD erase block; FAT16 up to 2 GiB, FAT32 up to 32 GiB, exFAT above, with the
 *    SD Association's cluster sizes) and the MBR sector itself. */
#define PC_FG_CODE_LEN    10
#define PC_FG_TIMEOUT_MS  30000u
typedef enum { PC_FG_OFF, PC_FG_WARN, PC_FG_CODE, PC_FG_ARM, PC_FG_GO } pc_fg_step_t;
typedef enum { PC_FG_UP, PC_FG_DOWN, PC_FG_LEFT, PC_FG_RIGHT, PC_FG_B, PC_FG_A, PC_FG_NKEYS } pc_fg_key_t;
typedef enum { PC_FG_KEY_IGNORED, PC_FG_KEY_OK, PC_FG_KEY_WRONG, PC_FG_KEY_DONE, PC_FG_KEY_TIMEOUT } pc_fg_keyres_t;
typedef struct { uint8_t step, got; uint32_t last_ms; } pc_fmtguard_t;

extern const uint8_t PC_FG_KONAMI[PC_FG_CODE_LEN];          /* UP UP DOWN DOWN LEFT RIGHT LEFT RIGHT B A */

/* reason: the SD layer's word for the last failed mount (NULL after a good mount): only "gpt" and "nofs" may be formatted ("nocard", "err" never) */
bool pc_fmt_entry_allowed(const char *reason, bool mounted, bool usb_mode);
bool pc_fmtguard_begin(pc_fmtguard_t *g, const char *reason, bool mounted, bool usb_mode);   /* WARN when allowed (true), else OFF */
bool pc_fmtguard_continue(pc_fmtguard_t *g, uint32_t now_ms);                                /* WARN -> CODE (entry empty, the timeout runs from now) */
/* one tap in CODE: TIMEOUT (PC_FG_TIMEOUT_MS since the last tap or since CODE began: back to WARN, the tap is not counted), WRONG (the entry clears),
 * OK (one more right), DONE (the tenth: -> ARM); IGNORED in any other step */
int  pc_fmtguard_key(pc_fmtguard_t *g, int key, uint32_t now_ms);
bool pc_fmtguard_tick(pc_fmtguard_t *g, uint32_t now_ms);                                    /* CODE and the timeout passed: back to WARN (true) */
bool pc_fmtguard_armed(pc_fmtguard_t *g);                                                    /* the final armed tap: ARM -> GO */
bool pc_fmtguard_take(pc_fmtguard_t *g);                                                     /* GO -> OFF: true once, the format may start */
void pc_fmtguard_restart(pc_fmtguard_t *g);                                                  /* CODE / ARM / GO -> WARN (the page was covered or the screen went dark) */
void pc_fmtguard_back(pc_fmtguard_t *g);                                                     /* any step -> OFF */
const char *pc_fmtguard_step_name(int step);                                                 /* off warn code arm go */

typedef enum { PC_FMT_NONE = 0, PC_FMT_FAT16, PC_FMT_FAT32, PC_FMT_EXFAT } pc_fmt_fs_t;
#define PC_FMT_ALIGN_MAX 32768u    /* FatFs f_mkfs takes an alignment of at most 0x8000 sectors (16 MiB) */
typedef struct {
    uint32_t total_sectors;    /* the card */
    uint32_t bu_sectors;       /* the SD erase block (boundary unit): the partition starts here */
    uint32_t part_start, part_sectors;
    uint32_t align;            /* f_mkfs data area alignment: bu_sectors, at most PC_FMT_ALIGN_MAX */
    uint32_t cluster_bytes;
    uint8_t fs, n_fat, mbr_type;
} pc_fmt_plan_t;
/* false: not 512-byte sectors, smaller than 32 MiB, or more than 2^32 - 1 sectors (FatFs without 64-bit LBA) */
bool pc_fmt_plan(uint64_t capacity_bytes, uint32_t sector_size, pc_fmt_plan_t *out);
const char *pc_fmt_fs_name(int fs);                                                          /* "FAT16" "FAT32" "exFAT" ("?") */
/* the MBR: disk signature, one partition (status 0, CHS FE FF FF = LBA only, type mbr_type, part_start, part_sectors), the other three empty, 55 AA */
void pc_fmt_mbr(const pc_fmt_plan_t *p, uint32_t disk_sig, uint8_t sec[512]);
/* the card the warning page showed and the card in the slot when the format starts: the same only when sector count, sector size and CID serial
 * are all equal (and the count is not 0). The board's bsp_sdcard_format and the simulator's fake SD layer both ask this before any write. */
typedef struct { uint64_t sectors; uint32_t sector_size, serial; } pc_fmt_card_t;
bool pc_fmt_same_card(const pc_fmt_card_t *shown, const pc_fmt_card_t *now);
/* how far a format got: BEFORE = nothing written yet (refused or failed before the first write), WRITE = the layout (GPT traces, MBR) was being
 * written, MKFS = f_mkfs / the label, MOUNT = the volume is made, the mount after it ran */
typedef enum { PC_FMT_STAGE_BEFORE, PC_FMT_STAGE_WRITE, PC_FMT_STAGE_MKFS, PC_FMT_STAGE_MOUNT } pc_fmt_stage_t;
/* what the result page says: DONE (formatted and mounted), UNTOUCHED (nothing written: the data is as it was), BROKEN (written, not finished: the
 * card cannot be used now, it can be formatted again), UNMOUNTED (formatted, the mount after it failed). An unknown stage is BROKEN (never "untouched"). */
typedef enum { PC_FMT_OUT_DONE, PC_FMT_OUT_UNTOUCHED, PC_FMT_OUT_BROKEN, PC_FMT_OUT_UNMOUNTED } pc_fmt_out_t;
int pc_fmt_outcome(int stage, bool ok);
const char *pc_fmt_stage_name(int stage);                                                    /* before write mkfs mount ("?") */
const char *pc_fmt_outcome_name(int out);                                                    /* done untouched broken unmounted ("?") */
/* the card's size as printed on it (decimal): "962 MB", "3.9 GB", "31.9 GB", "125 GB" */
void pc_fmt_size_label(uint64_t bytes, char *b, size_t n);

#ifdef __cplusplus
}
#endif
