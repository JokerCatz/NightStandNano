// SPDX-License-Identifier: LGPL-2.1-or-later
/* 記憶卡診斷: the service, see card_test.h. The state is PSRAM (EXT_RAM_BSS_ATTR): no internal RAM. */
#include "card_test.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "pc/pc_io.h"
#include "pc/pc_paths.h"
#include <time.h>
#include "clock.h"
#include "player.h"                              // nano: replaces clock_svc.h / player_ctl.h

#define TMP_PATH PC_CARDTEST_TMP
#define SAVE_PATH PC_CARDTEST_TXT
#define TMP_DIR  PC_INT
#define FILE_BYTES (4u << 20)
#define FILE_FREE_MIN (8ull << 20)
#define SECT 512u
#define CH16 (16u * 1024)
#define CH64 (64u * 1024)
#define PASS_BYTES (2u << 20)                 /* a raw pass reads this much ... */
#define PASS_US 1500000u                      /* ... or reads for this long, whichever ends first */
#define SLICE_US 30000u
#define MIN_SECTORS (131072u)                 /* 64 MB: the three regions of 6 MB need room */
#define REGION_ALIGN (8192u)                  /* 4 MB boundary in sectors */

typedef struct {
    int kind;                                 /* -1: none running */
    uint8_t *buf, *raw;                       /* raw: the malloc'd pointer; buf: 128-byte aligned (the SD host's DMA rule) */
    uint32_t chunk;
    /* RAW */
    uint32_t pass, sector, end_sector, pass_us, pass_bytes;   /* pass 0..8 (pc_cardtest.h): region = pass / 3; warm 16 KB, measured 16 KB (same sectors), 64 KB */
    uint64_t a_bytes, a_us, b_bytes, b_us;
    uint32_t sectors;
    /* FILE */
    int phase;                                /* 0 write, 1 read back */
    FILE *f;
    pc_reader_t rd;
    uint32_t off;
    uint64_t w_us, r_us;
} run_t;

EXT_RAM_BSS_ATTR static run_t s_run;
EXT_RAM_BSS_ATTR static ct_result_t s_res[CT_KINDS];
EXT_RAM_BSS_ATTR static bool s_inited;

static void init_once(void) { if (!s_inited) { s_inited = true; s_run.kind = -1; } }

/* ---- the result file ---- */
static bool read_saved(char *text, size_t cap)
{
    FILE *f = fopen(SAVE_PATH, "rb");
    if (!f) return false;
    size_t n = fread(text, 1, cap - 1, f);
    fclose(f);
    text[n] = 0;
    return true;
}

static void save_result(int kind)
{
    ct_result_t *r = &s_res[kind];
    ct_info_t info;
    if (!card_hw_info(&info)) { r->save_failed = true; return; }
    pc_ct_rec_t rec;
    memset(&rec, 0, sizeof rec);
    rec.serial = info.serial;
    rec.size = info.capacity;
    strcpy(rec.when, "-");
    if (nsn_clock_valid()) {                                    // nano: clock_time_valid() -> the clock is plausible (SNTP has run)
        time_t t = time(NULL);
        struct tm tm;
        if (localtime_r(&t, &tm)) strftime(rec.when, sizeof rec.when, "%Y-%m-%dT%H:%M", &tm);
    }
    snprintf(r->when, sizeof r->when, "%s", rec.when);                // nano: the fresh result carries its time too (the console report prints it)
    if (kind == CT_RAW) { rec.v[0] = r->avg16_x100; rec.v[1] = r->slow16_x100; rec.v[2] = r->max64_x100; rec.v[3] = r->worst_us; for (int i = 0; i < CT_REGIONS; i++) rec.v[4 + i] = r->reg16_x100[i]; }
    else { rec.v[0] = r->w_x100; rec.v[1] = r->r_x100; rec.v[2] = r->verify_ok; }
    char old[600], out[600];
    if (!read_saved(old, sizeof old)) old[0] = 0;
    size_t n = pc_ct_file_update(old, kind, &rec, out, sizeof out);
    mkdir(TMP_DIR, 0777);
    FILE *f = n ? fopen(SAVE_PATH, "wb") : NULL;
    bool ok = f && fwrite(out, 1, n, f) == n;
    if (f && fclose(f)) ok = false;
    r->save_failed = !ok;
    if (!ok) printf("APP cardtest save failed\n");
}

