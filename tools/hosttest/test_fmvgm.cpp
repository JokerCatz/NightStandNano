// SPDX-License-Identifier: LGPL-2.1-or-later
// Host runner and tests for components/fmvgm (plain g++, no ESP-IDF). See run_fm.sh.
//   test_fmvgm run FILE [-r OUTRATE] [-t TAPS] [-p inline|core0|core1] [-s SECONDS] [-w out.i16] [-n LOOPS]    render a file, print tags / length / checksums
//   test_fmvgm bench FILE [seconds]                                                                              producer only, as fast as possible (chips + resampler), prints ms per audio second
//   test_fmvgm selftest                                                                                          S98 / VGM edge cases and malformed input (run under ASan / UBSan)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <vector>
#include <algorithm>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <zlib.h>
#include "fmvgm.h"
#include "pc/pc_io.h"
#include "pc/pc_vgmhdr.h"
#include "vgmgate.h"                                                                      /* app/main/vgmgate.c, compiled in by run_fm.sh */

/* ---- a pthread port: the same worker contract as the firmware's ---- */
struct HostTask { pthread_t th; pthread_mutex_t mu; pthread_cond_t cv; int wake; void (*fn)(void *); void *arg; };
static pthread_key_t g_key;
static void *thr_main(void *p) { HostTask *t = (HostTask *)p; pthread_setspecific(g_key, t); t->fn(t->arg); return NULL; }
static void *h_alloc(size_t n, fmvgm_mem_t) { return aligned_alloc(8, (n + 7) & ~(size_t)7); }
static void h_free(void *p) { free(p); }
static void *h_task_start(void (*fn)(void *), void *arg, int, int, uint32_t)
{
    HostTask *t = new HostTask(); pthread_mutex_init(&t->mu, NULL); pthread_cond_init(&t->cv, NULL); t->wake = 0; t->fn = fn; t->arg = arg;
    pthread_create(&t->th, NULL, thr_main, t); return t;
}
static void h_task_wake(void *tp) { HostTask *t = (HostTask *)tp; pthread_mutex_lock(&t->mu); t->wake = 1; pthread_cond_signal(&t->cv); pthread_mutex_unlock(&t->mu); }
static void h_task_wait(uint32_t ms)
{
    HostTask *t = (HostTask *)pthread_getspecific(g_key);
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts); ts.tv_nsec += (long)ms * 1000000L; ts.tv_sec += ts.tv_nsec / 1000000000L; ts.tv_nsec %= 1000000000L;
    pthread_mutex_lock(&t->mu);
    while (!t->wake) if (pthread_cond_timedwait(&t->cv, &t->mu, &ts)) break;
    t->wake = 0; pthread_mutex_unlock(&t->mu);
}
static void h_sleep(uint32_t ms) { usleep(ms * 1000); }
static uint32_t h_now(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return (uint32_t)(ts.tv_sec * 1000000u + ts.tv_nsec / 1000); }
static const fmvgm_port_t k_port = { h_alloc, h_free, h_task_start, h_task_wake, h_task_wait, NULL, h_sleep, h_now };

static std::vector<uint8_t> slurp(const char *path)
{
    std::vector<uint8_t> d; gzFile g = gzopen(path, "rb"); if (!g) return d;
    uint8_t t[65536]; int n; while ((n = gzread(g, t, sizeof t)) > 0) d.insert(d.end(), t, t + n); gzclose(g); return d;
}

static int cmd_run(int argc, char **argv, bool bench)
{
    const char *path = argv[2]; const char *wav = NULL; double secs = 0; int place = 0;
    for (int i = 3; i < argc; i++) {
        if (!strcmp(argv[i], "-r") && i + 1 < argc) fmvgm_set_out_rate((uint32_t)atoi(argv[++i]));
        else if (!strcmp(argv[i], "-t") && i + 1 < argc) fmvgm_set_taps(atoi(argv[++i]));
        else if (!strcmp(argv[i], "-n") && i + 1 < argc) fmvgm_set_loops(atoi(argv[++i]));
        else if (!strcmp(argv[i], "-s") && i + 1 < argc) secs = atof(argv[++i]);
        else if (!strcmp(argv[i], "-w") && i + 1 < argc) wav = argv[++i];
        else if (!strcmp(argv[i], "-p") && i + 1 < argc) { ++i; place = !strcmp(argv[i], "core0") ? 1 : !strcmp(argv[i], "core1") ? 2 : 0; }
        else if (!strcmp(argv[i], "-R") && i + 1 < argc) { static std::vector<uint8_t> rom; rom = slurp(argv[++i]); fmvgm_set_rom(FMVGM_ROM_YM2608_RHYTHM, rom.data(), (uint32_t)rom.size()); }   /* the YM2608 rhythm ROM image */
        else if (!strcmp(argv[i], "-P") && i + 1 < argc) { uint8_t pl[4]; int np = 0; for (char *t = argv[++i]; *t && np < 4; t++) if (*t >= '0' && *t <= '2') pl[np++] = (uint8_t)(*t - '0'); fmvgm_set_plan(pl, np); }   /* -P 12: chip 0 on core 0, chip 1 on core 1 (0 = inline) */
        else if (bench && i == 3) secs = atof(argv[i]);
    }
    std::vector<uint8_t> d = slurp(path);
    if (d.empty()) { fprintf(stderr, "cannot read %s\n", path); return 2; }
    fmvgm_set_port(&k_port); fmvgm_set_placement(place); fmvgm_set_checksum(1);
    pc_reader_t r; pc_reader_mem(&r, d.data(), (uint32_t)d.size()); pc_reader_set_ext(&r, path);
    void *h = NULL;
    if (pc_backend_fmvgm.open(&h, &r, 0)) { printf("open failed: %s\n", fmvgm_error()); return 1; }
    fmvgm_info_t in; fmvgm_info(h, &in); fmvgm_tags_t tg; fmvgm_tags(h, &tg);
    printf("file=%s format=%s chips=%d", path, in.format ? "S98" : "VGM", in.n_chips);
    for (int c = 0; c < in.n_chips; c++) printf(" %s@%u(native %u Hz)", in.chip[c].name, in.chip[c].clock, in.chip[c].native_rate);
    printf("\nlanes=%d places:", in.lanes); for (int c = 0; c < in.n_chips; c++) printf(" %d", in.chip[c].place);
    printf("\nout_rate=%u taps=%d length_ms=%d loop=%d loop_ms=%d place=%d chip arena %u/%u bytes, spill %d\n", in.out_rate, in.taps, in.length_ms, in.loops, in.loop_ms, in.placement, in.chip_bytes, in.chip_bytes_cap, in.chip_spill);
    printf("tags: title=\"%s\" author=\"%s\" game=\"%s\" system=\"%s\"\n", tg.title, tg.author, tg.game, tg.system);
    FILE *w = wav ? fopen(wav, "wb") : NULL;
    uint64_t frames = 0, limit = secs > 0 ? (uint64_t)(secs * in.out_rate) : ~0ull;
    std::vector<float> buf(1024 * 2);
    uint32_t t0 = h_now(), busy = 0; int rc = 0;
    if (bench) { uint64_t n = fmvgm_bench(h, (uint32_t)secs, &busy); frames = n; }
    else while (frames < limit) {
        int n = pc_backend_fmvgm.render(h, buf.data(), 1024);
        if (n <= 0) { rc = n; break; }
        if (w) { for (int i = 0; i < n * 2; i++) { float v = buf[i] * 32768.0f; int16_t s = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v); fwrite(&s, 2, 1, w); } }
        frames += n;
    }
    uint32_t t1 = h_now();
    fmvgm_sums_t s; fmvgm_sums(h, &s); fmvgm_stats_t st; fmvgm_stats(h, &st);
    printf("frames=%llu (%.3f s) end_rc=%d wall_ms=%.1f busy_ms=%.1f\n", (unsigned long long)frames, (double)frames / in.out_rate, rc, (t1 - t0) / 1000.0, busy / 1000.0);
    printf("sumL=%lld sumR=%lld chip_samples=%llu out_hash=%08x out_frames=%llu table_hash=%08x\n", (long long)s.chip_sum_l, (long long)s.chip_sum_r, (unsigned long long)s.chip_samples, s.out_hash,
           (unsigned long long)s.out_frames, s.table_hash);
    printf("stats: made=%u read=%u starved=%u starved_ms=%u full_waits=%u low_water=%u/%u busy_us=%u migrations=%u\n", st.frames_made, st.frames_read, st.starved, st.starved_ms, st.full_waits, st.low_water, st.ring_frames, st.busy_us, st.migrations);
    if (w) fclose(w);
    pc_backend_fmvgm.close(h);
    return 0;
}


