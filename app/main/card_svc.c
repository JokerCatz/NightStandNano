// SPDX-License-Identifier: LGPL-2.1-or-later
/* Card diagnostics service, see card_svc.h. Also the lower half of card_test.h (card_hw_*) for Nano's SDMMC card. */
#include "card_svc.h"
#include <stdio.h>
#include <string.h>
#include "cardfmt.h"
#include "card_test.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "storage.h"

static SemaphoreHandle_t s_mx;          /* card_test_* are not thread safe: start / step / cancel / report all run under this */
static SemaphoreHandle_t s_verdict;     /* the worker's answer to a start request */
static TaskHandle_t s_worker;
static volatile int s_req = -1;         /* kind to start, -1 none (written by the command task, taken by the worker) */
static bool s_start_ok;
static uint8_t s_start_fail;

/* free space and file system type of the mounted volume: f_getfree can take seconds on a big FAT32 card, so only the worker computes it, once per mount;
 * everyone else reads the cache ("?" until known) */
static uint32_t s_vol_gen;              /* storage gen the cache belongs to (0 = none) */
static uint64_t s_free;
static const char *s_fs = "?";

static void volume_refresh(void)
{
    uint32_t gen = nsn_storage_gen();
    if (!nsn_storage_mounted() || s_vol_gen == gen) return;
    uint64_t fr; const char *fs;
    if (nsn_storage_volume(&fr, &fs)) { s_free = fr; s_fs = fs; s_vol_gen = gen; }
}

/* ---- card_hw_*, the lower half of card_test.c ---- */
bool card_hw_info(ct_info_t *i)
{
    nsn_card_t c;
    nsn_storage_card(&c);
    if (c.state != NSN_CARD_OK) return false;
    memset(i, 0, sizeof *i);
    i->capacity = c.bytes;
    memcpy(i->name, c.name, sizeof i->name);
    i->serial = c.serial;
    volume_refresh();                                                   // only ever called from the worker
    if (s_vol_gen == nsn_storage_gen()) { i->free_bytes = s_free; i->fs = s_fs; }
    return true;
}
int card_hw_read(uint32_t sector, uint32_t n, void *buf) { return nsn_storage_read_raw(sector, n, buf); }
int64_t card_hw_now_us(void) { return esp_timer_get_time(); }
void card_hw_account(uint32_t bytes, bool write) { (void)bytes; (void)write; }

static void lock(void) { xSemaphoreTake(s_mx, portMAX_DELAY); }
static void unlock(void) { xSemaphoreGive(s_mx); }

static void worker(void *arg)
{
    uint32_t loaded_gen = 0;
    for (;;) {                                                          /* persistent: a task made WithCaps must not delete itself */
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
        if (s_req >= 0) {
            int kind = s_req;
            s_req = -1;
            lock();
            s_start_ok = card_test_start(kind);
            s_start_fail = card_test_result(kind)->fail;
            unlock();
            xSemaphoreGive(s_verdict);
            while (card_test_running()) {                               // nano: round's console loop, here on the worker
                lock(); card_test_step(); unlock();
                vTaskDelay(1);
            }
            s_vol_gen = 0;                                              /* the file test changed the free space */
        }
        if (nsn_storage_mounted()) {                                    /* once per mount: free space, and the saved results of this card */
            volume_refresh();
            if (loaded_gen != nsn_storage_gen()) { lock(); card_test_load(); unlock(); loaded_gen = nsn_storage_gen(); }
        }
    }
}

static bool start_worker(void)
{
    if (s_worker) return true;
    s_mx = xSemaphoreCreateMutex();
    s_verdict = xSemaphoreCreateBinary();
    xTaskCreatePinnedToCoreWithCaps(worker, "cardsvc", 6144, NULL, 3, &s_worker, 0, MALLOC_CAP_SPIRAM);
    return s_worker != NULL;
}

void nsn_card_svc_start(void) { start_worker(); }

static const char *fail_word(int f)
{
    static const char *const W[] = { "none", "nocard", "small", "nospace", "io", "nomem" };
    return f >= 0 && f < 6 ? W[f] : "?";
}

const char *nsn_cardtest_start(const char *kind)
{
    int k = !strcmp(kind, "raw") ? CT_RAW : !strcmp(kind, "file") ? CT_FILE : -1;
    if (k < 0) return "unknown test";
    if (!start_worker()) return "no memory";
    nsn_card_t c;
    nsn_storage_card(&c);
    if (c.state == NSN_CARD_BAD) return "card unreadable";
    if (c.state != NSN_CARD_OK) return "no card";
    if (nsn_cardfmt_state() == NSN_FMT_RUNNING) return "formatting";
    if (card_test_running() || s_req >= 0) return "already running";
    s_req = k;
    xTaskNotifyGive(s_worker);
    if (xSemaphoreTake(s_verdict, pdMS_TO_TICKS(15000)) != pdTRUE) return "timeout";   /* the worker stops playback first (up to 8 s) */
    return s_start_ok ? NULL : fail_word(s_start_fail);
}

