// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* White-noise scenes: up to 3 looping sounds mixed with per-layer volume (sleepmixer's scenes). Files on the card:
 *   /sdcard/_player/scenes/scenes.txt   "name|file.ogg:vol|file.ogg:vol|..."  one scene per line
 *   /sdcard/_player/scenes/<file>       the sounds (ogg / mp3), loaded whole into PSRAM and looped
 * A track path "scene:N" (1-based) plays scene N. tools/make_scenes.py builds both from sleepmixer's library.muk. */
#include <stdbool.h>
#include "pc/pc_backend.h"
extern const pc_backend_t nsn_scene_backend;
int nsn_scene_load(void);                       /* scene count (re-reads scenes.txt) */
const char *nsn_scene_name(int i);              /* 0-based, NULL out of range */
void nsn_scene_debug_looplen(int ms);           /* UART test: every layer's pass ends after ms (0 = off), to hear / measure seams without waiting minutes */
void nsn_scene_debug_spare(bool on);            /* UART test: pc_loop's second decoder instead of the in-place rewind (applies from the next scene start) */
void nsn_scene_debug_add(const char *spec);     /* UART test: one more scene, "name|file.ogg:vol|..." (RAM only) */
void nsn_scene_debug_xfade(int ms);            /* UART test: crossfade length of the layer loops (default 50; 0 = a cut), from the next scene start */
