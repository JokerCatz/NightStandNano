// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* BLE GATT control service: Web Bluetooth page and phone app talk to the box through three characteristics.
 *   CMD  (write, write-no-response)  UTF-8 text line, same commands as the UART console (cmd.c)
 *   EVT  (notify)                    the command's reply text, one '\n'-terminated line per command line, split to the MTU
 *   SPEC (notify, read)              [seq, NSN_BANDS, band0..band31] about 30 times a second while subscribed
 * Service UUID 7f2a0001-a1c4-4e53-9a31-6e73616e6f00, CMD ...0002..., EVT ...0003..., SPEC ...0004... (same tail). */
#include "esp_err.h"
esp_err_t nsn_ble_start(void);
#include "esp_gap_ble_api.h"
void nsn_ble_set_gap_hook(void (*hook)(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *));   /* GAP events that are not the control service's own (the Auracast broadcaster) */
#include <stdbool.h>
void nsn_ble_quiesce(bool on);            /* true: drop the client and stop advertising (blocks until it is gone); false: advertise again */
bool nsn_ble_connected(void);
