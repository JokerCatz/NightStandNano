// SPDX-License-Identifier: LGPL-2.1-or-later
/* The chip layer: ymfm chips (YM2151, YM2203, YM2608, YM2610 / B, YM2612, the AY8910 / YM2149 SSG) and the adapters for the sample / PSG cores (SN76489, OKIM6295) behind one Chip interface,
 * plus the memory behind ADPCM data blocks (referenced in place in the file image where the format allows). All integer: the same register writes give the same raw frames everywhere. */
#include "fmvgm_int.h"
#include "fmvgm_pcm.h"
#include <string.h>
#include <new>
#include <type_traits>
#include "ymfm_opm.h"
#include "ymfm_opn.h"
#include "ymfm_misc.h"

/* ---- the YM2608 rhythm ROM (optional, supplied by the host) ------------------------------------------------------------------------------------------------------------ */
static const uint8_t *g_rom2608; static uint32_t g_rom2608_n;
void fmvgm_set_rom(int which, const void *data, uint32_t n)
{
    if (which == FMVGM_ROM_YM2608_RHYTHM) { g_rom2608 = n ? (const uint8_t *)data : NULL; g_rom2608_n = n; }
}

bool fm_rom2608_ready(void) { return g_rom2608 && g_rom2608_n; }

/* ---- chip table: names, clock ranges, rates ---------------------------------------------------------------------------------------------------------------------------- */
const char *fm_chip_name(int t)
{
    static const char *n[CT_N] = { "?", "YM2151", "YM2203", "YM2608", "YM2610", "YM2612", "AY8910", "SN76489", "OKIM6295" };
    return t > 0 && t < CT_N ? n[t] : "?";
}
bool fm_clock_ok(int t, uint32_t c)
{
    switch (t) {
    case CT_SSG: case CT_OKI: return c >= 500000u && c <= 8500000u;
    case CT_SN: return c >= 1000000u && c <= 5000000u;
    default: return c >= 1000000u && c <= 8500000u;
    }
}
void fm_chip_rate(int t, uint32_t clk, uint32_t variant, uint32_t *raw, uint8_t *dec, uint32_t *max_raw)
{
    uint32_t r = 0, mx = 0; uint8_t d = 1;
    switch (t) {
    case CT_OPM:  r = clk / 64; break;
    case CT_OPN:  r = clk / 24; d = 3; break;                  /* ymfm's lowest fidelity: FM at clock / 72 repeated three times, the SSG resampled; the backend averages the three */
    case CT_OPNA: r = clk / 48; d = 3; break;
    case CT_OPNB: r = clk / 144; break;
    case CT_OPN2: r = clk / 144; break;
    case CT_SSG:                                               /* the SSG engine of ymfm runs at clock / 8 (ym2149::sample_rate says clock / 64 but generate() clocks the engine once per sample: pitch 8x low, measured); averaged 4:1 here */
        r = clk / 8; d = (uint8_t)((r + 28000) / 56000); if (!d) d = 1; break;
    case CT_SN: {
        uint32_t div = (variant >> 24) & 0x08 ? 1 : 8;          /* SN76489 clock divider flag of the VGM header */
        r = clk / 2 / div; d = (uint8_t)((r + 28000) / 56000); if (!d) d = 1;
        break; }
    case CT_OKI: r = clk / ((variant & 1) ? 132 : 165); mx = clk / 132; break;
    }
    *raw = r; *dec = d; *max_raw = mx ? mx : r;
}

