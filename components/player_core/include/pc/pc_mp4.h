// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "pc/pc_io.h"
/* M4A files with the index (moov) after the audio (mdat) - ffmpeg's default - are refused by
 * esp_audio_codec's M4A parser. pc_mp4_faststart presents such a file as ftyp, moov, the rest,
 * with the chunk offsets patched (what `ffmpeg -movflags faststart` writes), without touching the file.
 * Returns 1 (out reads the rearranged file; free *st after out is done), 0 (nothing to do: use src as is,
 * *st = NULL), <0 (a damaged or too large moov). */
typedef struct pc_mp4_fs pc_mp4_fs_t;
int  pc_mp4_faststart(const pc_reader_t *src, pc_reader_t *out, pc_mp4_fs_t **st);
void pc_mp4_faststart_free(pc_mp4_fs_t *st);
