// SPDX-License-Identifier: LGPL-2.1-or-later
#include "fx.h"
#include "storage.h"
#include "player.h"
#include "bt_audio.h"
#include "pc/pc_paths.h"
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "pc/pc_fxchain.h"
#include "pc/pc_kv.h"
#include "pc/pc_preset.h"

/* LVGL side of the fx chain (spec §3.2, §3.2a, §3.3, §4, §5). The values live in one PSRAM block (ctx_t); a change is posted to the chain's
 * mailbox at once (fx_set / fx_set_on) and stored on the card 3 s after the last one. A 50 ms timer takes every return first (frees the blocks
 * the chain hands back, notes a REBUILD request), then retries what could not be posted: one dirty bit per (app, parameter) so only the newest
 * value travels, and a block for an app that is on is posted only when no earlier block of that app is still waiting in the mailbox
 * (at most one pending + one installed block per app, however fast a REBUILD value is turned).
 * Blocks are allocated on enable from PSRAM only (heap_caps_malloc with MALLOC_CAP_SPIRAM, no internal fallback): a failed allocation leaves
 * the app dim with the reason "記憶體不足", the stored "on" stays. */

#define SAVE_AFTER_US 3000000
#define KV_MAX 160                         /* pairs: the registry writes <= 128 (host test) */
#define DIRTY_MIX (1ull << 63)
#define TXT_MAX 16384

typedef struct {
    pc_fxchain_t *chain;
    pc_fxstate_t st;                       /* the values (the truth; the chain follows) */
    pc_fxstate_t tmp;                      /* an import's result before it is applied */
    char unsup[8][40];  int nunsup;        /* the reports of the last import */
    uint64_t dirty[PC_FXAPP_MAX];          /* bit p: parameter p must still be posted as a SET; DIRTY_MIX: the chain-level 濃度 */
    void *pend[PC_FXAPP_MAX];              /* a block made and not yet posted (the mailbox was full) */
    uint32_t swap_seq[PC_FXAPP_MAX];       /* nposts after the last SWAP of the app: it is still in the mailbox until the chain has taken that many */
    bool posted[PC_FXAPP_MAX];             /* the chain was given a SWAP and no OFF since */
    bool rebuild[PC_FXAPP_MAX];            /* a new block is wanted (a REBUILD value changed, or the audio side asked) */
    bool fail[PC_FXAPP_MAX];               /* the last allocation for the app failed */
    uint32_t nposts;                       /* messages accepted by the mailbox, ever */
    int live;                              /* state blocks allocated here and not yet freed */
    int64_t dirty_at;                      /* us of the last change not yet on the card; 0 = nothing to store */
    uint32_t look_sig;                     /* the looks the change callback last saw */
    bool batt;                             /* PC_FXM_BATT as this file set it */
    uint8_t rate_bits;                     /* PC_FXM_HIRES / HIRES2 as this file set them, from audio_svc_info().pipe_rate */
    bool ldac;                             /* PC_FXM_LDAC as this file set it */
    bool shed;                             /* heavy apps paused because core 1 was overloaded */
    bool bt;                               /* PC_FXM_BT as this file set it, from audio_svc_info().sink */
    bool mig_pending;                      /* migrated from cfg "sound": that key goes once fx.txt is really on the card */
} ctx_t;

static SemaphoreHandle_t s_mx;                              /* recursive: the console, the night mode and the 50 ms task all touch the same state */
static ctx_t *s_x;                         /* PSRAM; NULL until fx_svc_start */
static uint8_t s_mode;                     /* the mode bits (PC_FXM_*), any task, __atomic */
static void (*s_cb)(void);
#ifdef SIM
static int s_failalloc;                    /* `fx failalloc <n>`: the next n block allocations fail */
#endif

static int napps(void) { return pc_fxapps(NULL); }
int fx_count(void) { return napps(); }
const pc_fxapp_t *fx_app(int i)
{
    const pc_fxapp_t *const *a;
    int n = pc_fxapps(&a);
    return i >= 0 && i < n ? a[i] : NULL;
}
/* an app index that is already known to be in range (a loop over fx_count(), or checked by the public entry): no NULL case */
static const pc_fxapp_t *AP(int i)
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

