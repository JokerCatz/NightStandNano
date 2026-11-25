// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_kv.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_trimmed(char *dst, size_t cap, const char *b, const char *e)
{
    while (b < e && isspace((unsigned char)*b)) b++;
    while (e > b && isspace((unsigned char)e[-1])) e--;
    size_t n = (size_t)(e - b);
    if (n > cap - 1) n = cap - 1;
    memcpy(dst, b, n);
    dst[n] = 0;
}

static int find(const pc_kv_t *kv, int n, const char *key)
{
    for (int i = 0; i < n; i++)
        if (!strcmp(kv[i].key, key)) return i;
    return -1;
}

int pc_kv_set(pc_kv_t *kv, int *n, int max, const char *key, const char *val)
{
    int i = find(kv, *n, key);
    if (i < 0) {
        if (*n >= max) return -1;
        i = (*n)++;
        snprintf(kv[i].key, sizeof kv[i].key, "%s", key);
    }
    snprintf(kv[i].val, sizeof kv[i].val, "%s", val);
    return 0;
}

int pc_kv_del(pc_kv_t *kv, int *n, const char *key)
{
    int i = find(kv, *n, key);
    if (i < 0) return 0;
    memmove(&kv[i], &kv[i + 1], (size_t)(*n - i - 1) * sizeof *kv);
    (*n)--;
    return 1;
}

int pc_kv_parse(const char *text, pc_kv_t *out, int max)
{
    int n = 0;
    const char *p = text;
    while (*p) {
        const char *eol = strchr(p, '\n');
        if (!eol) eol = p + strlen(p);
        const char *q = p;
        while (q < eol && isspace((unsigned char)*q)) q++;
        const char *eq = memchr(q, '=', (size_t)(eol - q));
        if (q < eol && *q != '#' && eq) {
            char key[32], val[sizeof out->val];
            copy_trimmed(key, sizeof key, q, eq);
            copy_trimmed(val, sizeof val, eq + 1, eol);
            if (key[0]) pc_kv_set(out, &n, max, key, val);
        }
        p = *eol ? eol + 1 : eol;
    }
    return n;
}

int pc_kv_load(const char *path, pc_kv_t *out, int max)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len < 0 || len > 64 * 1024) { fclose(f); return 0; }
    char *buf = malloc((size_t)len + 1);
    if (!buf) { fclose(f); return 0; }
    size_t got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    buf[got] = 0;
    int n = pc_kv_parse(buf, out, max);
    free(buf);
    return n;
}

int pc_kv_save(const char *path, const pc_kv_t *kv, int n)
{
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return -1;
    int ok = 1;
    for (int i = 0; i < n; i++)
        if (fprintf(f, "%s=%s\n", kv[i].key, kv[i].val) < 0) ok = 0;
    if (fclose(f) != 0) ok = 0;
    if (!ok) { remove(tmp); return -1; }
    remove(path);              /* FATFS rename does not replace an existing file */
    return rename(tmp, path) == 0 ? 0 : -1;
}

const char *pc_kv_get(const pc_kv_t *kv, int n, const char *key)
{
    int i = find(kv, n, key);
    return i < 0 ? NULL : kv[i].val;
}

int pc_kv_get_int(const pc_kv_t *kv, int n, const char *key, int def)
{
    const char *v = pc_kv_get(kv, n, key);
    if (!v || !*v) return def;
    char *end;
    long x = strtol(v, &end, 10);
    return *end ? def : (int)x;
}
