// SPDX-License-Identifier: LGPL-2.1-or-later
/* exFAT proof without touching a card: format a PSRAM RAM disk as exFAT, mount it through the VFS, write and read back a file. */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_vfs_fat.h"
#include "diskio_impl.h"
#include "ff.h"

#include "cardfmt.h"

static const char *TAG = "exfat";
#define RD_SECTORS NSN_RAMDISK_SECTORS      /* 8 MiB of 512 B sectors */
static uint8_t *s_ram;

static DSTATUS rd_init(BYTE pdrv) { return 0; }
static DSTATUS rd_status(BYTE pdrv) { return 0; }
static DRESULT rd_read(BYTE pdrv, BYTE *buf, LBA_t sec, UINT n) { if (sec + n > RD_SECTORS) return RES_PARERR; memcpy(buf, s_ram + sec * 512, n * 512); return RES_OK; }
static DRESULT rd_write(BYTE pdrv, const BYTE *buf, LBA_t sec, UINT n) { if (sec + n > RD_SECTORS) return RES_PARERR; memcpy(s_ram + sec * 512, buf, n * 512); return RES_OK; }
static DRESULT rd_ioctl(BYTE pdrv, BYTE cmd, void *buf)
{
    switch (cmd) {
    case CTRL_SYNC: return RES_OK;
    case GET_SECTOR_COUNT: *(LBA_t *)buf = RD_SECTORS; return RES_OK;
    case GET_SECTOR_SIZE: *(WORD *)buf = 512; return RES_OK;
    case GET_BLOCK_SIZE: *(DWORD *)buf = 1; return RES_OK;
    }
    return RES_PARERR;
}

/* The RAM disk as a FatFs drive, shared with the card format's tests (cardfmt.c): a fresh zeroed disk each time, one at a time. */
bool nsn_ramdisk_open(BYTE *pdrv)
{
    if (s_ram) return false;
    s_ram = heap_caps_calloc(1, RD_SECTORS * 512, MALLOC_CAP_SPIRAM);
    if (!s_ram) return false;
    static const ff_diskio_impl_t impl = { .init = rd_init, .status = rd_status, .read = rd_read, .write = rd_write, .ioctl = rd_ioctl };
    if (ff_diskio_get_drive(pdrv) != ESP_OK) { heap_caps_free(s_ram); s_ram = NULL; return false; }
    ff_diskio_register(*pdrv, &impl);
    return true;
}

void nsn_ramdisk_close(BYTE pdrv)
{
    ff_diskio_unregister(pdrv);
    heap_caps_free(s_ram);
    s_ram = NULL;
}

bool nsn_exfat_selftest(void)
{
    bool ok = false;
    void *work = heap_caps_malloc(FF_MAX_SS * 4, MALLOC_CAP_INTERNAL);
    BYTE pdrv;
    if (!work || !nsn_ramdisk_open(&pdrv)) { ESP_LOGE(TAG, "work=%p ramdisk", work); goto out; }
    char drv[3] = { '0' + pdrv, ':', 0 };
    FATFS *fs = NULL;
    const esp_vfs_fat_conf_t vc = { .base_path = "/ramx", .fat_drive = drv, .max_files = 2 };
    if (esp_vfs_fat_register(&vc, &fs) != ESP_OK) { ESP_LOGE(TAG, "vfs register"); goto close; }
    MKFS_PARM opt = { .fmt = FM_EXFAT, .n_fat = 1, .align = 0, .n_root = 0, .au_size = 0 };
    FRESULT r = f_mkfs(drv, &opt, work, FF_MAX_SS * 4);
    if (r == FR_OK) r = f_mount(fs, drv, 1);
    if (r != FR_OK) { ESP_LOGE(TAG, "mkfs/mount exFAT failed: %d", r); goto unreg; }
    FATFS *pf; DWORD nclst;
    f_getfree(drv, &nclst, &pf);
    ESP_LOGI(TAG, "fs_type=%d (4 = exFAT) free clusters=%lu", fs->fs_type, (unsigned long)nclst);
    FILE *f = fopen("/ramx/\xe6\xb8\xac\xe8\xa9\xa6-long-file-name-over-eight-three.txt", "w");
    if (f) { fputs("hello exfat", f); fclose(f); }
    char buf[32] = { 0 };
    f = fopen("/ramx/\xe6\xb8\xac\xe8\xa9\xa6-long-file-name-over-eight-three.txt", "r");
    if (f) { fgets(buf, sizeof buf, f); fclose(f); }
    ok = fs->fs_type == FS_EXFAT && !strcmp(buf, "hello exfat");
    ESP_LOGI(TAG, "exFAT selftest %s (read back '%s')", ok ? "PASS" : "FAIL", buf);
    f_mount(NULL, drv, 0);
unreg:
    esp_vfs_fat_unregister_path("/ramx");
close:
    nsn_ramdisk_close(pdrv);
out:
    free(work);
    return ok;
}
