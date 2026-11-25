// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_loop.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK 256
#define MAX_EMPTY 3          /* consecutive seams without a single decoded frame -> error */

struct pc_loop {
    pc_layer_src_t src;
    void *a, *b;
    uint32_t rate;
    int xf;                  /* crossfade frames */
    float *q;                /* A's decoded samples not yet output (FIFO, stereo) */
    int qcap, qlen, qhead;
    float *ready;            /* crossfaded frames waiting to be output */
    int rlen, rpos;
    float tmp[2 * CHUNK];
    int a_done, seams, empty, a_frames;
};

static void q_push(pc_loop_t *l, const float *s, int n)
{
    for (int i = 0; i < n; i++) {
        int at = (l->qhead + l->qlen) % l->qcap;
        l->q[2 * at] = s[2 * i];
        l->q[2 * at + 1] = s[2 * i + 1];
        l->qlen++;
    }
}

static void q_pop(pc_loop_t *l, float *out, int n)
{
    for (int i = 0; i < n; i++) {
        out[2 * i] = l->q[2 * l->qhead];
        out[2 * i + 1] = l->q[2 * l->qhead + 1];
        l->qhead = (l->qhead + 1) % l->qcap;
    }
    l->qlen -= n;
}

static void fill(pc_loop_t *l, int need)
{
    while (!l->a_done && l->qlen < l->xf + need) {
        int space = l->qcap - l->qlen, want = space < CHUNK ? space : CHUNK;
        if (want <= 0) break;
        int n = l->src.read(l->a, l->tmp, want);
        if (n <= 0) { l->a_done = 1; break; }
        l->a_frames += n;
        q_push(l, l->tmp, n);
    }
}

/* A ended and q holds its last <= xf frames: blend them with the next pass's first frames.
 * The next pass is A itself, rewound, when the source can; otherwise the pre-opened B, which becomes A. */
static int seam(pc_loop_t *l)
{
    int t = l->qlen;
    q_pop(l, l->ready, t);
    l->empty = l->a_frames ? 0 : l->empty + 1;
    int reused = l->src.rewind && l->src.rewind(l->a) == 0;
    if (!reused) {
        if (!l->b) {                                          /* no spare decoder: open one now */
            uint32_t r;
            if (l->src.open(l->src.ctx, &l->b, &r)) { l->b = NULL; return -1; }
        }
        l->src.close(l->a);
        l->a = l->b;
        l->b = NULL;
    }
    for (int i = 0; i < t;) {
        int want = t - i < CHUNK ? t - i : CHUNK;
        int n = l->src.read(l->a, l->tmp, want);
        if (n <= 0) { memset(l->tmp, 0, sizeof l->tmp); n = want; }
        for (int k = 0; k < n; k++, i++) {
            float th = 1.5707963f * (float)i / (float)(t ? t : 1);
            float ga = cosf(th), gb = sinf(th);
            l->ready[2 * i] = l->ready[2 * i] * ga + l->tmp[2 * k] * gb;
            l->ready[2 * i + 1] = l->ready[2 * i + 1] * ga + l->tmp[2 * k + 1] * gb;
        }
    }
    l->rlen = t;
    l->rpos = 0;
    l->a_done = 0;
    l->a_frames = 0;
    l->seams++;
    if (!reused && !l->src.rewind) {                          /* keep a spare ready for the next seam */
        uint32_t r;
        if (l->src.open(l->src.ctx, &l->b, &r)) { l->b = NULL; return -1; }
    }
    return l->empty >= MAX_EMPTY ? -1 : 0;
}

pc_loop_t *pc_loop_create(const pc_layer_src_t *src, uint32_t xfade_ms)
{
    pc_loop_t *l = calloc(1, sizeof *l);
    if (!l) return NULL;
    l->src = *src;
    uint32_t r2;
    if (src->open(src->ctx, &l->a, &l->rate)) { free(l); return NULL; }
    if (!src->rewind && src->open(src->ctx, &l->b, &r2)) { src->close(l->a); free(l); return NULL; }
    l->xf = (int)(l->rate * xfade_ms / 1000u);
    if (l->xf < 1) l->xf = 1;
    l->qcap = l->xf + 2048;
    l->q = malloc((size_t)l->qcap * 2 * sizeof(float));
    l->ready = malloc((size_t)l->xf * 2 * sizeof(float));
    if (!l->q || !l->ready) { pc_loop_destroy(l); return NULL; }
    return l;
}

int pc_loop_render(pc_loop_t *l, float *out, int frames)
{
    int done = 0;
    while (done < frames) {
        if (l->rpos < l->rlen) {                              /* crossfaded frames first */
            int k = l->rlen - l->rpos < frames - done ? l->rlen - l->rpos : frames - done;
            memcpy(out + 2 * done, l->ready + 2 * l->rpos, (size_t)k * 2 * sizeof(float));
            l->rpos += k;
            done += k;
            continue;
        }
        int need = frames - done < 2048 ? frames - done : 2048;
        fill(l, need);
        if (l->qlen > l->xf) {                                /* keep xf frames of A in reserve */
            int k = l->qlen - l->xf < need ? l->qlen - l->xf : need;
            q_pop(l, out + 2 * done, k);
            done += k;
            continue;
        }
        if (!l->a_done) continue;                             /* fill made progress */
        if (seam(l)) return -1;
    }
    return done;
}

uint32_t pc_loop_rate(const pc_loop_t *l) { return l->rate; }
int pc_loop_seams(const pc_loop_t *l) { return l->seams; }

void pc_loop_destroy(pc_loop_t *l)
{
    if (!l) return;
    if (l->a) l->src.close(l->a);
    if (l->b) l->src.close(l->b);
    free(l->q);
    free(l->ready);
    free(l);
}
