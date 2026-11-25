// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_cardtest.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const pc_ct_fmt_t FMTS[] = {
    { "MP3 / AAC / Opus 320k", 0, 0, false, 40000 },      /* 320 kbit/s */
    { "FLAC / WAV 44.1k/16", 44100, 16, false, 0 },       /* the WAV figure: the heavier of the two */
    { "FLAC 96k/24", 96000, 24, true, 0 },
    { "FLAC 192k/24", 192000, 24, true, 0 },
    { "WAV 192k/24", 192000, 24, false, 0 },
    { "FLAC 384k/24", 384000, 24, true, 0 },
    { "WAV 384k/32", 384000, 32, false, 0 },              /* the heaviest the WAV reader takes (stereo, 16 / 24 / 32 bit, up to the 384 kHz rate guard) */
};

uint32_t pc_ct_x100(uint64_t bytes, uint64_t us) { return us ? (uint32_t)(bytes * 100 / us) : 0; }
int pc_ct_format_count(void) { return (int)(sizeof FMTS / sizeof FMTS[0]); }
const pc_ct_fmt_t *pc_ct_format(int i) { return i >= 0 && i < pc_ct_format_count() ? &FMTS[i] : NULL; }

uint32_t pc_ct_stream_bps(const pc_ct_fmt_t *f)
{
    if (!f) return 0;
    if (f->fixed_bps) return f->fixed_bps;
    uint64_t pcm = (uint64_t)f->rate * (f->bits / 8u) * 2u;
    return (uint32_t)(f->flac ? pcm * PC_CT_FLAC_RATIO_PCT / 100 : pcm);
}

uint32_t pc_ct_need_x100(const pc_ct_fmt_t *f)
{
    uint64_t n = (uint64_t)pc_ct_stream_bps(f) * PC_CT_SAFETY_X100;      /* bytes/s x 100 x 100 per MB */
    return (uint32_t)((n + 999999) / 1000000);
}

pc_ct_grade_t pc_ct_grade(uint32_t speed_x100, const pc_ct_fmt_t *f)
{
    uint32_t need = pc_ct_need_x100(f);
    if (speed_x100 >= need) return PC_CT_YES;
    if ((uint64_t)speed_x100 * 100 >= (uint64_t)need * PC_CT_MARGINAL_PCT) return PC_CT_MARGINAL;
    return PC_CT_NO;
}

static uint32_t word_at(uint64_t w)       /* splitmix-style mix of the word index */
{
    uint64_t z = (w + 1) * 0x9E3779B97F4A7C15ull;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return (uint32_t)((z ^ (z >> 31)) >> 8);
}

static uint8_t byte_at(uint64_t pos) { return (uint8_t)(word_at(pos >> 2) >> ((pos & 3) * 8)); }

void pc_ct_fill(uint8_t *buf, uint32_t len, uint64_t offset)
{
    for (uint32_t i = 0; i < len; i++) buf[i] = byte_at(offset + i);
}

bool pc_ct_check(const uint8_t *buf, uint32_t len, uint64_t offset, uint64_t *bad)
{
    for (uint32_t i = 0; i < len; i++)
        if (buf[i] != byte_at(offset + i)) { if (bad) *bad = offset + i; return false; }
    return true;
}

uint32_t pc_ct_follow(uint32_t raw_x100, uint32_t file_x100, bool *file)
{
    if (file) *file = file_x100 != 0;
    return file_x100 ? file_x100 : raw_x100;
}

int pc_ct_ladder(uint32_t raw_x100, uint32_t file_x100, pc_ct_rung_t *out, int max)
{
    const int n = pc_ct_format_count();
    if (max < n + 2) return 0;
    int idx[16];
    for (int i = 0; i < n; i++) idx[i] = i;
    for (int i = 1; i < n; i++) {                                          /* insertion sort, need descending, stable */
        int k = idx[i], j = i;
        while (j > 0 && pc_ct_need_x100(pc_ct_format(idx[j - 1])) < pc_ct_need_x100(pc_ct_format(k))) { idx[j] = idx[j - 1]; j--; }
        idx[j] = k;
    }
    const uint32_t speed = pc_ct_follow(raw_x100, file_x100, NULL);
    const uint32_t ms[2] = { raw_x100, file_x100 };
    bool placed[2] = { false, false };
    int o = 0;
    for (int i = 0; i < n; i++) {
        const pc_ct_fmt_t *f = pc_ct_format(idx[i]);
        for (int m = 0; m < 2; m++)
            if (!placed[m] && ms[m] && pc_ct_need_x100(f) <= ms[m]) { out[o++] = (pc_ct_rung_t){ (int8_t)(m ? PC_CT_RUNG_FILE : PC_CT_RUNG_RAW), 0, ms[m] }; placed[m] = true; }
        out[o++] = (pc_ct_rung_t){ (int8_t)idx[i], (uint8_t)(speed ? pc_ct_grade(speed, f) : PC_CT_NO), pc_ct_need_x100(f) };
    }
    for (int m = 0; m < 2; m++)
        if (!placed[m]) out[o++] = (pc_ct_rung_t){ (int8_t)(m ? PC_CT_RUNG_FILE : PC_CT_RUNG_RAW), 0, ms[m] };
    return o;
}

