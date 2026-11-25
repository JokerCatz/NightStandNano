// SPDX-License-Identifier: LGPL-2.1-or-later
#include "storage.h"
#include "board/board.h"
#include <string.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "diskio.h"
#include "diskio_sdmmc.h"
#include "diskio_impl.h"
#include "ff.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "storage";
static const char *s_why = "";
static sdmmc_card_t *s_card;
static SemaphoreHandle_t s_lock;          /* mount / hold / raw open against each other (created by the first mount at boot) */
static volatile bool s_hold;              /* a format owns the card */
static uint32_t s_gen;                    /* counts successful mounts: caches keyed on it know when the volume is a new one */
static bool s_bad;                        /* the card answered but did not mount */
static nsn_card_t s_badinfo;              /* what the raw probe read from it */

/* The wiring is chosen once at boot by hw.c (settings "hw_sd") and handed over before the first mount: later mounts and raw opens come from worker tasks
 * with PSRAM stacks, which stay away from NVS. A changed setting applies after a reboot. */
static nsn_sdpins_t s_pins;
static bool s_pins_set;

void nsn_storage_set_pins(const nsn_sdpins_t *p) { s_pins = *p; s_pins_set = true; }
static void ensure_pins(void) { if (!s_pins_set) { board_default_sd(&s_pins); s_pins_set = true; } }      /* hw.c did not run: the board's wiring */
const nsn_sdpins_t *nsn_storage_pins(void) { ensure_pins(); return &s_pins; }

static void slot_config(sdmmc_host_t *host, sdmmc_slot_config_t *slot)
{
    const nsn_sdpins_t p = s_pins;
    *host = (sdmmc_host_t)SDMMC_HOST_DEFAULT();
    *slot = (sdmmc_slot_config_t)SDMMC_SLOT_CONFIG_DEFAULT();
    slot->clk = p.clk; slot->cmd = p.cmd; slot->d0 = p.d0; slot->d1 = p.d1; slot->d2 = p.d2; slot->d3 = p.d3;
    slot->width = p.width;
    slot->flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
}

/* SD in SPI mode (hw sd spi ...): a GPIO-matrix SPI bus of our own, since the SDMMC host reaches fixed pins only. 20 MHz is the SD-SPI default;
 * on flying wires lower it (docs/porting.md). The bus is up while the card is mounted or opened raw, and released when that fails or ends. */
#define SPI_HOST_ID SPI2_HOST
#define SPI_FREQ_KHZ 20000
static bool s_spi_bus;
static sdspi_dev_handle_t s_raw_spi;

static esp_err_t spi_bus_up(void)
{
    if (s_spi_bus) return ESP_OK;
    spi_bus_config_t bus = { .mosi_io_num = s_pins.mosi, .miso_io_num = s_pins.miso, .sclk_io_num = s_pins.clk, .quadwp_io_num = -1, .quadhd_io_num = -1,
                             .max_transfer_sz = 4096 };
    esp_err_t e = spi_bus_initialize(SPI_HOST_ID, &bus, SPI_DMA_CH_AUTO);
    s_spi_bus = e == ESP_OK;
    return e;
}
static void spi_bus_down(void) { if (s_spi_bus) { spi_bus_free(SPI_HOST_ID); s_spi_bus = false; } }

static void spi_host_config(sdmmc_host_t *host, sdspi_device_config_t *dev)
{
    *host = (sdmmc_host_t)SDSPI_HOST_DEFAULT();
    host->slot = SPI_HOST_ID;
    host->max_freq_khz = SPI_FREQ_KHZ;
    *dev = (sdspi_device_config_t)SDSPI_DEVICE_CONFIG_DEFAULT();
    dev->host_id = SPI_HOST_ID;
    dev->gpio_cs = s_pins.cs;
}

