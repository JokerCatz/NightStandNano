// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_preset.h"
#include "pc/pc_fxapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toml.h"

/* a kind is supported when the fx registry has an app of that id (pc_fxapp_find) */

void pc_preset_set_alloc(void *(*alloc)(size_t), void (*release)(void *)) { toml_set_memutil(alloc, release); }

float pc_node_param(const pc_node_t *n, const char *id, float def)
{
    for (int i = 0; i < n->np; i++)
        if (!strcmp(n->pname[i], id)) return n->pval[i];
    return def;
}

static const char *skip_ws(const char *p) { while (*p == ' ' || *p == '\t') p++; return p; }

int pc_autoeq_parse(const char *text, pc_eq_band_t *b, int max, float *preamp)
{
    int n = 0;
    *preamp = 0;
    for (const char *line = text; line && *line; line = strchr(line, '\n') ? strchr(line, '\n') + 1 : NULL) {
        const char *p = skip_ws(line);
        float v, fc, g, q;
        char onoff[4], type[4];
        if (sscanf(p, "Preamp: %f dB", &v) == 1) { *preamp = v; continue; }
        if (strncmp(p, "Filter", 6)) continue;
        const char *c = strchr(p, ':');
        if (!c || n >= max) continue;
        if (sscanf(c + 1, " %3s %3s Fc %f Hz Gain %f dB Q %f", onoff, type, &fc, &g, &q) != 5 || strcmp(onoff, "ON")) continue;
        char t = !strcmp(type, "PK") ? 'P' : !strcmp(type, "LSC") || !strcmp(type, "LS") ? 'L' : !strcmp(type, "HSC") || !strcmp(type, "HS") ? 'H' : 0;
        if (!t || fc <= 0 || q <= 0) continue;
        b[n++] = (pc_eq_band_t){ .fc = fc, .gain_db = g, .q = q, .type = t };
    }
    return n;
}

static void note_unsupported(pc_preset_t *p, const char *what)
{
    if (p->nunsup < 8) snprintf(p->unsupported[p->nunsup++], sizeof p->unsupported[0], "%s", what);
}

static bool has_nonempty(toml_table_t *t, const char *key)
{
    if (!toml_key_exists(t, key)) return false;
    toml_datum_t s = toml_string_in(t, key);
    if (s.ok) { bool ne = s.u.s[0] != 0; free(s.u.s); return ne; }
    return true;                                     /* arrays etc. */
}

static void parse_node(toml_table_t *t, pc_node_t *n, pc_preset_t *p)
{
    memset(n, 0, sizeof *n);
    n->enabled = true;
    n->mix = 1.0f;
    toml_datum_t d = toml_string_in(t, "kind");
    if (d.ok) { snprintf(n->kind, sizeof n->kind, "%s", d.u.s); free(d.u.s); }
    d = toml_bool_in(t, "enabled");
    if (d.ok) n->enabled = d.u.b;
    d = toml_double_in(t, "mix");
    if (d.ok) n->mix = (float)d.u.d;
    toml_table_t *pt = toml_table_in(t, "params");
    for (int i = 0; pt && n->np < PC_NODE_MAX_PARAMS; i++) {
        const char *k = toml_key_in(pt, i);
        if (!k) break;
        double v;
        toml_datum_t x = toml_double_in(pt, k);
        if (x.ok) v = x.u.d;
        else if ((x = toml_int_in(pt, k)).ok) v = (double)x.u.i;
        else if ((x = toml_bool_in(pt, k)).ok) v = x.u.b ? 1.0 : 0.0;
        else continue;
        snprintf(n->pname[n->np], sizeof n->pname[0], "%s", k);
        n->pval[n->np++] = (float)v;
    }
    d = toml_string_in(t, "equalizer");
    if (d.ok) { n->neq = pc_autoeq_parse(d.u.s, n->eq, PC_EQ_MAX, &n->eq_preamp_db); free(d.u.s); }
    bool known = pc_fxapp_find(n->kind) >= 0;
    const char *why = NULL;
    if (has_nonempty(t, "only_for")) why = "only_for";
    else if (has_nonempty(t, "group")) why = "group";
    else if (has_nonempty(t, "of")) why = "of";
    else if (toml_key_exists(t, "impulse")) why = "impulse";
    n->supported = known && !why;
    if (!known) note_unsupported(p, n->kind);
    else if (why) {
        char s[40];
        snprintf(s, sizeof s, "%s (%s)", n->kind, why);
        note_unsupported(p, s);
    }
}

static int parse_list(toml_table_t *root, const char *key, pc_node_t *out, int max, pc_preset_t *p)
{
    toml_array_t *a = toml_array_in(root, key);
    int n = 0;
    for (int i = 0; a && i < toml_array_nelem(a) && n < max; i++) {
        toml_table_t *t = toml_table_at(a, i);
        if (t) parse_node(t, &out[n++], p);
    }
    return n;
}

int pc_preset_parse(const char *text, pc_preset_t *p)
{
    memset(p, 0, sizeof *p);
    char *buf = strdup(text), err[128];
    if (!buf) return -1;
    toml_table_t *root = toml_parse(buf, err, sizeof err);
    free(buf);
    if (!root) return -1;
    toml_datum_t f = toml_int_in(root, "format");
    if (!f.ok || f.u.i != 1) { toml_free(root); return -2; }
    toml_datum_t nm = toml_string_in(root, "name");
    if (nm.ok) { snprintf(p->name, sizeof p->name, "%s", nm.u.s); free(nm.u.s); }
    p->nmusic = parse_list(root, "music", p->music, 16, p);
    p->nmaster = parse_list(root, "master", p->master, 8, p);
    toml_array_t *sa = toml_array_in(root, "sends");
    for (int i = 0; sa && i < toml_array_nelem(sa) && p->nsends < 4; i++) {
        toml_table_t *t = toml_table_at(sa, i);
        toml_array_t *nodes = t ? toml_array_in(t, "nodes") : NULL;
        toml_table_t *n0 = nodes && toml_array_nelem(nodes) > 0 ? toml_table_at(nodes, 0) : NULL;
        if (!n0) continue;
        typeof(p->sends[0]) *s = &p->sends[p->nsends++];
        toml_datum_t d = toml_int_in(t, "from");
        s->from = d.ok ? (int)d.u.i : 0;
        d = toml_int_in(t, "to");
        s->to = d.ok ? (int)d.u.i : -1;
        d = toml_double_in(t, "level");
        s->level = d.ok ? (float)d.u.d : 0.0f;
        parse_node(n0, &s->node, p);
        d = toml_bool_in(t, "on");
        if (d.ok && !d.u.b) s->node.enabled = false;
        for (int k = 1; k < toml_array_nelem(nodes); k++) note_unsupported(p, "send chain");
    }
    toml_free(root);
    return 0;
}