/* ---------------------------------------------------------------------------------------- selftest ---- */
static int g_fail, g_pass;
#define CHECK(c, ...) do { if (c) g_pass++; else { g_fail++; printf("FAIL %s:%d: %s  ", __FILE__, __LINE__, #c); printf(__VA_ARGS__); printf("\n"); } } while (0)
static void le32(std::vector<uint8_t> &v, size_t off, uint32_t x) { if (v.size() < off + 4) v.resize(off + 4); for (int i = 0; i < 4; i++) v[off + i] = (uint8_t)(x >> (8 * i)); }

static std::vector<uint8_t> mk_vgm(uint32_t version, const std::vector<std::pair<uint32_t, uint32_t>> &clocks, const std::vector<uint8_t> &body, int32_t loop_in_body = -1, uint32_t total = 0, uint32_t loop_samples = 0)
{
    std::vector<uint8_t> v(0x100, 0); memcpy(v.data(), "Vgm ", 4);
    le32(v, 8, version); le32(v, 0x18, total); if (loop_in_body >= 0) { le32(v, 0x1C, 0x100 + loop_in_body - 0x1C); le32(v, 0x20, loop_samples); }
    le32(v, 0x34, 0x100 - 0x34);
    for (auto &c : clocks) le32(v, c.first, c.second);
    v.insert(v.end(), body.begin(), body.end());
    le32(v, 4, (uint32_t)v.size() - 4);
    return v;
}
static const uint32_t OPM_CLK = 3579545, OPN_CLK = 3993600;
static void wr(std::vector<uint8_t> &b, uint8_t cmd, uint8_t reg, uint8_t val) { b.push_back(cmd); b.push_back(reg); b.push_back(val); }
static void wait(std::vector<uint8_t> &b, uint32_t n) { b.push_back(0x61); b.push_back(n & 255); b.push_back(n >> 8); }

struct Res { int open_rc; int32_t len; int loops; uint64_t frames; char err[64]; int chips; uint32_t out_hash; };
static Res play_buf(const std::vector<uint8_t> &d, uint64_t cap_frames = 400000, bool probe_only = false)
{
    Res r; memset(&r, 0, sizeof r);
    fmvgm_info_t in; fmvgm_tags_t tg;
    if (fmvgm_probe(d.data(), (uint32_t)d.size(), &in, &tg) != 0) { r.open_rc = -1; snprintf(r.err, sizeof r.err, "%s", fmvgm_error()); return r; }
    if (probe_only) { r.len = in.length_ms; r.loops = in.loops; return r; }
    pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size());
    void *h = NULL;
    r.open_rc = pc_backend_fmvgm.open(&h, &rd, 0);
    if (r.open_rc) { snprintf(r.err, sizeof r.err, "%s", fmvgm_error()); return r; }
    fmvgm_info(h, &in); r.len = in.length_ms; r.loops = in.loops; r.chips = in.n_chips;
    CHECK(in.chip_spill == 0, "chip arena overflowed: %u of %u bytes, %d spills", in.chip_bytes, in.chip_bytes_cap, in.chip_spill);
    std::vector<float> buf(1024 * 2);
    while (r.frames < cap_frames) { int n = pc_backend_fmvgm.render(h, buf.data(), 1024); if (n <= 0) break; r.frames += n; }
    fmvgm_sums_t s; if (!fmvgm_sums(h, &s)) r.out_hash = s.out_hash;
    pc_backend_fmvgm.close(h);
    return r;
}

static std::vector<uint8_t> mk_s98(int ver, uint32_t num, uint32_t den, const std::vector<std::pair<uint32_t, uint32_t>> &devs, const std::vector<uint8_t> &body, const char *tag = NULL, int loop_in_body = -1)
{
    std::vector<uint8_t> v(0x20, 0); memcpy(v.data(), "S98", 3); v[3] = (uint8_t)('0' + ver);
    le32(v, 4, num); le32(v, 8, den);
    size_t tbl = 0;
    if (ver >= 2) { for (auto &d : devs) { size_t o = v.size(); v.resize(o + 16); le32(v, o, d.first); le32(v, o + 4, d.second); } if (ver == 2) v.resize(v.size() + 16, 0); if (ver == 3) le32(v, 0x1C, (uint32_t)devs.size()); tbl = v.size(); }
    (void)tbl;
    size_t tag_off = 0;
    if (tag) { tag_off = v.size(); v.insert(v.end(), tag, tag + strlen(tag) + 1); }
    size_t data_off = v.size();
    v.insert(v.end(), body.begin(), body.end());
    le32(v, 0x10, (uint32_t)tag_off); le32(v, 0x14, (uint32_t)data_off); le32(v, 0x18, loop_in_body >= 0 ? (uint32_t)data_off + loop_in_body : 0);
    return v;
}
static std::vector<uint8_t> s98_notes(int ticks, int dev = 0)
{
    std::vector<uint8_t> b;
    const uint8_t init[][2] = { {0xB0, 0x07}, {0x30, 0x01}, {0x40, 0x10}, {0x50, 0x1F}, {0x60, 0x08}, {0x70, 0x00}, {0x80, 0x1F}, {0xA4, 0x22}, {0xA0, 0x69}, {0x28, 0xF0} };
    for (auto &w : init) { b.push_back((uint8_t)(dev * 2)); b.push_back(w[0]); b.push_back(w[1]); }
    for (int i = 0; i < ticks; i++) b.push_back(0xFF);
    b.push_back(0xFD);
    return b;
}

