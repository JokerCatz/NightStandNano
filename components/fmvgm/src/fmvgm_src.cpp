// SPDX-License-Identifier: LGPL-2.1-or-later
/* VGM and S98 command sources, tags, routing. Plain C++ (no exceptions, no allocation). Formats: VGM specification 1.71 (command lengths as in the public spec),
 * S98 versions 0-3 (device table, sync / wait / end commands, loop offset, tags). */
#include "fmvgm_int.h"
#include "fmvgm.h"
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include "pc/pc_vgmhdr.h"

static uint32_t rd32(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }


/* ---- text ------------------------------------------------------------------------------------------------------------------------------------------------------- */
static void put_utf8(char *dst, size_t cap, size_t &n, uint32_t cp)
{
    char t[4]; int k;
    if (cp < 0x80) { t[0] = (char)cp; k = 1; }
    else if (cp < 0x800) { t[0] = (char)(0xC0 | cp >> 6); t[1] = (char)(0x80 | (cp & 63)); k = 2; }
    else if (cp < 0x10000) { t[0] = (char)(0xE0 | cp >> 12); t[1] = (char)(0x80 | ((cp >> 6) & 63)); t[2] = (char)(0x80 | (cp & 63)); k = 3; }
    else { t[0] = (char)(0xF0 | cp >> 18); t[1] = (char)(0x80 | ((cp >> 12) & 63)); t[2] = (char)(0x80 | ((cp >> 6) & 63)); t[3] = (char)(0x80 | (cp & 63)); k = 4; }
    if (n + k + 1 > cap) return;
    memcpy(dst + n, t, k); n += k;
}

/* NUL-terminated UTF-16LE at p (at most maxbytes) -> UTF-8 in dst; returns bytes consumed including the terminator, 0 when the string is not terminated inside maxbytes */
static uint32_t utf16_to_utf8(const uint8_t *p, uint32_t maxbytes, char *dst, size_t cap)
{
    size_t n = 0; uint32_t i = 0;
    for (;; i += 2) {
        if (i + 2 > maxbytes) { if (cap) dst[n] = 0; return 0; }
        uint32_t u = p[i] | (uint32_t)p[i + 1] << 8;
        if (!u) { if (cap) dst[n] = 0; return i + 2; }
        if (u >= 0xD800 && u < 0xDC00 && i + 4 <= maxbytes) {
            uint32_t v = p[i + 2] | (uint32_t)p[i + 3] << 8;
            if (v >= 0xDC00 && v < 0xE000) { u = 0x10000 + ((u - 0xD800) << 10) + (v - 0xDC00); i += 2; }
            else u = '?';
        } else if (u >= 0xD800 && u < 0xE000) u = '?';
        if (u < 0x20 && u != 0) u = ' ';
        if (cap) put_utf8(dst, cap, n, u);
    }
}

static void gd3_read(const uint8_t *d, uint32_t n, uint32_t off, SrcInfo &i)
{
    if (!off || off > n || n - off < 12 || memcmp(d + off, "Gd3 ", 4)) return;
    uint32_t len = rd32(d + off + 8);
    if (len > n - off - 12 || len > 0x10000) return;
    const uint8_t *p = d + off + 12; uint32_t left = len;
    char f[11][96]; memset(f, 0, sizeof f);
    for (int k = 0; k < 11; k++) {
        uint32_t c = utf16_to_utf8(p, left, f[k], sizeof f[k]);
        if (!c) break;
        p += c; left -= c;
    }
    snprintf(i.title, sizeof i.title, "%s", f[0][0] ? f[0] : f[1]);
    snprintf(i.game, sizeof i.game, "%s", f[2][0] ? f[2] : f[3]);
    snprintf(i.system, sizeof i.system, "%s", f[4][0] ? f[4] : f[5]);
    snprintf(i.author, sizeof i.author, "%s", f[6][0] ? f[6] : f[7]);
}

