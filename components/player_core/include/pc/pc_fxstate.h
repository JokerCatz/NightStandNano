// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The sound as device state (spec docs/archive/specs/2026-11-13-fx-apps-design.md §4, §7): one value table over the
 * registry (on, every parameter, the chain-level 濃度), its kv form (/sdcard/_player/config/fx.txt) and its TOML form (import
 * of a chipchill preset, export). Pure C, no ESP-IDF; host-tested (host_test/test_fxstate.c). */
#include <stdbool.h>
#include <stddef.h>
#include "pc/pc_fxapp.h"
#include "pc/pc_kv.h"
#include "pc/pc_preset.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PC_FXSTATE_MIX (-1)   /* the param index of the chain-level 濃度 */
typedef struct {
    bool on[PC_FXAPP_MAX];
    float v[PC_FXAPP_MAX][PC_FXP_MAX];
    float mix[PC_FXAPP_MAX];      /* the chain-level 濃度: meaningful only for PC_FXA_MIX apps, 1 for every other app, never written to kv */
    char state_from[48];          /* the preset file of the last stored import; "" after any other change (a label only) */
} pc_fxstate_t;

void  pc_fxstate_defaults(pc_fxstate_t *s);                                 /* every app off, every value its def, mix 1, state_from "" */
float pc_fxstate_clamp(int app, int param, float v);                        /* NaN/inf -> def; into lo..hi; CHOICE/BOOL rounded; param PC_FXSTATE_MIX: 0..1 (NaN -> 1) */
/* kv keys: format=1, state_from, <app>.on, <app>.<param> (not BAND params), <app>.mix (the chain-level 濃度 of a PC_FXA_MIX app;
 * the own "mix" parameter of an app that has one is the same key), equalizer.b<n>=<P|L|H> <fc> <gain> <q> (one key per band) */
int   pc_fxstate_to_kv(const pc_fxstate_t *s, pc_kv_t *kv, int max);        /* pairs written (floats as the shortest text that reads back identical), -1 when max is too small */
void  pc_fxstate_from_kv(pc_fxstate_t *s, const pc_kv_t *kv, int n);        /* defaults first; unknown keys ignored; missing = default; clamped; format present and not 1 = defaults */
/* The whole sound from a parsed preset: apps the file does not name are off (defaults); the first enabled node of a kind sets
 * its app, equalizer nodes merge (bands up to 10, preamps summed); state_from is left alone. Reports (not on this device,
 * "<kind> (重複)", "<kind> mix", "<kind>.<param>") go to unsup; returns how many (at most max). */
int   pc_fxstate_import(pc_fxstate_t *s, const pc_preset_t *p, char (*unsup)[40], int max);
int   pc_fxstate_export(const pc_fxstate_t *s, char *out, size_t n);        /* TOML format = 1, every app in registry order; bytes (without the NUL), -1 when n is too small */
#ifdef __cplusplus
}
#endif