static void selftest_s98()
{
    auto good = mk_s98(3, 1, 100, { {2, OPN_CLK} }, s98_notes(100), "[S98]title=Hello\ngame=G\nartist=A\n");
    Res r = play_buf(good);
    CHECK(r.open_rc == 0 && r.chips == 1, "good S98 opens (%s)", r.err);
    CHECK(r.len == 1000 && r.frames > 55000 && r.frames < 55600, "length %d frames %llu", r.len, (unsigned long long)r.frames);
    { fmvgm_info_t in; fmvgm_tags_t tg; fmvgm_probe(good.data(), (uint32_t)good.size(), &in, &tg); CHECK(!strcmp(tg.title, "Hello") && !strcmp(tg.game, "G") && !strcmp(tg.author, "A"), "tags %s/%s/%s", tg.title, tg.game, tg.author); }
    /* other devices are refused by name */
    r = play_buf(mk_s98(3, 1, 100, { {6, 3579545} }, s98_notes(10))); CHECK(r.open_rc != 0 && strstr(r.err, "YM2413"), "YM2413 refused: %s", r.err);
    r = play_buf(mk_s98(3, 1, 100, { {4, 7987200} }, s98_notes(10))); CHECK(r.open_rc == 0 && r.chips == 1, "a YM2608 plays: %s", r.err);
    r = play_buf(mk_s98(1, 0, 0, {}, s98_notes(10))); CHECK(r.open_rc == 0 && r.chips == 1, "v1 default device is a YM2608 and plays: %s", r.err);
    r = play_buf(mk_s98(3, 1, 100, { {2, OPN_CLK}, {6, 3579545} }, s98_notes(10))); CHECK(r.open_rc == 0, "a YM2413 that is never written does not matter: %s", r.err);
    { auto b = s98_notes(10); b.insert(b.begin(), { 0x02, 0x00, 0x00 });                          /* a write to device 1 (the YM2413) */
      r = play_buf(mk_s98(3, 1, 100, { {2, OPN_CLK}, {6, 3579545} }, b)); CHECK(r.open_rc != 0 && strstr(r.err, "YM2413"), "a written YM2413 is refused by name: %s", r.err); }
    r = play_buf(mk_s98(2, 1, 100, { {2, OPN_CLK} }, s98_notes(10))); CHECK(r.open_rc == 0, "v2 device list: %s", r.err);
    r = play_buf(mk_s98(3, 1, 100, { {5, OPM_CLK}, {2, OPN_CLK} }, s98_notes(10, 1))); CHECK(r.open_rc == 0 && r.chips == 2, "OPM + OPN: %s", r.err);
    r = play_buf(mk_s98(3, 1, 100, { {2, 12000000} }, s98_notes(10))); CHECK(r.open_rc != 0, "a 12 MHz YM2203 clock is refused");
    r = play_buf(mk_s98(3, 2000, 100, { {2, OPN_CLK} }, s98_notes(10))); CHECK(r.open_rc != 0, "timer numerator 2000 refused");
    r = play_buf(mk_s98(3, 1, 5, { {2, OPN_CLK} }, s98_notes(10))); CHECK(r.open_rc != 0, "a 200 ms tick refused: %s", r.err);
    /* loop */
    { auto b = s98_notes(0); b.pop_back(); for (int i = 0; i < 100; i++) b.push_back(0xFF); size_t lp = b.size(); for (int i = 0; i < 200; i++) b.push_back(0xFF); b.push_back(0xFD);
      auto v = mk_s98(3, 1, 100, { {2, OPN_CLK} }, b, NULL, (int)lp);
      r = play_buf(v, 3000000); CHECK(r.open_rc == 0 && r.loops == 1 && r.len == 1000 + 2 * 2000, "loop: len %d loops %d", r.len, r.loops);
      CHECK(r.frames > (uint64_t)(5.0 + 8.0) * 55466 - 600 && r.frames < (uint64_t)(5.0 + 8.0) * 55466 + 600, "loop plays len + 8 s fade: %llu", (unsigned long long)r.frames); }
    /* 0xFE wait */
    { auto b = s98_notes(0); b.pop_back(); b.push_back(0xFE); b.push_back(0x80 | 0x7F); b.push_back(0x01); b.push_back(0xFD);       /* wait 0xFF + 2 ticks */
      r = play_buf(mk_s98(3, 1, 1000, { {2, OPN_CLK} }, b), 1000000); CHECK(r.open_rc == 0 && r.len == 257, "FE varlen wait (0xFF + 2 ticks of 1 ms): %d", r.len); }
    /* truncation and corruption: nothing may crash, the result is either a refusal or a short song */
    int ok = 0, refused = 0;
    for (size_t n = 0; n <= good.size(); n++) { std::vector<uint8_t> t(good.begin(), good.begin() + n); Res q = play_buf(t, 120000); if (q.open_rc == 0) ok++; else refused++; }
    printf("S98 truncated at every length: %d opened, %d refused\n", ok, refused); CHECK(refused > 40, "short files are refused");
    uint32_t seed = 12345; int opened = 0;
    for (int it = 0; it < 1500; it++) {
        std::vector<uint8_t> t = good;
        int flips = 1 + it % 6;
        for (int k = 0; k < flips; k++) { seed = seed * 1664525u + 1013904223u; size_t pos = (seed >> 8) % t.size(); seed = seed * 1664525u + 1013904223u; t[pos] = (uint8_t)(seed >> 16); }
        Res q = play_buf(t, 60000); opened += q.open_rc == 0;
    }
    printf("S98 corrupted x1500: %d opened (the rest refused), no crash\n", opened);
    CHECK(true, "");
}

