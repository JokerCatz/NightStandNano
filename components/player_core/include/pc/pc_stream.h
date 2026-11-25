// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include "pc/pc_backend.h"
#include "pc/pc_ring.h"
#ifdef __cplusplus
extern "C" {
#endif
/* A network stream as a backend: a decoder (inner) reads the ring that the network task fills.
 * While the ring holds less than PC_STREAM_LOW bytes (and the source has not ended) it outputs silence
 * and waits until `prebuffer` bytes are in again: a stall is a pause, not the end of the track. */
#define PC_STREAM_LOW 8192
typedef struct pc_stream {
    pc_ring_t *ring;
    const pc_backend_t *inner;
    uint32_t prebuffer;
    volatile bool eof;          /* set by the network side: nothing more will come */
    /* state */
    void *ih;
    pc_reader_t rd;
    uint8_t head[64];           /* the first bytes, for decoders that look at the start again */
    uint32_t head_n, consumed;
    bool buffering;
    uint32_t underruns;
} pc_stream_t;
/* audio_svc_play(&pc_backend_stream, r, 0, 0, ...) with r->u = the pc_stream_t (other reader fields unused) */
extern const pc_backend_t pc_backend_stream;
#ifdef __cplusplus
}
#endif