/* Raw access to a card that is not mounted (same steps as JukeDial's bsp_sdcard_open_raw). The caller holds s_lock or the hold. */
static esp_err_t raw_open_spi(sdmmc_card_t *c)
{
    sdmmc_host_t host; sdspi_device_config_t dev;
    spi_host_config(&host, &dev);
    esp_err_t e = spi_bus_up();
    if (e != ESP_OK) return e;
    e = sdspi_host_init();
    if (e == ESP_OK) e = sdspi_host_init_device(&dev, &s_raw_spi);
    if (e == ESP_OK) { host.slot = s_raw_spi; e = sdmmc_card_init(&host, c); }
    if (e != ESP_OK) { if (s_raw_spi) sdspi_host_remove_device(s_raw_spi); s_raw_spi = 0; sdspi_host_deinit(); spi_bus_down(); }
    return e;
}

static esp_err_t raw_open_unlocked(sdmmc_card_t **out)
{
    if (s_card) return ESP_ERR_INVALID_STATE;
    ensure_pins();
    sdmmc_card_t *c = calloc(1, sizeof *c);
    if (!c) return ESP_ERR_NO_MEM;
    if (s_pins.mode == NSN_SD_SPI) {
        esp_err_t e = raw_open_spi(c);
        if (e != ESP_OK) { free(c); return e; }
        *out = c;
        return ESP_OK;
    }
    sdmmc_host_t host; sdmmc_slot_config_t slot;
    slot_config(&host, &slot);
    esp_err_t e = host.init();
    bool inited = e == ESP_OK;                 /* ours to release again if a later step fails */
    if (e == ESP_OK || e == ESP_ERR_INVALID_STATE) e = sdmmc_host_init_slot(host.slot, &slot);
    if (e == ESP_OK) e = sdmmc_card_init(&host, c);
    if (e != ESP_OK) { if (inited) sdmmc_host_deinit(); free(c); return e; }
    *out = c;
    return ESP_OK;
}

void nsn_storage_raw_close(sdmmc_card_t *c)
{
    if (!c) return;
    free(c);
    if (s_pins.mode == NSN_SD_SPI) {
        if (s_raw_spi) sdspi_host_remove_device(s_raw_spi);
        s_raw_spi = 0; sdspi_host_deinit(); spi_bus_down();
    } else sdmmc_host_deinit();
}

esp_err_t nsn_storage_raw_open(sdmmc_card_t **out)
{
    if (s_lock) xSemaphoreTake(s_lock, portMAX_DELAY);
    esp_err_t e = raw_open_unlocked(out);
    if (s_lock) xSemaphoreGive(s_lock);
    return e;
}

/* A failed mount released everything, so look at the card once more: size and serial tell "card present, no file system" from "no card" */
static void probe_bad_card(void)
{
    memset(&s_badinfo, 0, sizeof s_badinfo);      /* looked at again on every failed retry: a swapped card must not keep the old card's numbers */
    sdmmc_card_t *c = NULL;
    if (raw_open_unlocked(&c) != ESP_OK) return;
    s_badinfo.bytes = (uint64_t)c->csd.capacity * c->csd.sector_size;
    s_badinfo.serial = (uint32_t)c->cid.serial;
    memcpy(s_badinfo.name, c->cid.name, sizeof s_badinfo.name - 1);
    nsn_storage_raw_close(c);
}

