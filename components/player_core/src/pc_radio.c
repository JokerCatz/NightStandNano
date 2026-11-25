// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_radio.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void trim_copy(char *out, size_t cap, const char *b, const char *e)
{
    while (b < e && isspace((unsigned char)*b)) b++;
    while (e > b && isspace((unsigned char)e[-1])) e--;
    size_t n = (size_t)(e - b);
    if (n >= cap) n = cap - 1;
    memcpy(out, b, n);
    out[n] = 0;
}

static bool is_url_at(const char *b, const char *e)
{
    while (b < e && isspace((unsigned char)*b)) b++;
    return (size_t)(e - b) >= 7 && (!strncasecmp(b, "http://", 7) || ((size_t)(e - b) >= 8 && !strncasecmp(b, "https://", 8)));
}

int pc_station_urls(const pc_station_t *s)
{
    int n = 1;
    for (int k = 0; k < PC_STATION_ALTS; k++) if (s->alt[k][0]) n++;
    return n;
}
const char *pc_station_url(const pc_station_t *s, int k)
{
    if (k == 0) return s->url;
    int at = 0;
    for (int i = 0; i < PC_STATION_ALTS; i++) if (s->alt[i][0] && ++at == k) return s->alt[i];
    return NULL;
}

bool pc_url_is_https(const char *u) { return u && !strncasecmp(u, "https://", 8); }
bool pc_url_is_web(const char *u) { return u && (!strncasecmp(u, "http://", 7) || !strncasecmp(u, "https://", 8)); }
int pc_station_pick(const pc_station_t *s, bool tls_ok)
{
    int first_tls = -1;
    for (int k = 0; k < pc_station_urls(s); k++) {
        const char *u = pc_station_url(s, k);
        if (u && !strncasecmp(u, "http://", 7)) return k;
        if (first_tls < 0 && pc_url_is_https(u)) first_tls = k;
    }
    return tls_ok ? first_tls : -1;
}
int pc_url_http_twin(const char *u, char *out, size_t cap)
{
    if (!pc_url_is_https(u)) return -1;
    const char *host = u + 8, *e = host;
    while (*e && *e != '/' && *e != '?' && *e != '#') e++;
    for (const char *c = host; c < e; c++) if (*c == '@') return -1;      /* user:password@host: never sent in the clear */
    if (strchr(u, '?')) return -1;                                          /* a query string is often a token: https only */
    const char *colon = NULL;                                              /* the port: the last ':' of the host part, after any IPv6 ']' */
    for (const char *c = host; c < e; c++) { if (*c == ']') colon = NULL; else if (*c == ':') colon = c; }
    const char *hend = colon ? colon : e;
    const bool keep = colon && !((e - colon) == 4 && !strncmp(colon, ":443", 4));
    const char *port = keep ? colon : e;                                   /* what follows the host: ":8443" + the rest, or just the rest */
    int n = snprintf(out, cap, "http://%.*s%s", (int)(hend - host), host, port);
    return n < 0 || (size_t)n >= cap ? -1 : n;
}

int pc_url_redact(const char *u, char *out, size_t cap)
{
    if (!cap) return -1;
    const char *p = strstr(u, "://");
    p = p ? p + 3 : u;
    const char *e = p;
    while (*e && *e != '/' && *e != '?' && *e != '#') e++;
    const char *at = NULL;
    for (const char *c = p; c < e; c++) if (*c == '@') at = c;
    const char *host = at ? at + 1 : p;
    const char *pe = e;
    while (*pe && *pe != '?' && *pe != '#') pe++;                          /* the path stays, the query and the fragment go */
    int n = snprintf(out, cap, "%.*s%.*s", (int)(p - u), u, (int)(pe - host), host);
    return n < 0 || (size_t)n >= cap ? -1 : n;
}

/* https station, http first: the twin is tried, then https. cache (kept per address by the caller, PSRAM): 0 unknown, 1 twin worked, 2 twin failed.
 * try(): 1 = the address gave audio, 0 = it did not, PC_TRY_STOP = it ended without a verdict (a new request, a finished list): never a failure, no https attempt.
 * An http address is just tried. A cached failure skips the twin; a twin that worked before and fails now clears the cache. */
