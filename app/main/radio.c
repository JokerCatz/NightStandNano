// SPDX-License-Identifier: LGPL-2.1-or-later
#include "radio.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pc/pc_stream.h"
#include "pc/pc_ring.h"
#include "pc/pc_icy.h"
#include "pc/pc_radio.h"
#include "pc/pc_hls.h"
#include "pc/pc_ts.h"
#include "esp_timer.h"
#include "backend_sdec.h"

static const char *TAG = "radio";
#define LIST_PATH "/sdcard/_player/config/radio.txt"
#define MAX_ST 40
static pc_station_t *s_list; static int s_list_n;
static char s_alt[1 + PC_STATION_ALTS][256]; static int s_alt_n;   /* the addresses of the station being played, main first */

int nsn_radio_load(void)
{
    if (!s_list) s_list = heap_caps_malloc(sizeof(pc_station_t) * MAX_ST, MALLOC_CAP_SPIRAM);
    s_list_n = 0;
    if (!s_list) return 0;
    FILE *f = fopen(LIST_PATH, "rb");
    if (!f) return 0;
    char *txt = heap_caps_malloc(16384, MALLOC_CAP_SPIRAM);
    if (!txt) { fclose(f); return 0; }
    size_t n = fread(txt, 1, 16383, f); fclose(f); txt[n] = 0;
    s_list_n = pc_radio_parse(txt, s_list, MAX_ST);
    free(txt);
    return s_list_n;
}
const char *nsn_radio_station(int i, const char **name)
{
    if (i < 0 || i >= s_list_n) return NULL;
    if (name) *name = s_list[i].name;
    return s_list[i].url;
}
#define RING_SIZE (256 * 1024)
#define UA "NightStandNano/0.1"
#define BODY_MAX (128 * 1024)          /* a playlist text (an ended list of hours is ~100 KB) */
#define TLS_MIN_LARGEST (24 * 1024)    /* largest internal block needed to open a TLS connection (JukeDial's rule): Auracast + radio sits near it */

/* Everything the network task needs besides its stack, in PSRAM (allocated once with the ring): no 4 KB read buffer, no playlist text, no segment table in
 * internal RAM. */
typedef struct {
    uint8_t buf[4096];
    uint8_t es[4608];                  /* one TS feed of up to 4096 bytes yields at most 23 packets of 184 payload bytes */
    pc_hls_t h;
    char list[PC_HLS_URL], base[PC_HLS_URL], seg_url[PC_HLS_URL];
    char body[BODY_MAX];
} work_t;

static pc_ring_t s_ring;
static uint8_t *s_mem;
static work_t *s_w;
static pc_stream_t s_st;
static volatile bool s_run, s_task_done = true;
static char s_url[300], s_title[160], s_state[40];
static pc_icy_t s_icy;
static struct { int metaint, br; char ctype[64]; } s_hdr;
static volatile bool s_moved;                       /* the session put audio bytes into the ring */
static pc_ts_t s_ts;                                /* HLS: the TS demuxer, kept across segments */
static uint64_t s_hls_last;                         /* HLS: the last segment played; a reconnect goes on after it instead of replaying the window */
static int s_hls_fmt;                               /* HLS: the decoder chosen (PC_FMT_AAC / PC_FMT_MP3), 0 = not yet */
static volatile uint32_t s_conns;                   /* connections opened (TCP + TLS for https) since boot, for the reuse measurement */
static volatile bool s_noreuse;                     /* debug: close the connection after every request (measures what keep-alive saves) */

const char *nsn_radio_title(void) { return s_title; }
const char *nsn_radio_state(void) { return s_state; }
static void set_state(const char *s) { if (strcmp(s, s_state)) ESP_LOGI(TAG, "state: \"%s\"", s); snprintf(s_state, sizeof s_state, "%s", s); }
void nsn_radio_debug_reuse(int on) { s_noreuse = !on; }
unsigned nsn_radio_connections(void) { return s_conns; }

