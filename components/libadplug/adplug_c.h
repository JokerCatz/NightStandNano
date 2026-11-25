/* A small C API over AdPlug 2.4 (LGPL-2.1-or-later) for DOS AdLib / OPL music: RAD, HSC, IMF, DRO, CMF, A2M, S3M-with-OPL, ... (the
 * format list is in SOURCE.md). The emulator is AdPlug's fmopl 0.37a (YM3812, LGPL-relicensed old MAME core), one chip, or two when a
 * file uses a second one (chip 0 left, chip 1 right: "dual OPL2"; an OPL3 file that only uses 2-operator voices plays through the
 * same two chips, its stereo panning is lost). Every call may allocate: while one runs, C++ `new` goes to PSRAM (see adplug_c.cpp).
 * One handle per thread at a time. */
#ifndef ADPLUG_C_H
#define ADPLUG_C_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct adp adp_t;

enum {
    ADP_ERR_FORMAT = 1,    /* no player recognises the file (or it is damaged) */
    ADP_ERR_MEMORY = 2,
    ADP_ERR_4OP = 3,       /* the file uses OPL3 four-operator voices: not emulated */
    ADP_ERR_SIZE = 4,
    ADP_ERR_DAMAGED = 5    /* the file is damaged: a stream was read far past its end, or a size field asked for more memory than a song may use (ADP_ERR_MEMORY) */
};

/* `ext`: the file extension with its dot, ".rad" (any case; some formats need it: IMF's speed, MIDI-like variants), may be NULL.
 * Copies `data`. The song is scanned once without sound (length, 4-operator check). Returns NULL and *err on failure. */
adp_t *adp_open(const uint8_t *data, uint32_t size, const char *ext, int rate, int *err);
void adp_close(adp_t *a);
int adp_subsongs(adp_t *a);
int adp_start(adp_t *a, int sub);                       /* (re)start from the beginning; 0 or -1 */
/* Interleaved stereo 16-bit. Returns frames written, 0 at the song's end, -1 when the file turns out to need 4-operator voices. */
int adp_render(adp_t *a, short *out, int frames);
int32_t adp_length_ms(adp_t *a);                        /* -1: the song did not end within the scan budget */
const char *adp_type(adp_t *a);                         /* "Reality ADlib Tracker (version 2)" */
const char *adp_title(adp_t *a);
const char *adp_author(adp_t *a);
int adp_dual(adp_t *a);                                 /* 1 once the second chip has been written to */

/* Per-voice view (OPL channels: 9 per chip, 18 with the second chip), from the register shadow. */
typedef struct {
    uint8_t key_on;         /* the key-on bit as last written (a muted voice reads 0) */
    uint8_t level;          /* 0..255: the carrier's (or, additive, the louder operator's) total level while the key is on */
    uint8_t percussion;     /* a rhythm-mode drum voice (bass drum has a pitch, the others are noise) */
    uint8_t noise;          /* 1 = unpitched drum */
    uint16_t strikes;       /* key-on edges since the start, wraps */
    long hz_x100;           /* frequency x 100 (0 = none) */
} adp_voice_t;
int adp_voice_count(adp_t *a);                          /* 9, or 18 once the second chip is used */
int adp_voice(adp_t *a, int i, adp_voice_t *out);
void adp_set_mute(adp_t *a, uint32_t mask);             /* bit i = voice i silent: key-on masked, release rate forced to 15 */

#ifdef __cplusplus
}
#endif
#endif
