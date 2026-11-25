// SPDX-License-Identifier: LGPL-2.1-or-later
/* Commodore 64 SID tunes (PSID and RSID, 1-4 SIDs, 6581 / 8580), through cRSID (components/libcrsid, "WTF" licence).
 * The whole file is read into memory (at most 64 KB + header). Output 44.1 kHz stereo (2 or more SIDs are panned by cRSID's narrow
 * stereo mode, one SID is mono). The rendering is cycle-stepped (6502 + CIA + VIC + SID), cRSID oversampling ratio 2 (see the
 * component's CMakeLists): the host bench put the default ratio over the 60 % rule on the board.
 * A SID tune never ends by itself and the Songlengths database is not on the card: every tune plays pc_chip_length_ms(-1) = 180 s
 * with a 4 s fade at the end, like GME's chips. Sub-tunes = start_track. byte_pos: none (a resume restarts the tune).
 * Voice snapshot (pc_extvoice): 3 per SID (up to 12): level = the envelope generator, pitch from the 16-bit frequency register and
 * the clock (PAL or NTSC), noise = unpitched, strike = the key-on counter; mute = cRSID's per-voice envelope mask. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ccs_player.h"
#include "pc/pc_backend.h"
#include "pc/pc_chipseq.h"
#include "pc/pc_extvoice.h"
#include "pc/pc_vmute.h"

#define RATE 44100u
#define MAX_FILE (64u * 1024u + 512u)
#define FADE_MS 4000

typedef struct {
    ccs_player *p;
    int sub, nsub;
    int32_t len_ms;
    uint64_t played;                       /* frames handed out */
    uint32_t mute_applied;                 /* the mask cRSID has, bit = voice */
    uint32_t clock_hz;                     /* PAL 985248, NTSC 1022727 */
    bool ended;
} sid_t;

static int sid_open(void **hp, const pc_reader_t *r, uint32_t start_off)
{
    (void)start_off;
    if (r->size < 0x76 || r->size > MAX_FILE) return -1;
    uint8_t *buf = malloc(r->size < 1100 ? 1100 : r->size);   /* never a small (internal-RAM) block */
    if (!buf) return -1;
    if (r->read(r->u, 0, buf, r->size) != (int)r->size) { free(buf); return -1; }
    int err = 0;
    ccs_player *p = ccs_open(buf, r->size, (int)RATE, &err);   /* copies the data */
    free(buf);
    if (!p) return -1;
    sid_t *s = calloc(1, 1100);
    if (!s) { ccs_close(p); return -1; }
    s->p = p;
    s->nsub = ccs_subtunes(p);
    s->sub = ccs_default_subtune(p);
    s->len_ms = pc_chip_length_ms(-1);
    s->clock_hz = ccs_is_pal(p) ? 985248u : 1022727u;
    if (ccs_start(p, s->sub)) { ccs_close(p); free(s); return -1; }
    *hp = s;
    return 0;
}

/* -2 = a file the player cannot run (an RSID that is a BASIC program: no BASIC ROM), -1 = not a SID file at all, 0 = playable. */
int pc_sid_check(const pc_reader_t *r)
{
    if (!r || !r->read || r->size < 0x76 || r->size > MAX_FILE) return -1;
    uint8_t *buf = malloc(r->size < 1100 ? 1100 : r->size);
    if (!buf) return -1;
    int rc = -1;
    if (r->read(r->u, 0, buf, r->size) == (int)r->size) {
        int err = 0;
        ccs_player *p = ccs_open(buf, r->size, (int)RATE, &err);
        if (p) { ccs_close(p); rc = 0; }
        else if (err == CCS_ERR_BASIC) rc = -2;
    }
    free(buf);
    return rc;
}

static int sid_tracks(void *hp) { return ((sid_t *)hp)->nsub; }

static int sid_start_track(void *hp, int idx)
{
    sid_t *s = hp;
    if (idx < 0 || idx >= s->nsub) return -1;
    if (ccs_start(s->p, idx)) return -1;
    s->sub = idx;
    s->played = 0;
    s->ended = false;
    return 0;
}

