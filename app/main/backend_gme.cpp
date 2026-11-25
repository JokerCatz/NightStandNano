// SPDX-License-Identifier: LGPL-2.1-or-later
#include "backend_gme.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include "gme.h"
#include "Ay_Emu.h"
#include "Gbs_Emu.h"
#include "Hes_Emu.h"
#include "Kss_Emu.h"
#include "Nsf_Emu.h"
#include "Nes_Fds_Apu.h"
#include "Nes_Fme7_Apu.h"
#include "Nes_Mmc5_Apu.h"
#include "Nes_Namco_Apu.h"
#include "Nes_Vrc7_Apu.h"
#include "Nes_Vrc6_Apu.h"
#include "Kss_Scc_Apu.h"
#include "Sap_Emu.h"
#include "Spc_Emu.h"
#include "Vgm_Emu.h"
#include "pc/pc_chipseq.h"
#include "pc/pc_voice.h"
#include "pc/pc_vmute.h"
#if __has_include("esp_attr.h")
#include "esp_attr.h"
#endif
#ifndef EXT_RAM_BSS_ATTR
#define EXT_RAM_BSS_ATTR
#endif

/* The YM2612 core is chosen by a define the gme component exports (components/gme/CMakeLists.txt); if it ever stopped reaching this
 * file, Vgm_Emu's members would sit at other offsets and every peek would read the wrong bytes without a build error. */
static_assert(std::is_same<Ym2612_Emu, Ym2612_GENS_Emu>::value, "gme headers must see VGM_YM2612_GENS (same layout as the library)");

#define RATE 44100

struct gme_h { Music_Emu *emu; int track; int32_t len_ms; uint32_t mask; };   /* mask: the voice mute mask gme was given (pc_vmute) */

/* --- per-voice levels ---
 * One snapshot (322 bytes in PSRAM since the 2026-11-17 voice snapshot v2: pitch, key-on counter, pan; 32 voices), written by the audio task after every gme_play, read by the
 * UI task through backend_gme_voices(). The UI never touches the emulator: gme_delete runs on the audio task. No lock: the
 * snapshot is plain bytes and a torn read shows one stale level for one frame. `n` is written last when a snapshot opens and
 * first (to 0) when it closes (pc_voice_publish_n), so a reader never pairs a voice count with names that are not there yet.
 * Gapless playback opens the next file while this one still plays: open() therefore leaves the snapshot alone; the first block
 * a handle renders takes it over (s_owner), and closing a handle clears it only if that handle owns it. */
EXT_RAM_BSS_ATTR static pc_voices_t s_voices;
static const gme_h *s_owner;
/* Meter rule: for a meter a constant is silence. A voice whose reading is an instantaneous value that a held channel leaves
 * constant (the DMC and AY/HES DACs, the beeper) counts only on a block where the value changed from the previous block's; this
 * is the previous value, one byte per voice (PSRAM). The change reads at its own level. */
EXT_RAM_BSS_ATTR static uint8_t s_prev[PC_VOICES_MAX];   /* PSRAM since the snapshot grew to 32 voices (internal RAM is at its ceiling) */
EXT_RAM_BSS_ATTR static uint8_t s_armed[PC_VOICES_MAX];       /* key-on hysteresis per voice (pc_voice_level_strike) */
EXT_RAM_BSS_ATTR static int16_t s_lastc[PC_VOICES_MAX];       /* the pitch (cents) of the voice's last sounding block: a new semitone while it sounds is a strike too */

static uint8_t on_change(uint8_t *prev, uint8_t v)
{
    uint8_t was = *prev;
    *prev = v;
    return v != was ? v : 0;
}

/* amplitude -> 0..255; GB squares and SMS channels are signed (the phase of the wave), the magnitude is the loudness */
static uint8_t scale_level(int amp, int max_amp)
{
    if (amp < 0) amp = -amp;
    if (amp > max_amp) amp = max_amp;
    return (uint8_t)((amp * 255 + max_amp / 2) / max_amp);
}

/* NES 2A03 and its NSF expansion chips (Nsf_Emu.cpp init_sound order: vrc6, namco, fme7, fds, mmc5, vrc7; the last three through the accessors of patch 0008).
 * Ported from JukeChip voice_state_nes. */
