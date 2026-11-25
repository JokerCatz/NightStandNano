// SPDX-License-Identifier: LGPL-2.1-or-later
/* Formatting the TF card: see cardfmt.h. Plan logic (pc_fmt_plan), the MBR and the f_mkfs parameters are JukeDial's (player_core pc_fmtguard.c,
 * app/main/sd_hw.c, the BSP's bsp_sdcard.c); what is Nano's: the two-code confirmation, the worker, and the drive being a parameter. */
#include "cardfmt.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "diskio.h"
#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "pc/pc_fmtguard.h"
#include "player.h"
#include "storage.h"

static const char *TAG = "cardfmt";
#define CONFIRM_US (30 * 1000000LL)
#define WORK_LEN (64 * 1024)               /* f_mkfs's work area: 128 sectors at a time (PSRAM) */
#define GPT_SECTORS 33
#define UNREADABLE_MSG "the card is readable; formatting is only for a card that cannot be read"

extern PARTITION VolToPart[];

/* ---- the core: format a FatFs drive ---------------------------------------------------------------------------------------------- */

/* the old table's traces first (a GPT: header + entries at LBA 1..33 and the backup at the end), then the new MBR at LBA 0. Through the drive's own
 * diskio, so the same code writes the SD card and the RAM disk. */
static esp_err_t write_layout(BYTE pdrv, const pc_fmt_plan_t *p, uint32_t disk_sig, uint8_t *work)
{
    memset(work, 0, GPT_SECTORS * 512);
    if (disk_write(pdrv, work, 1, GPT_SECTORS) != RES_OK) return ESP_FAIL;
    if (disk_write(pdrv, work, p->total_sectors - GPT_SECTORS, GPT_SECTORS) != RES_OK) return ESP_FAIL;
    pc_fmt_mbr(p, disk_sig, work);
    return disk_write(pdrv, work, 0, 1) == RES_OK ? ESP_OK : ESP_FAIL;
}

/* f_mkfs into partition 1 of the MBR just written (FF_MULTI_PARTITION: VolToPart says which partition a logical drive is; IDF maps drive i to physical
 * drive i, partition 0 = auto), then the volume label. The mapping is put back on every path. */
static esp_err_t make_fs(BYTE pdrv, const pc_fmt_plan_t *p, uint8_t *work, size_t len)
{
    if (VolToPart[pdrv].pd != pdrv) return ESP_ERR_INVALID_STATE;
    char drv[3] = { (char)('0' + pdrv), ':', 0 }, lab[24];
    const MKFS_PARM opt = { .fmt = p->fs == PC_FMT_EXFAT ? FM_EXFAT : p->fs == PC_FMT_FAT32 ? FM_FAT32 : FM_FAT, .n_fat = p->n_fat,
                            .align = p->align, .n_root = 0, .au_size = p->cluster_bytes };
    BYTE pt = VolToPart[pdrv].pt;
    VolToPart[pdrv].pt = 1;
    FRESULT r = f_mkfs(drv, &opt, work, (UINT)len);
    FRESULT rl = FR_OK;
    if (r == FR_OK) {
        FATFS *fs = heap_caps_calloc(1, sizeof *fs, MALLOC_CAP_SPIRAM);
        snprintf(lab, sizeof lab, "%s%s", drv, NSN_FMT_LABEL);
        rl = !fs ? FR_NOT_ENOUGH_CORE : f_mount(fs, drv, 1);
        if (rl == FR_OK) rl = f_setlabel(lab);
        if (fs) { f_mount(NULL, drv, 0); free(fs); }
    }
    VolToPart[pdrv].pt = pt;
    ESP_LOGI(TAG, "mkfs fr=%d label_fr=%d", (int)r, (int)rl);       /* a label that failed does not fail the format: the volume is there */
    return r == FR_OK ? ESP_OK : ESP_FAIL;
}

/* *stage (pc_fmt_stage_t) says how far it got: nothing is written before WRITE */
static esp_err_t format_drive(BYTE pdrv, const pc_fmt_plan_t *p, uint32_t disk_sig, uint8_t *work, size_t work_len, int *stage)
{
    *stage = PC_FMT_STAGE_BEFORE;
    if (!p || !work || work_len < 32 * 1024 || work_len % 512) return ESP_ERR_INVALID_ARG;
    if (VolToPart[pdrv].pd != pdrv) return ESP_ERR_NOT_SUPPORTED;        /* make_fs's drive mapping, checked before any write */
    if (disk_initialize(pdrv) & STA_NOINIT) return ESP_ERR_INVALID_STATE;
    *stage = PC_FMT_STAGE_WRITE;                                         /* from here on the drive may be changed */
    esp_err_t e = write_layout(pdrv, p, disk_sig, work);
    if (e == ESP_OK) { *stage = PC_FMT_STAGE_MKFS; e = make_fs(pdrv, p, work, work_len); }
    return e;
}