/* ---- memory behind data blocks ----------------------------------------------------------------------------------------------------------------------------------------- */
namespace {
/* Pieces that point into the file image (nothing is copied), newest first on overlap; or one flat buffer (YM2608 DELTA-T RAM: the chip writes it too). Reads outside return 0, as ymfm's own
 * example player does for a ROM that was never filled. */
struct MemMap {
    enum { MAXSEG = 16 };
    struct Seg { uint32_t start, len; const uint8_t *p; };
    Seg seg[MAXSEG]; int n, last; bool overlap;
    uint8_t *flat; uint32_t flat_size;
    const MemMap *alias;                                       /* used while this one has no data (YM2610: DELTA-T reads the ADPCM-A ROM when the file has no separate block) */
    void init() { n = 0; last = 0; overlap = false; flat = NULL; flat_size = 0; alias = NULL; }
    uint8_t read(uint32_t a) const
    {
        if (flat) return a < flat_size ? flat[a] : 0;
        if (!n) return alias ? alias->read(a) : 0;
        if (!overlap && last < n && a - seg[last].start < seg[last].len) return seg[last].p[a - seg[last].start];
        for (int i = n - 1; i >= 0; i--)
            if (a - seg[i].start < seg[i].len) { const_cast<MemMap *>(this)->last = i; return seg[i].p[a - seg[i].start]; }
        return 0;
    }
    void write(uint32_t a, uint8_t d) { if (flat && a < flat_size) flat[a] = d; }
    /* 0 ok, -1 no room for another piece. start / len already clamped by the caller to what the file declares */
    int add(uint32_t start, uint32_t len, const uint8_t *p)
    {
        if (!len) return 0;
        if (flat) { if (start < flat_size) memcpy(flat + start, p, len < flat_size - start ? len : flat_size - start); return 0; }
        for (int i = 0; i < n; i++) if (seg[i].start == start && seg[i].len == len) { seg[i].p = p; last = i; return 0; }     /* the same block sent again */
        if (n >= MAXSEG) return -1;
        for (int i = 0; i < n; i++) if (start < seg[i].start + seg[i].len && seg[i].start < start + len) overlap = true;
        seg[n] = { start, len, p }; last = n++;
        return 0;
    }
};
}

