// SPDX-License-Identifier: LGPL-2.1-or-later
/* The card format guard and plan (pc_fmtguard.h). */
#include "pc/pc_fmtguard.h"
#include <stdio.h>
#include <string.h>

const uint8_t PC_FG_KONAMI[PC_FG_CODE_LEN] = { PC_FG_UP, PC_FG_UP, PC_FG_DOWN, PC_FG_DOWN, PC_FG_LEFT, PC_FG_RIGHT, PC_FG_LEFT, PC_FG_RIGHT, PC_FG_B, PC_FG_A };

bool pc_fmt_entry_allowed(const char *reason, bool mounted, bool usb_mode)
{
    if (mounted || usb_mode || !reason) return false;
    return !strcmp(reason, "gpt") || !strcmp(reason, "nofs");
}

bool pc_fmtguard_begin(pc_fmtguard_t *g, const char *reason, bool mounted, bool usb_mode)
{
    bool ok = pc_fmt_entry_allowed(reason, mounted, usb_mode);
    g->step = ok ? PC_FG_WARN : PC_FG_OFF;
    g->got = 0;
    g->last_ms = 0;
    return ok;
}

bool pc_fmtguard_continue(pc_fmtguard_t *g, uint32_t now_ms)
{
    if (g->step != PC_FG_WARN) return false;
    g->step = PC_FG_CODE;
    g->got = 0;
    g->last_ms = now_ms;
    return true;
}

static bool timed_out(const pc_fmtguard_t *g, uint32_t now_ms) { return (uint32_t)(now_ms - g->last_ms) >= PC_FG_TIMEOUT_MS; }   /* wraps with the clock */

static void to_warn(pc_fmtguard_t *g) { g->step = PC_FG_WARN; g->got = 0; }

int pc_fmtguard_key(pc_fmtguard_t *g, int key, uint32_t now_ms)
{
    if (g->step != PC_FG_CODE) return PC_FG_KEY_IGNORED;
    if (timed_out(g, now_ms)) { to_warn(g); return PC_FG_KEY_TIMEOUT; }
    g->last_ms = now_ms;
    if (key < 0 || key >= PC_FG_NKEYS || g->got >= PC_FG_CODE_LEN || key != PC_FG_KONAMI[g->got]) { g->got = 0; return PC_FG_KEY_WRONG; }
    if (++g->got < PC_FG_CODE_LEN) return PC_FG_KEY_OK;
    g->step = PC_FG_ARM;
    return PC_FG_KEY_DONE;
}

bool pc_fmtguard_tick(pc_fmtguard_t *g, uint32_t now_ms)
{
    if (g->step != PC_FG_CODE || !timed_out(g, now_ms)) return false;
    to_warn(g);
    return true;
}

bool pc_fmtguard_armed(pc_fmtguard_t *g)
{
    if (g->step != PC_FG_ARM) return false;
    g->step = PC_FG_GO;
    return true;
}

bool pc_fmtguard_take(pc_fmtguard_t *g)
{
    if (g->step != PC_FG_GO) return false;
    g->step = PC_FG_OFF;
    g->got = 0;
    return true;
}

void pc_fmtguard_restart(pc_fmtguard_t *g) { if (g->step >= PC_FG_CODE && g->step <= PC_FG_GO) to_warn(g); }
void pc_fmtguard_back(pc_fmtguard_t *g) { g->step = PC_FG_OFF; g->got = 0; }

const char *pc_fmtguard_step_name(int step)
{
    static const char *const N[] = { "off", "warn", "code", "arm", "go" };
    return step >= 0 && step <= PC_FG_GO ? N[step] : "?";
}

/* ---- the plan ---- */
#define MIB (1024ull * 1024)
#define GIB (1024ull * MIB)

