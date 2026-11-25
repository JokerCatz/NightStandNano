// SPDX-License-Identifier: LGPL-2.1-or-later
/* VGM header inspector, see pc/pc_vgmhdr.h. Pure C, no allocation, no dependencies beyond the C library. */
#include "pc/pc_vgmhdr.h"
#include <string.h>

static const char *const NAMES[PC_VGM_CHIP_N] = {
    "SN76489", "YM2413", "YM2612", "YM2151", "SegaPCM", "RF5C68", "YM2203", "YM2608", "YM2610", "YM3812", "YM3526", "Y8950", "YMF262", "YMF278B",
    "YMF271", "YMZ280B", "RF5C164", "PWM", "AY8910", "GameBoy", "NES APU", "MultiPCM", "uPD7759", "OKIM6258", "OKIM6295", "K051649", "K054539",
    "HuC6280", "C140", "K053260", "Pokey", "QSound", "SCSP", "WonderSwan", "VSU", "SAA1099", "ES5503", "ES5506", "X1-010", "C352", "GA20" };

/* the clock fields: header offset and the first version that has the field (BCD) */
typedef struct { uint8_t off, id; uint16_t ver; } field_t;
static const field_t FIELDS[] = {
    { 0x0C, PC_VGM_SN76489, 0x100 }, { 0x10, PC_VGM_YM2413, 0x100 }, { 0x2C, PC_VGM_YM2612, 0x110 }, { 0x30, PC_VGM_YM2151, 0x110 },
    { 0x38, PC_VGM_SEGAPCM, 0x151 }, { 0x40, PC_VGM_RF5C68, 0x151 }, { 0x44, PC_VGM_YM2203, 0x151 }, { 0x48, PC_VGM_YM2608, 0x151 },
    { 0x4C, PC_VGM_YM2610, 0x151 }, { 0x50, PC_VGM_YM3812, 0x151 }, { 0x54, PC_VGM_YM3526, 0x151 }, { 0x58, PC_VGM_Y8950, 0x151 },
    { 0x5C, PC_VGM_YMF262, 0x151 }, { 0x60, PC_VGM_YMF278B, 0x151 }, { 0x64, PC_VGM_YMF271, 0x151 }, { 0x68, PC_VGM_YMZ280B, 0x151 },
    { 0x6C, PC_VGM_RF5C164, 0x151 }, { 0x70, PC_VGM_PWM, 0x151 }, { 0x74, PC_VGM_AY8910, 0x151 },
    { 0x80, PC_VGM_GBDMG, 0x161 }, { 0x84, PC_VGM_NESAPU, 0x161 }, { 0x88, PC_VGM_MULTIPCM, 0x161 }, { 0x8C, PC_VGM_UPD7759, 0x161 },
    { 0x90, PC_VGM_OKIM6258, 0x161 }, { 0x98, PC_VGM_OKIM6295, 0x161 }, { 0x9C, PC_VGM_K051649, 0x161 }, { 0xA0, PC_VGM_K054539, 0x161 },
    { 0xA4, PC_VGM_HUC6280, 0x161 }, { 0xA8, PC_VGM_C140, 0x161 }, { 0xAC, PC_VGM_K053260, 0x161 }, { 0xB0, PC_VGM_POKEY, 0x161 },
    { 0xB4, PC_VGM_QSOUND, 0x161 },
    { 0xB8, PC_VGM_SCSP, 0x171 }, { 0xC0, PC_VGM_WSWAN, 0x171 }, { 0xC4, PC_VGM_VSU, 0x171 }, { 0xC8, PC_VGM_SAA1099, 0x171 },
    { 0xCC, PC_VGM_ES5503, 0x171 }, { 0xD0, PC_VGM_ES5506, 0x171 }, { 0xD8, PC_VGM_X1010, 0x171 }, { 0xDC, PC_VGM_C352, 0x171 },
    { 0xE0, PC_VGM_GA20, 0x171 },
};
#define N_FIELDS (sizeof FIELDS / sizeof FIELDS[0])
#define UNKNOWN_FROM 0xE4          /* v1.72+: clock fields this module has no name for start here */

