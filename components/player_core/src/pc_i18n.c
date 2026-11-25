// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_i18n.h"
#include <stdlib.h>
#include <string.h>

typedef struct { const char *tw, *cn; } pc_i18n_pair_t;
extern const pc_i18n_pair_t PC_I18N_DATA[];   /* GENERATED (pc_i18n_data.c), sorted by strcmp on tw */
extern const unsigned PC_I18N_DATA_N;

pc_lang_t pc_lang_norm(int v) { return v >= 0 && v < PC_LANG_COUNT ? (pc_lang_t)v : PC_LANG_TW; }

const char *pc_lang_code(pc_lang_t l) { return l == PC_LANG_CN ? "cn" : l == PC_LANG_EN ? "en" : "tw"; }

int pc_lang_parse(const char *s)
{
    if (!s) return -1;
    for (int i = 0; i < PC_LANG_COUNT; i++) if (!strcmp(s, pc_lang_code((pc_lang_t)i)) || (s[0] == '0' + i && !s[1])) return i;
    return -1;
}

bool pc_lang_zh(pc_lang_t l, bool cjk) { return cjk && l != PC_LANG_EN; }

const char *pc_i18n_text(pc_lang_t l, bool cjk, const char *tw, const char *cn, const char *en)
{
    if (!pc_lang_zh(l, cjk)) return en;
    return l == PC_LANG_CN && cn ? cn : tw;
}

__attribute__((weak)) pc_lang_t pc_i18n_cur(void) { return PC_LANG_TW; }

const char *pc_i18n_cn(const char *tw)
{
    if (!tw) return NULL;
    unsigned lo = 0, hi = PC_I18N_DATA_N;
    while (lo < hi) {
        unsigned mid = (lo + hi) / 2;
        int c = strcmp(tw, PC_I18N_DATA[mid].tw);
        if (!c) return PC_I18N_DATA[mid].cn;
        if (c < 0) hi = mid; else lo = mid + 1;
    }
    return NULL;
}

const char *pc_tz(const char *tw)
{
    if (pc_i18n_cur() != PC_LANG_CN) return tw;
    const char *cn = pc_i18n_cn(tw);
    return cn ? cn : tw;
}
