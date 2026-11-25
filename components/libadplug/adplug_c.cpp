/* adplug_c.h over AdPlug 2.4 and libbinio (patched copies in adplug/ and binio/, see SOURCE.md and patches/).
 * Memory: C++ `new` in AdPlug's players is mostly sub-1 KB (CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=1024 would put it in internal RAM, which
 * has no headroom): while an adp_* call runs, a counted scope makes the global operator new allocate from PSRAM (internal when PSRAM
 * is full). Allocations made by other tasks during that window are PSRAM too, which only costs speed; free() takes either. */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <string>
#include <new>
#include <setjmp.h>
#include <stdint.h>
#include "adplug_c.h"
#include "binstr.h"
#include "fprovide.h"
#include "opl.h"
#include "player.h"
extern "C" {
#include "fmopl.h"
}
#include "players_table.h"

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <pthread.h>
#include <time.h>
#endif

static int g_scope;

/* Hostile files: a header field can ask `new` for gigabytes, and the firmware builds -fno-exceptions, so a failed `new` used to abort() the
 * audio task (the failure path here has no abort and no exception: a longjmp back to the API call). Now every adp_open / adp_start / adp_render runs under a Ctx: its allocations are kept on a list (16-byte header), one request
 * is capped (ADP_MAX_ONE), the live total per handle is capped (ADP_MAX_LIVE), and a failure longjmps back to the API call, which refuses the
 * file (open) or turns the handle dead (start / render: it only frees, no destructor runs on half-built players). Allocations made while
 * another task holds no Ctx are plain (header with a NULL Ctx). Up to 4 Ctx slots, matched by task. */
