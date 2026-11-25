// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The ESP-IDF side of fmvgm_port_t: memory from heap_caps (FAST = internal RAM while it is plentiful, BULK = PSRAM), a persistent worker task with its stack in PSRAM
 * (xTaskCreatePinnedToCoreWithCaps: it never deletes itself), task notifications for wake / wait, esp_timer for the clock. Call once at boot (or before the first open). */
#ifdef __cplusplus
extern "C" {
#endif
void fmvgm_port_install_idf(void);
/* largest internal block that must remain free after a FAST allocation (default 24 KB): a request that would leave less goes to PSRAM (the chip state is then about 35 % slower, measured) */
void fmvgm_port_set_internal_reserve(unsigned bytes);
#ifdef __cplusplus
}
#endif
