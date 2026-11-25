// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* USB Audio Class 2 host, playback only: finds the DAC's OUT streaming alt setting, sets the sample rate, streams PCM over ISO OUT.
 * IDF has no UAC host class driver, so this is ours. Pull model: the ISO completion callback asks for the next frames. */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

typedef struct {
    bool present;                 /* a DAC with a playback alt setting is attached and parsed */
    uint16_t vid, pid;
    char product[48];
    uint8_t channels;             /* of the chosen alt */
    uint8_t subslot, bits;        /* bytes per sample slot, bits used */
    uint32_t rate_min, rate_max;  /* from the clock's GET_RANGE (0 when it answered nothing) */
} nsn_uac_info_t;

/* Fill dst with `frames` stereo S16 frames (interleaved L,R); return frames written (the rest becomes silence). Runs in the USB task: be fast. */
typedef size_t (*nsn_pcm_pull_t)(int16_t *dst, size_t frames, void *ctx);

esp_err_t nsn_uac_start(void);                                   /* USB host + tasks; attaches/detaches are handled by itself */
void      nsn_uac_info(nsn_uac_info_t *out);
esp_err_t nsn_uac_play(uint32_t rate, nsn_pcm_pull_t pull, void *ctx);
void      nsn_uac_stop(void);
bool      nsn_uac_playing(void);
void      nsn_uac_stats(uint32_t *transfers_done, uint32_t *transfers_bad);
bool      nsn_uac_info_present(void);
uint32_t  nsn_uac_frames(void);                                  /* frames of the ISO transfers that completed since boot (with nsn_uac_stats: the measured data rate) */
void      nsn_uac_last_clock(bool *set_ok, uint32_t *readback);  /* the latest nsn_uac_play: was SET_CUR accepted, and the rate the clock reported afterwards (0 = no answer) */
bool      nsn_uac_rates_known(void);                             /* the clock answered GET_RANGE */
bool      nsn_uac_rate_advertised(uint32_t rate);                /* inside one of the advertised ranges (false for every rate when none is known) */
