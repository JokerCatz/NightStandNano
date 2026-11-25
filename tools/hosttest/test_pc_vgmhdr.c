// SPDX-License-Identifier: LGPL-2.1-or-later
/* Host test of components/player_core/src/pc_vgmhdr.c with synthetic headers (no files needed).
 * With file arguments it instead prints what the inspector sees in each (already un-gzipped) VGM. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pc/pc_vgmhdr.h"

static int fails, checks;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static void w32(uint8_t *b, size_t o, uint32_t v) { b[o] = v; b[o + 1] = v >> 8; b[o + 2] = v >> 16; b[o + 3] = v >> 24; }
/* a file: header of `hdr` bytes (zero), then `cmds` */
static size_t mk(uint8_t *b, uint32_t ver, size_t hdr, int set_dataoff, const uint8_t *cmds, size_t nc)
{
    memset(b, 0, hdr + nc);
    memcpy(b, "Vgm ", 4); w32(b, 8, ver);
    if (set_dataoff) w32(b, 0x34, (uint32_t)(hdr - 0x34));
    memcpy(b + hdr, cmds, nc);
    w32(b, 4, (uint32_t)(hdr + nc - 4));
    return hdr + nc;
}
static const uint8_t PSG[] = { 0x50, 0x9F, 0x61, 0x10, 0x00, 0x66 };                 /* PSG write, wait 16, end */
static const uint8_t OPM[] = { 0x54, 0x20, 0xC7, 0x61, 0x10, 0x00, 0x66 };           /* YM2151 write, wait, end */
static const uint8_t MIX[] = { 0x50, 0x9F, 0x54, 0x20, 0xC7, 0x66 };

