// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Polyphase windowed-sinc resampler for interleaved stereo float: every Bluetooth rate conversion (44.1 <-> 48, LC3 48 -> 24).
 * One table per rate pair (PC_RSQ_PHASES + 1 rows of PC_RSQ_TAPS floats, 70.5 KB with 68 taps, Kaiser beta 8.6, cutoff 0.5 * min(in, out), unity DC gain per row),
 * the output position is exact (integer fraction in 1/out_rate units) and the table is read at the nearest phase pair with linear
 * interpolation between them: one code path for every ratio. The output is the input delayed by PC_RSQ_TAPS / 2 input frames.
 * The state block is fixed size (a PSRAM block the caller allocates once; pc_rsq_init rebuilds it in place). */
#define PC_RSQ_PHASES 256
#define PC_RSQ_TAPS 68              /* 56 gives 68.8 dB of image rejection at 44.1 -> 48, 64 gives 75.1 dB of alias rejection at 48 -> 24; 66 and 68 pass all (test_rsq pins it) */

typedef struct pc_rsq pc_rsq_t;
size_t pc_rsq_size(void);                        /* bytes the caller allocates (multiple of 8; the same for every rate pair) */
/* 0 ok; -1 if a rate is 0 or the ratio is outside 1/4 ... 4. Builds the table (a few ms; call it where blocking is allowed). */
int    pc_rsq_init(pc_rsq_t *q, uint32_t in_rate, uint32_t out_rate);
/* Same contract as pc_rs_process: up to out_cap output frames from in_frames input frames, *consumed = input frames taken (all of them unless
 * out_cap ran out first: call again with the rest). The state carries across calls of any size. Returns the output frames written. */
int    pc_rsq_process(pc_rsq_t *q, const float *in, int in_frames, float *out, int out_cap, int *consumed);
/* Back to the state pc_rsq_init leaves (silent history, position 0), the table kept: a fresh stream at the same pair (sink on, after a
 * pass-through stretch). Cheap (no table work): the audio task may call it. */
void   pc_rsq_reset(pc_rsq_t *q);

/* hires T10b (H29: the table build takes about 26 ms on the board, too long for the audio task). Two table blocks: the live one the audio
 * task reads and a spare that another task (the builder) fills for the next pair; the audio task swaps them at the pair change. Plain loads
 * and stores only (the state may sit in PSRAM, where read-modify-write atomics are not allowed); every field has one writer at a time:
 *   ready 0 -> 1: the builder, after it built blk[!live] (pair[!live] written before, release);
 *   ready 1 -> 0, live: the audio task (a swap: live flips, then ready 0; or the spare holds a pair nobody wants now: ready 0, no flip);
 *   req: the audio task (the pair it waits for; cleared when a block no longer waits), read by the builder (a torn read builds a wrong pair,
 *        which the audio task's exact check refuses and asks for again: never used);
 *   gen: the builder (one more at each publish, before ready 1); reclaim: the builder (fix 2: the gen of a ready spare it wants back because
 *        its pair is neither asked for, nor the plan's, nor the next track's - the audio task hands that spare back at its next block, no flip;
 *        a later publish has another gen, so an old reclaim never takes a new spare);
 *   fresh: set by any task (pc_rsq_swap_restart), cleared by the audio task right before it resets the live table's history.
 * While ready is 0 only the builder touches blk[!live]; while it is 1 nobody writes it.
 * The hand-off, step by step (fix 2; test_rsq_order pins every store order below and its acquire/release):
 *   1. ready 0: the builder owns blk[!live] and pair[!live]; live cannot change (a flip needs ready 1). It loads ready (acquire), then live.
 *   2. it builds, writes pair[!live], gen + 1, then stores ready = 1 (release): table, pair and gen are visible to whoever sees ready 1.
 *   3. ready 1: nobody writes the spare. The audio task loads ready (acquire) before it reads pair[!live] / gen.
 *   4a. swap: the audio task stores live = !live, then ready = 0 (release, in this order): a builder that sees ready 0 sees the new live and
 *       takes the old block as its spare - never the block that just became live.
 *   4b. hand-back (a WAIT for another pair, or the builder's reclaim): ready = 0 (release), no flip.
 * The audio task processes only blk[live] (after 4a: the new live one), the builder only blk[!live] while ready is 0. */
