// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "pc/pc_voice.h"
#include <stdint.h>
#include "pc/pc_io.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Pull decoder (JukeChip player_backend style). Output: interleaved stereo float in [-1, 1]
 * at the backend's own rate(). Optional members may be NULL. */
typedef struct pc_backend {
    int      (*open)(void **h, const pc_reader_t *r, uint32_t start_off);   /* 0 on success; r must outlive h */
    int      (*render)(void *h, float *stereo, int frames);    /* frames written, 0 = end, <0 = error */
    uint32_t (*rate)(void *h);
    int      (*track_count)(void *h);                          /* NULL -> 1 */
    int      (*start_track)(void *h, int idx);                 /* NULL -> not supported */
    int32_t  (*length_ms)(void *h);                            /* -1 unknown */
    int32_t  (*position_ms)(void *h);
    void     (*close)(void *h);
    uint32_t (*byte_pos)(void *h);                             /* NULL -> unknown; where to open() again to continue */
    const char *name;                                          /* ASCII format name for the status page (MP3 FLAC WAV OGG M4A AAC MOD Chip Radio); NULL = unknown */
    uint8_t  (*bits)(void *h);                                 /* source bit depth; NULL for lossy and chip sources */
    uint32_t (*burst)(void *h);                                /* largest frames one decode step can produce (FLAC: max block size); NULL = no bursts */
} pc_backend_t;

extern const pc_backend_t pc_backend_mp3;   /* minimp3; 16 KB read buffer; needs ~16 KB of caller stack; gapless: a LAME/Xing tag trims the encoder delay and padding (length_ms from its frame count) */
extern const pc_backend_t pc_backend_flac;  /* dr_flac; any rate: above 48 kHz decimated by 2 or 4; byte_pos = PCM frame (opaque resume token) */
extern const pc_backend_t pc_backend_aiff;  /* .aif .aiff .aifc: big-endian PCM 8/16/24/32-bit (AIFF-C NONE and sowt), 1-2 channels (mono to both sides); byte_pos = payload bytes to the next frame (open takes the same) */
extern const pc_backend_t pc_backend_wav;   /* .wav: PCM 16/24/32-bit, float 32-bit, extensible; 1-2 channels (mono to both sides); byte_pos = payload bytes to the next frame (open takes the same) */
extern const pc_backend_t pc_backend_mpc;   /* .mpc: Musepack SV7 / SV8 (libmpcdec, BSD-3); stereo or mono, 32-48 kHz; byte_pos = PCM frame index (opaque resume token) */
extern const pc_backend_t pc_backend_tta;   /* .tta: True Audio (libtta, LGPL), 16/24-bit 1-2 channels; one decoder at a time (libtta keeps global state); byte_pos = PCM frame index */
extern const pc_backend_t pc_backend_hvl;   /* .ahx .hvl: HivelyTracker / AHX modules (libhvl, BSD-3), 44.1 kHz, played once; sub-songs = start_track; byte_pos = song position (order) */
extern const pc_backend_t pc_backend_sid;   /* .sid: C64 PSID / RSID (cRSID, components/libcrsid), 44.1 kHz, 180 s with a 4 s fade (no Songlengths database), sub-tunes = start_track, no resume position */
int pc_sid_check(const pc_reader_t *r);       /* 0 playable; -2 an RSID that is a BASIC program (no BASIC ROM); -1 anything else */
extern const pc_backend_t pc_backend_opl;   /* DOS AdLib / OPL music (pc_opl_ext): AdPlug 2.4 + fmopl (components/libadplug), 44.1 kHz, played once; sub-songs = start_track; the extension is read from pc_reader_t.ext; no resume position */
int pc_opl_check(const pc_reader_t *r);       /* 0 playable; -4 damaged (hostile size field / reads far past the end); -2 needs OPL3 four-operator voices; -3 A2M / A2T packed with aPlib (v9-11); -1 anything else */
extern const pc_backend_t pc_backend_p8;    /* .p8 .p8.png: PICO-8 carts, music and sfx only (zepto8-derived synth, WTFPL); 44.1 kHz; played once; sub-tracks = sequence starts; byte_pos = pattern */
extern const pc_backend_t pc_backend_adx;   /* .adx: CRI ADX type 3 (ADPCM), 1-8 channels, played once (the loop is ignored); byte_pos = PCM frame (decoded and dropped on resume) */
extern const pc_backend_t pc_backend_vag;   /* .vag: PlayStation VAGp (PS-ADPCM, mono); same rules */
extern const pc_backend_t pc_backend_brstm; /* .brstm: Nintendo RSTM with DSP-ADPCM, 1-8 channels (more than 2 folded to stereo); same rules */
int pc_stream_check(const pc_reader_t *r, const char *ext);   /* why a stream file is refused: 1 ADX that is not type 3, 2 BRSTM that is not DSP-ADPCM, 0 unknown */
extern const pc_backend_t pc_backend_ym;    /* .ym: Atari ST YM2 / YM3 / YM3b / YM5 / YM6 register dumps (plain or LHA -lh5-), own YM2149 model, SID / digidrum / sync-buzzer; 44.1 kHz; played once; byte_pos = frame */
int pc_ym_check(const pc_reader_t *r);        /* 0 playable; -2 MIX1 / YMT1 / YMT2 (not played); -1 anything else */
int pc_p8_check(const pc_reader_t *r);       /* 0 playable; -2 a PNG that is not a PICO-8 cart (160x205 RGBA); -3 a cart without music; -1 anything else */
/* Amiga tone (setting amiga_mix): libxmp'"'"'s Paula (BLEP) mixer for the Amiga MOD family only (the loaders that set libxmp'"'"'s Amiga MOD mode: MOD / Soundtracker / Noisetracker / ProWizard packs; other formats are unchanged).
 * 0 off (the plain mixer: today'"'"'s sound), 1 A500 (fixed filter; the LED filter follows the module'"'"'s E0x), 2 A500 with the LED filter always on, 3 A1200 (no analogue filter; our own band-limit table). Any task; a playing module takes it within one block. */
enum { PC_AMIGA_OFF = 0, PC_AMIGA_A500 = 1, PC_AMIGA_A500LED = 2, PC_AMIGA_A1200 = 3, PC_AMIGA_N = 4 };
void pc_backend_xmp_set_amiga(int mode);
bool pc_backend_xmp_amiga_active(void);            /* the module being played (or last opened) runs on the Paula mixer: it is an Amiga MOD and the mode is not 0 */
int pc_backend_xmp_voices(pc_voices_t *out);   /* any task: copies the per-voice level snapshot of the module being played (first 16 channels); returns n, 0 = none */
extern const pc_backend_t pc_backend_xmp;   /* libxmp 4.7.2 (full, no Paula mixer): every module loader + ProWizard + in-memory Amiga depackers (pc_xmp_ext), played once (no looping); byte_pos = order index */
extern const pc_backend_t pc_backend_chain; /* every suffix of pc_opl_ext / pc_xmp_ext: tries AdPlug, then libxmp, as the one table in pc_chipseq.c says, skipping experimental readers while the setting is off (name NULL: ask pc_backend_resolve) */
const pc_backend_t *pc_backend_resolve(const pc_backend_t *be, void *h);   /* the backend that really decodes (pc_backend_chain -> opl / xmp); any other backend as is */

#ifdef __cplusplus
}
#endif