int pc_https_play(uint8_t *cache, const char *url, char *used, size_t cap, pc_try_fn try, void *ctx)
{
    char twin[256];
    if (!pc_url_is_https(url) || pc_url_http_twin(url, twin, sizeof twin) < 0) { snprintf(used, cap, "%s", url); return try(ctx, used); }
    if (*cache != PC_TWIN_BAD) {
        snprintf(used, cap, "%s", twin);
        int r = try(ctx, used);
        if (r == PC_TRY_STOP) return PC_TRY_STOP;
        if (r) { *cache = PC_TWIN_OK; return 1; }
        *cache = *cache == PC_TWIN_OK ? PC_TWIN_UNKNOWN : PC_TWIN_BAD;     /* a later failure clears the good mark; a first failure remembers the bad one */
    }
    snprintf(used, cap, "%s", url);
    return try(ctx, used);
}

#define MAX_BARS 8
int pc_radio_parse(const char *text, pc_station_t *out, int max)
{
    int n = 0;
    const char *p = text;
    while (*p && n < max) {
        const char *e = strchr(p, '\n');
        if (!e) e = p + strlen(p);
        const char *bars[MAX_BARS];
        int nb = 0;
        for (const char *q = p; q < e; q++) if (*q == '|') { if (nb < MAX_BARS) bars[nb] = q; nb++; }
        const char *b = p;
        while (b < e && isspace((unsigned char)*b)) b++;
        if (nb && *b != '#') {
            if (nb > MAX_BARS) {                              /* too many bars: keep the first MAX_BARS - 1 and the last (the old rule's bar) */
                const char *last = NULL;
                for (const char *q = p; q < e; q++) if (*q == '|') last = q;
                bars[MAX_BARS - 1] = last;
                nb = MAX_BARS;
            }
            int first = nb - 1;                               /* the old rule: the address starts after the last bar */
            for (int k = 0; k < nb; k++) {
                const char *fe = k + 1 < nb ? bars[k + 1] : e;
                if (is_url_at(bars[k] + 1, fe)) { first = k; break; }
            }
            pc_station_t *s = &out[n];
            memset(s, 0, sizeof *s);
            trim_copy(s->name, sizeof s->name, p, bars[first]);
            int u = 0;
            for (int k = first; k < nb && u <= PC_STATION_ALTS; k++) {
                const char *fe = k + 1 < nb ? bars[k + 1] : e;
                char tmp[256];
                trim_copy(tmp, sizeof tmp, bars[k] + 1, fe);
                if (!tmp[0]) { if (k == first) break; continue; }
                if (u == 0) memcpy(s->url, tmp, sizeof s->url);
                else memcpy(s->alt[u - 1], tmp, sizeof tmp);
                u++;
            }
            if (s->url[0]) {
                if (!s->name[0]) trim_copy(s->name, sizeof s->name, s->url, s->url + strlen(s->url));
                n++;
            }
        }
        p = *e ? e + 1 : e;
    }
    return n;
}

int pc_radio_format(const pc_station_t *s, int n, char *out, size_t cap)
{
    size_t len = 0;
    if (cap) out[0] = 0;
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < pc_station_urls(&s[i]); k++) {
            int m = k ? snprintf(out + len, cap - len, "|%s", pc_station_url(&s[i], k)) : snprintf(out + len, cap - len, "%s|%s", s[i].name, s[i].url);
            if (m < 0 || (size_t)m >= cap - len) { if (cap) out[0] = 0; return -1; }
            len += (size_t)m;
        }
        if (len + 1 >= cap) { if (cap) out[0] = 0; return -1; }
        out[len++] = '\n';
        out[len] = 0;
    }
    return (int)len;
}