static esp_err_t on_http(esp_http_client_event_t *e)
{
    if (e->event_id == HTTP_EVENT_ON_CONNECTED) s_conns++;
    if (e->event_id == HTTP_EVENT_ON_HEADER && e->header_key && e->header_value) {
        if (!strcasecmp(e->header_key, "icy-metaint")) s_hdr.metaint = atoi(e->header_value);
        else if (!strcasecmp(e->header_key, "icy-br")) s_hdr.br = atoi(e->header_value);
        else if (!strcasecmp(e->header_key, "Content-Type")) snprintf(s_hdr.ctype, sizeof s_hdr.ctype, "%s", e->header_value);
    }
    return ESP_OK;
}

static bool nap(int ms) { for (; ms > 0 && s_run; ms -= 100) vTaskDelay(pdMS_TO_TICKS(100)); return s_run; }
static bool tls_ok(void) { return heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) >= TLS_MIN_LARGEST; }
/* a refusal that ends the session for good (the state text stays until the next station): the format is not something retrying fixes */
static int refuse(const char *why) { ESP_LOGW(TAG, "refused: %s", why); set_state(why); s_run = false; return 1; }

/* ---- HLS (JukeDial's flow, radio_svc.c hls_session; departures are listed in docs/design.md) ----
 * One esp_http_client handle serves the playlists and every segment: esp_http_client_set_url keeps the connection while host and port stay the same
 * and the server allows keep-alive, so an https stream pays one TLS handshake, not one per segment. Reads use a 1 s timeout so a cancel is seen
 * within a second; a connect attempt is 3 s (3 attempts), the header wait 6 x 1 s. */

/* GET url on the shared handle; redirects are followed (the final address is written back into `url`). Returns the status or -1; *clen = Content-Length (<= 0 unknown) */
static int h_get(esp_http_client_handle_t cl, char *url, size_t cap, int64_t *clen)
{
    for (int hop = 0, retry = 0; hop < 5 && s_run;) {
        if (esp_http_client_set_url(cl, url) != ESP_OK) return -1;
        esp_err_t e = ESP_FAIL;
        for (int a = 0; a < 3 && s_run && e != ESP_OK; a++) {      /* 3 x 3 s instead of one 9 s wait: a cancel is seen within 3 s even for an address that never answers */
            esp_http_client_set_timeout_ms(cl, 3000);
            e = esp_http_client_open(cl, 0);
            if (e != ESP_OK) esp_http_client_close(cl);
        }
        if (e != ESP_OK) return -1;
        int64_t len = -1;
        bool eagain = false;
        if (e == ESP_OK) {
            esp_http_client_set_timeout_ms(cl, 1000);
            for (int w = 0; w < 6 && s_run; w++) { len = esp_http_client_fetch_headers(cl); eagain = len == -ESP_ERR_HTTP_EAGAIN; if (!eagain) break; }
        }
        if (e != ESP_OK || len < 0) {
            esp_http_client_close(cl);
            if (!eagain && retry++ == 0 && s_run) continue;      /* a kept-alive connection the server had closed: once more on a fresh one */
            return -1;
        }
        if (clen) *clen = len;
        int st = esp_http_client_get_status_code(cl);
        if (st >= 300 && st < 400) {
            int dummy;
            esp_http_client_set_redirection(cl);
            esp_http_client_get_url(cl, url, (int)cap);
            if (esp_http_client_flush_response(cl, &dummy) != ESP_OK) esp_http_client_close(cl);
            hop++;
            continue;
        }
        return st;
    }
    return -1;
}

/* one read of the body; the 1 s timeout is retried up to 8 times (a stalled server), checking for a cancel each time. >0 bytes, 0 end of body, <0 failed / cancelled */
static int h_read(esp_http_client_handle_t cl, uint8_t *b, int n)
{
    for (int stall = 0; s_run; ) {
        int k = esp_http_client_read(cl, (char *)b, n);
        if (k == -ESP_ERR_HTTP_EAGAIN) { if (++stall >= 8) return -1; continue; }
        return k;
    }
    return -1;
}

