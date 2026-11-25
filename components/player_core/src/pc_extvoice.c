// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_extvoice.h"
#include <string.h>
#include "pc/pc_vmute.h"
#if defined(__has_include)
#if __has_include("esp_attr.h")
#include "esp_attr.h"
#endif
#endif
#ifndef EXT_RAM_BSS_ATTR
#define EXT_RAM_BSS_ATTR
#endif

EXT_RAM_BSS_ATTR static pc_voices_t s_v;   /* 322 bytes, PSRAM */
static const void *s_owner;

pc_voices_t *pc_extvoice_take(const void *owner, int n)
{
    if (s_owner && s_owner != owner) return NULL;   /* held (a crossfade renders two handles in one block): the owner keeps it, no reset every block */
    pc_voice_publish_n(&s_v, 0);
    s_owner = owner;
    if (n < 0) n = 0;
    if (n > PC_VOICES_MAX) n = PC_VOICES_MAX;
    memset(s_v.level, 0, sizeof s_v.level);
    for (int i = 0; i < PC_VOICES_MAX; i++) { pc_voice_clear_extra(&s_v, i); s_v.name[i][0] = 0; }
    for (int i = 0; i < n; i++) pc_voice_short(NULL, i, s_v.name[i]);
    pc_vmute_reset();                       /* a new source: every voice on */
    pc_voice_publish_n(&s_v, (uint8_t)n);
    return &s_v;
}
pc_voices_t *pc_extvoice_get(const void *owner) { return owner && owner == s_owner ? &s_v : NULL; }
void pc_extvoice_release(const void *owner)
{
    if (!owner || owner != s_owner) return;
    pc_voice_publish_n(&s_v, 0);
    s_owner = NULL;
    pc_vmute_reset();
}
int pc_extvoice_read(pc_voices_t *out) { return pc_voice_read(&s_v, out); }