static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

const char *pc_vgm_chip_name(int id) { return id >= 0 && id < PC_VGM_CHIP_N ? NAMES[id] : "?"; }

const char *pc_vgm_chip_label(const pc_vgm_chipinfo_t *c)
{
    if (c->flags & PC_VGM_CF_VARIANT) {
        if (c->id == PC_VGM_SN76489) return "T6W28";
        if (c->id == PC_VGM_YM2610) return "YM2610B";
        if (c->id == PC_VGM_YM2413) return "VRC7";
    }
    return pc_vgm_chip_name(c->id);
}

static void add_chip(pc_vgm_info_t *i, int id, uint32_t clock, uint8_t flags)
{
    for (int k = 0; k < i->n_chips; k++)
        if (i->chips[k].id == id) {                       /* already declared (main header): the extra header can only add a second chip */
            i->chips[k].flags |= flags & PC_VGM_CF_DUAL;
            if (flags & PC_VGM_CF_DUAL) i->dual_mask |= PC_VGM_BIT(id);
            return;
        }
    i->chips[i->n_chips].id = (uint8_t)id; i->chips[i->n_chips].flags = flags; i->chips[i->n_chips].clock = clock;
    i->n_chips++;
    i->chip_mask |= PC_VGM_BIT(id);
    if (flags & PC_VGM_CF_DUAL) i->dual_mask |= PC_VGM_BIT(id);
}

int pc_vgm_parse(const uint8_t *p, size_t n, pc_vgm_info_t *i)
{
    if (!p || !i || n < 0x40) return PC_VGM_E_SHORT;
    if (memcmp(p, "Vgm ", 4)) return PC_VGM_E_IDENT;
    memset(i, 0, sizeof *i);
    i->version = rd32(p + 0x08);
    if (i->version < 0x100) return PC_VGM_E_VERSION;
    i->eof_size = rd32(p + 0x04) + 4;
    i->total_samples = rd32(p + 0x18); i->loop_samples = rd32(p + 0x20);
    uint32_t v;
    if ((v = rd32(p + 0x14)) != 0) i->gd3_off = 0x14 + v;
    if ((v = rd32(p + 0x1C)) != 0) i->loop_off = 0x1C + v;
    if (i->version >= 0x101) i->rate = rd32(p + 0x24);
    i->data_off = 0x40;
    if (i->version >= 0x150 && (v = rd32(p + 0x34)) != 0) i->data_off = 0x34 + v;      /* relative to 0x34; 0 = the old 0x40 */
    if (i->data_off < 0x40 || i->data_off > 0x7FFFFFFFu) return PC_VGM_E_DATAOFF;
    size_t valid = i->data_off;                                                       /* bytes of header that are fields; the rest is command data */
    if (valid > n) { valid = n; i->flags |= PC_VGM_I_TRUNC; }
    for (unsigned k = 0; k < N_FIELDS; k++) {
        const field_t *f = &FIELDS[k];
        if (i->version < f->ver || (size_t)f->off + 4 > valid) continue;
        uint32_t c = rd32(p + f->off);
        if ((c & 0x3FFFFFFFu) == 0) continue;
        uint8_t fl = (uint8_t)((c & 0x40000000u ? PC_VGM_CF_DUAL : 0) | (c & 0x80000000u ? PC_VGM_CF_VARIANT : 0));
        if (f->id == PC_VGM_YM2413 && i->version < 0x110) fl |= PC_VGM_CF_LEGACY_FM;
        add_chip(i, f->id, c & 0x3FFFFFFFu, fl);
    }
    if (i->version >= 0x172)
        for (size_t o = UNKNOWN_FROM; o + 4 <= valid; o += 4) if (rd32(p + o)) { i->flags |= PC_VGM_I_UNKNOWN_CHIP; break; }
    /* the extra header (v1.70+): chip clocks that the main header cannot carry, mainly second chips. Every read is bounded by n. */
    if (i->version >= 0x170 && 0xBC + 4 <= valid && (v = rd32(p + 0xBC)) != 0) {
        size_t ex = 0xBC + (size_t)v;
        if (ex + 12 <= n) {
            uint32_t co = rd32(p + ex + 4);                                           /* chip clock offset, relative to its own field */
            if (co && ex + 4 + (size_t)co + 1 <= n) {
                size_t cc = ex + 4 + (size_t)co;
                unsigned cnt = p[cc++];
                for (unsigned k = 0; k < cnt && cc + 5 <= n; k++, cc += 5) {
                    unsigned id = p[cc] & 0x7F; uint32_t c = rd32(p + cc + 1) & 0x3FFFFFFFu;
                    if (id < PC_VGM_CHIP_N && c) add_chip(i, (int)id, c, (uint8_t)(p[cc] & 0x80 ? PC_VGM_CF_DUAL : 0));
                }
            }
        }
    }
    return PC_VGM_OK;
}

