// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The card's paths, in one place (2026-11-15). Everything the device makes or needs for itself lives under PC_INT
 * ("/sdcard/_player", a visible folder; backup = copy it): config/, fonts/, sleep/, caches, logs, flag files.
 * What the owner puts on the card (music, lists, vocab, chiptune, photos, export, instrument kits/fm, fortune texts)
 * stays at the top level. The library scan skips PC_INT_NAME. tools/check/ui_lint.py: no other file spells these literals. */

#define PC_SD            "/sdcard"
#define PC_INT_NAME      "_player"
#define PC_INT           PC_SD "/" PC_INT_NAME

/* ---- internal (device-owned) ---- */
#define PC_CFG_DIR       PC_INT "/config"
#define PC_CFG_APP       PC_CFG_DIR "/app.txt"
#define PC_CFG_STATS     PC_CFG_DIR "/stats.txt"
#define PC_CFG_GAMES     PC_CFG_DIR "/games.txt"
#define PC_CFG_FX        PC_CFG_DIR "/fx.txt"
#define PC_CFG_WIFI      PC_CFG_DIR "/wifi.txt"
#define PC_CFG_SCENES    PC_CFG_DIR "/scenes.json"
#define PC_CFG_KEYS      PC_CFG_DIR "/keys.txt"
#define PC_CFG_CARDS     PC_CFG_DIR "/cards.txt"        /* fortune cards the machine keeps */
#define PC_CFG_CRASHED   PC_CFG_DIR "/crashed.txt"       /* crash safety net: ids of tracks that were playing at a crash (pc_crashnet.h) */
#define PC_SOUND_DIR     PC_CFG_DIR "/sound"            /* sound-effect presets, imported / exported */
#define PC_FONTS_DIR     PC_INT "/fonts"
#define PC_SLEEP_DIR     PC_INT "/sleep"
#define PC_NOISE_MUK     PC_SLEEP_DIR "/library.muk"
#define PC_FACES_ZIP     PC_INT "/faces.zip"            /* the designed HUD faces: one STORE-method zip (tools/gen/make_faces.py; ui/face_art.c); a plug-in, the firmware works without it */
#define PC_PACK_TXT      PC_INT "/pack.txt"             /* the card pack's version stamp (tools/gen/make_card_zip.py; pc_pack.h, app/main/pack_stamp.c) */
#define PC_INDEX_DIR     PC_INT "/index"
#define PC_LIB_INDEX     PC_INDEX_DIR "/library.bin"
#define PC_THUMB_PAK     PC_INDEX_DIR "/thumbs.pak"
#define PC_THUMB_PAK_NEW PC_INDEX_DIR "/thumbs.new"
#define PC_ALBUM_CACHE   PC_INT "/album"
#define PC_LOOPS_DIR     PC_INT "/loops"                /* machine-recorded instrument loops (.jcl) */
#define PC_LOGS_DIR      PC_INT "/logs"
#define PC_SPLASH_JPG    PC_INT "/splash.jpg"           /* the power-on picture, general one (pc_splash.h); OPTIONAL user file */
#define PC_SPLASH_DIR    PC_INT "/splash"               /* per boot mode: PC_SPLASH_DIR/<mode>.jpg wins over PC_SPLASH_JPG */
#define PC_CARDTEST_TMP  PC_INT "/cardtest.tmp"         /* card diagnostics: the file test's 4 MB temp file (made and removed by a test; app/main/card_test.c) */
#define PC_CARDTEST_TXT  PC_INT "/cardtest.txt"         /* card diagnostics: the latest result of each mode, for this card (pc_cardtest.h) */
#define PC_FLAG_CRASHTEST PC_INT "/.crashtest"
#define PC_FLAG_SOAK     PC_INT "/.soak"
#define PC_FLAG_RISKY    PC_INT "/.app_risky"

/* ---- user content (top level) ---- */
#define PC_EXPORT_DIR    PC_SD "/export"
#define PC_LISTS_DIR     PC_SD "/lists"
#define PC_VOCAB_DIR     PC_SD "/vocab"
#define PC_PHOTOS_DIR    PC_SD "/photos"
#define PC_CHIP_DIR      PC_SD "/chiptune"
#define PC_FORTUNE_DIR   PC_SD "/fortune"
#define PC_KITS_DIR      PC_SD "/instrument/kits"
#define PC_FM_DIR        PC_SD "/instrument/fm"
#define PC_INST_DIR      PC_SD "/instrument"

/* the card as the PC sees it while the USB drive is on (the same folders) */
#define PC_USB           "/usbcard"
#define PC_USB_CFG_KEYS  PC_USB "/" PC_INT_NAME "/config/keys.txt"

#include <sys/stat.h>
/* mkdir of a folder inside PC_INT (PC_INT itself first; both are fine when they already exist) */
static inline int pc_int_mkdir(const char *dir) { mkdir(PC_INT, 0777); return mkdir(dir, 0777); }