void card_test_load(void)
{
    init_once();
    ct_info_t info;
    char text[600];
    if (card_test_running() || !card_hw_info(&info) || !read_saved(text, sizeof text)) return;
    for (int k = 0; k < CT_KINDS; k++) {
        ct_result_t *r = &s_res[k];
        pc_ct_rec_t rec;
        if (r->state != CTS_NONE || !pc_ct_file_find(text, k, info.serial, info.capacity, &rec)) continue;
        r->state = CTS_DONE;
        r->saved = true;
        snprintf(r->when, sizeof r->when, "%s", rec.when);
        if (k == CT_RAW) { r->avg16_x100 = rec.v[0]; r->slow16_x100 = rec.v[1]; r->max64_x100 = rec.v[2]; r->worst_us = rec.v[3]; for (int i = 0; i < CT_REGIONS; i++) r->reg16_x100[i] = rec.v[4 + i]; }
        else { r->w_x100 = rec.v[0]; r->r_x100 = rec.v[1]; r->verify_ok = rec.v[2] != 0; }
    }
}

static void fail(int kind, int why) { s_res[kind].state = CTS_FAIL; s_res[kind].fail = (uint8_t)why; }

static void release(void)
{
    if (s_run.f) { fclose(s_run.f); s_run.f = NULL; }
    free(s_run.raw);
    s_run.raw = s_run.buf = NULL;
    s_run.kind = -1;
}

static bool alloc_buf(uint32_t n)
{
    s_run.raw = heap_caps_malloc(n + 128, MALLOC_CAP_SPIRAM);        /* PSRAM, aligned by hand (the decoders' read-ahead buffers are 128-aligned PSRAM) */
    if (!s_run.raw) return false;
    s_run.buf = (uint8_t *)(((uintptr_t)s_run.raw + 127) & ~(uintptr_t)127);
    return true;
}

bool card_test_running(void) { return s_inited && s_run.kind >= 0; }
int card_test_kind(void) { return card_test_running() ? s_run.kind : -1; }
const ct_result_t *card_test_result(int kind) { return kind >= 0 && kind < CT_KINDS ? &s_res[kind] : NULL; }


static void raw_pass_begin(void)
{
    const uint32_t region = (uint32_t)pc_ct_pass_region(s_run.pass);
    const int role = pc_ct_pass_role(s_run.pass);
    const bool odd = role == PC_CT_PASS_MEAS64;
    uint32_t base;
    if (region == 0) base = 0;
    else if (region == 1) base = (s_run.sectors / 2) / REGION_ALIGN * REGION_ALIGN;
    else base = (s_run.sectors - 4 * REGION_ALIGN) / REGION_ALIGN * REGION_ALIGN;     /* 16 MB from the end, 4 MB aligned */
    s_run.sector = pc_ct_pass_sector(base, role, REGION_ALIGN);                        /* the warm and the measured 16 KB pass read the same sectors; the 64 KB pass: the 4 MB after, so no card cache helps */
    s_run.end_sector = s_run.sector + PASS_BYTES / SECT;
    s_run.chunk = odd ? CH64 : CH16;
    s_run.pass_us = 0;
    s_run.pass_bytes = 0;
    if (role == PC_CT_PASS_MEAS16) s_res[CT_RAW].reg_sector[region] = s_run.sector;
}

static void stop_playback(void)
{
    nsn_player_stop();                                                // nano: player_ctl_now()->stop; the card is shared: nothing plays during a test
}

bool card_test_start(int kind)
{
    init_once();
    if (kind < 0 || kind >= CT_KINDS) return false;
    if (card_test_running()) { return false; }
    ct_result_t *r = &s_res[kind];
    memset(r, 0, sizeof *r);
    ct_info_t info;
    if (!card_hw_info(&info)) { fail(kind, CTF_NOCARD); return false; }
    stop_playback();
    s_run.kind = kind;
    r->state = CTS_RUN;
    if (kind == CT_RAW) {
        s_run.sectors = (uint32_t)(info.capacity / SECT);
        if (info.capacity / SECT < MIN_SECTORS) { release(); fail(kind, CTF_SMALL); return false; }
        if (!alloc_buf(CH64)) { release(); fail(kind, CTF_NOMEM); return false; }
        s_run.pass = 0;
        s_run.a_bytes = s_run.a_us = s_run.b_bytes = s_run.b_us = 0;
        raw_pass_begin();
        return true;
    }
    if (info.free_bytes < FILE_FREE_MIN) { release(); fail(kind, CTF_NOSPACE); return false; }
    if (!alloc_buf(CH16)) { release(); fail(kind, CTF_NOMEM); return false; }
    remove(TMP_PATH);                                                 /* a leftover of a test that was cut off */
    mkdir(TMP_DIR, 0777);
    s_run.f = fopen(TMP_PATH, "wb");
    if (s_run.f) setvbuf(s_run.f, NULL, _IONBF, 0);
    else { release(); fail(kind, CTF_IO); return false; }
    s_run.phase = 0;
    s_run.off = 0;
    s_run.w_us = s_run.r_us = 0;
    return true;
}