static void apply_mute(sid_t *s, uint32_t mask)
{
    int n = ccs_voice_count(s->p);
    for (int i = 0; i < n; i++) {
        uint32_t bit = 1u << i;
        if ((mask ^ s->mute_applied) & bit) ccs_mute_voice(s->p, i, (mask & bit) != 0);
    }
    s->mute_applied = mask & ((1u << n) - 1u);
}

static void voices_block(sid_t *s, pc_voices_t *v, uint32_t mask)
{
    int n = ccs_voice_count(s->p);
    for (int i = 0; i < n && i < PC_VOICES_MAX; i++) {
        ccs_voice_t cv;
        if (ccs_voice(s->p, i, &cv)) continue;
        unsigned lv = (mask >> i & 1) || cv.test || !cv.waveform ? 0u : (unsigned)cv.envelope;
        if (i % 3 == 2 && cv.off3) lv = 0;                 /* voice 3 cut from the mix by the volume register */
        v->level[i] = pc_voice_decay(v->level[i], lv > 255u ? 255 : (uint8_t)lv);
        v->strikes[i] = (uint8_t)cv.key_on_count;
        v->pan[i] = PC_PAN_UNKNOWN;
        if (cv.waveform & 8) { v->cents[i] = PC_VOICE_NOPITCH; v->flags[i] = PC_VF_NOISE; }     /* noise, alone or in a combination */
        else if (cv.freq_reg > 0 && cv.waveform && !cv.test) {
            long hz100 = (long)((int64_t)cv.freq_reg * s->clock_hz * 100 >> 24);
            v->cents[i] = (int16_t)pc_voice_cents_of_hz_x100(hz100);
            v->flags[i] = v->cents[i] == PC_VOICE_NOPITCH ? 0 : PC_VF_PITCH;
        } else { v->cents[i] = PC_VOICE_NOPITCH; v->flags[i] = 0; }
    }
}

static int sid_render(void *hp, float *out, int frames)
{
    sid_t *s = hp;
    if (s->ended) return 0;
    pc_voices_t *v = pc_extvoice_get(s);
    if (!v) v = pc_extvoice_take(s, ccs_voice_count(s->p));
    uint32_t mask = pc_vmute_mask();
    apply_mute(s, mask);
    int total = (int)((int64_t)s->len_ms * RATE / 1000);
    int left = total - (int)s->played;
    if (left <= 0) { s->ended = true; return 0; }
    if (frames > left) frames = left;
    short pcm[2 * 441];
    int done = 0;
    while (done < frames) {
        int n = frames - done > 441 ? 441 : frames - done;
        ccs_render(s->p, pcm, n);
        for (int i = 0; i < n; i++) {
            float g = 1.0f / 32768.0f;
            int rem = total - (int)(s->played + (uint64_t)(done + i));
            if (rem < (int)(RATE / 1000 * FADE_MS)) g *= (float)rem / (float)(RATE / 1000 * FADE_MS);
            out[2 * (done + i)] = (float)pcm[2 * i] * g;
            out[2 * (done + i) + 1] = (float)pcm[2 * i + 1] * g;
        }
        done += n;
    }
    s->played += (uint64_t)done;
    if (v) voices_block(s, v, mask);
    return done;
}

static uint32_t sid_rate(void *hp) { (void)hp; return RATE; }
static int32_t sid_len(void *hp) { return ((sid_t *)hp)->len_ms; }
static int32_t sid_pos(void *hp) { sid_t *s = hp; return (int32_t)(s->played * 1000 / RATE); }
static void sid_close(void *hp)
{
    sid_t *s = hp;
    pc_extvoice_release(s);
    ccs_close(s->p);
    free(s);
}

const pc_backend_t pc_backend_sid = {
    .open = sid_open, .render = sid_render, .rate = sid_rate, .track_count = sid_tracks, .start_track = sid_start_track,
    .length_ms = sid_len, .position_ms = sid_pos, .close = sid_close, .byte_pos = NULL, .name = "SID",
};