static uint8_t level_nes(Music_Emu *emu, int i, uint8_t *prev)
{
    Nsf_Emu *nsf = static_cast<Nsf_Emu *>(emu);
    if (i == 2) return nsf->apu_()->triangle_active_() ? 192 : 0;   /* no volume: one fixed level while it runs; its amplitude never rests at 0 */
    if (i == 4) return on_change(prev, scale_level(nsf->apu_()->osc_last_amp(4), 127));   /* DMC: a 7 bit DAC */
    if (i < Nes_Apu::osc_count) return scale_level(nsf->apu_()->osc_last_amp(i), 15);
    int offset = Nes_Apu::osc_count;
    if (Nes_Vrc6_Apu *v = nsf->vrc6_()) {
        if (i < offset + Nes_Vrc6_Apu::osc_count) {
            /* display order is saw, square, square; the chip's array is square, square, saw (what Nsf_Emu::set_voice remaps too) */
            int d = i - offset, phys = d - 1;
            if (phys < 0) phys = 2;
            return scale_level(v->osc_last_amp(phys), d == 0 ? 31 : 15);
        }
        offset += Nes_Vrc6_Apu::osc_count;
    }
    if (Nes_Namco_Apu *n = nsf->namco_()) {
        if (i < offset + Nes_Namco_Apu::osc_count) return scale_level(n->osc_last_amp(i - offset), 225);   /* 15 * 15 */
        offset += Nes_Namco_Apu::osc_count;
    }
    if (Nes_Fme7_Apu *f = nsf->fme7_()) {
        if (i < offset + Nes_Fme7_Apu::osc_count) return scale_level(f->osc_last_amp(i - offset), 192);   /* amp_range */
        offset += Nes_Fme7_Apu::osc_count;
    }
    if (Nes_Fds_Apu *f = nsf->fds_()) {                          /* FDS: one wave voice, wave sample 0..63 x volume 0..32 x master 0..10 (CHIP2A, patch 0008) */
        if (i < offset + Nes_Fds_Apu::osc_count) return scale_level(f->osc_last_amp(0), 63 * 32 * 10);
        offset += Nes_Fds_Apu::osc_count;
    }
    if (Nes_Mmc5_Apu *m = nsf->mmc5_()) {                        /* MMC5: two squares like the 2A03's (volume 0..15), then the 8 bit PCM (Nes_Apu slot 4) */
        if (i < offset + Nes_Mmc5_Apu::osc_count) {
            const int d = i - offset;
            return d < 2 ? scale_level(m->osc_last_amp(d), 15) : on_change(prev, scale_level(m->osc_last_amp(4), 127));
        }
        offset += Nes_Mmc5_Apu::osc_count;
    }
    if (Nes_Vrc7_Apu *v = nsf->vrc7_()) {                        /* VRC7: six FM voices, emu2413's output sample (signed, about +-4096) */
        if (i < offset + Nes_Vrc7_Apu::osc_count) return scale_level(v->osc_last_amp(i - offset), 2048);
    }
    return 0;
}

/* An AY-3-8910 channel (in an AY file or hosted by KSS). Tone and noise both off (mixer register 7): a 4 bit DAC. */
static uint8_t level_ay(Ay_Apu *apu, int i, uint8_t *prev)
{
    uint8_t v = scale_level(apu->osc_last_amp(i), Ay_Apu::amp_range);
    int m = apu->mixer_reg();
    bool dac = (m >> i & 1) && (m >> (i + 3) & 1);               /* a constant is silence */
    return dac ? on_change(prev, v) : v;
}

/* Known limitation (the level functions read an instantaneous amplitude): a VRC6 saw cut mid-ramp, an N163 channel stopped by a
 * low frequency, and a POKEY channel in volume-only mode can leave a resting bar (their readings are wave samples or a volume,
 * so no change detector is used: it would hide steady 50/100 Hz notes sampled every 10 ms). The AY "inaudible period" automatic
 * tone-off is not detected, only mixer register 7. */

