// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_dacmax.h"
#include "pc/pc_rate.h"

static const uint32_t TIERS[] = { 48000, 96000, 192000, 384000 };

int pc_dacmax_count(void)
{
    int n = 0;
    for (unsigned i = 0; i < sizeof TIERS / sizeof TIERS[0]; i++)
        if (TIERS[i] <= PC_AUDIO_MAX_RATE) n++;
    return n;                                            /* the first tier (48000) is always offered */
}

static uint32_t floor_tier(uint32_t v)                  /* the largest offered tier <= v, at least the first */
{
    int n = pc_dacmax_count(), best = 0;
    for (int i = 0; i < n; i++)
        if (TIERS[i] <= v) best = i;
    return TIERS[best];
}

uint32_t pc_dacmax_norm(int stored, uint32_t def)
{
    return floor_tier(stored > 0 ? (uint32_t)stored : def);
}

uint32_t pc_dacmax_step(uint32_t v, int detents)
{
    int n = pc_dacmax_count(), i = 0;
    uint32_t t = floor_tier(v);
    while (i < n - 1 && TIERS[i] != t) i++;
    return TIERS[(((i + detents) % n) + n) % n];
}

const char *pc_dacmax_label(uint32_t v)
{
    switch (v) {
    case 48000: return "48k";
    case 96000: return "96k";
    case 192000: return "192k";
    case 384000: return "384k";
    default: return "?";
    }
}
