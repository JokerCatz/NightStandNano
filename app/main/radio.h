// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Internet radio source: an HTTP(S) MP3 / AAC / Ogg stream (Shoutcast/Icecast, ICY titles, .pls/.m3u redirects) or an HLS playlist (TS / packed AAC / MP3
 * segments) read by one network task into a ring, decoded through JukeDial's pc_backend_stream + pc_backend_mp3 / backend_aac. The player treats an http(s):// "file" like any other track. */
#include <stdbool.h>
#include "pc/pc_backend.h"
#include "pc/pc_io.h"
bool nsn_radio_begin(const char *url, const pc_backend_t **be, pc_reader_t *r);   /* blocks until the prebuffer is in (or fails); true = ready to open */
void nsn_radio_end(void);
void nsn_radio_cancel(void);              /* any task: make a connect / prebuffer wait give up now (the decode task then ends the session) */
void nsn_radio_debug_reuse(int on);      /* HLS: 1 (default) keeps the connection between requests, 0 closes it after each one (measurement) */
unsigned nsn_radio_connections(void);    /* connections opened since boot */
const char *nsn_radio_title(void);       /* current ICY title, "" when none */
const char *nsn_radio_state(void);       /* "", "connecting", "buffering", "reconnecting", an error */
int nsn_radio_load(void);                                 /* read /sdcard/_player/config/radio.txt ("name|url[|backup...]"); count */
const char *nsn_radio_station(int i, const char **name); /* i-th station address (name via *name), NULL when out of range */
int nsn_radio_add(const char *name, const char *url);   /* append to radio.txt; station number (also when it was already there), <0 = refused / full / write failed */
int nsn_radio_del(int i);                               /* 0-based; 0 ok */