/* the whole (small) body into w->body after `have` bytes already there; text length, -1 failed. The last half line of a cut-off list is dropped. */
static int h_body(esp_http_client_handle_t cl, int have)
{
    char *body = s_w->body;
    int n = have, k = 1;
    while (n < BODY_MAX - 1 && (k = h_read(cl, (uint8_t *)body + n, BODY_MAX - 1 - n)) > 0) n += k;
    if (k < 0) { esp_http_client_close(cl); return -1; }
    body[n] = 0;
    if (n == BODY_MAX - 1) { char *nl = strrchr(body, '\n'); if (nl) nl[1] = 0; esp_http_client_close(cl); }
    return (int)strlen(body);
}

/* fetch a playlist: status 200 -> text in w->body; `final` = the address it was served from (the base for relative addresses) */
static int h_text(esp_http_client_handle_t cl, const char *url, char *final, size_t cap)
{
    snprintf(final, cap, "%s", url);
    int64_t clen;
    int st = h_get(cl, final, cap, &clen);
    if (st != 200) { if (st > 0) esp_http_client_close(cl); return -1; }
    int n = h_body(cl, 0);
    if (s_noreuse) esp_http_client_close(cl);
    return n;
}

static void ring_put(const uint8_t *p, int m)
{
    for (int off = 0; off < m && s_run;) {                  /* full ring: the decoder is behind, wait */
        off += (int)pc_ring_write(&s_ring, p + off, (uint32_t)(m - off));
        if (off < m) vTaskDelay(pdMS_TO_TICKS(20));
    }
    s_moved = true;
    if (!s_state[0]) return;
    if (pc_ring_used(&s_ring) >= s_st.prebuffer) set_state("");
}

enum { SEG_OK, SEG_CANCEL, SEG_NET, SEG_REFUSED, SEG_GONE };

