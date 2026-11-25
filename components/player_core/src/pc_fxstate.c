// SPDX-License-Identifier: LGPL-2.1-or-later
/* The sound as device state: value table, kv form (fx.txt), import of a chipchill preset, export (spec §4, §7). */
#include "pc/pc_fxstate.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const pc_fxapp_t *app_at(int i)
{
    const pc_fxapp_t *const *a;
    int n = pc_fxapps(&a);
    return i >= 0 && i < n ? a[i] : NULL;
}
static int napps(void) { return pc_fxapps(NULL); }
/* an app of a loop over 0..napps()-1: no range check (pc_fxstate_clamp checks the index it is given) */
static const pc_fxapp_t *A(int i)
{
    const pc_fxapp_t *const *a;
    pc_fxapps(&a);
    return a[i];
}
static int pfind(const pc_fxapp_t *ap, const char *id)
{
    for (int p = 0; p < ap->nparam; p++)
        if (!strcmp(ap->param[p].id, id)) return p;
    return -1;
}
/* the equalizer's band rows: type, fc, gain, q per band, the first PC_FXP_BAND parameter starts band 0 */
static int band_base(const pc_fxapp_t *ap)
{
    for (int p = 0; p < ap->nparam; p++)
        if (ap->param[p].flags & PC_FXP_BAND) return p;
    return -1;
}
static int band_count(const pc_fxapp_t *ap)
{
    int b = band_base(ap);
    return b < 0 ? 0 : (ap->nparam - b) / 4;
}
static const char BAND_LETTER[3] = { 'P', 'L', 'H' };

void pc_fxstate_defaults(pc_fxstate_t *s)
{
    memset(s, 0, sizeof *s);
    for (int i = 0; i < PC_FXAPP_MAX; i++) s->mix[i] = 1.0f;
    for (int i = 0; i < napps(); i++) {
        const pc_fxapp_t *ap = A(i);
        for (int p = 0; p < ap->nparam; p++) s->v[i][p] = ap->param[p].def;
    }
}

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

float pc_fxstate_clamp(int app, int param, float v)
{
    const pc_fxapp_t *ap = app_at(app);
    const pc_fxparam_t *q = NULL;
    if (ap && param == PC_FXSTATE_MIX) q = &pc_fxapp_mix_param;
    else if (ap && param >= 0 && param < ap->nparam) q = &ap->param[param];
    if (!q) return isfinite(v) ? v : 0.0f;
    if (!isfinite(v)) v = q->def;
    v = clampf(v, q->lo, q->hi);
    if (q->unit == PC_FXU_BOOL || q->unit == PC_FXU_CHOICE) v = clampf(roundf(v), q->lo, q->hi);
    return v;
}

/* ---- kv ---- */
/* the shortest text that reads back as the same float (lossless; "-30", "0.45", not "-3e+01" or "0.449999988"): fixed decimals first, then %g */
static void fmt_f(char *b, size_t n, float v)
{
    for (int d = 0; d <= 6 && fabsf(v) < 1e7f; d++) {
        snprintf(b, n, "%.*f", d, (double)v);
        if (strtof(b, NULL) == v) return;
    }
    for (int prec = 1; prec <= 9; prec++) {
        snprintf(b, n, "%.*g", prec, (double)v);
        if (strtof(b, NULL) == v) return;
    }
}

static int kv_put(pc_kv_t *kv, int *n, int max, const char *key, const char *val)
{
    if (strlen(key) >= sizeof kv->key) return 0;                 /* cannot be stored: never truncated into another key */
    return pc_kv_set(kv, n, max, key, val);
}