/* One voice's level now, by emulator type (JukeChip player_backend_gme.cpp voice_state_*). false: this type has no adapter. */
static bool voice_now(Music_Emu *emu, gme_type_t type, int i, int count, uint8_t *out, uint8_t *prev)
{
    *out = 0;
    if (type == gme_spc_type) {                                  /* ENVX 0..127 -> 0..254 */
        if (i < 8) {
            int e = static_cast<Spc_Emu *>(emu)->apu_()->dsp_read_reg(0x08 + i * 0x10);
            *out = (uint8_t)((e < 0 ? 0 : e > 127 ? 127 : e) << 1);
        }
    } else if (type == gme_nsf_type || type == gme_nsfe_type) {
        *out = level_nes(emu, i, prev);
    } else if (type == gme_gbs_type) {
        if (i < Gb_Apu::osc_count) *out = scale_level(static_cast<Gbs_Emu *>(emu)->apu_()->osc_last_amp(i), i == 2 ? 30 : 15);
    } else if (type == gme_ay_type) {                            /* 3 AY voices + the beeper, a Z80 port bit */
        Ay_Emu *ay = static_cast<Ay_Emu *>(emu);
        if (i < Ay_Apu::osc_count) {
            *out = level_ay(ay->apu_(), i, prev);
        } else if (i == Ay_Apu::osc_count) {                     /* the bit toggling = sound; a held bit = silence */
            uint8_t t = (uint8_t)ay->beeper_toggles();
            *out = t != *prev ? 255 : 0;
            *prev = t;
        }
    } else if (type == gme_kss_type) {                           /* AY 0-2, SCC 3-7 (|wave| 128 * volume 120) */
        Kss_Emu *kss = static_cast<Kss_Emu *>(emu);
        if (i < Ay_Apu::osc_count) *out = level_ay(kss->ay_(), i, prev);
        else if (i < Ay_Apu::osc_count + Scc_Apu::osc_count) *out = scale_level(kss->scc_()->osc_last_amp(i - Ay_Apu::osc_count), 15360);
    } else if (type == gme_hes_type) {
        if (i < Hes_Apu::osc_count) {
            Hes_Apu *apu = static_cast<Hes_Emu *>(emu)->apu_();
            int ctl = apu->osc_control(i);
            uint8_t v = scale_level(apu->osc_last_amp(i), 0x8000);
            *out = !(ctl & 0x80) ? 0 : (ctl & 0x40) ? on_change(prev, v) : v;   /* off: 0; direct D/A mode: a constant is silence */
        }
    } else if (type == gme_sap_type) {                           /* 4 voices, 8 when stereo */
        Sap_Emu *sap = static_cast<Sap_Emu *>(emu);
        if (i < Sap_Apu::osc_count) *out = scale_level(sap->apu_()->osc_last_amp(i), 30);
        else if (i < Sap_Apu::osc_count * 2) *out = scale_level(sap->apu2_()->osc_last_amp(i - Sap_Apu::osc_count), 30);
    } else if (type == gme_vgm_type || type == gme_vgz_type) {
        Vgm_Emu *vgm = static_cast<Vgm_Emu *>(emu);
        if (vgm->is_classic_emu()) {                             /* PSG only: 3 squares and noise */
            if (i < Sms_Apu::osc_count) *out = scale_level(vgm->psg_(0)->osc_last_amp(i), 64);
        } else if (i < 6) {                                      /* FM 1-6; a YM2413-only file has no peek: stays 0 */
            if (vgm->ym2612_enabled_()) *out = (uint8_t)vgm->ym2612_(0)->level(i);   /* GENS level() is already 0..255 */
        } else if (i < 10) {                                     /* PSG squares and noise (the louder of a dual pair) */
            int a = abs(vgm->psg_(0)->osc_last_amp(i - 6));
            if (vgm->psg_dual_()) { int b = abs(vgm->psg_(1)->osc_last_amp(i - 6)); if (b > a) a = b; }
            *out = scale_level(a, 64);
        }
    } else {
        (void)count;
        return false;                                            /* GYM (its chip is private to Gym_Emu) and anything else */
    }
    return true;
}


/* ---- pitch, noise flag and pan (visualiser phase 1). The formulas are ChipChill's ccg_tuning.h, the data a read of the chip's registers (accessors added in
 * components/gme, patches/0006). Chips without an entry here report no pitch (level and strikes only): the YM2612 / YM2413 / VRC7 FM, SPC, N163, FME-7, SCC, SAP, FDS, MMC5. */
static const double NES_CLOCK = 1789773.0, GB_CLOCK = 4194304.0, SMS_CLOCK = 3579545.0, MSX_CLOCK = 3579545.0, SPECTRUM_CLOCK = 3546900.0, PCE_CLOCK = 7159091.0;
static int cents_hz(double hz) { return hz > 0.0 ? pc_voice_cents_of_hz_x100((long)(hz * 100.0 + 0.5)) : PC_VOICE_NOPITCH; }

