// SPDX-License-Identifier: LGPL-2.1-or-later
#include "events.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define N 32
#define LEN 56
static struct { unsigned seq; int64_t us; char text[LEN]; } s_ev[N];
static unsigned s_next = 1;                       /* sequence number of the next entry; entry seq lives at s_ev[seq % N] */
static portMUX_TYPE s_mx = portMUX_INITIALIZER_UNLOCKED;

void nsn_event(const char *fmt, ...)
{
    char b[LEN]; va_list ap;
    va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    int64_t now = esp_timer_get_time();
    portENTER_CRITICAL(&s_mx);
    unsigned q = s_next++;
    s_ev[q % N].seq = q; s_ev[q % N].us = now; memcpy(s_ev[q % N].text, b, LEN);
    portEXIT_CRITICAL(&s_mx);
}

unsigned nsn_events_next_seq(void) { return s_next; }

int nsn_events_get(unsigned from_seq, int idx, unsigned *seq, int *age_s, char *text, int n)
{
    int64_t now = esp_timer_get_time(); int ok = 0;
    portENTER_CRITICAL(&s_mx);
    unsigned first = s_next > N ? s_next - N : 1;
    if (from_seq > first) first = from_seq;
    unsigned q = first + (unsigned)idx;
    if (q < s_next && s_ev[q % N].seq == q) {
        *seq = q; *age_s = (int)((now - s_ev[q % N].us) / 1000000);
        snprintf(text, (size_t)n, "%s", s_ev[q % N].text);
        ok = 1;
    }
    portEXIT_CRITICAL(&s_mx);
    return ok;
}
