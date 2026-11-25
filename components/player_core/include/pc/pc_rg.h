// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* ReplayGain values from a file's tags (ideas 2026-11-10 #2), stored in the library index.
 * Gains in 0.01 dB (PC_RG_NONE = no tag); peaks as sample peak * 10000 (0 = no tag). */
#define PC_RG_NONE INT16_MIN
typedef struct { int16_t track_cdb, album_cdb; uint16_t track_peak, album_peak; } pc_rg_t;
typedef enum { PC_RG_OFF = 0, PC_RG_TRACK = 1, PC_RG_ALBUM = 2 } pc_rg_mode_t;   /* cfg "rg" */

static inline void pc_rg_clear(pc_rg_t *r) { r->track_cdb = r->album_cdb = PC_RG_NONE; r->track_peak = r->album_peak = 0; }
/* "-6.53 dB", "+1.2dB", "-12": 0 and *cdb; -1 when not a number or beyond +-60 dB */
int pc_rg_parse_gain(const char *s, int16_t *cdb);
/* "0.988739": 0 and *peak (x10000); -1 when not a number, <= 0 or above 6.5 */
int pc_rg_parse_peak(const char *s, uint16_t *peak);
/* tag name (any case) -> stores the value; 1 when the name was a ReplayGain one */
int pc_rg_tag(pc_rg_t *r, const char *name, const char *value);
/* The linear gain for a track: mode picks track or album gain (the other one when it is missing),
 * untagged_db is used for a file without either; the result is limited so peak * gain <= 1 when the peak
 * that goes with the chosen gain is known. *db_out (may be NULL) gets the gain in dB. Off -> 1. */
float pc_rg_linear(const pc_rg_t *r, int mode, float untagged_db, float *db_out);
#ifdef __cplusplus
}
#endif