int pc_fxstate_to_kv(const pc_fxstate_t *s, pc_kv_t *kv, int max)
{
    int n = 0;
    char key[64], val[128], f[4][32];
    if (kv_put(kv, &n, max, "format", "1") || kv_put(kv, &n, max, "state_from", s->state_from)) return -1;
    for (int i = 0; i < napps(); i++) {
        const pc_fxapp_t *ap = A(i);
        snprintf(key, sizeof key, "%s.on", ap->id);
        if (kv_put(kv, &n, max, key, s->on[i] ? "1" : "0")) return -1;
        for (int p = 0; p < ap->nparam; p++) {
            if (ap->param[p].flags & PC_FXP_BAND) continue;
            fmt_f(val, sizeof val, s->v[i][p]);
            snprintf(key, sizeof key, "%s.%s", ap->id, ap->param[p].id);
            if (kv_put(kv, &n, max, key, val)) return -1;
        }
        if (ap->flags & PC_FXA_MIX) {
            fmt_f(val, sizeof val, s->mix[i]);
            snprintf(key, sizeof key, "%s.mix", ap->id);
            if (kv_put(kv, &n, max, key, val)) return -1;
        }
        int b0 = band_base(ap);
        for (int b = 0; b0 >= 0 && b < band_count(ap); b++) {
            const float *v = &s->v[i][b0 + 4 * b];
            int t = (int)pc_fxstate_clamp(i, b0 + 4 * b, v[0]);
            fmt_f(f[0], sizeof f[0], v[1]); fmt_f(f[1], sizeof f[1], v[2]); fmt_f(f[2], sizeof f[2], v[3]);
            snprintf(val, sizeof val, "%c %s %s %s", BAND_LETTER[t], f[0], f[1], f[2]);
            snprintf(key, sizeof key, "%s.b%d", ap->id, b);
            if (kv_put(kv, &n, max, key, val)) return -1;
        }
    }
    return n;
}

/* a number, or NaN (which the clamp turns into the default) when the text is empty or not a number */
static float parse_f(const char *t, const char **end)
{
    if (!t) return NAN;
    char *e;
    float v = strtof(t, &e);
    if (e == t) return NAN;
    if (end) *end = e;
    return v;
}
static float parse_whole(const char *t)
{
    const char *e = NULL;
    float v = parse_f(t, &e);
    return e && (*e == 0 || *e == ' ') ? v : NAN;
}

void pc_fxstate_from_kv(pc_fxstate_t *s, const pc_kv_t *kv, int n)
{
    pc_fxstate_defaults(s);
    const char *fmt = pc_kv_get(kv, n, "format");
    if (fmt && strcmp(fmt, "1")) return;                          /* a format this build does not know: defaults, everything off */
    const char *sf = pc_kv_get(kv, n, "state_from");
    if (sf) snprintf(s->state_from, sizeof s->state_from, "%s", sf);
    char key[64];
    for (int i = 0; i < napps(); i++) {
        const pc_fxapp_t *ap = A(i);
        snprintf(key, sizeof key, "%s.on", ap->id);
        float on = parse_whole(pc_kv_get(kv, n, key));
        s->on[i] = isfinite(on) && on >= 0.5f;
        for (int p = 0; p < ap->nparam; p++) {
            if (ap->param[p].flags & PC_FXP_BAND) continue;
            snprintf(key, sizeof key, "%s.%s", ap->id, ap->param[p].id);
            const char *t = pc_kv_get(kv, n, key);
            if (t) s->v[i][p] = pc_fxstate_clamp(i, p, parse_whole(t));
        }
        if (ap->flags & PC_FXA_MIX) {
            snprintf(key, sizeof key, "%s.mix", ap->id);
            const char *t = pc_kv_get(kv, n, key);
            if (t) s->mix[i] = pc_fxstate_clamp(i, PC_FXSTATE_MIX, parse_whole(t));
        }
        int b0 = band_base(ap);
        for (int b = 0; b0 >= 0 && b < band_count(ap); b++) {
            snprintf(key, sizeof key, "%s.b%d", ap->id, b);
            const char *t = pc_kv_get(kv, n, key);
            if (!t) continue;
            float *v = &s->v[i][b0 + 4 * b];
            while (*t == ' ') t++;
            int ty = *t == 'P' ? 0 : *t == 'L' ? 1 : *t == 'H' ? 2 : -1;
            if ((*t >= 'A' && *t <= 'Z') || (*t >= 'a' && *t <= 'z')) t++;   /* the type letter (an unknown one too); a line without one starts with the number */
            const char *e = t;
            for (int k = 1; k <= 3; k++) {
                float x = parse_f(e, &e);
                v[k] = pc_fxstate_clamp(i, b0 + 4 * b + k, x);
                if (!isfinite(x)) { e = ""; }
            }
            if (ty >= 0) v[0] = (float)ty;                        /* else the default type stays */
        }
    }
}

/* ---- import ---- */
typedef struct { char (*u)[40]; int max, n; } rep_t;
static void rep(rep_t *r, const char *s)
{
    for (int i = 0; i < r->n; i++)
        if (!strncmp(r->u[i], s, 39)) return;                     /* once */
    if (r->u && r->n < r->max) snprintf(r->u[r->n++], 40, "%s", s);
}
static void rep2(rep_t *r, const char *a, const char *sep, const char *b)
{
    char t[80];
    snprintf(t, sizeof t, "%s%s%s", a, sep, b);
    rep(r, t);
}