static void selftest_vgm()
{
    std::vector<uint8_t> b; wr(b, 0x55, 0xB0, 7); wr(b, 0x55, 0xA4, 0x22); wr(b, 0x55, 0xA0, 0x69); wr(b, 0x55, 0x28, 0xF0); wait(b, 44100); b.push_back(0x66);
    auto v = mk_vgm(0x171, { {0x44, OPN_CLK} }, b, -1, 44100);
    Res r = play_buf(v);
    CHECK(r.open_rc == 0 && r.len == 1000 && r.frames > 55400 && r.frames < 55500, "plain YM2203: len %d frames %llu (%s)", r.len, (unsigned long long)r.frames, r.err);
    /* loop count and fade */
    { std::vector<uint8_t> c; wr(c, 0x54, 0x20, 0xC7); wait(c, 44100); size_t lp = c.size(); wr(c, 0x54, 0x28, 0x4A); wait(c, 44100); wait(c, 44100); c.push_back(0x66);
      auto w = mk_vgm(0x171, { {0x30, OPM_CLK} }, c, (int)lp, 44100 + 88200, 88200);
      r = play_buf(w, 2000000); CHECK(r.open_rc == 0 && r.loops == 1 && r.len == 1000 + 2 * 2000, "loop: len %d", r.len);
      CHECK(r.frames >= (uint64_t)((5.0 + 8.0) * 55930) - 600 && r.frames <= (uint64_t)((5.0 + 8.0) * 55930) + 600, "frames %llu", (unsigned long long)r.frames);
      fmvgm_set_loops(3); r = play_buf(w, 4000000); CHECK(r.len == 1000 + 3 * 2000, "3 loops: %d", r.len); fmvgm_set_loops(2); }
    /* dual chip: two instances, commands 0xA4 for the second */
    { std::vector<uint8_t> c; wr(c, 0x54, 0x20, 0xC7); wr(c, 0xA4, 0x20, 0xC7); wait(c, 4410); c.push_back(0x66);
      r = play_buf(mk_vgm(0x171, { {0x30, OPM_CLK | 0x40000000} }, c, -1, 4410)); CHECK(r.open_rc == 0 && r.chips == 2, "dual OPM: chips %d (%s)", r.chips, r.err); }
    /* unknown commands, data block, cut-off streams */
    { std::vector<uint8_t> c; wr(c, 0x55, 0xB0, 7); c.push_back(0x30); c.push_back(0x12); c.push_back(0x4F); c.push_back(0x00); c.push_back(0x67); c.push_back(0x66); c.push_back(0x00); c.push_back(4); c.push_back(0); c.push_back(0); c.push_back(0);
      c.insert(c.end(), { 1, 2, 3, 4 }); wait(c, 441); c.push_back(0x66);
      r = play_buf(mk_vgm(0x171, { {0x44, OPN_CLK} }, c, -1, 441)); CHECK(r.open_rc == 0 && r.frames > 540 && r.frames < 570, "skipped commands and a data block: %llu frames", (unsigned long long)r.frames);
      c.clear(); wr(c, 0x55, 0xB0, 7); c.push_back(0x67); c.push_back(0x66); c.push_back(0x00); c.push_back(0xFF); c.push_back(0xFF); c.push_back(0xFF); c.push_back(0x7F);   /* a data block longer than the file */
      r = play_buf(mk_vgm(0x171, { {0x44, OPN_CLK} }, c, -1, 441)); CHECK(r.open_rc == 0 && r.frames < 100, "oversize data block ends the song: %llu", (unsigned long long)r.frames);
      c.clear(); wr(c, 0x55, 0xB0, 7); c.push_back(0xE9); c.push_back(1);                                                                                   /* a command cut off by the end */
      r = play_buf(mk_vgm(0x171, { {0x44, OPN_CLK} }, c, -1, 0)); CHECK(r.open_rc == 0 && r.frames < 100, "cut-off command: %llu", (unsigned long long)r.frames);
      c.clear(); wr(c, 0x55, 0xB0, 7); c.push_back(0x2F);                                                                                                    /* an opcode no spec defines */
      r = play_buf(mk_vgm(0x171, { {0x44, OPN_CLK} }, c, -1, 0)); CHECK(r.open_rc == 0 && r.frames < 100, "unknown opcode ends the song: %llu", (unsigned long long)r.frames); }
    /* chips and clocks */
    { std::vector<uint8_t> c; wr(c, 0x54, 0x20, 0xC7); wait(c, 441); c.push_back(0x66);
      r = play_buf(mk_vgm(0x171, { {0x30, 12000000} }, c, -1, 441)); CHECK(r.open_rc != 0, "YM2151 at 12 MHz refused: %s", r.err);
      r = play_buf(mk_vgm(0x171, { {0x30, 0} }, c, -1, 441)); CHECK(r.open_rc != 0, "no clock, no chip: %s", r.err);
      std::vector<uint8_t> d; wr(d, 0x56, 0x20, 0xC7); wait(d, 441); d.push_back(0x66);
      r = play_buf(mk_vgm(0x171, { {0x48, 7987200} }, d, -1, 441)); CHECK(r.open_rc == 0 && r.chips == 1, "YM2608 plays: %s", r.err);
      std::vector<uint8_t> e; wr(e, 0x54, 0x20, 0xC7); e.push_back(0x50); e.push_back(0x9F); wait(e, 441); e.push_back(0x66);
      r = play_buf(mk_vgm(0x171, { {0x30, OPM_CLK}, {0x0C, 3579545} }, e, -1, 441)); CHECK(r.open_rc == 0 && r.chips == 2, "YM2151 + SN76489 written plays: %s", r.err);
      uint64_t used = 0, miss = 0; auto mix = mk_vgm(0x171, { {0x30, OPM_CLK}, {0x0C, 3579545} }, e, -1, 441);
      CHECK(fmvgm_route(mix.data(), (uint32_t)mix.size(), &used, &miss) == 0, "route: SN76489 + YM2151 is the FM backend's");
      std::vector<uint8_t> e2; wr(e2, 0x54, 0x20, 0xC7); e2.push_back(0x51); e2.push_back(0x20); e2.push_back(0x01); wait(e2, 441); e2.push_back(0x66);
      auto mix2 = mk_vgm(0x171, { {0x30, OPM_CLK}, {0x10, 3579545} }, e2, -1, 441);
      CHECK(fmvgm_route(mix2.data(), (uint32_t)mix2.size(), &used, &miss) == 2, "route: YM2413 + YM2151 is refused (mixed)");
      auto only = mk_vgm(0x171, { {0x30, OPM_CLK} }, c, -1, 441); CHECK(fmvgm_route(only.data(), (uint32_t)only.size(), &used, &miss) == 0, "route: fm only");
      std::vector<uint8_t> g; g.push_back(0x50); g.push_back(0x9F); wait(g, 441); g.push_back(0x66);
      auto psg = mk_vgm(0x171, { {0x0C, 3579545} }, g, -1, 441); CHECK(fmvgm_route(psg.data(), (uint32_t)psg.size(), &used, &miss) == 1, "route: gme only");
      auto decl = mk_vgm(0x171, { {0x0C, 3579545}, {0x30, OPM_CLK} }, g, -1, 441); CHECK(fmvgm_route(decl.data(), (uint32_t)decl.size(), &used, &miss) == 1, "route: a declared but never written YM2151 does not count");
      CHECK(fmvgm_route(c.data(), 10, &used, &miss) == -1, "route: junk"); }
    /* v1.50 header (0x40 bytes: whatever follows is data, not clocks) and v1.00 shared clock */
    { std::vector<uint8_t> v150(0x40, 0); memcpy(v150.data(), "Vgm ", 4); le32(v150, 8, 0x150); le32(v150, 0x34, 0); std::vector<uint8_t> c; wr(c, 0x54, 0x20, 0xC7); wait(c, 441); c.push_back(0x66);
      v150.insert(v150.end(), c.begin(), c.end()); le32(v150, 4, (uint32_t)v150.size() - 4); r = play_buf(v150); CHECK(r.open_rc != 0, "v1.50 without any clock: %s", r.err);
      std::vector<uint8_t> v100(0x40, 0); memcpy(v100.data(), "Vgm ", 4); le32(v100, 8, 0x100); le32(v100, 0x10, OPM_CLK); v100.insert(v100.end(), c.begin(), c.end()); le32(v100, 4, (uint32_t)v100.size() - 4);
      r = play_buf(v100); CHECK(r.open_rc == 0 && r.chips == 1, "v1.00 YM2151 in the shared field: %s", r.err); }
    /* GD3 */
    { std::vector<uint8_t> c; wr(c, 0x54, 0x20, 0xC7); wait(c, 441); c.push_back(0x66);
      auto w = mk_vgm(0x171, { {0x30, OPM_CLK} }, c, -1, 441);
      auto u16 = [](const char *s16, std::vector<uint8_t> &o) { for (const char *p = s16; *p; p++) { o.push_back((uint8_t)*p); o.push_back(0); } o.push_back(0); o.push_back(0); };
      std::vector<uint8_t> g; g.insert(g.end(), { 'G', 'd', '3', ' ', 0, 1, 0, 0, 0, 0, 0, 0 }); std::vector<uint8_t> body;
      u16("Song", body); u16("", body); u16("Game", body); u16("", body); u16("Sys", body); u16("", body); u16("Auth", body); u16("", body); u16("2026", body); u16("c", body); u16("n", body);
      /* a supplementary-plane character (surrogate pair) in the title would go here: kept simple, the converter is checked by gen_fm_vgm's file */
      le32(g, 8, (uint32_t)body.size()); g.insert(g.end(), body.begin(), body.end());
      le32(w, 0x14, (uint32_t)w.size() - 0x14); w.insert(w.end(), g.begin(), g.end()); le32(w, 4, (uint32_t)w.size() - 4);
      fmvgm_info_t in; fmvgm_tags_t tg; CHECK(fmvgm_probe(w.data(), (uint32_t)w.size(), &in, &tg) == 0 && !strcmp(tg.title, "Song") && !strcmp(tg.game, "Game") && !strcmp(tg.author, "Auth") && !strcmp(tg.system, "Sys"), "GD3: %s/%s/%s/%s", tg.title, tg.game, tg.author, tg.system);
      for (size_t n = w.size() - g.size(); n <= w.size(); n++) { std::vector<uint8_t> t(w.begin(), w.begin() + n); play_buf(t, 2000); }          /* GD3 cut at every length */
      CHECK(true, ""); }
    /* corrupted VGMs */
    { std::vector<uint8_t> c; wr(c, 0x54, 0x20, 0xC7); wr(c, 0x54, 0x28, 0x4A); wait(c, 4410); c.push_back(0x66); auto w = mk_vgm(0x171, { {0x30, OPM_CLK} }, c, -1, 4410);
      uint32_t seed = 99; int opened = 0;
      for (int it = 0; it < 3000; it++) { std::vector<uint8_t> t = w; int flips = 1 + it % 5; for (int k = 0; k < flips; k++) { seed = seed * 1664525u + 1013904223u; size_t pos = (seed >> 8) % std::min<size_t>(t.size(), 0x140); seed = seed * 1664525u + 1013904223u; t[pos] = (uint8_t)(seed >> 16); } opened += play_buf(t, 20000).open_rc == 0; }
      for (size_t n = 0; n <= w.size(); n++) { std::vector<uint8_t> t(w.begin(), w.begin() + n); play_buf(t, 20000); }
      printf("VGM corrupted x3000 (%d opened) and truncated at every length: no crash\n", opened); CHECK(true, ""); }
}


