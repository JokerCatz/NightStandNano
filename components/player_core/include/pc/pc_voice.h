// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Per-voice level snapshot of the playing chip or tracker file (the data side of the voice meter).
 * The audio task writes it after every rendered block; the UI task copies it with audio_svc_voices().
 * No lock: the struct is plain bytes, so a torn read shows one stale level for one frame. The one rule that matters:
 * `n` is written last when a snapshot opens and first (to 0) when it closes, so a reader never sees names or levels of
 * a voice count that is not there yet or no longer is. */
#include <stdbool.h>
#include <stdint.h>

#define PC_VOICES_MAX 32            /* pitch.nsf shows 29 NSF voices, a YM2413 VGM has 18 */
#define PC_VOICE_NOPITCH (-1)       /* cents: this voice has no note (noise, samples, off) */
#define PC_PAN_UNKNOWN (-128)       /* pan: the chip has no pan, or the backend does not know */
/* flags */
#define PC_VF_PITCH 1               /* cents is a real note */
#define PC_VF_NOISE 2               /* a noise voice: drawn in the unpitched lane */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t n;                          /* voices in use, 0..PC_VOICES_MAX; 0 = this source has no separate voices */
    uint8_t level[PC_VOICES_MAX];       /* 0..255 */
    char name[PC_VOICES_MAX][4];        /* short label, NUL-terminated */
    int16_t cents[PC_VOICES_MAX];       /* the note being played as MIDI note x 100 (A4 = 6900), PC_VOICE_NOPITCH when unpitched; every field below is optional: nothing is drawn where nothing is known */
    uint8_t strikes[PC_VOICES_MAX];     /* key-on counter, wraps: a new note (or a re-strike) adds one */
    int8_t pan[PC_VOICES_MAX];          /* -127 left .. 127 right, PC_PAN_UNKNOWN when the chip has none */
    uint8_t flags[PC_VOICES_MAX];       /* PC_VF_* */
} pc_voices_t;                          /* 322 bytes (1 + 32 + 128 + 1 pad + 64 + 32 + 32 + 32): the snapshot lives in PSRAM, written once per 10 ms block, read once per frame */                          /* 81 bytes (1 + 16 + 64) */

/* Short label of a voice name, at most 3 characters: the first letter upper-cased plus the number the name ends in
 * (up to 2 digits): "Square 1" -> "S1", "Pulse 2" -> "P2", "Triangle" -> "T", "Noise" -> "N", "DMC" -> "D", "Wave 3" -> "W3",
 * "FM 6" -> "F6", "PCM" -> "P", "Beeper" -> "B". A name that starts with a digit gives its first 3 digits.
 * NULL or an empty name gives the voice's number as digits; `index` is 0-based, so index 0 -> "1" ... index 15 -> "16". */
void pc_voice_short(const char *full, int index, char out[4]);

/* One step of the meter's fall: max(now, held - held/4 - 1), never below 0. The audio side calls it once per rendered block. */
uint8_t pc_voice_decay(uint8_t held, uint8_t now);

/* Clears the optional fields of voice i (no pitch, no strikes, pan unknown, no flags). Level and name are the caller's. */
void pc_voice_clear_extra(pc_voices_t *v, int i);
/* MIDI note x 100 (cents) of a frequency in Hz x 100 (A4 = 440 Hz = 6900): the nearest 1 cent; <= 0 or out of the 0..127 note range gives PC_VOICE_NOPITCH. */
int pc_voice_cents_of_hz_x100(long hz_x100);
/* "C4", "C#4", "A0": the note of cents (rounded to the nearest semitone), at most 3 characters + NUL; PC_VOICE_NOPITCH gives "--". */
void pc_voice_note_name(int cents, char out[4]);
/* Key-on detection for a level meter that has no key-on bit: a strike when the level rises from below `lo` to at least `hi`
 * (hysteresis); `armed` is the per-voice state (1 = may strike). Returns true on a strike. */
bool pc_voice_level_strike(uint8_t level, uint8_t *armed);

/* Writer side: store the voice count last on open and first (0) on close, so the names and levels around it are never read
 * under a count they do not belong to. */
static inline void pc_voice_publish_n(pc_voices_t *v, uint8_t n) { __atomic_store_n(&v->n, n, __ATOMIC_RELEASE); }

/* Reader side: copy a snapshot another task writes; returns n, 0 (and out->n = 0) when the snapshot is closed. */
int pc_voice_read(const pc_voices_t *src, pc_voices_t *out);

#ifdef __cplusplus
}
#endif