static esp_err_t mount_locked(bool respect_hold)
{
    if (!s_lock) s_lock = xSemaphoreCreateMutex();
    if (s_card) return ESP_OK;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (respect_hold && s_hold) { xSemaphoreGive(s_lock); return ESP_ERR_INVALID_STATE; }
    ensure_pins();
    const nsn_sdpins_t p = s_pins;
    esp_vfs_fat_sdmmc_mount_config_t mc = { .format_if_mount_failed = false, .max_files = 6, .allocation_unit_size = 16 * 1024 };
    esp_err_t e;
    if (p.mode == NSN_SD_SPI) {
        sdmmc_host_t host; sdspi_device_config_t dev;
        spi_host_config(&host, &dev);
        e = spi_bus_up();
        if (e == ESP_OK) e = esp_vfs_fat_sdspi_mount("/sdcard", &host, &dev, &mc, &s_card);
        if (e != ESP_OK) spi_bus_down();
    } else {
        sdmmc_host_t host; sdmmc_slot_config_t slot;
        slot_config(&host, &slot);
        e = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot, &mc, &s_card);
    }
    if (e != ESP_OK) {
        s_card = NULL;
        s_why = e == ESP_ERR_TIMEOUT ? "nocard" : e == ESP_FAIL ? "fs" : "err";
        s_bad = e == ESP_FAIL;                    /* the card answered, f_mount did not: a card with no usable file system */
        if (s_bad) probe_bad_card(); else memset(&s_badinfo, 0, sizeof s_badinfo);
        static bool told;
        if (!told) {
            told = true;
            if (p.mode == NSN_SD_SPI) ESP_LOGE(TAG, "mount failed (%s): %s; spi pins clk%d mosi%d miso%d cs%d (retrying quietly)", s_why, esp_err_to_name(e), p.clk, p.mosi, p.miso, p.cs);
            else ESP_LOGE(TAG, "mount failed (%s): %s; pins clk%d cmd%d d0-3 %d/%d/%d/%d width%d (retrying quietly)", s_why, esp_err_to_name(e), p.clk, p.cmd, p.d0, p.d1, p.d2, p.d3, p.width);
        }
        xSemaphoreGive(s_lock);
        return e;
    }
    s_why = "";
    s_bad = false;
    s_gen++;
    memset(&s_badinfo, 0, sizeof s_badinfo);
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t nsn_storage_mount(void) { return mount_locked(true); }          /* main.c's retry: stands back while a format owns the card */
esp_err_t nsn_storage_mount_now(void) { return mount_locked(false); }     /* the format itself */
uint32_t nsn_storage_gen(void) { return s_gen; }

const char *nsn_storage_why(void) { return s_why; }
bool nsn_storage_mounted(void) { return s_card != NULL; }

void nsn_storage_hold(bool on)
{
    if (!s_lock) s_lock = xSemaphoreCreateMutex();
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_hold = on;
    xSemaphoreGive(s_lock);
}

nsn_card_state_t nsn_storage_state(void) { return s_card ? NSN_CARD_OK : s_bad ? NSN_CARD_BAD : NSN_CARD_NONE; }

void nsn_storage_card(nsn_card_t *c)
{
    memset(c, 0, sizeof *c);
    sdmmc_card_t *k = s_card;
    if (k) {
        c->state = NSN_CARD_OK;
        c->bytes = (uint64_t)k->csd.capacity * k->csd.sector_size;
        c->serial = (uint32_t)k->cid.serial;
        memcpy(c->name, k->cid.name, sizeof c->name - 1);
    } else if (s_bad) {
        *c = s_badinfo;
        c->state = NSN_CARD_BAD;
    }
}

bool nsn_storage_volume(uint64_t *free_bytes, const char **fs_name)
{
    sdmmc_card_t *k = s_card;
    if (!k) return false;
    char drv[3] = { (char)('0' + ff_diskio_get_pdrv_card(k)), ':', 0 };
    FATFS *fs = NULL; DWORD n = 0;
    if (f_getfree(drv, &n, &fs) != FR_OK || !fs) return false;
    *free_bytes = (uint64_t)n * fs->csize * 512u;
    *fs_name = fs->fs_type == FS_EXFAT ? "exFAT" : fs->fs_type == FS_FAT32 ? "FAT32" : fs->fs_type == FS_FAT16 ? "FAT16" : "FAT12";
    return true;
}

int nsn_storage_read_raw(uint32_t sector, uint32_t n, void *buf)
{
    sdmmc_card_t *k = s_card;
    if (!k || !n || !buf) return -1;
    return disk_read(ff_diskio_get_pdrv_card(k), buf, sector, n) == RES_OK ? 0 : -1;     /* FatFs' own block layer, below the files; read only */
}
