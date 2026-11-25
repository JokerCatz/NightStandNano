// SPDX-License-Identifier: LGPL-2.1-or-later
/* SN76489 / SN76496 adapter. The core is emu/cores/sn76496.c (MAME, BSD-3-Clause, by Nicola Salmoria and others), included here so its static entry points are reachable. */
#include "fmvgm_pcm.h"
#include "emu/cores/sn76496.c"

typedef struct { DEV_INFO inf; } sn_t;
static void *sn_create(uint32_t clock, uint32_t variant)
{
    SN76496_CFG cfg; memset(&cfg, 0, sizeof cfg);
    uint32_t fb = variant & 0xFFFF, width = (variant >> 16) & 0xFF, flags = variant >> 24;
    cfg._genCfg.clock = clock;
    cfg._genCfg.smplRate = clock / 16 / 4;                     /* the rate after the backend's 4:1 decimation: tones above its Nyquist are silenced by the core's own limiter */
    cfg.shiftRegWidth = width ? width : 16;
    cfg.noiseTaps = fb ? fb : 0x09;
    cfg.segaPSG = (flags & 0x01) ? 0 : 1;                      /* header flags of VGM 1.51, mapped as libvgm's player does */
    cfg.negate = (flags & 0x02) ? 1 : 0;
    cfg.stereo = (flags & 0x04) ? 0 : 1;
    cfg.clkDiv = (flags & 0x08) ? 1 : 8;
    cfg.ncrPSG = (flags & 0x10) ? 1 : 0;
    sn_t *s = (sn_t *)fmvgm_pcm_calloc(1, sizeof *s);
    if (!s) return NULL;
    /* the core allocates its own state (calloc is the port's): hand back the chip, keep nothing else */
    DEV_INFO inf;
    if (device_start_sn76496_mame(&cfg, &inf) != 0) { fmvgm_pcm_free(s); return NULL; }
    s->inf = inf;
    sn76496_reset(inf.dataPtr);
    return s;
}
static uint32_t sn_rate(void *c) { return ((sn_t *)c)->inf.sampleRate; }
static void sn_reset(void *c) { sn76496_reset(((sn_t *)c)->inf.dataPtr); }
static void sn_write(void *c, uint32_t reg, uint32_t val)
{
    sn76496_w_mame(((sn_t *)c)->inf.dataPtr, (UINT8)reg, (UINT8)val);      /* reg 0: the data byte; reg 1: the Game Gear stereo byte (0x4F) */
}
static void sn_render(void *c, int32_t *lr, uint32_t n)
{
    DEV_SMPL b0[128], b1[128]; DEV_SMPL *o[2] = { b0, b1 };
    void *chip = ((sn_t *)c)->inf.dataPtr;
    while (n) {
        uint32_t m = n > 128 ? 128 : n;
        sn76496_update(chip, m, o);
        for (uint32_t i = 0; i < m; i++) { lr[2 * i] = b0[i]; lr[2 * i + 1] = b1[i]; }
        lr += 2 * m; n -= m;
    }
}
static void sn_destroy(void *c) { if (!c) return; sn76496_shutdown(((sn_t *)c)->inf.dataPtr); fmvgm_pcm_free(c); }
const fmvgm_pcm_ops fmvgm_pcm_sn76496 = { "SN76489", sn_create, sn_rate, sn_reset, sn_write, sn_render, NULL, sn_destroy };