static const char *const MODE_NAME[PC_CT_MODES] = { "raw", "file" };
/* the first `req` keys must be present; the rest are optional (the raw line's three region speeds reg0..reg2 are written only when measured: older lines still parse) */
static const char *const KEYS_RAW[PC_CT_NV] = { "avg16", "slow16", "max64", "worst_us", "reg0", "reg1", "reg2" };
static const char *const KEYS_FILE[PC_CT_NV] = { "write", "read", "verify", NULL };
static int keys_required(int mode) { return mode == PC_CT_RAW ? 4 : 3; }

int pc_ct_rec_format(int mode, const pc_ct_rec_t *r, char *buf, size_t n)
{
    if (mode < 0 || mode >= PC_CT_MODES) return 0;
    const char *const *k = mode == PC_CT_RAW ? KEYS_RAW : KEYS_FILE;
    int len = snprintf(buf, n, "%s serial=%08lX size=%llu time=%s", MODE_NAME[mode], (unsigned long)r->serial, (unsigned long long)r->size, r->when[0] ? r->when : "-");
    const int req = keys_required(mode);
    for (int i = 0; i < PC_CT_NV && k[i] && len > 0 && (size_t)len < n; i++) if (i < req || r->v[i]) len += snprintf(buf + len, n - (size_t)len, " %s=%lu", k[i], (unsigned long)r->v[i]);
    if (len > 0 && (size_t)len + 1 < n) { buf[len++] = '\n'; buf[len] = 0; return len; }
    return 0;
}

bool pc_ct_rec_parse(const char *line, int *mode, pc_ct_rec_t *r)
{
    int m = -1;
    for (int i = 0; i < PC_CT_MODES; i++) {
        size_t l = strlen(MODE_NAME[i]);
        if (!strncmp(line, MODE_NAME[i], l) && line[l] == ' ') { m = i; line += l; break; }
    }
    if (m < 0) return false;
    const char *const *k = m == PC_CT_RAW ? KEYS_RAW : KEYS_FILE;
    pc_ct_rec_t t;
    memset(&t, 0, sizeof t);
    bool got_serial = false, got_size = false, got_time = false, got[PC_CT_NV] = { false };
    int want = 0;
    for (int i = 0; i < PC_CT_NV && k[i]; i++) want++;
    const int req = keys_required(m);
    while (*line) {
        while (*line == ' ') line++;
        const char *e = line;
        while (*e && *e != ' ' && *e != '\n' && *e != '\r') e++;
        if (e == line) break;
        char tok[48];
        size_t tl = (size_t)(e - line);
        if (tl >= sizeof tok) return false;
        memcpy(tok, line, tl);
        tok[tl] = 0;
        char *eq = strchr(tok, '=');
        if (!eq) return false;
        *eq++ = 0;
        char *end;
        if (!strcmp(tok, "serial")) { t.serial = (uint32_t)strtoul(eq, &end, 16); got_serial = *eq && !*end; }
        else if (!strcmp(tok, "size")) { t.size = strtoull(eq, &end, 10); got_size = *eq && !*end; }
        else if (!strcmp(tok, "time")) {
            if (strlen(eq) >= sizeof t.when) return false;
            strcpy(t.when, eq);
            got_time = true;
        } else {
            for (int i = 0; i < want; i++) if (!strcmp(tok, k[i])) { t.v[i] = (uint32_t)strtoul(eq, &end, 10); got[i] = *eq && !*end; if (!got[i]) return false; }
        }
        line = e;
    }
    for (int i = 0; i < req; i++) if (!got[i]) return false;
    if (!got_serial || !got_size || !got_time) return false;
    *mode = m;
    *r = t;
    return true;
}

bool pc_ct_file_find(const char *text, int mode, uint32_t serial, uint64_t size, pc_ct_rec_t *r)
{
    if (!text) return false;
    while (*text) {
        const char *nl = strchr(text, '\n');
        size_t l = nl ? (size_t)(nl - text) : strlen(text);
        char line[200];
        if (l < sizeof line) {
            memcpy(line, text, l);
            line[l] = 0;
            int m;
            pc_ct_rec_t t;
            if (pc_ct_rec_parse(line, &m, &t) && m == mode && t.serial == serial && t.size == size) { *r = t; return true; }
        }
        text += l + (nl ? 1 : 0);
    }
    return false;
}

size_t pc_ct_file_update(const char *old, int mode, const pc_ct_rec_t *r, char *out, size_t cap)
{
    size_t o = 0;
    for (int m = 0; m < PC_CT_MODES; m++) {
        pc_ct_rec_t t;
        const pc_ct_rec_t *use = NULL;
        if (m == mode) use = r;
        else if (pc_ct_file_find(old, m, r->serial, r->size, &t)) use = &t;
        if (!use) continue;
        int len = pc_ct_rec_format(m, use, out + o, cap - o);
        if (!len) return 0;
        o += (size_t)len;
    }
    return o;
}

uint32_t pc_ct_slowest(const uint32_t *reg, int n)
{
    uint32_t m = 0;
    for (int i = 0; i < n; i++) if (reg[i] && (!m || reg[i] < m)) m = reg[i];
    return m;
}

int pc_ct_pass_role(uint32_t pass) { return (int)(pass % PC_CT_PASSES_PER_REGION); }
int pc_ct_pass_region(uint32_t pass) { return (int)(pass / PC_CT_PASSES_PER_REGION); }
uint32_t pc_ct_pass_sector(uint32_t base, int role, uint32_t region_align) { return role == PC_CT_PASS_MEAS64 ? base + region_align : base; }
