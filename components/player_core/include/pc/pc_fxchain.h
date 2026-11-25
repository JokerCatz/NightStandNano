// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The fx chain (spec §3). The order table (pc_fxchain.c: pc_fxapps / pc_fxapp_find, declared in pc_fxapp.h), the slots,
 * the one-block fades, the mode rule per block, the SPSC mailbox (LVGL -> audio) and the return ring (audio -> LVGL).
 * Pure C, no ESP-IDF; every byte comes from the allocator passed to pc_fxchain_create (PSRAM on the board); the audio
 * side (pc_fxchain_process) allocates, frees and logs nothing.
 *
 * Threads: the LVGL side is the only producer of the mailbox (pc_fxchain_post) and the only consumer of the return
 * ring (pc_fxchain_take); the audio side the reverse. Indices are plain words moved with acquire/release atomics. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "pc/pc_fxapp.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct pc_fxchain pc_fxchain_t;
enum { PC_FXOP_SET = 1, PC_FXOP_SWAP, PC_FXOP_OFF, PC_FXOP_MIX };
typedef struct { uint8_t op, app, param; float v; void *ptr; } pc_fxmsg_t;
/* SET(app, param, v): non-REBUILD parameters only (a SET of a REBUILD one is ignored, see pc_fxchain_post): header value, reaches the app as one set() per drain. SWAP(app, ptr): install a
 * block made by pc_fxchain_block_init (= turn on). OFF(app): fade out, hand the block back. MIX(app, v): chain dry/wet. */
enum { PC_FXRET_FREE = 1, PC_FXRET_REBUILD };
typedef struct { uint8_t op, app; void *ptr; } pc_fxret_t;   /* FREE: ptr is a block to free; REBUILD: app needs a new block */
#define PC_FXMAIL_N 32
#define PC_FXRET_N 16

pc_fxchain_t *pc_fxchain_create_with(void *(*psram_alloc)(size_t), const pc_fxapp_t *const *tab, int n);   /* test seam: a chain over another table (spy apps); slot i uses tab[i]. Blocks still come from pc_fxchain_block_* of the registry, so the table's apps must fit the layout those blocks were built with. NULL when n > PC_FXAPP_MAX or an entry has nparam > PC_FXP_MAX */
pc_fxchain_t *pc_fxchain_create(void *(*psram_alloc)(size_t));      /* slots only; chain struct, scratch 2x480x2 floats, both rings from psram_alloc (one call) */
void   pc_fxchain_destroy(pc_fxchain_t *c, void (*release)(void *)); /* host tests and sim: frees remaining blocks (slots, mailbox, return ring) and the chain */
size_t pc_fxchain_block_size(int app, uint32_t rate, const float *v); /* header (values copy + mix) + app->state_size */
void  *pc_fxchain_block_init(void *blk, int app, uint32_t rate, const float *v, float mix);   /* LVGL side: header filled, app->init; returns blk */
/* LVGL side only (one producer); false = full. REBUILD parameters change only through SWAP (the new block's header carries
 * the value): the chain ignores a SET whose parameter has PC_FXP_REBUILD (no header write, no set(), no changed bit). */
bool   pc_fxchain_post(pc_fxchain_t *c, const pc_fxmsg_t *m);
bool   pc_fxchain_take(pc_fxchain_t *c, pc_fxret_t *out);            /* LVGL side; false = empty */
/* audio task; drains first; frames > PC_FX_BLOCK split. A new block is warmed up (runs at fade 0 for ceil(latency()/frames)
 * blocks, fed the slot's input) before it fades in; a running block it replaces stays at full until then. A rate change
 * is not click-free on a continuous signal (rate() clears history): callers change rate only at a track boundary. */
void   pc_fxchain_process(pc_fxchain_t *c, float *lr, int frames, uint32_t rate, uint8_t mode);
/* audio task only (same thread rule as pc_fxchain_process): is there still something for a block to do? True when the mailbox is not empty, a slot
 * has a replaced block, a block waits to retire (turned off), one is warming up or one is mid-fade. A caller with no source to play calls
 * process on silence only while this is true, so fades finish and blocks go back to the LVGL side while nothing plays; it ends within a bounded
 * number of blocks (look-ahead + two fades). A block suspended by the mode is not "warming". */
bool   pc_fxchain_busy(const pc_fxchain_t *c);
/* audio task only: the hook for a block with no source (lr is silent). Does nothing unless pc_fxchain_busy; otherwise runs the chain (messages
 * applied, fades finished, blocks handed back) and then zeroes lr: whatever the chain made of the silent input (the old music still in a limiter's
 * look-ahead line, a reverb tail resuming) is discarded, never played. */
void   pc_fxchain_idle(pc_fxchain_t *c, float *lr, int frames, uint32_t rate, uint8_t mode);
uint32_t pc_fxchain_rate_seen(const pc_fxchain_t *c);                /* atomic; 0 until the first block */
typedef struct { int mail_free, ret_free, blocks; } pc_fxchain_stat_t;
void   pc_fxchain_stat(const pc_fxchain_t *c, pc_fxchain_stat_t *out);
void   pc_fxchain_set_trace(pc_fxchain_t *c, void (*fn)(const pc_fxmsg_t *m, void *u), void *u);   /* called for each drained message (sim fake); set before the audio side starts, never while it runs */
void   pc_fxchain_set_probe(pc_fxchain_t *c, void (*fn)(int app, void *u), void *u);   /* called after each slot that ran this block, in order (tests only; production never sets it: one NULL check per slot); set before the audio side starts, never while it runs */
#ifdef __cplusplus
}
#endif
