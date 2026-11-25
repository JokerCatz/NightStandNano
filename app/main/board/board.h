// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* The board: everything that belongs to one piece of hardware and not to the firmware. board_default.c is the stock ESP32-S31 dev board
 * (TF card slot on the SDMMC pins, one WS2812 on GPIO60, USB-C DAC dongle). A different board changes that file (or replaces it) and nothing else,
 * unless it adds one of the extension points below. docs/porting.md (zh-TW) is the guide; every hook is called from one place and says when.
 *
 * Part 1, data (implemented): default pins, the LED, the reserved-pin table. The `hw` command (hw.c) validates user settings against it.
 * Part 2, hooks (EMPTY on the stock board, not implemented, not tested on any hardware): display, GPIO buttons, I2S output, I2C codec.
 *
 * Rules for every hook:
 *   - internal RAM is the scarce resource (Bluetooth and Wi-Fi live there): buffers, frame buffers, rings and task stacks go to PSRAM
 *     (heap_caps_malloc(.., MALLOC_CAP_SPIRAM), xTaskCreate*WithCaps(.. MALLOC_CAP_SPIRAM)); a task created WithCaps never deletes itself.
 *   - a task that writes NVS (nsn_settings_set_*, anything that can write flash) needs an internal stack; a PSRAM-stack task must not.
 *   - hooks run once, in app_main's task (8 KB stack): start tasks, do not loop; return quickly (the boot waits for them).
 *   - the box must stay usable when a hook fails: log and return an error, never abort. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "led_strip_types.h"
#include "hw.h"

/* ---- part 1: data ---- */

void board_default_sd(nsn_sdpins_t *p);                  /* the stock TF card wiring (SDMMC slot 0) */
void board_default_led(nsn_hw_led_t *l);                 /* the stock LED: GPIO60, wire order grb */
led_model_t board_led_model(void);                       /* WS2812 here; SK6812 / WS2811 timings differ (led_strip_types.h). One pixel is driven (index 0). */

typedef struct { int8_t gpio; const char *why; } board_reserved_t;
const board_reserved_t *board_reserved_table(size_t *n); /* GPIOs no setting may use, with the reason and its source (the comment in board_default.c) */
int  board_gpio_max(void);                               /* highest GPIO number of the chip (61) */
bool board_gpio_exists(int gpio);                        /* bonded out on this chip (SOC_GPIO_VALID_GPIO_MASK) */
bool board_gpio_reserved(int gpio);                      /* in the table, or does not exist */

/* ---- part 2: hooks ---- */

/* Display. Called once from app_main after storage and the player are up (card mounted or not), after the BLE control service is started and before
 * the main loop. Default: nothing. What a display may read, all read-only and callable from any task:
 *   nsn_player_info()   player.h   what plays: title, artist, position, volume, rate, sleep timer, output
 *   nsn_spec_last()     spectrum.h the 32 spectrum bands, latest frame; nsn_spec_start() takes ONE callback and main does not use it, a second
 *                                  consumer registers with nsn_spec_observe() (up to 2 observers, called in the spectrum task at ~30 Hz)
 *   nsn_led_*           led.h      the LED is separate; a display does not need it
 * A frame buffer lives in PSRAM; LVGL was left out on purpose (its working memory and its tick task are not free). */
esp_err_t board_display_init(void);

/* GPIO buttons. Same place in the boot order. Default: nothing. Start a task that debounces the pins and calls nsn_keys_event(NSN_KEY_CENTER / _PLUS /
 * _MINUS, down) (keys.h): the gesture and action map applies (`keys` command). The example is in board_default.c. */
esp_err_t board_buttons_init(void);

/* I2S output (board/sink_i2s.c, compiled with CONFIG_NSN_SINK_I2S): board_sinks_register() is called from nsn_player_init_settings() at boot and adds the
 * board's outputs to the table of sink.h. board_codec_init() is the I2C part of an I2S DAC: called by the I2S sink before it opens the I2S port. */
void      board_sinks_register(void);
esp_err_t board_codec_init(uint32_t rate);               /* I2C bring-up of the codec at this sample rate; ESP_ERR_NOT_SUPPORTED = no codec (plain I2S DAC) */
void      board_codec_mute(bool mute);                   /* mute / unmute the codec around start / stop; may be empty */
