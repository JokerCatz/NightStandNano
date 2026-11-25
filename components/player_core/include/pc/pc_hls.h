// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* HLS (m3u8) for live radio: a playlist of short segments that keeps growing ("a folder that keeps getting files").
 * The device reads the list, downloads the segments it has not played, and repeats.
 * Copied from JukeDial; NightStandNano additions (marked "Nano"): variant choice by CODECS / RESOLUTION and audio renditions, encryption / fMP4 /
 * AC-3 flags, #EXTINF titles, and a window that starts at a given sequence number (ended lists longer than the window). */
#define PC_HLS_MAX_SEG 16
#define PC_HLS_TITLE 48
#define PC_HLS_URL 1024                  /* signed CDN addresses (CloudFront policies, tokens) run to 700+ characters */
typedef struct {
    bool master;                          /* a list of variants, not of segments */
    char variant[PC_HLS_URL];                    /* master: the chosen variant, absolute */
    uint32_t variant_bps;                 /* Nano: its BANDWIDTH, 0 unknown */
    uint64_t first_seq;                   /* media: #EXT-X-MEDIA-SEQUENCE */
    int target_s;                         /* #EXT-X-TARGETDURATION */
    int nseg;
    char seg[PC_HLS_MAX_SEG][PC_HLS_URL];        /* absolute; the newest PC_HLS_MAX_SEG of the list (or the first ones from `from_seq`) */
    uint64_t seg_seq[PC_HLS_MAX_SEG];
    char title[PC_HLS_MAX_SEG][PC_HLS_TITLE];   /* Nano: the #EXTINF text after the comma ("" when none or a placeholder) */
    bool ended;                           /* #EXT-X-ENDLIST */
    bool more;                            /* Nano: with from_seq, segments after the window are still in the list */
    bool encrypted;                       /* Nano: #EXT-X-KEY with a METHOD other than NONE (AES-128, SAMPLE-AES) */
    bool fmp4;                            /* Nano: #EXT-X-MAP (an fMP4 / CMAF init section) */
    char scratch_line[PC_HLS_URL + 128], scratch_abs[PC_HLS_URL];   /* parser work space (keeps it off the caller's small stack) */
    char cand_best[3][PC_HLS_URL], cand_low[3][PC_HLS_URL], media[PC_HLS_URL];   /* parser work space */
    bool no_audio;                        /* Nano, master: every variant is a codec the box cannot decode (AC-3, E-AC-3, ...) */
} pc_hls_t;
bool pc_hls_is_hls(const char *body);    /* "#EXTM3U" with "#EXT-X-" tags (a plain .m3u has none) */
/* base = the playlist's own address. Variants (Nano): audio-only ones first (no RESOLUTION, no video codec), else an audio rendition
 * (#EXT-X-MEDIA TYPE=AUDIO with a URI), else the rest; variants with AC-3 / E-AC-3 / FLAC / Opus codecs never; among those the highest
 * bandwidth at or under max_bps, else the lowest. */
int  pc_hls_parse(const char *body, const char *base, uint32_t max_bps, pc_hls_t *out);   /* 0 ok; the newest segments of the list */
/* from_seq = UINT64_MAX: as pc_hls_parse. Otherwise the first PC_HLS_MAX_SEG segments numbered from_seq or more (`more` tells when there are others). */
int  pc_hls_parse_from(const char *body, const char *base, uint32_t max_bps, uint64_t from_seq, pc_hls_t *out);
/* index of the first segment to fetch after `last` (UINT64_MAX = start: a few segments back from live;
 * also when the list's numbers went below last - the encoder restarted), -1 none */
int  pc_hls_next(const pc_hls_t *h, uint64_t last);
int  pc_url_resolve(const char *base, const char *ref, char *out, size_t cap);   /* 0 ok */
#ifdef __cplusplus
}
#endif
