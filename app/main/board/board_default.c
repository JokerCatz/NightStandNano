// SPDX-License-Identifier: LGPL-2.1-or-later
/* The stock board: ESP32-S31 dev board with a TF card slot (SDMMC slot 0), one WS2812 on GPIO60 and the USB-C port used as a USB host for the DAC.
 * To port the firmware to another board, copy this file, change it, and name it in main/CMakeLists.txt (docs/porting.md). */
#include "board.h"
#include "esp_log.h"
#include "soc/soc_caps.h"

/* ---- defaults ---- */

void board_default_sd(nsn_sdpins_t *p)
{
    /* Kconfig NSN_SD_*: the SDMMC slot 0 pins of the chip (esp_hal_sd/esp32s31/include/soc/sdmmc_pins.h: CLK 24, CMD 25, D0 20, D1 21, D2 22, D3 23) */
    *p = (nsn_sdpins_t){ .clk = CONFIG_NSN_SD_CLK, .cmd = CONFIG_NSN_SD_CMD, .d0 = CONFIG_NSN_SD_D0, .d1 = CONFIG_NSN_SD_D1, .d2 = CONFIG_NSN_SD_D2,
                         .d3 = CONFIG_NSN_SD_D3, .width = CONFIG_NSN_SD_WIDTH, .mode = NSN_SD_SDMMC, .mosi = -1, .miso = -1, .cs = -1 };
}

void board_default_led(nsn_hw_led_t *l)
{
    *l = (nsn_hw_led_t){ .gpio = 60, .order = "grb" };       /* the dev board's WS2812; a board without one: gpio -1 */
}

led_model_t board_led_model(void) { return LED_MODEL_WS2812; }

/* ---- the GPIOs no setting may use ----
 * Every entry names its source in the IDF tree (~/.espressif/v6.1/esp-idf/components/...). Where the headers do not settle a pin, it is reserved
 * (a conservative choice, marked "conservative"): a wrong setting that stops the boot cannot be undone with the `hw` command, only by erasing the NVS
 * over the ROM bootloader. The numbers are the same table as in docs/porting.md.
 *   - GPIOs that do not exist (29, 41): SOC_GPIO_VALID_GPIO_MASK in soc/esp32s31/include/soc/soc_caps.h, added by board_gpio_reserved() itself.
 *   - there is no input-only pin: SOC_GPIO_VALID_OUTPUT_GPIO_MASK == SOC_GPIO_VALID_GPIO_MASK (same file), so nothing to add for "output needed".
 *   - the octal PSRAM has dedicated pads without a GPIO number (soc/esp32s31/register/soc/iomux_mspi_pin_reg.h: IOMUX_MSPI_PIN_PSRAM_*_PIN0_REG,
 *     not in io_mux_reg.h); GPIO9..19 carry only an optional debug function "DBG_PSRAM_*" (io_mux_reg.h, function 4). So they are NOT reserved.
 *   - GPIO0 / GPIO1 (XTAL_32K pads) stay free: CONFIG_RTC_CLK_SRC_INT_RC=y here, no 32 kHz crystal is used by the firmware (a board that fits one must reserve them).
 *   - GPIO54..57 are the pad JTAG (MTDO, MTCK, MTDI, MTMS; io_mux_reg.h): free, using them as GPIO only loses pad-JTAG debugging. */
static const board_reserved_t k_reserved[] = {
    { 26, "spi flash CS0 (esp_hal_gpspi/esp32s31/include/soc/spi_pins.h MSPI_IOMUX_PIN_NUM_CS0; io_mux_reg.h FUNC_SPICS_FLASH_CS_PAD)" },
    { 27, "spi flash Q / MISO (MSPI_IOMUX_PIN_NUM_MISO; FUNC_SPIQ_FLASH_Q_PAD)" },
    { 28, "spi flash WP (MSPI_IOMUX_PIN_NUM_WP; FUNC_SPIWP_FLASH_WP_PAD)" },
    { 30, "spi flash HOLD (MSPI_IOMUX_PIN_NUM_HD; FUNC_SPIHD_FLASH_HOLD_PAD)" },
    { 31, "spi flash CLK (MSPI_IOMUX_PIN_NUM_CLK; FUNC_SPICLK_FLASH_CK_PAD)" },
    { 32, "spi flash D / MOSI (MSPI_IOMUX_PIN_NUM_MOSI; FUNC_SPID_FLASH_D_PAD)" },
    { 33, "USB D- of the internal PHY0 = the USB host port of the DAC (io_mux_reg.h USB_INT_PHY0_DM_GPIO_NUM)" },
    { 34, "USB D+ of the internal PHY0 (io_mux_reg.h USB_INT_PHY0_DP_GPIO_NUM); USB-Serial-JTAG has no pads of its own in the headers (CONFIG_SOC_USB_FSLS_PHY_NUM=0)" },
    { 36, "strapping: LDO select (io_mux_reg.h comment above PERIPHS_IO_MUX_U_GPIO36)" },
    { 37, "strapping: USB-JTAG / pad-JTAG select (io_mux_reg.h, GPIO37)" },
    { 38, "strapping: boot mode 0 (io_mux_reg.h, GPIO38; soc/boot_mode.h)" },
    { 39, "strapping: boot mode 1 (io_mux_reg.h, GPIO39)" },
    { 40, "strapping: boot mode 2 (io_mux_reg.h, GPIO40)" },
    { 58, "console UART0 TX (soc/uart_pins.h U0TXD_GPIO_NUM; CONFIG_ESP_CONSOLE_UART_NUM=0): the wired console and the flashing tool" },
    { 59, "console UART0 RX (soc/uart_pins.h U0RXD_GPIO_NUM)" },
    { 60, "strapping: boot mode 3 (io_mux_reg.h, GPIO60). The stock LED sits here and boots (proven on the dev board), so board_default_led() may use it; conservative for anything else" },
    { 61, "strapping: boot mode 4 (io_mux_reg.h, GPIO61), conservative" },
};