/* one segment: TS (-> the AAC / MP3 inside) or packed audio (ADTS AAC / MP3 behind an optional ID3 tag). *bytes = the segment's size */
static int fetch_seg(esp_http_client_handle_t cl, const char *seg_url, uint32_t *bytes)
{
    work_t *w = s_w;
    char *url = w->seg_url;
    snprintf(url, PC_HLS_URL, "%s", seg_url);
    int64_t clen = 0;
    int st = h_get(cl, url, PC_HLS_URL, &clen);
    if (st < 0) { ESP_LOGW(TAG, "segment request failed"); return s_run ? SEG_NET : SEG_CANCEL; }
    if (st != 200) { esp_http_client_close(cl); { char r[200]; pc_url_redact(url, r, sizeof r); ESP_LOGW(TAG, "segment status %d %s", st, r); } return st == 404 || st == 410 ? SEG_GONE : SEG_NET; }
    int kind = -1;                                           /* 0 TS, 1 ADTS, 2 MP3 */
    uint32_t skip = 0, total = 0, es_total = 0;
    bool first = true;
    for (;;) {
        int n = 0;
        while (n < (int)sizeof w->buf && (first || n == 0)) {   /* the first chunk is filled up: the format sniff needs a few frames */
            int k = h_read(cl, w->buf + n, (int)sizeof w->buf - n);
            if (k < 0) { n = -1; break; }
            if (k == 0) break;
            n += k;
            if (!first) break;
        }
        if (n < 0) { ESP_LOGW(TAG, "segment read failed after %u bytes", (unsigned)total); esp_http_client_close(cl); return s_run ? SEG_NET : SEG_CANCEL; }
        if (n == 0) break;
        total += (uint32_t)n;
        const uint8_t *p = w->buf;
        int m = n;
        if (first) {
            first = false;
            if (m >= 10 && !memcmp(p, "ID3", 3)) {             /* packed audio starts with a timestamp tag */
                skip = 10 + ((uint32_t)(p[6] & 0x7F) << 21 | (uint32_t)(p[7] & 0x7F) << 14 | (uint32_t)(p[8] & 0x7F) << 7 | (p[9] & 0x7F));
                if (p[5] & 0x10) skip += 10;
            }
            uint32_t k = skip < (uint32_t)m ? skip : (uint32_t)m;
            p += k; m -= (int)k; skip -= k;
            if (skip) { esp_http_client_close(cl); refuse("HLS: unsupported segment"); return SEG_REFUSED; }
            if (m >= 189 && p[0] == 0x47 && p[188] == 0x47) kind = 0;
            else if (m >= 12 && (!memcmp(p + 4, "ftyp", 4) || !memcmp(p + 4, "styp", 4) || !memcmp(p + 4, "moof", 4) || !memcmp(p + 4, "moov", 4) || !memcmp(p + 4, "mdat", 4)))
                { esp_http_client_close(cl); refuse("HLS: fMP4 not supported"); return SEG_REFUSED; }
            else if (m >= 2 && p[0] == 0x0B && p[1] == 0x77) { esp_http_client_close(cl); refuse("HLS: AC-3 not supported"); return SEG_REFUSED; }
            else {
                pc_fmt_t f = pc_radio_sniff(p, (size_t)m);
                kind = f == PC_FMT_AAC ? 1 : f == PC_FMT_MP3 ? 2 : -1;
                if (kind < 0) { esp_http_client_close(cl); refuse("HLS: unsupported segment"); return SEG_REFUSED; }
            }
        } else if (skip) {
            uint32_t k = skip < (uint32_t)m ? skip : (uint32_t)m;
            p += k; m -= (int)k; skip -= k;
        }
        const uint8_t *o = p;
        int om = m;
        if (kind == 0) { om = (int)pc_ts_feed(&s_ts, p, (size_t)m, w->es, sizeof w->es); o = w->es; }
        if (om <= 0) continue;
        int fmt = kind == 1 || (kind == 0 && s_ts.stream_type == 0x0F) ? PC_FMT_AAC : PC_FMT_MP3;
        if (!s_hls_fmt) {                                    /* the first audio of the session picks the decoder, before the ring holds anything */
            s_hls_fmt = fmt;
            s_st.inner = fmt == PC_FMT_AAC ? &backend_aac : &pc_backend_mp3;
            ESP_LOGI(TAG, "stream is %s (%s)", s_st.inner->name, kind == 0 ? "TS" : "packed");
        } else if (fmt != s_hls_fmt) { esp_http_client_close(cl); refuse("HLS: format changed"); return SEG_REFUSED; }
        es_total += (uint32_t)om;
        ring_put(o, om);
    }
    if (kind == 0 && !es_total) {                            /* a TS whose audio is not AAC / MP3 (AC-3 0x81, E-AC-3 0x87, ...) */
        refuse(s_ts.pmt_pid >= 0 && s_ts.audio_pid < 0 ? "HLS: unsupported audio" : "HLS: no audio in segment");
        return SEG_REFUSED;
    }
    if (s_noreuse) esp_http_client_close(cl);
    *bytes = total;
    return SEG_OK;
}

