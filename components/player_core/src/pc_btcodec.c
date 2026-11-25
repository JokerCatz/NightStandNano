// SPDX-License-Identifier: LGPL-2.1-or-later
/* Bluetooth A2DP codec helpers (see pc_btcodec.h). No ESP-IDF here: the player and the test peer share it. */
#include "pc/pc_btcodec.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

static const char *const NAMES[PC_BTC_N] = { "LDAC", "aptX HD", "aptX", "AAC", "SBC" };
const char *pc_btc_name(pc_btc_t c) { return c >= 0 && c < PC_BTC_N ? NAMES[c] : "?"; }

pc_btc_t pc_btc_pick(unsigned mask)
{
    for (int c = 0; c < PC_BTC_N; c++)
        if (mask & (1u << c)) return (pc_btc_t)c;
    return PC_BTC_NONE;
}

int pc_btc_order(unsigned built, pc_btc_t out[PC_BTC_N])
{
    int n = 0;
    for (int c = 0; c < PC_BTC_N; c++)
        if (built & (1u << c)) out[n++] = (pc_btc_t)c;
    return n;
}

pc_btc_enc_t pc_btc_encoder(pc_btc_t c)
{
    if (c == PC_BTC_LDAC || c == PC_BTC_APTX_HD || c == PC_BTC_APTX) return PC_ENC_VENDOR;
    if (c == PC_BTC_AAC || c == PC_BTC_SBC) return PC_ENC_STD;
    return PC_ENC_NONE;
}

bool pc_btc_fits_mtu(pc_btc_t c, int mtu)
{
    switch (c) {
    case PC_BTC_LDAC: return mtu >= PC_LDAC_MTU_MIN;
    case PC_BTC_APTX_HD: return mtu >= 12 + pc_aptx_codeword_bytes(true);   /* RTP header + one codeword */
    case PC_BTC_APTX: return mtu >= pc_aptx_codeword_bytes(false);
    case PC_BTC_AAC:
    case PC_BTC_SBC: return true;
    default: return false;
    }
}

/* ---------------- LDAC ---------------- */
/* libldac's 2-DH5 table (ldacBT_internal.c): HQ, SQ, Q0, MQ */
static const pc_ldac_level_t LEVELS[PC_LDAC_LEVELS] = { { 990, 330, 2 }, { 660, 220, 3 }, { 492, 164, 4 }, { 330, 110, 6 } };
#define LDAC_TX_BUDGET (679 - 18)     /* LDACBT_MTU_REQUIRED - LDACBT_TX_HEADER_SIZE */

const pc_ldac_level_t *pc_ldac_level(int i) { return i >= 0 && i < PC_LDAC_LEVELS ? &LEVELS[i] : NULL; }

int pc_ldac_level_for_kbps(int kbps)
{
    for (int i = 0; i < PC_LDAC_LEVELS; i++)
        if (LEVELS[i].kbps <= kbps) return i;
    return PC_LDAC_LEVELS - 1;
}

int pc_ldac_kbps(int frame_bytes, uint32_t rate)
{
    return (int)((uint64_t)frame_bytes * 8u * rate / PC_LDAC_FRAME_SAMPLES / 1000);   /* as libldac: truncated */
}

int pc_ldac_frames_per_packet(int frame_bytes, int mtu)
{
    if (frame_bytes <= 0) return 0;
    int n = LDAC_TX_BUDGET / frame_bytes;
    while (n > 1 && n * frame_bytes > mtu - 13) n--;
    return n;
}

int pc_ldac_abr_cap(bool wifi_on, int max_kbps)
{
    if (wifi_on && max_kbps > 660) max_kbps = 660;
    return pc_ldac_level_for_kbps(max_kbps);
}

void pc_ldac_abr_init(pc_ldac_abr_t *a, bool wifi_on, int max_kbps)
{
    *a = (pc_ldac_abr_t){ 0 };
    a->level = pc_ldac_abr_cap(wifi_on, max_kbps);
    a->up_wait_ms = PC_ABR_UP_MS;
}