/* ---- ymfm chips ---------------------------------------------------------------------------------------------------------------------------------------------------- */
namespace {
const int BATCH = 128;
struct Mems { MemMap a, b; };
struct NoMems {};

template<bool M> struct HostT : ymfm::ymfm_interface {
    typename std::conditional<M, Mems, NoMems>::type mm;
    MemMap *get(ymfm::access_class t) { return M ? (t == ymfm::ACCESS_ADPCM_A ? &((Mems &)mm).a : t == ymfm::ACCESS_ADPCM_B ? &((Mems &)mm).b : nullptr) : nullptr; }
    uint8_t ymfm_external_read(ymfm::access_class t, uint32_t a) override { MemMap *m = get(t); return m ? m->read(a) : 0; }
    void ymfm_external_write(ymfm::access_class t, uint32_t a, uint8_t d) override { MemMap *m = get(t); if (m) m->write(a, d); }
};

static void fid(ymfm::ym2151 &) {}
static void fid(ymfm::ym2612 &) {}
static void fid(ymfm::ym2149 &) {}
static void fid(ymfm::ym2203 &c) { c.set_fidelity(ymfm::OPN_FIDELITY_MIN); }
static void fid(ymfm::ym2608 &c) { c.set_fidelity(ymfm::OPN_FIDELITY_MIN); }
static void fid(ymfm::ym2610 &c) { c.set_fidelity(ymfm::OPN_FIDELITY_MIN); }

template<int T> struct Mix;
template<> struct Mix<CT_OPM>  { template<class O> static void f(const O &o, int32_t *lr) { lr[0] = o.data[0]; lr[1] = o.data[1]; } };
template<> struct Mix<CT_OPN2> { template<class O> static void f(const O &o, int32_t *lr) { lr[0] = o.data[0]; lr[1] = o.data[1]; } };
template<> struct Mix<CT_OPN>  { template<class O> static void f(const O &o, int32_t *lr) { int32_t s = o.data[0] + o.data[1] + o.data[2] + o.data[3]; lr[0] = s; lr[1] = s; } };
template<> struct Mix<CT_OPNA> { template<class O> static void f(const O &o, int32_t *lr) { lr[0] = o.data[0] + o.data[2]; lr[1] = o.data[1] + o.data[2]; } };
template<> struct Mix<CT_OPNB> { template<class O> static void f(const O &o, int32_t *lr) { lr[0] = o.data[0] + o.data[2]; lr[1] = o.data[1] + o.data[2]; } };
template<> struct Mix<CT_SSG>  { template<class O> static void f(const O &o, int32_t *lr) { int32_t s = o.data[0] + o.data[1] + o.data[2]; lr[0] = s; lr[1] = s; } };

template<class C, int T> struct YChip : Chip {
    static constexpr bool HASMEM = T == CT_OPNA || T == CT_OPNB;
    HostT<HASMEM> host; C chip; typename C::output_data buf[BATCH];
    bool good; bool rhythm; uint32_t pcm_pos, pcm_len; MemMap pcm_unused;
    Mems *mems() { return (Mems *)&host.mm; }
    YChip(uint32_t clock, bool dummy = false) : chip(host)
    {
        type = T; gain = 256; flags = 0; rate_dirty = false; good = true; rhythm = true; pcm_pos = pcm_len = 0;
        chip.reset(); fid(chip);
        uint32_t r, mx; uint8_t d; fm_chip_rate(T, clock, 0, &r, &d, &mx);
        raw_rate = T == CT_SSG ? r : chip.sample_rate(clock);
        dec = d;
        if (HASMEM) {
            Mems *m = mems(); m->a.init(); m->b.init();
            if (T == CT_OPNA) {
                flags |= CF_NEEDS_ROM | CF_ADPCM;
                m->b.flat = (uint8_t *)fm_alloc(0x40000, FMVGM_MEM_BULK);                    /* DELTA-T RAM: 256 KB, written by the chip's own memory-write register and by data blocks */
                if (m->b.flat) { m->b.flat_size = 0x40000; memset(m->b.flat, 0, 0x40000); } else good = false;
                if (g_rom2608 && g_rom2608_n) m->a.add(0, g_rom2608_n, g_rom2608);           /* the rhythm ROM image, in place */
                else { rhythm = false; flags |= CF_ROM_MISSING; }
            } else { flags |= CF_ADPCM; m->b.alias = &m->a; }
        }
    }
    ~YChip() override { if (T == CT_OPNA && HASMEM) fm_free(mems()->b.flat); }
    bool ok() override { return good; }
    void start_s98() override { if (T == CT_OPNA) write(0, 0x29, 0x80); }         /* S98 dumps of a YM2608 expect OPNA mode (6 channels) to be on: libvgm's player writes 0x29 = 0x80 first */
    void write(uint8_t port, uint8_t reg, uint8_t val) override
    {
        if (T == CT_OPNA && !rhythm && port == 0 && reg >= 0x10 && reg <= 0x1D) return;     /* the rhythm channels read a ROM we do not have: they stay silent */
        if (T == CT_SSG) { chip.write(0, reg); chip.write(2, val); return; }              /* ymfm's ym2149: offset 0 = address, 2 = data (BC2 / BC1) */
        if (T == CT_OPM || T == CT_OPN) port = 0;
        chip.write(port * 2u, reg); chip.write(port * 2u + 1u, val);
    }
    void gen(int32_t *lr, uint32_t n) override
    {
        while (n) {
            uint32_t m = n > (uint32_t)BATCH ? BATCH : n;
            chip.generate(buf, m);
            for (uint32_t i = 0; i < m; i++) Mix<T>::f(buf[i], lr + 2 * i);
            lr += 2 * m; n -= m;
        }
    }
    int block(uint8_t cls, uint32_t total, uint32_t start, const uint8_t *p, uint32_t len) override
    {
        if (HASMEM) {
            Mems *m = mems();
            MemMap *mm = cls == MC_ADPCM_A ? &m->a : cls == MC_ADPCM_B ? &m->b : NULL;
            if (!mm || (T == CT_OPNA && cls == MC_ADPCM_A)) return 0;                           /* the rhythm ROM is not in the file */
            if (start >= total) return 0;
            uint64_t end = (uint64_t)start + len; if (end > total) end = total;
            return mm->add(start, (uint32_t)(end - start), p) ? -1 : 0;
        }
        return 0;
    }
};
/* YM2612: the PCM bank behind the 0x8n commands is its own memory (data blocks of type 0, appended), not a ymfm access class */
template<class C> struct Y2612 : YChip<C, CT_OPN2> {
    MemMap bank; uint32_t pos, blen;
    Y2612(uint32_t clock) : YChip<C, CT_OPN2>(clock) { bank.init(); pos = blen = 0; }
    void dac() override { if (pos < blen) { this->chip.write(0, 0x2A); this->chip.write(1, bank.read(pos++)); } }
    void dac_seek(uint32_t p) override { pos = p; }
    int block(uint8_t cls, uint32_t, uint32_t, const uint8_t *p, uint32_t len) override
    {
        if (cls != MC_PCM || !len) return 0;
        if (bank.add(blen, len, p)) return -1;
        blen += len; return 0;
    }
};

typedef YChip<ymfm::ym2151, CT_OPM> Opm;
typedef YChip<ymfm::ym2203, CT_OPN> Opn;
typedef YChip<ymfm::ym2608, CT_OPNA> Opna;
typedef YChip<ymfm::ym2610, CT_OPNB> Opnb;
typedef YChip<ymfm::ym2610b, CT_OPNB> Opnb2;
typedef Y2612<ymfm::ym2612> Opn2;
typedef Y2612<ymfm::ym3438> Opn2b;
typedef YChip<ymfm::ym2149, CT_SSG> Ssg;
}

