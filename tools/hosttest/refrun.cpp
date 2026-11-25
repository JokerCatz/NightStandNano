// SPDX-License-Identifier: LGPL-2.1-or-later
// An independent reference run for the FM backend's bit-exactness tests: its own small VGM / S98 command walker, the chip cores driven directly (ymfm as in the upstream examples, the MAME-derived SN76496 /
// OKIM6295 through their own device functions), no resampler, no lanes, no ring. It prints what test_fmvgm prints as sumL / sumR / chip_samples: the sum of the chips' raw output samples, so that
// any difference between the two programs is a bug in the backend (timing, routing, ROM handling) or in a patch to ymfm. Built by run_fm.sh against the unpatched ymfm (REF_YMFM_DIR) when one is given.
//   refrun FILE.vgm|vgz|s98 [-R rhythm_rom.bin] [-N]     -N: the YM2608 rhythm writes (port 0 regs 0x10-0x1D) are dropped, as the backend does without the ROM
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include <zlib.h>
#include "ymfm_opm.h"
#include "ymfm_opn.h"
#include "ymfm_misc.h"
#include "fmvgm_pcm.h"
extern "C" {
void *fmvgm_pcm_calloc(size_t n, size_t sz) { return calloc(n, sz); }
void *fmvgm_pcm_realloc(void *p, size_t n) { return realloc(p, n); }
void fmvgm_pcm_free(void *p) { free(p); }
}

namespace ymfm { void *ymfm_alloc(size_t n) { return malloc(n); } void ymfm_free(void *p) { free(p); } }   /* the allocation hook of patch 0001 (unused by an unpatched ymfm) */
static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

struct Host : ymfm::ymfm_interface {
    std::vector<uint8_t> mem[ymfm::ACCESS_CLASSES];
    uint8_t ymfm_external_read(ymfm::access_class t, uint32_t a) override { auto &m = mem[t]; return a < m.size() ? m[a] : 0; }
    void ymfm_external_write(ymfm::access_class t, uint32_t a, uint8_t d) override { auto &m = mem[t]; if (a < m.size()) m[a] = d; }
};

struct Ref {
    virtual ~Ref() {}
    virtual void write(int port, int reg, int val) = 0;
    virtual void run(uint32_t n) = 0;
    virtual void data(int cls, uint32_t total, uint32_t start, const uint8_t *p, uint32_t len) {}
    virtual void dac(uint8_t b) {}
    uint32_t rate = 0; uint64_t gen = 0, posq = 0, step = 0;
    int64_t sl = 0, sr = 0; uint64_t cnt = 0; int type = 0; bool drop_rhythm = false;
};

template<class C, int K> struct YRef : Ref {
    Host host; C chip; std::vector<typename C::output_data> buf;
    YRef(uint32_t clk, bool fid) : chip(host), buf(256)
    {
        chip.reset(); if (fid) set_fid(chip);
        rate = K == 6 ? clk / 8 : chip.sample_rate(clk);
        if (K == 3) host.mem[ymfm::ACCESS_ADPCM_B].assign(0x40000, 0);
    }
    template<class X> static void set_fid(X &c) { c.set_fidelity(ymfm::OPN_FIDELITY_MIN); }
    static void set_fid(ymfm::ym2151 &) {}
    static void set_fid(ymfm::ym2612 &) {}
    static void set_fid(ymfm::ym3438 &) {}
    static void set_fid(ymfm::ym2149 &) {}
    void write(int port, int reg, int val) override
    {
        if (K == 6) { chip.write(0, reg); chip.write(2, val); return; }
        if (K == 3 && drop_rhythm && port == 0 && reg >= 0x10 && reg <= 0x1D) return;
        chip.write(port * 2, reg); chip.write(port * 2 + 1, val);
    }
    void run(uint32_t n) override
    {
        while (n) {
            uint32_t m = n > 256 ? 256 : n;
            chip.generate(buf.data(), m);
            for (uint32_t i = 0; i < m; i++) {
                const int32_t *d = buf[i].data; int32_t l, r;
                if (K == 1) { l = d[0]; r = d[1]; }
                else if (K == 2) { l = r = d[0] + d[1] + d[2] + d[3]; }
                else if (K == 3) { l = d[0] + d[2]; r = d[1] + d[2]; }
                else { l = r = d[0] + d[1] + d[2]; }
                sl += l; sr += r;
            }
            cnt += m; n -= m;
        }
    }
    void data(int cls, uint32_t total, uint32_t start, const uint8_t *p, uint32_t len) override
    {
        auto &m = host.mem[cls];
        if (m.size() < total) m.resize(total);
        if (start < total) memcpy(&m[start], p, std::min<uint32_t>(len, total - start));
    }
};
struct Y2612Ref : YRef<ymfm::ym2612, 1> {
    using YRef::YRef; std::vector<uint8_t> bank; uint32_t pos = 0;
    void data(int cls, uint32_t, uint32_t, const uint8_t *p, uint32_t len) override { if (cls == 3) bank.insert(bank.end(), p, p + len); }
    void dac(uint8_t) override { if (pos < bank.size()) { chip.write(0, 0x2A); chip.write(1, bank[pos++]); } }
};

