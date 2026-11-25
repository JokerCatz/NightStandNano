// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* One volume per output (owner 2026-11-22; replaces the 2026-11-14 "one volume" ruling). Pure: no I/O, no state of its own.
 * Outputs: WIRED (the 3.5 mm out; also what speaker mode and Auracast listening play through), BT (Bluetooth headphones, one value per
 * headset address), CAST (the Auracast broadcast: it keeps the wired value, as before the split).
 * The headsets' values live in one bounded text ("aabbccddeeff:40,112233445566:33", most recently used first, at most PC_VOLSEL_MAX
 * entries: Bluedroid's own bond table holds 15). A headset that is not in the text starts at PC_VOLSEL_BT_DEFAULT (about one third). */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define PC_VOLSEL_BT_DEFAULT 33
#define PC_VOLSEL_SPK_DEFAULT 60        /* the wired jack in Bluetooth speaker mode: the phone has its own volume in front of ours, so this starts higher than an ordinary wired listen */
#define PC_VOLSEL_MAX 15
#define PC_VOLSEL_TEXT 256              /* 15 entries of at most 16 characters and their commas, the NUL: fits one settings value (pc_kv_t.val) */

/* SPEAKER: the jack while the device is in Bluetooth speaker mode (phone -> device -> jack) or Auracast listening mode (a broadcast -> device -> jack): its own stored value */
typedef enum { PC_VOUT_WIRED, PC_VOUT_BT, PC_VOUT_CAST, PC_VOUT_SPEAKER } pc_vout_t;

static inline bool pc_volsel_is_wired_value(pc_vout_t out) { return out == PC_VOUT_WIRED || out == PC_VOUT_CAST; }   /* CAST shares the wired value; SPEAKER has its own */

/* a stored or default value inside [0, max] (max itself held to 0..100) */
int pc_volsel_clamp(int v, int max);
/* the wired output's value: stored (< 0: nothing stored -> PC_VOL_DEFAULT), clamped */
int pc_volsel_wired(int stored, int max);
/* the jack's value in speaker mode: stored (< 0: nothing stored -> PC_VOLSEL_SPK_DEFAULT), clamped */
int pc_volsel_spk(int stored, int max);
/* the headset's value: its entry in `list` (NULL or no entry -> PC_VOLSEL_BT_DEFAULT), clamped */
int pc_volsel_bt(const char *list, const uint8_t bda[6], int max);
/* store `vol` for the headset: its entry moves to the front, the least recently used entry beyond PC_VOLSEL_MAX (or beyond cap) is dropped.
 * `list` is rewritten in place (cap >= PC_VOLSEL_TEXT keeps all PC_VOLSEL_MAX). true: the text changed */
bool pc_volsel_bt_put(char *list, size_t cap, const uint8_t bda[6], int vol);
/* the headset connected: an existing entry moves to the front (it is the most recently used); nothing is added. true: the text changed */
bool pc_volsel_bt_touch(char *list, size_t cap, const uint8_t bda[6]);
/* the headset was forgotten: its entry goes. true: the text changed */
bool pc_volsel_bt_del(char *list, const uint8_t bda[6]);
int  pc_volsel_bt_count(const char *list);
/* "wired", "bt:aabbccddeeff", "cast", "speaker" (out: at least 16 bytes) */
const char *pc_volsel_name(pc_vout_t out, const uint8_t bda[6], char *buf, size_t cap);

/* ---- the change of output ----
 * `req`: the value last asked of the audio path; `old_full`: the old output's own value; `new_full`: the new output's own value.
 * The value the new output gets: its own, except while something holds the volume below the old output's own value (an alarm's ramp, a
 * duck): then that lower value carries over (never above the new output's own) and its owner raises it as it would have. */
int pc_volsel_next(int req, int old_full, int new_full);
/* the value in force across the switch: never above what plays now (`applied`), never above what the new output gets */
int pc_volsel_across(int applied, int next);

/* ---- the hold: a ceiling on every value handed to the audio path while the output changes ----
 * begin(cap): values above cap are kept back (the newest one is remembered); end(): the remembered value, to be applied now, or -1.
 * A second begin while one is open only lowers the ceiling. Zero-initialised = no hold. */
typedef struct { int cap1, want1; } pc_volhold_t;       /* each value + 1; 0 = none */
void pc_volhold_begin(pc_volhold_t *h, int cap);
int  pc_volhold_apply(pc_volhold_t *h, int vol);        /* what the audio path may take for a request of vol */
int  pc_volhold_end(pc_volhold_t *h);
static inline bool pc_volhold_on(const pc_volhold_t *h) { return h->cap1 != 0; }
/* the newest value asked for: the kept-back one while a hold is open, else `applied` */
static inline int pc_volhold_req(const pc_volhold_t *h, int applied) { return h->cap1 && h->want1 ? h->want1 - 1 : applied; }

#ifdef __cplusplus
}
#endif