static void *fx_alloc(size_t n) { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
static void notify(void) { if (s_cb) s_cb(); }

/* ---------------- mode bits ---------------- */
uint8_t fx_mode_bits(void) { return __atomic_load_n(&s_mode, __ATOMIC_ACQUIRE); }
void fx_mode_set(uint32_t reason_bit, bool on)
{
    uint8_t b = (uint8_t)reason_bit;
    if (!b) return;
    uint8_t old = on ? __atomic_fetch_or(&s_mode, b, __ATOMIC_ACQ_REL) : __atomic_fetch_and(&s_mode, (uint8_t)~b, __ATOMIC_ACQ_REL);
    uint8_t now = on ? (uint8_t)(old | b) : (uint8_t)(old & ~b);
    if (now == old) return;
    printf("APP fx mode=%u heavy=%s\n", now, pc_fx_allowed(PC_FXC_HEAVY, now) ? "run" : "off");
}

/* ---------------- blocks and posting ---------------- */
static bool post(const pc_fxmsg_t *m)
{
    if (!s_x || !pc_fxchain_post(s_x->chain, m)) return false;
    s_x->nposts++;
    return true;
}

static void free_block(void *b)
{
    if (!b) return;
    heap_caps_free(b);
    s_x->live--;
}

static void free_pend(int i)
{
    free_block(s_x->pend[i]);
    s_x->pend[i] = NULL;
}

/* a block for app i from the current values, at max(rate seen, 48 kHz); NULL (and the fail flag) when memory is out */
static void *build_block(int i)
{
    ctx_t *x = s_x;
    uint32_t rate = pc_fxchain_rate_seen(x->chain);
    if (rate < 48000) rate = 48000;
    size_t n = pc_fxchain_block_size(i, rate, x->st.v[i]);
    void *b = NULL;
#ifdef SIM
    if (s_failalloc > 0) s_failalloc--; else
#endif
    b = n ? fx_alloc(n) : NULL;
    if (!b) {
        if (!x->fail[i]) printf("APP fx alloc fail app=%s bytes=%u\n", AP(i)->id, (unsigned)n);
        x->fail[i] = true;
        return NULL;
    }
    pc_fxchain_block_init(b, i, rate, x->st.v[i], x->st.mix[i]);
    x->live++;
    x->fail[i] = false;
    return b;
}

/* has the chain taken the SWAP that was last posted for app i out of the mailbox? */
static bool swap_taken(int i)
{
    ctx_t *x = s_x;
    if (!x->swap_seq[i]) return true;
    pc_fxchain_stat_t st;
    pc_fxchain_stat(x->chain, &st);
    uint32_t taken = x->nposts - (uint32_t)(PC_FXMAIL_N - st.mail_free);
    return (int32_t)(taken - x->swap_seq[i]) >= 0;
}

static void sync_app(int i)
{
    ctx_t *x = s_x;
    if (!x || !x->chain) return;
    pc_fxstate_t *st = &x->st;
    if (!st->on[i]) {
        free_pend(i);                                       /* never reached the chain */
        if (x->posted[i]) {
            pc_fxmsg_t m = { .op = PC_FXOP_OFF, .app = (uint8_t)i };
            if (post(&m)) x->posted[i] = false;             /* the chain fades it out and hands the block back */
        }
        if (!x->posted[i]) { x->rebuild[i] = false; x->dirty[i] = 0; }   /* pending work goes only once the chain has no block coming: an OFF that could not be posted must not eat the changes of a following on */
        return;
    }
    bool want = !x->posted[i] || x->rebuild[i];
    if (want && !x->pend[i] && (!x->fail[i] || x->rebuild[i]) && (!x->posted[i] || swap_taken(i))) {
        x->pend[i] = build_block(i);
        if (x->pend[i]) { x->rebuild[i] = false; x->dirty[i] = 0; }   /* the new block carries every value as it is now */
    }
    if (x->pend[i]) {
        pc_fxmsg_t m = { .op = PC_FXOP_SWAP, .app = (uint8_t)i, .ptr = x->pend[i] };
        if (post(&m)) { x->pend[i] = NULL; x->posted[i] = true; x->swap_seq[i] = x->nposts; }
    }
    if (!x->posted[i] || x->pend[i]) return;                /* SETs only after the SWAP they belong to */
    const pc_fxapp_t *ap = AP(i);
    for (int p = 0; p < ap->nparam && x->dirty[i]; p++) {
        if (!(x->dirty[i] & (1ull << p))) continue;
        pc_fxmsg_t m = { .op = PC_FXOP_SET, .app = (uint8_t)i, .param = (uint8_t)p, .v = st->v[i][p] };
        if (!post(&m)) return;
        x->dirty[i] &= ~(1ull << p);
    }
    if (x->dirty[i] & DIRTY_MIX) {
        pc_fxmsg_t m = { .op = PC_FXOP_MIX, .app = (uint8_t)i, .v = st->mix[i] };
        if (post(&m)) x->dirty[i] &= ~DIRTY_MIX;
    }
}

static void take_returns(void)
{
    pc_fxret_t r;
    while (pc_fxchain_take(s_x->chain, &r)) {
        if (r.op == PC_FXRET_FREE) free_block(r.ptr);
        else if (r.op == PC_FXRET_REBUILD && r.app < napps()) { s_x->rebuild[r.app] = true; s_x->fail[r.app] = false; }
    }
}

/* ---------------- the values ---------------- */
static void touch(void) { int64_t t = esp_timer_get_time(); s_x->dirty_at = t ? t : 1; }   /* 0 means "nothing to store" (the simulator starts its clock at 0) */

/* a value into the table; true = it changed (dirty bit or rebuild noted when the app is on) */
static bool put_value(int i, int p, float v)
{
    ctx_t *x = s_x;
    const pc_fxapp_t *ap = AP(i);
    float *cur;
    uint64_t bit;
    if (p == FX_P_MIX) {
        if (ap->flags & PC_FXA_MIX) { cur = &x->st.mix[i]; bit = DIRTY_MIX; }
        else { p = pfind(ap, "mix"); if (p < 0) return false; cur = &x->st.v[i][p]; bit = 1ull << p; }
    } else {
        if (p < 0 || p >= ap->nparam) return false;
        cur = &x->st.v[i][p];
        bit = 1ull << p;
    }
    v = pc_fxstate_clamp(i, cur == &x->st.mix[i] ? FX_P_MIX : p, v);
    if (*cur == v) return false;
    *cur = v;
    if (!x->st.on[i]) return true;
    if (cur != &x->st.mix[i] && (ap->param[p].flags & PC_FXP_REBUILD)) { free_pend(i); x->rebuild[i] = true; }   /* a new block with the new header, no SET (the chain ignores one) */
    else x->dirty[i] |= bit;
    return true;
}

static void put_on(int i, bool on)
{
    ctx_t *x = s_x;
    if (x->st.on[i] == on) return;
    x->st.on[i] = on;
    if (on) x->fail[i] = false;                             /* a tap on 開 tries the allocation again */
}

float fx_get(int i, int p)
{
    const pc_fxapp_t *ap = fx_app(i);
    if (!s_x || !ap) return 0;
    if (p == FX_P_MIX) {
        if (ap->flags & PC_FXA_MIX) return s_x->st.mix[i];
        int mi = pfind(ap, "mix");
        return mi >= 0 ? s_x->st.v[i][mi] : 1.0f;
    }
    return p >= 0 && p < ap->nparam ? s_x->st.v[i][p] : 0;
}

bool fx_on(int i) { return s_x && fx_app(i) && s_x->st.on[i]; }

static void changed(void)                                    /* a state change that is not a stored import */
{
    s_x->st.state_from[0] = 0;
    touch();
}

static void fx_set_on_u(int i, bool on)
{
    if (!s_x || !fx_app(i) || s_x->st.on[i] == on) return;
    put_on(i, on);
    changed();
    sync_app(i);
    notify();
}

static void fx_set_u(int i, int p, float v)
{
    if (!s_x || !fx_app(i)) return;
    if (!put_value(i, p, v)) return;
    changed();
    sync_app(i);
    notify();                                               /* an open filter page shows the new value (the page callback runs on this task: LVGL) */
}

void fx_reset_app(int i)
{
    const pc_fxapp_t *ap = fx_app(i);
    if (!s_x || !ap) return;
    bool any = false;
    for (int p = 0; p < ap->nparam; p++) any |= put_value(i, p, ap->param[p].def);
    if (ap->flags & PC_FXA_MIX) any |= put_value(i, FX_P_MIX, pc_fxapp_mix_param.def);
    if (!any) return;
    changed();
    sync_app(i);
    notify();
}

int fx_on_count(void)
{
    int k = 0;
    for (int i = 0; s_x && i < napps(); i++) k += s_x->st.on[i];
    return k;
}
void fx_state_running(int *run, int *paused, const char **why)
{
    int r = 0, d = 0;
    const char *w = NULL;
    for (int i = 0; s_x && i < napps(); i++) {
        const char *wi;
        fx_look_t lk = fx_look(i, &wi);
        if (lk == FX_RUN) r++;
        else if (lk == FX_DIM) { d++; if (!w) w = wi; }
    }
    if (run) *run = r;
    if (paused) *paused = d;
    if (why) *why = w;
}
const char *fx_state_from(void) { return s_x ? s_x->st.state_from : ""; }

fx_look_t fx_look(int i, const char **why)
{
    if (why) *why = NULL;
    const pc_fxapp_t *ap = fx_app(i);
    if (!s_x || !ap) return FX_OFF;
    uint8_t mode = fx_mode_bits();
    bool zh = false, allowed = pc_fx_app_allowed(ap, mode);
    if (!s_x->st.on[i]) { if (why && !allowed) *why = pc_fx_app_why(ap, mode, zh); return FX_OFF; }
    if (s_x->fail[i]) { if (why) *why = "Out of memory"; return FX_DIM; }
    if (!allowed) { if (why) *why = pc_fx_app_why(ap, mode, zh); return FX_DIM; }
    if (s_x->rebuild[i] || s_x->pend[i]) { if (why) *why = "Preparing"; return FX_DIM; }
    return FX_RUN;
}

void fx_set_change_cb(void (*cb)(void)) { s_cb = cb; }

/* ---------------- card: fx.txt ---------------- */
static int save_now(void)
{
    pc_kv_t *kv = fx_alloc(KV_MAX * sizeof *kv);
    if (!kv) return -1;
    int n = pc_fxstate_to_kv(&s_x->st, kv, KV_MAX), rc = n > 0 ? pc_kv_save(FX_STATE_PATH, kv, n) : -1;
    heap_caps_free(kv);
    printf("APP fx store n=%d rc=%d\n", n, rc);
    if (rc == 0) {
        s_x->dirty_at = 0;
    } else touch();                                           /* try again in 3 s (a full or missing card) */
    return rc;
}

void fx_state_save_now(void)
{
    if (s_x && s_x->dirty_at && nsn_storage_mounted()) save_now();
}

/* ---------------- import / export / library ---------------- */
static int cmp(const void *a, const void *b) { return strcmp(a, b); }

int fx_list(char (*names)[48], int max)
{
    DIR *d = opendir(FX_DIR);
    if (!d) return 0;
    int n = 0;
    struct dirent *e;
    while (n < max && (e = readdir(d))) {
        size_t l = strlen(e->d_name);
        if (e->d_name[0] != '.' && l > 5 && !strcasecmp(e->d_name + l - 5, ".toml")) snprintf(names[n++], 48, "%s", e->d_name);
    }
    closedir(d);
    qsort(names, (size_t)n, 48, cmp);
    return n;
}

char *fx_read(const char *name)
{
    char path[128];
    if (!name[0] || strchr(name, '/')) return NULL;
    snprintf(path, sizeof path, FX_DIR "/%s", name);
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *text = n > 0 && n < 65536 ? fx_alloc((size_t)n + 1) : NULL;   /* PSRAM also under 1 KB (malloc would put a small preset in internal RAM; final review M4) */
    size_t got = text ? fread(text, 1, (size_t)n, f) : 0;
    fclose(f);
    if (!text || got != (size_t)n) { heap_caps_free(text); return NULL; }
    text[n] = 0;
    return text;
}

int fx_unsupported(const char **out, int max)
{
    int n = 0;
    for (int i = 0; s_x && i < s_x->nunsup && n < max; i++) out[n++] = s_x->unsup[i];
    return n;
}

int fx_import_text(const char *toml, bool store, const char **unsup, int max)
{
    if (!s_x) return -1;
    ctx_t *x = s_x;
    pc_preset_t *p = fx_alloc(sizeof *p);
    if (!p) return -1;
    int rc = pc_preset_parse(toml, p);
    if (rc) { heap_caps_free(p); printf("APP fx parse failed rc=%d\n", rc); return rc < 0 ? rc : -1; }   /* nothing changed */
    x->tmp = x->st;
    x->nunsup = pc_fxstate_import(&x->tmp, p, x->unsup, 8);
    printf("APP fx import music=%d master=%d sends=%d unsupported=%d store=%d\n", p->nmusic, p->nmaster, p->nsends, x->nunsup, store);
    heap_caps_free(p);
    for (int i = 0; i < napps(); i++) {                    /* values first, then on/off: a block made for an app that turns on carries the final values */
        const pc_fxapp_t *ap = AP(i);
        for (int q = 0; q < ap->nparam; q++) put_value(i, q, x->tmp.v[i][q]);
        if (ap->flags & PC_FXA_MIX) put_value(i, FX_P_MIX, x->tmp.mix[i]);
        put_on(i, x->tmp.on[i]);
        if (x->tmp.on[i]) x->fail[i] = false;               /* choosing a sound is a deliberate act: one more try for an app that is on */
    }
    x->st.state_from[0] = 0;
    if (store) touch();
    for (int i = 0; i < napps(); i++) sync_app(i);
    for (int i = 0; unsup && i < x->nunsup && i < max; i++) unsup[i] = x->unsup[i];
    notify();
    return 0;
}

char *fx_export_text(void)
{
    if (!s_x) return NULL;
    char *t = fx_alloc(TXT_MAX);
    if (!t) return NULL;
    if (pc_fxstate_export(&s_x->st, t, TXT_MAX) < 0) { heap_caps_free(t); return NULL; }
    return t;
}

int fx_select(const char *name)
{
    if (!s_x) return -1;
    if (!name[0]) {                                        /* every app off, the values stay */
        for (int i = 0; i < napps(); i++) { put_on(i, false); sync_app(i); }
        changed();
        notify();
        return 0;
    }
    char *text = fx_read(name);
    if (!text) return -1;
    int rc = fx_import_text(text, true, NULL, 0);
    heap_caps_free(text);
    if (rc) return rc;
    snprintf(s_x->st.state_from, sizeof s_x->st.state_from, "%s", name);
    return 0;
}

int fx_save(const char *name, const char *text)
{
    pc_preset_t *p = fx_alloc(sizeof *p);
    int rc = p ? pc_preset_parse(text, p) : -1;              /* never write a file that does not load */
    heap_caps_free(p);
    if (rc || !name[0] || strchr(name, '/') || strlen(name) > 40) return -1;
    char path[128];
    mkdir("/sdcard/_player", 0777); mkdir("/sdcard/_player/config", 0777); mkdir(FX_DIR, 0777);
    snprintf(path, sizeof path, FX_DIR "/%s", name);
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    size_t n = strlen(text), w = fwrite(text, 1, n, f);
    if (fclose(f) || w != n) return -1;
    return fx_select(name);
}

/* ---------------- service ---------------- */
static uint32_t look_sig(void)
{
    uint32_t h = fx_mode_bits();
    for (int i = 0; i < napps(); i++) h = h * 31u + (uint32_t)fx_look(i, NULL);
    return h;
}

static void tick(void)
{
    ctx_t *x = s_x;
    take_returns();                                         /* every return first: a full return ring blocks the whole mailbox */
    nsn_player_info_t ai; nsn_player_info(&ai);
    bool bt = ai.active && nsn_player_sink() == 2;          /* a Bluetooth sink carries the sound: velvet_room is wired-only */
    if (bt != x->bt) { x->bt = bt; fx_mode_set(PC_FXM_BT, bt); }
    bool ldac = bt && !strcmp(nsn_bt_codec_name(), "LDAC");  /* LDAC encoding takes about half a core: the heavy apps step aside (round's rule) */
    if (ldac != x->ldac) { x->ldac = ldac; fx_mode_set(PC_FXM_LDAC, ldac); }
    uint8_t rb = ai.active ? pc_fx_rate_mode(ai.rate) : 0;
    if (rb != x->rate_bits) {
        uint8_t ch = (uint8_t)(rb ^ x->rate_bits);
        x->rate_bits = rb;
        if (ch & PC_FXM_HIRES) fx_mode_set(PC_FXM_HIRES, rb & PC_FXM_HIRES);
        if (ch & PC_FXM_HIRES2) fx_mode_set(PC_FXM_HIRES2, rb & PC_FXM_HIRES2);
    }
    /* load shedding: fx are optional. When core 1 (decoder + encoder + fx) stays at 90 % or more for two seconds with heavy apps
     * running, they are paused (the same bit a battery uses on JukeDial: heavy off, light stay) until this playback ends or the
     * output changes. Measured 2026-11-25: AAC to Bluetooth is 45-49 % of core 1 by itself, all eleven apps add about 60 %. */
    {
        static uint32_t last_idle; static int64_t last_us; static int hot, last_sink;
        int64_t now = esp_timer_get_time();
        if (now - last_us >= 1000000) {
            uint32_t idle = (uint32_t)ulTaskGetIdleRunTimeCounterForCore(1);
            int load = last_us ? 100 - (int)((uint64_t)(uint32_t)(idle - last_idle) * 100 / (uint64_t)(now - last_us)) : 0;
            last_idle = idle; last_us = now;
            int sink = ai.active ? nsn_player_sink() : 0;
            if (x->shed && (!ai.active || sink != last_sink)) { x->shed = false; fx_mode_set(PC_FXM_BATT, false); ESP_LOGI("fx", "heavy effects allowed again"); }
            last_sink = sink;
            bool heavy_on = false;
            for (int i = 0; i < napps(); i++) if (x->st.on[i] && AP(i)->cls == PC_FXC_HEAVY) heavy_on = true;
            hot = (ai.active && heavy_on && !x->shed && load >= 90) ? hot + 1 : 0;
            if (hot >= 2) { hot = 0; x->shed = true; fx_mode_set(PC_FXM_BATT, true); ESP_LOGW("fx", "core 1 at %d %%: heavy effects paused for this playback", load); }
        }
    }
    for (int i = 0; i < napps(); i++) sync_app(i);
    if (x->dirty_at && nsn_storage_mounted() && esp_timer_get_time() - x->dirty_at >= SAVE_AFTER_US) save_now();
}

static void lk(void); static void ul(void);
static void fx_task(void *arg)
{
    for (;;) { vTaskDelay(pdMS_TO_TICKS(50)); lk(); tick(); ul(); }
}

static void load_state(void)
{
    ctx_t *x = s_x;
    pc_kv_t *kv = fx_alloc(KV_MAX * sizeof *kv);
    int n = kv ? pc_kv_load(FX_STATE_PATH, kv, KV_MAX) : 0;
    if (n > 0) pc_fxstate_from_kv(&x->st, kv, n);
    heap_caps_free(kv);
    if (n > 0) return;
    printf("APP fx state missing\n");
}

void nsn_fx_start(void)
{
    ctx_t *x = fx_alloc(sizeof *x);
    if (!x) { printf("APP fx state alloc failed\n"); return; }
    memset(x, 0, sizeof *x);
    pc_fxstate_defaults(&x->st);
    x->chain = pc_fxchain_create(fx_alloc);
    if (!x->chain) { printf("APP fx chain alloc failed\n"); heap_caps_free(x); return; }
    s_x = x;
    s_mx = xSemaphoreCreateRecursiveMutex();
    load_state();
    for (int i = 0; i < napps(); i++) sync_app(i);          /* the SWAPs wait in the mailbox; the first audio block installs them */
    x->look_sig = look_sig();
    xTaskCreatePinnedToCoreWithCaps(fx_task, "fx", 4096, NULL, 3, NULL, 0, MALLOC_CAP_SPIRAM);
}

/* ---------------- console ---------------- */
static int app_by_id(const char *id) { return pc_fxapp_find(id); }

/* console output: lines go to the caller's sink (UART or BLE) when one is set, else stdout */
static void (*s_sink)(void *, const char *); static void *s_sink_ctx;
void fx_set_out(void (*sink)(void *, const char *), void *ctx) { s_sink = sink; s_sink_ctx = ctx; }
static void fxp(const char *fmt, ...)
{
    static char ln[600]; static size_t n;
    char b[400]; va_list ap; va_start(ap, fmt); int k = vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    if (k < 0) return;
    for (int j = 0; j < k && j < (int)sizeof b - 1; j++) {
        if (b[j] == '\n') { ln[n] = 0; if (s_sink) s_sink(s_sink_ctx, ln); else printf("%s\n", ln); n = 0; }
        else if (n < sizeof ln - 1) ln[n++] = b[j];
    }
}

static void print_state(bool full)                          /* full: every band, also those past the count (tools/simtools/ui_baseline.py puts the whole table back) */
{
    ctx_t *x = s_x;
    fxp("APP fx state n=%d mode=%u from=%s\n", napps(), fx_mode_bits(), x ? x->st.state_from : "");
    for (int i = 0; x && i < napps(); i++) {
        const pc_fxapp_t *ap = AP(i);
        fx_look_t lk = fx_look(i, NULL);
        fxp("APP fx app=%s cls=%s on=%d run=%d look=%s", ap->id, ap->cls == PC_FXC_HEAVY ? "heavy" : "light", x->st.on[i], lk == FX_RUN, lk == FX_OFF ? "off" : lk == FX_RUN ? "run" : "dim");
        int bands = (int)(ap->flags & PC_FXA_BANDS ? x->st.v[i][pfind(ap, "count")] : 0);
        for (int p = 0; p < ap->nparam; p++) {
            if (!full && (ap->param[p].flags & PC_FXP_BAND)) {   /* only the bands that count */
                int first = 0;
                while (first < ap->nparam && !(ap->param[first].flags & PC_FXP_BAND)) first++;
                if ((p - first) / 4 >= bands) continue;
            }
            fxp(" %s=%g", ap->param[p].id, (double)x->st.v[i][p]);
        }
        if (ap->flags & PC_FXA_MIX) fxp(" mix=%g", (double)x->st.mix[i]);
        fxp("\n");
    }
}

static bool fx_console_u(const char *c)
{
    char a[40], b[40];
    float v;
    int n;
    if (!strcmp(c, "fxs")) { static char nm[16][48]; int k = fx_list(nm, 16); for (int i = 0; i < k; i++) fxp("APP fx file %s\n", nm[i]); return true; }
    if (strncmp(c, "fx ", 3)) return false;
    const char *r = c + 3;
    if (!strcmp(r, "state")) print_state(false);
    else if (!strcmp(r, "state full")) print_state(true);
    else if (!strcmp(r, "params")) {                         /* one line per tunable value: app param lo hi def cur (generic UI) */
        for (int i = 0; s_x && i < napps(); i++) {
            const pc_fxapp_t *ap = AP(i);
            fxp("APP fxp %s on %d %s\n", ap->id, s_x->st.on[i], ap->en ? ap->en : ap->id);
            for (int p = 0; p < ap->nparam; p++) {
                if (ap->param[p].flags & (PC_FXP_BAND | PC_FXP_REBUILD)) continue;
                fxp("APP fxp %s %s %g %g %g %g\n", ap->id, ap->param[p].id, (double)ap->param[p].lo, (double)ap->param[p].hi, (double)ap->param[p].def, (double)s_x->st.v[i][p]);
            }
            if (ap->flags & PC_FXA_BANDS) {                  /* the equalizer's band rows: APP fxb <app> <band> <type 0=P 1=L 2=H> <fc> <gain dB> <q>; set with b<n>_type / _fc / _gain_db / _q */
                int first = 0;
                while (first < ap->nparam && !(ap->param[first].flags & PC_FXP_BAND)) first++;
                for (int bnd = 0; first + 4 * bnd + 3 < ap->nparam; bnd++) {
                    const float *v = &s_x->st.v[i][first + 4 * bnd];
                    fxp("APP fxb %s %d %g %g %g %g\n", ap->id, bnd, (double)v[0], (double)v[1], (double)v[2], (double)v[3]);
                }
            }
        }
    }
    else if (!strcmp(r, "stat")) {
        pc_fxchain_stat_t st = { 0, 0, 0 };
        if (s_x) pc_fxchain_stat(s_x->chain, &st);
        fxp("APP fx stat mail_free=%d ret_free=%d blocks=%d live=%d psram_free=%u\n", st.mail_free, st.ret_free, st.blocks, s_x ? s_x->live : 0, (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    }
    else if (sscanf(r, "on %39s %d", a, &n) == 2) {
        int i = app_by_id(a);
        if (i < 0) fxp("APP cmd fx unknown app %s\n", a); else fx_set_on(i, n != 0);
    }
    else if (sscanf(r, "set %39s %39s %f", a, b, &v) == 3) {
        int i = app_by_id(a), p = -1;
        if (i >= 0) p = !strcmp(b, "mix") ? FX_P_MIX : pfind(AP(i), b);
        if (i < 0 || (p < 0 && p != FX_P_MIX)) fxp("APP cmd fx unknown %s %s\n", a, b); else fx_set(i, p, v);
    }
    else if (sscanf(r, "mode %d", &n) == 1) { for (int k = 1; k < 4; k++) fx_mode_set(1u << k, (n >> k) & 1); fx_mode_set(PC_FXM_BT, (n & PC_FXM_BT) != 0); }   /* aptX / LDAC / CAST / BT; BATT (bit 0) stays with the power tick, the rate bits with the pipeline */
    else if (!strcmp(r, "off")) fx_select("");
    else if (!strncmp(r, "import ", 7)) fxp("APP cmd fx rc=%d\n", fx_select(r + 7));
    else if (!strcmp(r, "export")) {
        char *t = fx_export_text();
        for (char *l = t, *e; l && *l; l = e ? e + 1 : NULL) { e = strchr(l, '\n'); fxp("APP fxexp %.*s\n", e ? (int)(e - l) : (int)strlen(l), l); if (!e) break; }
        heap_caps_free(t);
    }
#ifdef SIM
    else if (sscanf(r, "failalloc %d", &n) == 1) s_failalloc = n;
#endif
    else fxp("APP cmd fx unknown\n");
    return true;
}

/* audio side: called by the player for every decoded block (float stereo, at the track rate), in place. */
void nsn_fx_process(float *lr, int frames, uint32_t rate)
{
    if (!s_x || !s_x->chain) return;
    uint8_t mode = (uint8_t)(fx_mode_bits() | pc_fx_rate_mode(rate));
    pc_fxchain_process(s_x->chain, lr, frames, rate, mode);
}

static void lk(void) { if (s_mx) xSemaphoreTakeRecursive(s_mx, portMAX_DELAY); }
static void ul(void) { if (s_mx) xSemaphoreGiveRecursive(s_mx); }
void fx_set_on(int i, bool on) { lk(); fx_set_on_u(i, on); ul(); }
void fx_set(int i, int p, float v) { lk(); fx_set_u(i, p, v); ul(); }
bool fx_console(const char *c) { lk(); bool r = fx_console_u(c); ul(); return r; }

bool nsn_fx_shed(void) { return s_x && s_x->shed; }
