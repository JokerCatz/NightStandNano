// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_fmtreason.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint16_t le16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

/* VGM header clock fields (VGM 1.71). The three the emulator has (SN76489 0x0C, YM2413 0x10, YM2612 0x2C) are not listed. */
static const struct { uint8_t off; const char *name; } VGM_CHIPS[] = {
    { 0x30, "YM2151" }, { 0x38, "SegaPCM" }, { 0x40, "RF5C68" }, { 0x44, "YM2203" }, { 0x48, "YM2608" }, { 0x4C, "YM2610" },
    { 0x50, "YM3812" }, { 0x54, "YM3526" }, { 0x58, "Y8950" }, { 0x5C, "YMF262" }, { 0x60, "YMF278B" }, { 0x64, "YMF271" },
    { 0x68, "YMZ280B" }, { 0x6C, "RF5C164" }, { 0x70, "PWM" }, { 0x74, "AY8910" }, { 0x80, "GameBoy" }, { 0x84, "NES APU" },
    { 0x88, "MultiPCM" }, { 0x8C, "uPD7759" }, { 0x90, "OKIM6258" }, { 0x98, "OKIM6295" }, { 0x9C, "K051649" }, { 0xA0, "K054539" },
    { 0xA4, "HuC6280" }, { 0xA8, "C140" }, { 0xAC, "K053260" }, { 0xB0, "Pokey" }, { 0xB4, "QSound" }, { 0xB8, "SCSP" },
    { 0xC0, "WonderSwan" }, { 0xC4, "VSU" }, { 0xC8, "SAA1099" }, { 0xCC, "ES5503" }, { 0xD0, "ES5506" }, { 0xD8, "X1-010" },
    { 0xDC, "C352" }, { 0xE0, "GA20" },
};

int pc_vgm_unsupported_chips(const uint8_t *d, size_t n, char *out, size_t cap)
{
    if (cap) out[0] = 0;
    if (n < 0x40 || memcmp(d, "Vgm ", 4)) return -1;
    uint32_t ver = le32(d + 8), start = 0x40;
    if (ver >= 0x150 && le32(d + 0x34)) start = 0x34u + le32(d + 0x34);   /* the header ends where the data starts: a field beyond it does not exist */
    if (start > n) start = (uint32_t)n;
    int count = 0;
    size_t len = 0;
    for (size_t i = 0; i < sizeof VGM_CHIPS / sizeof VGM_CHIPS[0]; i++) {
        uint32_t o = VGM_CHIPS[i].off;
        if (o + 4 > start || !(le32(d + o) & 0x3FFFFFFFu)) continue;   /* bit 30 = dual chip, bit 31 = a variant flag: not the clock */
        if (cap) len += (size_t)snprintf(out + len, len < cap ? cap - len : 0, "%s%s", count ? ", " : "", VGM_CHIPS[i].name);
        count++;
    }
    return count;
}

/* S3M: an instrument of type 2 or more is an OPL (AdLib) voice. */
static bool s3m_has_opl(const pc_reader_t *r)
{
    uint8_t h[0x60];
    if (r->size < sizeof h || r->read(r->u, 0, h, sizeof h) != (int)sizeof h || memcmp(h + 0x2C, "SCRM", 4)) return false;
    uint32_t ord = le16(h + 0x20), ins = le16(h + 0x22);
    for (uint32_t i = 0; i < ins && i < 255; i++) {
        uint8_t p[2], t;
        if (r->read(r->u, sizeof h + ord + 2 * i, p, 2) != 2) return false;
        uint32_t o = (uint32_t)le16(p) * 16u;
        if (o && o < r->size && r->read(r->u, o, &t, 1) == 1 && t >= 2) return true;
    }
    return false;
}

/* AIFF / AIFC: the COMM chunk's compression type of an AIFC, if it is not 'NONE' or 'sowt'. */
static bool aiff_codec(const pc_reader_t *r, char *detail, size_t cap)
{
    uint8_t h[12], c[8], f[30];
    if (r->size < 12 || r->read(r->u, 0, h, 12) != 12 || memcmp(h, "FORM", 4) || memcmp(h + 8, "AIFC", 4)) return false;
    uint64_t o = 12;
    for (int k = 0; k < 64 && o + 8 <= r->size; k++) {
        if (r->read(r->u, (uint32_t)o, c, 8) != 8) return false;
        uint32_t sz = be32(c + 4);
        if (!memcmp(c, "COMM", 4) && sz >= 22 && r->read(r->u, (uint32_t)o + 8, f, 22 + 4) == 26) {
            if (memcmp(f + 18, "NONE", 4) && memcmp(f + 18, "sowt", 4)) {
                if (detail && cap) snprintf(detail, cap, "%c%c%c%c", f[18], f[19], f[20], f[21]);
                return true;
            }
            return false;
        }
        o += 8u + sz + (sz & 1);
    }
    return false;
}

pc_refuse_t pc_refuse_reason(const pc_reader_t *r, const char *ext, char *detail, size_t cap)
{
    if (detail && cap) detail[0] = 0;
    if (!r || !r->read || !ext) return PC_REFUSE_NONE;
    if (!strcasecmp(ext, ".s3m")) return s3m_has_opl(r) ? PC_REFUSE_OPL_MODULE : PC_REFUSE_NONE;
    if (!strcasecmp(ext, ".aif") || !strcasecmp(ext, ".aiff") || !strcasecmp(ext, ".aifc"))
        return aiff_codec(r, detail, cap) ? PC_REFUSE_AIFF_CODEC : PC_REFUSE_NONE;
    if (!strcasecmp(ext, ".tta")) {
        uint8_t h[8];
        if (r->read(r->u, 0, h, 8) == 8 && !memcmp(h, "TTA1", 4) && le16(h + 4) == 2) return PC_REFUSE_TTA_ENCRYPTED;
        return PC_REFUSE_NONE;
    }
    if (!strcasecmp(ext, ".mpc") || !strcasecmp(ext, ".mp+")) {
        uint8_t h[4];
        if (r->read(r->u, 0, h, 4) == 4 && !memcmp(h, "MP+", 3) && (h[3] & 15) < 7) return PC_REFUSE_MPC_OLD;
        return PC_REFUSE_NONE;
    }
    return PC_REFUSE_NONE;
}