bool pc_playlist_first_url(const char *body, char *out, size_t cap)
{
    bool found = false;
    for (const char *p = body; *p;) {
        const char *e = strpbrk(p, "\r\n");
        if (!e) e = p + strlen(p);
        const char *v = p;
        if (!strncasecmp(v, "File", 4)) { const char *eq = memchr(v, '=', (size_t)(e - v)); if (eq) v = eq + 1; }
        if (!strncasecmp(v, "http://", 7)) { trim_copy(out, cap, v, e); return true; }
        if (!found && !strncasecmp(v, "https://", 8)) { trim_copy(out, cap, v, e); found = true; }   /* only if no http one */
        p = e;
        while (*p == '\r' || *p == '\n') p++;
    }
    return found;
}

int pc_radio_prebuffer(int br)
{
    if (br <= 0 || br > 1000) return 32768;
    int b = br * 1000 / 8 * 2;
    return b < 8192 ? 8192 : b > 98304 ? 98304 : b;
}

/* frame length at b, 0 when b is not a frame header */
static size_t adts_len(const unsigned char *b, size_t left)
{
    if (left < 7 || b[0] != 0xFF || (b[1] & 0xF6) != 0xF0 || ((b[2] >> 2) & 0xF) > 12) return 0;
    size_t len = ((size_t)(b[3] & 3) << 11) | ((size_t)b[4] << 3) | (b[5] >> 5);
    return len >= 7 ? len : 0;
}
static size_t mp3_len(const unsigned char *b, size_t left)
{
    static const unsigned short br1[15] = { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    static const unsigned short br2[15] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 };
    static const unsigned sr1[3] = { 44100, 48000, 32000 };
    if (left < 4 || b[0] != 0xFF || (b[1] & 0xE0) != 0xE0) return 0;
    int ver = (b[1] >> 3) & 3, layer = (b[1] >> 1) & 3, bi = b[2] >> 4, si = (b[2] >> 2) & 3, pad = (b[2] >> 1) & 1;
    if (ver == 1 || layer != 1 || bi == 0 || bi == 15 || si == 3) return 0;   /* Layer III only */
    unsigned sr = sr1[si] >> (ver == 3 ? 0 : ver == 2 ? 1 : 2);
    return ver == 3 ? 144000u * br1[bi] / sr + (unsigned)pad : 72000u * br2[bi] / sr + (unsigned)pad;
}
static bool chain(const unsigned char *b, size_t n, size_t at, size_t (*len)(const unsigned char *, size_t))
{
    for (int k = 0; k < 3; k++) {
        size_t l = at < n ? len(b + at, n - at) : 0;
        if (!l || at + l > n) return false;
        at += l;
    }
    return true;
}
pc_fmt_t pc_radio_sniff(const unsigned char *b, size_t n)
{
    for (size_t i = 0; i + 188 * 2 < n && i < 188; i++)  /* MPEG-TS: a sync byte every 188 */
        if (b[i] == 0x47 && b[i + 188] == 0x47 && b[i + 376] == 0x47) return PC_FMT_TS;
    for (size_t i = 0; i + 4 <= n; i++)                 /* Ogg pages carry a 4-byte capture pattern */
        if (b[i] == 'O' && !memcmp(b + i, "OggS", 4)) return PC_FMT_OGG;
    for (size_t i = 0; i + 4 < n; i++) {
        if (b[i] != 0xFF) continue;
        if (chain(b, n, i, adts_len)) return PC_FMT_AAC;
        if (chain(b, n, i, mp3_len)) return PC_FMT_MP3;
    }
    return PC_FMT_UNKNOWN;
}

static bool ends_with(const char *s, const char *suf)
{
    size_t a = strlen(s), b = strlen(suf);
    return a >= b && !strcasecmp(s + a - b, suf);
}