#ifndef ADP_MAX_ONE
#define ADP_MAX_ONE (8u << 20)
#endif
#ifndef ADP_MAX_LIVE
#define ADP_MAX_LIVE (16u << 20)
#endif
namespace {
struct Hdr;
struct COplLite;
struct Ctx {
    Hdr *head;
    size_t live;
    jmp_buf jb;
    bool armed;
    int why;                             /* 1 an allocation failed, 2 a stream was read far past its end (hostile file) */
    uint8_t *data;                       /* the file copy (malloc, not tracked) */
    COplLite *opl;                       /* its chips are malloc'd by fmopl */
};
/* magic = the header's own address xor a constant: a pointer that did not come from adp_alloc (a std::string buffer that libstdc++ allocated and an
 * inline destructor in an AdPlug object frees) has no such word before it, and goes to free() as is */
#define ADP_MAGIC ((uintptr_t)0x5AD91C0DE5A5A5A5ull)
struct alignas(16) Hdr { Hdr *prev, *next; Ctx *ctx; size_t size; uintptr_t magic; };
struct Slot { void *task; Ctx *cx; };
Slot g_slot[4];
inline void *cur_task()
{
#ifdef ESP_PLATFORM
    return (void *)xTaskGetCurrentTaskHandle();
#else
    return (void *)pthread_self();
#endif
}
Ctx *cur_ctx()
{
    void *me = cur_task();
    for (int i = 0; i < 4; i++)
        if (__atomic_load_n(&g_slot[i].task, __ATOMIC_ACQUIRE) == me) return g_slot[i].cx;
    return nullptr;
}
[[noreturn]] void ctx_fail(Ctx *cx, int why = 1)
{
    cx->why = why;
    if (cx->armed) longjmp(cx->jb, 1);
    abort();
}
}   // namespace
#ifndef ADP_NO_NEW_OVERRIDE
static void *adp_alloc(size_t n)
{
    Ctx *cx = cur_ctx();
    if (cx && (n > ADP_MAX_ONE || cx->live + n > ADP_MAX_LIVE)) ctx_fail(cx);
    void *p = nullptr;
    size_t tot = n + sizeof(Hdr);
#ifdef ESP_PLATFORM
    if (__atomic_load_n(&g_scope, __ATOMIC_RELAXED) > 0) p = heap_caps_malloc(tot, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#endif
    if (!p) p = malloc(tot);
    if (!p) { if (cx) ctx_fail(cx); abort(); }
    Hdr *h = (Hdr *)p;
    h->size = n; h->ctx = cx; h->prev = nullptr; h->next = nullptr; h->magic = (uintptr_t)h ^ ADP_MAGIC;
    if (cx) { h->next = cx->head; if (cx->head) cx->head->prev = h; cx->head = h; cx->live += n; }
    return h + 1;
}
__attribute__((no_sanitize("address"))) static bool is_ours(void *p)    /* reads the bytes before a foreign allocation: its allocator's own header */
{
    Hdr *h = (Hdr *)p - 1;
    return h->magic == ((uintptr_t)h ^ ADP_MAGIC);
}
static void adp_release(void *p)
{
    if (!p) return;
    if (!is_ours(p)) { free(p); return; }
    Hdr *h = (Hdr *)p - 1;
    h->magic = 0;
    if (Ctx *cx = h->ctx) {
        if (h->prev) h->prev->next = h->next; else cx->head = h->next;
        if (h->next) h->next->prev = h->prev;
        cx->live -= h->size;
    }
    free(h);
}
/* No global operator new / delete is replaced: adplug_scope.cmake renames the operator new / delete symbols that the libadplug objects reference
 * (objcopy --redefine-sym on the component's own archive) to these, so only AdPlug's allocations (and this file's) take the tracked path;
 * every other C++ allocation in the image (GME, ...) keeps the toolchain's operator new. The extra arguments of the nothrow / sized forms are
 * ignored by the calls. Memory made here must be freed here: libstdc++'s own code (std::string buffers) allocates and frees on its side. */
#define ADP_OP_NEW(sym) extern "C" void *adp_op_##sym(size_t n) { return adp_alloc(n); }
#define ADP_OP_DEL(sym) extern "C" void adp_op_##sym(void *p) { adp_release(p); }
ADP_OP_NEW(Znwj) ADP_OP_NEW(Znwm) ADP_OP_NEW(Znaj) ADP_OP_NEW(Znam)                       /* size_t: j on the RISC-V target, m on a 64-bit host */
ADP_OP_NEW(ZnwjRKSt9nothrow_t) ADP_OP_NEW(ZnwmRKSt9nothrow_t) ADP_OP_NEW(ZnajRKSt9nothrow_t) ADP_OP_NEW(ZnamRKSt9nothrow_t)
ADP_OP_DEL(ZdlPv) ADP_OP_DEL(ZdaPv) ADP_OP_DEL(ZdlPvj) ADP_OP_DEL(ZdlPvm) ADP_OP_DEL(ZdaPvj) ADP_OP_DEL(ZdaPvm)
ADP_OP_DEL(ZdlPvRKSt9nothrow_t) ADP_OP_DEL(ZdaPvRKSt9nothrow_t)
extern "C" void *adp_raw_alloc(size_t n) { return adp_alloc(n); }       /* the players' STL containers (adplug/adp_alloc.h) */
extern "C" void adp_raw_free(void *p) { adp_release(p); }
#ifdef __SANITIZE_ADDRESS__
/* a std::string buffer allocated inside libstdc++ (operator new) and freed by an inline destructor of ours goes to free(): what the toolchain's
 * operator delete does anyway; ASan would call it a mismatch (host builds only; only binaries that link AdPlug get this default) */
extern "C" __attribute__((no_sanitize_coverage)) const char *__asan_default_options() { return "alloc_dealloc_mismatch=0"; }
#endif
/* the players' own calloc / free (a2m-v2, mid): same list */
extern "C" void *adp_tcalloc(size_t a, size_t b)
{
    if (b && a > (size_t)0x7FFFFFFF / b) { Ctx *cx = cur_ctx(); if (cx) ctx_fail(cx); return nullptr; }
    void *p = adp_alloc(a * b);
    memset(p, 0, a * b);
    return p;
}
extern "C" void adp_tfree(void *p) { adp_release(p); }
extern "C" void adp_eof_overrun(void) { if (Ctx *cx = cur_ctx()) ctx_fail(cx, 2); }
#else
extern "C" void adp_eof_overrun(void) {}
extern "C" void *adp_raw_alloc(size_t n) { void *p = malloc(n); if (!p) abort(); return p; }
extern "C" void adp_raw_free(void *p) { free(p); }
extern "C" void *adp_tcalloc(size_t a, size_t b) { return calloc(a, b); }
extern "C" void adp_tfree(void *p) { free(p); }
#endif
namespace {
struct Scope {
    int slot;
    explicit Scope(Ctx *cx)
    {
        __atomic_add_fetch(&g_scope, 1, __ATOMIC_RELAXED);
        slot = -1;
        for (int i = 0; i < 4 && cx; i++) {
            void *exp = nullptr;
            if (__atomic_compare_exchange_n(&g_slot[i].task, &exp, cur_task(), false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) { g_slot[i].cx = cx; slot = i; break; }
        }
    }
    ~Scope()
    {
        if (slot >= 0) { g_slot[slot].cx = nullptr; __atomic_store_n(&g_slot[slot].task, nullptr, __ATOMIC_RELEASE); }
        __atomic_sub_fetch(&g_scope, 1, __ATOMIC_RELAXED);
    }
};

const int MAXV = 18;
/* operator slot (register offset 0x00..0x15, gaps skipped) -> channel 0..8 and operator 0 (modulator) / 1 (carrier) */
bool slot_of(int off, int *ch, int *op)
{
    if (off < 0 || off > 0x15 || (off & 7) > 5) return false;
    int s = (off >> 3) * 6 + (off & 7);
    *ch = (s / 6) * 3 + s % 3;
    *op = (s % 6) / 3;
    return true;
}

/* Two YM3812 chips behind AdPlug's Copl interface. Chip 1 is only run once something has been written to it. */
class COplLite : public Copl {
public:
    FM_OPL *chip[2];
    uint8_t raw[2][256];                 /* what the player wrote, before the mute filter */
    uint16_t strikes[MAXV];
    uint32_t mute;
    bool dry;                            /* length scan: record only */
    bool used1;
    bool fourop;
    bool sticky1, sticky4;               /* seen at any time since clear_sticky(): the song's own end rewinds it (init), the scan must still know */
    uint8_t lvl[MAXV];
    short tmpbuf[2048];                  /* chip 1 of one block (in the PSRAM object, not a static) */

    explicit COplLite(int rate)
    {
        chip[0] = OPLCreate(OPL_TYPE_YM3812, 3579545, rate);
        chip[1] = OPLCreate(OPL_TYPE_YM3812, 3579545, rate);
        currType = TYPE_DUAL_OPL2;
        mute = 0; dry = false;
        sticky1 = sticky4 = false;
        reset_state();
    }
    ~COplLite() { if (chip[0]) OPLDestroy(chip[0]); if (chip[1]) OPLDestroy(chip[1]); }
    bool ok() const { return chip[0] && chip[1]; }

    void reset_state()
    {
        memset(raw, 0, sizeof raw); memset(strikes, 0, sizeof strikes); memset(lvl, 0, sizeof lvl);
        used1 = false; fourop = false; currChip = 0;
    }
    void clear_sticky() { sticky1 = sticky4 = false; }
    void init() override                    /* the players call this from rewind() */
    {
        if (!dry) { OPLResetChip(chip[0]); OPLResetChip(chip[1]); }
        reset_state();
        memset(strikes, 0, sizeof strikes);
    }
    void hard_reset()
    {
        OPLResetChip(chip[0]); OPLResetChip(chip[1]);
        reset_state();
        memset(strikes, 0, sizeof strikes);
    }
    bool muted_ch(int c, int ch) const { return (mute >> (c * 9 + ch)) & 1; }

    void emit(int c, int reg, int val)
    {
        OPLWrite(chip[c], 0, reg);
        OPLWrite(chip[c], 1, val);
    }
    void write(int reg, int val) override
    {
        int c = currChip ? 1 : 0;
        reg &= 0xFF; val &= 0xFF;
        if (c == 1) {
            if (!used1 && !(reg == 5 || reg == 4 || reg == 1)) used1 = true;    /* reg 1/4/5 on chip 1 are OPL3 setup, not a second OPL2 yet */
            if (reg == 4 && val) fourop = true;
            if (used1) sticky1 = true;
            if (fourop) sticky4 = true;
        }
        int old = raw[c][reg];
        raw[c][reg] = (uint8_t)val;
        if (reg >= 0xB0 && reg <= 0xB8 && (val & 0x20) && !(old & 0x20)) strikes[c * 9 + reg - 0xB0]++;
        if (reg == 0xBD) { int d = val & ~old & 0x1F; if (d) for (int b = 0; b < 5; b++) if (d >> b & 1) strikes[c * 9 + (b == 4 ? 6 : (b == 3 || b == 0) ? 7 : 8)]++; }
        if (dry) return;
        emit(c, reg, filtered(c, reg, val));
    }
    int filtered(int c, int reg, int val) const
    {
        if (!mute) return val;
        if (reg >= 0xB0 && reg <= 0xB8) { if (muted_ch(c, reg - 0xB0)) val &= ~0x20; }
        else if (reg >= 0x80 && reg <= 0x95) { int ch, op; if (slot_of(reg - 0x80, &ch, &op) && muted_ch(c, ch)) val |= 0x0F; }
        else if (reg == 0xBD && (val & 0x20)) {
            if (muted_ch(c, 6)) val &= ~0x10;
            if (muted_ch(c, 7)) val &= ~0x09;
            if (muted_ch(c, 8)) val &= ~0x06;
        }
        return val;
    }
    void set_mute(uint32_t m)
    {
        if (m == mute) return;
        mute = m;
        if (dry) return;
        for (int c = 0; c < 2; c++) {                // re-send everything the mute filter touches, from the raw shadow
            for (int r = 0x80; r <= 0x95; r++) emit(c, r, filtered(c, r, raw[c][r]));
            for (int r = 0xB0; r <= 0xB8; r++) emit(c, r, filtered(c, r, raw[c][r]));
            emit(c, 0xBD, filtered(c, 0xBD, raw[c][0xBD]));
        }
    }
    void update(short *buf, int n) override
    {
        short *tmp = tmpbuf;
        if (n > 2048) n = 2048;
        YM3812UpdateOne(chip[0], buf, n);              /* mono into the first half, widened below */
        if (used1) {
            YM3812UpdateOne(chip[1], tmp, n);
            for (int i = n - 1; i >= 0; i--) { buf[2 * i] = buf[i]; buf[2 * i + 1] = tmp[i]; }
        } else {
            for (int i = n - 1; i >= 0; i--) { buf[2 * i] = buf[i]; buf[2 * i + 1] = buf[i]; }
        }
    }
};

/* a memory file for the players */
class MemProvider : public CFileProvider {
public:
    const uint8_t *d; uint32_t n;
    MemProvider(const uint8_t *dd, uint32_t nn) : d(dd), n(nn), eof_hits(0) {}
    binistream *open(std::string) const override
    {
        binisstream *s = new binisstream((void *)d, n);
        s->setFlag(binio::BigEndian, false);
        s->setFlag(binio::FloatIEEE);
        return s;
    }
    mutable int eof_hits;                  /* streams that ended with a read or seek past the end of the file data (the bytes read were 0) */
    void close(binistream *f) const override { if (static_cast<binisstream *>(f)->pastend) eof_hits++; delete f; }
};
}   // namespace

struct adp {
    uint8_t *data; uint32_t size;
    int rate;
    COplLite *opl;
    CPlayer *pl;
    MemProvider *fp;
    std::string name, type, title, author;
    int nsub;
    int32_t length_ms;
    double acc;
    int pending;
    bool ended, bad4op;
    int64_t ticks;
    bool dual_scan;                        /* the length scan saw the second chip written to */
    Ctx *cx;
    bool dead;                             /* an allocation failed inside start / render: only adp_close (it frees, runs no destructor) is valid */
};

/* frees everything a failed or dead handle owns without running destructors (they would see half-built players) */
static void ctx_abandon(Ctx *cx)
{
    if (cx->opl) for (int c = 0; c < 2; c++) if (cx->opl->chip[c]) OPLDestroy(cx->opl->chip[c]);
    free(cx->data);
    for (Hdr *h = cx->head; h;) { Hdr *n = h->next; free(h); h = n; }
    free(cx);
}

#ifndef ADP_SCAN_TICKS
#define ADP_SCAN_TICKS 120000      /* the silent length scan stops here (2 minutes of a 1 kHz DRO) */
#endif
static const int SCAN_TICKS = ADP_SCAN_TICKS;
#ifndef ADP_SCAN_MS
#define ADP_SCAN_MS 3000           /* wall-clock budget of one silent scan: 120000 ticks of a heavy player (A2M macros) take minutes on the board */
#endif
static int64_t now_us()
{
#ifdef ESP_PLATFORM
    return esp_timer_get_time();
#else
    timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)t.tv_sec * 1000000 + t.tv_nsec / 1000;
#endif
}

static CPlayer *make_player(adp *a, const char *ext)
{
    return adplug_players_factory(a->opl, ext ? ext : "", a->name, *a->fp);
}

static void destroy(adp *a)
{
    delete a->pl;
    delete a->opl;
    a->cx->opl = nullptr;
    delete a->fp;
    free(a->data);
    a->cx->data = nullptr;
    delete a;
}

static adp_t *open_impl(Ctx *cx, const uint8_t *data, uint32_t size, const char *ext, int rate, int *err)
{
    adp *a = new adp();
    a->cx = cx;
    a->size = size;
    a->data = (uint8_t *)malloc(size < 1100 ? 1100 : size);
    if (!a->data) { delete a; if (err) *err = ADP_ERR_MEMORY; return nullptr; }
    cx->data = a->data;
    memcpy(a->data, data, size);
    a->rate = rate;
    a->name = std::string("song") + (ext && *ext ? ext : "");
    a->fp = new MemProvider(a->data, size);
    a->opl = new COplLite(rate);
    cx->opl = a->opl;
    if (!a->opl->ok()) { destroy(a); if (err) *err = ADP_ERR_MEMORY; return nullptr; }
    a->pl = make_player(a, ext);
    if (!a->pl) { destroy(a); return nullptr; }
    a->type = a->pl->gettype();
    a->title = a->pl->gettitle();
    a->author = a->pl->getauthor();
    a->nsub = (int)a->pl->getsubsongs();
    if (a->nsub < 1) a->nsub = 1;
    /* the silent scan: how long the song is, and whether it needs OPL3 four-operator voices (written before the song's end) */
    a->opl->dry = true;
    a->opl->clear_sticky();
    a->pl->rewind(-1);
    int64_t us = 0;                         /* the song length in microseconds (float divide per tick; a double divide is a library call on the S31) */
    int ticks = 0;
    bool done = false;
    int64_t t_scan = now_us();
    while (ticks < SCAN_TICKS) {
        if ((ticks & 63) == 63 && now_us() - t_scan > (int64_t)ADP_SCAN_MS * 1000) break;      /* too slow: the length stays unknown */
        if (!a->pl->update()) { done = true; break; }
        float hz = a->pl->getrefresh();
        us += hz > 1.0f ? (int64_t)(1000000.0f / hz) : 20000;
        ticks++;
    }
    bool four = a->opl->sticky4;
    a->dual_scan = a->opl->sticky1;
    a->opl->dry = false;
    a->opl->hard_reset();
    a->length_ms = done ? (int32_t)(us / 1000) : -1;
    if (four) { destroy(a); if (err) *err = ADP_ERR_4OP; return nullptr; }
    a->pl->rewind(-1);
    if (err) *err = 0;
    return a;
}

adp_t *adp_open(const uint8_t *data, uint32_t size, const char *ext, int rate, int *err)
{
    if (err) *err = ADP_ERR_FORMAT;
    if (!data || size < 4 || size > 2u * 1024 * 1024 || rate < 8000) { if (err) *err = ADP_ERR_SIZE; return nullptr; }
    Ctx *cx = (Ctx *)calloc(1, sizeof *cx);
    if (!cx) { if (err) *err = ADP_ERR_MEMORY; return nullptr; }
    Scope sc(cx);
    if (setjmp(cx->jb)) { int why = cx->why; ctx_abandon(cx); if (err) *err = why == 2 ? ADP_ERR_DAMAGED : ADP_ERR_MEMORY; return nullptr; }     /* an allocation failed (hostile size field) */
    cx->armed = true;
    adp_t *a = open_impl(cx, data, size, ext, rate, err);
    if (!a) { ctx_abandon(cx); return nullptr; }
    cx->armed = false;
    return a;
}

void adp_close(adp_t *a)
{
    if (!a) return;
    Ctx *cx = a->cx;
    Scope sc(cx);
    if (a->dead) { ctx_abandon(cx); return; }
    destroy(a);
    free(cx);
}

int adp_subsongs(adp_t *a) { return a ? a->nsub : 0; }

static int start_impl(adp_t *a, int sub)
{
    a->opl->hard_reset();
    a->pl->rewind(sub);
    a->acc = 0; a->pending = 0; a->ended = false; a->ticks = 0;
    if (a->nsub > 1) {                      /* the length of this sub-song */
        a->opl->dry = true;
        a->opl->clear_sticky();
        int64_t us = 0; int t = 0; bool done = false;
        int64_t t_scan = now_us();
        while (t < SCAN_TICKS) { if ((t & 63) == 63 && now_us() - t_scan > (int64_t)ADP_SCAN_MS * 1000) break; if (!a->pl->update()) { done = true; break; } float hz = a->pl->getrefresh(); us += hz > 1.0f ? (int64_t)(1000000.0f / hz) : 20000; t++; }
        if (a->opl->sticky1) a->dual_scan = true;
        a->opl->dry = false;
        a->opl->hard_reset();
        a->pl->rewind(sub);
        a->length_ms = done ? (int32_t)(us / 1000) : -1;
    }
    return 0;
}

int adp_start(adp_t *a, int sub)
{
    if (!a || a->dead || sub < 0 || sub >= a->nsub) return -1;
    Scope sc(a->cx);
    if (setjmp(a->cx->jb)) { a->dead = true; return -1; }
    a->cx->armed = true;
    int r = start_impl(a, sub);
    a->cx->armed = false;
    return r;
}

static int render_impl(adp_t *a, short *out, int frames)
{
    int done = 0;
    while (done < frames) {
        if (a->pending <= 0) {
            if (a->ended) break;
            if (!a->pl->update()) { a->ended = true; break; }
            if (a->opl->fourop) { a->bad4op = true; return done ? done : -1; }
            float hz = a->pl->getrefresh();
            if (!(hz > 1.0f)) hz = 50.0f;
            if (hz > 2500.0f) hz = 2500.0f;         /* a hostile tick rate made this loop spin with no audio (pending stays 0) */
            a->acc += (double)a->rate / hz;         /* kept a double: the render timing stays bit-identical to the host's earlier runs (one soft-double divide per tick, 0.03-0.5 % of a core at 70-1000 ticks/s; a fixed-point version moved event samples) */
            a->pending = (int)a->acc;
            a->acc -= a->pending;
            a->ticks++;
            continue;
        }
        int n = a->pending < frames - done ? a->pending : frames - done;
        if (n > 2048) n = 2048;
        a->opl->update(out + 2 * done, n);
        a->pending -= n;
        done += n;
    }
    return done;
}

int adp_render(adp_t *a, short *out, int frames)
{
    if (!a || !out || frames <= 0) return 0;
    if (a->dead) return -1;
    Scope sc(a->cx);
    if (setjmp(a->cx->jb)) { a->dead = true; return -1; }
    a->cx->armed = true;
    int r = render_impl(a, out, frames);
    a->cx->armed = false;
    return r;
}

int32_t adp_length_ms(adp_t *a) { return a ? a->length_ms : -1; }
const char *adp_type(adp_t *a) { return a ? a->type.c_str() : ""; }
const char *adp_title(adp_t *a) { return a ? a->title.c_str() : ""; }
const char *adp_author(adp_t *a) { return a ? a->author.c_str() : ""; }
int adp_dual(adp_t *a) { return a && (a->opl->used1 || a->dual_scan); }
int adp_voice_count(adp_t *a) { return !a ? 0 : adp_dual(a) ? 18 : 9; }

int adp_voice(adp_t *a, int i, adp_voice_t *o)
{
    if (!a || !o || i < 0 || i >= adp_voice_count(a)) return -1;
    COplLite *p = a->opl;
    int c = i / 9, ch = i % 9;
    const uint8_t *r = p->raw[c];
    bool rhythm = (r[0xBD] & 0x20) != 0 && ch >= 6;
    memset(o, 0, sizeof *o);
    o->strikes = p->strikes[i];
    if (rhythm) {
        static const uint8_t bits[3] = { 0x10, 0x09, 0x06 };          /* bass drum; snare + hi-hat; tom + cymbal */
        o->key_on = (r[0xBD] & bits[ch - 6]) != 0 && !p->muted_ch(c, ch);
        o->percussion = 1;
        o->noise = ch != 6;
    } else {
        o->key_on = (r[0xB0 + ch] & 0x20) != 0 && !p->muted_ch(c, ch);
    }
    int fnum = r[0xA0 + ch] | (r[0xB0 + ch] & 3) << 8, block = r[0xB0 + ch] >> 2 & 7;
    o->hz_x100 = fnum ? ((long)fnum * 4971600L) >> (20 - block) : 0;
    /* the audible operator: the carrier, or the louder of the two when the voice is additive */
    static const uint8_t car[9] = { 3, 4, 5, 11, 12, 13, 19, 20, 21 }, mod[9] = { 0, 1, 2, 8, 9, 10, 16, 17, 18 };
    int tlc = r[0x40 + car[ch]] & 63, tlm = r[0x40 + mod[ch]] & 63;
    int tl = (r[0xC0 + ch] & 1) ? (tlc < tlm ? tlc : tlm) : tlc;
    o->level = o->key_on ? (uint8_t)((63 - tl) * 255 / 63) : 0;
    return 0;
}

void adp_set_mute(adp_t *a, uint32_t mask)
{
    if (!a || a->dead) return;
    a->opl->set_mute(mask);
}
