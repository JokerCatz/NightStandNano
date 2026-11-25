// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Classic Bluetooth audio on Bluedroid with external codecs (esp_audio_codec): one A2DP role at a time, switchable (the stack allows only one).
 *   speaker mode    (A2DP sink)   a phone streams SBC/AAC to us; decoded PCM is read by the player as the source "bt:in"
 *   headphones mode (A2DP source) our audio goes to headphones / a speaker (SBC, AAC; LDAC and aptX later)
 * Bluedroid, the BLE control service and the controller stay up in both modes. */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

typedef size_t (*nsn_a2dp_pull_t)(int16_t *dst, size_t frames, void *ctx);   /* stereo S16 at the link rate */
typedef struct { uint8_t bda[6]; char name[33]; int8_t rssi; uint32_t cod; bool audio; } nsn_bt_dev_t;
typedef enum { NSN_BT_SPEAKER = 0, NSN_BT_HEADPHONES = 1 } nsn_bt_mode_t;

esp_err_t nsn_bt_init(void);                              /* stack up, role from settings */
esp_err_t nsn_bt_set_mode(nsn_bt_mode_t m);               /* async (worker); persists */
nsn_bt_mode_t nsn_bt_mode(void);
bool      nsn_bt_mode_ready(void);                        /* the A2DP role of the current mode is up */
void      nsn_bt_pairable(int seconds);                   /* speaker mode: discoverable for phones for this long (0 = stop) */
int       nsn_bt_forget_all(void);                        /* drop every bond (returns how many) */

/* headphones mode */
esp_err_t nsn_bt_scan(void);                              /* inquiry ~8 s */
int       nsn_bt_scan_results(nsn_bt_dev_t *out, int max);
bool      nsn_bt_scanning(void);
esp_err_t nsn_bt_connect(const uint8_t bda[6]);
esp_err_t nsn_bt_disconnect(void);
void      nsn_bt_set_source(nsn_a2dp_pull_t pull, void *ctx);   /* NULL = silence */
esp_err_t nsn_bt_start(void);                             /* AVDTP start when connected */
esp_err_t nsn_bt_suspend(void);
uint32_t  nsn_bt_rate(void);                              /* negotiated rate of the headphones link */
const char *nsn_bt_codec_name(void);                      /* "SBC" / "AAC" / "" */
bool      nsn_bt_streaming(void);
bool      nsn_bt_connected(void);                         /* headphones mode: a sink is connected */
void      nsn_bt_peer(uint8_t out[6]);

/* speaker mode: what a phone sends */
bool      nsn_bt_in_connected(void);                      /* a phone is connected */
bool      nsn_bt_in_streaming(void);                      /* and is playing */
uint32_t  nsn_bt_in_rate(void);
size_t    nsn_bt_in_read(int16_t *stereo, size_t frames);  /* non-blocking, frames actually read */
size_t    nsn_bt_in_fill(void);                           /* frames waiting */
void      nsn_bt_in_flush(void);
void      nsn_bt_set_in_callback(void (*cb)(bool streaming));  /* phone started / stopped streaming (BTC task context: only post work) */

void      nsn_bt_status(char *out, int n);
void      nsn_bt_ldac_selftest(char *res, int n);                  /* encoder check without a peer */
