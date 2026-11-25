// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Wi-Fi station for network radio and the clock.
 *
 * Where the credentials come from (precedence, most deliberate first):
 *   1. NVS keys wifi_ssid / wifi_pass, written by `wifi set` (from the web page over BLE or from the console);
 *   2. the TF card file /sdcard/_player/config/wifi.txt (JukeDial's format: ssid=... / pass=...), used only while nothing is stored.
 * `wifi forget` erases the stored pair and the box falls back to the card file. No credentials at all = Wi-Fi idles.
 * The password is never logged, printed or readable back over any interface; the SSID is not secret but is not logged either
 * (it is shown by the `wifi` status reply and by scans only).
 * BLE control is unencrypted and open by the owner's design, so `wifi set` sends the password in clear over the air: accepted trade-off. */
#include <stdbool.h>
#include "esp_err.h"
typedef struct { signed char rssi; bool open; char ssid[33]; } nsn_ap_t;
esp_err_t nsn_wifi_start(void);        /* idempotent; ESP_ERR_NOT_FOUND when there are no credentials anywhere */
void      nsn_wifi_stop(void);
bool      nsn_wifi_connected(void);
bool      nsn_wifi_enabled(void);       /* started (connected or still trying): the radio is in use */
void      nsn_wifi_boot(void);          /* create the lock, quiet the driver's own logs (they print the network name); call before any other */
void      nsn_wifi_status(char *out, int n);     /* state, ip, rssi, time, src, ssid */
/* NULL = stored and connecting; otherwise the reason it was refused. Separator rule is the caller's: ssid and pass are already split. */
const char *nsn_wifi_set(const char *ssid, const char *pass);
void      nsn_wifi_forget(void);        /* erase the stored pair, reconnect with the card file if there is one */
/* strongest first, at most `max`, one entry per name. Returns the count, -1 if a scan is already running, -2 on failure. */
int       nsn_wifi_scan(nsn_ap_t *out, int max);