static void make_skeleton(const char *base)
{
    static const char *const dirs[] = { "/_player", "/_player/config", "/_player/logs", "/_player/scenes" };
    char path[48];
    for (int i = 0; i < 4; i++) { snprintf(path, sizeof path, "%s%s", base, dirs[i]); mkdir(path, 0777); }
}

/* ---- the RAM disk plans and the check of a formatted RAM disk (self-test and simulation) ------------------------------------------ */

static pc_fmt_plan_t ram_plan(bool exfat)
{
    pc_fmt_plan_t p = { .total_sectors = NSN_RAMDISK_SECTORS, .bu_sectors = 128, .part_start = 128, .part_sectors = NSN_RAMDISK_SECTORS - 128, .align = 128 };
    p.fs = exfat ? PC_FMT_EXFAT : PC_FMT_FAT16;       /* 8 MiB is too small for FAT32 (it needs 65525 clusters) */
    p.cluster_bytes = exfat ? 4096 : 512;
    p.n_fat = exfat ? 1 : 2;
    p.mbr_type = exfat ? 0x07 : 0x06;
    return p;
}

/* mount the formatted RAM disk at /ramx, check type and label, make the skeleton, write and read a file back, unmount. false: *why says what failed */
static bool check_ramdisk(BYTE pdrv, bool exfat, char *why, size_t n)
{
    char drv[3] = { (char)('0' + pdrv), ':', 0 };
    FATFS *fs = NULL;
    const esp_vfs_fat_conf_t vc = { .base_path = "/ramx", .fat_drive = drv, .max_files = 2 };
    if (esp_vfs_fat_register(&vc, &fs) != ESP_OK) { snprintf(why, n, "vfs register"); return false; }
    bool ok = false;
    FRESULT r = f_mount(fs, drv, 1);
    if (r != FR_OK) { snprintf(why, n, "mount fr=%d", (int)r); goto unreg; }
    const int want = exfat ? FS_EXFAT : FS_FAT16;
    char lab[24] = "";
    f_getlabel(drv, lab, NULL);
    if (fs->fs_type != want) { snprintf(why, n, "fs_type=%d want %d", fs->fs_type, want); goto unmount; }
    if (strcmp(lab, NSN_FMT_LABEL)) { snprintf(why, n, "label=\"%s\"", lab); goto unmount; }
    make_skeleton("/ramx");
    struct stat st;
    if (stat("/ramx/_player/config", &st) || stat("/ramx/_player/logs", &st) || stat("/ramx/_player/scenes", &st)) { snprintf(why, n, "skeleton missing"); goto unmount; }
    FILE *f = fopen("/ramx/_player/config/t.txt", "w");
    if (f) { fputs("hello format", f); fclose(f); }
    char buf[32] = "";
    f = fopen("/ramx/_player/config/t.txt", "r");
    if (f) { fgets(buf, sizeof buf, f); fclose(f); }
    if (strcmp(buf, "hello format")) { snprintf(why, n, "readback \"%s\"", buf); goto unmount; }
    snprintf(why, n, "fs=%s label=%s skeleton ok file ok", exfat ? "exFAT" : "FAT16", lab);
    ok = true;
unmount:
    f_mount(NULL, drv, 0);
unreg:
    esp_vfs_fat_unregister_path("/ramx");
    return ok;
}

/* format a fresh RAM disk with the plan and check it; the whole of what the real path does after the drive is chosen */
static bool ramdisk_roundtrip(bool exfat, uint8_t *work, char *why, size_t n)
{
    BYTE pdrv;
    if (!nsn_ramdisk_open(&pdrv)) { snprintf(why, n, "no RAM disk"); return false; }
    const pc_fmt_plan_t plan = ram_plan(exfat);
    int stage;
    esp_err_t e = format_drive(pdrv, &plan, esp_random(), work, WORK_LEN, &stage);
    bool ok = false;
    if (e != ESP_OK) snprintf(why, n, "format %s at stage %s", esp_err_to_name(e), pc_fmt_stage_name(stage));
    else ok = check_ramdisk(pdrv, exfat, why, n);
    nsn_ramdisk_close(pdrv);
    return ok;
}