int pc_ldac_abr_update(pc_ldac_abr_t *a, uint32_t now, int queued, int lost, bool wifi_on, int max_kbps)
{
    int cap = pc_ldac_abr_cap(wifi_on, max_kbps);
    if (!a->started) {
        a->started = true;
        a->quiet_since_ms = now;
        a->last_down_ms = now - PC_ABR_DOWN_GAP_MS;
    }
    if (a->level < cap) { a->level = cap; a->quiet_since_ms = now; }      /* Wi-Fi came on, or a lower top rate */
    if (a->ups && !a->held && now - a->last_up_ms >= PC_ABR_UP_FAIL_MS) {   /* the last up-step held */
        a->held = true;
        a->up_wait_ms /= 2;
        if (a->up_wait_ms < PC_ABR_UP_MS) a->up_wait_ms = PC_ABR_UP_MS;
    }
    if (queued >= PC_ABR_QUEUE_DEEP) {
        if (!a->deep) { a->deep = true; a->deep_since_ms = now; }
    } else a->deep = false;
    bool down = lost > 0 || queued >= PC_ABR_QUEUE_CRITICAL || (a->deep && now - a->deep_since_ms >= PC_ABR_DEEP_MS);
    if (lost > 0 || queued > PC_ABR_QUEUE_QUIET) a->quiet_since_ms = now;
    if (down && now - a->last_down_ms >= PC_ABR_DOWN_GAP_MS && a->level < PC_LDAC_LEVELS - 1) {
        if (a->ups && now - a->last_up_ms < PC_ABR_UP_FAIL_MS) {               /* the last up-step did not hold */
            a->up_wait_ms *= 2;
            if (a->up_wait_ms > PC_ABR_UP_MAX_MS) a->up_wait_ms = PC_ABR_UP_MAX_MS;
        }
        a->level++;
        a->downs++;
        a->held = true;                                                    /* the last up-step is settled */
        a->last_down_ms = now;
        a->deep = false;
        a->quiet_since_ms = now;
    } else if (a->level > cap && now - a->quiet_since_ms >= a->up_wait_ms) {
        a->level--;
        a->ups++;
        a->last_up_ms = now;
        a->held = false;
        a->quiet_since_ms = now;
    }
    return a->level;
}

int pc_ldac_rate_id(uint32_t rate)
{
    return rate == 44100 ? 0 : rate == 48000 ? 1 : rate == 88200 ? 2 : rate == 96000 ? 3 : -1;
}

/* frame header (pack_ldac.c): sync 0xAA (8 bits), sampling-rate id (3), channel config (2), frame length - 1 (9),
 * frame status (2), MSB first; the frame length counts the octets after this 3-octet header */
void pc_ldac_check(const uint8_t *p, int len, int hdr_frames, int want_rate, int want_ch, pc_ldac_check_t *o)
{
    *o = (pc_ldac_check_t){ .rate_id = -1, .ch_id = -1 };
    int pos = 0;
    while (pos + 3 <= len) {
        if (p[pos] != 0xAA) break;
        int rate = p[pos + 1] >> 5, ch = (p[pos + 1] >> 3) & 3;
        int flen = (((p[pos + 1] & 7) << 6) | (p[pos + 2] >> 2)) + 1 + 3;
        if (pos + flen > len) break;
        bool good = (want_rate < 0 || rate == want_rate) && (want_ch < 0 || ch == want_ch);
        if (!good) o->bad_frames++;
        if (!o->frames) { o->frame_bytes = flen; o->rate_id = rate; o->ch_id = ch; }
        else if (flen != o->frame_bytes) o->bad_frames++;                  /* one packet, one frame size */
        o->frames++;
        pos += flen;
    }
    if (pos != len) o->bad_frames++;                                      /* no sync, or the frames do not fit */
    if (o->frames != hdr_frames) o->bad_frames++;
    o->ok = o->bad_frames == 0;
}

/* ---------------- aptX ---------------- */
int pc_aptx_codeword_bytes(bool hd) { return hd ? 6 : 4; }
int pc_aptx_packet_bytes(bool hd, int samples) { return samples / 4 * pc_aptx_codeword_bytes(hd); }
int pc_aptx_samples_per_packet(bool hd, uint32_t rate, int mtu)
{
    int s = hd ? (rate == 48000 ? 432 : 396) : (rate == 48000 ? 672 : 660);
    int hdr = hd ? 12 : 0;
    while (s > 4 && pc_aptx_packet_bytes(hd, s) + hdr > mtu) s -= 4;
    return s;
}

