// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* MPEG-TS (HLS segments) -> the audio elementary stream: AAC as ADTS (stream type 0x0F) or MP3 (0x03/0x04).
 * Other streams (timed ID3, ...) are skipped. Bytes may arrive in any chunking. */
typedef struct {
    uint8_t pkt[188];
    int pkt_n;
    int pmt_pid, audio_pid, stream_type;   /* -1 until known */
} pc_ts_t;
void   pc_ts_init(pc_ts_t *t);
size_t pc_ts_feed(pc_ts_t *t, const uint8_t *in, size_t n, uint8_t *out, size_t cap);   /* audio bytes written */
#ifdef __cplusplus
}
#endif
