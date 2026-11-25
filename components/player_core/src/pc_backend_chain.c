// SPDX-License-Identifier: LGPL-2.1-or-later
/* One backend for every suffix that DOS OPL music (AdPlug) or tracker modules (libxmp) can own, as the single table in pc_chipseq.c says (tier 2026-11-18).
 * open() tries the backends that list the suffix in the table's column order (AdPlug first: its open is a content probe and refuses what is not its format,
 * then libxmp) and keeps the first that opens; a backend whose table cell is experimental is skipped while the setting 實驗性格式 is off. Nothing is
 * decoded here: every other call goes straight to the backend that opened. The handle is one 1.1 KB block (never a small internal-RAM block). */
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"
#include "pc/pc_chipseq.h"

typedef struct { const pc_backend_t *be; void *h; } chain_t;

static int c_open(void **hp, const pc_reader_t *r, uint32_t start_off)
{
    int opl = 0, xmp = 0;
    pc_ext_backends(r->ext, &opl, &xmp);
    const pc_backend_t *try_[2];
    int n = 0;
    if (opl && pc_tier_allowed(opl)) try_[n++] = &pc_backend_opl;
    if (xmp && pc_tier_allowed(xmp)) try_[n++] = &pc_backend_xmp;
    chain_t *c = n ? calloc(1, 1100) : NULL;
    if (!c) return -1;
    for (int i = 0; i < n; i++) {
        void *h = NULL;
        if (try_[i]->open(&h, r, start_off) == 0) { c->be = try_[i]; c->h = h; *hp = c; return 0; }
    }
    free(c);
    return -1;
}
static int c_render(void *h, float *s, int f) { chain_t *c = h; return c->be->render(c->h, s, f); }
static uint32_t c_rate(void *h) { chain_t *c = h; return c->be->rate(c->h); }
static int c_tracks(void *h) { chain_t *c = h; return c->be->track_count ? c->be->track_count(c->h) : 1; }
static int c_start_track(void *h, int i) { chain_t *c = h; return c->be->start_track ? c->be->start_track(c->h, i) : -1; }
static int32_t c_length(void *h) { chain_t *c = h; return c->be->length_ms ? c->be->length_ms(c->h) : -1; }
static int32_t c_position(void *h) { chain_t *c = h; return c->be->position_ms ? c->be->position_ms(c->h) : 0; }
static void c_close(void *h) { chain_t *c = h; if (!c) return; c->be->close(c->h); free(c); }
static uint32_t c_byte_pos(void *h) { chain_t *c = h; return c->be->byte_pos ? c->be->byte_pos(c->h) : 0; }
static uint8_t c_bits(void *h) { chain_t *c = h; return c->be->bits ? c->be->bits(c->h) : 0; }
static uint32_t c_burst(void *h) { chain_t *c = h; return c->be->burst ? c->be->burst(c->h) : 0; }

const pc_backend_t pc_backend_chain = {
    .open = c_open, .render = c_render, .rate = c_rate, .track_count = c_tracks, .start_track = c_start_track,
    .length_ms = c_length, .position_ms = c_position, .close = c_close, .byte_pos = c_byte_pos, .name = NULL,
    .bits = c_bits, .burst = c_burst,
};

const pc_backend_t *pc_backend_resolve(const pc_backend_t *be, void *h)
{
    return be == &pc_backend_chain && h ? ((chain_t *)h)->be : be;
}
