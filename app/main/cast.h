// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Auracast broadcast source (LE Audio BAP broadcast source + PBP), ported from JukeDial's bt_svc_cast_tx.c without its UI and services.
 * Stereo as two BIS (left / right), LC3 48 kHz 10 ms frames, unencrypted, broadcast name "NightStand Nano".
 * It is one of the player's outputs (`out cast`): the encoder task pulls 10 ms of PCM from the player every 10 ms.
 * Exclusive with Bluetooth headphone output (both encoders want core 1). LE Audio cannot be shut down again once started:
 * stopping the broadcast removes the streams and the advertising, the LE Audio host stays until the next reboot. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
typedef size_t (*nsn_cast_pull_t)(int16_t *stereo, size_t frames, void *ctx);
esp_err_t nsn_cast_start(void);                  /* bring the broadcast up (first call also starts LE Audio); blocks up to a few seconds */
void      nsn_cast_stop(void);
bool      nsn_cast_on(void);
uint32_t  nsn_cast_rate(void);                   /* 48000 */
void      nsn_cast_set_source(nsn_cast_pull_t pull, void *ctx);   /* NULL = silence */
void      nsn_cast_status(char *out, int n);     /* frames sent, underflows, LC3 cost, send errors */
