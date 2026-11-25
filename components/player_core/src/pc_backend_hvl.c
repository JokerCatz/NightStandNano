// SPDX-License-Identifier: LGPL-2.1-or-later
/* HivelyTracker modules: AHX (THX) and HVL, through the BSD-3 replayer in components/libhvl. The whole file is read into a padded
 * buffer and its structure is checked before the replayer sees it (the replayer trusts every count in the header, see
 * hvl_validate). Output 44.1 kHz: the replayer renders one 50 Hz frame (882 stereo samples) at a time, handed out in blocks of any
 * size. The song plays once (the replayer reports when it returns to its restart position); the length comes from running the
 * song logic (no mixing) once at open. byte_pos = the song position (order) being played; open() starts there. Sub-songs =
 * start_track. Voice snapshot (pc_extvoice): level, pitch from the Amiga period + wave length, strike, pan; mute = a mask the mixer honours. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "hvl_replay.h"
#include "hvl_tables.h"
#include "pc/pc_backend.h"
#include "pc/pc_extvoice.h"
#include "pc/pc_vmute.h"

#define RATE 44100u
#define FRAME (RATE / 50)                 /* 882 stereo samples per 50 Hz frame */
#define MAX_FILE (512u * 1024u)
#define PAD 64u
#define DRY_MAX_IRQ 20000                 /* the length scan stops here (400 s at speed 1) */
#define PAULA_HZ 3546895L                 /* PAL Paula clock / 2: the replayer steps waves at PAULA_HZ / period samples a second */

typedef struct {
    struct hvl_tune *ht;
    uint8_t *file;                        /* padded copy, kept for the re-parse after the length scan and for sub-songs */
    uint32_t file_len;
    int16_t pcm[2 * FRAME];               /* one decoded frame */
    int avail, used;                      /* stereo samples in pcm / consumed */
    int32_t length_ms;
    uint32_t played;                      /* stereo samples handed out */
    int sub;
    bool ended;
    uint32_t armed;                       /* strike hysteresis, bit i */
    uint32_t start_pos;
    uint32_t resume;                      /* a resume position handed to open(): applied after the start_track that follows it (render clears it) */
    bool tabs;                            /* this tune holds a reference on the wave tables (kept across the scan and the re-parse) */
} hvl_t;

static uint16_t be16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }

/* 0 = structure is safe to parse. Mirrors the size walks of hvl_load_ahx / hvl_load_hvl with a bounds check at every step. */
static int hvl_validate(const uint8_t *b, uint32_t len)
{
    if (len < 16) return -1;
    bool ahx = !memcmp(b, "THX", 3) && b[3] < 3;
    bool hvl = !memcmp(b, "HVL", 3) && b[3] < 2;
    if (!ahx && !hvl) return -1;
    uint32_t posn = (uint32_t)((b[6] & 0x0f) << 8 | b[7]), insn = b[12], ssn = b[13], trkl = b[10], trkn = b[11];
    uint32_t chn = ahx ? 4u : (uint32_t)(b[8] >> 2) + 4u;
    if (posn < 1 || posn > 1000 || trkl < 1 || trkl > 64 || insn > 64 || chn > MAX_CHANNELS) return -1;
    if (hvl && b[15] > 4) return -1;                         /* default stereo mode indexes a 5-entry table */
    if (be16(b + 4) >= len) return -1;                       /* the song name */
    uint64_t o = ahx ? 14u : 16u;
    o += (uint64_t)ssn * 2 + (uint64_t)posn * chn * 2;
    bool skip0 = (b[6] & 0x80) != 0;
    if (ahx) {
        o += (uint64_t)trkn * trkl * 3 + (skip0 ? 0u : (uint64_t)trkl * 3);
    } else {
        for (uint32_t i = skip0 ? 1u : 0u; i <= trkn; i++)
            for (uint32_t j = 0; j < trkl; j++) {
                if (o >= len) return -1;
                o += b[o] == 0x3f ? 1u : 5u;
            }
    }
    for (uint32_t i = 1; i <= insn; i++) {
        if (o + 22 > len) return -1;
        o += 22 + (uint64_t)b[o + 21] * (ahx ? 4u : 5u);
    }
    return o <= len ? 0 : -1;
}