/* ---- VGM -------------------------------------------------------------------------------------------------------------------------------------------------------- */
uint64_t g_drop_mask;
void fmvgm_set_drop(uint64_t mask) { g_drop_mask = mask; }

/* chip type by the header's chip id (pc_vgmhdr.h); CT_NONE = not played by this backend */
static int ct_of(int id)
{
    switch (id) {
    case PC_VGM_YM2151: return CT_OPM; case PC_VGM_YM2203: return CT_OPN; case PC_VGM_YM2608: return CT_OPNA; case PC_VGM_YM2610: return CT_OPNB;
    case PC_VGM_YM2612: return CT_OPN2; case PC_VGM_AY8910: return CT_SSG; case PC_VGM_SN76489: return CT_SN; case PC_VGM_OKIM6295: return CT_OKI;
    }
    return CT_NONE;
}
static uint64_t played_mask(void)
{
    uint64_t m = 0;
    for (int id = 0; id < PC_VGM_CHIP_N; id++) if (ct_of(id)) m |= PC_VGM_BIT(id);
    return m;
}
uint64_t fmvgm_played_chips(void) { return played_mask(); }

static void put_cmd(Src &s, uint8_t cmd, int slot, int kind, int port) { s.slotmap[cmd] = (uint8_t)slot; s.cmdkind[cmd] = (uint8_t)kind; s.cmdport[cmd] = (uint8_t)port; }

