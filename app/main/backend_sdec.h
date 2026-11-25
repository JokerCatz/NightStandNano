// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "pc/pc_backend.h"
/* Network radio decoders through esp_audio_codec's simple decoder (it finds the frames itself):
 * AAC / HE-AAC in ADTS, Ogg (Vorbis or Opus). HLS TS segments are demuxed to ADTS first (pc_ts). */
extern const pc_backend_t backend_aac, backend_ogg;
/* library .m4a (AAC or ALAC in MP4), through the same simple decoder; always starts at 0 */
extern const pc_backend_t backend_m4a;
