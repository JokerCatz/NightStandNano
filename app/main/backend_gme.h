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
/* Why the last backend_gme open failed, in GME's words ("Unsupported player type", "Wrong file type", ...); "" when it did not or the reason is unknown. */
const char *backend_gme_error(void);
/* Tags of the tune playing in handle h (NSF / GBS / SAP / AY / HES / VGM GD3 ...): song, author, game as GME gives them (any may be ""). Same task as render. */
void backend_gme_tags(void *h, char *song, size_t sn, char *author, size_t an, char *game, size_t gn);
/* MSX-MUSIC (YM2413, emu2413) for KSS files that have it: ON by default (Kss_Emu::fm_off is zero-initialised; 33-37 % of core 1 measured in JukeDial); this turns it off (0) or on (1). */
void backend_gme_kss_fm(int on);
#ifdef __cplusplus
}
#endif