/* ---- sample / PSG chips behind fmvgm_pcm_ops ------------------------------------------------------------------------------------------------------------------------- */
namespace {
struct PcmChip : Chip {
    const fmvgm_pcm_ops *ops; void *h;
    PcmChip(const fmvgm_pcm_ops *o, int t, uint32_t clock, uint32_t variant, uint16_t g) : ops(o)
    {
        type = t; gain = g; flags = 0; rate_dirty = false;
        uint32_t r, mx; uint8_t d; fm_chip_rate(t, clock, variant, &r, &d, &mx); dec = d;
        h = o->create(clock, variant);
        raw_rate = h ? o->rate(h) : r;
        if (o->rom) flags |= CF_ADPCM;
    }
    ~PcmChip() override { if (h) ops->destroy(h); }
    bool ok() override { return h != NULL; }
    void write(uint8_t, uint8_t reg, uint8_t val) override
    {
        ops->write(h, reg, val);
        uint32_t r = ops->rate(h);
        if (r != raw_rate) { raw_rate = r; rate_dirty = true; }
    }
    void gen(int32_t *lr, uint32_t n) override { ops->render(h, lr, n); }
    int block(uint8_t cls, uint32_t total, uint32_t start, const uint8_t *p, uint32_t len) override
    {
        if (!ops->rom || cls != MC_PCM) return 0;
        if (start >= total) return 0;
        uint64_t end = (uint64_t)start + len; if (end > total) end = total;
        return ops->rom(h, total, start, p, (uint32_t)(end - start));
    }
};
}

/* the chip gain (1/256): ymfm's outputs are on a 32768 full scale, the MAME-derived cores are too; MAME's own driver mixes put the OKI at about 0.86 of a YM2151 (CPS-1: 0.30 vs 0.35) and an SN76496
 * at about half of a YM2612 (Mega Drive). Heuristic, set by the arithmetic of those drivers, not tuned by ear: see docs/design.md. */
static uint16_t chip_gain(int t) { return t == CT_OKI ? 220 : t == CT_SN ? 128 : 256; }

size_t fm_chip_size(int t)
{
    switch (t) {
    case CT_OPM: return sizeof(Opm); case CT_OPN: return sizeof(Opn); case CT_OPNA: return sizeof(Opna);
    case CT_OPNB: return sizeof(Opnb2) > sizeof(Opnb) ? sizeof(Opnb2) : sizeof(Opnb);
    case CT_OPN2: return sizeof(Opn2b) > sizeof(Opn2) ? sizeof(Opn2b) : sizeof(Opn2);
    case CT_SSG: return sizeof(Ssg); case CT_SN: case CT_OKI: return sizeof(PcmChip);
    }
    return 0;
}
size_t fm_chip_arena(int t)
{
    switch (t) {
    case CT_OPM: return 4096; case CT_OPN: return 2048; case CT_OPNA: return 4096; case CT_OPNB: return 4096; case CT_OPN2: return 3072;
    }
    return 0;
}
Chip *fm_chip_make(void *m, int t, uint32_t clk, uint32_t variant)
{
    switch (t) {
    case CT_OPM: return new (m) Opm(clk);
    case CT_OPN: return new (m) Opn(clk);
    case CT_OPNA: return new (m) Opna(clk);
    case CT_OPNB: return (variant & 1) ? (Chip *)new (m) Opnb2(clk) : (Chip *)new (m) Opnb(clk);    /* variant bit 0: YM2610B (six FM channels) */
    case CT_OPN2: return (variant & 1) ? (Chip *)new (m) Opn2b(clk) : (Chip *)new (m) Opn2(clk);    /* variant bit 0: YM3438 */
    case CT_SSG: return new (m) Ssg(clk);
    case CT_SN: return new (m) PcmChip(&fmvgm_pcm_sn76496, t, clk, variant, chip_gain(t));
    case CT_OKI: return new (m) PcmChip(&fmvgm_pcm_okim6295, t, clk, variant, chip_gain(t));
    }
    return NULL;
}