/* data blocks: ADPCM / ROM images of the YM2610, YM2608 and the OKIM6295 (pieces, re-sends, out-of-range offsets, truncation, too many pieces, the second-chip bit) */
static std::vector<uint8_t> blk(uint8_t type, uint32_t total, uint32_t start, const std::vector<uint8_t> &data, bool second = false, bool header = true)
{
    std::vector<uint8_t> b; b.push_back(0x67); b.push_back(0x66); b.push_back(type);
    uint32_t sz = (uint32_t)data.size() + (header ? 8 : 0); if (second) sz |= 0x80000000u;
    for (int i = 0; i < 4; i++) b.push_back((uint8_t)(sz >> (8 * i)));
    if (header) { for (int i = 0; i < 4; i++) b.push_back((uint8_t)(total >> (8 * i))); for (int i = 0; i < 4; i++) b.push_back((uint8_t)(start >> (8 * i))); }
    b.insert(b.end(), data.begin(), data.end()); return b;
}
static std::vector<uint8_t> rnd_bytes(size_t n, uint32_t seed) { std::vector<uint8_t> v(n); for (auto &x : v) { seed = seed * 1664525u + 1013904223u; x = (uint8_t)(seed >> 16); } return v; }
static std::vector<uint8_t> opnb_song(const std::vector<uint8_t> &blocks)
{
    std::vector<uint8_t> c = blocks;
    wr(c, 0x59, 0x01, 0x3F); wr(c, 0x59, 0x08, 0xDF); wr(c, 0x59, 0x10, 0x00); wr(c, 0x59, 0x18, 0x00); wr(c, 0x59, 0x20, 0x08); wr(c, 0x59, 0x28, 0x00); wr(c, 0x59, 0x00, 0x01);     /* ADPCM-A channel 0 */
    wr(c, 0x58, 0x11, 0xC0); wr(c, 0x58, 0x1B, 0xC0); wr(c, 0x58, 0x12, 0); wr(c, 0x58, 0x13, 0); wr(c, 0x58, 0x14, 0x08); wr(c, 0x58, 0x15, 0); wr(c, 0x58, 0x19, 0x00); wr(c, 0x58, 0x1A, 0x40); wr(c, 0x58, 0x10, 0x80);   /* ADPCM-B */
    wait(c, 22050); c.push_back(0x66);
    return mk_vgm(0x171, { {0x4C, 8000000} }, c, -1, 22050);
}
static void selftest_blocks()
{
    auto rom = rnd_bytes(0x1000, 5);
    auto cat = [](std::vector<uint8_t> a, const std::vector<uint8_t> &b) { a.insert(a.end(), b.begin(), b.end()); return a; };
    fmvgm_set_checksum(1);
    Res none = play_buf(opnb_song({}));
    Res one = play_buf(opnb_song(blk(0x82, 0x1000, 0, rom)));
    CHECK(one.open_rc == 0 && one.out_hash != none.out_hash, "a ROM block changes the sound: %08x vs %08x", one.out_hash, none.out_hash);
    /* pieces: 4 pieces of the same ROM = one block; the first piece sent again = the same */
    std::vector<uint8_t> p4;
    for (int q = 0; q < 4; q++) p4 = cat(p4, blk(0x82, 0x1000, q * 0x400, std::vector<uint8_t>(rom.begin() + q * 0x400, rom.begin() + (q + 1) * 0x400)));
    Res pieces = play_buf(opnb_song(p4)), resend = play_buf(opnb_song(cat(p4, blk(0x82, 0x1000, 0, std::vector<uint8_t>(rom.begin(), rom.begin() + 0x400)))));
    Res reorder = play_buf(opnb_song(cat(cat(blk(0x82, 0x1000, 0xC00, std::vector<uint8_t>(rom.begin() + 0xC00, rom.end())), blk(0x82, 0x1000, 0, std::vector<uint8_t>(rom.begin(), rom.begin() + 0xC00))), {})));
    CHECK(pieces.out_hash == one.out_hash, "4 pieces = 1 block: %08x vs %08x", pieces.out_hash, one.out_hash);
    CHECK(resend.out_hash == one.out_hash, "a re-sent piece changes nothing: %08x vs %08x", resend.out_hash, one.out_hash);
    CHECK(reorder.out_hash == one.out_hash, "pieces in another order: %08x vs %08x", reorder.out_hash, one.out_hash);
    /* a re-send with different data replaces it (the newest wins) */
    auto rom2 = rnd_bytes(0x1000, 99);
    Res replaced = play_buf(opnb_song(cat(blk(0x82, 0x1000, 0, rom), blk(0x82, 0x1000, 0, rom2)))), second = play_buf(opnb_song(blk(0x82, 0x1000, 0, rom2)));
    CHECK(replaced.out_hash == second.out_hash, "newest block wins: %08x vs %08x", replaced.out_hash, second.out_hash);
    /* out-of-range and malformed blocks: ignored, never a crash, nothing stored */
    Res oor1 = play_buf(opnb_song(blk(0x82, 0x1000, 0x1000, rom)));                   /* start == total */
    Res oor2 = play_buf(opnb_song(blk(0x82, 0x1000, 0xFFFFFFF0u, rom)));               /* start near 2^32 */
    Res oor3 = play_buf(opnb_song(blk(0x82, 0, 0, rom)));                              /* total 0 */
    Res tiny = play_buf(opnb_song(blk(0x82, 0, 0, {1, 2, 3}, false, false)));          /* a block smaller than its own header */
    Res dualflag = play_buf(opnb_song(blk(0x82, 0x1000, 0, rom, true)));               /* for a second chip that does not exist */
    Res huge = play_buf(opnb_song(blk(0x82, 0xFFFFFFFFu, 0, rom)));                    /* a declared size of 4 GB: referenced in place, nothing allocated */
    Res clipped = play_buf(opnb_song(blk(0x82, 0x800, 0x400, std::vector<uint8_t>(rom.begin(), rom.begin() + 0x1000))));   /* data longer than the declared ROM: clipped */
    CHECK(oor1.open_rc == 0 && oor1.out_hash == none.out_hash, "start == total ignored");
    CHECK(oor2.open_rc == 0 && oor2.out_hash == none.out_hash, "start near 2^32 ignored");
    CHECK(oor3.open_rc == 0 && oor3.out_hash == none.out_hash, "total 0 ignored");
    CHECK(tiny.open_rc == 0 && tiny.out_hash == none.out_hash, "block smaller than its header ignored");
    CHECK(dualflag.open_rc == 0 && dualflag.out_hash == none.out_hash, "a block for a missing second chip ignored");
    CHECK(huge.open_rc == 0 && huge.out_hash == one.out_hash, "declared size 4 GB, data in place: %08x vs %08x", huge.out_hash, one.out_hash);
    CHECK(clipped.open_rc == 0, "data longer than total: %s", clipped.err);
    { auto v = opnb_song(blk(0x82, 0x1000, 0, rom)); v.resize(v.size() - 0x600); Res t = play_buf(v); CHECK(t.open_rc == 0, "a truncated file opens: %s", t.err); }
    { auto b = blk(0x82, 0x1000, 0, rom); b.resize(b.size() - 100); b = cat(b, {0x66}); Res t = play_buf(opnb_song(b)); CHECK(t.open_rc == 0 && t.frames < 100, "a block cut short ends the song: %llu", (unsigned long long)t.frames); }
    /* too many pieces: refused at open with a reason */
    { std::vector<uint8_t> many; for (int q = 0; q < 40; q++) many = cat(many, blk(0x82, 0x10000, q * 0x100, std::vector<uint8_t>(0x100, (uint8_t)q)));
      Res t = play_buf(opnb_song(many)); CHECK(t.open_rc != 0 && strstr(t.err, "ADPCM"), "40 distinct pieces: %s", t.err); }
    /* probe reports the data size */
    { auto v = opnb_song(blk(0x82, 0x1000, 0, rom)); fmvgm_info_t in; fmvgm_probe(v.data(), (uint32_t)v.size(), &in, NULL); CHECK(in.adpcm_bytes == 0x1000, "probe: adpcm_bytes %u", in.adpcm_bytes); }
    /* YM2608: DELTA-T block into the 256 KB RAM, out-of-range start ignored */
    { std::vector<uint8_t> c = blk(0x81, 0x40000, 0x100, rnd_bytes(0x800, 3)); wr(c, 0x56, 0x29, 0x80); wr(c, 0x57, 0x01, 0xC0); wr(c, 0x57, 0x0B, 0xC0); wr(c, 0x57, 0x02, 0x40); wr(c, 0x57, 0x03, 0); wr(c, 0x57, 0x04, 0xC0); wr(c, 0x57, 0x05, 0); wr(c, 0x57, 0x09, 0); wr(c, 0x57, 0x0A, 0x40); wr(c, 0x57, 0x0C, 0xFF); wr(c, 0x57, 0x0D, 0xFF); wr(c, 0x57, 0x00, 0xA0);
      wait(c, 22050); c.push_back(0x66); auto v = mk_vgm(0x171, { {0x48, 7987200} }, c, -1, 22050);
      std::vector<uint8_t> c2 = c; c2.erase(c2.begin(), c2.begin() + 7 + 8 + 0x800); auto v0 = mk_vgm(0x171, { {0x48, 7987200} }, c2, -1, 22050);
      std::vector<uint8_t> c3 = blk(0x81, 0x40000, 0x80000, rnd_bytes(0x800, 3)); c3.insert(c3.end(), c2.begin(), c2.end()); auto v3 = mk_vgm(0x171, { {0x48, 7987200} }, c3, -1, 22050);
      Res a = play_buf(v), b = play_buf(v0), d = play_buf(v3);
      CHECK(a.open_rc == 0 && a.out_hash != b.out_hash, "YM2608 DELTA-T block is played: %08x vs %08x", a.out_hash, b.out_hash);
      CHECK(d.open_rc == 0 && d.out_hash == b.out_hash, "YM2608 DELTA-T block beyond 256 KB ignored"); }
    /* OKIM6295: ROM block, ROM too large, a block for a missing chip */
    { std::vector<uint8_t> c = blk(0x8B, 0x1000, 0, rnd_bytes(0x1000, 8)); c.push_back(0xB8); c.push_back(0x00); c.push_back(0x81); c.push_back(0xB8); c.push_back(0x00); c.push_back(0x10); wait(c, 22050); c.push_back(0x66);
      Res a = play_buf(mk_vgm(0x171, { {0x98, 1056000} }, c, -1, 22050)); CHECK(a.open_rc == 0, "OKI with a ROM block: %s", a.err);
      auto c2 = blk(0x8B, 0x800000, 0, rnd_bytes(0x1000, 8)); c2.insert(c2.end(), c.begin() + 7 + 8 + 0x1000, c.end());
      Res b = play_buf(mk_vgm(0x171, { {0x98, 1056000} }, c2, -1, 22050)); CHECK(b.open_rc != 0 && strstr(b.err, "ROM"), "an 8 MB OKI ROM is refused: %s", b.err); }
    fmvgm_set_checksum(0);
    /* fuzz: flips in the header and the blocks of a file with pieces, ASan / UBSan must stay quiet */
    { auto v = opnb_song(cat(p4, blk(0x83, 0x800, 0, rnd_bytes(0x800, 4)))); uint32_t seed = 4242; int opened = 0;
      for (int it = 0; it < 2500; it++) { auto t = v; int flips = 1 + it % 6; for (int k = 0; k < flips; k++) { seed = seed * 1664525u + 1013904223u; size_t pos = 0x4C + (seed >> 8) % std::min<size_t>(t.size() - 0x4C, 0x1800); seed = seed * 1664525u + 1013904223u; t[pos] = (uint8_t)(seed >> 16); } opened += play_buf(t, 30000).open_rc == 0; }
      printf("blocks fuzz x2500: %d opened, no crash\n", opened); CHECK(true, ""); }
}

