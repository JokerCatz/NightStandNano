// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
#include "sdmmc_cmd.h"

/* TF card wiring. Stored by hw.c (settings key "hw_sd"), read once at boot; the defaults come from the board (board/board.h). Two ways to wire a card:
 *   SDMMC  the SD host peripheral. On the ESP32-S31 it is IOMUX-only (fixed pins: slot 0 = GPIO 24 25 20 21 22 23), see docs/porting.md; width 1 or 4
 *   SPI    SD in SPI mode on a GPIO-matrix SPI bus (any free pins), slower (1 bit): for a hand-wired socket on other pins
 * SDMMC uses clk cmd d0..d3 width; SPI uses clk mosi miso cs (the rest stays -1). */
typedef enum { NSN_SD_SDMMC, NSN_SD_SPI } nsn_sdmode_t;
typedef struct { int8_t clk, cmd, d0, d1, d2, d3; uint8_t width; nsn_sdmode_t mode; int8_t mosi, miso, cs; } nsn_sdpins_t;

void nsn_storage_set_pins(const nsn_sdpins_t *p);         /* boot, before the first mount: the wiring this boot runs with (hw.c) */
const nsn_sdpins_t *nsn_storage_pins(void);               /* ... and what it is */

/* Mount at /sdcard, FAT12/16/32 and exFAT, never formats. Returns the reason on failure ("nocard", "fs", ...) via nsn_storage_why(). */
esp_err_t   nsn_storage_mount(void);
const char *nsn_storage_why(void);
bool        nsn_storage_mounted(void);

/* Card state, kept apart from the mount so the page can tell "no card" from "a card the file system cannot read" (only the latter may be formatted, cardfmt.c):
 *   NONE  no card, or the card did not answer at the SDMMC level
 *   OK    mounted
 *   BAD   the card answers (size and serial are known) but no FAT / exFAT volume mounted */
typedef enum { NSN_CARD_NONE, NSN_CARD_OK, NSN_CARD_BAD } nsn_card_state_t;
typedef struct { nsn_card_state_t state; uint64_t bytes; uint32_t serial; char name[8]; } nsn_card_t;
void nsn_storage_card(nsn_card_t *c);                    /* state now; bytes / serial / name when known (state OK or BAD) */
nsn_card_state_t nsn_storage_state(void);
/* free space and file system type of the mounted volume (f_getfree: slow on a big FAT32 card the first time, call from a worker). False when not mounted. */
bool nsn_storage_volume(uint64_t *free_bytes, const char **fs_name);
/* raw sector read below the files, read only (card test); 0 = ok */
int  nsn_storage_read_raw(uint32_t sector, uint32_t n, void *buf);
/* A format owns the card: while held, nsn_storage_mount() does nothing (main.c retries it every 5 s). Waits for a mount in progress. */
void nsn_storage_hold(bool on);
/* A card that answers but does not mount: open it for raw access (the SD host is ours until nsn_storage_raw_close). Refused (INVALID_STATE) while mounted. */
esp_err_t nsn_storage_raw_open(sdmmc_card_t **out);
void      nsn_storage_raw_close(sdmmc_card_t *c);
esp_err_t nsn_storage_mount_now(void);                   /* mount ignoring the hold (the format's own remount) */
uint32_t  nsn_storage_gen(void);                         /* changes with every successful mount */
