// SPDX-License-Identifier: LGPL-2.1-or-later
/* The fixed chain order (owner point 3, spec §2): THE one const table. Each port inserts its app at its place. */
#include <math.h>
#include <string.h>
#include "pc/pc_fxchain.h"

static const pc_fxapp_t *const ORDER[] = {
    &pc_fxapp_gain,
    &pc_fxapp_balance, &pc_fxapp_widener, &pc_fxapp_crossfeed, &pc_fxapp_velvet_widener,  /* stereo difference */
    &pc_fxapp_equalizer, &pc_fxapp_tone, &pc_fxapp_bass, &pc_fxapp_loudness,           /* tone */
    &pc_fxapp_tube,                                                                     /* tube */
    &pc_fxapp_compressor,
    &pc_fxapp_freeverb,                                                                 /* reverb */
#if CONFIG_FX_VELVET_ROOM
    &pc_fxapp_velvet_room,
#endif
    &pc_fxapp_limiter,                                                                  /* always last (PC_FXA_ALWAYS_LAST, checked by test_fxchain_order) */
};
#define NORDER ((int)(sizeof ORDER / sizeof *ORDER))
_Static_assert(sizeof ORDER / sizeof *ORDER <= PC_FXAPP_MAX, "order table larger than PC_FXAPP_MAX");

int pc_fxapps(const pc_fxapp_t *const **out)
{
    if (out) *out = ORDER;
    return NORDER;
}

int pc_fxapp_find(const char *id)
{
    if (!id) return -1;
    for (int i = 0; i < NORDER; i++)
        if (!strcmp(ORDER[i]->id, id)) return i;
    return -1;
}


/* ---------------------------------------------------------------------------------------------------------------
 * The chain (spec §3). One allocation (pc_fxchain_create): the struct (slots), the two scratch blocks, both rings.
 * A state block = header (values copy, mix, rate) + the app's state, never moved after init. */

typedef struct { float v[PC_FXP_MAX]; float mix; uint32_t rate; } hdr_t;
#define HDR ((sizeof(hdr_t) + 15u) & ~(size_t)15u)

typedef struct {
    void *blk;                 /* the block that runs (NULL: app off, nothing installed) */
    void *old;                 /* a replaced / dropped block: held at full while blk warms up, then fading out; then handed back (FREE) */
    bool on;                   /* user's on (SWAP sets, OFF clears) */
    bool run;                  /* blk was allowed to run in the last block (mode, rate): what separates "warming" from "suspended" for pc_fxchain_busy */
    bool warm;                 /* blk's look-ahead line is filled: its fade may start (apps without latency: at once) */
    uint32_t warmed;           /* frames blk has run silently towards warm */
    uint32_t rb_rate;          /* the rate a REBUILD was posted for (0: none pending) */
    uint32_t changed;          /* non-REBUILD SETs drained this block, one set() per app */
    pc_fx_ramp_t fade, mix;    /* of blk: fade moves to run ? 1 : 0, mix is the chain's dry/wet */
    float ofade, omix;         /* of old: the level it had when replaced, the mix it had */
} slot_t;

struct pc_fxchain {
    slot_t slot[PC_FXAPP_MAX];
    const pc_fxapp_t *const *tab;  int n;       /* the order (the registry; tests may pass another) */
    float *dry, *work;                          /* scratch, PC_FX_BLOCK stereo frames each */
    pc_fxmsg_t *mail;  pc_fxret_t *ret;
    uint32_t mh, mt, rh, rt;                    /* mailbox head (LVGL) / tail (audio); return ring head (audio) / tail (LVGL) */
    uint32_t rate_seen;
    int blocks;
    void (*trace)(const pc_fxmsg_t *, void *);  void *trace_u;
    void (*probe)(int, void *);                 void *probe_u;
};

#define LD(p) __atomic_load_n(&(p), __ATOMIC_ACQUIRE)
#define LDR(p) __atomic_load_n(&(p), __ATOMIC_RELAXED)
#define ST(p, v) __atomic_store_n(&(p), (v), __ATOMIC_RELEASE)
static inline size_t up16(size_t n) { return (n + 15u) & ~(size_t)15u; }