static void dump(const char *name, const uint8_t *p, size_t n)
{
    pc_vgm_info_t i; int rc = pc_vgm_parse(p, n, &i);
    if (rc) { printf("%s: not a VGM header (%d)\n", name, rc); return; }
    printf("%s: v%x data@%x gd3@%x loop@%x samples %u/%u flags %u chips:", name, i.version, i.data_off, i.gd3_off, i.loop_off, i.total_samples, i.loop_samples, i.flags);
    for (int k = 0; k < i.n_chips; k++) printf(" %s@%u%s%s", pc_vgm_chip_label(&i.chips[k]), i.chips[k].clock, i.chips[k].flags & 1 ? "(dual)" : "", i.chips[k].flags & 4 ? "(legacy)" : "");
    pc_vgm_use_t u; pc_vgm_scan(p, n, &i, &u);
    char b[160]; pc_vgm_names(&i, u.used & i.chip_mask, b, sizeof b);
    printf("  | commands %u valid %d used-declared [%s] dacstream %d\n", u.commands, u.valid, b, u.dac_stream);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        for (int a = 1; a < argc; a++) {
            FILE *f = fopen(argv[a], "rb"); if (!f) { printf("%s: cannot open\n", argv[a]); continue; }
            static uint8_t buf[8 << 20]; size_t n = fread(buf, 1, sizeof buf, f); fclose(f);
            dump(argv[a], buf, n);
        }
        return 0;
    }
    static uint8_t f[0x400]; pc_vgm_info_t i; pc_vgm_use_t u; size_t n;
    const uint64_t GME = PC_VGM_BIT(PC_VGM_SN76489) | PC_VGM_BIT(PC_VGM_YM2413) | PC_VGM_BIT(PC_VGM_YM2612);

    /* errors */
    CHECK(pc_vgm_parse(f, 0, &i) == PC_VGM_E_SHORT);
    n = mk(f, 0x171, 0x100, 1, PSG, sizeof PSG);
    CHECK(pc_vgm_parse(f, 10, &i) == PC_VGM_E_SHORT);
    CHECK(pc_vgm_parse(f, 0x3F, &i) == PC_VGM_E_SHORT);
    f[0] = 'X'; CHECK(pc_vgm_parse(f, n, &i) == PC_VGM_E_IDENT); f[0] = 'V';
    w32(f, 8, 0x50); CHECK(pc_vgm_parse(f, n, &i) == PC_VGM_E_VERSION); w32(f, 8, 0x171);
    w32(f, 0x34, 4); CHECK(pc_vgm_parse(f, n, &i) == PC_VGM_E_DATAOFF); w32(f, 0x34, 0xCC);

    /* v1.71, YM2151 only */
    n = mk(f, 0x171, 0x100, 1, OPM, sizeof OPM); w32(f, 0x30, 3579545);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.version == 0x171 && i.data_off == 0x100 && i.n_chips == 1 && i.chips[0].id == PC_VGM_YM2151 && i.chips[0].clock == 3579545);
    CHECK(pc_vgm_unsupported(&i, GME, NULL) == PC_VGM_BIT(PC_VGM_YM2151));
    pc_vgm_scan(f, n, &i, &u); CHECK(u.valid && u.used == PC_VGM_BIT(PC_VGM_YM2151) && u.commands == 3);
    CHECK(pc_vgm_unsupported(&i, GME, &u) == PC_VGM_BIT(PC_VGM_YM2151));
    char nm[64]; pc_vgm_names(&i, pc_vgm_unsupported(&i, GME, &u), nm, sizeof nm); CHECK(!strcmp(nm, "YM2151"));

    /* YM2203 only, then PSG + YM2151, names list in header order */
    n = mk(f, 0x171, 0x100, 1, OPM, sizeof OPM); w32(f, 0x44, 3993600);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 1 && i.chips[0].id == PC_VGM_YM2203);
    n = mk(f, 0x171, 0x100, 1, MIX, sizeof MIX); w32(f, 0x0C, 3579545); w32(f, 0x30, 3579545); w32(f, 0x98, 1000000);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 3);
    pc_vgm_scan(f, n, &i, &u);
    uint64_t un = pc_vgm_unsupported(&i, GME, &u);
    CHECK(un == PC_VGM_BIT(PC_VGM_YM2151));                                  /* OKIM6295 is declared but never written: ignored */
    CHECK(pc_vgm_unsupported(&i, GME, NULL) == (PC_VGM_BIT(PC_VGM_YM2151) | PC_VGM_BIT(PC_VGM_OKIM6295)));
    pc_vgm_names(&i, pc_vgm_unsupported(&i, GME, NULL), nm, sizeof nm); CHECK(!strcmp(nm, "YM2151, OKIM6295"));
    pc_vgm_names(&i, pc_vgm_unsupported(&i, GME, NULL), nm, 8); CHECK(!strcmp(nm, "YM2151"));   /* too small: whole names only, terminated */
    pc_vgm_names(&i, 0, nm, 1); CHECK(nm[0] == 0);

    /* v1.50 with a 0x40 header: the command bytes after it are NOT clocks */
    uint8_t g[64]; memset(g, 0xA5, sizeof g);
    n = mk(f, 0x150, 0x40, 1, g, sizeof g); w32(f, 0x0C, 3579545);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 1 && i.chips[0].id == PC_VGM_SN76489 && i.data_off == 0x40);
    /* v1.50 without a data offset field set (0) */
    n = mk(f, 0x150, 0x40, 0, g, sizeof g); w32(f, 0x0C, 3579545);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 1 && i.data_off == 0x40);
    /* v1.50 with a long header full of garbage (every field after 0x40 is above its version) */
    n = mk(f, 0x150, 0x100, 1, PSG, sizeof PSG); memset(f + 0x38, 0xFF, 0x100 - 0x38); w32(f, 0x0C, 3579545);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 1 && i.chips[0].id == PC_VGM_SN76489);
    /* v1.51 sees 0x38..0x74 but not 0x80+ */
    w32(f, 0x34, 0xCC); w32(f, 8, 0x151); for (int o = 0x38; o < 0x100; o += 4) w32(f, o, 0x1000000 + o);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && (i.chip_mask & PC_VGM_BIT(PC_VGM_AY8910)) && !(i.chip_mask & PC_VGM_BIT(PC_VGM_GBDMG)) && !(i.chip_mask & PC_VGM_BIT(PC_VGM_GA20)));
    /* v1.71 with a short data offset: fields behind it are zero even when bytes are there */
    n = mk(f, 0x171, 0x100, 1, PSG, sizeof PSG); memset(f + 0x38, 0xFF, 0x100 - 0x38); w32(f, 0x0C, 3579545); w32(f, 0x34, 0x44 - 0x34 + 0x1C);   /* data at 0x60 */
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.data_off == 0x60 && (i.chip_mask & PC_VGM_BIT(PC_VGM_YM2203)) && !(i.chip_mask & PC_VGM_BIT(PC_VGM_AY8910)) && !(i.chip_mask & PC_VGM_BIT(PC_VGM_GA20)));
    /* header cut short: flagged, no read past the bytes given */
    n = mk(f, 0x171, 0x100, 1, PSG, sizeof PSG); w32(f, 0x30, 1); w32(f, 0xE0, 1);
    CHECK(pc_vgm_parse(f, 0x80, &i) == 0 && (i.flags & PC_VGM_I_TRUNC) && (i.chip_mask & PC_VGM_BIT(PC_VGM_YM2151)) && !(i.chip_mask & PC_VGM_BIT(PC_VGM_GA20)));
    CHECK(pc_vgm_parse(f, 0x40, &i) == 0 && (i.flags & PC_VGM_I_TRUNC));

    /* clock bits: dual, variant */
    n = mk(f, 0x171, 0x100, 1, PSG, sizeof PSG); w32(f, 0x0C, 0xC0000000u | 3579545); w32(f, 0x4C, 0x80000000u | 8000000);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 2 && i.chips[0].clock == 3579545 && (i.chips[0].flags & 3) == 3 && !strcmp(pc_vgm_chip_label(&i.chips[0]), "T6W28")
          && !strcmp(pc_vgm_chip_label(&i.chips[1]), "YM2610B") && (i.dual_mask == PC_VGM_BIT(PC_VGM_SN76489)));
    w32(f, 0x0C, 0x40000000u); w32(f, 0x4C, 0); CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 0);       /* only the dual bit: no chip */

    /* loop, gd3, totals, rate */
    n = mk(f, 0x171, 0x100, 1, PSG, sizeof PSG); w32(f, 0x14, 0x300 - 0x14); w32(f, 0x1C, 0x120 - 0x1C); w32(f, 0x18, 44100); w32(f, 0x20, 22050); w32(f, 0x24, 60);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.gd3_off == 0x300 && i.loop_off == 0x120 && i.total_samples == 44100 && i.loop_samples == 22050 && i.rate == 60 && i.eof_size == n);
    /* version < 1.01: no rate; version < 1.50: data at 0x40 even if the 0x34 bytes hold something */
    n = mk(f, 0x100, 0x40, 0, PSG, sizeof PSG); w32(f, 0x24, 60); w32(f, 0x34, 0x1234); w32(f, 0x0C, 3579545); w32(f, 0x2C, 7670454);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.rate == 0 && i.data_off == 0x40 && i.n_chips == 1);

    /* legacy shared FM clock (v1.00): the YM2413 field belongs to whichever FM chip is written */
    static const uint8_t L151[] = { 0x54, 0x20, 0xC7, 0x66 }, L152[] = { 0x52, 0x2B, 0x80, 0x66 }, L151b[] = { 0x51, 1, 2, 0x66 };
    n = mk(f, 0x100, 0x40, 0, L151, sizeof L151); w32(f, 0x10, 3579545);
    pc_vgm_parse(f, n, &i); CHECK(i.n_chips == 1 && i.chips[0].id == PC_VGM_YM2413 && (i.chips[0].flags & PC_VGM_CF_LEGACY_FM));
    pc_vgm_scan(f, n, &i, &u); CHECK(i.chips[0].id == PC_VGM_YM2151 && i.chip_mask == PC_VGM_BIT(PC_VGM_YM2151) && pc_vgm_unsupported(&i, GME, &u) == PC_VGM_BIT(PC_VGM_YM2151));
    n = mk(f, 0x101, 0x40, 0, L152, sizeof L152); w32(f, 0x10, 7670454);
    pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u); CHECK(i.chips[0].id == PC_VGM_YM2612 && pc_vgm_unsupported(&i, GME, &u) == 0);
    n = mk(f, 0x101, 0x40, 0, L151b, sizeof L151b); w32(f, 0x10, 3579545);
    pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u); CHECK(i.chips[0].id == PC_VGM_YM2413 && pc_vgm_unsupported(&i, GME, &u) == 0);

    /* scan: data blocks, waits, dual-chip commands, a truncated command, an unknown opcode */
    static uint8_t sc[64]; size_t k = 0;
    sc[k++] = 0x67; sc[k++] = 0x66; sc[k++] = 0x82; sc[k++] = 4; sc[k++] = 0; sc[k++] = 0; sc[k++] = 0; sc[k++] = 0x54; sc[k++] = 0x54; sc[k++] = 0x54; sc[k++] = 0x54;   /* block whose bytes look like YM2151 commands */
    sc[k++] = 0x62; sc[k++] = 0x73; sc[k++] = 0xA5; sc[k++] = 1; sc[k++] = 2;   /* 0xA5 = second YM2151? no: 0x55 + 0x50 = YM2203 #2 */
    sc[k++] = 0xB8; sc[k++] = 0x10; sc[k++] = 0x20; sc[k++] = 0x66;
    n = mk(f, 0x171, 0x100, 1, sc, k); w32(f, 0x44, 1); w32(f, 0x98, 1); w32(f, 0x30, 1);
    pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u);
    CHECK(u.valid && u.used == (PC_VGM_BIT(PC_VGM_YM2203) | PC_VGM_BIT(PC_VGM_OKIM6295)));       /* the block is skipped, YM2151 is declared but unused */
    CHECK(pc_vgm_unsupported(&i, GME, &u) == (PC_VGM_BIT(PC_VGM_YM2203) | PC_VGM_BIT(PC_VGM_OKIM6295)));
    pc_vgm_scan(f, n - 2, &i, &u); CHECK(!u.valid);                                              /* cut inside the OKIM6295 command */
    CHECK(pc_vgm_unsupported(&i, GME, &u) == (PC_VGM_BIT(PC_VGM_YM2203) | PC_VGM_BIT(PC_VGM_OKIM6295) | PC_VGM_BIT(PC_VGM_YM2151)));   /* invalid scan: every declared chip counts */
    static const uint8_t bad[] = { 0x50, 0x01, 0x6A, 0x54, 0, 0, 0x66 };                       /* 0x6A is reserved: the walk cannot go on */
    n = mk(f, 0x171, 0x100, 1, bad, sizeof bad); w32(f, 0x30, 1); pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u);
    CHECK(!u.valid && pc_vgm_unsupported(&i, GME, &u) == PC_VGM_BIT(PC_VGM_YM2151));
    static const uint8_t ds[] = { 0x90, 0, 2, 0, 0x2A, 0x91, 0, 0, 0, 0, 0x92, 0, 1, 2, 3, 4, 0x93, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x94, 0, 0x95, 0, 0, 0, 0, 0x66 };
    n = mk(f, 0x171, 0x100, 1, ds, sizeof ds); pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u); CHECK(u.valid && u.dac_stream && u.commands == 7);
    /* a stream that just stops (no end command) at a command boundary is fine; empty stream too */
    n = mk(f, 0x171, 0x100, 1, PSG, 2); pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u); CHECK(u.valid && u.used == PC_VGM_BIT(PC_VGM_SN76489));
    n = mk(f, 0x171, 0x100, 1, PSG, 0); pc_vgm_parse(f, n, &i); pc_vgm_scan(f, n, &i, &u); CHECK(u.valid && u.used == 0 && u.commands == 0);

    /* extra header (v1.70): a second chip and a chip that only it declares */
    n = mk(f, 0x171, 0x120, 1, PSG, sizeof PSG); w32(f, 0x0C, 3579545); w32(f, 0xBC, 0x100 - 0xBC);
    w32(f, 0x100, 12); w32(f, 0x104, 8); w32(f, 0x108, 0);                                      /* size, clock offset (relative to 0x104 -> 0x10C), volume offset */
    f[0x10C] = 2; f[0x10D] = 0x80 | PC_VGM_SN76489; w32(f, 0x10E, 3579545); f[0x112] = PC_VGM_OKIM6295; w32(f, 0x113, 1056000);
    CHECK(pc_vgm_parse(f, n, &i) == 0 && i.n_chips == 2 && (i.dual_mask == PC_VGM_BIT(PC_VGM_SN76489)) && (i.chip_mask & PC_VGM_BIT(PC_VGM_OKIM6295)));
    CHECK(pc_vgm_parse(f, 0x110, &i) == 0 && i.n_chips == 1);                                    /* extra header cut off: ignored, no overread */

    /* v1.72+: bytes in the unnamed clock area */
    n = mk(f, 0x172, 0x100, 1, PSG, sizeof PSG); w32(f, 0xE4, 5); CHECK(pc_vgm_parse(f, n, &i) == 0 && (i.flags & PC_VGM_I_UNKNOWN_CHIP));
    n = mk(f, 0x171, 0x100, 1, PSG, sizeof PSG); w32(f, 0xE4, 5); CHECK(pc_vgm_parse(f, n, &i) == 0 && !(i.flags & PC_VGM_I_UNKNOWN_CHIP));

    /* every id has a name, in field order */
    for (int c = 0; c < PC_VGM_CHIP_N; c++) CHECK(strcmp(pc_vgm_chip_name(c), "?") != 0);
    CHECK(!strcmp(pc_vgm_chip_name(PC_VGM_CHIP_N), "?"));

    printf("%d checks, %d failed\n", checks, fails);
    return fails != 0;
}
