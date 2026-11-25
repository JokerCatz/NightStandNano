// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Why a file cannot be played (honest disclosure): pure checks on a file's header, run by the UI after an open failed (or before a chip
 * file is handed to the emulator). No state: nothing is kept between calls. The backend refuses, this says why. */
#include <stddef.h>
#include <stdint.h>
#include "pc/pc_io.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PC_REFUSE_NONE = 0,       /* no known reason (a damaged file, memory, ...) */
    PC_REFUSE_OPL_MODULE,     /* S3M with AdLib (OPL) instruments: libxmp has no OPL */
    PC_REFUSE_VGM_CHIPS,      /* VGM that uses chips the emulator does not have; detail = their names, comma separated */
    PC_REFUSE_AIFF_CODEC,     /* AIFF-C with a compression other than none / sowt; detail = the four characters */
    PC_REFUSE_TTA_ENCRYPTED,  /* password-protected TTA */
    PC_REFUSE_MPC_OLD,        /* Musepack before stream version 7 */
    PC_REFUSE_P8_NO_MUSIC,    /* a PICO-8 cart with no music pattern (CHIP2A) */
    PC_REFUSE_ADX_TYPE,       /* an ADX that is not type 3 ADPCM (fixed-coefficient, exponential-scale or AHX) */
    PC_REFUSE_BRSTM_CODEC,    /* a BRSTM that is not DSP-ADPCM (PCM8 / PCM16) */
    PC_REFUSE_YM_VARIANT,     /* an Atari ST YM that is a digital mix (MIX1) or a tracker (YMT1 / YMT2) file */
    PC_REFUSE_OPL_4OP,        /* DOS OPL music that needs OPL3 four-operator voices (the emulator is OPL2) */
    PC_REFUSE_OPL_PACKED,     /* an Adlib Tracker 2 module packed with aPlib (versions 9-11): its depacker has no licence */
    PC_REFUSE_SID_BASIC,      /* an RSID that is a BASIC program: needs the C64 BASIC ROM, which the player has not got */
    PC_REFUSE_XMP_BAD,        /* a tracker module libxmp could not load: damaged, or a variant it does not play (set by the caller from the extension: pc_xmp_ext) */
    PC_REFUSE_OPL_DAMAGED,    /* DOS OPL music with a size field out of range or reads far past the end (hostile or damaged) */
    PC_REFUSE_EXPERIMENTAL_OFF, /* a format of the experimental tier while the setting 實驗性格式 is off (set by the caller from pc_chipseq.h) */
    PC_REFUSE_P8_PNG,         /* a .p8.png that is not a 160x205 8-bit RGBA PICO-8 cart image */
} pc_refuse_t;

/* VGM: the chips the header names that Game_Music_Emu does not emulate (it emulates SN76489, YM2413, YM2612). Writes their names
 * ("YM2151, SegaPCM") into out; returns how many; -1 when data is not a VGM header. */
int pc_vgm_unsupported_chips(const uint8_t *data, size_t n, char *out, size_t cap);

/* ext: the file extension with its dot (".s3m", ".aif", ...), any case. detail (may be NULL) receives the words for the message. */
pc_refuse_t pc_refuse_reason(const pc_reader_t *r, const char *ext, char *detail, size_t cap);

#ifdef __cplusplus
}
#endif