static void tune_free(hvl_t *h)
{
    if (h->ht) { hvl_FreeTune(h->ht); h->ht = NULL; }
    if (h->tabs) { hvl_TablesRelease(); h->tabs = false; }
}

static void tune_drop(hvl_t *h)                              /* the tune only: the tables stay (the re-parse after the length scan) */
{
    if (h->ht) { hvl_FreeTune(h->ht); h->ht = NULL; }
}

static int tune_load(hvl_t *h, int sub)
{
    if (!h->ht) {
        if (!h->tabs) {
            if (!hvl_TablesAcquire()) return -1;
            h->tabs = true;
        }
        h->ht = hvl_ParseTune(h->file, h->file_len, RATE, 2);
        if (!h->ht) { tune_free(h); return -1; }
        for (int i = 1; i <= h->ht->ht_InstrumentNr; i++)    /* wave lengths above 5 do not exist: the replayer indexes with them */
            if (h->ht->ht_Instruments[i].ins_WaveLength > 5) h->ht->ht_Instruments[i].ins_WaveLength = 5;
    }
    return hvl_InitSubsong(h->ht, (uint32_t)sub) ? 0 : -1;
}

/* the song logic without the mixer: how long until the song comes back to its restart position */
static int32_t scan_length(hvl_t *h)
{
    struct hvl_tune *ht = h->ht;
    int irqs = 0;
    while (!ht->ht_SongEndReached && irqs < DRY_MAX_IRQ) { hvl_play_irq(ht); irqs++; }
    if (!ht->ht_SongEndReached) return -1;
    return (int32_t)((int64_t)irqs * 1000 * 1 / (50 * ht->ht_SpeedMultiplier));
}

static int hvl_open(void **hp, const pc_reader_t *r, uint32_t start_off)
{
    if (r->size < 16 || r->size > MAX_FILE) return -1;
    hvl_t *h = calloc(1, sizeof *h);   /* > 1 KiB: PSRAM */
    if (!h) return -1;
    h->file = calloc(1, r->size + PAD < 1100 ? 1100 : r->size + PAD);   /* never a small (internal-RAM) block */
    if (!h->file || r->read(r->u, 0, h->file, r->size) != (int)r->size || hvl_validate(h->file, r->size)) { free(h->file); free(h); return -1; }
    h->file_len = r->size;
    h->armed = 0xFFFFu;                                      /* every voice may strike */
    if (tune_load(h, 0)) { tune_free(h); free(h->file); free(h); return -1; }
    h->length_ms = scan_length(h);
    tune_drop(h);                                            /* the scan used the song up: parse it again (tables kept) */
    if (tune_load(h, 0)) { tune_free(h); free(h->file); free(h); return -1; }
    if (start_off > 0 && start_off < h->ht->ht_PositionNr) { h->ht->ht_PosNr = (int16_t)start_off; h->start_pos = start_off; h->resume = start_off; }
    *hp = h;
    return 0;
}

static int hvl_tracks(void *hp) { hvl_t *h = hp; return h->ht->ht_SubsongNr + 1; }

static int hvl_start_track(void *hp, int idx)
{
    hvl_t *h = hp;
    if (idx < 0 || idx > h->ht->ht_SubsongNr) return -1;
    tune_drop(h);
    if (tune_load(h, idx)) return -1;
    h->length_ms = scan_length(h);
    tune_drop(h);
    if (tune_load(h, idx)) return -1;
    h->sub = idx;
    h->avail = h->used = 0;
    h->played = 0;
    h->ended = false;
    h->start_pos = 0;
    if (h->resume && h->resume < h->ht->ht_PositionNr) { h->ht->ht_PosNr = (int16_t)h->resume; h->start_pos = h->resume; }   /* a resume inside a sub-song */
    h->resume = 0;
    return 0;
}