static int pitch_nes(Music_Emu *emu, int i)
{
    Nsf_Emu *nsf = static_cast<Nsf_Emu *>(emu);
    if (i < 2) return cents_hz(NES_CLOCK / (16.0 * (nsf->apu_()->osc_period(i) + 1)));
    if (i == 2) return cents_hz(NES_CLOCK / (32.0 * (nsf->apu_()->osc_period(2) + 1)));
    if (i < Nes_Apu::osc_count) return PC_VOICE_NOPITCH;                  /* noise, DMC */
    int offset = Nes_Apu::osc_count;
    if (Nes_Vrc6_Apu *v = nsf->vrc6_()) {
        if (i < offset + Nes_Vrc6_Apu::osc_count) {
            const int d = i - offset, phys = d == 0 ? 2 : d - 1;          /* display order saw, square, square */
            const double p = v->osc_period(phys);
            return cents_hz(NES_CLOCK / ((phys == 2 ? 14.0 : 16.0) * p));
        }
    }
    return PC_VOICE_NOPITCH;
}

/* one voice's pitch (cents or NOPITCH), noise flag and pan; type as voice_now */
static void voice_extra(Music_Emu *emu, gme_type_t type, int i, int *cents, uint8_t *flags, int8_t *pan)
{
    *cents = PC_VOICE_NOPITCH; *flags = 0; *pan = PC_PAN_UNKNOWN;
    if (type == gme_spc_type) {                                  /* no pitch (a sample's pitch is a rate, not a note); pan from VOL L/R */
        if (i < 8) {
            Snes_Spc *a = static_cast<Spc_Emu *>(emu)->apu_();
            const int l = (int8_t)a->dsp_read_reg(i * 0x10), r = (int8_t)a->dsp_read_reg(i * 0x10 + 1);
            const int sum = abs(l) + abs(r);
            if (sum) *pan = (int8_t)((r - l) * 127 / sum);
        }
    } else if (type == gme_nsf_type || type == gme_nsfe_type) {
        *cents = pitch_nes(emu, i);
        if (i == 3 || i == 4) *flags |= PC_VF_NOISE;             /* the 2A03 noise and DMC */
    } else if (type == gme_gbs_type) {
        if (i < 3) {
            const int x = static_cast<Gbs_Emu *>(emu)->apu_()->osc_freq(i);
            if (x > 0 && x < 2048) *cents = cents_hz(GB_CLOCK / ((i == 2 ? 64.0 : 32.0) * (2048 - x)));
        } else *flags |= PC_VF_NOISE;
    } else if (type == gme_ay_type) {
        if (i < Ay_Apu::osc_count) {
            const int p = static_cast<Ay_Emu *>(emu)->apu_()->osc_period(i);
            if (p > 16) *cents = cents_hz(SPECTRUM_CLOCK / (2.0 * p));
        }
    } else if (type == gme_kss_type) {
        if (i < Ay_Apu::osc_count) {
            const int p = static_cast<Kss_Emu *>(emu)->ay_()->osc_period(i);
            if (p > 16) *cents = cents_hz(MSX_CLOCK / (2.0 * p));
        }
    } else if (type == gme_hes_type) {
        if (i < Hes_Apu::osc_count) {
            const int p = static_cast<Hes_Emu *>(emu)->apu_()->osc_period(i);
            if (p > 0) *cents = cents_hz(PCE_CLOCK / (64.0 * p));
        }
    } else if (type == gme_vgm_type || type == gme_vgz_type) {
        Vgm_Emu *vgm = static_cast<Vgm_Emu *>(emu);
        const int psg = vgm->is_classic_emu() ? i : i - 6;       /* the PSG voices: 0..3 alone, 6..9 behind the FM voices */
        if (psg >= 0 && psg < 4 && (vgm->is_classic_emu() || i >= 6)) {
            if (psg == 3) *flags |= PC_VF_NOISE;
            else {
                const int p = vgm->psg_(0)->osc_period(psg);
                if (p > 128) *cents = cents_hz(SMS_CLOCK / (2.0 * p));
            }
        }
    }
    if (*cents != PC_VOICE_NOPITCH) *flags |= PC_VF_PITCH;
}

/* previous values of the change detector: zero, then one read of every voice, so the next block does not count as a change */
static void prime_prev(gme_h *g, int n)
{
    gme_type_t type = gme_type(g->emu);
    uint8_t unused;
    memset(s_prev, 0, sizeof s_prev);
    for (int i = 0; i < n; i++) voice_now(g->emu, type, i, n, &unused, &s_prev[i]);
}

