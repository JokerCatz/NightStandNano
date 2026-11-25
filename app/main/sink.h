// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Audio outputs ("sinks") as a table. The player decodes, resamples and volume-scales into one PSRAM ring; the selected sink pulls PCM from it
 * through one callback and plays it somewhere: the USB DAC (uac_host.c), Bluetooth headphones (bt_audio.c), the Auracast broadcast (cast.c) and,
 * on a board that has one, an I2S codec (board/sink_i2s.c, a template). player.c's built-in three are registered by nsn_player_init_settings();
 * the board adds its own in board_sinks_register() (board/board.h).
 *
 * A sink's id is its position in the table, from 1 (0 = none). Ids are also what `st` / the volume keys / `out` use, so the built-in ids never change:
 * 1 dac, 2 bt, 3 cast; a board sink gets 4 and up. The id doubles as the output preference value (player.h NSN_OUT_*; 0 = auto). */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

/* Fill `stereo` with `frames` interleaved S16 frames (L,R); returns the frames written, the rest the sink pads with silence. Runs in the sink's own
 * context (USB task, BT task, encoder task): fast, no blocking, no NVS, no logging. */
typedef size_t (*nsn_sink_pull_t)(int16_t *stereo, size_t frames, void *ctx);

typedef struct nsn_sink {
    const char *name;            /* "dac" "bt" "cast" "i2s": what `st` reports in "out", what `out <name>` takes */
    const char *label;           /* short name in the "track -> <label> <rate> Hz" log line */
    const char *desc;            /* long name in "first use of <desc>" */
    const char *vol_key;         /* NVS key of this output's own volume; NULL = the main volume (g_set.vol) */
    uint8_t auto_rank;           /* `out auto` takes the present sink with the lowest rank; 0 = never chosen by auto (Auracast: an explicit choice) */
    bool resume_after_drop;      /* when it disappears mid-track, remember the track so main.c can resume it when the output is back (Bluetooth) */
    const char *gone_msg;        /* warning logged when present() turns false while the sink plays */
    bool (*present)(void);                                   /* ready to take audio now (device attached / link up / broadcast on); cheap, any task */
    uint32_t (*link_rate)(uint32_t src_rate);                /* the rate to run at for a source at src_rate; the player resamples when they differ */
    void (*describe)(char *buf, size_t n);                   /* optional: extra text for the log line (", dacmax auto"); NULL = none */
    esp_err_t (*start)(uint32_t rate, nsn_sink_pull_t pull, void *ctx);   /* begin pulling; called when the ring is half full */
    void (*stop)(void);                                      /* stop pulling (and unhook pull); idempotent */
} nsn_sink_t;

#define NSN_SINK_MAX 6
int  nsn_sink_register(const nsn_sink_t *s);     /* static storage; returns the id, 0 when the table is full. Boot time only (not thread-safe) */
const nsn_sink_t *nsn_sink_get(int id);          /* NULL for 0 / unknown */
int  nsn_sink_count(void);
int  nsn_sink_by_name(const char *name);         /* 0 = no such sink */
const char *nsn_sink_name(int id);               /* "none" for 0 / unknown */
int  nsn_sink_auto(void);                        /* the sink `out auto` plays on now: present, lowest auto_rank (ties: lower id); 0 = none */
