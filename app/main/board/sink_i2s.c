// SPDX-License-Identifier: LGPL-2.1-or-later
/* TEMPLATE for an I2S output (DAC / codec board). Compiled only with CONFIG_NSN_SINK_I2S (Kconfig, default n); with the option off none of this exists.
 * It is NOT implemented and NOT tested on any hardware: every place that needs a decision of yours is marked TODO(board). As delivered it registers an
 * output named "i2s" that always reports "not present", so nothing ever selects it and the box behaves exactly as without it.
 *
 * What it does when you fill it in: the player pulls its PCM like it does for the USB DAC (sink.h). One task of ours pulls 16-bit stereo frames from the
 * player's callback and writes them to the I2S TX channel; the channel's blocking write paces the loop (the DMA buffers are the only clock).
 *
 * Selection: `out i2s` chooses it explicitly (when present); `out auto` takes it after the USB DAC and before Bluetooth (auto_rank 20 against the DAC's 10
 * and Bluetooth's 30 in player.c). Volume is software, in the player, per output: this sink only receives finished samples; its own volume is stored under "vol_i2s".
 * Checklist: docs/porting.md "新增 I2S DAC". */
#include "sdkconfig.h"
#if CONFIG_NSN_SINK_I2S

#include <string.h>
#include "driver/i2s_std.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board.h"
#include "sink.h"

static const char *TAG = "i2s";

/* TODO(board): the I2S pins. GPIO_NUM_NC (-1) = not configured = the sink stays "not present". Choose pins that are not in the reserved table
 * (board_default.c), not the TF card / LED pins, and (for BCLK / WS / DOUT) any GPIO: I2S goes through the GPIO matrix. MCLK only if the codec needs it. */
#define I2S_PIN_MCLK GPIO_NUM_NC
#define I2S_PIN_BCLK GPIO_NUM_NC
#define I2S_PIN_WS   GPIO_NUM_NC
#define I2S_PIN_DOUT GPIO_NUM_NC

#define CHUNK_FRAMES 256                  /* frames per pull / write; 1 KB of S16 stereo, in PSRAM */

static i2s_chan_handle_t s_tx;
static TaskHandle_t s_task;
static volatile bool s_run, s_busy;       /* s_run: the task should pull; s_busy: it is inside the loop (stop() waits for it to leave) */
static nsn_sink_pull_t s_pull;
static void *s_pull_ctx;
static uint32_t s_rate;

static bool pins_configured(void) { return I2S_PIN_BCLK != GPIO_NUM_NC && I2S_PIN_WS != GPIO_NUM_NC && I2S_PIN_DOUT != GPIO_NUM_NC; }

/* "Is it there?" Cheap, any task. TODO(board): a codec on I2C may be absent or unplugged: probe it here (board_codec_init once, cache the answer), or return
 * true for a DAC that is always soldered on. Unconfigured pins mean not present, which is what keeps this template harmless. */
static bool i2s_present(void) { return pins_configured(); }

/* Which rates the I2S DAC takes. The player resamples the source to what this returns (it does so only when the rates differ).
 * TODO(board): the DAC's datasheet. A plain PCM5102-class DAC takes anything up to 384 kHz from BCLK; most codecs take the usual 44.1 / 48 / 96 kHz
 * families. Return src when it is acceptable, else the nearest rate that is. Default: everything is run at 48 kHz (safe, costs one resampler). */
static uint32_t i2s_link_rate(uint32_t src) { return 48000; }

static void i2s_task(void *arg)           /* persistent: a task created WithCaps never deletes itself (it parks on the notification between sessions) */
{
    int16_t *buf = heap_caps_malloc(CHUNK_FRAMES * 4, MALLOC_CAP_SPIRAM);
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        s_busy = true;
        while (s_run && buf) {
            size_t got = s_pull(buf, CHUNK_FRAMES, s_pull_ctx);
            if (got < CHUNK_FRAMES) memset(buf + got * 2, 0, (CHUNK_FRAMES - got) * 4);      /* the sink pads with silence, never leaves stale samples */
            size_t written = 0;
            if (i2s_channel_write(s_tx, buf, CHUNK_FRAMES * 4, &written, 1000) != ESP_OK) break;
        }
        s_busy = false;
    }
}

static esp_err_t i2s_start(uint32_t rate, nsn_sink_pull_t pull, void *ctx)
{
    if (!i2s_present()) return ESP_ERR_NOT_SUPPORTED;
    esp_err_t e = board_codec_init(rate);                      /* I2C part; ESP_ERR_NOT_SUPPORTED = a DAC without registers */
    if (e != ESP_OK && e != ESP_ERR_NOT_SUPPORTED) return e;
    i2s_chan_config_t cc = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    e = i2s_new_channel(&cc, &s_tx, NULL);
    if (e != ESP_OK) return e;
    i2s_std_config_t sc = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate),           /* TODO(board): mclk_multiple (256 default) if the codec wants another ratio */
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),   /* TODO(board): MSB / PCM format for DACs that are not Philips */
        .gpio_cfg = { .mclk = I2S_PIN_MCLK, .bclk = I2S_PIN_BCLK, .ws = I2S_PIN_WS, .dout = I2S_PIN_DOUT, .din = I2S_GPIO_UNUSED },
    };
    e = i2s_channel_init_std_mode(s_tx, &sc);
    if (e == ESP_OK) e = i2s_channel_enable(s_tx);
    if (e != ESP_OK) { i2s_del_channel(s_tx); s_tx = NULL; return e; }
    if (!s_task && xTaskCreatePinnedToCoreWithCaps(i2s_task, "i2s", 3072, NULL, 5, &s_task, 1, MALLOC_CAP_SPIRAM) != pdPASS) {
        i2s_channel_disable(s_tx); i2s_del_channel(s_tx); s_tx = NULL;
        return ESP_ERR_NO_MEM;
    }
    s_pull = pull; s_pull_ctx = ctx; s_rate = rate; s_run = true;
    board_codec_mute(false);
    xTaskNotifyGive(s_task);
    ESP_LOGI(TAG, "started at %lu Hz", (unsigned long)rate);
    return ESP_OK;
}

static void i2s_stop(void)
{
    if (!s_tx) return;
    board_codec_mute(true);
    s_run = false;
    for (int i = 0; i < 100 && s_busy; i++) vTaskDelay(pdMS_TO_TICKS(10));       /* let the task leave the loop before its channel goes */
    i2s_channel_disable(s_tx);
    i2s_del_channel(s_tx);
    s_tx = NULL;
}

static const nsn_sink_t k_sink_i2s = {
    .name = "i2s", .label = "I2S", .desc = "I2S output", .vol_key = "vol_i2s", .auto_rank = 20, .gone_msg = "I2S output gone",
    .present = i2s_present, .link_rate = i2s_link_rate, .start = i2s_start, .stop = i2s_stop,
};

void nsn_sink_i2s_register(void) { nsn_sink_register(&k_sink_i2s); }

#endif /* CONFIG_NSN_SINK_I2S */