pc_fxchain_t *pc_fxchain_create_with(void *(*psram_alloc)(size_t), const pc_fxapp_t *const *tab, int n)
{
    if (!tab || n < 0 || n > PC_FXAPP_MAX) return NULL;
    for (int i = 0; i < n; i++)
        if (!tab[i] || tab[i]->nparam > PC_FXP_MAX) return NULL;     /* the header holds PC_FXP_MAX values */
    size_t o_scr = up16(sizeof(struct pc_fxchain));
    size_t o_mail = o_scr + 2 * up16(PC_FX_BLOCK * 2 * sizeof(float));
    size_t o_ret = o_mail + up16(PC_FXMAIL_N * sizeof(pc_fxmsg_t));
    size_t total = o_ret + up16(PC_FXRET_N * sizeof(pc_fxret_t));
    char *m = psram_alloc ? psram_alloc(total) : NULL;
    if (!m) return NULL;
    memset(m, 0, total);
    pc_fxchain_t *c = (pc_fxchain_t *)m;
    c->tab = tab; c->n = n;
    c->dry = (float *)(m + o_scr);
    c->work = (float *)(m + o_scr + up16(PC_FX_BLOCK * 2 * sizeof(float)));
    c->mail = (pc_fxmsg_t *)(m + o_mail);
    c->ret = (pc_fxret_t *)(m + o_ret);
    return c;
}

pc_fxchain_t *pc_fxchain_create(void *(*psram_alloc)(size_t)) { return pc_fxchain_create_with(psram_alloc, ORDER, NORDER); }

void pc_fxchain_destroy(pc_fxchain_t *c, void (*release)(void *))
{
    if (!c) return;
    for (int i = 0; i < c->n; i++) {
        if (c->slot[i].blk) release(c->slot[i].blk);
        if (c->slot[i].old) release(c->slot[i].old);
    }
    for (uint32_t t = c->mt; t != c->mh; t++) {
        const pc_fxmsg_t *m = &c->mail[t % PC_FXMAIL_N];
        if (m->op == PC_FXOP_SWAP && m->ptr) release(m->ptr);
    }
    for (uint32_t t = c->rt; t != c->rh; t++) {
        const pc_fxret_t *r = &c->ret[t % PC_FXRET_N];
        if (r->op == PC_FXRET_FREE && r->ptr) release(r->ptr);
    }
    release(c);
}

size_t pc_fxchain_block_size(int app, uint32_t rate, const float *v)
{
    if (app < 0 || app >= NORDER) return 0;
    return HDR + ORDER[app]->state_size(rate, v);
}

void *pc_fxchain_block_init(void *blk, int app, uint32_t rate, const float *v, float mix)
{
    if (!blk || app < 0 || app >= NORDER) return NULL;
    const pc_fxapp_t *ap = ORDER[app];
    hdr_t *h = blk;
    memset(h, 0, sizeof *h);
    memcpy(h->v, v, ap->nparam * sizeof(float));
    h->mix = isfinite(mix) ? fminf(fmaxf(mix, 0.0f), 1.0f) : 1.0f;
    h->rate = rate;
    ap->init((char *)blk + HDR, rate, h->v);
    return blk;
}

bool pc_fxchain_post(pc_fxchain_t *c, const pc_fxmsg_t *m)
{
    uint32_t h = LDR(c->mh), t = LD(c->mt);
    if (h - t >= PC_FXMAIL_N) return false;
    c->mail[h % PC_FXMAIL_N] = *m;
    ST(c->mh, h + 1);
    return true;
}

bool pc_fxchain_take(pc_fxchain_t *c, pc_fxret_t *out)
{
    uint32_t t = LDR(c->rt), h = LD(c->rh);
    if (t == h) return false;
    *out = c->ret[t % PC_FXRET_N];
    ST(c->rt, t + 1);
    return true;
}