int vgm_open(const uint8_t *d, uint32_t n, Src &s, SrcInfo &i, uint64_t drop_mask)
{
    memset(&s, 0, sizeof s);
    pc_vgm_info_t info;
    if (pc_vgm_parse(d, n, &info) != PC_VGM_OK) { snprintf(i.why, sizeof i.why, "bad VGM header"); return -1; }
    pc_vgm_use_t use;
    pc_vgm_scan(d, n, &info, &use);
    uint64_t bad = pc_vgm_unsupported(&info, played_mask(), &use);
    i.dropped = bad & drop_mask; bad &= ~drop_mask;                                    /* sample chips the host allows to be left out (the partial-playback policy) */
    if (bad) { char nm[40]; pc_vgm_names(&info, bad, nm, sizeof nm); snprintf(i.why, sizeof i.why, "chip %s", nm); return -2; }
    if (info.flags & PC_VGM_I_UNKNOWN_CHIP) { snprintf(i.why, sizeof i.why, "unknown chip"); return -2; }
    if (i.dropped) pc_vgm_names(&info, i.dropped, i.dropped_names, sizeof i.dropped_names);
    memset(s.slotmap, 0xFF, sizeof s.slotmap); memset(s.tslot, 0xFF, sizeof s.tslot);
    i.n_chips = 0;
    for (int k = 0; k < info.n_chips; k++) {
        const pc_vgm_chipinfo_t &c = info.chips[k];
        int type = ct_of(c.id);
        if (!type) continue;
        if (use.valid && !(use.used & PC_VGM_BIT(c.id))) continue;                       /* declared and never written */
        if (!fm_clock_ok(type, c.clock)) { snprintf(i.why, sizeof i.why, "%s clock %lu Hz", pc_vgm_chip_name(c.id), (unsigned long)c.clock); return -3; }
        uint32_t variant = 0;
        if (c.flags & PC_VGM_CF_VARIANT) {
            if (type == CT_SN) { snprintf(i.why, sizeof i.why, "T6W28"); return -3; }
            variant = 1;                                                                 /* YM2610B, YM3438, OKIM6295 pin 7 */
        }
        if (type == CT_SN && info.version >= 0x151 && info.data_off > 0x2B)
            variant = (uint32_t)(d[0x28] | d[0x29] << 8) | (uint32_t)d[0x2A] << 16 | (uint32_t)d[0x2B] << 24;     /* feedback, shift register width, flags (VGM 1.51) */
        int copies = (c.flags & PC_VGM_CF_DUAL) ? 2 : 1;
        for (int q = 0; q < copies; q++) {
            if (i.n_chips >= FM_MAX_CHIPS) { snprintf(i.why, sizeof i.why, "too many chips"); return -3; }
            int slot = i.n_chips++;
            i.chip[slot].type = (uint8_t)type; i.chip[slot].clock = c.clock; i.chip[slot].variant = variant;
            s.tslot[type][q] = (uint8_t)slot;
            int o = q ? 0x50 : 0;                                                        /* the second chip's commands are the first's + 0x50 (0xA0-based ones are special) */
            switch (type) {
            case CT_OPM:  put_cmd(s, 0x54 + o, slot, 1, 0); break;
            case CT_OPN:  put_cmd(s, 0x55 + o, slot, 1, 0); break;
            case CT_OPN2: put_cmd(s, 0x52 + o, slot, 1, 0); put_cmd(s, 0x53 + o, slot, 1, 1); break;
            case CT_OPNA: put_cmd(s, 0x56 + o, slot, 1, 0); put_cmd(s, 0x57 + o, slot, 1, 1); break;
            case CT_OPNB: put_cmd(s, 0x58 + o, slot, 1, 0); put_cmd(s, 0x59 + o, slot, 1, 1); break;
            case CT_SN:   put_cmd(s, q ? 0x30 : 0x50, slot, 2, 0); put_cmd(s, q ? 0x3F : 0x4F, slot, 4, 0); break;
            case CT_SSG:  if (!q) put_cmd(s, 0xA0, slot, 3, CT_SSG); break;              /* 0xA0 aa dd: bit 7 of aa picks the chip */
            case CT_OKI:  if (!q) put_cmd(s, 0xB8, slot, 3, CT_OKI); break;
            }
        }
    }
    if (!i.n_chips) { snprintf(i.why, sizeof i.why, "no chip to play"); return -3; }
    if (s.tslot[CT_OPN2][0] != 0xFF && use.dac_stream) { snprintf(i.why, sizeof i.why, "YM2612 DAC stream"); return -3; }      /* 0x90-0x95: not played (GME does not walk it correctly either) */
    s.dac_slot = s.tslot[CT_OPN2][0];
    s.d = d; s.size = n; s.fmt = SRC_VGM; s.unit_p = 44100; s.unit_q = 1;
    s.data_off = info.data_off; s.stream_end = n;
    if (s.data_off >= n) { snprintf(i.why, sizeof i.why, "no data"); return -1; }
    s.loop_off = (info.loop_off >= s.data_off && info.loop_off < n) ? info.loop_off : 0;
    s.pos = s.data_off;
    i.format = SRC_VGM; i.unit_p = 44100; i.unit_q = 1;
    gd3_read(d, n, info.gd3_off, i);
    return 0;
}

/* Bytes after the opcode for the commands this player passes over (VGM 1.71 spec). -1 = unknown opcode. */
static int vgm_skip_len(uint8_t c)
{
    if (c >= 0x30 && c <= 0x3F) return 1;
    if (c >= 0x40 && c <= 0x4E) return 2;
    if (c == 0x4F || c == 0x50) return 1;
    if (c >= 0x51 && c <= 0x5F) return 2;
    if (c >= 0xA0 && c <= 0xBF) return 2;
    if (c >= 0xC0 && c <= 0xDF) return 3;
    if (c >= 0xE1 && c <= 0xFF) return 4;
    switch (c) { case 0x90: case 0x91: case 0x95: return 4; case 0x92: return 5; case 0x93: return 10; case 0x94: return 1; case 0x68: return 11; case 0xE0: return 4; }
    return -1;
}