/* ---- the confirmation state machine ----------------------------------------------------------------------------------------------- */

typedef enum { JOB_NONE, JOB_REAL, JOB_SIM, JOB_SELFTEST } job_t;

static struct {
    nsn_fmt_state_t st;
    int step;                       /* while PENDING: 1 = waiting for code 1, 2 = for code 2 */
    int code;
    int64_t deadline_us;
    bool sim, held;                 /* sim: UART-only simulation; held: storage_hold is ours */
    pc_fmt_plan_t plan;
    uint32_t serial;
    char reason[64];
    job_t job;
    nsn_out_t out; void *ctx; bool st_ok;      /* self-test: where the lines go and the result */
} s;
static SemaphoreHandle_t s_mx, s_done;
static TaskHandle_t s_worker;

static void lock(void) { if (!s_mx) s_mx = xSemaphoreCreateMutex(); xSemaphoreTake(s_mx, portMAX_DELAY); }
static void unlock(void) { xSemaphoreGive(s_mx); }

static void cancel_locked(void)
{
    if (s.st != NSN_FMT_PENDING) return;                       /* a running format is not cancelled (and keeps its hold) */
    s.st = NSN_FMT_IDLE;
    s.step = 0;
    if (s.held) { s.held = false; nsn_storage_hold(false); }
}

static void expire_locked(void)
{
    if (s.st == NSN_FMT_PENDING && esp_timer_get_time() > s.deadline_us) cancel_locked();
}

/* the precondition, checked at every step: NULL = a card that answers but has no readable file system (or the simulation) */
static const char *precond(uint64_t *bytes, uint32_t *serial)
{
    if (s.sim) { *bytes = (uint64_t)NSN_RAMDISK_SECTORS * 512; *serial = 0; return NULL; }
    nsn_card_t c;
    nsn_storage_card(&c);
    if (c.state == NSN_CARD_OK) return UNREADABLE_MSG;
    if (c.state != NSN_CARD_BAD || !c.bytes) return "no card";
    *bytes = c.bytes;
    *serial = c.serial;
    return NULL;
}

static int new_code(int not_this)
{
    int c;
    do c = (int)(esp_random() % 10000); while (c == not_this);
    return c;
}

const char *nsn_cardfmt_request(char *plan_line, size_t n, int *code)
{
    lock();
    expire_locked();
    const char *why = NULL;
    uint64_t bytes = 0; uint32_t serial = 0;
    if (s.st == NSN_FMT_RUNNING) why = "already running";
    else if (!(why = precond(&bytes, &serial))) {
        pc_fmt_plan_t plan;
        if (s.sim) plan = ram_plan(true);
        else if (!pc_fmt_plan(bytes, 512, &plan)) why = "card size not supported";
        if (!why) {
            if (!s.sim && !s.held) { nsn_storage_hold(true); s.held = true; }     /* main.c stops retrying the mount while a request is open */
            s.plan = plan;
            s.serial = serial;
            s.st = NSN_FMT_PENDING;
            s.step = 1;
            s.code = new_code(-1);
            s.deadline_us = esp_timer_get_time() + CONFIRM_US;
            *code = s.code;
            snprintf(plan_line, n, "fs=%s label=\"%s\" size=%llu", pc_fmt_fs_name(plan.fs), NSN_FMT_LABEL, (unsigned long long)bytes);
        }
    }
    if (why && s.st == NSN_FMT_PENDING) cancel_locked();         /* a refused re-request leaves nothing pending */
    unlock();
    return why;
}

static void worker(void *arg);

