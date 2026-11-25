// SPDX-License-Identifier: LGPL-2.1-or-later
/* DOS AdLib / OPL music (RAD, HSC, IMF, DRO, CMF, A2M, MID/IMS/..., ~45 formats) through AdPlug 2.4 (components/libadplug) and its fmopl
 * 0.37a OPL2 emulator (one chip, two when the file uses a second one: chip 0 left, chip 1 right). Output 44.1 kHz. The song plays once
 * (AdPlug's own loop detection ends it); the length is a silent scan at open (-1 when the song did not end within 120,000 ticks, then
 * it plays to its end or 10 minutes). The file extension (pc_reader_t.ext) picks the player first: IMF needs it for its tick rate.
 * Sub-songs = start_track. byte_pos: none. Voice snapshot (pc_extvoice): 9 or 18 OPL channels from the register shadow: level =
 * the audible operator's total level while the key is on, pitch from fnum + block, rhythm-mode drums (bass drum pitched, the others
 * noise); mute = key-on masked and release forced to 15 for that channel. A file that needs OPL3 four-operator voices is refused. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "adplug_c.h"
#include "pc/pc_backend.h"
#include "pc/pc_extvoice.h"
#include "pc/pc_vmute.h"

#define RATE 44100u
#define MAX_FILE (2u * 1024u * 1024u)
#define MAX_PLAY_MS (10 * 60 * 1000)

typedef struct {
    adp_t *a;
    int32_t len_ms;
    uint64_t played;
    int nsub;
    bool ended;
} opl_t;

static int read_all(const pc_reader_t *r, uint8_t **out)
{
    if (r->size < 4 || r->size > MAX_FILE) return -1;
    uint8_t *buf = malloc(r->size < 1100 ? 1100 : r->size);   /* never a small (internal-RAM) block */
    if (!buf) return -1;
    if (r->read(r->u, 0, buf, r->size) != (int)r->size) { free(buf); return -1; }
    *out = buf;
    return 0;
}

static int opl_open(void **hp, const pc_reader_t *r, uint32_t start_off)
{
    (void)start_off;
    uint8_t *buf;
    if (read_all(r, &buf)) return -1;
    int err = 0;
    adp_t *a = adp_open(buf, r->size, r->ext, (int)RATE, &err);
    free(buf);
    if (!a) return -1;
    opl_t *o = calloc(1, 1100);
    if (!o) { adp_close(a); return -1; }
    o->a = a;
    o->nsub = adp_subsongs(a);
    o->len_ms = adp_length_ms(a);
    *hp = o;
    return 0;
}

/* 0 playable; -2 needs four-operator OPL3 voices; -3 an A2M / A2T packed with aPlib (versions 9-11, no depacker); -4 damaged (a size field out of range, reads far past the end); -1 anything else */
int pc_opl_check(const pc_reader_t *r)
{
    if (!r || !r->read) return -1;
    uint8_t h[24];
    if (r->size >= sizeof h && r->read(r->u, 0, h, sizeof h) == (int)sizeof h) {
        int off = !memcmp(h, "_A2module_", 10) ? 14 : !memcmp(h, "_A2tiny_module_", 15) ? 19 : 0;
        if (off && h[off] >= 9 && h[off] <= 11) return -3;
    }
    uint8_t *buf;
    if (read_all(r, &buf)) return -1;
    int err = 0;
    adp_t *a = adp_open(buf, r->size, r->ext, (int)RATE, &err);
    free(buf);
    if (a) { adp_close(a); return 0; }
    return err == ADP_ERR_4OP ? -2 : (err == ADP_ERR_DAMAGED || err == ADP_ERR_MEMORY) ? -4 : -1;
}

static int opl_tracks(void *hp) { return ((opl_t *)hp)->nsub; }

static int opl_start_track(void *hp, int idx)
{
    opl_t *o = hp;
    if (adp_start(o->a, idx)) return -1;
    o->played = 0;
    o->ended = false;
    o->len_ms = adp_length_ms(o->a);
    return 0;
}

static void voices_block(opl_t *o, pc_voices_t *v)
{
    int n = adp_voice_count(o->a);
    for (int i = 0; i < n && i < PC_VOICES_MAX; i++) {
        adp_voice_t av;
        if (adp_voice(o->a, i, &av)) continue;
        v->level[i] = pc_voice_decay(v->level[i], av.level);
        v->strikes[i] = (uint8_t)av.strikes;
        v->pan[i] = PC_PAN_UNKNOWN;
        if (av.noise) { v->cents[i] = PC_VOICE_NOPITCH; v->flags[i] = PC_VF_NOISE; }
        else if (av.key_on && av.hz_x100 > 0) {
            v->cents[i] = (int16_t)pc_voice_cents_of_hz_x100(av.hz_x100);
            v->flags[i] = v->cents[i] == PC_VOICE_NOPITCH ? 0 : PC_VF_PITCH;
        } else { v->cents[i] = PC_VOICE_NOPITCH; v->flags[i] = 0; }
    }
}

static int opl_render(void *hp, float *out, int frames)
{
    opl_t *o = hp;
    if (o->ended) return 0;
    pc_voices_t *v = pc_extvoice_get(o);
    if (!v) v = pc_extvoice_take(o, adp_voice_count(o->a));
    adp_set_mute(o->a, pc_vmute_mask());
    if (o->played >= (uint64_t)MAX_PLAY_MS * RATE / 1000) { o->ended = true; return 0; }
    short pcm[2 * 441];
    int done = 0;
    while (done < frames) {
        int n = frames - done > 441 ? 441 : frames - done;
        int g = adp_render(o->a, pcm, n);
        if (g < 0) { if (done) break; return -1; }         /* four-operator voices turned up mid-song */
        for (int i = 0; i < g; i++) {
            out[2 * (done + i)] = (float)pcm[2 * i] * (1.0f / 32768.0f);
            out[2 * (done + i) + 1] = (float)pcm[2 * i + 1] * (1.0f / 32768.0f);
        }
        done += g;
        if (g < n) { o->ended = true; break; }              /* the song's end */
    }
    o->played += (uint64_t)done;
    if (v) voices_block(o, v);
    return done;
}

static uint32_t opl_rate(void *hp) { (void)hp; return RATE; }
static int32_t opl_len(void *hp) { return ((opl_t *)hp)->len_ms; }
static int32_t opl_pos(void *hp) { opl_t *o = hp; return (int32_t)(o->played * 1000 / RATE); }
static void opl_close(void *hp)
{
    opl_t *o = hp;
    pc_extvoice_release(o);
    adp_close(o->a);
    free(o);
}

const pc_backend_t pc_backend_opl = {
    .open = opl_open, .render = opl_render, .rate = opl_rate, .track_count = opl_tracks, .start_track = opl_start_track,
    .length_ms = opl_len, .position_ms = opl_pos, .close = opl_close, .byte_pos = NULL, .name = "OPL",
};
