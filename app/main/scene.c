// SPDX-License-Identifier: LGPL-2.1-or-later
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "pc/pc_io.h"
#include "pc/pc_loop.h"
#include "backend_sdec.h"

static const char *TAG = "scene";
#define DIR "/sdcard/_player/scenes/"
#define MAX_SCENES 24
#define MAX_LAYERS 3
#define MAXF 4096
typedef struct { char name[48]; int n; struct { char file[72]; float vol; } l[MAX_LAYERS]; } scene_t;
static scene_t *s_sc; static int s_nsc;

/* "name|file.ogg:vol|file.ogg:vol" -> one scene (line is edited in place); false when it has no layer */
static bool parse_scene(char *line, scene_t *s)
{
    memset(s, 0, sizeof *s);
    char *sv, *t = strtok_r(line, "|", &sv);
    if (!t) return false;
    snprintf(s->name, sizeof s->name, "%s", t);
    while ((t = strtok_r(NULL, "|", &sv)) && s->n < MAX_LAYERS) {
        char *c = strrchr(t, ':');
        s->l[s->n].vol = c ? (float)atof(c + 1) : 1.0f;
        if (c) *c = 0;
        snprintf(s->l[s->n].file, sizeof s->l[s->n].file, "%s", t);
        s->n++;
    }
    return s->n > 0;
}

static char s_dbg_scene[200];                    /* UART test scene (RAM only): appended after the card's */
void nsn_scene_debug_add(const char *spec) { snprintf(s_dbg_scene, sizeof s_dbg_scene, "%s", spec); }

int nsn_scene_load(void)
{
    if (!s_sc) s_sc = heap_caps_calloc(MAX_SCENES, sizeof(scene_t), MALLOC_CAP_SPIRAM);
    s_nsc = 0;
    if (!s_sc) return 0;
    FILE *f = fopen(DIR "scenes.txt", "rb");
    char line[512];
    while (f && s_nsc < MAX_SCENES && fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0] || line[0] == '#') continue;
        if (parse_scene(line, &s_sc[s_nsc])) s_nsc++;
    }
    if (f) fclose(f);
    if (s_dbg_scene[0] && s_nsc < MAX_SCENES) {
        snprintf(line, sizeof line, "%s", s_dbg_scene);
        if (parse_scene(line, &s_sc[s_nsc])) s_nsc++;
    }
    return s_nsc;
}
const char *nsn_scene_name(int i) { return i >= 0 && i < s_nsc ? s_sc[i].name : NULL; }

/* Each layer loops through JukeDial's pc_loop: the output runs a crossfade behind the decoder and the end of a pass is blended into the start of the next
 * one, so the seam is a short equal-power crossfade instead of a cut. The file is in PSRAM, so "starting over" is an open from memory at offset 0.
 * Two ways to get the next pass, chosen by s_spare:
 *   rewind (default): the ended decoder is closed and reopened in place; one decoder per layer exists at any time (no extra internal RAM).
 *   spare: pc_loop keeps a second decoder open around the seam and swaps (no open on the audio thread at the seam, but two decoders per layer). */
#define XFADE_MS 50
static volatile int s_xfade_ms = XFADE_MS;       /* debug: 0 = a hard cut (what the old close-and-reopen did), to compare */
void nsn_scene_debug_xfade(int ms) { s_xfade_ms = ms >= 0 ? ms : XFADE_MS; }
typedef struct { uint8_t *mem; size_t len; const pc_backend_t *be; pc_reader_t r; float vol; pc_loop_t *loop; int seams, dead; } layer_t;
typedef struct { void *h; layer_t *l; uint32_t rate; uint64_t played; } dec_t;
typedef struct { int n; uint32_t rate; uint64_t frames; layer_t l[MAX_LAYERS]; float *tmp; } mix_t;
static volatile int s_looplen_ms;               /* debug: a pass ends after this long (0 = at the end of the file) */
static volatile bool s_spare;                   /* debug: use pc_loop's second decoder instead of the rewind */
void nsn_scene_debug_looplen(int ms) { s_looplen_ms = ms > 0 ? ms : 0; }
void nsn_scene_debug_spare(bool on) { s_spare = on; }

static int dec_open(dec_t *d)
{
    d->played = 0;
    if (d->l->be->open(&d->h, &d->l->r, 0) != 0) { d->h = NULL; return -1; }
    d->rate = d->l->be->rate(d->h);
    return 0;
}
static int src_open(void *ctx, void **dec, uint32_t *rate)
{
    dec_t *d = heap_caps_calloc(1, sizeof *d, MALLOC_CAP_SPIRAM);
    if (!d) return -1;
    d->l = ctx;
    if (dec_open(d) != 0) { heap_caps_free(d); return -1; }
    *rate = d->rate; *dec = d;
    return 0;
}
static int src_read(void *dec, float *stereo, int frames)
{
    dec_t *d = dec;
    if (s_looplen_ms > 0) {
        int64_t left = (int64_t)s_looplen_ms * d->rate / 1000 - (int64_t)d->played;
        if (left <= 0) return 0;
        if (frames > left) frames = (int)left;
    }
    int n = d->l->be->render(d->h, stereo, frames);
    if (n <= 0) return 0;
    d->played += (uint64_t)n;
    return n;
}
static void src_close(void *dec)
{
    dec_t *d = dec;
    if (d->h) d->l->be->close(d->h);
    heap_caps_free(d);
}
static int src_rewind(void *dec)
{
    dec_t *d = dec;
    ESP_LOGI(TAG, "layer pass ended after %.1f s", (double)d->played / (d->rate ? d->rate : 1));
    d->l->be->close(d->h); d->h = NULL;
    return dec_open(d);
}

