/* The player table of AdPlug 2.4's adplug.cpp (CAdPlug::allplayers) as a plain array, without the std::list / CPlayerDesc machinery (their
 * static initialisers allocate at boot) and without Westwood ADL (adl.cpp: its header carries a GPL-2+ block) and JBM (jbm.cpp: GPL).
 * Same lookup order as CAdPlug::factory: players whose extension matches first, then every player in table order. */
#include <string.h>
#include <strings.h>
#include <string>
#include "players_table.h"
#include "hsc.h"
#include "amd.h"
#include "a2m.h"
#include "a2m-v2.h"
#include "imf.h"
#include "sng.h"
#include "adtrack.h"
#include "bam.h"
#include "cmf.h"
#include "d00.h"
#include "dfm.h"
#include "hsp.h"
#include "ksm.h"
#include "mad.h"
#include "mid.h"
#include "mkj.h"
#include "cff.h"
#include "dmo.h"
#include "s3m.h"
#include "dtm.h"
#include "fmc.h"
#include "mtk.h"
#include "rad2.h"
#include "raw.h"
#include "sa2.h"
#include "bmf.h"
#include "flash.h"
#include "hybrid.h"
#include "hyp.h"
#include "psi.h"
#include "rat.h"
#include "lds.h"
#include "u6m.h"
#include "rol.h"
#include "xsm.h"
#include "dro.h"
#include "dro2.h"
#include "msc.h"
#include "rix.h"
#include "got.h"
#include "mus.h"
#include "mdi.h"
#include "cmfmcsop.h"
#include "vgm.h"
#include "sop.h"
#include "herad.h"
#include "coktel.h"
#include "pis.h"
#include "mtr.h"
#include "plx.h"

typedef CPlayer *(*factory_fn)(Copl *);
static const struct { factory_fn f; const char *ext; } PLAYERS[] = {
    { ChscPlayer::factory, ".hsc" },
    { CsngPlayer::factory, ".sng" },
    { CimfPlayer::factory, ".imf;.wlf;.adlib" },
    { Ca2mLoader::factory, ".a2m" },
    { Ca2mv2Player::factory, ".a2m;.a2t" },
    { CadtrackLoader::factory, ".sng" },
    { CamdLoader::factory, ".amd" },
    { CamdLoader::factory, ".xms" },
    { CbamPlayer::factory, ".bam" },
    { CcmfPlayer::factory, ".cmf" },
    { CcoktelPlayer::factory, ".adl" },
    { Cd00Player::factory, ".d00" },
    { CdfmLoader::factory, ".dfm" },
    { ChspLoader::factory, ".hsp" },
    { CksmPlayer::factory, ".ksm" },
    { CmadLoader::factory, ".mad" },
    { CmusPlayer::factory, ".mus;.mdy;.ims" },
    { CmdiPlayer::factory, ".mdi" },
    { CmidPlayer::factory, ".mid;.sci;.laa" },
    { CmkjPlayer::factory, ".mkj" },
    { CcffLoader::factory, ".cff" },
    { CdmoLoader::factory, ".dmo" },
    { Cs3mPlayer::factory, ".s3m" },
    { CdtmLoader::factory, ".dtm" },
    { CfmcLoader::factory, ".sng" },
    { CmtkLoader::factory, ".mtk" },
    { CmtrLoader::factory, ".mtr" },
    { Crad2Player::factory, ".rad" },
    { CrawPlayer::factory, ".rac;.raw" },
    { Csa2Loader::factory, ".sat;.sa2" },
    { CxadbmfPlayer::factory, ".xad;.bmf" },
    { CxadflashPlayer::factory, ".xad" },
    { CxadhybridPlayer::factory, ".xad" },
    { CxadhypPlayer::factory, ".xad" },
    { CxadpsiPlayer::factory, ".xad" },
    { CxadratPlayer::factory, ".xad" },
    { CldsPlayer::factory, ".lds" },
    { CplxPlayer::factory, ".plx" },
    { Cu6mPlayer::factory, ".m" },
    { CrolPlayer::factory, ".rol" },
    { CxsmPlayer::factory, ".xsm" },
    { CdroPlayer::factory, ".dro" },
    { Cdro2Player::factory, ".dro" },
    { CpisPlayer::factory, ".pis" },
    { CmscPlayer::factory, ".msc" },
    { CrixPlayer::factory, ".rix;.mkf" },
    { CgotPlayer::factory, ".got" },
    { CcmfmacsoperaPlayer::factory, ".cmf" },
    { CvgmPlayer::factory, ".vgm;.vgz" },
    { CsopPlayer::factory, ".sop" },
    { CheradPlayer::factory, ".hsq;.sqx;.sdb;.agd;.ha2" },
};

static bool ext_matches(const char *list, const char *ext)
{
    size_t el = strlen(ext);
    if (!el) return false;
    for (const char *p = list; *p;) {
        const char *q = strchr(p, ';');
        size_t l = q ? (size_t)(q - p) : strlen(p);
        if (l == el && !strncasecmp(p, ext, el)) return true;
        if (!q) break;
        p = q + 1;
    }
    return false;
}

CPlayer *adplug_players_factory(Copl *opl, const char *ext, const std::string &fn, const CFileProvider &fp)
{
    const int n = (int)(sizeof PLAYERS / sizeof *PLAYERS);
    for (int i = 0; i < n; i++)
        if (ext_matches(PLAYERS[i].ext, ext)) {
            CPlayer *p = PLAYERS[i].f(opl);
            if (p) { if (p->load(fn, fp)) return p; delete p; }
        }
    for (int i = 0; i < n; i++) {
        CPlayer *p = PLAYERS[i].f(opl);
        if (p) { if (p->load(fn, fp)) return p; delete p; }
    }
    return 0;
}