static bool ret_push(pc_fxchain_t *c, uint8_t op, int app, void *ptr)
{
    uint32_t h = LDR(c->rh), t = LD(c->rt);
    if (h - t >= PC_FXRET_N) return false;
    c->ret[h % PC_FXRET_N] = (pc_fxret_t){ .op = op, .app = (uint8_t)app, .ptr = ptr };
    ST(c->rh, h + 1);
    return true;
}

uint32_t pc_fxchain_rate_seen(const pc_fxchain_t *c) { return LD(c->rate_seen); }

void pc_fxchain_stat(const pc_fxchain_t *c, pc_fxchain_stat_t *out)
{
    int m = (int)(PC_FXMAIL_N - (LD(c->mh) - LD(c->mt))), r = (int)(PC_FXRET_N - (LD(c->rh) - LD(c->rt)));
    out->mail_free = m < 0 ? 0 : m > PC_FXMAIL_N ? PC_FXMAIL_N : m;
    out->ret_free = r < 0 ? 0 : r > PC_FXRET_N ? PC_FXRET_N : r;
    out->blocks = LDR(c->blocks);
}

void pc_fxchain_set_trace(pc_fxchain_t *c, void (*fn)(const pc_fxmsg_t *m, void *u), void *u) { c->trace = fn; c->trace_u = u; }
void pc_fxchain_set_probe(pc_fxchain_t *c, void (*fn)(int app, void *u), void *u) { c->probe = fn; c->probe_u = u; }

/* ---- audio side ---- */
static inline hdr_t *hdr_of(void *blk) { return blk; }
static inline void *st_of(void *blk) { return (char *)blk + HDR; }
static inline uint32_t lat_of(const pc_fxapp_t *ap, void *blk)
{
    int l = ap->latency ? ap->latency(st_of(blk)) : 0;
    return l > 0 ? (uint32_t)l : 0;
}

/* a block that no longer contributes goes back to the LVGL side; a full ring keeps it here (silent) for the next block */
static void retire(pc_fxchain_t *c, int i)
{
    slot_t *s = &c->slot[i];
    if (!s->old && s->blk && !s->on && s->fade.cur <= 0.0f) {     /* turned off (or never heard) */
        s->old = s->blk; s->blk = NULL; s->ofade = 0.0f;
        s->rb_rate = 0; s->changed = 0; s->warm = false; s->warmed = 0;
    }
    if (s->old && s->ofade <= 0.0f && ret_push(c, PC_FXRET_FREE, i, s->old)) s->old = NULL;
}

/* false = not applicable now (stays in the mailbox: the next block retries) */
static bool apply(pc_fxchain_t *c, const pc_fxmsg_t *m)
{
    int i = m->app;
    if (i >= c->n) {                                        /* a bug upstream; do not leak the block if we can help it */
        if (m->op == PC_FXOP_SWAP && m->ptr) return ret_push(c, PC_FXRET_FREE, 0, m->ptr);
        return true;
    }
    slot_t *s = &c->slot[i];
    const pc_fxapp_t *ap = c->tab[i];
    switch (m->op) {
    case PC_FXOP_SET:
        /* REBUILD parameters change only through SWAP (the new block's header carries them): ignored here */
        if (s->blk && m->param < ap->nparam && !(ap->param[m->param].flags & PC_FXP_REBUILD)) {
            hdr_of(s->blk)->v[m->param] = m->v;
            s->changed |= 1u << (m->param < 31 ? m->param : 31);
        }
        break;
    case PC_FXOP_SWAP:
        if (!m->ptr) break;
        if (s->blk) {
            if (s->fade.cur <= 0.0f) {                      /* never heard (fresh, warming, suspended): straight back */
                if (!ret_push(c, PC_FXRET_FREE, i, s->blk)) return false;
            } else {
                if (s->old) return false;                   /* the previous retiree has not left yet */
                s->old = s->blk; s->ofade = s->fade.cur; s->omix = s->mix.cur;
            }
        }
        s->blk = m->ptr; s->on = true; s->changed = 0; s->rb_rate = 0;
        s->warm = false; s->warmed = 0;
        s->fade.cur = s->fade.tgt = 0.0f;
        s->mix.cur = s->mix.tgt = hdr_of(s->blk)->mix;
        break;
    case PC_FXOP_OFF:
        if (!s->blk) break;
        if (s->old && s->fade.cur <= 0.0f) {                /* the new block never got heard; the old one fades out as itself */
            if (!ret_push(c, PC_FXRET_FREE, i, s->blk)) return false;
            s->blk = s->old; s->old = NULL;
            s->fade.cur = s->fade.tgt = s->ofade; s->mix.cur = s->mix.tgt = s->omix;
            s->warm = true; s->warmed = 0; s->ofade = 0.0f; s->changed = 0;
        }
        s->on = false;
        break;
    case PC_FXOP_MIX:
        if (s->blk && isfinite(m->v)) {
            float v = fminf(fmaxf(m->v, 0.0f), 1.0f);
            hdr_of(s->blk)->mix = v;
            pc_fx_ramp_set(&s->mix, v);
        }
        break;
    default: break;
    }
    return true;
}