static void voices_update(gme_h *g)
{
    if (s_owner != g) {                                          /* take the snapshot over (first block of this handle) */
        pc_voice_publish_n(&s_voices, 0);
        s_owner = g;
        memset(s_voices.level, 0, sizeof s_voices.level);
        for (int i = 0; i < PC_VOICES_MAX; i++) { pc_voice_clear_extra(&s_voices, i); s_armed[i] = 1; s_lastc[i] = PC_VOICE_NOPITCH; }
        pc_vmute_reset();                                        /* a new source: every voice on (owner 2026-11-17: reset on track change) */
        g->mask = 0;
        gme_mute_voices(g->emu, 0);
        gme_ignore_silence(g->emu, 0);
        int n = gme_voice_count(g->emu);
        gme_type_t type = gme_type(g->emu);
        uint8_t probe;
        if (n > PC_VOICES_MAX) n = PC_VOICES_MAX;
        if (n < 0 || !voice_now(g->emu, type, 0, n, &probe, &s_prev[0])) n = 0;   /* no adapter: no bars rather than wrong bars */
        for (int i = 0; i < n; i++) pc_voice_short(gme_voice_name(g->emu, i), i, s_voices.name[i]);
        prime_prev(g, n);
        pc_voice_publish_n(&s_voices, (uint8_t)n);
    }
    int n = s_voices.n;
    gme_type_t type = gme_type(g->emu);
    for (int i = 0; i < n; i++) {
        uint8_t now;
        voice_now(g->emu, type, i, n, &now, &s_prev[i]);
        if (g->mask >> i & 1) now = 0;                           /* a muted voice's oscillator still runs and reports its amplitude: it reads silent (the audio is already gone) */
        /* one decay step per render call; the fall rate assumes the audio task's block: rate / 100 = 441 frames = 10 ms at 44.1 kHz
         * (audio_svc.c), i.e. 16 steps = 160 ms from full scale to 0 */
        s_voices.level[i] = pc_voice_decay(s_voices.level[i], now);
        int cents; uint8_t fl; int8_t pan;
        voice_extra(g->emu, type, i, &cents, &fl, &pan);
        /* strikes read the HELD level (the meter's 160 ms fall), not the raw block value: a square's amplitude is read at one instant and is 0 on half the blocks, which must not re-arm the detector */
        const uint8_t held = s_voices.level[i];
        bool strike = pc_voice_level_strike(held, &s_armed[i]);
        if (held >= 64 && cents != PC_VOICE_NOPITCH && s_lastc[i] != PC_VOICE_NOPITCH && (cents + 50) / 100 != (s_lastc[i] + 50) / 100) strike = true;   /* legato: a new semitone */
        if (held >= 24) s_lastc[i] = (int16_t)cents; else if (held == 0) s_lastc[i] = PC_VOICE_NOPITCH;
        if (strike) s_voices.strikes[i]++;
        s_voices.cents[i] = (int16_t)cents;
        s_voices.pan[i] = pan;
        s_voices.flags[i] = fl;
    }
}

/* VGM with an FM chip lists FM 1-6 then the 4 PSG voices (6..9), while gme's mute bits skip bit 6 (the PCM / DAC): voice i >= 6 is bit i + 1.
 * Every other emulator's voice index is its mute bit. */
static int mute_bits(gme_h *g, uint32_t m)
{
    gme_type_t type = gme_type(g->emu);
    if (type == gme_vgm_type || type == gme_vgz_type) {
        if (static_cast<Vgm_Emu *>(g->emu)->is_classic_emu()) return (int)m;
        return (int)((m & 0x3Fu) | ((m >> 6) << 7));
    }
    return (int)m;
}

/* before the block is decoded: the mute mask the UI posted reaches the emulator (only while this handle owns the voices: a pre-opened gapless successor must not mute the song that plays) */
static void mute_apply(gme_h *g)
{
    if (s_owner != g) return;
    const uint32_t want = pc_vmute_mask();
    if (want == g->mask) return;
    g->mask = want;
    /* gme runs ahead through a silent stretch (up to 3 s, silence_lookahead) to find sound: with the only sounding voice muted that would skip the music, and an unmute would
     * come back seconds late. While anything is muted the skipping is off (found with a host harness: unmute came back 2.9 s late). */
    gme_ignore_silence(g->emu, want != 0);
    gme_mute_voices(g->emu, mute_bits(g, want));
}

