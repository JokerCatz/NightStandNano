// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Small persistent settings in NVS (namespace "nsn"). Saved when changed, loaded once at boot. */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef struct {
    int vol;            /* 0..100 */
    int out;            /* NSN_OUT_AUTO / DAC / BT */
    int led;            /* LED peak level 0..255 */
    int autoplay;       /* 1 = play `last` at boot (off by default: a bedside box must not start by itself in the night) */
    int alarm_min;      /* (the three alarm_* fields are unused since the alarm was dropped 2026-11-24; kept so stored settings stay readable) */
    int alarm_ramp_s;   /* volume rises from 0 to the set volume over this long */
    char alarm_src[128];/* file / folder / radio url for the alarm; empty = `last` */
    int bt_mode;        /* 0 = speaker (A2DP sink: phones stream to us), 1 = headphones (A2DP source) */
    int bt_have;        /* 1 = bt_addr is the last A2DP device that connected: the box reconnects to it by itself */
    uint8_t bt_addr[6];
    char last[128];     /* last file / folder / radio url handed to `play` / `radio` */
    int play_mode;      /* 0 = folder repeat, 1 = shuffle, 2 = repeat one, 3 = play once (the NVS blob of older builds lacks this field: still read) */
} nsn_settings_t;
extern nsn_settings_t g_set;
void nsn_settings_load(void);      /* defaults, then NVS */
void nsn_settings_save(void);      /* one NVS key per field; only changed keys are written; thread-safe */
/* Other modules' own integer values in the same namespace (key max 15 chars). Thread-safe; set skips an unchanged value. */
int nsn_settings_get_i32(const char *key, int def);
void nsn_settings_set_i32(const char *key, int v);
/* Other modules' own strings (value max 127 chars). get returns false when the key is absent; set skips an unchanged value; erase removes the key. */
bool nsn_settings_get_str(const char *key, char *buf, size_t cap);
bool nsn_settings_set_str(const char *key, const char *v);     /* false: not stored */
void nsn_settings_erase(const char *key);