void card_test_cancel(void)
{
    if (!card_test_running()) return;
    int k = s_run.kind;
    release();
    if (k == CT_FILE) remove(TMP_PATH);
    memset(&s_res[k], 0, sizeof s_res[k]);                           /* the half result is dropped; nothing stays that was not finished */
}

static void raw_finish(void)
{
    ct_result_t *r = &s_res[CT_RAW];
    r->avg16_x100 = pc_ct_x100(s_run.a_bytes, s_run.a_us);
    r->max64_x100 = pc_ct_x100(s_run.b_bytes, s_run.b_us);
    r->slow16_x100 = pc_ct_slowest(r->reg16_x100, CT_REGIONS);
    r->bytes = s_run.a_bytes + s_run.b_bytes;
    r->state = CTS_DONE;
    release();
    save_result(CT_RAW);
    card_test_print(CT_RAW);
}

static void raw_step(void)
{
    ct_result_t *r = &s_res[CT_RAW];
    const int64_t t0 = card_hw_now_us();
    do {
        const uint32_t n = s_run.chunk / SECT;
        const int64_t a = card_hw_now_us();
        int rc = card_hw_read(s_run.sector, n, s_run.buf);
        const uint32_t us = (uint32_t)(card_hw_now_us() - a);
        if (rc) { release(); fail(CT_RAW, CTF_IO); return; }
        s_run.sector += n;
        s_run.pass_bytes += s_run.chunk;
        s_run.pass_us += us;
        const int role = pc_ct_pass_role(s_run.pass);
        if (role == PC_CT_PASS_MEAS16 && us > r->worst_us) r->worst_us = us;
        if (s_run.pass_bytes >= PASS_BYTES || s_run.pass_us >= PASS_US) {
            const uint32_t region = (uint32_t)pc_ct_pass_region(s_run.pass);
            if (role == PC_CT_PASS_MEAS64) { s_run.b_bytes += s_run.pass_bytes; s_run.b_us += s_run.pass_us; }
            else if (role == PC_CT_PASS_WARM16) r->warm16_x100[region] = pc_ct_x100(s_run.pass_bytes, s_run.pass_us);   /* only reported: it wakes the card */
            else { s_run.a_bytes += s_run.pass_bytes; s_run.a_us += s_run.pass_us; r->reg16_x100[region] = pc_ct_x100(s_run.pass_bytes, s_run.pass_us); }
            if (++s_run.pass >= PC_CT_PASSES_PER_REGION * CT_REGIONS) { raw_finish(); return; }
            raw_pass_begin();
        }
        r->pct = (uint8_t)(s_run.pass * 100 / (PC_CT_PASSES_PER_REGION * CT_REGIONS));
    } while ((uint64_t)(card_hw_now_us() - t0) < SLICE_US);
}

static void file_abort(int why)
{
    release();
    remove(TMP_PATH);
    fail(CT_FILE, why);
}

static void file_step(void)
{
    ct_result_t *r = &s_res[CT_FILE];
    const int64_t t0 = card_hw_now_us();
    do {
        if (s_run.phase == 0) {
            pc_ct_fill(s_run.buf, CH16, s_run.off);                  /* not timed */
            const int64_t a = card_hw_now_us();
            size_t w = fwrite(s_run.buf, 1, CH16, s_run.f);
            card_hw_account(CH16, true);
            s_run.w_us += (uint64_t)(card_hw_now_us() - a);
            if (w != CH16) { file_abort(CTF_IO); return; }
            s_run.off += CH16;
            if (s_run.off >= FILE_BYTES) {
                const int64_t b = card_hw_now_us();                  /* the close is part of the write: the data is on the card when it returns */
                fflush(s_run.f);
                fsync(fileno(s_run.f));
                int rc = fclose(s_run.f);
                s_run.f = NULL;
                s_run.w_us += (uint64_t)(card_hw_now_us() - b);
                FILE *f = rc ? NULL : fopen(TMP_PATH, "rb");
                if (f) setvbuf(f, NULL, _IONBF, 0);                  /* as the decoders open it */
                if (!f || pc_reader_file(&s_run.rd, f)) { if (f) fclose(f); file_abort(CTF_IO); return; }
                s_run.f = f;
                r->w_x100 = pc_ct_x100(FILE_BYTES, s_run.w_us);
                s_run.phase = 1;
                r->verify_ok = true;                                 /* cleared at the first mismatch */
                s_run.off = 0;
            }
        } else {
            const int64_t a = card_hw_now_us();
            int got = s_run.rd.read(s_run.rd.u, s_run.off, s_run.buf, CH16);
            card_hw_account(CH16, false);
            s_run.r_us += (uint64_t)(card_hw_now_us() - a);
            if (got != (int)CH16) { file_abort(CTF_IO); return; }
            if (!pc_ct_check(s_run.buf, CH16, s_run.off, NULL)) { r->verify_ok = false; s_run.off = FILE_BYTES; }
            else s_run.off += CH16;
            if (s_run.off >= FILE_BYTES) {
                bool ok = r->verify_ok;
                fclose(s_run.f);
                s_run.f = NULL;
                remove(TMP_PATH);
                r->r_x100 = pc_ct_x100(FILE_BYTES, s_run.r_us);
                r->bytes = FILE_BYTES;
                r->verify_ok = ok;
                r->state = CTS_DONE;
                release();
                save_result(CT_FILE);
                card_test_print(CT_FILE);
                return;
            }
        }
        r->pct = (uint8_t)((s_run.phase * FILE_BYTES + s_run.off) * 50ull / FILE_BYTES);
    } while ((uint64_t)(card_hw_now_us() - t0) < SLICE_US);
}