extern "C" int backend_gme_voices(pc_voices_t *out) { return pc_voice_read(&s_voices, out); }
extern "C" void backend_gme_kss_fm(int on) { Kss_Emu::fm_off = on == 0; }

static int start(gme_h *g, int idx)
{
    if (gme_start_track(g->emu, idx)) return -1;
    gme_info_t *info = nullptr;
    int32_t len = -1;
    if (!gme_track_info(g->emu, &info, idx) && info) { len = info->play_length; gme_free_info(info); }
    g->len_ms = pc_chip_length_ms(len);
    g->track = idx;
    if (s_owner == g) {
        prime_prev(g, s_voices.n);   /* the chips were reset (the beeper counter too): old previous values would read as a change */
        pc_vmute_reset(); g->mask = 0; gme_mute_voices(g->emu, 0); gme_ignore_silence(g->emu, 0);   /* another track: every voice on */
    }
    gme_set_fade(g->emu, (int)g->len_ms);        /* GME's default 8 s fade at the end (spec 4.3) */
    return 0;
}

static int g_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    if (!r->mem.p || r->u == nullptr || start_off >= r->size) return -1;
    gme_h *g = (gme_h *)calloc(1, sizeof *g);
    if (!g) return -1;
    if (gme_open_data(r->mem.p + start_off, (long)(r->size - start_off), &g->emu, RATE) || !g->emu) { free(g); return -1; }
    if (start(g, 0)) { gme_delete(g->emu); free(g); return -1; }
    *h = g;
    return 0;
}

static int g_render(void *h, float *out, int frames)
{
    gme_h *g = (gme_h *)h;
    if (gme_track_ended(g->emu)) return 0;
    short pcm[2 * 480];
    if (frames > 480) frames = 480;
    mute_apply(g);
    if (gme_play(g->emu, frames * 2, pcm)) return -1;
    voices_update(g);          /* the oscillators have just run to the end of this block: their last amplitude is current */
    for (int i = 0; i < 2 * frames; i++) out[i] = (float)pcm[i] * (1.0f / 32768.0f);
    return frames;
}

static uint32_t g_rate(void *) { return RATE; }
static int g_count(void *h) { return gme_track_count(((gme_h *)h)->emu); }
static int g_start_track(void *h, int idx) { return start((gme_h *)h, idx); }
static int32_t g_len(void *h) { return ((gme_h *)h)->len_ms; }
static int32_t g_pos(void *h) { return gme_tell(((gme_h *)h)->emu); }
static void g_close(void *h)
{
    gme_h *g = (gme_h *)h;
    if (s_owner == g) { pc_voice_publish_n(&s_voices, 0); s_owner = nullptr; pc_vmute_reset(); }   /* before the emulator goes */
    gme_delete(g->emu);
    free(g);
}

extern "C" const pc_backend_t backend_gme = {
    .open = g_open, .render = g_render, .rate = g_rate, .track_count = g_count,
    .start_track = g_start_track, .length_ms = g_len, .position_ms = g_pos, .close = g_close, .byte_pos = nullptr,
    .name = "Chip", .bits = nullptr, .burst = nullptr,
};

extern "C" int backend_gme_probe(const void *data, size_t size, int *tracks, char *sys, size_t ssys, char *game, size_t sgame)
{
    Music_Emu *emu = nullptr;
    if (gme_open_data(data, (long)size, &emu, gme_info_only) || !emu) return -1;
    *tracks = gme_track_count(emu);
    gme_info_t *info = nullptr;
    if (!gme_track_info(emu, &info, 0) && info) {
        snprintf(sys, ssys, "%s", info->system ? info->system : "");
        snprintf(game, sgame, "%s", info->game ? info->game : "");
        gme_free_info(info);
    }
    gme_delete(emu);
    return 0;
}

extern "C" int backend_gme_track_names(const void *data, size_t size, char (*names)[64], int max)
{
    Music_Emu *emu = nullptr;
    if (gme_open_data(data, (long)size, &emu, gme_info_only) || !emu) return 0;
    int n = gme_track_count(emu);
    if (n > max) n = max;
    for (int i = 0; i < n; i++) {
        gme_info_t *info = nullptr;
        names[i][0] = 0;
        if (!gme_track_info(emu, &info, i) && info) {
            snprintf(names[i], 64, "%s", info->song ? info->song : "");
            gme_free_info(info);
        }
    }
    gme_delete(emu);
    return n;
}
