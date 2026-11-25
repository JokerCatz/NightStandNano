// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Hardware settings that are needed at init and so live in NVS, not on the card: the TF card wiring and the status LED (GPIO and colour order).
 * The defaults belong to the board (board/board.h). They are read once at boot (nsn_hw_load); a change applies after a reboot (`hw pending 1`).
 * The one place that changes them is the `hw` command, and only on the wired console: over BLE the same command reads. docs/porting.md. */
#include <stdbool.h>
#include "cmd.h"
#include "storage.h"

typedef struct { int8_t gpio; char order[4]; } nsn_hw_led_t;     /* gpio -1 = no LED; order = wire order of the colours: rgb grb brg rbg gbr bgr */

void nsn_hw_load(void);                       /* boot, after nsn_settings_load and before storage / led: reads the stored settings, hands the SD wiring to storage */
const nsn_hw_led_t *nsn_hw_led(void);         /* what this boot runs with */
bool nsn_hw_order_ok(const char *order);
void nsn_hw_exec(const char *args, bool local, nsn_out_t out, void *ctx);    /* the `hw` command; args may be NULL */
void nsn_hw_sdpins_legacy(const char *args, bool local, nsn_out_t out, void *ctx);   /* the older `sdpins` command, same rules */
