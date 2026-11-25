// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_rg.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

int pc_rg_parse_gain(const char *s, int16_t *cdb)
{
    char *end;
    double v = strtod(s, &end);
    if (end == s || !isfinite(v) || v < -60.0 || v > 60.0) return -1;
    while (*end == ' ') end++;
    if (*end && strncasecmp(end, "dB", 2)) return -1;
    *cdb = (int16_t)lround(v * 100.0);
    return 0;
}

int pc_rg_parse_peak(const char *s, uint16_t *peak)
{
    char *end;
    double v = strtod(s, &end);
    if (end == s || !isfinite(v) || v <= 0.0 || v > 6.5) return -1;
    long p = lround(v * 10000.0);
    *peak = (uint16_t)(p < 1 ? 1 : p);
    return 0;
}

int pc_rg_tag(pc_rg_t *r, const char *name, const char *value)
{
    if (strncasecmp(name, "replaygain_", 11)) return 0;
    const char *k = name + 11;
    if (!strcasecmp(k, "track_gain")) pc_rg_parse_gain(value, &r->track_cdb);
    else if (!strcasecmp(k, "album_gain")) pc_rg_parse_gain(value, &r->album_cdb);
    else if (!strcasecmp(k, "track_peak")) pc_rg_parse_peak(value, &r->track_peak);
    else if (!strcasecmp(k, "album_peak")) pc_rg_parse_peak(value, &r->album_peak);
    else return 0;
    return 1;
}

float pc_rg_linear(const pc_rg_t *r, int mode, float untagged_db, float *db_out)
{
    float db = 0.0f, g = 1.0f;
    if (mode == PC_RG_TRACK || mode == PC_RG_ALBUM) {
        bool album = mode == PC_RG_ALBUM ? r->album_cdb != PC_RG_NONE : r->track_cdb == PC_RG_NONE && r->album_cdb != PC_RG_NONE;
        int16_t cdb = album ? r->album_cdb : r->track_cdb;
        uint16_t peak = album ? r->album_peak : r->track_peak;
        db = cdb == PC_RG_NONE ? untagged_db : (float)cdb / 100.0f;
        g = powf(10.0f, db / 20.0f);
        if (cdb != PC_RG_NONE && peak && g * (float)peak / 10000.0f > 1.0f) {   /* clip prevention */
            g = 10000.0f / (float)peak;
            db = 20.0f * log10f(g);
        }
    }
    if (db_out) *db_out = db;
    return g;
}