static int scene_open(void **h, const pc_reader_t *r, uint32_t off)
{
    int idx = (int)(intptr_t)r->u;
    if (!s_nsc) nsn_scene_load();
    if (idx < 0 || idx >= s_nsc) return -1;
    mix_t *m = heap_caps_calloc(1, sizeof *m, MALLOC_CAP_SPIRAM);
    if (!m) return -1;
    m->tmp = heap_caps_malloc(MAXF * 2 * sizeof(float), MALLOC_CAP_SPIRAM);
    scene_t *s = &s_sc[idx];
    for (int i = 0; i < s->n && m->tmp; i++) {
        layer_t *l = &m->l[m->n];
        char path[128]; snprintf(path, sizeof path, DIR "%s", s->l[i].file);
        const char *e = strrchr(path, '.');
        l->be = e && !strcasecmp(e, ".mp3") ? &pc_backend_mp3 : &backend_ogg;
        FILE *f = fopen(path, "rb");
        if (!f) { ESP_LOGW(TAG, "missing %s", path); continue; }
        fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
        if (sz <= 0 || sz > 6 * 1024 * 1024 || !(l->mem = heap_caps_malloc((size_t)sz, MALLOC_CAP_SPIRAM))) { fclose(f); continue; }
        l->len = fread(l->mem, 1, (size_t)sz, f); fclose(f);
        pc_reader_mem(&l->r, l->mem, l->len);
        pc_reader_set_ext(&l->r, path);
        l->vol = s->l[i].vol;
        pc_layer_src_t src = { .open = src_open, .read = src_read, .close = src_close, .rewind = s_spare ? NULL : src_rewind, .ctx = l };
        l->loop = pc_loop_create(&src, (uint32_t)s_xfade_ms);
        if (!l->loop) { ESP_LOGW(TAG, "cannot decode %s", path); heap_caps_free(l->mem); l->mem = NULL; continue; }
        uint32_t rt = pc_loop_rate(l->loop);
        if (!m->rate) m->rate = rt;
        if (rt != m->rate) { ESP_LOGW(TAG, "%s is %lu Hz, scene is %lu Hz: skipped", path, (unsigned long)rt, (unsigned long)m->rate); pc_loop_destroy(l->loop); l->loop = NULL; heap_caps_free(l->mem); l->mem = NULL; continue; }
        m->n++;
    }
    if (!m->n) { heap_caps_free(m->tmp); heap_caps_free(m); return -1; }
    ESP_LOGI(TAG, "scene \"%s\": %d layer(s) at %lu Hz, %s", s->name, m->n, (unsigned long)m->rate, s_spare ? "spare decoder" : "rewind");
    *h = m;
    return 0;
}

static int scene_render(void *h, float *out, int frames)
{
    mix_t *m = h;
    if (frames > MAXF) frames = MAXF;
    memset(out, 0, (size_t)frames * 2 * sizeof(float));
    for (int i = 0; i < m->n; i++) {
        layer_t *l = &m->l[i];
        if (l->dead) continue;
        if (pc_loop_render(l->loop, m->tmp, frames) < frames) { ESP_LOGW(TAG, "layer %d cannot go on", i); l->dead = 1; continue; }
        for (int k = 0; k < frames * 2; k++) out[k] += m->tmp[k] * l->vol;
        if (pc_loop_seams(l->loop) != l->seams) { l->seams = pc_loop_seams(l->loop); ESP_LOGI(TAG, "layer %d: seam %d", i, l->seams); }
    }
    for (int k = 0; k < frames * 2; k++) { if (out[k] > 1.0f) out[k] = 1.0f; else if (out[k] < -1.0f) out[k] = -1.0f; }
    m->frames += (uint64_t)frames;
    return frames;
}
static uint32_t scene_rate(void *h) { return ((mix_t *)h)->rate; }
static int32_t scene_len(void *h) { return -1; }
static int32_t scene_pos(void *h) { mix_t *m = h; return (int32_t)(m->frames * 1000 / m->rate); }
static void scene_close(void *h)
{
    mix_t *m = h;
    for (int i = 0; i < m->n; i++) { pc_loop_destroy(m->l[i].loop); heap_caps_free(m->l[i].mem); }
    heap_caps_free(m->tmp); heap_caps_free(m);
}
const pc_backend_t nsn_scene_backend = { .open = scene_open, .render = scene_render, .rate = scene_rate, .length_ms = scene_len, .position_ms = scene_pos, .close = scene_close, .name = "Scene" };
