// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The device description structures the vendored cores fill in. Only what they use. fmvgm's adapters (pcm/fmvgm_pcm_*.c) include the core's .c file and use its DEV_DEF directly. */
#include "snddef.h"
typedef struct { void *chipInf; } DEV_DATA;
typedef struct { int unused; } DEV_LOGGER;
typedef void (*DEVCB_LOG)(void *userParam, void *source, UINT8 level, const char *message);
typedef void (*DEVCB_SRATE_CHG)(void *info, UINT32 newSRate);
enum { DEVLOG_OFF = 0, DEVLOG_ERROR, DEVLOG_WARN, DEVLOG_INFO, DEVLOG_DEBUG, DEVLOG_TRACE };
typedef struct { UINT32 clock; UINT32 smplRate; UINT32 flags; } DEV_GEN_CFG;
typedef struct dev_def DEV_DEF;
typedef struct { DEV_DATA *dataPtr; UINT32 sampleRate; const DEV_DEF *devDef; } DEV_INFO;
typedef UINT8 (*DEVFUNC_START)(const DEV_GEN_CFG *cfg, DEV_INFO *retDevInf);
typedef void (*DEVFUNC_CTRL)(void *info);
typedef void (*DEVFUNC_UPDATE)(void *info, UINT32 samples, DEV_SMPL **outputs);
typedef struct { UINT8 type; UINT8 rwType; UINT16 user; void *funcPtr; } DEVDEF_RWFUNC;
struct dev_def {
    const char *name; const char *author; UINT32 coreID;
    DEVFUNC_START Start; DEVFUNC_CTRL Stop; DEVFUNC_CTRL Reset; DEVFUNC_UPDATE Update;
    void *SetOptionBits; void *SetMuteMask; void *SetPanning; void *SetSampleRateChangeCallback; void *SetLoggingCallback; void *LinkDevice;
    DEVDEF_RWFUNC *rwFuncs;
};
typedef struct { int unused; } DEVLINK_IDS;
typedef struct {
    UINT8 deviceID;
    const char *(*name)(const DEV_GEN_CFG *);
    UINT16 (*channelCount)(const DEV_GEN_CFG *);
    const char **(*channelNames)(const DEV_GEN_CFG *);
    const DEVLINK_IDS *(*linkIDs)(const DEV_GEN_CFG *);
    const DEV_DEF *cores[2];
} DEV_DECL;
#define INIT_DEVINF(info, devData, rate, devdef) do { (info)->dataPtr = (devData); (info)->sampleRate = (rate); (info)->devDef = (devdef); } while (0)
#define DEVRW_A8D8 0
#define DEVRW_BLOCK 1
#define DEVRW_MEMSIZE 2
#define DEVRW_VALUE 3
#define DEVRW_ALL 4
#define RWF_REGISTER 0x01
#define RWF_MEMORY 0x02
#define RWF_CLOCK 0x03
#define RWF_SRATE 0x04
#define RWF_CHN_MUTE 0x05
#define RWF_WRITE 0x10
#define RWF_READ 0x20