/* the YM2612's PCM bank and 0x8n commands, YM2608 without its rhythm ROM, partial / drop mask */
static void selftest_misc_chips()
{
    std::vector<uint8_t> c; auto bank = rnd_bytes(500, 2); auto b0 = blk(0x00, 0, 0, bank, false, false); c = b0;
    wr(c, 0x52, 0x2B, 0x80); wr(c, 0x52, 0xB4, 0xC0); for (int i = 0; i < 400; i++) c.push_back(0x85); c.push_back(0x66);     /* 400 DAC writes, each followed by a 5-sample wait */
    auto v = mk_vgm(0x171, { {0x2C, 7670454} }, c, -1, 2000);
    Res r = play_buf(v); CHECK(r.open_rc == 0 && r.frames > 2000 * 53267 / 44100 - 60, "YM2612 DAC: %llu frames", (unsigned long long)r.frames);
    { auto c2 = c; c2.erase(c2.begin(), c2.begin() + b0.size()); Res q = play_buf(mk_vgm(0x171, { {0x2C, 7670454} }, c2, -1, 2000)); fmvgm_set_checksum(1); Res a = play_buf(v), b = play_buf(mk_vgm(0x171, { {0x2C, 7670454} }, c2, -1, 2000)); fmvgm_set_checksum(0);
      CHECK(a.out_hash != b.out_hash, "the PCM bank is audible"); (void)q; }
    /* a DAC stream command on a YM2612 that the FM backend plays is refused by name */
    { std::vector<uint8_t> d = c; d.insert(d.begin() + b0.size(), { 0x90, 0, 0x02, 0x00, 0x2A }); Res q = play_buf(mk_vgm(0x171, { {0x2C, 7670454}, {0x30, 3579545} }, d, -1, 2000)); CHECK(q.open_rc != 0 && strstr(q.err, "DAC"), "DAC stream: %s", q.err); }
    /* the drop mask: a SEGAPCM write next to a YM2151 */
    { std::vector<uint8_t> e; wr(e, 0x54, 0x20, 0xC7); e.push_back(0xC0); e.push_back(0x00); e.push_back(0x00); e.push_back(0x10); wait(e, 441); e.push_back(0x66);   /* 0xC0 = SegaPCM memory write */
      auto w = mk_vgm(0x171, { {0x30, OPM_CLK}, {0x38, 4000000} }, e, -1, 441);
      fmvgm_set_drop(0); Res a = play_buf(w); fmvgm_set_drop(PC_VGM_BIT(PC_VGM_SEGAPCM)); Res b = play_buf(w);
      fmvgm_info_t in; int pr = fmvgm_probe(w.data(), (uint32_t)w.size(), &in, NULL); fmvgm_set_drop(0);
      CHECK(a.open_rc != 0 && strstr(a.err, "SegaPCM"), "a sample chip is refused without a drop mask: %s", a.err);
      CHECK(b.open_rc == 0 && pr == 0, "and left out with it");
      uint64_t used, miss; CHECK(fmvgm_route(w.data(), (uint32_t)w.size(), &used, &miss) == 3 && (miss & PC_VGM_BIT(PC_VGM_SEGAPCM)), "route: 3, missing SEGAPCM");
      /* the partial-playback policy of vgmgate.c */
      char why[48]; nsn_vgm_partial_t pt;
      CHECK(nsn_vgm_route(w.data(), w.size(), why, sizeof why, 1, &pt) == NSN_VGM_FM && pt.drop == PC_VGM_BIT(PC_VGM_SEGAPCM) && !strcmp(pt.names, "SegaPCM"), "partial on: YM2151 + SegaPCM plays in the FM backend without it (%s)", pt.names);
      CHECK(nsn_vgm_route(w.data(), w.size(), why, sizeof why, 0, &pt) < 0 && !strcmp(why, "SegaPCM") && !pt.drop, "partial off: refused, naming the chip (%s)", why);
      std::vector<uint8_t> g; g.push_back(0x52); g.push_back(0x22); g.push_back(0x00); g.push_back(0xC0); g.push_back(0); g.push_back(0); g.push_back(0x10); wait(g, 441); g.push_back(0x66);
      auto md = mk_vgm(0x171, { {0x2C, 7670454}, {0x38, 4000000} }, g, -1, 441);
      CHECK(nsn_vgm_route(md.data(), md.size(), why, sizeof why, 1, &pt) == NSN_VGM_GME && pt.drop, "partial on: YM2612 + SegaPCM goes to GME (it ignores the PCM chip)");
      std::vector<uint8_t> q; q.push_back(0xC4); q.push_back(0); q.push_back(0); q.push_back(1); wait(q, 441); q.push_back(0x66);
      auto qs = mk_vgm(0x171, { {0xB4, 4000000} }, q, -1, 441);
      CHECK(nsn_vgm_route(qs.data(), qs.size(), why, sizeof why, 1, &pt) < 0, "a sample chip alone has nothing to play: refused (%s)", why);
      std::vector<uint8_t> k; wr(k, 0x54, 0x20, 0xC7); k.push_back(0xD2); k.push_back(0); k.push_back(0); k.push_back(0); wait(k, 441); k.push_back(0x66);
      auto ks = mk_vgm(0x171, { {0x30, OPM_CLK}, {0x9C, 1789773} }, k, -1, 441);
      CHECK(nsn_vgm_route(ks.data(), ks.size(), why, sizeof why, 1, &pt) < 0 && strstr(why, "K051649"), "a melodic chip (K051649) is never dropped: %s", why); }
}

