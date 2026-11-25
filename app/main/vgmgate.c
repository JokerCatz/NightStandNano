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

/* The partial-playback policy (owner: "support what can be supported; skip what cannot"). A file may need chips that neither GME nor the FM backend plays. What happens depends on what the chip
 * is for, which is decided per chip class here (one table, the reasoning in the comments):
 *   CLS_SAMPLE  : a sample / PCM player (Sega PCM, RF5C68 / RF5C164, MultiPCM, uPD7759, OKIM6258, K053260, K054539, C140, C352, QSound, YMZ280B, GA20, X1-010, ES5503 / 5506, PWM). In a
 *                 game soundtrack these play drums, voices and effects; the tune is carried by the FM / PSG chips next to them. Leaving them out loses a layer and keeps the music: partial.
 *                 With `partial on` (default) the file plays without them and the player logs one "partial <file>: missing <chips>" event; with `partial off` it is refused as before.
 *   CLS_MELODIC : everything that carries the melody or the whole mix: unsupported FM chips (YM2413 next to other chips, OPL family, YMF271, SCSP), wavetable / PSG chips (K051649, HuC6280,
 *                 NES APU, Game Boy, POKEY, WonderSwan, VSU, SAA1099). Without them the file is not the song: refused, always.
 * A file with a sample chip and nothing else (CPS-2's QSound alone) has nothing to play and is refused too: partial needs at least one chip that is played. */
typedef enum { CLS_MELODIC = 0, CLS_SAMPLE = 1 } vgm_class_t;
static vgm_class_t chip_class(int id)
{
    switch (id) {
    case PC_VGM_SEGAPCM: case PC_VGM_RF5C68: case PC_VGM_RF5C164: case PC_VGM_PWM: case PC_VGM_MULTIPCM: case PC_VGM_UPD7759: case PC_VGM_OKIM6258: case PC_VGM_K054539:
    case PC_VGM_C140: case PC_VGM_K053260: case PC_VGM_QSOUND: case PC_VGM_ES5503: case PC_VGM_ES5506: case PC_VGM_X1010: case PC_VGM_C352: case PC_VGM_GA20: case PC_VGM_YMZ280B:
        return CLS_SAMPLE;
    default: return CLS_MELODIC;
    }
}
static uint64_t sample_mask(void)
{
    static uint64_t m;
    if (!m) for (int id = 0; id < PC_VGM_CHIP_N; id++) if (chip_class(id) == CLS_SAMPLE) m |= PC_VGM_BIT(id);
    return m;
}

int nsn_vgm_gate(const uint8_t *vgm, size_t len, char *why, size_t n, uint64_t extra)
{
    pc_vgm_info_t info;
    if (why && n) why[0] = 0;
    if (pc_vgm_parse(vgm, len, &info) != PC_VGM_OK) return 0;
    uint64_t sup = GME_SET | extra;
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

/* Which backend plays a .vgm / .vgz (and what is left out), or the reason for a refusal. */
int nsn_vgm_route(const uint8_t *vgm, size_t len, char *why, size_t n, int partial_ok, nsn_vgm_partial_t *pt)
{
    if (why && n) why[0] = 0;
    if (pt) { pt->drop = 0; pt->names[0] = 0; }
    uint64_t used = 0, missing = 0;
    int r = fmvgm_route(vgm, len, &used, &missing);
    pc_vgm_info_t info;
    if (r == 0) return NSN_VGM_FM;
    if (r == 2) {                                                               /* a YM2413 next to chips GME lacks: neither backend plays both */
        if (pc_vgm_parse(vgm, len, &info) == PC_VGM_OK && why && n) {
            char a[24] = "", b[24] = "";
            pc_vgm_names(&info, used & PC_VGM_BIT(PC_VGM_YM2413), a, sizeof a);
            pc_vgm_names(&info, used & ~PC_VGM_BIT(PC_VGM_YM2413), b, sizeof b);
            snprintf(why, n, "%s + %s (mixed)", a, b);
        } else if (why && n) snprintf(why, n, "unknown");
        return -1;
    }
    if (r == 3) {                                                               /* a chip nobody here plays */
        bool all_sample = missing && !(missing & ~sample_mask());
        uint64_t rest = used & ~missing;                                         /* what is played */
        if (partial_ok && all_sample && rest && pt) {
            pt->drop = missing;
            if (pc_vgm_parse(vgm, len, &info) == PC_VGM_OK) pc_vgm_names(&info, missing, pt->names, sizeof pt->names);
            if (!(rest & ~GME_SET)) return nsn_vgm_gate(vgm, len, why, n, missing) == 0 ? NSN_VGM_GME : -1;   /* GME plays the rest (it ignores the chips it does not know) */
            return NSN_VGM_FM;
        }
        if (pc_vgm_parse(vgm, len, &info) == PC_VGM_OK && why && n) pc_vgm_names(&info, missing ? missing : used, why, n);
        else if (why && n) snprintf(why, n, "unknown");
        return -1;
    }
    return nsn_vgm_gate(vgm, len, why, n, 0) == 0 ? NSN_VGM_GME : -1;          /* GME's chips only (or no chip written): the gate decides the rest (YM2612 + YM2413) */
}
