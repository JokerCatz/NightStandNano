// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Sound effects as apps (spec docs/archive/specs/2026-11-13-fx-apps-design.md §2, §5).
 * One app = DSP + parameter table + description + cost class. The chain order is ONE const table (pc_fxchain.c);
 * pages are generated from the parameter tables. Pure C, no ESP-IDF: host-tested (host_test/test_fxapp*.c). */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* ---- spec §2, verbatim ---- */
#define PC_FX_BLOCK   480            /* frames per chain step: audio_svc blocks (<= PC_AUDIO_MAX_BLOCK, 3840 at 384k) are split into these */
#define PC_FXP_MAX    44             /* values per app (equalizer: preamp + count + 10 x 4) */
typedef enum { PC_FXU_NONE, PC_FXU_DB, PC_FXU_HZ, PC_FXU_MS, PC_FXU_S, PC_FXU_PCT /* 0..1 shown as % */,
               PC_FXU_RATIO /* "2.5:1" */, PC_FXU_BOOL, PC_FXU_CHOICE } pc_fxunit_t;
enum { PC_FXP_LOG = 1, PC_FXP_REBUILD = 2, PC_FXP_BAND = 4 };   /* LOG: step is a ratio; REBUILD: new state (lines) needed; BAND: one of the EQ band rows */
typedef struct {
    const char *id;                      /* TOML params key ("threshold_db"); kv key suffix */
    const char *zh, *en;                 /* row name */
    float lo, hi, def, step;             /* step: one detent / one tap (ratio when LOG) */
    uint8_t unit, flags;
    const char *const *ch_zh, *const *ch_en;   /* CHOICE labels, (hi - lo + 1) of them */
} pc_fxparam_t;
typedef enum { PC_FXC_LIGHT, PC_FXC_HEAVY } pc_fxclass_t;
typedef enum { PC_FXFAM_GAIN, PC_FXFAM_STEREO, PC_FXFAM_TONE, PC_FXFAM_TUBE, PC_FXFAM_DYN, PC_FXFAM_REVERB, PC_FXFAM_LIMIT } pc_fxfam_t;
enum { PC_FXA_MIX = 1 /* the chain's dry/wet: a 濃度 row, TOML `mix` */, PC_FXA_BANDS = 2 /* equalizer rows */, PC_FXA_ALWAYS_LAST = 4,
       PC_FXA_WIRED_ONLY = 8 /* runs only with no mode bit at all: wired, mains, <= 48 kHz (owner 2026-11-14); the page and list row say why */ };
typedef struct pc_fxapp {
    const char *id;                      /* chipchill kind where one exists ("compressor"); ports: "tube", "velvet_widener", "velvet_room" */
    const char *zh, *en;                 /* list row and page title */
    const char *desc_zh, *desc_en;       /* 說明: what it does to the sound, when to use it (plain words, no device words) */
    uint8_t fam, cls, flags;             /* cls: provisional until the board bench (§11) sets it */
    uint8_t nparam;  const pc_fxparam_t *param;
    size_t (*state_size)(uint32_t rate, const float *v);        /* one block incl. delay lines; LVGL task */
    void (*init)(void *st, uint32_t rate, const float *v);     /* lay out, coefficients, clear. LVGL task (no alloc inside: st is given) */
    bool (*fits)(const void *st, uint32_t rate, const float *v); /* this block holds that rate and those REBUILD values (else: rebuild) */
    void (*set)(void *st, const float *v, uint32_t changed);   /* audio task: non-REBUILD values changed (bit per param; params >= 31 share bit 31); stores targets, keeps history; gains ramp over the next block; biquad values slew to the target in bounded steps per block (a big jump becomes a run of small steps) */
    void (*rate)(void *st, uint32_t rate);                     /* audio task, only when fits(): coefficients, history cleared */
    void (*reset)(void *st);                                   /* audio task: history cleared */
    void (*process)(void *st, float *lr, int frames);          /* audio task, interleaved stereo, frames <= PC_FX_BLOCK, in place */
    int  (*latency)(const void *st);                           /* frames (limiter look-ahead); NULL = 0 */
} pc_fxapp_t;
int pc_fxapps(const pc_fxapp_t *const **out);                  /* the chain order: THE one const table (pc_fxchain.c) */
int pc_fxapp_find(const char *id);                             /* index or -1 (import mapping, kv keys) */
float pc_fxparam_step(const pc_fxparam_t *p, float v, int detents, bool from_tap);   /* bounded; tap at hi wraps to lo (ui_axis_step's rule, §6.3) */
int  pc_fxparam_text(const pc_fxparam_t *p, float v, bool zh, char *b, size_t n);    /* "−3.5 dB", "1.2 kHz", "40 %", "2.5:1", "開" */
/* ---- end of §2's block ---- */