/* the chip a write command belongs to; -1 none / not a write */
static int cmd_chip(unsigned c)
{
    switch (c) {
    case 0x30: case 0x3F: case 0x4F: case 0x50: return PC_VGM_SN76489;
    case 0xA0: return PC_VGM_AY8910;
    case 0xB0: case 0xC1: return PC_VGM_RF5C68;
    case 0xB1: case 0xC2: return PC_VGM_RF5C164;
    case 0xB2: return PC_VGM_PWM;
    case 0xB3: return PC_VGM_GBDMG;
    case 0xB4: return PC_VGM_NESAPU;
    case 0xB5: case 0xC3: return PC_VGM_MULTIPCM;
    case 0xB6: return PC_VGM_UPD7759;
    case 0xB7: return PC_VGM_OKIM6258;
    case 0xB8: return PC_VGM_OKIM6295;
    case 0xB9: return PC_VGM_HUC6280;
    case 0xBA: return PC_VGM_K053260;
    case 0xBB: return PC_VGM_POKEY;
    case 0xBC: case 0xC6: return PC_VGM_WSWAN;
    case 0xBD: return PC_VGM_SAA1099;
    case 0xBE: case 0xD6: return PC_VGM_ES5506;
    case 0xBF: return PC_VGM_GA20;
    case 0xC0: return PC_VGM_SEGAPCM;
    case 0xC4: return PC_VGM_QSOUND;
    case 0xC5: return PC_VGM_SCSP;
    case 0xC7: return PC_VGM_VSU;
    case 0xC8: return PC_VGM_X1010;
    case 0xD0: return PC_VGM_YMF278B;
    case 0xD1: return PC_VGM_YMF271;
    case 0xD2: return PC_VGM_K051649;
    case 0xD3: return PC_VGM_K054539;
    case 0xD4: return PC_VGM_C140;
    case 0xD5: return PC_VGM_ES5503;
    case 0xE1: return PC_VGM_C352;
    }
    if (c >= 0xA1 && c <= 0xAF) c -= 0x50;                  /* the second chip of 0x51..0x5F */
    switch (c) {
    case 0x51: return PC_VGM_YM2413;
    case 0x52: case 0x53: return PC_VGM_YM2612;
    case 0x54: return PC_VGM_YM2151;
    case 0x55: return PC_VGM_YM2203;
    case 0x56: case 0x57: return PC_VGM_YM2608;
    case 0x58: case 0x59: return PC_VGM_YM2610;
    case 0x5A: return PC_VGM_YM3812;
    case 0x5B: return PC_VGM_YM3526;
    case 0x5C: return PC_VGM_Y8950;
    case 0x5D: return PC_VGM_YMZ280B;
    case 0x5E: case 0x5F: return PC_VGM_YMF262;
    }
    return -1;
}