bool nsn_cardtest_cancel(void)
{
    if (!s_mx || !card_test_running()) return false;
    lock();
    card_test_cancel();
    unlock();
    return true;
}

bool nsn_cardtest_running(void) { return s_mx && card_test_running(); }

static void x100(char *b, size_t n, uint32_t v) { snprintf(b, n, "%u.%02u", (unsigned)(v / 100), (unsigned)(v % 100)); }

void nsn_cardtest_report(nsn_out_t out, void *ctx)
{
    char b[160], a[16], c[16], d[16];
    if (s_mx) lock();
    int kind = card_test_kind();
    if (kind >= 0) snprintf(b, sizeof b, "cardtest state running %s %d%%", kind == CT_RAW ? "raw" : "file", card_test_result(kind)->pct);
    else snprintf(b, sizeof b, "cardtest state idle");
    out(ctx, b);
    for (int k = 0; k < CT_KINDS; k++) {
        const ct_result_t *r = card_test_result(k);
        if (r->state == CTS_FAIL) { snprintf(b, sizeof b, "cardtest fail %s %s", k == CT_RAW ? "raw" : "file", fail_word(r->fail)); out(ctx, b); }
    }
    const ct_result_t *r = card_test_result(CT_RAW);
    if (r->state == CTS_DONE) {
        x100(a, sizeof a, r->avg16_x100); x100(c, sizeof c, r->slow16_x100); x100(d, sizeof d, r->max64_x100);
        snprintf(b, sizeof b, "cardtest raw avg16=%s slow16=%s max64=%s worst_us=%lu time=%s", a, c, d, (unsigned long)r->worst_us, r->when[0] ? r->when : "-");
        out(ctx, b);
    }
    r = card_test_result(CT_FILE);
    if (r->state == CTS_DONE) {
        x100(a, sizeof a, r->w_x100); x100(c, sizeof c, r->r_x100);
        snprintf(b, sizeof b, "cardtest file write=%s read=%s verify=%d time=%s", a, c, r->verify_ok ? 1 : 0, r->when[0] ? r->when : "-");
        out(ctx, b);
    }
    uint32_t raw, file;
    card_test_speeds(&raw, &file);
    bool from_file;
    uint32_t follow = pc_ct_follow(raw, file, &from_file);
    if (follow) { x100(a, sizeof a, follow); snprintf(b, sizeof b, "cardtest basis %s speed=%s", from_file ? "file" : "raw", a); out(ctx, b); }
    pc_ct_rung_t rung[24];
    int n = pc_ct_ladder(raw, file, rung, 24);
    for (int i = 0; i < n; i++) {
        x100(a, sizeof a, rung[i].need_x100);
        if (rung[i].fmt < 0) {                                          /* a marker: where the measured speed sits in the ladder (speed 0.00 = not tested) */
            snprintf(b, sizeof b, "cardtest mark %s speed=%s", rung[i].fmt == PC_CT_RUNG_RAW ? "raw" : "file", a);
        } else {
            const char *g = !follow ? "unknown" : rung[i].grade == PC_CT_YES ? "ok" : rung[i].grade == PC_CT_MARGINAL ? "marginal" : "slow";
            snprintf(b, sizeof b, "cardtest fmt \"%s\" need=%s grade=%s", pc_ct_format(rung[i].fmt)->label, a, g);
        }
        out(ctx, b);
    }
    if (s_mx) unlock();
}

void nsn_card_line(char *out, size_t n)
{
    nsn_card_t c;
    nsn_storage_card(&c);
    char name[8];
    memcpy(name, c.name, sizeof name);
    for (int i = 0; name[i]; i++) if ((unsigned char)name[i] < 32 || (unsigned char)name[i] > 126 || name[i] == '"') name[i] = '?';
    const char *fmt = nsn_cardfmt_state_word();
    if (c.state == NSN_CARD_NONE) { snprintf(out, n, "card state=none fmt=%s", fmt); return; }
    char fr[24] = "?";
    const char *fs = "?";
    if (c.state == NSN_CARD_OK && s_vol_gen == nsn_storage_gen()) { snprintf(fr, sizeof fr, "%llu", (unsigned long long)s_free); fs = s_fs; }
    snprintf(out, n, "card state=%s size=%llu free=%s fs=%s name=\"%s\" serial=%08lX fmt=%s", c.state == NSN_CARD_OK ? "mounted" : "unreadable",
             (unsigned long long)c.bytes, fr, fs, name, (unsigned long)c.serial, fmt);
}