/* ---------------- fixed-rate codec steps ---------------- */
int pc_btc_ladder(pc_btc_t c, unsigned sink, pc_btc_t out[PC_BTC_N])
{
    if (c < 0 || c >= PC_BTC_N) return 0;
    int n = 0;
    out[n++] = c;
    if (c == PC_BTC_APTX_HD && (sink & (1u << PC_BTC_APTX))) out[n++] = PC_BTC_APTX;
    if (c == PC_BTC_APTX_HD || c == PC_BTC_APTX) out[n++] = PC_BTC_SBC;
    return n;
}

int pc_btc_step_kbps(pc_btc_t c, uint32_t rate)
{
    switch (c) {
    case PC_BTC_APTX_HD: return (int)(rate * 12 / 1000);     /* 6 octets per 4 stereo samples */
    case PC_BTC_APTX: return (int)(rate * 8 / 1000);         /* 4 octets */
    case PC_BTC_SBC: {                                       /* joint stereo, 8 subbands, 16 blocks */
        int frame = 4 + (4 * 8 * 2) / 8 + (8 + 16 * PC_SBC_STEP_BITPOOL + 7) / 8;
        return (int)((uint64_t)frame * 8 * rate / 128 / 1000);
    }
    default: return 0;
    }
}

void pc_cstep_init(pc_cstep_t *s, int n)
{
    *s = (pc_cstep_t){ 0 };
    s->n = n;
    s->up_wait_ms = PC_CS_UP_MS;
}

static void cs_clear(pc_cstep_t *s) { memset(s->lost, 0, sizeof s->lost); }

int pc_cstep_update(pc_cstep_t *s, uint32_t now, int lost, bool wifi)
{
    if (s->n <= 1 || s->off) return s->level;
    uint32_t sec = now / 1000;
    if (!s->started) {
        s->started = true;
        s->sec = sec;
        s->quiet_since_ms = now;
        s->last_down_ms = now - PC_CS_MIN_GAP_MS;
        s->wifi = wifi;
    }
    for (uint32_t k = 0; k < PC_CS_WIN_S && s->sec != sec; k++) {     /* age the window */
        s->sec++;
        s->lost[s->sec % PC_CS_WIN_S] = 0;
    }
    s->sec = sec;
    if (s->wifi && !wifi) s->quiet_since_ms = now;                     /* Wi-Fi went off: the link changed */
    s->wifi = wifi;
    if (lost > 0) {
        uint32_t v = s->lost[sec % PC_CS_WIN_S] + (uint32_t)lost;
        s->lost[sec % PC_CS_WIN_S] = (uint16_t)(v > 0xFFFF ? 0xFFFF : v);
        s->quiet_since_ms = now;
    }
    if (s->ups && !s->held && now - s->last_up_ms >= PC_CS_UP_FAIL_MS) {   /* the last up-step held */
        s->held = true;
        s->up_wait_ms /= 2;
        if (s->up_wait_ms < PC_CS_UP_MS) s->up_wait_ms = PC_CS_UP_MS;
    }
    uint32_t sum = 0;
    int secs = 0;
    for (int i = 0; i < PC_CS_WIN_S; i++) { sum += s->lost[i]; secs += s->lost[i] > 0; }
    bool after_up = s->ups && !s->held;                                   /* the last switch was an up-step */
    if (sum >= PC_CS_DOWN_LOST && secs >= PC_CS_DOWN_SECS && s->level < s->n - 1 && (after_up || now - s->last_down_ms >= PC_CS_MIN_GAP_MS)) {
        if (after_up) {
            s->up_wait_ms *= 2;
            if (s->up_wait_ms > PC_CS_UP_MAX_MS) s->up_wait_ms = PC_CS_UP_MAX_MS;
        }
        s->level++;
        s->downs++;
        s->held = true;
        s->last_down_ms = now;
        s->quiet_since_ms = now;
        cs_clear(s);                                                      /* losses at the old rate */
        return s->level;
    }
    uint32_t wait = s->up_wait_ms;
    if (!wifi && wait > PC_CS_UP_NOWIFI_MS) wait = PC_CS_UP_NOWIFI_MS;
    if (s->level > 0 && now - s->quiet_since_ms >= wait) {
        s->level--;
        s->ups++;
        s->held = false;
        s->last_up_ms = now;
        s->quiet_since_ms = now;
        cs_clear(s);
    }
    return s->level;
}

void pc_cstep_failed(pc_cstep_t *s, int level_in_use)
{
    s->level = level_in_use;
    s->off = true;
}

