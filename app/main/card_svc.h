// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Card diagnostics service: the card speed test (card_test.c, from JukeDial) driven from one persistent worker task, the card_hw_* glue it needs,
 * and the one-line card facts. Console and BLE both reach it through cmd.c. */
#include <stdbool.h>
#include <stddef.h>
#include "cmd.h"

void nsn_card_svc_start(void);                 /* the worker (PSRAM stack): free space per mount, the saved results, the tests; call once the card mount was tried */

/* NULL: the test started; else the refusal reason ("no card", "already running", "small", "nospace", ...). kind: "raw" or "file". Waits for the worker's verdict. */
const char *nsn_cardtest_start(const char *kind);
bool        nsn_cardtest_cancel(void);                 /* false: nothing was running */
bool        nsn_cardtest_running(void);                /* the LED shows it */
/* status and the latest results, one line each (format documented at the `cardtest` handler in cmd.c) */
void        nsn_cardtest_report(nsn_out_t out, void *ctx);
/* "card state=mounted size=... free=... fs=... name=\"...\" serial=... fmt=idle" */
void        nsn_card_line(char *out, size_t n);
