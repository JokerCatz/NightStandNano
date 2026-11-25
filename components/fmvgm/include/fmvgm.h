// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* fmvgm: FM-chip music from VGM / VGZ (un-gzipped) and S98 files, on ymfm (BSD-3-Clause, see ymfm/LICENSE and SOURCE.md).
 * Plays YM2151, YM2203, YM2608, YM2610 / B, YM2612 / YM3438, AY8910 / YM2149, SN76489 and OKIM6295, one or two of each, from the chips' VGM / S98 streams, with the ADPCM / ROM data
 * the format carries. Chips may be spread over both cores of the host (lanes, below). A pc_backend_t (player_core's decoder interface), no other dependency:
 * the only outside services (memory, a second-core worker thread, a clock) come through fmvgm_port_t, which the firmware fills in once.
 *
 * What the backend does: opens a whole file from memory (pc_reader_t.mem, kept until close), runs the chips (integer, bit-exact with ymfm everywhere) on a producer, resamples the
 * chips' odd native rates (YM2151 clock/64, YM2203 clock/72 after a 3:1 decimation) to the output rate with an integer polyphase filter, applies the loop / fade rules, and hands int16
 * frames to render(), which converts them to float. The producer runs inline in render() or on a persistent worker (core 0 or core 1), switchable at run time.
 * Nothing allocates in the render path; the worker never deletes itself. */
#include <stddef.h>
#include <stdint.h>
#include "pc/pc_backend.h"
#ifdef __cplusplus
extern "C" {
#endif

extern const pc_backend_t pc_backend_fmvgm;   /* .vgm .vgz (un-gzipped by the caller) .s98; name "FM" */

/* ---- porting layer ------------------------------------------------------------------------------------------------------------------------------------------------ */
typedef enum { FMVGM_MEM_FAST = 0,    /* chip state, FIFOs: touched every sample (internal RAM where there is a choice) */
               FMVGM_MEM_BULK = 1 } fmvgm_mem_t;   /* ring, coefficient table: PSRAM is fine */
typedef struct fmvgm_port {
    void *(*alloc)(size_t n, fmvgm_mem_t kind);          /* 8-byte aligned; NULL when out of memory. Called at open / close only, never while rendering */
    void  (*free)(void *p);
    /* a persistent worker thread: fn(arg) runs for ever. core 0 / 1 as the platform numbers them. NULL when the platform has no threads (then only FMVGM_PLACE_INLINE works) */
    void *(*task_start)(void (*fn)(void *arg), void *arg, int core, int prio, uint32_t stack_bytes);
    void  (*task_wake)(void *task);                      /* any thread: wakes task_wait of that task (also from the consumer when it has freed ring space) */
    void  (*task_wait)(uint32_t ms);                     /* on the worker: returns at a task_wake or after ms */
    void  (*task_prio)(void *task, int prio);            /* change the priority of a running worker; may be NULL */
    void  (*sleep_ms)(uint32_t ms);                      /* caller sleeps (at least 1 ms of yield) */
    uint32_t (*now_us)(void);                            /* free-running microsecond counter (wraps at 2^32) */
} fmvgm_port_t;
void fmvgm_set_port(const fmvgm_port_t *port);           /* before the first open; the default is malloc / free and inline rendering only */

/* ---- placement and tuning (any task, effective for the next render / open as noted) ---------------------------------------------------------------------------------- */
enum { FMVGM_PLACE_INLINE = 0, FMVGM_PLACE_CORE0 = 1, FMVGM_PLACE_CORE1 = 2 };
void fmvgm_set_placement(int place);                     /* every chip of the next open; a single-lane session also migrates at its next render() when this changes */
/* Split rendering: chips that share a placement form one lane (a worker thread on that core, or the consumer inline), each with its own cursor in the command stream and its own ring; render()
 * mixes the lanes. The plan gives the placement of each chip of the next open(s), in the order of fmvgm_info_t.chip[] (header order, a dual chip counts twice); n = 0 clears it and every chip
 * follows fmvgm_set_placement. The output does not depend on the plan: every chip is resampled on its own and the lanes are summed in integers (host test: bit-identical for every plan). */
void fmvgm_set_plan(const uint8_t *place_of_chip, int n);
int  fmvgm_placement(void);
void fmvgm_set_worker_priority(int prio);                /* live; default 12 (measured, docs/design.md): above the USB host and housekeeping tasks, below lwIP, the cast task and the Wi-Fi / Bluetooth stacks */
void fmvgm_set_out_rate(uint32_t hz);                    /* the rate render() delivers (what rate() reports) for the next open; 0 = the chip's native rate, no resampling */
void fmvgm_set_taps(int taps);                           /* resampler length, even, 8..48, next open (default 24) */
void fmvgm_set_ring_ms(int ms);                          /* producer ring depth at the output rate, next open (default 340) */
void fmvgm_set_loops(int n);                             /* a looping file plays its loop this many times after the intro (default 2), then fades out for FMVGM_FADE_MS */
void fmvgm_set_checksum(int on);                         /* next open: keep the integer checksums below (small cost) */
void fmvgm_set_overload(uint32_t window_ms, uint32_t thr_ms);   /* render() reports overload (-2) when more than thr_ms of silence was inserted within window_ms of output; thr_ms 0 = never */
void fmvgm_get_overload(uint32_t *window_ms, uint32_t *thr_ms);   /* the same numbers, for a host that applies the rule to something else too (an output ring that keeps running dry under an inline producer) */
#define FMVGM_FADE_MS 8000
#define FMVGM_E_OVERLOAD (-2)                            /* render(): the producer cannot keep up (see fmvgm_set_overload) */

/* Partial playback: chips the file writes that nobody here plays (sample chips) are left out when their PC_VGM_BIT is in this mask, instead of refusing the file (open then reports them in
 * fmvgm_info_t.dropped_names). The host decides which chips may be dropped (a melodic chip may not). 0 = refuse every unplayable chip. */
void fmvgm_set_drop(uint64_t chips);
uint64_t fmvgm_played_chips(void);                       /* PC_VGM_BIT mask of the chips this backend plays */
/* Optional ROM images the chips cannot get from the file. The memory must stay valid while any file is open (it is referenced, not copied). */
enum { FMVGM_ROM_YM2608_RHYTHM = 1 };                    /* the 8 KB ADPCM-A image of the YM2608's rhythm section, in the layout of ymfm's ym2608_adpcm_rom.bin; without it the rhythm channels stay silent */
void fmvgm_set_rom(int which, const void *data, uint32_t n);   /* n = 0 removes it */

/* ---- looking at a file before opening it (any task; whole un-gzipped file in memory) ---------------------------------------------------------------------------------------- */
/* Routing of a .vgm / .vgz: 0 = fmvgm plays every chip the file writes; 1 = only GME's chips (SN76489 YM2413 YM2612: GME's job); 2 = both kinds in one file (fmvgm cannot play
 * the GME chip: refused); 3 = a chip nobody here plays; -1 = not a VGM. used / missing: PC_VGM_BIT masks (pc_vgmhdr.h) of the chips written, and of those fmvgm cannot play. */
int fmvgm_route(const void *mem, uint32_t size, uint64_t *used, uint64_t *missing);
int fmvgm_is_s98(const void *mem, uint32_t size);        /* 1 when it starts like an S98 file */
/* What an S98 file needs, by name: "" when fmvgm can play every device in it, else "YM2608, SN76489" (the devices it cannot). -1 = not an S98 file. */
int fmvgm_s98_missing(const void *mem, uint32_t size, char *names, size_t cap);

const char *fmvgm_error(void);                           /* why the last open failed ("YM2608", "bad VGM header", "YM2151 clock 12000000 Hz" ...), "" when it did not */

/* ---- an open handle (the void * of pc_backend_t) ---------------------------------------------------------------------------------------------------------------------------- */
typedef struct { char title[96], author[96], game[96], system[48]; } fmvgm_tags_t;   /* UTF-8; English fields, the Japanese ones when those are empty (Shift-JIS bytes in S98 tags are shown as '?') */
void fmvgm_tags(void *h, fmvgm_tags_t *t);
typedef struct {
    int      format;                 /* 0 VGM, 1 S98 */
    int      n_chips;
    struct { char name[10]; uint8_t type; uint8_t place; uint32_t clock; uint32_t native_rate; } chip[4];   /* native_rate: what the chip delivers to the filter (YM2203: clock / 72); place: FMVGM_PLACE_* of its lane (open handle) */
    uint32_t out_rate;               /* what render() delivers */
    int      loops;                  /* the file has a loop */
    int32_t  length_ms;              /* intro + loop count * loop, or the whole song; -1 unknown (180 s is then assumed by the player) */
    int32_t  loop_ms;
    int      taps;                   /* 0 = no resampling */
    int      placement;              /* FMVGM_PLACE_* in force for this handle */
    uint32_t chip_bytes, chip_bytes_cap; int chip_spill;   /* the chips' arena: used / reserved, and allocations that did not fit it (must be 0) */
    int      lanes;                  /* rendering lanes of this handle (1 = all chips on one core) */
    uint32_t adpcm_bytes;            /* ADPCM / ROM data the file carries (probe) */
    int      rom_missing;            /* a YM2608 is used and its rhythm ROM was not supplied: the rhythm channels are silent */
    uint64_t dropped; char dropped_names[40];   /* chips left out under fmvgm_set_drop */
} fmvgm_info_t;
void fmvgm_info(void *h, fmvgm_info_t *i);
/* The same facts from the file alone, without building a handle (no chips, no memory beyond the stack): 0, or -1 with fmvgm_error() set. For routing and budget checks. out_rate / taps / placement
 * in `i` are what an open now would use. */
int  fmvgm_probe(const void *mem, uint32_t size, fmvgm_info_t *i, fmvgm_tags_t *t);
int  fmvgm_probe_drop(const void *mem, uint32_t size, fmvgm_info_t *i, fmvgm_tags_t *t, uint64_t drop);   /* with an explicit drop mask (a task other than the one that opens must not touch the global one) */

/* statistics of the open handle (h) or, when h is NULL, a snapshot of the newest handle refreshed about every 250 ms while it renders and when it closes (safe to read from any task); counters since open / the last restart */
typedef struct {
    uint32_t frames_made;            /* output frames the producer produced */
    uint32_t frames_read;            /* output frames render() handed on */
    uint32_t starved;                /* render() calls that found fewer frames than asked (producer behind) */
    uint32_t starved_ms;             /* milliseconds of silence inserted for that */
    uint32_t window_ms;              /* silence inserted in the current overload window */
    uint32_t full_waits;             /* producer found the ring full (it is ahead: good) */
    uint32_t low_water;              /* lowest ring fill seen at the start of a render(), frames */
    uint32_t ring_frames;            /* ring capacity */
    uint32_t busy_us;                /* microseconds the producers spent making frames, all lanes (wall time, a preempted producer counts that too) */
    int      lanes; uint32_t lane_busy_us[3]; int lane_place[3];   /* the same per lane (placement -1: no such lane) */
    uint32_t made_ms;                /* audio milliseconds made (for busy_us / made_ms = cost per audio second, in thousandths) */
    uint32_t migrations;
    int      overloaded;
    int      placement;
} fmvgm_stats_t;
void fmvgm_stats(void *h, fmvgm_stats_t *s);

/* integer checksums (when enabled before open): of the chips' raw output (before any mixing or resampling: the same numbers ymfm gives anywhere) and of the frames render() receives */
typedef struct {
    int64_t  chip_sum_l, chip_sum_r;  /* sum of the chips' raw left / right samples (after the OPN FM + SSG sum, at the chip's own rate) */
    uint64_t chip_samples;            /* raw samples summed (all chips) */
    uint32_t out_hash;                /* FNV-1a over every output int16 sample (L, R, L, R ... little endian) */
    uint64_t out_frames;
    uint32_t table_hash;              /* FNV-1a of the resampler's coefficients (0 when not resampling) */
} fmvgm_sums_t;
int fmvgm_sums(void *h, fmvgm_sums_t *s);   /* 0, or -1 when checksums were not enabled for this handle */

/* Render the whole file as fast as possible on the calling thread without a ring (host tests, the board's `dbg fm bench`): seconds of audio, 0 = to the end; returns frames made and
 * fills the busy time. */
uint64_t fmvgm_bench(void *h, uint32_t seconds, uint32_t *busy_us);

#ifdef __cplusplus
}
#endif
