// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
/* The UI language (i18n phase A): 繁中 (default), 簡中, English. A text is a string id (T(S_xxx), tools/gen/i18n/strings.tsv): three texts, one per language.
 * No CJK font: English whatever is chosen. */
typedef enum { PC_LANG_TW = 0, PC_LANG_CN = 1, PC_LANG_EN = 2, PC_LANG_COUNT } pc_lang_t;   /* the values of the cfg key "lang" (pc_settings) */

pc_lang_t pc_lang_norm(int v);                                 /* out of range: 繁中 (zh-TW) */
const char *pc_lang_code(pc_lang_t l);                         /* "tw" | "cn" | "en" */
int pc_lang_parse(const char *s);                              /* "tw" "cn" "en" (also "0".."2"): the language, else -1 */
bool pc_lang_zh(pc_lang_t l, bool cjk);                        /* Chinese text wanted: a CJK font is there and the language is not English */
const char *pc_i18n_text(pc_lang_t l, bool cjk, const char *tw, const char *cn, const char *en);   /* the one of the three for the language (en without a CJK font; a NULL cn: tw) */

/* Data texts (phase B): a fixed table keyed by the zh-TW literal (tools/gen/i18n/data.tsv -> src/pc_i18n_data.c) for the texts that live in tables, struct fields and
 * formats, where no id fits. TZ("zh-TW literal") is the literal itself in 繁中 and the 簡中 text in 簡中: put it where the zh branch is chosen
 * (zh ? TZ("文字") : "text"). The current language comes from pc_i18n_cur(), which the app overrides (a weak default says 繁中). */
pc_lang_t pc_i18n_cur(void);
const char *pc_i18n_cn(const char *zh_tw);   /* the 簡中 text of a data literal, NULL when there is none */
const char *pc_tz(const char *zh_tw);
#define TZ(zh_tw) pc_tz(zh_tw)