/* The playlist loop. `cl` is the handle that fetched the first playlist (its text is in w->body); it is cleaned up here. 1 = ended / stopped / refused, 0 = reconnect */
static int hls_run(esp_http_client_handle_t cl, const char *url0)
{
    work_t *w = s_w;
    char *list = w->list, *base = w->base;                   /* the playlist address (reloaded as it is) and the address it was served from (the base for relative addresses) */
    uint64_t last = s_hls_last;
    int idle = 0, hops = 0, ret = 0;
    uint32_t est = 64 * 1024;                                /* the size of the last segment: wait for that much room in the ring before asking for the next */
    bool vod = false, have = true;
    pc_ts_init(&s_ts);
    esp_http_client_delete_header(cl, "Icy-MetaData");
    snprintf(list, PC_HLS_URL, "%s", url0);
    snprintf(base, PC_HLS_URL, "%s", url0);
    set_state(s_moved ? "" : "buffering");
    for (;;) {
        if (!s_run) { ret = 1; break; }
        if (!have) {
            if (h_text(cl, list, base, PC_HLS_URL) < 0) { ESP_LOGW(TAG, "playlist request failed"); if (s_run) set_state("reconnecting"); ret = s_run ? 0 : 1; break; }
        }
        have = false;
        int rc = pc_hls_parse_from(w->body, base, 200000, vod ? (last == UINT64_MAX ? 0 : last + 1) : UINT64_MAX, &w->h);
        pc_hls_t *h = &w->h;
        if (rc && h->master && h->no_audio) { ret = refuse("HLS: unsupported audio"); break; }
        if (rc) { { char dbg[400]; snprintf(dbg, sizeof dbg, "%.390s", w->body); for (char *q = dbg; *q; q++) if (*q == '\n' || *q == '\r') *q = '|';
              ESP_LOGW(TAG, "hls: cannot read the list (%d bytes, master %d, variants: no_audio %d): %s", (int)strlen(w->body), h->master, h->no_audio, dbg); } set_state("HLS: bad playlist"); ret = 0; break; }
        if (h->master) {
            if (++hops > 3) { set_state("HLS: bad playlist"); ret = 0; break; }
            snprintf(list, PC_HLS_URL, "%s", h->variant);
            s_st.prebuffer = (uint32_t)pc_radio_prebuffer((int)(h->variant_bps / 1000));
            { char r[200]; pc_url_redact(list, r, sizeof r); ESP_LOGI(TAG, "variant %u bps: %s", (unsigned)h->variant_bps, r); }
            continue;
        }
        if (h->encrypted) { ret = refuse("HLS: encrypted"); break; }
        if (h->fmp4) { ret = refuse("HLS: fMP4 not supported"); break; }
        if (h->ended && !vod && last == UINT64_MAX) {         /* an ended list: from its first segment, window by window (the parser keeps 16 at a time) */
            vod = true;
            ESP_LOGI(TAG, "ended list: playing from the start");
            have = true;                                     /* parse the same text again from the first segment */
            continue;
        }
        int i = vod ? 0 : pc_hls_next(h, last);
        if (i < 0) {                                         /* caught up: the list grows every few seconds */
            if (h->ended) { ret = 1; ESP_LOGI(TAG, "list ended"); s_st.eof = true; break; }
            if (++idle > 6) { set_state("reconnecting"); ESP_LOGW(TAG, "hls: the list stopped moving"); ret = 0; break; }   /* ~3 target lengths */
            if (!nap(h->target_s * 500)) { ret = 1; break; }
            continue;
        }
        idle = 0;
        bool reload = false;
        for (; i < h->nseg && !reload; i++) {
            if (!s_run) break;
            if (h->title[i][0] && strcmp(h->title[i], s_title)) { snprintf(s_title, sizeof s_title, "%s", h->title[i]); ESP_LOGI(TAG, "title: %s", s_title); }
            uint32_t want = est + 8192;
            if (want > RING_SIZE - 16384) want = RING_SIZE - 16384;
            while (s_run && pc_ring_free(&s_ring) < want) vTaskDelay(pdMS_TO_TICKS(100));
            if (!s_run) break;
            if (pc_url_is_https(h->seg[i]) && !esp_http_client_is_persistent_connection(cl) && !tls_ok()) { ret = refuse("https: low memory"); goto out; }
            int64_t t0 = esp_timer_get_time();
            uint32_t got = 0;
            int r = fetch_seg(cl, h->seg[i], &got);
            if (r == SEG_CANCEL) { ret = 1; goto out; }
            if (r == SEG_REFUSED) { ret = 1; goto out; }
            if (r == SEG_NET) { if (s_run) set_state("reconnecting"); ret = s_run ? 0 : 1; goto out; }
            last = h->seg_seq[i];
            s_hls_last = last;
            if (r == SEG_GONE) { reload = true; continue; }  /* fell out of the window: skip it, read the list again */
            est = got;
            ESP_LOGI(TAG, "seg %llu %u B in %d ms (connections %u, ring %u)", (unsigned long long)last, (unsigned)got,
                     (int)((esp_timer_get_time() - t0) / 1000), (unsigned)s_conns, (unsigned)pc_ring_used(&s_ring));
        }
        if (vod) {                                           /* the window is played: more of the same text, or the end */
            if (!h->more && last != UINT64_MAX && s_run) { ESP_LOGI(TAG, "list ended"); s_st.eof = true; ret = 1; break; }
            have = true;
        }
    }
out:
    esp_http_client_cleanup(cl);
    return ret;
}

