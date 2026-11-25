// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* chipchill sound presets (format = 1), the subset this device plays (spec 4.6 / 5.5).
 * A node: kind, enabled, mix, params = { id = value }, equalizer = "<Equalizer APO text>". */
#define PC_NODE_MAX_PARAMS 24
#define PC_EQ_MAX 10
typedef struct { float fc, gain_db, q; char type; /* 'P' peak, 'L' low shelf, 'H' high shelf */ } pc_eq_band_t;
typedef struct {
    char kind[20];
    bool enabled, supported;
    float mix;
    char pname[PC_NODE_MAX_PARAMS][20];
    float pval[PC_NODE_MAX_PARAMS];
    int np;
    pc_eq_band_t eq[PC_EQ_MAX];
    int neq;
    float eq_preamp_db;
} pc_node_t;
typedef struct {
    char name[48];
    pc_node_t music[16]; int nmusic;
    pc_node_t master[8]; int nmaster;
    struct { int from, to; float level; pc_node_t node; } sends[4]; int nsends;   /* to = -1: end of music */
    char unsupported[8][40]; int nunsup;   /* shown as "not on this device" */
} pc_preset_t;
/* supported kinds: every app of the fx registry (pc_fxapp_find); other kinds and nodes using only_for / group / of / impulse are
 * kept with supported = false. Where a node is placed (music, master, send) does not decide whether it is supported. */
int   pc_preset_parse(const char *toml_text, pc_preset_t *out);   /* 0, or <0 for bad TOML / format != 1 */
float pc_node_param(const pc_node_t *n, const char *id, float def);
/* allocator for the TOML parser (the device routes it to PSRAM) */
void  pc_preset_set_alloc(void *(*alloc)(size_t), void (*release)(void *));
int   pc_autoeq_parse(const char *text, pc_eq_band_t *bands, int max, float *preamp_db);   /* bands parsed (OFF lines skipped) */
#ifdef __cplusplus
}
#endif
