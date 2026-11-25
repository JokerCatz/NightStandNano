// SPDX-License-Identifier: LGPL-2.1-or-later
#include "vgmgate.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pc/pc_vgmhdr.h"
#include "fmvgm.h"

/* What Vgm_Emu really plays (components/gme/gme/Vgm_Emu.cpp setup_fm): the SN76489 (also dual and T6W28), the YM2612 and the YM2413 (also dual). With both
 * FM chips declared it takes the YM2612 and silently drops the YM2413, so that pair is refused unless the YM2413 is never written. */
#define GME_SET (PC_VGM_BIT(PC_VGM_SN76489) | PC_VGM_BIT(PC_VGM_YM2413) | PC_VGM_BIT(PC_VGM_YM2612))

int nsn_vgm_gate(const uint8_t *vgm, size_t len, char *why, size_t n)
{
    pc_vgm_info_t info;
    if (why && n) why[0] = 0;
    if (pc_vgm_parse(vgm, len, &info) != PC_VGM_OK) return 0;
    uint64_t sup = GME_SET;
    if ((info.chip_mask & PC_VGM_BIT(PC_VGM_YM2612)) && (info.chip_mask & PC_VGM_BIT(PC_VGM_YM2413))) sup &= ~PC_VGM_BIT(PC_VGM_YM2413);
    bool legacy = false;
    for (int k = 0; k < info.n_chips; k++) if (info.chips[k].flags & PC_VGM_CF_LEGACY_FM) legacy = true;
    if (!(info.chip_mask & ~sup) && !legacy && !(info.flags & PC_VGM_I_UNKNOWN_CHIP)) return 0;       /* the common case: no scan */
    pc_vgm_use_t use;
    pc_vgm_scan(vgm, len, &info, &use);                                                               /* resolves a pre-1.10 shared FM clock, finds declared-but-unused chips */
    uint64_t bad = pc_vgm_unsupported(&info, sup, &use);
    if (!bad && (info.flags & PC_VGM_I_UNKNOWN_CHIP)) { if (why && n) snprintf(why, n, "unknown"); return -1; }
    if (!bad) return 0;
    if (why && n) pc_vgm_names(&info, bad, why, n);
    return -1;
}

/* Which chips a refused file needs, as the text of the refusal. For a mix the GME chip comes first ("SN76489 + YM2151 (mixed)"). */
int nsn_vgm_route(const uint8_t *vgm, size_t len, char *why, size_t n)
{
    if (why && n) why[0] = 0;
    uint64_t used = 0, missing = 0;
    int r = fmvgm_route(vgm, len, &used, &missing);
    pc_vgm_info_t info;
    if (r == 0) return NSN_VGM_FM;
    if (r == 2 || r == 3) {                                                     /* a mix of GME's chips and the FM chips, or a chip nobody here plays */
        if (pc_vgm_parse(vgm, len, &info) == PC_VGM_OK && why && n) {
            if (r == 2) {
                char a[24] = "", b[24] = "";
                pc_vgm_names(&info, used & ((uint64_t)1 << PC_VGM_SN76489 | (uint64_t)1 << PC_VGM_YM2413 | (uint64_t)1 << PC_VGM_YM2612), a, sizeof a);
                pc_vgm_names(&info, used & ((uint64_t)1 << PC_VGM_YM2151 | (uint64_t)1 << PC_VGM_YM2203), b, sizeof b);
                snprintf(why, n, "%s + %s (mixed)", a, b);
            } else pc_vgm_names(&info, missing ? missing : used, why, n);
        } else if (why && n) snprintf(why, n, "unknown");
        return -1;
    }
    return nsn_vgm_gate(vgm, len, why, n) == 0 ? NSN_VGM_GME : -1;             /* GME's chips only (or no chip written): the existing gate decides the rest (YM2612 + YM2413) */
}