/* the chip, memory class and header size of a ROM / bank data block type; false = not a block this backend uses */
static bool block_target(uint8_t tt, int *type, uint8_t *cls, bool *rom)
{
    switch (tt) {
    case 0x00: *type = CT_OPN2; *cls = MC_PCM; *rom = false; return true;              /* YM2612 PCM bank (the 0x8n commands) */
    case 0x81: *type = CT_OPNA; *cls = MC_ADPCM_B; *rom = true; return true;           /* YM2608 DELTA-T */
    case 0x82: *type = CT_OPNB; *cls = MC_ADPCM_A; *rom = true; return true;           /* YM2610 ADPCM-A ROM */
    case 0x83: *type = CT_OPNB; *cls = MC_ADPCM_B; *rom = true; return true;           /* YM2610 DELTA-T ROM */
    case 0x8B: *type = CT_OKI;  *cls = MC_PCM; *rom = true; return true;               /* OKIM6295 ROM */
    }
    return false;
}

static int vgm_next(Src &s, Ev &e)
{
    const uint8_t *d = s.d;
    for (;;) {
        if (s.scan && !s.intro_set && s.loop_off && s.pos == s.loop_off) { s.intro = s.acc; s.intro_set = 1; }
        if (s.pos >= s.stream_end) return EV_END;
        if (++s.no_wait_run > 4000000u) { s.err = 1; return EV_END; }
        uint8_t c = d[s.pos];
        uint32_t left = s.stream_end - s.pos - 1;
        switch (c) {
        case 0x61: if (left < 2) { s.err = 1; return EV_END; } e.kind = EV_WAIT; e.units = d[s.pos + 1] | d[s.pos + 2] << 8; s.pos += 3; s.no_wait_run = 0; return EV_WAIT;
        case 0x62: s.pos++; e.kind = EV_WAIT; e.units = 735; s.no_wait_run = 0; return EV_WAIT;
        case 0x63: s.pos++; e.kind = EV_WAIT; e.units = 882; s.no_wait_run = 0; return EV_WAIT;
        case 0x66:
            if (s.loop_off && !s.scan && s.loop_off < s.stream_end) { s.pos = s.loop_off; continue; }
            s.pos = s.stream_end; return EV_END;
        case 0x67: {                                                    /* data block: 0x66 tt ssssssss data (bit 31 of the size: the second chip) */
            if (left < 6 || d[s.pos + 1] != 0x66) { s.err = 1; return EV_END; }
            uint8_t tt = d[s.pos + 2]; uint32_t raw = rd32(d + s.pos + 3), sz = raw & 0x7FFFFFFFu; int second = raw >> 31;
            if (sz > left - 6) { s.err = 1; return EV_END; }
            const uint8_t *blk = d + s.pos + 7;
            s.pos += 7 + sz;
            int type; uint8_t cls; bool rom;
            if (!block_target(tt, &type, &cls, &rom)) continue;
            uint8_t slot = s.tslot[type][second];
            if (slot == 0xFF) continue;
            e.kind = EV_DATA; e.slot = slot; e.cls = cls; e.units = 0;
            if (rom) { if (sz < 8) continue; e.total = rd32(blk); e.start = rd32(blk + 4); e.p = blk + 8; e.len = sz - 8; }
            else { e.total = 0; e.start = 0xFFFFFFFFu; e.p = blk; e.len = sz; }
            return EV_DATA; }
        case 0xE0:
            if (left < 4) { s.err = 1; s.pos = s.stream_end; return EV_END; }
            if (s.dac_slot != 0xFF) { e.kind = EV_SEEK; e.slot = s.dac_slot; e.start = rd32(d + s.pos + 1); s.pos += 5; return EV_SEEK; }
            s.pos += 5; continue;
        default: break;
        }
        if ((c & 0xF0) == 0x70) { s.pos++; e.kind = EV_WAIT; e.units = (c & 15) + 1u; s.no_wait_run = 0; return EV_WAIT; }
        if ((c & 0xF0) == 0x80) {                                       /* YM2612: write the next byte of the PCM bank to register 0x2A, then wait */
            s.pos++; s.no_wait_run = 0;
            if (s.dac_slot != 0xFF) { e.kind = EV_DAC; e.slot = s.dac_slot; e.units = c & 15; return EV_DAC; }
            e.kind = EV_WAIT; e.units = c & 15; if (e.units) return EV_WAIT; continue;
        }
        if (s.slotmap[c] != 0xFF) {
            int kind = s.cmdkind[c];
            if (kind == 1) {
                if (left < 2) { s.err = 1; return EV_END; }
                e.kind = EV_WRITE; e.slot = s.slotmap[c]; e.port = s.cmdport[c]; e.reg = d[s.pos + 1]; e.val = d[s.pos + 2]; s.pos += 3; return EV_WRITE;
            }
            if (kind == 2 || kind == 4) {                               /* SN76489: one data byte (0x4F / 0x3F: the Game Gear stereo byte, register 1) */
                if (left < 1) { s.err = 1; return EV_END; }
                e.kind = EV_WRITE; e.slot = s.slotmap[c]; e.port = 0; e.reg = kind == 4; e.val = d[s.pos + 1]; s.pos += 2; return EV_WRITE;
            }
            if (kind == 3) {                                            /* aa dd, bit 7 of aa: second chip */
                if (left < 2) { s.err = 1; return EV_END; }
                uint8_t aa = d[s.pos + 1], v = d[s.pos + 2]; s.pos += 3;
                uint8_t slot = s.tslot[s.cmdport[c]][aa >> 7];
                if (slot == 0xFF) continue;
                e.kind = EV_WRITE; e.slot = slot; e.port = 0; e.reg = aa & 0x7F; e.val = v; return EV_WRITE;
            }
        }
        int k = vgm_skip_len(c);
        if (k < 0 || (uint32_t)k > left) { s.err = 1; s.pos = s.stream_end; return EV_END; }
        s.pos += 1 + k;
    }
}

