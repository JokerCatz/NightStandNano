// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Exact-length wrappers for the simple-decoder formats (backend_sdec.c is JukeDial's file and stays verbatim; these sit in front of it).
 * The encoders add priming at the start and padding at the end of a file; the containers say how many frames are real:
 *   Ogg Vorbis: the last page's granule position = the frame count.
 *   Ogg Opus:   the last page's granule position (48 kHz units) minus the pre-skip in the OpusHead packet.
 *   M4A:        iTunSMPB (delay + sample count) when the file has it, else the edit list (elst media_time = priming, segment duration = length),
 *               refined to the exact frame count from the media header's duration when that agrees to within one movie-timescale tick.
 * When the container does not say (raw ADTS AAC, M4A without either record, damaged files) the wrapper passes the decoder's output through unchanged.
 * It also gives the track a length (length_ms), which the bare decoders do not. */
#include <stdbool.h>
#include "pc/pc_backend.h"
extern const pc_backend_t backend_ogg_trim, backend_m4a_trim;
void nsn_trim_enable(bool on);          /* debug (UART `dbg trim on|off`): off = the wrappers pass the decoder through, to measure what trimming changes */
bool nsn_trim_enabled(void);