static void append_band(pc_fxstate_t *s, int app, const pc_eq_band_t *b)
{
    const pc_fxapp_t *ap = A(app);
    int b0 = band_base(ap), ci = pfind(ap, "count");
    if (b0 < 0 || ci < 0) return;
    int n = (int)s->v[app][ci];
    if (n >= band_count(ap)) return;
    float *v = &s->v[app][b0 + 4 * n];
    v[0] = b->type == 'L' ? 1.0f : b->type == 'H' ? 2.0f : 0.0f;
    v[1] = pc_fxstate_clamp(app, b0 + 4 * n + 1, b->fc);
    v[2] = pc_fxstate_clamp(app, b0 + 4 * n + 2, b->gain_db);
    v[3] = pc_fxstate_clamp(app, b0 + 4 * n + 3, b->q);
    s->v[app][ci] = (float)(n + 1);
}

/* the first node of an app: its values (parameters by id, mix), the equalizer's bands */
static void load_node(pc_fxstate_t *s, int app, const pc_node_t *x, float *eq_pre, rep_t *r)
{
    const pc_fxapp_t *ap = A(app);
    bool param_mix = false;
    for (int k = 0; k < x->np; k++) {
        int q = pfind(ap, x->pname[k]);
        if (q >= 0) { s->v[app][q] = pc_fxstate_clamp(app, q, x->pval[k]); if (!strcmp(x->pname[k], "mix")) param_mix = true; }
        else if (!strcmp(x->pname[k], "mix") && (ap->flags & PC_FXA_MIX)) { s->mix[app] = pc_fxstate_clamp(app, PC_FXSTATE_MIX, x->pval[k]); param_mix = true; }
        else if (!(ap->flags & PC_FXA_BANDS)) rep2(r, x->kind, ".", x->pname[k]);
    }
    if (!param_mix) {
        int mi = pfind(ap, "mix");
        if (ap->flags & PC_FXA_MIX) s->mix[app] = pc_fxstate_clamp(app, PC_FXSTATE_MIX, x->mix);
        else if (mi >= 0) { if (fabsf(x->mix - 1.0f) > 1e-6f) s->v[app][mi] = pc_fxstate_clamp(app, mi, x->mix); }   /* an own-mix app: the node's mix field only when it says something */
        else if (fabsf(x->mix - 1.0f) > 1e-6f) rep2(r, x->kind, " ", "mix");
    }
    if (ap->flags & PC_FXA_BANDS) {
        int ci = pfind(ap, "count");
        if (ci >= 0) s->v[app][ci] = 0;
        *eq_pre = x->eq_preamp_db;
        for (int b = 0; b < x->neq; b++) append_band(s, app, &x->eq[b]);
    }
}

int pc_fxstate_import(pc_fxstate_t *s, const pc_preset_t *p, char (*unsup)[40], int max)
{
    char keep[sizeof s->state_from];
    memcpy(keep, s->state_from, sizeof keep);
    pc_fxstate_defaults(s);
    memcpy(s->state_from, keep, sizeof keep);
    rep_t r = { unsup, unsup ? max : 0, 0 };
    for (int i = 0; i < p->nunsup; i++) rep(&r, p->unsupported[i]);
    bool claimed[PC_FXAPP_MAX] = { false };
    float eq_pre[PC_FXAPP_MAX] = { 0 };
    for (int pass = 0; pass < 2; pass++) {                        /* enabled nodes first; a disabled one only keeps its values for an app nobody enabled */
        for (int list = 0; list < 3; list++) {                    /* music, master, sends */
            int cnt = list == 0 ? p->nmusic : list == 1 ? p->nmaster : p->nsends;
            for (int k = 0; k < cnt; k++) {
                const pc_node_t *x = list == 0 ? &p->music[k] : list == 1 ? &p->master[k] : &p->sends[k].node;
                if (!x->supported) continue;
                int app = pc_fxapp_find(x->kind);
                if (app < 0 || x->enabled != (pass == 0)) continue;
                if (list == 2 && strcmp(x->kind, "freeverb")) { if (pass == 0) rep2(&r, x->kind, " ", "(send)"); continue; }
                const pc_fxapp_t *ap = A(app);
                if (claimed[app]) {
                    if (pass) continue;
                    if (ap->flags & PC_FXA_BANDS) {              /* equalizer nodes merge: bands appended, preamps summed */
                        eq_pre[app] += x->eq_preamp_db;
                        for (int b = 0; b < x->neq; b++) append_band(s, app, &x->eq[b]);
                    } else rep2(&r, x->kind, " ", "(重複)");
                    continue;
                }
                claimed[app] = true;
                s->on[app] = pass == 0;
                load_node(s, app, x, &eq_pre[app], &r);
                if (list == 2) { int lv = pfind(ap, "level"); if (lv >= 0) s->v[app][lv] = pc_fxstate_clamp(app, lv, p->sends[k].level); }
            }
        }
    }
    for (int i = 0; i < napps(); i++) {
        int pi = pfind(A(i), "preamp_db");
        if (claimed[i] && (A(i)->flags & PC_FXA_BANDS) && pi >= 0) s->v[i][pi] = pc_fxstate_clamp(i, pi, eq_pre[i]);
    }
    return r.n;
}