/* a producer on a worker thread: start / stop / seek / close in every order, the placement switched while rendering. Under TSan / ASan this is the race test. */
static void selftest_worker()
{
    auto d = slurp("/tmp/nsn_fm/cases/ym2151_typical.vgm");
    if (d.empty()) { printf("worker test skipped (no /tmp/nsn_fm/cases)\n"); return; }
    fmvgm_set_port(&k_port);
    std::vector<float> buf(1024 * 2);
    uint32_t seed = 7;
    for (int it = 0; it < 60; it++) {
        fmvgm_set_out_rate(it & 1 ? 48000 : 44100);
        fmvgm_set_placement(it % 3);
        pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size());
        void *h = NULL; if (pc_backend_fmvgm.open(&h, &rd, 0)) { CHECK(false, "open %s", fmvgm_error()); return; }
        int blocks = 3 + (seed = seed * 1664525u + 1013904223u, (seed >> 16) % 40);
        for (int b = 0; b < blocks; b++) {
            seed = seed * 1664525u + 1013904223u;
            if ((seed >> 20) % 5 == 0) fmvgm_set_placement((seed >> 8) % 3);
            if ((seed >> 20) % 17 == 1) pc_backend_fmvgm.start_track(h, 0);
            int n = pc_backend_fmvgm.render(h, buf.data(), 1024); if (n <= 0) break;
        }
        pc_backend_fmvgm.close(h);                                   /* close at any moment: a worker may be inside a chunk */
    }
    CHECK(true, "");
    /* an unthrottled run must equal the inline run bit for bit, whatever the thread did */
    fmvgm_set_checksum(1); fmvgm_set_out_rate(48000); uint32_t hs[3];
    for (int p = 0; p < 3; p++) {
        fmvgm_set_placement(p);
        pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size()); void *h = NULL; pc_backend_fmvgm.open(&h, &rd, 0);
        for (int n; (n = pc_backend_fmvgm.render(h, buf.data(), 1024)) > 0; ) {}
        fmvgm_sums_t s; fmvgm_sums(h, &s); hs[p] = s.out_hash; pc_backend_fmvgm.close(h);
    }
    CHECK(hs[0] == hs[1] && hs[1] == hs[2], "placements give %08x %08x %08x", hs[0], hs[1], hs[2]);
    /* a restart gives the same bytes again */
    { fmvgm_set_placement(1); pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size()); void *h = NULL; pc_backend_fmvgm.open(&h, &rd, 0);
      for (int i = 0; i < 20; i++) pc_backend_fmvgm.render(h, buf.data(), 1024);
      pc_backend_fmvgm.start_track(h, 0);
      for (int n; (n = pc_backend_fmvgm.render(h, buf.data(), 1024)) > 0; ) {}
      fmvgm_sums_t s; fmvgm_sums(h, &s); CHECK(s.out_hash == hs[0], "restart: %08x vs %08x", s.out_hash, hs[0]); pc_backend_fmvgm.close(h); }
    fmvgm_set_placement(0); fmvgm_set_checksum(0); fmvgm_set_out_rate(0);
}