/* The text of a playlist (the response is open on `cl`, `head` bytes of it were already read into w->body): plain .m3u / .pls -> the first stream's address into
 * `url` and -100; HLS -> played here (takes over `cl`). */
static int take_playlist(esp_http_client_handle_t cl, char *url, int head)
{
    int n = h_body(cl, head);
    if (n < 0) { esp_http_client_cleanup(cl); if (s_run) set_state("reconnecting"); return s_run ? 0 : 1; }
    if (pc_hls_is_hls(s_w->body)) return hls_run(cl, url);
    esp_http_client_cleanup(cl);
    char next[300];
    if (!pc_playlist_first_url(s_w->body, next, sizeof next)) { set_state("empty playlist"); return 0; }
    snprintf(url, 300, "%s", next);
    return -100;
}

/* One connection. Returns 1 when stopped on purpose, 0 to reconnect. */
static int session(char *url)
{
    uint8_t *buf = s_w->buf;
    for (int hop = 0; hop < 6 && s_run; hop++) {
        memset(&s_hdr, 0, sizeof s_hdr);
        if (pc_url_is_https(url) && !tls_ok()) { set_state("https: low memory"); return 0; }
        esp_http_client_config_t c = { .url = url, .user_agent = UA, .timeout_ms = 8000, .buffer_size = 4096, .buffer_size_tx = 1024, .event_handler = on_http,
                                       .disable_auto_redirect = true, .crt_bundle_attach = esp_crt_bundle_attach, .keep_alive_enable = true };
        esp_http_client_handle_t cl = esp_http_client_init(&c);
        if (!cl) return 0;
        esp_http_client_set_header(cl, "Icy-MetaData", "1");
        esp_http_client_set_header(cl, "Accept", "*/*");
        int64_t clen;
        char cur[300];
        snprintf(cur, sizeof cur, "%s", url);
        int status = h_get(cl, cur, sizeof cur, &clen);       /* redirects followed; connect attempts and the header wait check for a cancel */
        esp_http_client_set_timeout_ms(cl, 8000);              /* the stream loop below: a stalled server is a reconnect after 8 s */
        if (status < 0) { esp_http_client_cleanup(cl); if (s_run) set_state("no connection"); return s_run ? 0 : 1; }
        if (strcmp(cur, url)) snprintf(url, 300, "%s", cur);     /* the address the answer came from decides .m3u8 / playlist handling */
        if (status != 200) { ESP_LOGW(TAG, "status %d", status); esp_http_client_cleanup(cl); set_state("server refused"); return 0; }
        pc_fmt_t fmt = pc_radio_guess(s_hdr.ctype, url);
        if (fmt == PC_FMT_PLAYLIST) {                      /* .pls / .m3u: the first stream in it; .m3u8 / HLS: played by hls_run */
            int r = take_playlist(cl, url, 0);
            if (r == -100) continue;
            return r;
        }
        bool first = true;
        pc_icy_init(&s_icy, s_hdr.metaint > 0 ? (uint32_t)s_hdr.metaint : 0);
        s_st.prebuffer = (uint32_t)pc_radio_prebuffer(s_hdr.br);
        set_state("buffering");
        while (s_run) {
            int n = esp_http_client_read(cl, (char *)buf, 4096);
            if (n <= 0) break;                              /* closed or timed out */
            if (first) {                                    /* servers mislabel AAC as audio/mpeg: trust the bytes over the header */
                first = false;
                if (n >= 7 && !memcmp(buf, "#EXTM3U", 7)) { /* a playlist the headers did not announce (HLS served as text/plain or octet-stream) */
                    memcpy(s_w->body, buf, (size_t)n);
                    int r = take_playlist(cl, url, n);
                    if (r == -100) goto next_hop;
                    return r;
                }
                pc_fmt_t sn = pc_radio_sniff(buf, (size_t)n);
                if (sn == PC_FMT_AAC || sn == PC_FMT_OGG || sn == PC_FMT_MP3) fmt = sn;
                if (sn == PC_FMT_TS) { esp_http_client_cleanup(cl); return refuse("HLS: bare TS stream not supported"); }
                s_st.inner = fmt == PC_FMT_AAC ? &backend_aac : fmt == PC_FMT_OGG ? &backend_ogg : &pc_backend_mp3;
                ESP_LOGI(TAG, "stream is %s", s_st.inner->name);
            }
            size_t a = pc_icy_feed(&s_icy, buf, (size_t)n);
            char t[160];
            if (pc_icy_take_title(&s_icy, t, sizeof t)) { snprintf(s_title, sizeof s_title, "%s", t); ESP_LOGI(TAG, "title: %s", t); }
            for (size_t off = 0; off < a && s_run;) {       /* full ring: the decoder is behind, wait */
                off += pc_ring_write(&s_ring, buf + off, (uint32_t)(a - off));
                if (off < a) vTaskDelay(pdMS_TO_TICKS(20));
            }
            s_moved = true;
            if (pc_ring_used(&s_ring) >= s_st.prebuffer) set_state("");
        }
        esp_http_client_cleanup(cl);
        return s_run ? 0 : 1;
    next_hop:;
    }
    set_state("too many redirects");
    return 0;
}

