// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include "pc/pc_paths.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Station list /sdcard/_player/config/radio.txt: "name|url[|backup url[|backup url]]" per line, '#' comments. The name ends at the first bar that is followed by
 * http:// or https:// (names may hold bars); a line with no such field keeps the old rule: the address starts after the last bar. */
#define PC_STATION_ALTS 2
typedef struct { char name[64]; char url[256]; char alt[PC_STATION_ALTS][256]; } pc_station_t;   /* alt: backup addresses, "" = none */
int  pc_station_urls(const pc_station_t *s);              /* 1 + the backups that are set */
const char *pc_station_url(const pc_station_t *s, int k);  /* k-th address (0 = the main one); NULL when out of range */
/* https policy: http is preferred. Which address of the station starts the play: the first http one; with none, the first https one when tls_ok, else -1.
 * An https address whose http twin (same host, path; scheme http, :443 dropped) answers is played over http: pc_url_http_twin makes that address. */
bool pc_url_is_https(const char *url);
bool pc_url_is_web(const char *url);                      /* http:// or https:// */
int  pc_station_pick(const pc_station_t *s, bool tls_ok);
int  pc_url_http_twin(const char *https_url, char *out, size_t cap);   /* length, -1 when not https, too small, or the address has user:password@ or a ?query (those never go out in the clear) */
/* a copy of the address for a log line: scheme, host and path only (no userinfo, no query, no fragment). length, -1 too small */
int  pc_url_redact(const char *url, char *out, size_t cap);
enum { PC_TWIN_UNKNOWN, PC_TWIN_OK, PC_TWIN_BAD };
#define PC_TRY_STOP 2                                         /* pc_try_fn: the attempt ended without a verdict (a new request came, a finished list): not a failure, nothing follows */
typedef int (*pc_try_fn)(void *ctx, const char *url);       /* 1 = this address played (2xx, audio, no redirect back to https), 0 = it did not, PC_TRY_STOP */
int  pc_https_play(uint8_t *cache, const char *url, char *used, size_t cap, pc_try_fn try, void *ctx);   /* 1 played, PC_TRY_STOP stopped; used = the address that did */
int  pc_radio_parse(const char *text, pc_station_t *out, int max);       /* stations read */
int  pc_radio_format(const pc_station_t *s, int n, char *out, size_t cap);   /* length, -1 too small */
/* body of a .pls or .m3u playlist -> the first http URL, else the first https one */
bool pc_playlist_first_url(const char *body, char *out, size_t cap);
typedef enum { PC_FMT_UNKNOWN, PC_FMT_MP3, PC_FMT_AAC, PC_FMT_PLAYLIST, PC_FMT_OGG, PC_FMT_TS } pc_fmt_t;
int pc_radio_prebuffer(int icy_br_kbps);   /* bytes: 2 s at the bitrate, 8..96 KB; 32 KB when unknown (0 or > 1000) */
/* the stream bytes themselves: an Ogg page ("OggS"), or 3 back-to-back MP3 or ADTS frames (servers mislabel AAC as audio/mpeg) */
pc_fmt_t pc_radio_sniff(const unsigned char *b, size_t n);
pc_fmt_t pc_radio_guess(const char *content_type, const char *url);    /* either may be NULL */
/* the connection diagnosis the player shows for a failed try: what the HTTP client gave (connected / status / errno) and the address */
typedef enum { PC_RDIAG_OK, PC_RDIAG_TIMEOUT, PC_RDIAG_FAILED, PC_RDIAG_REFUSED, PC_RDIAG_STATUS, PC_RDIAG_UNSUPPORTED, PC_RDIAG_HTTPS } pc_rdiag_t;   /* PC_RDIAG_HTTPS: set by the player only, https is refused in this mode (memory) */
/* the wait before the next try: 1 s while another backup address of this round is still untried (mid_round), else 1 s doubling with every
 * full round of failures (fails / n_urls), up to 30 s, as with a single address */
int pc_radio_backoff_ms(int fails, int n_urls, bool mid_round);
pc_rdiag_t pc_radio_diag(const char *url, bool connected, int http_status, int os_errno, bool bytes_known_bad);   /* bytes_known_bad: the stream is a format the decoders cannot play */

/* several station lists on the card: radio.txt (stem "") and radio_<stem>.txt; the one shown is cfg "radio_list", else the language's default */
#define PC_RADIO_STEM_MAX 24
#define PC_RADIO_DIR PC_CFG_DIR
bool pc_radio_stem_ok(const char *stem);                              /* "" (the main list) or 1..23 of a-z 0-9 _ - */
int  pc_radio_list_path(const char *stem, char *out, size_t cap);     /* <dir>/radio.txt or <dir>/radio_<stem>.txt; -1 bad stem or too small */
const char *pc_radio_default_stem(int lang);                          /* pc_lang_t: 簡中 "cn", else "" */
int  pc_radio_scan_lists(const char *dir, char (*stems)[PC_RADIO_STEM_MAX], int max);   /* the lists found in dir, the main one first; count */

#ifdef __cplusplus
}
#endif
