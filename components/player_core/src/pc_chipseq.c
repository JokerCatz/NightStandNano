// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_chipseq.h"
#include <string.h>
#include <strings.h>

pc_chippos_t pc_chip_next(pc_chippos_t cur, int sub_count, int file_count, pc_seq_t mode, uint32_t rnd)
{
    if (cur.sub + 1 < sub_count) return (pc_chippos_t){ cur.file, cur.sub + 1 };
    if (file_count <= 1) return (pc_chippos_t){ 0, 0 };
    if (mode == PC_SEQ_LOOP) return (pc_chippos_t){ (cur.file + 1) % file_count, 0 };
    int pick = (int)(rnd % (uint32_t)(file_count - 1));   /* uniform over the other files */
    return (pc_chippos_t){ pick >= cur.file ? pick + 1 : pick, 0 };
}

int32_t pc_chip_length_ms(int32_t len) { return len >= 0 ? len : 180000; }

/* THE per-extension table of the two module families and their format tier (owner 2026-11-18): one row per suffix, one cell per backend that can play it.
 * opl = AdPlug (DOS AdLib / OPL music, components/libadplug), xmp = libxmp (tracker modules, components/libxmp). Cell: 0 = this backend does not take the
 * suffix, 1 = STABLE (plays always), 2 = EXPERIMENTAL (plays only with the setting 實驗性格式 on; with it off the library scan does not even read the file,
 * and opening it is refused with a reason). To promote a reader after more fuzzing: change its 2 to 1 here, nothing else.
 * A suffix with both cells set (.imf .dtm .ims) is tried in the column order: AdPlug first (a content probe: its open refuses what is not its format),
 * then libxmp (pc_backend_chain). Experimental today = the AdPlug players of OPLFUZZ-report.md that were slow to scan (a2m a2t d00 herad lds) or gave most
 * findings (u6m family: sa2 dtm plx rix xms; protrack family: mtr sat dfm mad; mid family: laa sci cmf mdi; pis), and the libxmp format whose packer depackers
 * XMPFULL-report.md left unseeded (.ice: ice1 / ice2 header-only). The other unseeded ProWizard packers have no suffix (they are found by content in any
 * module-like file, Amiga "mod.name" naming), so a suffix cannot gate them. */
typedef struct { const char *ext; uint8_t opl, xmp; } fmt_row_t;
static const fmt_row_t FMT[] = {
    { ".rad", 1, 0 }, { ".hsc", 1, 0 }, { ".hsp", 1, 0 }, { ".imf", 1, 1 }, { ".wlf", 1, 0 }, { ".a2m", 2, 0 }, { ".a2t", 2, 0 }, { ".amd", 1, 0 },
    { ".xms", 2, 0 }, { ".bam", 1, 0 }, { ".cmf", 2, 0 }, { ".adl", 1, 0 }, { ".d00", 2, 0 }, { ".dfm", 2, 0 }, { ".ksm", 1, 0 }, { ".mad", 2, 0 },
    { ".mus", 1, 0 }, { ".mdy", 1, 0 }, { ".ims", 1, 1 }, { ".mdi", 2, 0 }, { ".sci", 2, 0 }, { ".laa", 2, 0 }, { ".mkj", 1, 0 }, { ".cff", 1, 0 },
    { ".dmo", 1, 0 }, { ".dtm", 2, 1 }, { ".mtk", 1, 0 }, { ".mtr", 2, 0 }, { ".rac", 1, 0 }, { ".sat", 2, 0 }, { ".sa2", 2, 0 }, { ".xad", 1, 0 },
    { ".bmf", 1, 0 }, { ".lds", 2, 0 }, { ".plx", 2, 0 }, { ".rol", 1, 0 }, { ".xsm", 1, 0 }, { ".dro", 1, 0 }, { ".pis", 2, 0 }, { ".msc", 1, 0 },
    { ".rix", 2, 0 }, { ".mkf", 1, 0 }, { ".got", 1, 0 }, { ".sop", 1, 0 }, { ".hsq", 2, 0 }, { ".sqx", 2, 0 }, { ".sdb", 2, 0 }, { ".agd", 2, 0 },
    { ".ha2", 2, 0 }, { ".sng", 1, 0 }, { ".mod", 0, 1 }, { ".xm", 0, 1 }, { ".s3m", 0, 1 }, { ".it", 0, 1 }, { ".m15", 0, 1 }, { ".nt", 0, 1 },
    { ".flx", 0, 1 }, { ".wow", 0, 1 }, { ".st", 0, 1 }, { ".stk", 0, 1 }, { ".stm", 0, 1 }, { ".stx", 0, 1 }, { ".mtm", 0, 1 }, { ".ice", 0, 2 },
    { ".mtn", 0, 1 }, { ".ptm", 0, 1 }, { ".mdl", 0, 1 }, { ".ult", 0, 1 }, { ".liq", 0, 1 }, { ".psm", 0, 1 }, { ".ps16", 0, 1 }, { ".j2b", 0, 1 },
    { ".amf", 0, 1 }, { ".gdm", 0, 1 }, { ".med", 0, 1 }, { ".mmd0", 0, 1 }, { ".mmd1", 0, 1 }, { ".mmd2", 0, 1 }, { ".mmd3", 0, 1 },
    { ".mmdc", 0, 1 }, { ".rtm", 0, 1 }, { ".pt3", 0, 1 }, { ".mgt", 0, 1 }, { ".dsym", 0, 1 }, { ".digi", 0, 1 }, { ".dbm", 0, 1 },
    { ".emod", 0, 1 }, { ".okt", 0, 1 }, { ".sfx", 0, 1 }, { ".far", 0, 1 }, { ".umx", 0, 1 }, { ".hmn", 0, 1 }, { ".stim", 0, 1 }, { ".coco", 0, 1 },
    { ".669", 0, 1 }, { ".fnk", 0, 1 }, { ".abk", 0, 1 }, { ".xmf", 0, 1 }, { ".flt", 0, 1 },
};

