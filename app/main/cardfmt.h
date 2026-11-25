// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Formatting the TF card (port of JukeDial's cardfmt, sd_format.c / bsp_sdcard_format), owner ruling 2026-11-25:
 *   - offered ONLY for a card that answers at the SDMMC level but has no readable file system (storage.h: NSN_CARD_BAD). A card that mounts, or no card, is refused
 *     at every step; there is no way on any interface to format a card that mounts.
 *   - three steps, two codes: request -> code 1 -> code 2 -> started; each within 30 s of the previous reply; anything else cancels the whole request.
 *   - the format runs on the "cardfmt" worker (PSRAM stack, persistent, work area in PSRAM): MBR + f_mkfs + label, then a remount and the folder skeleton.
 * The core (format_drive) takes the FatFs drive as a parameter: the real card, or the RAM disk of the self-test and of the UART-only simulation. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ff.h"
#include "cmd.h"

#define NSN_RAMDISK_SECTORS 16384          /* 8 MiB of 512 B sectors (exfat_selftest.c) */
bool nsn_ramdisk_open(BYTE *pdrv);         /* a fresh zeroed RAM disk registered as a FatFs drive; false when one is open or no drive is free */
void nsn_ramdisk_close(BYTE pdrv);

#define NSN_FMT_LABEL "NIGHTSTAND"

typedef enum { NSN_FMT_IDLE, NSN_FMT_PENDING, NSN_FMT_RUNNING, NSN_FMT_DONE, NSN_FMT_FAILED } nsn_fmt_state_t;

/* Step 1. NULL: accepted; plan_line is "fs=exFAT label=... size=N" (text after "cardfmt plan "), *code the first code. Else the refusal reason (nothing pending). */
const char *nsn_cardfmt_request(char *plan_line, size_t n, int *code);
/* Steps 2 and 3. NULL: accepted; *next is the second code, or 0 when the format has started. Else the refusal reason: the request is cancelled. */
const char *nsn_cardfmt_confirm(int code, int *next);
void        nsn_cardfmt_cancel(void);
nsn_fmt_state_t nsn_cardfmt_state(void);              /* pending ends by itself after 30 s */
void        nsn_cardfmt_status(char *out, size_t n);  /* "cardfmt state idle|pending|running|done|failed <reason>" */
const char *nsn_cardfmt_state_word(void);             /* idle pending running done failed */
int         nsn_cardfmt_led(void);                    /* 0 nothing, 1 warning (code 1 pending), 2 urgent (code 2 pending) */
/* UART-only developer switches (cmd.c checks "local") */
void        nsn_cardfmt_sim(bool on);                 /* pretend an unreadable 8 MiB card; a confirmed format goes to the RAM disk, never to the SD slot; off at every boot */
bool        nsn_cardfmt_sim_on(void);
bool        nsn_cardfmt_selftest(nsn_out_t out, void *ctx);   /* RAM-disk format tests (exFAT and FAT16 plans); prints PASS / FAIL lines; true = all passed */