const char *nsn_cardfmt_confirm(int code, int *next)
{
    lock();
    expire_locked();
    const char *why = NULL;
    uint64_t bytes; uint32_t serial;
    if (s.st != NSN_FMT_PENDING) why = s.st == NSN_FMT_RUNNING ? "already running" : "no request pending (or it timed out); start again with cardfmt";
    else if (esp_timer_get_time() > s.deadline_us) why = "too late";
    else if ((why = precond(&bytes, &serial)) != NULL) { /* refused: the card changed */ }
    else if (code != s.code) why = "wrong code";
    else if (s.step == 1) {
        s.step = 2;
        s.code = new_code(s.code);
        s.deadline_us = esp_timer_get_time() + CONFIRM_US;
        *next = s.code;
    } else {
        if (!s_worker) {
            s_done = xSemaphoreCreateBinary();
            xTaskCreatePinnedToCoreWithCaps(worker, "cardfmt", 8192, NULL, 3, &s_worker, 0, MALLOC_CAP_SPIRAM);
        }
        if (!s_worker || s.job != JOB_NONE) why = "busy";
        else {
            s.st = NSN_FMT_RUNNING;
            s.step = 0;
            s.reason[0] = 0;
            s.job = s.sim ? JOB_SIM : JOB_REAL;
            *next = 0;
            xTaskNotifyGive(s_worker);
        }
    }
    if (why && s.st == NSN_FMT_PENDING) cancel_locked();          /* wrong code, late, changed card: the request is gone, start over */
    unlock();
    return why;
}

void nsn_cardfmt_cancel(void)
{
    lock();
    cancel_locked();
    unlock();
}

nsn_fmt_state_t nsn_cardfmt_state(void)
{
    lock();
    expire_locked();
    nsn_fmt_state_t st = s.st;
    unlock();
    return st;
}

const char *nsn_cardfmt_state_word(void)
{
    static const char *const N[] = { "idle", "pending", "running", "done", "failed" };
    return N[nsn_cardfmt_state()];
}

void nsn_cardfmt_status(char *out, size_t n)
{
    const char *w = nsn_cardfmt_state_word();
    lock();
    if (s.st == NSN_FMT_FAILED || (s.st == NSN_FMT_DONE && s.reason[0])) snprintf(out, n, "cardfmt state %s %s", w, s.reason);
    else snprintf(out, n, "cardfmt state %s", w);
    unlock();
}

int nsn_cardfmt_led(void)
{
    if (!s_mx) return 0;
    lock();
    expire_locked();
    int v = s.st == NSN_FMT_PENDING ? s.step : 0;
    unlock();
    return v;
}

void nsn_cardfmt_sim(bool on)
{
    lock();
    cancel_locked();
    s.sim = on;
    unlock();
}

bool nsn_cardfmt_sim_on(void) { return s.sim; }

/* ---- the worker -------------------------------------------------------------------------------------------------------------------- */

/* the real card: it does not mount (that is what allowed this), so take it raw, write the layout and the file system, mount it and make the skeleton */
static esp_err_t job_real(const pc_fmt_plan_t *plan, uint32_t serial, uint8_t *work, int *stage)
{
    *stage = PC_FMT_STAGE_BEFORE;
    nsn_player_stop();                                         /* playback and a radio connection hold no file on a card that is not mounted, but must not start one */
    nsn_storage_hold(false);                                   /* look once more, right now: a card that mounts stays as it is */
    if (nsn_storage_mount_now() == ESP_OK) return ESP_ERR_INVALID_STATE;
    nsn_storage_hold(true);
    sdmmc_card_t *c = NULL;
    esp_err_t e = nsn_storage_raw_open(&c);
    if (e != ESP_OK) return e;
    const pc_fmt_card_t shown = { plan->total_sectors, 512, serial }, now = { (uint64_t)c->csd.capacity, (uint32_t)c->csd.sector_size, (uint32_t)c->cid.serial };
    BYTE pdrv = 0xFF;
    if (!pc_fmt_same_card(&shown, &now)) e = ESP_ERR_INVALID_SIZE;          /* another card than the one the plan was made for */
    if (e == ESP_OK) e = ff_diskio_get_drive(&pdrv);
    if (e == ESP_OK) {
        ff_diskio_register_sdmmc(pdrv, c);
        e = format_drive(pdrv, plan, esp_random(), work, WORK_LEN, stage);
        ff_diskio_unregister(pdrv);
    }
    nsn_storage_raw_close(c);
    if (e == ESP_OK) {
        *stage = PC_FMT_STAGE_MOUNT;
        e = nsn_storage_mount_now();
        if (e == ESP_OK) make_skeleton("/sdcard");             /* config / logs / scenes; the Wi-Fi settings file is gone with the card's contents */
    }
    return e;
}

