// SPDX-License-Identifier: LGPL-2.1-or-later
#include "zipfs.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "pc/pc_io.h"
#include "pc/pc_zip.h"
#include "pc/pc_crc32.h"

/* pc_inflate.h is private to player_core; these two are in the library (the state variant keeps the 11 KB decompressor off the stack) */
size_t pc_inflate_state_size(void);
long pc_inflate_raw_state(const void *in, size_t in_len, void *out, size_t out_len, void *state);

static const char *TAG = "zip";

bool nsn_zip_is_archive_name(const char *name)
{
    size_t n = strlen(name);
    return n > 4 && !strcasecmp(name + n - 4, ".zip");
}

bool nsn_zip_split(const char *path, char *zip, size_t zcap, const char **inner)
{
    for (const char *p = path; (p = strcasestr(p, ".zip")); p += 4) {
        if (p[4] != '/' && p[4] != 0) continue;
        size_t n = (size_t)(p + 4 - path);
        if (n >= zcap) return false;
        memcpy(zip, path, n); zip[n] = 0;
        struct stat st;
        if (stat(zip, &st) != 0 || !S_ISREG(st.st_mode)) continue;        /* a folder called x.zip, or nothing there: not an archive */
        if (inner) *inner = p[4] ? p + 5 : p + 4;
        return true;
    }
    return false;
}

/* an open archive: the file, its reader and the central directory */
typedef struct { FILE *f; pc_reader_t r; pc_zip_entry_t *e; int n; } arc_t;

static int arc_open(arc_t *a, const char *zip)
{
    memset(a, 0, sizeof *a);
    a->f = fopen(zip, "rb");
    if (!a->f) return -ENOENT;
    if (pc_reader_file(&a->r, a->f) != 0) { fclose(a->f); return -EINVAL; }
    int rc = pc_zip_open(&a->r, &a->e, &a->n);
    if (rc) { fclose(a->f); return rc; }
    return 0;
}
static void arc_close(arc_t *a) { pc_zip_free(a->e, a->n); fclose(a->f); }

/* "dir/" prefix of a folder inside the archive ("" for the root); trailing slashes of the caller's text are dropped */
static size_t prefix_of(const char *dir, char *pre, size_t cap)
{
    size_t n = strlen(dir);
    while (n && dir[n - 1] == '/') n--;
    if (n + 2 > cap) n = cap - 2;
    memcpy(pre, dir, n);
    if (n) pre[n++] = '/';
    pre[n] = 0;
    return n;
}

int nsn_zip_stat(const char *zip, const char *inner, nsn_zip_kind_t *kind, uint32_t *size)
{
    arc_t a;
    int rc = arc_open(&a, zip);
    if (rc) return rc;
    char pre[200];
    size_t pl = prefix_of(inner, pre, sizeof pre);
    *kind = NSN_ZIP_NONE; if (size) *size = 0;
    if (!pl) *kind = NSN_ZIP_DIR;                                              /* the archive itself */
    else {
        char name[200]; strlcpy(name, pre, sizeof name); name[pl - 1] = 0;     /* the entry's own name */
        const pc_zip_entry_t *e = pc_zip_find(a.e, a.n, name);
        if (e) { *kind = NSN_ZIP_FILE; if (size) *size = e->usize; }
        else for (int i = 0; i < a.n; i++) if (!strncmp(a.e[i].name, pre, pl)) { *kind = NSN_ZIP_DIR; break; }
    }
    arc_close(&a);
    return 0;
}

static const char *base_of(const char *name) { const char *s = strrchr(name, '/'); return s ? s + 1 : name; }

int nsn_zip_ls(const char *zip, const char *dir, bool (*ok)(const char *), void (*emit)(void *, const char *, bool), void *ctx)
{
    arc_t a;
    int rc = arc_open(&a, zip);
    if (rc) return rc;
    char pre[200], pend[200] = ""; bool pend_has = false; int cnt = 0;
    size_t pl = prefix_of(dir, pre, sizeof pre);
    for (int i = 0; i < a.n; i++) {
        const char *name = a.e[i].name;
        if (strncmp(name, pre, pl)) continue;
        const char *rest = name + pl, *slash = strchr(rest, '/');
        if (!slash) {
            if (pend[0]) { if (pend_has) { emit(ctx, pend, true); cnt++; } pend[0] = 0; pend_has = false; }
            if (ok(rest)) { emit(ctx, rest, false); cnt++; }
            continue;
        }
        size_t cl = (size_t)(slash - rest);                                    /* the sub-folder's name; its entries are contiguous in the sorted list */
        if (strlen(pend) != cl || strncmp(pend, rest, cl)) {
            if (pend[0] && pend_has) { emit(ctx, pend, true); cnt++; }
            if (cl >= sizeof pend) cl = sizeof pend - 1;
            memcpy(pend, rest, cl); pend[cl] = 0; pend_has = false;
        }
        if (ok(base_of(rest))) pend_has = true;
    }
    if (pend[0] && pend_has) { emit(ctx, pend, true); cnt++; }
    arc_close(&a);
    return cnt;
}

