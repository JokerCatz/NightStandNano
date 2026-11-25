// SPDX-License-Identifier: LGPL-2.1-or-later
/* OKI MSM6295 adapter. The core is emu/cores/okim6295.c + okiadpcm.c (MAME, BSD-3-Clause, by Mirko Buffoni and Aaron Giles; ADPCM by Andrew Gardner and Aaron Giles), included here so its
 * static entry points are reachable. The ROM image is copied into the core's own buffer (OKI ROMs are a few hundred KB; the port allocator decides where it lives). */
#include "fmvgm_pcm.h"
#include "emu/cores/okiadpcm.c"
#include "emu/cores/okim6295.c"

typedef struct { DEV_INFO inf; } oki_t;
static void *oki_create(uint32_t clock, uint32_t variant)
{
    DEV_GEN_CFG cfg; cfg.clock = clock; cfg.smplRate = 0; cfg.flags = variant & 1;      /* pin 7 */
    DEV_INFO inf;
    oki_t *s = (oki_t *)fmvgm_pcm_calloc(1, sizeof *s);
    if (!s) return NULL;
    if (device_start_okim6295(&cfg, &inf) != 0) { fmvgm_pcm_free(s); return NULL; }
    s->inf = inf;
    device_reset_okim6295(inf.dataPtr);
    return s;
}
static uint32_t oki_rate(void *c) { return okim6295_get_rate(((oki_t *)c)->inf.dataPtr); }
static void oki_reset(void *c) { device_reset_okim6295(((oki_t *)c)->inf.dataPtr); }
static void oki_write(void *c, uint32_t reg, uint32_t val) { okim6295_w(((oki_t *)c)->inf.dataPtr, (UINT8)reg, (UINT8)val); }
static void oki_render(void *c, int32_t *lr, uint32_t n)
{
    DEV_SMPL b0[128], b1[128]; DEV_SMPL *o[2] = { b0, b1 };
    void *chip = ((oki_t *)c)->inf.dataPtr;
    while (n) {
        uint32_t m = n > 128 ? 128 : n;
        okim6295_update(chip, m, o);
        for (uint32_t i = 0; i < m; i++) { lr[2 * i] = b0[i]; lr[2 * i + 1] = b1[i]; }
        lr += 2 * m; n -= m;
    }
}
static int oki_rom(void *c, uint32_t total, uint32_t start, const uint8_t *data, uint32_t len)
{
    void *chip = ((oki_t *)c)->inf.dataPtr;
    okim6295_state *st = (okim6295_state *)chip;
    if (total > 0x400000u) return -1;                          /* more than 4 MB: no OKI board has that (4 banks of 256 KB at most 1 MB, NMK112 boards up to 2 MB) */
    if (st->ROMSize != total) {
        okim6295_alloc_rom(chip, total);
    }
    if (total && !st->ROM) { st->ROMSize = 0; return -1; }       /* the allocation failed: the core would write through NULL */
    okim6295_write_rom(chip, start, len, data);
    return 0;
}
static void oki_destroy(void *c) { if (!c) return; device_stop_okim6295(((oki_t *)c)->inf.dataPtr); fmvgm_pcm_free(c); }
const fmvgm_pcm_ops fmvgm_pcm_okim6295 = { "OKIM6295", oki_create, oki_rate, oki_reset, oki_write, oki_render, oki_rom, oki_destroy };
