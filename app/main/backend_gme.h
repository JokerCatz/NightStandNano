// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
#include "pc/pc_backend.h"
#include "pc/pc_voice.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Game_Music_Emu over a memory reader (the extracted zip entry). The data must stay alive
 * until close (GME keeps pointers into it for some formats). 44.1 kHz. */
extern const pc_backend_t backend_gme;
/* Info-only parse for the UI (core 0): track count and names. 0 on success. */
int backend_gme_probe(const void *data, size_t size, int *tracks, char *system, size_t ssys, char *game, size_t sgame);
/* song names of the first max tracks (empty string when untitled); returns the count written */
int backend_gme_track_names(const void *data, size_t size, char (*names)[64], int max);
/* Audio task writes, any task reads: copies the per-voice level snapshot of the file being played; returns n (0: none, or a type
 * without an adapter: GYM). */
int backend_gme_voices(pc_voices_t *out);
/* MSX-MUSIC (YM2413, emu2413) for KSS files that have it: off by default (patch 0009: 0.9 us of a host core per output sample, over the board budget); the benches and the test switch it on. */
void backend_gme_kss_fm(int on);
#ifdef __cplusplus
}
#endif