/* Notes on the contract (T1's calls, binding for the ports):
 * - `v` is the app's whole value array, index = parameter index, nparam floats; init/set copy what they need.
 * - set(): `changed` is a hint only (bit p for parameter p; parameters >= 31 share bit 31); every app recomputes
 *   from the whole `v`, so a set() with the same values and changed == 0 changes nothing. A non-finite value
 *   (NaN, inf) counts as the parameter's default (here and in init / pc_fxparam_step / pc_fxparam_text).
 * - set() never receives a changed REBUILD value: such values arrive only through init() of a new block (the chain drops
 *   a SET of a PC_FXP_REBUILD parameter; spec §3.2). The v passed to set() therefore always matches the block's layout.
 * - A state block is never copied or moved once init() laid it out (freeverb keeps pointers into its own block).
 * - Linear stepping snaps to the grid lo + k * step (no drift); LOG stepping multiplies the current value.
 * - Text: linear values with the step's decimals (PCT as x100, no decimals); LOG values with 3 significant
 *   digits, trailing zeros dropped; Hz >= 1000 as kHz; ASCII '-'. Returns the length snprintf would give. */
#define PC_FXAPP_MAX 14                                        /* order-table entries with every port (velvet_room incl.) */
extern const pc_fxparam_t pc_fxapp_mix_param;                  /* the chain-level 濃度 of PC_FXA_MIX apps: id "mix", 0..1, step 0.05, def 1 */

/* gains that move linearly over one block (the next process() call) instead of jumping */
typedef struct { float cur, tgt; } pc_fx_ramp_t;
void  pc_fx_ramp_set(pc_fx_ramp_t *r, float tgt);              /* new target; reached at the end of the next block */
float pc_fx_ramp_at(const pc_fx_ramp_t *r, int i, int frames); /* value at frame i of a block of `frames`; = cur when cur == tgt */
void  pc_fx_ramp_end(pc_fx_ramp_t *r);                         /* after the block: cur = tgt */

/* ---- spec §5, verbatim ---- */
enum { PC_FXM_BATT = 1, PC_FXM_APTX = 2, PC_FXM_LDAC = 4, PC_FXM_CAST = 8,
       PC_FXM_HIRES = 16, PC_FXM_HIRES2 = 32, PC_FXM_BT = 64 };   /* reasons; 0 = normal; HIRES: pipeline rate > 48 kHz, HIRES2: > 96 kHz (both bits); BT: a Bluetooth sink carries the sound (any codec; no class pauses for it, only PC_FXA_WIRED_ONLY apps) */
int pc_fx_bench_block(uint32_t rate);                             /* fxbench: frames per process() call = min(rate/100, PC_FX_BLOCK) */
uint8_t pc_fx_rate_mode(uint32_t rate);                           /* the rate bits: > 96000 -> HIRES | HIRES2, > 48000 -> HIRES, else 0 */
bool pc_fx_allowed(pc_fxclass_t cls, uint8_t mode_bits);          /* the one table below (by class) */
bool pc_fx_app_allowed(const pc_fxapp_t *ap, uint8_t mode_bits);   /* what the chain and the list ask: the class rule plus the per-app re-allow (H27: the limiter runs under HIRES2) */
const char *pc_fx_mode_why(uint8_t mode_bits, bool zh);           /* "aptX 編碼需要 CPU" ... first reason that suspends heavy */
const char *pc_fx_app_why(const pc_fxapp_t *ap, uint8_t mode_bits, bool zh);   /* the reason this app is not allowed now (the class reason, or "wired playback only" for PC_FXA_WIRED_ONLY); NULL when none */

/* The registry entries, one per app (defined in the family files). Only pc_fxchain.c's order table names them;
 * everyone else goes through pc_fxapps() / pc_fxapp_find(). */
extern const pc_fxapp_t pc_fxapp_gain, pc_fxapp_balance, pc_fxapp_widener, pc_fxapp_crossfeed,
    pc_fxapp_equalizer, pc_fxapp_tone, pc_fxapp_bass, pc_fxapp_loudness, pc_fxapp_compressor,
    pc_fxapp_freeverb, pc_fxapp_limiter, pc_fxapp_tube, pc_fxapp_velvet_widener;
extern const pc_fxapp_t pc_fxapp_velvet_room;   /* defined only in a build with CONFIG_FX_VELVET_ROOM (Kconfig, default off) */
#ifdef __cplusplus
}
#endif
