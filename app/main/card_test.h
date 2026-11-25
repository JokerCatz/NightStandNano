// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "pc/pc_cardtest.h"
/* 記憶卡診斷 (cardtest, owner 2026-11-20): the service behind the 記憶卡診斷 page (ui/scr_cardtest.c) and the console "cardtest".
 * Two tests, each its own result (kept until reboot, and saved on the card in /sdcard/_player/cardtest.txt: the latest per mode, see pc_cardtest.h):
 *   RAW  read only, straight from the block device (bsp_sdcard_read_raw: FatFs' disk_read on the mounted drive, below the files): three regions
 *        (start, middle, near the end of the card), each a 16 KB-chunk pass (what a decoder asks for, the headline) and a 64 KB-chunk pass
 *        (the card's maximum), about 2 MB or 1.5 s of read time per pass. Works on a full card. Never writes.
 *   FILE writes /sdcard/_player/cardtest.tmp (4 MB of a fixed pattern), reads it back through pc_reader_file (what the decoders get through the
 *        filesystem), verifies, deletes it. Needs 8 MB free. Never touches another file; never formats.
 * The work is time-sliced: card_test_step() does about 30 ms of card time and is called from the page's timer (LVGL task) or, for the console on a
 * page-less run, in a loop. Playback stops when a test starts. Buffers are PSRAM and live only while a test runs; the state is PSRAM. */
enum { CT_RAW, CT_FILE, CT_KINDS };
enum { CTS_NONE, CTS_RUN, CTS_DONE, CTS_FAIL };
enum { CTF_NONE, CTF_NOCARD, CTF_SMALL, CTF_NOSPACE, CTF_IO, CTF_NOMEM };
#define CT_REGIONS 3

typedef struct {
    uint8_t state, fail;
    uint8_t pct;                          /* progress while running */
    uint32_t reg16_x100[CT_REGIONS];      /* RAW: MB/s x 100 per region, 16 KB chunks */
    uint32_t warm16_x100[CT_REGIONS];     /* RAW: the first (warm-up) 16 KB pass of each region, MB/s x 100: reported, not used (the second pass is reg16_x100) */
    uint32_t reg_sector[CT_REGIONS];
    uint32_t avg16_x100, slow16_x100;     /* all regions together; the slowest region */
    uint32_t max64_x100;                  /* 64 KB chunks, all regions */
    uint32_t worst_us;                    /* the slowest single 16 KB read */
    uint32_t w_x100, r_x100;              /* FILE: write, read-back (16 KB through pc_reader_file) */
    bool verify_ok;
    bool saved;                           /* loaded from the card's result file: the numbers of an earlier run ("last"), until a new run replaces them */
    bool save_failed;                     /* the run is done but its result could not be written to the card (the numbers still show) */
    char when[20];                        /* saved: when it ran ("YYYY-MM-DDTHH:MM", "-" = clock not set) */
    uint64_t bytes;
} ct_result_t;

/* ---- the lower half: the board's app/main/card_hw.c, the simulator's sim/fake_sd.c ---- */
typedef struct { uint64_t capacity, free_bytes; char name[8]; uint32_t freq_khz, serial; const char *fs; } ct_info_t;
bool card_hw_info(ct_info_t *i);                              /* false: no card mounted (fs NULL: unknown) */
int  card_hw_read(uint32_t sector, uint32_t n, void *buf);    /* raw read, 0 = ok */
int64_t card_hw_now_us(void);                                 /* the clock the tests are timed by */
void card_hw_account(uint32_t bytes, bool write);             /* a file chunk moved (the simulator's clock advances by the fake card's speed; the board: nothing) */

/* ---- the service (nano: only card_svc.c calls these, from its worker task, under its mutex) ---- */
bool card_test_start(int kind);                               /* false: refused, result.state FAIL with .fail; playback is stopped first */
void card_test_step(void);                                    /* a slice of the running test (no-op when none) */
void card_test_cancel(void);                                  /* a running test is dropped (the temp file removed); its previous result stays */
bool card_test_running(void);
int  card_test_kind(void);                                    /* the kind running now, or -1 */
const ct_result_t *card_test_result(int kind);
/* the two scores, MB/s x 100, never merged: raw = the slowest region's 16 KB speed, file = the read-back through the filesystem (0: no result, a saved "last run" counts).
 * The table's marks follow the file one when it exists, else the raw one (pc_ct_follow). */
void card_test_speeds(uint32_t *raw, uint32_t *file);
void card_test_load(void);                                    /* the earlier results of THIS card from /sdcard/_player/cardtest.txt (modes not run this session); another card's lines are ignored */
void card_test_print(int kind);                               /* APP cardtest lines of a finished test */