static void net_task(void *arg)
{
    int fails = 0;
    while (s_run) {
        char url[300]; snprintf(url, sizeof url, "%s", s_url);
        if (s_alt_n > 1 && fails > 0) {                                /* a failed round: next backup address */
            snprintf(url, sizeof url, "%s", s_alt[fails % s_alt_n]);
            ESP_LOGW(TAG, "trying backup %d", fails % s_alt_n);
        }
        s_moved = false;
        int r = session(url);
        if (r) break;
        fails = s_moved ? 1 : fails + 1;                              /* a session that played is not a failed round: the backoff starts over */
        if (!s_run) break;
        set_state("reconnecting");
        int wait_s = fails > 5 ? 15 : fails * 2 + 1;
        for (int i = 0; i < wait_s * 10 && s_run; i++) vTaskDelay(pdMS_TO_TICKS(100));
    }
    s_st.eof = true;
    s_task_done = true;
    vTaskDelete(NULL);
}

bool nsn_radio_begin(const char *url, const pc_backend_t **be, pc_reader_t *r)
{
    nsn_radio_end();
    if (!s_mem) s_mem = heap_caps_malloc(RING_SIZE, MALLOC_CAP_SPIRAM);
    if (!s_w) s_w = heap_caps_malloc(sizeof(work_t), MALLOC_CAP_SPIRAM);
    if (!s_mem || !s_w) return false;
    pc_ring_init(&s_ring, s_mem, RING_SIZE);
    s_st = (pc_stream_t){ .ring = &s_ring, .inner = &pc_backend_mp3, .prebuffer = 32768 };
    snprintf(s_url, sizeof s_url, "%s", url); s_title[0] = 0;
    s_hls_last = UINT64_MAX; s_hls_fmt = 0; s_moved = false;
    s_alt_n = 0;
    if (!s_list_n) nsn_radio_load();
    for (int i = 0; i < s_list_n && !s_alt_n; i++)
        for (int k = 0; k < pc_station_urls(&s_list[i]); k++)
            if (!strcmp(pc_station_url(&s_list[i], k), url)) {
                for (int j = 0; j < pc_station_urls(&s_list[i]); j++) snprintf(s_alt[s_alt_n++], 256, "%s", pc_station_url(&s_list[i], j));
                break;
            }
    set_state("connecting");
    s_run = true; s_task_done = false;
    if (xTaskCreatePinnedToCore(net_task, "radio", 8192, NULL, 5, NULL, 0) != pdPASS) { s_run = false; s_task_done = true; return false; }
    for (int i = 0; i < 300 && s_run; i++) {               /* up to 30 s for the first prebuffer */
        if (pc_ring_used(&s_ring) >= s_st.prebuffer) { *be = &pc_backend_stream; *r = (pc_reader_t){ .u = &s_st }; return true; }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ESP_LOGE(TAG, "no stream: %s", s_state);
    char why[sizeof s_state];
    snprintf(why, sizeof why, "%s", s_state);
    nsn_radio_end();
    if (strcmp(why, "connecting") && strcmp(why, "buffering") && strcmp(why, "reconnecting")) set_state(why);   /* the reason stays readable (st rstate) until the next source */
    return false;
}

void nsn_radio_cancel(void) { s_run = false; }

void nsn_radio_end(void)
{
    s_run = false;
    for (int i = 0; i < 200 && !s_task_done; i++) vTaskDelay(pdMS_TO_TICKS(50));
    s_state[0] = 0;
}

/* Favourites are the card's station list: these two are the only way a Bluetooth client changes a card file, so they are narrow on purpose
 * (one fixed file, http(s) addresses only, printable text, bounded sizes and count). */
static int save_list(void)
{
    char *txt = heap_caps_malloc(40960, MALLOC_CAP_SPIRAM);
    if (!txt) return -1;
    int n = pc_radio_format(s_list, s_list_n, txt, 40960), rc = -1;
    if (n >= 0) {
        mkdir("/sdcard/_player", 0777); mkdir("/sdcard/_player/config", 0777);
        FILE *f = fopen(LIST_PATH ".tmp", "wb");
        if (f) {
            bool ok = fwrite(txt, 1, (size_t)n, f) == (size_t)n;
            if (fclose(f) == 0 && ok) { remove(LIST_PATH); rc = rename(LIST_PATH ".tmp", LIST_PATH); }
        }
    }
    free(txt);
    return rc;
}
static bool clean(const char *s, size_t max)
{
    size_t n = strlen(s);
    if (!n || n >= max) return false;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) if (*p < 32 || *p == 127) return false;
    return true;
}
int nsn_radio_add(const char *name, const char *url)
{
    if (!clean(name, sizeof s_list[0].name) || !clean(url, sizeof s_list[0].url) || strchr(name, '|') || strchr(url, '|')) return -1;
    if (strncmp(url, "http://", 7) && strncmp(url, "https://", 8)) return -1;
    nsn_radio_load();
    if (!s_list) return -1;
    for (int i = 0; i < s_list_n; i++) if (!strcmp(s_list[i].url, url)) return i + 1;      /* already there */
    if (s_list_n >= MAX_ST) return -2;
    pc_station_t *st = &s_list[s_list_n];
    memset(st, 0, sizeof *st);
    snprintf(st->name, sizeof st->name, "%s", name); snprintf(st->url, sizeof st->url, "%s", url);
    s_list_n++;
    if (save_list() != 0) { s_list_n--; return -3; }
    return s_list_n;
}
int nsn_radio_del(int i)
{
    nsn_radio_load();
    if (!s_list || i < 0 || i >= s_list_n) return -1;
    pc_station_t keep = s_list[i];
    memmove(&s_list[i], &s_list[i + 1], sizeof(pc_station_t) * (size_t)(s_list_n - i - 1));
    s_list_n--;
    if (save_list() != 0) { memmove(&s_list[i + 1], &s_list[i], sizeof(pc_station_t) * (size_t)(s_list_n - i)); s_list[i] = keep; s_list_n++; return -3; }
    return 0;
}