/* ---- S98 -------------------------------------------------------------------------------------------------------------------------------------------------------- */
static const char *s98_dev_name(uint32_t t)
{
    switch (t) { case 1: return "YM2149"; case 2: return "YM2203"; case 3: return "YM2612"; case 4: return "YM2608"; case 5: return "YM2151"; case 6: return "YM2413"; case 7: return "YM3526";
                 case 8: return "YM3812"; case 9: return "YMF262"; case 15: return "AY8910"; case 16: return "SN76489"; }
    return "device?";
}

struct S98Dev { uint32_t type, clock; };
/* S98 device type -> chip type of this backend (0 = not played): 1 YM2149 and 15 AY8910 (SSG), 2 YM2203, 3 YM2612, 4 YM2608, 5 YM2151, 16 SN76489. S98 v3 defines no YM2610 / OPNB device. */
static int s98_type(uint32_t t)
{
    switch (t) { case 1: case 15: return CT_SSG; case 2: return CT_OPN; case 3: return CT_OPN2; case 4: return CT_OPNA; case 5: return CT_OPM; case 16: return CT_SN; }
    return CT_NONE;
}
/* the device table: V0 / V1 none (one YM2608 at 7987200 Hz), V2 / V3 16-byte entries from 0x20 (V3 says how many at 0x1C; V2 ends with type 0). Returns the count, -1 on a damaged table */
static int s98_devices(const uint8_t *d, uint32_t n, int ver, uint32_t data_off, S98Dev *dev, int maxdev)
{
    if (ver < 2) { dev[0].type = 4; dev[0].clock = 7987200; return 1; }
    uint32_t lim = data_off < n ? data_off : n;
    uint32_t cnt = ver >= 3 ? rd32(d + 0x1C) : 0xFFFFFFFFu;
    int k = 0;
    for (uint32_t off = 0x20; ; off += 16, k++) {
        if (ver >= 3 && (uint32_t)k >= cnt) break;
        if (off + 16 > lim) { if (ver >= 3) return -1; break; }
        uint32_t t = rd32(d + off);
        if (ver < 3 && t == 0) break;
        if (k >= maxdev) return -1;
        dev[k].type = t; dev[k].clock = rd32(d + off + 4);
    }
    return k;
}

int fmvgm_is_s98(const void *mem, uint32_t size)
{
    const uint8_t *d = (const uint8_t *)mem;
    return size >= 0x20 && d[0] == 'S' && d[1] == '9' && d[2] == '8' && d[3] >= '0' && d[3] <= '3';
}