/* split rendering: chips on two (or three) lanes, every plan gives the same bytes as the single-threaded reference; start / stop / seek / close in every order with workers on both cores
 * (run under ThreadSanitizer: `sh run_fm.sh tsan`) */
static void selftest_split()
{
    const char *files[] = { "/tmp/nsn_fm/cases/mix_opm_opn.vgm", "/tmp/nsn_fm/cases/ym2203_dual.vgm", "/tmp/nsn_fm/cases2/ym2151_oki6295.vgm", "/tmp/nsn_fm/cases2/ym2151_sn76489.vgm", "/tmp/nsn_fm/cases2/ym2610_dual.vgm", "/tmp/nsn_fm/cases2/ym2612_ym2151_dac.vgm" };
    const char *plans[] = { "00", "11", "22", "12", "21", "10", "01", "02", "20" };            /* the port is installed once at the start of selftest: fmvgm_set_port is not for a library with running workers */
    std::vector<float> buf(1024 * 2);
    for (const char *f : files) {
        auto d = slurp(f);
        if (d.empty()) { printf("split test skipped for %s\n", f); continue; }
        fmvgm_set_checksum(1); fmvgm_set_out_rate(48000);
        uint32_t ref = 0; bool first = true;
        for (const char *pl : plans) {
            uint8_t p[2] = { (uint8_t)(pl[0] - '0'), (uint8_t)(pl[1] - '0') }; fmvgm_set_plan(p, 2);
            pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size()); void *h = NULL;
            if (pc_backend_fmvgm.open(&h, &rd, 0)) { CHECK(false, "%s open: %s", f, fmvgm_error()); break; }
            fmvgm_info_t in; fmvgm_info(h, &in);
            int lanes = 1 + (pl[0] != pl[1]);
            CHECK(in.lanes == lanes, "%s plan %s: %d lanes", f, pl, in.lanes);
            for (int n; (n = pc_backend_fmvgm.render(h, buf.data(), 1024)) > 0; ) {}
            fmvgm_sums_t s; fmvgm_sums(h, &s); pc_backend_fmvgm.close(h);
            if (first) { ref = s.out_hash; first = false; }
            CHECK(s.out_hash == ref, "%s plan %s: %08x vs %08x", strrchr(f, '/') + 1, pl, s.out_hash, ref);
        }
        /* the reference: producers only, one thread, no ring (bench) */
        { fmvgm_set_plan(NULL, 0); fmvgm_set_placement(0); pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size()); void *h = NULL; pc_backend_fmvgm.open(&h, &rd, 0);
          fmvgm_bench(h, 0, NULL); fmvgm_sums_t s; fmvgm_sums(h, &s); CHECK(s.out_hash == ref, "%s bench: %08x vs %08x", strrchr(f, '/') + 1, s.out_hash, ref); pc_backend_fmvgm.close(h); }
    }
    /* races: random plans, restarts, close at any moment */
    auto d = slurp("/tmp/nsn_fm/cases/mix_opm_opn.vgm"); if (d.empty()) return;
    uint32_t seed = 11; fmvgm_set_checksum(0);
    for (int it = 0; it < 80; it++) {
        uint8_t p[2]; for (int k = 0; k < 2; k++) { seed = seed * 1664525u + 1013904223u; p[k] = (seed >> 20) % 3; } fmvgm_set_plan(p, 2);
        fmvgm_set_out_rate(it & 1 ? 48000 : 44100);
        pc_reader_t rd; pc_reader_mem(&rd, d.data(), (uint32_t)d.size()); void *h = NULL;
        if (pc_backend_fmvgm.open(&h, &rd, 0)) { CHECK(false, "open %s", fmvgm_error()); break; }
        int blocks = 2 + (seed = seed * 1664525u + 1013904223u, (seed >> 16) % 50);
        for (int b = 0; b < blocks; b++) {
            seed = seed * 1664525u + 1013904223u;
            if ((seed >> 20) % 13 == 1) pc_backend_fmvgm.start_track(h, 0);
            if ((seed >> 20) % 7 == 2) fmvgm_set_placement((seed >> 8) % 3);                    /* ignored by a split handle, migrates a single-lane one */
            int n = pc_backend_fmvgm.render(h, buf.data(), 1024); if (n <= 0) break;
        }
        pc_backend_fmvgm.close(h);
    }
    CHECK(true, "");
    fmvgm_set_plan(NULL, 0); fmvgm_set_placement(0); fmvgm_set_out_rate(0);
}

static int cmd_selftest()
{
    fmvgm_set_port(&k_port);
    selftest_s98(); selftest_vgm(); selftest_blocks(); selftest_misc_chips(); selftest_worker(); selftest_split();
    printf("selftest: %d checks passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

int main(int argc, char **argv)
{
    pthread_key_create(&g_key, NULL);
    if (argc >= 3 && !strcmp(argv[1], "run")) return cmd_run(argc, argv, false);
    if (argc >= 3 && !strcmp(argv[1], "bench")) return cmd_run(argc, argv, true);
    if (argc >= 3 && (!strcmp(argv[1], "route") || !strcmp(argv[1], "route-strict"))) {
        int partial = !strcmp(argv[1], "route");                   // route FILE...: where a .vgm / .vgz goes (what the player does after un-gzipping)
        for (int i = 2; i < argc; i++) {
            std::vector<uint8_t> d = slurp(argv[i]); char why[48] = ""; nsn_vgm_partial_t pt;
            int r = nsn_vgm_route(d.data(), d.size(), why, sizeof why, partial, &pt);
            printf("%-40s %s%s%s%s%s\n", strrchr(argv[i], '/') ? strrchr(argv[i], '/') + 1 : argv[i], r == 0 ? "GME" : r == 1 ? "FM" : "REFUSED", why[0] ? "  " : "", why, pt.drop ? "  partial, missing " : "", pt.names);
        }
        return 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "selftest")) return cmd_selftest();
    fprintf(stderr, "usage: test_fmvgm run|bench FILE [...] | selftest\n");
    return 2;
}
