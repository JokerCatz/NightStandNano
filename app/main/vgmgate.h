// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Before a VGM goes to game-music-emu: GME opens files of chips it does not emulate (YM2151, YM2203, ...) and plays digital silence without a word.
 * 0: hand it over (every chip in the file is one GME plays, a declared chip is never written, or the data is no VGM and GME may say why).
 * -1: refuse; why = the chips ("YM2151, OKIM6295"). `vgm` is the whole un-gzipped file. */
int nsn_vgm_gate(const uint8_t *vgm, size_t len, char *why, size_t n);
#ifdef __cplusplus
}
#endif