/* ---- export ---- */
typedef struct { char *o; size_t n, len; bool bad; } wr_t;
static void wr(wr_t *w, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void wr(wr_t *w, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    size_t room = w->len < w->n ? w->n - w->len : 0;
    int k = vsnprintf(room ? w->o + w->len : NULL, room, fmt, ap);
    va_end(ap);
    if (k < 0 || (size_t)k >= room) w->bad = true;
    w->len += k > 0 ? (size_t)k : 0;
}
static const char *fl(char *b, float v)           /* a TOML float: a decimal point or an exponent always */
{
    fmt_f(b, 32, v);
    if (!strpbrk(b, ".e")) strcat(b, ".0");
    return b;
}

static void export_node(wr_t *w, const pc_fxstate_t *s, int i, bool send)
{
    const pc_fxapp_t *ap = A(i);
    char f[32];
    bool first = true;
    if (send) wr(w, "[[sends]]\nfrom = 0\nlevel = %s\n%snodes = [{ kind = \"%s\"", fl(f, s->v[i][pfind(ap, "level")]), s->on[i] ? "" : "on = false\n", ap->id);
    else wr(w, "[[%s]]\nkind = \"%s\"\n%s", (ap->flags & PC_FXA_ALWAYS_LAST) ? "master" : "music", ap->id, s->on[i] ? "" : "enabled = false\n");
    if (!send && (ap->flags & PC_FXA_MIX)) wr(w, "mix = %s\n", fl(f, s->mix[i]));
    if (ap->flags & PC_FXA_BANDS) {
        int b0 = band_base(ap), ci = pfind(ap, "count"), pi = pfind(ap, "preamp_db");
        char g[32], q[32], c[32];
        wr(w, "equalizer = \"\"\"\nPreamp: %s dB\n", fl(f, s->v[i][pi]));
        for (int b = 0; b < (int)s->v[i][ci] && b < band_count(ap); b++) {
            const float *v = &s->v[i][b0 + 4 * b];
            int t = (int)pc_fxstate_clamp(i, b0 + 4 * b, v[0]);
            wr(w, "Filter %d: ON %s Fc %s Hz Gain %s dB Q %s\n", b + 1, t == 1 ? "LSC" : t == 2 ? "HSC" : "PK", fl(c, v[1]), fl(g, v[2]), fl(q, v[3]));
        }
        wr(w, "\"\"\"\n");
        return;
    }
    for (int p = 0; p < ap->nparam; p++) {
        if ((ap->param[p].flags & PC_FXP_BAND) || (send && !strcmp(ap->param[p].id, "level"))) continue;
        wr(w, first ? (send ? ", params = { %s = %s" : "params = { %s = %s") : ", %s = %s", ap->param[p].id, fl(f, s->v[i][p]));
        first = false;
    }
    if (send) wr(w, "%s }]\n", first ? "" : " }");
    else wr(w, "%s", first ? "" : " }\n");
}

int pc_fxstate_export(const pc_fxstate_t *s, char *out, size_t n)
{
    wr_t w = { out, n, 0, false };
    wr(&w, "format = 1\n");
    for (int pass = 0; pass < 3; pass++)                          /* music nodes, the master node(s), the reverb send, each in registry order */
        for (int i = 0; i < napps(); i++) {
            const pc_fxapp_t *ap = A(i);
            int which = (ap->flags & PC_FXA_ALWAYS_LAST) ? 1 : !strcmp(ap->id, "freeverb") ? 2 : 0;
            if (which != pass) continue;
            wr(&w, "\n");
            export_node(&w, s, i, which == 2);
        }
    if (w.bad) { if (n) out[0] = 0; return -1; }
    return (int)w.len;
}