/* total length of the command (opcode included); 0 = unknown / reserved opcode; the data block (0x67) is handled by the caller */
static unsigned cmd_len(unsigned c)
{
    if (c >= 0x30 && c <= 0x3F) return 2;
    if (c >= 0x40 && c <= 0x4E) return 3;
    if (c == 0x4F || c == 0x50) return 2;
    if (c >= 0x51 && c <= 0x5F) return 3;
    if (c == 0x61) return 3;
    if (c == 0x62 || c == 0x63) return 1;
    if (c == 0x64) return 4;
    if (c == 0x68) return 12;
    if (c >= 0x70 && c <= 0x8F) return 1;
    if (c == 0x90 || c == 0x91 || c == 0x95) return 5;
    if (c == 0x92) return 6;
    if (c == 0x93) return 11;
    if (c == 0x94) return 2;
    if (c >= 0xA0 && c <= 0xBF) return 3;
    if (c >= 0xC0 && c <= 0xDF) return 4;
    if (c >= 0xE0) return 5;
    return 0;
}

void pc_vgm_scan(const uint8_t *vgm, size_t len, pc_vgm_info_t *info, pc_vgm_use_t *use)
{
    memset(use, 0, sizeof *use);
    size_t pos = info->data_off;
    int legacy = -1;
    for (int k = 0; k < info->n_chips; k++) if (info->chips[k].flags & PC_VGM_CF_LEGACY_FM) legacy = k;
    while (pos < len) {
        unsigned c = vgm[pos];
        use->commands++;
        if (c == 0x66) { use->valid = 1; break; }
        if (c == 0x67) {                                    /* 0x67 0x66 type size32 data */
            if (pos + 7 > len || vgm[pos + 1] != 0x66) break;
            uint32_t sz = rd32(vgm + pos + 3);
            if (sz > len - pos - 7) break;
            pos += 7 + (size_t)sz;
            continue;
        }
        unsigned l = cmd_len(c);
        if (!l || pos + l > len) break;
        if (c >= 0x90 && c <= 0x95) use->dac_stream = 1;
        int id = cmd_chip(c);
        if (id >= 0) {
            if (legacy >= 0 && (id == PC_VGM_YM2413 || id == PC_VGM_YM2612 || id == PC_VGM_YM2151)) {   /* the shared pre-1.10 clock belongs to this chip */
                info->chip_mask &= ~PC_VGM_BIT(PC_VGM_YM2413); info->dual_mask &= ~PC_VGM_BIT(PC_VGM_YM2413);
                info->chips[legacy].id = (uint8_t)id; info->chip_mask |= PC_VGM_BIT(id);
                legacy = -1;
            }
            use->used |= PC_VGM_BIT(id);
        }
        pos += l;
    }
    if (pos == len) use->valid = 1;                         /* ran out exactly at a command boundary: an end command was merely missing */
}

uint64_t pc_vgm_unsupported(const pc_vgm_info_t *info, uint64_t supported, const pc_vgm_use_t *use)
{
    uint64_t m = info->chip_mask & ~supported;
    if (use && use->valid) m &= use->used;
    return m;
}

int pc_vgm_names(const pc_vgm_info_t *info, uint64_t mask, char *buf, size_t cap)
{
    size_t n = 0;
    if (!cap) return 0;
    buf[0] = 0;
    for (int k = 0; k < info->n_chips; k++) {
        if (!(mask & PC_VGM_BIT(info->chips[k].id))) continue;
        const char *s = pc_vgm_chip_label(&info->chips[k]);
        size_t sl = strlen(s);
        if (n + (n ? 2 : 0) + sl + 1 > cap) break;
        if (n) { buf[n++] = ','; buf[n++] = ' '; }
        memcpy(buf + n, s, sl); n += sl; buf[n] = 0;
    }
    return (int)n;
}