int nsn_zip_collect(const char *zip, const char *dir, bool (*ok)(const char *), bool (*emit)(void *, const char *), void *ctx, int *skipped)
{
    arc_t a;
    int rc = arc_open(&a, zip);
    if (rc) return rc;
    char pre[200]; int cnt = 0, skip = 0;
    size_t pl = prefix_of(dir, pre, sizeof pre);
    for (int i = 0; i < a.n; i++) {
        const pc_zip_entry_t *e = &a.e[i];
        if (strncmp(e->name, pre, pl) || !ok(base_of(e->name))) continue;
        if (e->usize > NSN_ZIP_MAX_ENTRY || (e->method != 0 && e->method != 8)) { skip++; ESP_LOGW(TAG, "%s left out of the playlist (%lu bytes, method %d)", e->name, (unsigned long)e->usize, e->method); continue; }
        cnt++;
        if (!emit(ctx, e->name)) break;
    }
    if (skipped) *skipped = skip;
    arc_close(&a);
    return cnt;
}

int nsn_zip_read(const char *zip, const char *inner, uint32_t max, uint8_t **out, uint32_t *len)
{
    *out = NULL; *len = 0;
    arc_t a;
    int rc = arc_open(&a, zip);
    if (rc) return rc;
    char name[200]; size_t nl = prefix_of(inner, name, sizeof name);
    if (nl) name[nl - 1] = 0;
    const pc_zip_entry_t *e = nl ? pc_zip_find(a.e, a.n, name) : NULL;
    uint8_t *dst = NULL, *src = NULL, *st = NULL;
    if (!e) { rc = -ENOENT; goto done; }
    if (e->usize > max || e->usize > NSN_ZIP_MAX_ENTRY) { rc = -E2BIG; goto done; }
    if (e->method != 0 && e->method != 8) { rc = -ENOTSUP; goto done; }
    uint32_t off;
    if ((rc = pc_zip_data_offset(&a.r, e, &off)) != 0) goto done;
    dst = heap_caps_malloc(e->usize ? e->usize : 1, MALLOC_CAP_SPIRAM);
    if (!dst) { rc = -ENOMEM; goto done; }
    if (e->method == 0) {
        if (e->csize != e->usize || (uint32_t)a.r.read(a.r.u, off, dst, e->usize) != e->usize) { rc = -EBADMSG; goto done; }
    } else {
        src = heap_caps_malloc(e->csize ? e->csize : 1, MALLOC_CAP_SPIRAM);
        st = heap_caps_malloc(pc_inflate_state_size(), MALLOC_CAP_SPIRAM);
        if (!src || !st) { rc = -ENOMEM; goto done; }
        if ((uint32_t)a.r.read(a.r.u, off, src, e->csize) != e->csize || pc_inflate_raw_state(src, e->csize, dst, e->usize, st) != (long)e->usize) { rc = -EBADMSG; goto done; }
    }
    if (pc_crc32(0, dst, e->usize) != e->crc) { rc = -EBADMSG; goto done; }
    *out = dst; *len = e->usize; dst = NULL; rc = 0;
done:
    heap_caps_free(dst); heap_caps_free(src); heap_caps_free(st);
    arc_close(&a);
    return rc;
}

const char *nsn_zip_err(int rc)
{
    switch (-rc) {
    case 0: return "ok";
    case ENOENT: return "no such entry";
    case E2BIG: return "entry over the 8 MB limit";
    case ENOTSUP: return "compression method not supported (stored and deflate only)";
    case EBADMSG: return "corrupt entry (size or CRC)";
    case ENOMEM: return "out of memory";
    default: return "not a usable zip file";
    }
}
