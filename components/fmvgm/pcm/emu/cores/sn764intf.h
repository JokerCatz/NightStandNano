// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "../EmuStructs.h"
/* configuration of the SN76496 core: field names and meaning as the core reads them (written for this repository) */
typedef struct sn76496_config {
    DEV_GEN_CFG _genCfg;
    UINT16 noiseTaps; UINT8 shiftRegWidth; UINT8 negate; UINT8 clkDiv; UINT8 ncrPSG; UINT8 segaPSG; UINT8 stereo;
    void *t6w28_tone;
} SN76496_CFG;
#define SN76496_W_REG 0x00     /* normal register write */
#define SN76496_W_GGST 0x01    /* Game Gear stereo write */
