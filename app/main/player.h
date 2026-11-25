// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdint.h>
#include "esp_err.h"

/* File -> decoder (JukeDial player_core pc_backend_t) -> S16 ring -> USB DAC. One track at a time for now. */
esp_err_t nsn_player_play(const char *path);      /* async; replaces the current track */
esp_err_t nsn_player_play_why(const char *path, char *why, size_t n);   /* same; a VGM refused for its chips returns ESP_ERR_NOT_SUPPORTED with them in `why` ("YM2151, OKIM6295") */
void      nsn_player_stop(void);
bool      nsn_player_active(void);
void nsn_player_set_rg(int mode);                 /* ReplayGain: 0 off, 1 track, 2 album (from the file's tags; untagged files are left alone) */
int  nsn_player_rg(void);
esp_err_t nsn_player_resume(void);                /* continue the bookmarked file (its folder as the playlist) from where it stopped */
void nsn_player_set_bookmark(const char *path, uint32_t pos);   /* boot: what NVS remembered */
bool nsn_player_bookmark_dirty(char *path, size_t n, uint32_t *pos);   /* main task: the bookmark changed, store it */
const char *nsn_player_bookmark(void);            /* "" = none */
void      nsn_player_status(char *out, int n);    /* one line for the console */

typedef struct { bool active, paused; const char *fmt; uint32_t rate; int32_t pos_ms, len_ms; uint32_t underruns; int volume; int index, count, sleep_left_s, sub, subn; char path[160]; char title[128], artist[96], album[128]; } nsn_player_info_t;
void nsn_player_info(nsn_player_info_t *out);
bool nsn_player_take_dropped(char *out, size_t n);   /* track interrupted by a Bluetooth drop (<10 min ago); main resumes it after reconnect */
void nsn_player_pause(bool on);
void nsn_player_set_volume(int pct);              /* 0..100, perceptual (cubic) software gain, never above the limit; sets the volume of the output playing now (idle: the one that would be picked) */
int  nsn_player_volume(void);                     /* that output's volume (the effective one) */
void nsn_player_set_vollimit(int n);              /* 0..100: no volume above it is applied or shown */
int  nsn_player_vollimit(void);
void nsn_player_set_gapless(bool on);
bool nsn_player_gapless(void);                    /* track ends flow into the next track of the playlist when rate and output are the same */
void nsn_player_set_expfmt(bool on);              /* the experimental tier of tracker / AdLib readers (a2m d00 sa2 ...): on by default, NVS "expfmt" */
bool nsn_player_expfmt(void);
void nsn_player_init_settings(void);              /* boot: gapless, limit, per-output volumes from NVS */
void nsn_player_persist(void);                    /* write what changed to NVS; only from a task with an internal stack */
void nsn_player_vol_forget(int o);                /* debug: 1 Bluetooth, 2 Auracast forget their own volume */
void nsn_player_debug_opendelay(int ms);       /* debug: every open of a track takes this much longer */
void nsn_player_vol_dump(char *out, int n);       /* debug */
void nsn_player_ramp_dump(char *out, int n);      /* debug */
void nsn_player_set_mode(int m);                 /* 0 folder repeat, 1 shuffle, 2 repeat one, 3 once */
int  nsn_player_mode(void);
void nsn_player_skip(int dir);                 /* +1 next, -1 previous (folder play) */
void nsn_player_sleep(int minutes);            /* 0 cancels; fades out over the last 30 s, then stops */
enum { NSN_OUT_AUTO, NSN_OUT_DAC, NSN_OUT_BT, NSN_OUT_CAST, NSN_OUT_I2S };   /* = the sink ids of sink.h; CAST: Auracast broadcast (cast.h), never chosen by AUTO; I2S exists only with a board sink */
void nsn_player_set_output(int pref);           /* applies from the next track; auto = the best present sink (sink.h auto_rank: DAC, then I2S if the board has one, then Bluetooth) */
int  nsn_player_output(void);
int  nsn_player_sink(void);                    /* what is playing now: the sink id (sink.h; 1 DAC, 2 Bluetooth headphones, 3 Auracast broadcast, 4 I2S), 0 none */
void nsn_player_set_volume_temp(int pct);
void nsn_player_follow_phone(void);              /* speaker mode: play "bt:in" whenever a phone streams, stop when it stops */
void nsn_player_set_dacmax(uint32_t hz);          /* ceiling for the USB DAC: 0 = auto, else 48000 / 96000 / 192000 / 384000 (anything else is rounded down to a tier); applies from the next track */
uint32_t nsn_player_dacmax(void);                /* 0 = auto */
bool nsn_player_can_play(const char *name);       /* a decoder exists for this file name's extension (what `ls` shows inside a zip archive) */