void card_test_step(void)
{
    if (!card_test_running()) return;
    if (s_run.kind == CT_RAW) raw_step(); else file_step();
    if (s_res[0].state == CTS_FAIL && !card_test_running()) printf("APP cardtest raw fail=%d\n", s_res[CT_RAW].fail);
    if (s_res[1].state == CTS_FAIL && !card_test_running()) printf("APP cardtest file fail=%d\n", s_res[CT_FILE].fail);
}

void card_test_speeds(uint32_t *raw, uint32_t *file)
{
    init_once();
    *raw = s_res[CT_RAW].state == CTS_DONE ? s_res[CT_RAW].slow16_x100 : 0;
    *file = s_res[CT_FILE].state == CTS_DONE ? s_res[CT_FILE].r_x100 : 0;
}

static void line_x100(char *b, size_t n, uint32_t v) { snprintf(b, n, "%u.%02u", (unsigned)(v / 100), (unsigned)(v % 100)); }

void card_test_print(int kind)
{
    const ct_result_t *r = &s_res[kind];
    char a[16], b[16], c[16];
    if (r->state != CTS_DONE) { printf("APP cardtest %s state=%d fail=%d\n", kind == CT_RAW ? "raw" : "file", r->state, r->fail); return; }
    if (kind == CT_RAW) {
        for (int i = 0; i < CT_REGIONS; i++) { char w[16]; line_x100(w, sizeof w, r->warm16_x100[i]); line_x100(a, sizeof a, r->reg16_x100[i]);
            printf("APP cardtest raw region=%d sector=%lu mbps16=%s warm=%s method=%s\n", i, (unsigned long)r->reg_sector[i], a, w, PC_CT_RAW_METHOD); }
        line_x100(a, sizeof a, r->avg16_x100); line_x100(b, sizeof b, r->slow16_x100); line_x100(c, sizeof c, r->max64_x100);
        printf("APP cardtest raw avg16=%s slowest_region=%s max64=%s slowest_read_ms=%lu.%lu%s\n", a, b, c, (unsigned long)(r->worst_us / 1000), (unsigned long)(r->worst_us % 1000 / 100), r->saved ? " saved=1" : "");
    } else {
        line_x100(a, sizeof a, r->w_x100); line_x100(b, sizeof b, r->r_x100);
        printf("APP cardtest file write=%s read=%s verify=%s bytes=%lu%s\n", a, b, r->verify_ok ? "ok" : "FAIL", (unsigned long)r->bytes, r->saved ? " saved=1" : "");
    }
    uint32_t raw, fl;
    card_test_speeds(&raw, &fl);
    bool from_file;
    uint32_t v = pc_ct_follow(raw, fl, &from_file);
    line_x100(a, sizeof a, v);
    printf("APP cardtest basis=%s speed=%s\n", from_file ? "file" : "raw", a);
    for (int i = 0; i < pc_ct_format_count(); i++) {
        const pc_ct_fmt_t *f = pc_ct_format(i);
        pc_ct_grade_t g = pc_ct_grade(v, f);
        line_x100(b, sizeof b, pc_ct_need_x100(f));
        printf("APP cardtest verdict %s=%s need=%s\n", f->label, g == PC_CT_YES ? "yes" : g == PC_CT_MARGINAL ? "marginal" : "no", b);
    }
}
