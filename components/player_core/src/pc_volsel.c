// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_volsel.h"
#include <stdio.h>
#include <string.h>
#include "pc/pc_volume.h"

int pc_volsel_clamp(int v, int max)
{
    if (max > 100) max = 100;
    if (max < 0) max = 0;
    return v < 0 ? 0 : v > max ? max : v;
}

int pc_volsel_wired(int stored, int max) { return pc_volsel_clamp(stored < 0 ? PC_VOL_DEFAULT : stored, max); }
int pc_volsel_spk(int stored, int max) { return pc_volsel_clamp(stored < 0 ? PC_VOLSEL_SPK_DEFAULT : stored, max); }

typedef struct { uint8_t bda[6]; int vol; } ent_t;

static int hexv(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1; }

/* the well-formed entries, in order, at most PC_VOLSEL_MAX (a broken entry is skipped; a repeated address keeps its first, newest, place) */
static int parse(const char *s, ent_t *e)
{
    int n = 0;
    while (s && *s && n < PC_VOLSEL_MAX) {
        const char *end = strchr(s, ',');
        size_t len = end ? (size_t)(end - s) : strlen(s);
        ent_t t;
        bool ok = len >= 14 && len <= 16 && s[12] == ':';
        for (int i = 0; ok && i < 6; i++) {
            int h = hexv(s[2 * i]), l = hexv(s[2 * i + 1]);
            if (h < 0 || l < 0) ok = false; else t.bda[i] = (uint8_t)(h * 16 + l);
        }
        int v = 0;
        for (size_t i = 13; ok && i < len; i++) { if (s[i] < '0' || s[i] > '9') ok = false; else v = v * 10 + (s[i] - '0'); }
        if (ok && v <= 100) {
            bool dup = false;
            for (int i = 0; i < n; i++) if (!memcmp(e[i].bda, t.bda, 6)) dup = true;
            if (!dup) { t.vol = v; e[n++] = t; }
        }
        s += len;
        if (*s == ',') s++;
    }
    return n;
}

static bool emit(char *list, size_t cap, const ent_t *e, int n)
{
    char out[PC_VOLSEL_TEXT];
    size_t at = 0;
    if (cap > sizeof out) cap = sizeof out;
    for (int i = 0; i < n; i++) {
        char one[20];
        int k = snprintf(one, sizeof one, "%s%02x%02x%02x%02x%02x%02x:%d", i ? "," : "", e[i].bda[0], e[i].bda[1], e[i].bda[2], e[i].bda[3], e[i].bda[4],
                         e[i].bda[5], e[i].vol);
        if (at + (size_t)k + 1 > cap) break;                 /* no room: the least recently used ones are dropped */
        memcpy(out + at, one, (size_t)k);
        at += (size_t)k;
    }
    out[at] = 0;
    if (!cap) return false;
    if (!strcmp(list, out)) return false;
    memcpy(list, out, at + 1);
    return true;
}

static int find(const ent_t *e, int n, const uint8_t bda[6])
{
    for (int i = 0; i < n; i++) if (!memcmp(e[i].bda, bda, 6)) return i;
    return -1;
}

int pc_volsel_bt(const char *list, const uint8_t bda[6], int max)
{
    ent_t e[PC_VOLSEL_MAX];
    int n = parse(list, e), i = bda ? find(e, n, bda) : -1;
    return pc_volsel_clamp(i < 0 ? PC_VOLSEL_BT_DEFAULT : e[i].vol, max);
}

static bool to_front(char *list, size_t cap, const uint8_t bda[6], int vol, bool add)
{
    ent_t e[PC_VOLSEL_MAX + 1];
    int n = parse(list, e + 1), i = find(e + 1, n, bda);
    if (i < 0 && !add) return false;
    memcpy(e[0].bda, bda, 6);
    e[0].vol = i < 0 || add ? vol : e[1 + i].vol;
    if (i >= 0) { memmove(e + 1 + i, e + 2 + i, (size_t)(n - 1 - i) * sizeof e[0]); n--; }
    n++;
    if (n > PC_VOLSEL_MAX) n = PC_VOLSEL_MAX;
    return emit(list, cap, e, n);
}

bool pc_volsel_bt_put(char *list, size_t cap, const uint8_t bda[6], int vol) { return to_front(list, cap, bda, pc_volsel_clamp(vol, 100), true); }
bool pc_volsel_bt_touch(char *list, size_t cap, const uint8_t bda[6]) { return to_front(list, cap, bda, 0, false); }

bool pc_volsel_bt_del(char *list, const uint8_t bda[6])
{
    ent_t e[PC_VOLSEL_MAX];
    int n = parse(list, e), i = find(e, n, bda);
    if (i < 0) return false;
    memmove(e + i, e + i + 1, (size_t)(n - 1 - i) * sizeof e[0]);
    return emit(list, strlen(list) + 1, e, n - 1);
}

int pc_volsel_bt_count(const char *list)
{
    ent_t e[PC_VOLSEL_MAX];
    return parse(list, e);
}

const char *pc_volsel_name(pc_vout_t out, const uint8_t bda[6], char *buf, size_t cap)
{
    if (out == PC_VOUT_BT && bda) snprintf(buf, cap, "bt:%02x%02x%02x%02x%02x%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    else snprintf(buf, cap, "%s", out == PC_VOUT_CAST ? "cast" : out == PC_VOUT_SPEAKER ? "speaker" : out == PC_VOUT_BT ? "bt:?" : "wired");
    return buf;
}

int pc_volsel_next(int req, int old_full, int new_full) { return req < old_full && req < new_full ? req : new_full; }
int pc_volsel_across(int applied, int next) { return applied < next ? applied : next; }

void pc_volhold_begin(pc_volhold_t *h, int cap)
{
    if (cap < 0) cap = 0;
    if (!h->cap1 || cap + 1 < h->cap1) h->cap1 = cap + 1;
}

int pc_volhold_apply(pc_volhold_t *h, int vol)
{
    if (!h->cap1) return vol;
    h->want1 = vol + 1;                                      /* the newest request, whatever it is: end() applies it */
    return vol > h->cap1 - 1 ? h->cap1 - 1 : vol;
}

int pc_volhold_end(pc_volhold_t *h)
{
    int w = h->cap1 ? h->want1 - 1 : -1;
    h->cap1 = h->want1 = 0;
    return w;
}