const board_reserved_t *board_reserved_table(size_t *n) { *n = sizeof k_reserved / sizeof k_reserved[0]; return k_reserved; }

int board_gpio_max(void) { return SOC_GPIO_IN_RANGE_MAX; }

bool board_gpio_exists(int gpio) { return gpio >= 0 && gpio <= SOC_GPIO_IN_RANGE_MAX && ((SOC_GPIO_VALID_GPIO_MASK >> gpio) & 1ULL); }

bool board_gpio_reserved(int gpio)
{
    if (!board_gpio_exists(gpio)) return true;
    for (size_t i = 0; i < sizeof k_reserved / sizeof k_reserved[0]; i++) if (k_reserved[i].gpio == gpio) return true;
    return false;
}

/* ---- hooks: nothing here is implemented. Each says what to do and where it is called; docs/porting.md has the checklists. ---- */

/* Display: called once from app_main (after storage, the player and the BLE control service, before the main loop).
 * Start your own task (PSRAM stack and frame buffer, see board.h) and read nsn_player_info() / nsn_spec_last() / nsn_spec_observe(). Never write the
 * player's state from here; a display that needs commands goes through nsn_cmd_exec() (cmd.h), as the BLE side does. */
esp_err_t board_display_init(void)
{
    /* TODO(board): bus init (SPI / I2C / parallel), panel reset, backlight, then xTaskCreatePinnedToCoreWithCaps(display_task, ..., MALLOC_CAP_SPIRAM) */
    return ESP_OK;
}

/* GPIO buttons: called at the same point. Example of what to put here (pins are the board's business, not the `hw` command's):
 *
 *   static const struct { gpio_num_t pin; int key; } k_btn[] = { { GPIO_NUM_5, NSN_KEY_CENTER }, { GPIO_NUM_6, NSN_KEY_PLUS }, { GPIO_NUM_7, NSN_KEY_MINUS } };
 *   static void btn_task(void *arg)                                       // xTaskCreate(btn_task, "btn", 3072, NULL, 4, NULL): internal stack is fine,
 *   {                                                                     // and keys.c's own task does the NVS work, not this one
 *       bool last[3] = { 0 };
 *       for (;;) {
 *           for (int i = 0; i < 3; i++) {
 *               bool down = gpio_get_level(k_btn[i].pin) == 0;            // active low with the internal pull-up
 *               if (down != last[i]) { last[i] = down; nsn_keys_event(k_btn[i].key, down); }   // press and release, any task; keys.c does the gestures
 *           }
 *           vTaskDelay(pdMS_TO_TICKS(10));                                // 10 ms poll = debounce enough for tact switches
 *       }
 *   }
 *   gpio_config() the pins as inputs with pull-up first; pick pins that are neither reserved above nor used by the TF card / LED.
 * The key numbers (keys.h) are the headset's three buttons; more buttons need NSN_KEY_N and the nvs key table in keys.c extended, and the stored map of
 * the first three stays valid. */
esp_err_t board_buttons_init(void)
{
    /* TODO(board): see the example above */
    return ESP_OK;
}

/* Extra outputs: called from nsn_player_init_settings() once at boot, after the three built-in sinks (ids 1..3), so a board sink gets id 4.
 * With the option off the I2S template is not even compiled. */
#if CONFIG_NSN_SINK_I2S
void nsn_sink_i2s_register(void);       /* board/sink_i2s.c */
void board_sinks_register(void) { nsn_sink_i2s_register(); }

/* I2C bring-up of an I2S codec: called by the I2S sink before it opens the I2S port and again when the sample rate changes.
 * ESP_ERR_NOT_SUPPORTED = no codec to set up (a plain I2S DAC such as a PCM5102). Any other error keeps the sink "not present". */
esp_err_t board_codec_init(uint32_t rate)
{
    /* TODO(board): i2c_new_master_bus() (esp_driver_i2c) with this board's SDA / SCL, add the codec's address, write its registers for `rate` (PLL / MCLK
     * ratio, 16-bit I2S, unmute off); free the bus handle again when done unless something else shares it. Needs internal RAM only for the driver's few
     * hundred bytes. Pins for I2C / I2S are the board's own: check them against the reserved table above. */
    return ESP_ERR_NOT_SUPPORTED;
}

/* mute / unmute around start and stop (a pop at the start of a stream is the usual reason for one) */
void board_codec_mute(bool mute)
{
    /* TODO(board): the codec's mute register, or an amplifier's shutdown pin */
}
#else
void board_sinks_register(void) { }
#endif
