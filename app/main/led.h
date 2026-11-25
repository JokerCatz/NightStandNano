// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The board's single addressable RGB LED (GPIO60 and colour order grb on the ESP32-S31 dev boards, set by `hw led` / the board defaults; assumed WS2812-compatible: unverified on this board;
 * with `hw led off` there is no LED task and the pin is left alone).
 * Bedside rules: dark by default, dim when it shows something. */
#include "esp_err.h"
esp_err_t nsn_led_start(void);
void nsn_led_set_level(int level);        /* 0 = off, 1..255 peak brightness (default 12) */
int  nsn_led_level(void);
void nsn_led_set_cap(int cap);            /* night mode: peak brightness never above cap (not saved); -1 = no cap */
void nsn_led_test(int r, int g, int b);   /* hold a colour for 3 s (console: led R G B) */