int fmvgm_s98_missing(const void *mem, uint32_t size, char *names, size_t cap)
{
    if (names && cap) names[0] = 0;
    if (!fmvgm_is_s98(mem, size)) return -1;
    const uint8_t *d = (const uint8_t *)mem;
    int ver = d[3] - '0';
    uint32_t data_off = rd32(d + 0x14);
    S98Dev dev[16];
    int nd = s98_devices(d, size, ver, data_off, dev, 16);
    if (nd < 0) { if (names && cap) snprintf(names, cap, "damaged device table"); return 0; }
    size_t w = 0;
    for (int k = 0; k < nd && names && w + 12 < cap; k++) {
        if (s98_type(dev[k].type)) continue;
        w += snprintf(names + w, cap - w, "%s%s", w ? ", " : "", s98_dev_name(dev[k].type));
    }
    return 0;
}

static void s98_text(char *dst, size_t cap, const char *src, size_t n, bool utf8)
{
    size_t w = 0;
    for (size_t k = 0; k < n && w + 5 < cap; ) {
        uint8_t c = (uint8_t)src[k];
        if (c < 0x20) { if (c == 0) break; k++; continue; }
        if (c < 0x80) { dst[w++] = (char)c; k++; continue; }
        if (utf8) {                                                   /* copy a well-formed sequence, '?' for anything else */
            int m = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 0;
            bool ok = m && k + m <= n;
            for (int q = 1; ok && q < m; q++) if (((uint8_t)src[k + q] & 0xC0) != 0x80) ok = false;
            if (ok && w + m < cap) { memcpy(dst + w, src + k, m); w += m; k += m; continue; }
        }
        dst[w++] = '?'; k++;
    }
    dst[w] = 0;
}

static void s98_tags(const uint8_t *d, uint32_t n, uint32_t off, SrcInfo &i)
{
    if (!off || off >= n) return;
    const char *t = (const char *)d + off; size_t len = n - off;
    if (len >= 5 && !memcmp(t, "[S98]", 5)) {
        t += 5; len -= 5;
        bool utf8 = len >= 3 && !memcmp(t, "\xEF\xBB\xBF", 3);
        if (utf8) { t += 3; len -= 3; }
        size_t p = 0;
        while (p < len && t[p]) {
            size_t e = p; while (e < len && t[e] && t[e] != '\n') e++;
            size_t le = e; if (le > p && t[le - 1] == '\r') le--;
            const char *eq = (const char *)memchr(t + p, '=', le - p);
            if (eq) {
                size_t kl = eq - (t + p); const char *v = eq + 1; size_t vl = le - p - kl - 1;
                if (kl == 5 && !strncasecmp(t + p, "title", 5)) s98_text(i.title, sizeof i.title, v, vl, utf8);
                else if (kl == 6 && !strncasecmp(t + p, "artist", 6)) s98_text(i.author, sizeof i.author, v, vl, utf8);
                else if (kl == 4 && !strncasecmp(t + p, "game", 4)) s98_text(i.game, sizeof i.game, v, vl, utf8);
                else if (kl == 6 && !strncasecmp(t + p, "system", 6)) s98_text(i.system, sizeof i.system, v, vl, utf8);
            }
            p = e < len && t[e] == '\n' ? e + 1 : e;
        }
    } else {                                                           /* V1 / V2: one string, the song title (Shift-JIS: only ASCII is shown) */
        s98_text(i.title, sizeof i.title, t, len < 200 ? len : 200, false);
    }
}

/* which devices the command stream writes to (bit per device index, 0..15), the stream walked once from `from` to the end command without following the loop */
static uint32_t s98_devs_used(const uint8_t *d, uint32_t from, uint32_t end)
{
    uint32_t hit = 0, p = from;
    while (p < end) {
        uint8_t b = d[p];
        if (b == 0xFF) p++;
        else if (b == 0xFE) { p++; while (p < end && (d[p] & 0x80)) p++; p++; }
        else if (b == 0xFD) break;
        else { if ((b >> 1) < 16) hit |= 1u << (b >> 1); p += 3; }
    }
    return hit;
}