static void drain(pc_fxchain_t *c)
{
    uint32_t h = LD(c->mh), t = LDR(c->mt);
    while (t != h) {
        const pc_fxmsg_t *m = &c->mail[t % PC_FXMAIL_N];
        if (!apply(c, m)) break;
        if (c->trace) c->trace(m, c->trace_u);
        t++;
        ST(c->mt, t);
    }
    for (int i = 0; i < c->n; i++) {
        slot_t *s = &c->slot[i];
        if (s->changed && s->blk) c->tab[i]->set(st_of(s->blk), hdr_of(s->blk)->v, s->changed);
        s->changed = 0;
    }
}

/* lr += (process(copy of dry) - dry) * g(i); g from the ramps (the slot's block) or linear from `a` to `b` (a retiree) */
static void contribute(const pc_fxapp_t *ap, void *blk, float *lr, const float *dry, float *work, int frames,
                       const pc_fx_ramp_t *fade, const pc_fx_ramp_t *mix, float a, float b)
{
    memcpy(work, dry, (size_t)frames * 2 * sizeof(float));
    ap->process(st_of(blk), work, frames);
    for (int i = 0; i < frames; i++) {
        float g = fade ? pc_fx_ramp_at(fade, i, frames) * pc_fx_ramp_at(mix, i, frames)
                       : a + (b - a) * (float)(i + 1) / (float)frames;
        lr[2 * i] += (work[2 * i] - dry[2 * i]) * g;
        lr[2 * i + 1] += (work[2 * i + 1] - dry[2 * i + 1]) * g;
    }
}