/* ---------------- test tone ---------------- */
typedef struct { double a, b, d, energy, sig; } fit_t;
static fit_t fit(const float *x, int n, double w)
{
    /* least squares x ~ a cos + b sin + d: the 3x3 normal equations */
    double cc = 0, ss = 0, cs = 0, c1 = 0, s1 = 0, xc = 0, xs = 0, x1 = 0;
    double cr = cos(w), sr = sin(w), c = 1, s = 0;
    for (int i = 0; i < n; i++) {
        double v = x[i];
        cc += c * c; ss += s * s; cs += c * s; c1 += c; s1 += s;
        xc += v * c; xs += v * s; x1 += v;
        double c2 = c * cr - s * sr;
        s = s * cr + c * sr;
        c = c2;
        if ((i & 1023) == 1023) { double m = 1.0 / sqrt(c * c + s * s); c *= m; s *= m; }   /* keep |e^jw| = 1 */
    }
    double N = n;
    double m[3][4] = { { cc, cs, c1, xc }, { cs, ss, s1, xs }, { c1, s1, N, x1 } };
    for (int k = 0; k < 3; k++) {                                          /* Gauss-Jordan, the matrix is SPD */
        double piv = m[k][k];
        if (fabs(piv) < 1e-12) return (fit_t){ 0 };
        for (int j = k; j < 4; j++) m[k][j] /= piv;
        for (int r = 0; r < 3; r++) {
            if (r == k) continue;
            double f = m[r][k];
            for (int j = k; j < 4; j++) m[r][j] -= f * m[k][j];
        }
    }
    fit_t f = { m[0][3], m[1][3], m[2][3], 0, 0 };
    f.energy = f.a * xc + f.b * xs + f.d * x1;                             /* explained energy */
    f.sig = f.a * f.a * cc + f.b * f.b * ss + 2 * f.a * f.b * cs;          /* the sine alone */
    return f;
}

bool pc_tone_analyse(const float *x, int n, float rate, pc_tone_t *out)
{
    *out = (pc_tone_t){ 0, -200, 0 };
    if (n < 64) return false;
    double mean = 0, xx = 0;
    for (int i = 0; i < n; i++) mean += x[i];
    mean /= n;
    for (int i = 0; i < n; i++) xx += (x[i] - mean) * (x[i] - mean);
    if (xx <= 0) return false;
    /* zero crossings (upwards, interpolated) for a first estimate */
    double first = -1, last = -1;
    int cross = 0;
    for (int i = 1; i < n; i++) {
        double a = x[i - 1] - mean, b = x[i] - mean;
        if (a < 0 && b >= 0) {
            double t = i - 1 + a / (a - b);
            if (first < 0) first = t;
            last = t;
            cross++;
        }
    }
    if (cross < 2 || last <= first) return false;
    double f0 = (cross - 1) / (last - first) * rate;
    /* refine: the frequency with the most explained energy, coarse then fine */
    double best = f0, step = f0 * 0.002;
    for (int pass = 0; pass < 3; pass++) {
        double centre = best, e_best = -1;
        for (int k = -10; k <= 10; k++) {
            double f = centre + k * step;
            if (f <= 0 || f >= rate / 2) continue;
            fit_t r = fit(x, n, 2 * M_PI * f / rate);
            if (r.energy > e_best) { e_best = r.energy; best = f; }
        }
        step /= 10;
    }
    double w = 2 * M_PI * best / rate;
    fit_t r = fit(x, n, w);
    double resid = 0;                                                      /* everything the sine and DC leave */
    for (int i = 0; i < n; i++) {
        double e = x[i] - (r.a * cos(w * i) + r.b * sin(w * i) + r.d);
        resid += e * e;
    }
    if (resid < r.sig * 1e-14) resid = r.sig * 1e-14;
    out->freq_hz = (float)best;
    out->level_db = (float)(20 * log10(sqrt(r.a * r.a + r.b * r.b) + 1e-12));
    out->snr_db = (float)(10 * log10(r.sig / resid));
    return true;
}

uint32_t pc_bt_reconnect_ms(int attempt)
{
    if (attempt < 0) attempt = 0;
    if (attempt >= 5) return 60000;
    uint32_t ms = 3000u << attempt;
    return ms > 60000 ? 60000 : ms;
}