static void voices_frame(hvl_t *h, pc_voices_t *v)
{
    struct hvl_tune *ht = h->ht;
    uint32_t mask = ht->ht_MuteMask;
    for (int i = 0; i < ht->ht_Channels && i < PC_VOICES_MAX; i++) {
        const struct hvl_voice *vc = &ht->ht_Voices[i];
        unsigned lv = vc->vc_TrackOn && !(mask >> i & 1) ? (unsigned)vc->vc_VoiceVolume * 255u / 64u : 0u;
        uint8_t now = lv > 255u ? 255 : (uint8_t)lv;
        uint8_t arm = (uint8_t)(h->armed >> i & 1);
        if (pc_voice_level_strike(now, &arm)) v->strikes[i]++;
        h->armed = (h->armed & ~(1u << i)) | (uint32_t)arm << i;
        v->level[i] = pc_voice_decay(v->level[i], now);
        if (vc->vc_Waveform == 4 - 1) { v->cents[i] = PC_VOICE_NOPITCH; v->flags[i] = PC_VF_NOISE; }
        else if (vc->vc_VoicePeriod > 0 && vc->vc_WaveLength <= 5) {
            long hz100 = (long)((int64_t)PAULA_HZ * 100 / ((int64_t)vc->vc_VoicePeriod << (2 + vc->vc_WaveLength)));
            v->cents[i] = (int16_t)pc_voice_cents_of_hz_x100(hz100);
            v->flags[i] = v->cents[i] == PC_VOICE_NOPITCH ? 0 : PC_VF_PITCH;
        } else { v->cents[i] = PC_VOICE_NOPITCH; v->flags[i] = 0; }
        int pn = (int)vc->vc_Pan - 128;
        v->pan[i] = (int8_t)(pn < -127 ? -127 : pn > 127 ? 127 : pn);
    }
}

static int hvl_render(void *hp, float *out, int frames)
{
    hvl_t *h = hp;
    h->resume = 0;
    pc_voices_t *v = pc_extvoice_get(h);
    if (!v && !h->ended) v = pc_extvoice_take(h, h->ht->ht_Channels);
    if (v) h->ht->ht_MuteMask = pc_vmute_mask();
    int done = 0;
    while (done < frames) {
        if (h->used >= h->avail) {
            if (h->ended || h->ht->ht_SongEndReached) { h->ended = true; break; }
            hvl_DecodeFrame(h->ht, (int8 *)h->pcm, (int8 *)h->pcm + 2, 4);
            h->avail = (int)(RATE / 50 / h->ht->ht_SpeedMultiplier * h->ht->ht_SpeedMultiplier);   /* what hvl_DecodeFrame renders: 880 at x4 */
            h->used = 0;
            if (v) voices_frame(h, v);
        }
        int n = h->avail - h->used;
        if (n > frames - done) n = frames - done;
        for (int i = 0; i < n; i++) {
            out[2 * (done + i)] = (float)h->pcm[2 * (h->used + i)] * (1.0f / 32768.0f);
            out[2 * (done + i) + 1] = (float)h->pcm[2 * (h->used + i) + 1] * (1.0f / 32768.0f);
        }
        h->used += n;
        h->played += (uint32_t)n;
        done += n;
    }
    return done;
}

static uint32_t hvl_rate(void *hp) { (void)hp; return RATE; }
static int32_t hvl_len(void *hp) { return ((hvl_t *)hp)->length_ms; }
static int32_t hvl_pos(void *hp) { hvl_t *h = hp; return (int32_t)((uint64_t)h->played * 1000 / RATE); }
static uint32_t hvl_bytepos(void *hp) { hvl_t *h = hp; return h->played ? (uint32_t)h->ht->ht_PosNr : h->start_pos; }
static void hvl_close(void *hp)
{
    hvl_t *h = hp;
    pc_extvoice_release(h);
    tune_free(h);
    free(h->file);
    free(h);
}

const pc_backend_t pc_backend_hvl = {
    .open = hvl_open, .render = hvl_render, .rate = hvl_rate, .track_count = hvl_tracks, .start_track = hvl_start_track,
    .length_ms = hvl_len, .position_ms = hvl_pos, .close = hvl_close, .byte_pos = hvl_bytepos, .name = "HVL",
};