static void run_block(pc_fxchain_t *c, float *lr, int frames, uint32_t rate, uint8_t mode)
{
    for (int i = 0; i < c->n; i++) retire(c, i);
    drain(c);
    for (int i = 0; i < c->n; i++) {
        slot_t *s = &c->slot[i];
        const pc_fxapp_t *ap = c->tab[i];
        bool ran = false, run = false, warming = false, cur_on = false;
        s->run = false;
        if (s->blk) {
            hdr_t *h = hdr_of(s->blk);
            bool fits = rate > 0 && ap->fits(st_of(s->blk), rate, h->v);
            run = s->on && fits && pc_fx_app_allowed(ap, mode);
            if (s->on && !fits) {
                if (s->rb_rate != rate && ret_push(c, PC_FXRET_REBUILD, i, NULL)) s->rb_rate = rate;
            } else if (fits) s->rb_rate = 0;
            if (fits && h->rate != rate && (run || s->fade.cur > 0.0f)) {
                ap->rate(st_of(s->blk), rate); h->rate = rate; s->warmed = 0;
            }
            if (run && !s->warm && s->warmed >= lat_of(ap, s->blk)) s->warm = true;   /* latency 0: at once */
            s->run = run;
            warming = run && !s->warm;                  /* runs silently until its look-ahead line is full */
            if (!run && !s->warm && s->warmed) { ap->reset(st_of(s->blk)); s->warmed = 0; }   /* suspended while warming: start over */
            pc_fx_ramp_set(&s->fade, run && s->warm ? 1.0f : 0.0f);
            cur_on = s->warm && (run || s->fade.cur > 0.0f);
        }
        bool old_on = s->old && s->ofade > 0.0f;
        bool hold = old_on && warming;                  /* the old block stays at full until the new one can be heard */
        if (old_on || cur_on || warming) {
            bool plain = !old_on && !warming && s->fade.cur >= 1.0f && s->fade.tgt >= 1.0f && s->mix.cur >= 1.0f && s->mix.tgt >= 1.0f;
            if (plain) ap->process(st_of(s->blk), lr, frames);      /* steady state, whole wet: in place */
            else {
                memcpy(c->dry, lr, (size_t)frames * 2 * sizeof(float));
                if (old_on) contribute(ap, s->old, lr, c->dry, c->work, frames, NULL, NULL, s->ofade * s->omix, hold ? s->ofade * s->omix : 0.0f);
                if (cur_on) contribute(ap, s->blk, lr, c->dry, c->work, frames, &s->fade, &s->mix, 0.0f, 0.0f);
                if (warming) {                           /* same input as the slot sees, output not heard */
                    memcpy(c->work, c->dry, (size_t)frames * 2 * sizeof(float));
                    ap->process(st_of(s->blk), c->work, frames);
                    s->warmed += (uint32_t)frames;
                }
            }
            ran = true;
        }
        if (s->old && !hold) s->ofade = 0.0f;
        if (s->blk) {
            pc_fx_ramp_end(&s->fade);
            pc_fx_ramp_end(&s->mix);
            if (cur_on && !run && s->fade.cur <= 0.0f) { ap->reset(st_of(s->blk)); s->warm = false; s->warmed = 0; }   /* starts clean next time */
        }
        if (ran && c->probe) c->probe(i, c->probe_u);
    }
    int n = 0;
    for (int i = 0; i < c->n; i++) {
        retire(c, i);
        n += (c->slot[i].blk != NULL) + (c->slot[i].old != NULL);
    }
    __atomic_store_n(&c->blocks, n, __ATOMIC_RELAXED);
}

bool pc_fxchain_busy(const pc_fxchain_t *c)
{
    if (LD(c->mh) != LDR(c->mt)) return true;                       /* a message waits */
    for (int i = 0; i < c->n; i++) {
        const slot_t *s = &c->slot[i];
        if (s->old) return true;                                    /* a replaced block has not left yet */
        if (!s->blk) continue;
        if (!s->on) return true;                                    /* turned off, not retired yet */
        if (s->run && !s->warm) return true;                        /* warming up */
        if (s->fade.cur != s->fade.tgt || s->mix.cur != s->mix.tgt || (s->fade.cur > 0.0f && s->fade.cur < 1.0f)) return true;   /* mid-fade */
    }
    return false;
}

void pc_fxchain_idle(pc_fxchain_t *c, float *lr, int frames, uint32_t rate, uint8_t mode)
{
    if (!pc_fxchain_busy(c)) return;
    pc_fxchain_process(c, lr, frames, rate, mode);
    memset(lr, 0, (size_t)(frames > 0 ? frames : 0) * 2 * sizeof *lr);       /* bookkeeping only: a look-ahead line or a reverb tail left from the music must not be heard */
}

void pc_fxchain_process(pc_fxchain_t *c, float *lr, int frames, uint32_t rate, uint8_t mode)
{
    ST(c->rate_seen, rate);
    while (frames > 0) {
        int n = frames > PC_FX_BLOCK ? PC_FX_BLOCK : frames;
        run_block(c, lr, n, rate, mode);
        lr += 2 * n;
        frames -= n;
    }
}