static void finish(esp_err_t e, int stage, bool sim)
{
    lock();
    if (s.held) { s.held = false; nsn_storage_hold(false); }
    if (e == ESP_OK) {
        s.st = NSN_FMT_DONE;
        snprintf(s.reason, sizeof s.reason, "%s", sim ? "sim" : "");
    } else {
        s.st = NSN_FMT_FAILED;
        snprintf(s.reason, sizeof s.reason, "%s %s", pc_fmt_outcome_name(pc_fmt_outcome(stage, false)), esp_err_to_name(e));
    }
    s.job = JOB_NONE;
    ESP_LOGI(TAG, "format %s (%s)", e == ESP_OK ? "done" : "failed", s.reason[0] ? s.reason : "-");
    unlock();
}

static void selftest_line(const char *fmt, const char *name, bool ok, const char *detail)
{
    char b[160];
    snprintf(b, sizeof b, fmt, name, ok ? "PASS" : "FAIL", detail);
    if (!ok) s.st_ok = false;
    s.out(s.ctx, b);
}

static void selftest(uint8_t *work)
{
    char why[96];
    s.st_ok = true;
    for (int i = 0; i < 2; i++) {
        bool exfat = i == 0;
        bool ok = ramdisk_roundtrip(exfat, work, why, sizeof why);
        selftest_line("cardfmt selftest %s %s %s", exfat ? "exFAT" : "FAT16", ok, why);
    }
    /* the plan the real card would get, from sizes on both sides of each rule */
    static const struct { uint64_t bytes; int fs; } P[] = { { 1ull << 30, PC_FMT_FAT16 }, { 16000000000ull, PC_FMT_FAT32 }, { 32ull << 30, PC_FMT_FAT32 },
                                                              { 64000000000ull, PC_FMT_EXFAT }, { 256000000000ull, PC_FMT_EXFAT } };
    bool ok = true;
    for (unsigned i = 0; i < sizeof P / sizeof P[0]; i++) { pc_fmt_plan_t p; ok = ok && pc_fmt_plan(P[i].bytes, 512, &p) && p.fs == P[i].fs; }
    selftest_line("cardfmt selftest %s %s %s", "plan", ok, "FAT16 <= 2 GiB, FAT32 <= 32 GiB, exFAT above");
}

static void worker(void *arg)
{
    for (;;) {                                                 /* persistent: a task made WithCaps must not delete itself */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        lock();
        job_t job = s.job;
        pc_fmt_plan_t plan = s.plan;
        uint32_t serial = s.serial;
        unlock();
        uint8_t *work = heap_caps_aligned_alloc(64, WORK_LEN, MALLOC_CAP_SPIRAM);
        int stage = PC_FMT_STAGE_BEFORE;
        esp_err_t e = work ? ESP_OK : ESP_ERR_NO_MEM;
        if (e == ESP_OK && job == JOB_REAL) e = job_real(&plan, serial, work, &stage);
        else if (e == ESP_OK && job == JOB_SIM) {
            /* bound to the RAM disk: this branch never touches the SD slot (no storage_* call, no sdmmc drive) */
            char why[96];
            if (!ramdisk_roundtrip(true, work, why, sizeof why)) { ESP_LOGE(TAG, "sim: %s", why); e = ESP_FAIL; }
            else ESP_LOGI(TAG, "sim: %s", why);
        } else if (e == ESP_OK && job == JOB_SELFTEST) {
            selftest(work);
            lock(); s.job = JOB_NONE; unlock();
            heap_caps_free(work);
            xSemaphoreGive(s_done);
            continue;
        }
        heap_caps_free(work);
        finish(e, stage, job == JOB_SIM);
    }
}

bool nsn_cardfmt_selftest(nsn_out_t out, void *ctx)
{
    lock();
    bool busy = s.job != JOB_NONE || s.st == NSN_FMT_RUNNING;
    if (!busy && !s_worker) {
        s_done = xSemaphoreCreateBinary();
        xTaskCreatePinnedToCoreWithCaps(worker, "cardfmt", 8192, NULL, 3, &s_worker, 0, MALLOC_CAP_SPIRAM);
    }
    if (!busy && s_worker) { s.job = JOB_SELFTEST; s.out = out; s.ctx = ctx; xTaskNotifyGive(s_worker); }
    else busy = true;
    unlock();
    if (busy) { out(ctx, "cardfmt selftest FAIL busy"); return false; }
    xSemaphoreTake(s_done, portMAX_DELAY);
    return s.st_ok;
}