static const fmt_row_t *fmt_row(const char *ext)
{
    if (!ext) return NULL;
    for (size_t i = 0; i < sizeof FMT / sizeof *FMT; i++)
        if (!strcasecmp(ext, FMT[i].ext)) return &FMT[i];
    return NULL;
}

static bool (*s_exp_src)(void);
void pc_tier_set_source(bool (*experimental_on)(void)) { s_exp_src = experimental_on; }
bool pc_tier_experimental_on(void) { return s_exp_src && s_exp_src(); }

bool pc_xmp_ext(const char *ext) { const fmt_row_t *r = fmt_row(ext); return r && r->xmp; }
bool pc_opl_ext(const char *ext) { const fmt_row_t *r = fmt_row(ext); return r && r->opl; }
int pc_ext_backends(const char *ext, int *opl_cell, int *xmp_cell)
{
    const fmt_row_t *r = fmt_row(ext);
    if (opl_cell) *opl_cell = r ? r->opl : 0;
    if (xmp_cell) *xmp_cell = r ? r->xmp : 0;
    return r ? (r->opl != 0) + (r->xmp != 0) : 0;
}
bool pc_tier_allowed(int cell) { return cell == PC_TIER_STABLE || (cell == PC_TIER_EXPERIMENTAL && pc_tier_experimental_on()); }
bool pc_ext_has_gated(const char *ext)
{
    int o, x;
    if (!pc_ext_backends(ext, &o, &x)) return false;
    return (o == PC_TIER_EXPERIMENTAL || x == PC_TIER_EXPERIMENTAL) && !pc_tier_experimental_on();
}
bool pc_ext_blocked(const char *ext)
{
    int o, x;
    if (!pc_ext_backends(ext, &o, &x)) return false;
    return !(o && pc_tier_allowed(o)) && !(x && pc_tier_allowed(x));
}
bool pc_ext_scan_blind(const char *ext) { return pc_ext_has_gated(ext); }

bool pc_chip_supported_ext(const char *name)
{
    static const char *const ext[] = { "nsf", "nsfe", "gbs", "gym", "hes", "kss", "sap", "spc", "vgm", "vgz", "ay", "ahx", "hvl", "p8", "ym", "sid" };
    size_t nl = strlen(name);
    if (nl > 7 && !strcasecmp(name + nl - 7, ".p8.png")) return true;     /* a PICO-8 cart image (the last dot says "png") */
    const char *dot = strrchr(name, '.');
    if (!dot || dot == name || strchr(dot, '/')) return false;
    for (size_t i = 0; i < sizeof ext / sizeof *ext; i++)
        if (!strcasecmp(dot + 1, ext[i])) return true;
    return pc_opl_ext(dot);
}