bool pc_fmt_plan(uint64_t capacity_bytes, uint32_t sector_size, pc_fmt_plan_t *out)
{
    memset(out, 0, sizeof *out);
    if (sector_size != 512 || capacity_bytes < 32 * MIB) return false;
    uint64_t total = capacity_bytes / 512;
    if (total > 0xFFFFFFFFull) return false;
    uint32_t spc, bu;   /* sectors per cluster, boundary unit (SD Association, SD Physical Layer Part 2 File System: SDSC / SDHC / SDXC tables) */
    uint8_t fs;
    if (capacity_bytes <= 2 * GIB) {                       /* SDSC: FAT12/16 */
        fs = PC_FMT_FAT16;
        if (capacity_bytes <= 64 * MIB) { spc = 32; bu = 32; }
        else if (capacity_bytes <= 256 * MIB) { spc = 32; bu = 64; }
        else if (capacity_bytes <= 1 * GIB) { spc = 32; bu = 128; }
        else { spc = 64; bu = 128; }
        while (spc < 128 && (total - bu) / spc > 65000) spc *= 2;   /* FAT16 holds at most 65525 clusters: one size up at the edge of a row */
    } else if (capacity_bytes <= 32 * GIB) {               /* SDHC: FAT32, 32 KB clusters, 4 MiB boundary */
        fs = PC_FMT_FAT32;
        spc = 64; bu = 8192;
        while (spc > 8 && (total - bu) / spc < 66000) spc /= 2;     /* FAT32 needs more than 65525 clusters: one size down just above 2 GiB */
    } else {                                               /* SDXC: exFAT */
        fs = PC_FMT_EXFAT;
        if (capacity_bytes <= 128 * GIB) { spc = 256; bu = 32768; }
        else if (capacity_bytes <= 512 * GIB) { spc = 512; bu = 65536; }
        else { spc = 1024; bu = 131072; }
    }
    out->total_sectors = (uint32_t)total;
    out->bu_sectors = bu;
    out->part_start = bu;
    out->part_sectors = (uint32_t)total - bu;
    out->align = bu < PC_FMT_ALIGN_MAX ? bu : PC_FMT_ALIGN_MAX;
    out->cluster_bytes = spc * 512;
    out->fs = fs;
    out->n_fat = fs == PC_FMT_EXFAT ? 1 : 2;
    out->mbr_type = fs == PC_FMT_EXFAT ? 0x07 : fs == PC_FMT_FAT32 ? 0x0C : 0x06;
    return true;
}

const char *pc_fmt_fs_name(int fs) { return fs == PC_FMT_FAT16 ? "FAT16" : fs == PC_FMT_FAT32 ? "FAT32" : fs == PC_FMT_EXFAT ? "exFAT" : "?"; }

static void st32(uint8_t *b, uint32_t v) { b[0] = (uint8_t)v; b[1] = (uint8_t)(v >> 8); b[2] = (uint8_t)(v >> 16); b[3] = (uint8_t)(v >> 24); }

void pc_fmt_mbr(const pc_fmt_plan_t *p, uint32_t disk_sig, uint8_t sec[512])
{
    memset(sec, 0, 512);
    st32(sec + 440, disk_sig);
    uint8_t *e = sec + 446;
    e[0] = 0x00;
    e[1] = 0xFE; e[2] = 0xFF; e[3] = 0xFF;                 /* CHS beyond 8 GB: "use the LBA" */
    e[4] = p->mbr_type;
    e[5] = 0xFE; e[6] = 0xFF; e[7] = 0xFF;
    st32(e + 8, p->part_start);
    st32(e + 12, p->part_sectors);
    sec[510] = 0x55;
    sec[511] = 0xAA;
}

void pc_fmt_size_label(uint64_t bytes, char *b, size_t n)
{
    if (!n) return;
    if (bytes < 1000000000ull) { snprintf(b, n, "%u MB", (unsigned)((bytes + 500000) / 1000000)); return; }
    uint64_t tenths = (bytes + 50000000ull) / 100000000ull;
    if (tenths < 1000) snprintf(b, n, "%u.%u GB", (unsigned)(tenths / 10), (unsigned)(tenths % 10));
    else snprintf(b, n, "%llu GB", (unsigned long long)((bytes + 500000000ull) / 1000000000ull));
}

bool pc_fmt_same_card(const pc_fmt_card_t *shown, const pc_fmt_card_t *now)
{
    if (!shown || !now || !shown->sectors) return false;
    return shown->sectors == now->sectors && shown->sector_size == now->sector_size && shown->serial == now->serial;
}

int pc_fmt_outcome(int stage, bool ok)
{
    if (ok) return stage == PC_FMT_STAGE_MOUNT ? PC_FMT_OUT_DONE : PC_FMT_OUT_BROKEN;
    switch (stage) {
    case PC_FMT_STAGE_BEFORE: return PC_FMT_OUT_UNTOUCHED;
    case PC_FMT_STAGE_MOUNT: return PC_FMT_OUT_UNMOUNTED;
    default: return PC_FMT_OUT_BROKEN;                    /* WRITE, MKFS and anything unknown: the card may be written */
    }
}

const char *pc_fmt_stage_name(int stage)
{
    static const char *const N[] = { "before", "write", "mkfs", "mount" };
    return stage >= 0 && stage <= PC_FMT_STAGE_MOUNT ? N[stage] : "?";
}

const char *pc_fmt_outcome_name(int out)
{
    static const char *const N[] = { "done", "untouched", "broken", "unmounted" };
    return out >= 0 && out <= PC_FMT_OUT_UNMOUNTED ? N[out] : "?";
}
