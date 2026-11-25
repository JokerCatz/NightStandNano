// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Seamless looping layer (spec 5.5): output runs xfade_ms behind the decoder; when the decoder ends,
 * its last xfade_ms fade out (cos) over the first xfade_ms of a second, pre-opened decoder (sin).
 * That decoder carries on and a fresh one is opened for the next seam.
 * If the source can rewind, the ended decoder is rewound and reused instead: one decoder, and no open
 * (header parse, tens of ms) on the audio thread at a seam. A failed rewind falls back to an open. */
typedef struct {
    int  (*open)(void *ctx, void **dec, uint32_t *rate);   /* from the start; 0 ok */
    int  (*read)(void *dec, float *stereo, int frames);    /* 0 = end */
    void (*close)(void *dec);
    int  (*rewind)(void *dec);                             /* optional: back to the first frame; 0 ok */
    void *ctx;
} pc_layer_src_t;
typedef struct pc_loop pc_loop_t;
pc_loop_t *pc_loop_create(const pc_layer_src_t *src, uint32_t xfade_ms);   /* NULL if the first opens fail */
int  pc_loop_render(pc_loop_t *l, float *stereo, int frames);  /* fills frames; <0 when the source cannot produce audio */
uint32_t pc_loop_rate(const pc_loop_t *l);
int  pc_loop_seams(const pc_loop_t *l);
void pc_loop_destroy(pc_loop_t *l);
#ifdef __cplusplus
}
#endif