typedef struct { uint32_t in, out; } pc_rsq_pair_t;          /* a (source rate, link rate) pair; {0, 0} = none */
typedef enum { PC_RSQ_PASS, PC_RSQ_USE, PC_RSQ_SWAP, PC_RSQ_WAIT } pc_rsq_pick_t;
/* the audio task's rule for one block at pair want: PASS (in == out: no table), USE (the live table holds want), SWAP (the ready spare holds
 * want), WAIT (neither: the block is dropped, never resampled with another pair's table). ready = the spare's pair when it is ready, else {0, 0}. */
pc_rsq_pick_t pc_rsq_pick(pc_rsq_pair_t want, pc_rsq_pair_t live, pc_rsq_pair_t ready);
/* the builder's rule: the pair to build into the free spare, {0, 0} = nothing. The audio task's request first, then the plan the audio side
 * published (now: the pair of the blocks being made), then the look-ahead (next: the queued next track's pair, fix 1: built before a gapless join
 * so the swap there finds it ready); a pass-through pair, a zero rate or the live table's own pair is never built. */
pc_rsq_pair_t pc_rsq_build_pick(pc_rsq_pair_t req, pc_rsq_pair_t now, pc_rsq_pair_t next, pc_rsq_pair_t live);
/* fix 2, the builder's rule for a ready spare: nonzero when nobody wants its pair - not the audio task's request, not the plan of the moment, not
 * the queued next track's (a 96k table after the plan halved to 48k, a next track that was cancelled or replaced): the builder reclaims it */
int pc_rsq_spare_unwanted(pc_rsq_pair_t spare, pc_rsq_pair_t req, pc_rsq_pair_t now, pc_rsq_pair_t next);

typedef struct {
    pc_rsq_t *blk[2];
    pc_rsq_pair_t pair[2];      /* the pair each block's table was built for ({0, 0}: none yet) */
    int live, ready, fresh;
    pc_rsq_pair_t req;
    uint32_t swaps;             /* 10c fix: swap count (audio task); a reclaim is tagged gen * 65536 + swaps, so one decided before a flip never matches */
    uint32_t gen, reclaim;      /* fix 2: the publish count (gen of the ready spare) and the gen the builder wants back (0: none) */
} pc_rsq_swap_t;
void pc_rsq_swap_setup(pc_rsq_swap_t *s, pc_rsq_t *a, pc_rsq_t *b);   /* once, before any other call (a or b NULL: every resampled block waits) */
/* audio task, per block: the pick for (in, out); *q = the table to process with on USE and SWAP (after a swap or a restart its history is
 * fresh), NULL otherwise. On WAIT the request is (re)published and a spare holding another pair is handed back to the builder; on PASS and USE
 * the request is cleared and a spare the builder reclaimed (fix 2) is handed back. */
pc_rsq_pick_t pc_rsq_swap_take(pc_rsq_swap_t *s, uint32_t in, uint32_t out, pc_rsq_t **q);
/* builder: the block to build and its pair (*build) when the spare is free and pc_rsq_build_pick(request, now, next, live) names a pair; NULL = nothing to do now.
 * A ready spare nobody wants (pc_rsq_spare_unwanted) is reclaimed: the audio task hands it back at its next block and a later call builds into it. */
pc_rsq_t *pc_rsq_swap_spare(pc_rsq_swap_t *s, pc_rsq_pair_t now, pc_rsq_pair_t next, pc_rsq_pair_t *build);
void pc_rsq_swap_publish(pc_rsq_swap_t *s, pc_rsq_pair_t built);     /* builder: the spare from pc_rsq_swap_spare is built for `built` (pc_rsq_init returned 0) */
/* 10c: true when a table for p is live or sits ready in the spare (the sink start waits for this before the first block; a pass-through or zero pair: 0).
 * Any task; plain acquire loads. 10c cache: after a swap the old live table stays as the ready spare, a pointer swap back; the builder evicts it
 * only to build another pair (nothing else to build: it stays). */
int pc_rsq_swap_has(pc_rsq_swap_t *s, pc_rsq_pair_t p);
/* 10c fix: nonzero while the spare holds a ready table (the cache): the builder cannot build until the audio task hands it back at a block */
int pc_rsq_swap_spare_busy(pc_rsq_swap_t *s);
void pc_rsq_swap_restart(pc_rsq_swap_t *s);                           /* any task: the next resampled block starts from a silent history (sink on) */

#ifdef __cplusplus
}
#endif