pc_fmt_t pc_radio_guess(const char *ct, const char *url)
{
    if (ct) {
        if (strcasestr(ct, "mpegurl") || strcasestr(ct, "scpls") || strcasestr(ct, "x-pls")) return PC_FMT_PLAYLIST;
        if (strcasestr(ct, "audio/mpeg") || strcasestr(ct, "audio/mp3")) return PC_FMT_MP3;
        if (strcasestr(ct, "aac")) return PC_FMT_AAC;
        if (strcasestr(ct, "ogg") || strcasestr(ct, "opus") || strcasestr(ct, "vorbis")) return PC_FMT_OGG;
    }
    if (url) {
        char u[256];
        snprintf(u, sizeof u, "%s", url);
        char *q = strpbrk(u, "?#");
        if (q) *q = 0;
        if (ends_with(u, ".pls") || ends_with(u, ".m3u") || ends_with(u, ".m3u8")) return PC_FMT_PLAYLIST;
        if (ends_with(u, ".mp3")) return PC_FMT_MP3;
        if (ends_with(u, ".aac") || ends_with(u, ".aacp")) return PC_FMT_AAC;
        if (ends_with(u, ".ogg") || ends_with(u, ".opus") || ends_with(u, ".oga")) return PC_FMT_OGG;
    }
    return PC_FMT_UNKNOWN;
}

int pc_radio_backoff_ms(int fails, int n, bool mid)
{
    if (mid || n < 1 || fails < 1) return 1000;
    int rounds = fails / n;
    if (rounds < 1) rounds = 1;
    int ms = 1000;
    for (int i = 1; i < rounds && ms < 30000; i++) ms *= 2;
    return ms > 30000 ? 30000 : ms;
}

pc_rdiag_t pc_radio_diag(const char *url, bool connected, int status, int os_errno, bool bad_format)
{
    (void)url;                                                 /* https is judged like http: the player refuses it by itself where memory is short */
    if (!connected) return os_errno == ECONNREFUSED ? PC_RDIAG_REFUSED : (os_errno == ETIMEDOUT || os_errno == EAGAIN || os_errno == EWOULDBLOCK) ? PC_RDIAG_TIMEOUT : PC_RDIAG_FAILED;
    if (status && status != 200) return PC_RDIAG_STATUS;
    return bad_format ? PC_RDIAG_UNSUPPORTED : PC_RDIAG_OK;
}

bool pc_radio_stem_ok(const char *s)
{
    if (!s) return false;
    size_t n = strlen(s);
    if (n >= PC_RADIO_STEM_MAX) return false;
    for (size_t i = 0; i < n; i++) if (!((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= '0' && s[i] <= '9') || s[i] == '_' || s[i] == '-')) return false;
    return true;
}

int pc_radio_list_path(const char *stem, char *out, size_t cap)
{
    if (!pc_radio_stem_ok(stem)) return -1;
    int n = stem[0] ? snprintf(out, cap, PC_RADIO_DIR "/radio_%s.txt", stem) : snprintf(out, cap, PC_RADIO_DIR "/radio.txt");
    return n < 0 || (size_t)n >= cap ? -1 : n;
}

const char *pc_radio_default_stem(int lang) { return lang == 1 ? "cn" : ""; }

static int stem_cmp(const void *a, const void *b) { return strcmp((const char *)a, (const char *)b); }
int pc_radio_scan_lists(const char *dir, char (*stems)[PC_RADIO_STEM_MAX], int max)
{
    int n = 0;
    DIR *d = opendir(dir);
    if (!d) return 0;
    struct dirent *e;
    bool main_list = false;
    while ((e = readdir(d))) {                                  /* to the end: radio.txt may come after the slots are full */
        const char *f = e->d_name;
        size_t len = strlen(f);
        if (!strcmp(f, "radio.txt")) { main_list = true; continue; }
        if (len > 10 && !strncmp(f, "radio_", 6) && !strcmp(f + len - 4, ".txt")) {
            char stem[PC_RADIO_STEM_MAX];
            size_t k = len - 10;
            if (k >= sizeof stem) continue;
            memcpy(stem, f + 6, k);
            stem[k] = 0;
            if (pc_radio_stem_ok(stem) && stem[0] && n < max) memcpy(stems[n++], stem, sizeof stem);
        }
    }
    closedir(d);
    if (n > 1) qsort(stems, (size_t)n, PC_RADIO_STEM_MAX, stem_cmp);
    if (main_list && max > 0) {                                 /* the main list first */
        if (n == max) n--;
        memmove(stems + 1, stems, (size_t)n * PC_RADIO_STEM_MAX);
        stems[0][0] = 0;
        n++;
    }
    return n;
}
