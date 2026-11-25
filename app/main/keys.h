// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Headset line-control buttons (through the USB DAC dongle's HID interface): up to three buttons, each with four gestures, each gesture
 * mapped to one simple action. The map is the user's (NVS), with presets for 1-, 2- and 3-button headsets.
 *   buttons   c = centre (play/pause key), p = plus, m = minus
 *   gestures  1 = one click, 2 = two clicks, 3 = three clicks, h = hold
 *   actions   none play next prev vol+ vol- stop       (play = play / pause; resumes the bookmark when idle; vol on hold repeats)
 * A button whose 2 / 3-click gestures are all "none" acts at once on the press; otherwise a click waits 400 ms for the next one. */
#include <stdbool.h>
enum { NSN_KEY_CENTER, NSN_KEY_PLUS, NSN_KEY_MINUS, NSN_KEY_N };
void nsn_keys_start(void);
void nsn_keys_event(int key, bool down);                       /* from the USB side (any task) */
bool nsn_keys_set(char button, char gesture, const char *action);   /* false = unknown name */
void nsn_keys_preset(int buttons);                             /* 1, 2 or 3 */
void nsn_keys_line(int key, char *out, int n);                 /* "keys c 1=play 2=next 3=prev h=none" */
void nsn_keys_last(char *out, int n);                          /* "keys last c2 next 3 s ago" / "keys last none": to learn what a headset sends */