/* the MAME-derived cores (BSD-3-Clause) are driven through their adapters (the same fmvgm_pcm_ops the backend uses): this program supplies the timing and the ROM data on its own */
struct PcmRef : Ref {
    const fmvgm_pcm_ops *ops; void *h;
    PcmRef(const fmvgm_pcm_ops *o, uint32_t clk, uint32_t variant) : ops(o) { h = o->create(clk, variant); rate = o->rate(h); }
    ~PcmRef() { ops->destroy(h); }
    void write(int, int reg, int val) override { ops->write(h, reg, val); rate = ops->rate(h); }
    void run(uint32_t n) override
    {
        int32_t b[2 * 64];
        while (n) { uint32_t m = n > 64 ? 64 : n; ops->render(h, b, m); for (uint32_t i = 0; i < m; i++) { sl += b[2 * i]; sr += b[2 * i + 1]; } cnt += m; n -= m; }
    }
    void data(int cls, uint32_t total, uint32_t start, const uint8_t *p, uint32_t len) override
    {
        if (!ops->rom || cls != 3 || start >= total) return;
        ops->rom(h, total, start, p, std::min<uint32_t>(len, total - start));
    }
};

int main(int argc, char **argv)
{
    if (argc < 2) return 1;
    const char *rompath = NULL; bool nodrop = false;
    for (int i = 2; i < argc; i++) { if (!strcmp(argv[i], "-R") && i + 1 < argc) rompath = argv[++i]; else if (!strcmp(argv[i], "-N")) nodrop = true; }
    std::vector<uint8_t> d;
    { gzFile g = gzopen(argv[1], "rb"); if (!g) { perror("open"); return 1; } uint8_t t[65536]; int n; while ((n = gzread(g, t, sizeof t)) > 0) d.insert(d.end(), t, t + n); gzclose(g); }
    std::vector<uint8_t> rom;
    if (rompath) { FILE *f = fopen(rompath, "rb"); if (f) { uint8_t t[8192]; size_t n; while ((n = fread(t, 1, sizeof t, f)) > 0) rom.insert(rom.end(), t, t + n); fclose(f); } }
    bool s98 = d.size() > 4 && !memcmp(d.data(), "S98", 3);
    std::vector<std::unique_ptr<Ref>> chips;
    uint32_t pos, loop_abs = 0;
    uint8_t slotmap[256]; memset(slotmap, 0xFF, sizeof slotmap); int by_type[16][2]; memset(by_type, 0xFF, sizeof by_type);   // by_type: kind (1 opm 2 opn 3 opna 4 opnb 5 opn2 6 ssg 7 sn 8 oki)
    uint64_t unit_p = 44100, unit_q = 1;
    auto add = [&](int kind, uint32_t clk, uint32_t variant, int copy) -> int {
        Ref *r = nullptr;
        switch (kind) {
        case 1: r = new YRef<ymfm::ym2151, 1>(clk, false); break;
        case 2: r = new YRef<ymfm::ym2203, 2>(clk, true); break;
        case 3: { auto *y = new YRef<ymfm::ym2608, 3>(clk, true); if (!rom.empty()) y->host.mem[ymfm::ACCESS_ADPCM_A] = rom; else y->drop_rhythm = !nodrop; r = y; break; }
        case 4: if (variant & 1) r = new YRef<ymfm::ym2610b, 3>(clk, true); else r = new YRef<ymfm::ym2610, 3>(clk, true); break;
        case 5: r = new Y2612Ref(clk, false); break;
        case 6: r = new YRef<ymfm::ym2149, 6>(clk, false); break;
        case 7: r = new PcmRef(&fmvgm_pcm_sn76496, clk, variant); break;
        case 8: r = new PcmRef(&fmvgm_pcm_okim6295, clk, variant); break;
        }
        r->type = kind;
        uint32_t dec = (kind == 2 || kind == 3) ? 3 : 1; (void)dec;
        r->step = ((uint64_t)r->rate << 32) / 1;                                    // fixed below, once the time unit is known
        chips.emplace_back(r); by_type[kind][copy] = (int)chips.size() - 1; return (int)chips.size() - 1;
    };
    struct { uint32_t off; int kind; } hdr[] = { { 0x2C, 5 }, { 0x30, 1 }, { 0x44, 2 }, { 0x48, 3 }, { 0x4C, 4 }, { 0x74, 6 }, { 0x0C, 7 }, { 0x98, 8 } };
    if (!s98) {
        if (d.size() < 0x40 || memcmp(d.data(), "Vgm ", 4)) { fprintf(stderr, "not vgm\n"); return 1; }
        uint32_t ver = le32(&d[8]);
        pos = ver >= 0x150 && le32(&d[0x34]) ? 0x34 + le32(&d[0x34]) : 0x40;
        loop_abs = le32(&d[0x1C]) ? 0x1C + le32(&d[0x1C]) : 0;
        for (auto &h : hdr) {
            if (h.off + 4 > pos) continue;
            uint32_t c = le32(&d[h.off]); if (!(c & 0x3FFFFFFF)) continue;
            uint32_t clk = c & 0x3FFFFFFF, variant = (c >> 31) & 1;
            if (h.kind == 7 && ver >= 0x151 && pos > 0x2B) variant = (uint32_t)(d[0x28] | d[0x29] << 8) | (uint32_t)d[0x2A] << 16 | (uint32_t)d[0x2B] << 24;
            int a = add(h.kind, clk, variant, 0);
            static const uint8_t c0[9][2] = { {0,0}, {0x54,0}, {0x55,0}, {0x56,0x57}, {0x58,0x59}, {0x52,0x53}, {0xA0,0}, {0x50,0}, {0xB8,0} };
            slotmap[c0[h.kind][0]] = (uint8_t)a; if (c0[h.kind][1]) slotmap[c0[h.kind][1]] = (uint8_t)a;
            if (c & 0x40000000) {
                int b = add(h.kind, clk, variant, 1);
                if (h.kind <= 5) { slotmap[c0[h.kind][0] + 0x50] = (uint8_t)b; if (c0[h.kind][1]) slotmap[c0[h.kind][1] + 0x50] = (uint8_t)b; }
                else if (h.kind == 7) slotmap[0x30] = (uint8_t)b;
            }
        }
    } else {
        unit_q = le32(&d[4]) ? le32(&d[4]) : 10; unit_p = le32(&d[8]) ? le32(&d[8]) : 1000;
        pos = le32(&d[0x14]); int ver = d[3] - '0';
        std::vector<std::pair<uint32_t, uint32_t>> dev;
        if (ver < 2) dev.push_back({ 4, 7987200 });
        else { uint32_t cnt = ver >= 3 ? le32(&d[0x1C]) : 0xFFFFFFFF; for (uint32_t off = 0x20; dev.size() < cnt && off + 16 <= pos; off += 16) { uint32_t t = le32(&d[off]); if (ver < 3 && !t) break; dev.push_back({ t, le32(&d[off + 4]) }); } }
        for (size_t k = 0; k < dev.size(); k++) {
            int kind = dev[k].first == 2 ? 2 : dev[k].first == 3 ? 5 : dev[k].first == 4 ? 3 : dev[k].first == 5 ? 1 : (dev[k].first == 1 || dev[k].first == 15) ? 6 : dev[k].first == 16 ? 7 : 0;
            if (kind) slotmap[k] = (uint8_t)add(kind, dev[k].second, 0, 0);
        }
        for (auto &c : chips) if (c->type == 3) c->write(0, 0x29, 0x80);
    }
    for (auto &c : chips) c->step = ((uint64_t)c->rate * unit_q << 32) / unit_p;
    uint64_t posq_units = 0; (void)posq_units;
    auto wait = [&](uint32_t n) { for (auto &c : chips) { c->posq += (uint64_t)n * c->step; uint64_t want = c->posq >> 32; if (want > c->gen) { c->run((uint32_t)(want - c->gen)); c->gen = want; } } };
    uint32_t dacslot = by_type[5][0] >= 0 ? (uint32_t)by_type[5][0] : 0xFFFFFFFFu;
    bool done = false;
    if (!s98) {
        while (!done && pos < d.size()) {
            uint8_t c = d[pos++];
            auto W = [&](int slot, int port) { if (slot >= 0xFF || slot >= (int)chips.size()) { pos += 2; return; } chips[slot]->write(port, d[pos], d[pos + 1]); pos += 2; };
            if (c == 0x66) break;
            else if (c == 0x61) { wait(d[pos] | d[pos + 1] << 8); pos += 2; }
            else if (c == 0x62) wait(735); else if (c == 0x63) wait(882);
            else if ((c & 0xF0) == 0x70) wait((c & 15) + 1);
            else if ((c & 0xF0) == 0x80) { if (dacslot != 0xFFFFFFFFu) chips[dacslot]->dac(0); wait(c & 15); }
            else if (c == 0x67) {
                uint8_t tt = d[pos + 1]; uint32_t raw = le32(&d[pos + 2]), sz = raw & 0x7FFFFFFF; int second = raw >> 31; const uint8_t *b = &d[pos + 6]; pos += 6 + sz;
                int kind = tt == 0x00 ? 5 : tt == 0x81 ? 3 : (tt == 0x82 || tt == 0x83) ? 4 : tt == 0x8B ? 8 : 0; int cls = tt == 0x00 ? 3 : tt == 0x81 ? 2 : tt == 0x82 ? 1 : tt == 0x83 ? 2 : 3;
                if (!kind || by_type[kind][second] < 0) continue;
                Ref *r = chips[by_type[kind][second]].get();
                if (tt == 0x00) r->data(3, 0, 0, b, sz); else if (sz >= 8) r->data(cls, le32(b), le32(b + 4), b + 8, sz - 8);
            }
            else if (c == 0xE0) pos += 4;
            else if (c == 0x50 || c == 0x30) { int s = slotmap[c]; if (s != 0xFF) chips[s]->write(0, 0, d[pos]); pos += 1; }
            else if (c == 0x4F || c == 0x3F) { int s = slotmap[c == 0x4F ? 0x50 : 0x30]; if (s != 0xFF) chips[s]->write(0, 1, d[pos]); pos += 1; }
            else if (c == 0xA0 || c == 0xB8) { int kind = c == 0xA0 ? 6 : 8; int s = by_type[kind][d[pos] >> 7]; if (s >= 0) chips[s]->write(0, d[pos] & 0x7F, d[pos + 1]); pos += 2; }
            else if (slotmap[c] != 0xFF) { int port = (c == 0x53 || c == 0x57 || c == 0x59 || c == 0xA3 || c == 0xA7 || c == 0xA9 || c == 0x103) ? 1 : 0; W(slotmap[c], port); }
            else if (c >= 0x30 && c <= 0x3F) pos += 1; else if (c >= 0x40 && c <= 0x4E) pos += 2; else if (c == 0x4F || c == 0x50) pos += 1; else if (c >= 0x51 && c <= 0x5F) pos += 2;
            else if (c >= 0xA0 && c <= 0xBF) pos += 2; else if (c >= 0xC0 && c <= 0xDF) pos += 3; else if (c >= 0xE1) pos += 4;
            else if (c == 0x90 || c == 0x91 || c == 0x95) pos += 4; else if (c == 0x92) pos += 5; else if (c == 0x93) pos += 10; else if (c == 0x94) pos += 1; else if (c == 0x68) pos += 11;
            else { fprintf(stderr, "unknown command %02x\n", c); return 3; }
        }
    } else {
        while (pos < d.size()) {
            uint8_t b = d[pos];
            if (b == 0xFF) { pos++; wait(1); }
            else if (b == 0xFE) { pos++; uint32_t n = 0, sh = 0; while (pos < d.size()) { uint8_t v = d[pos++]; n |= (uint32_t)(v & 0x7F) << sh; sh += 7; if (!(v & 0x80)) break; } wait(n + 2); }
            else if (b == 0xFD) break;
            else { int dev = b >> 1, port = b & 1; if (dev < 128 && slotmap[dev] != 0xFF && (port == 0 || chips[slotmap[dev]]->type == 3 || chips[slotmap[dev]]->type == 5)) chips[slotmap[dev]]->write(port, d[pos + 1], d[pos + 2]); pos += 3; }
        }
    }
    int64_t sl = 0, sr = 0; uint64_t n = 0;
    for (auto &c : chips) { sl += c->sl; sr += c->sr; n += c->cnt; }
    printf("sumL=%lld sumR=%lld chip_samples=%llu\n", (long long)sl, (long long)sr, (unsigned long long)n);
    return 0;
}