int s98_open(const uint8_t *d, uint32_t n, Src &s, SrcInfo &i, uint64_t)
{
    memset(&s, 0, sizeof s);
    if (!fmvgm_is_s98(d, n)) { snprintf(i.why, sizeof i.why, "not S98"); return -1; }
    int ver = d[3] - '0';
    uint32_t num = rd32(d + 4), den = rd32(d + 8);
    if (ver == 0) num = 0;
    if (num == 0) num = 10;
    if (den == 0) den = 1000;
    if (rd32(d + 0x0C) != 0) { snprintf(i.why, sizeof i.why, "compressed S98"); return -1; }
    uint32_t tag_off = rd32(d + 0x10), data_off = rd32(d + 0x14), loop_off = rd32(d + 0x18);
    if (data_off < 0x20 || data_off >= n) { snprintf(i.why, sizeof i.why, "bad data offset"); return -1; }
    if (num > 1000 || den > 1000000u || den == 0) { snprintf(i.why, sizeof i.why, "timer %lu/%lu", (unsigned long)num, (unsigned long)den); return -3; }
    if ((uint64_t)num * 1000 > (uint64_t)den * 100) { snprintf(i.why, sizeof i.why, "tick over 100 ms"); return -3; }     /* num/den seconds per tick */
    S98Dev dev[16];
    int nd = s98_devices(d, n, ver, data_off, dev, 16);
    if (nd < 1) { snprintf(i.why, sizeof i.why, "bad device table"); return -1; }
    memset(s.slotmap, 0xFF, sizeof s.slotmap);
    i.n_chips = 0;
    uint32_t badmask = 0;
    for (int k = 0; k < nd; k++) {
        int type = s98_type(dev[k].type);
        if (!type) { badmask |= 1u << k; continue; }
        if (!fm_clock_ok(type, dev[k].clock)) { snprintf(i.why, sizeof i.why, "%s clock %lu Hz", s98_dev_name(dev[k].type), (unsigned long)dev[k].clock); return -3; }
        if (i.n_chips >= FM_MAX_CHIPS) { snprintf(i.why, sizeof i.why, "too many chips"); return -3; }
        i.chip[i.n_chips].type = (uint8_t)type; i.chip[i.n_chips].clock = dev[k].clock; i.chip[i.n_chips].variant = 0;
        s.sports[i.n_chips] = (type == CT_OPN2 || type == CT_OPNA) ? 3 : 1;
        s.slotmap[k] = (uint8_t)i.n_chips++;
    }
    s.d = d; s.size = n; s.fmt = SRC_S98; s.n_dev = (uint8_t)nd;
    s.data_off = data_off; s.stream_end = n; s.pos = data_off;
    s.loop_off = (loop_off >= data_off && loop_off < n) ? loop_off : 0;
    s.unit_p = den; s.unit_q = num;                                    /* ticks per second = den / num */
    i.unit_p = den; i.unit_q = num; i.format = SRC_S98;
    /* a device this player lacks only matters if the stream writes to it */
    if (badmask) {
        uint32_t hit = s98_devs_used(d, data_off, n);
        if (hit & badmask) {
            size_t w = snprintf(i.why, sizeof i.why, "chip ");
            for (int k = 0, first = 1; k < nd; k++) if ((hit & badmask & (1u << k)) && w + 12 < sizeof i.why) { w += snprintf(i.why + w, sizeof i.why - w, "%s%s", first ? "" : ", ", s98_dev_name(dev[k].type)); first = 0; }
            return -2;
        }
    }
    if (!i.n_chips) { snprintf(i.why, sizeof i.why, "no FM chip"); return -3; }
    s98_tags(d, n, tag_off, i);
    return 0;
}

