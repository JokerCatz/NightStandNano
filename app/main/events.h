// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* A short in-memory list of recent things the box noticed (button presses on a wired headset or on Bluetooth headphones, ...),
 * so the control page can show what arrived without a wired console. RAM only: gone after a reboot.
 *   events            -> one line per entry, oldest first:  ev <seq> <age in seconds> <text>     then  ev end <next seq>
 *   events <seq>      -> only entries with a sequence number >= seq (what is new since the last call) */
void nsn_event(const char *fmt, ...) __attribute__((format(printf, 1, 2)));   /* any task; never blocks for long */
int  nsn_events_get(unsigned from_seq, int idx, unsigned *seq, int *age_s, char *text, int n);   /* idx-th entry >= from_seq; 0 when there is none */
unsigned nsn_events_next_seq(void);
