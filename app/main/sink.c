// SPDX-License-Identifier: LGPL-2.1-or-later
#include "sink.h"
#include <string.h>

static const nsn_sink_t *s_sinks[NSN_SINK_MAX];
static int s_n;

int nsn_sink_register(const nsn_sink_t *s)
{
    if (!s || s_n >= NSN_SINK_MAX) return 0;
    s_sinks[s_n++] = s;
    return s_n;
}

const nsn_sink_t *nsn_sink_get(int id) { return id >= 1 && id <= s_n ? s_sinks[id - 1] : NULL; }
int nsn_sink_count(void) { return s_n; }

int nsn_sink_by_name(const char *name)
{
    for (int i = 0; i < s_n; i++) if (!strcmp(s_sinks[i]->name, name)) return i + 1;
    return 0;
}

const char *nsn_sink_name(int id) { const nsn_sink_t *s = nsn_sink_get(id); return s ? s->name : "none"; }

int nsn_sink_auto(void)
{
    int best = 0;
    for (int i = 0; i < s_n; i++) {
        const nsn_sink_t *s = s_sinks[i];
        if (!s->auto_rank || (best && s->auto_rank >= s_sinks[best - 1]->auto_rank)) continue;
        if (s->present()) best = i + 1;
    }
    return best;
}