static int s98_next(Src &s, Ev &e)
{
    const uint8_t *d = s.d;
    for (;;) {
        if (s.scan && !s.intro_set && s.loop_off && s.pos == s.loop_off) { s.intro = s.acc; s.intro_set = 1; }
        if (s.pos >= s.stream_end) return EV_END;
        if (++s.no_wait_run > 4000000u) { s.err = 1; return EV_END; }
        uint8_t b = d[s.pos];
        uint32_t left = s.stream_end - s.pos - 1;
        if (b == 0xFF) { s.pos++; e.kind = EV_WAIT; e.units = 1; s.no_wait_run = 0; return EV_WAIT; }
        if (b == 0xFE) {
            uint32_t n = 0, sh = 0, p = s.pos + 1; bool ok = false;
            while (p < s.stream_end && sh < 32) { uint8_t v = d[p++]; n |= (uint32_t)(v & 0x7F) << sh; sh += 7; if (!(v & 0x80)) { ok = true; break; } }
            if (!ok || n > 0x100000u) { s.err = 1; s.pos = s.stream_end; return EV_END; }
            s.pos = p; e.kind = EV_WAIT; e.units = n + 2; s.no_wait_run = 0; return EV_WAIT;
        }
        if (b == 0xFD) {
            if (s.loop_off && !s.scan) { s.pos = s.loop_off; continue; }
            s.pos = s.stream_end; return EV_END;
        }
        if (left < 2) { s.err = 1; s.pos = s.stream_end; return EV_END; }
        uint8_t dev = b >> 1, port = b & 1, reg = d[s.pos + 1], val = d[s.pos + 2];
        s.pos += 3;
        if (dev < 128 && s.slotmap[dev] != 0xFF && (s.sports[s.slotmap[dev]] >> port & 1)) { e.kind = EV_WRITE; e.slot = s.slotmap[dev]; e.port = port; e.reg = reg; e.val = val; return EV_WRITE; }
    }
}

int src_next(Src &s, Ev &e) { int k = s.fmt == SRC_VGM ? vgm_next(s, e) : s98_next(s, e); if ((k == EV_WAIT || k == EV_DAC) && s.scan) s.acc += e.units; return k; }
void src_rewind(Src &s) { s.pos = s.data_off; s.no_wait_run = 0; s.err = 0; }

/* ---- routing ---------------------------------------------------------------------------------------------------------------------------------------------------- */
int fmvgm_route(const void *mem, uint32_t size, uint64_t *used_out, uint64_t *missing_out)
{
    const uint8_t *d = (const uint8_t *)mem;
    pc_vgm_info_t info;
    if (used_out) *used_out = 0;
    if (missing_out) *missing_out = 0;
    if (pc_vgm_parse(d, size, &info) != PC_VGM_OK) return -1;
    pc_vgm_use_t use;
    pc_vgm_scan(d, size, &info, &use);
    const uint64_t gme = PC_VGM_BIT(PC_VGM_SN76489) | PC_VGM_BIT(PC_VGM_YM2413) | PC_VGM_BIT(PC_VGM_YM2612);
    const uint64_t ours = played_mask();
    uint64_t mask = 0;
    for (int k = 0; k < info.n_chips; k++) {
        uint64_t b = PC_VGM_BIT(info.chips[k].id);
        if (use.valid && !(use.used & b)) continue;                    /* declared, never written */
        mask |= b;
    }
    if (used_out) *used_out = mask;
    uint64_t other = mask & ~(gme | ours);
    if (other || (info.flags & PC_VGM_I_UNKNOWN_CHIP)) { if (missing_out) *missing_out = other; return 3; }
    if (!(mask & ~gme)) return 1;                                      /* only GME's chips, or no chip at all */
    if (mask & PC_VGM_BIT(PC_VGM_YM2413)) { if (missing_out) *missing_out = PC_VGM_BIT(PC_VGM_YM2413); return 2; }       /* a YM2413 next to chips GME lacks: neither backend plays both */
    return 0;
}
