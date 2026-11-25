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

static pc_ring_t s_ring;
static uint8_t *s_mem;
static pc_stream_t s_st;
static volatile bool s_run, s_task_done = true;
static char s_url[300], s_title[160], s_state[40];
static pc_icy_t s_icy;
static struct { int metaint, br; char ctype[64]; } s_hdr;

const char *nsn_radio_title(void) { return s_title; }
const char *nsn_radio_state(void) { return s_state; }
static void set_state(const char *s) { snprintf(s_state, sizeof s_state, "%s", s); }

static esp_err_t on_http(esp_http_client_event_t *e)
{
    if (e->event_id == HTTP_EVENT_ON_HEADER && e->header_key && e->header_value) {
        if (!strcasecmp(e->header_key, "icy-metaint")) s_hdr.metaint = atoi(e->header_value);
        else if (!strcasecmp(e->header_key, "icy-br")) s_hdr.br = atoi(e->header_value);
        else if (!strcasecmp(e->header_key, "Content-Type")) snprintf(s_hdr.ctype, sizeof s_hdr.ctype, "%s", e->header_value);
    }
    return ESP_OK;
}

/* One connection. Returns 1 when stopped on purpose, 0 to reconnect. */
static int session(char *url)
{
    static uint8_t buf[4096];
    for (int hop = 0; hop < 4 && s_run; hop++) {
        memset(&s_hdr, 0, sizeof s_hdr);
        esp_http_client_config_t c = { .url = url, .user_agent = UA, .timeout_ms = 8000, .buffer_size = 4096, .event_handler = on_http,
                                       .crt_bundle_attach = esp_crt_bundle_attach };
        esp_http_client_handle_t cl = esp_http_client_init(&c);
        if (!cl) return 0;
        esp_http_client_set_header(cl, "Icy-MetaData", "1");
        esp_http_client_set_header(cl, "Accept", "*/*");
        if (esp_http_client_open(cl, 0) != ESP_OK) { esp_http_client_cleanup(cl); set_state("no connection"); return 0; }
        esp_http_client_fetch_headers(cl);
        int status = esp_http_client_get_status_code(cl);
        if (status != 200) { ESP_LOGW(TAG, "status %d", status); esp_http_client_cleanup(cl); set_state("server refused"); return 0; }
        pc_fmt_t fmt = pc_radio_guess(s_hdr.ctype, url);
        if (fmt == PC_FMT_PLAYLIST) {                      /* .pls / .m3u: the first stream in it */
            static char text[8192]; int n = 0, k;
            while (n < (int)sizeof text - 1 && (k = esp_http_client_read(cl, text + n, sizeof text - 1 - n)) > 0) n += k;
            esp_http_client_cleanup(cl);
            text[n] = 0;
            char next[300];
            if (!pc_playlist_first_url(text, next, sizeof next)) { set_state("empty playlist"); return 0; }
            snprintf(url, 300, "%s", next);
            continue;
        }
        if (fmt == PC_FMT_TS) { esp_http_client_cleanup(cl); set_state("HLS not supported"); s_run = false; return 1; }
        bool first = true;
        pc_icy_init(&s_icy, s_hdr.metaint > 0 ? (uint32_t)s_hdr.metaint : 0);
        s_st.prebuffer = (uint32_t)pc_radio_prebuffer(s_hdr.br);
        set_state("buffering");
        while (s_run) {
            int n = esp_http_client_read(cl, (char *)buf, sizeof buf);
            if (n <= 0) break;                              /* closed or timed out */
            if (first) {                                    /* servers mislabel AAC as audio/mpeg: trust the bytes over the header */
                first = false;
                pc_fmt_t sn = pc_radio_sniff(buf, (size_t)n);
                if (sn == PC_FMT_AAC || sn == PC_FMT_OGG || sn == PC_FMT_MP3) fmt = sn;
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
            if (pc_ring_used(&s_ring) >= s_st.prebuffer) set_state("");
        }
        esp_http_client_cleanup(cl);
        return s_run ? 0 : 1;
    }
    set_state("too many redirects");
    return 0;
}

static void net_task(void *arg)
{
    int fails = 0;
    while (s_run) {
        char url[300]; snprintf(url, sizeof url, "%s", s_url);
        uint32_t before = pc_ring_used(&s_ring);
        if (s_alt_n > 1 && fails > 0) {                                /* a failed round: next backup address */
            snprintf(url, sizeof url, "%s", s_alt[fails % s_alt_n]);
            ESP_LOGW(TAG, "trying backup %d", fails % s_alt_n);
        }
        int r = session(url);
        if (r) break;
        fails = pc_ring_used(&s_ring) != before || fails == 0 ? fails + 1 : fails + 1;
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
    if (!s_mem) return false;
    pc_ring_init(&s_ring, s_mem, RING_SIZE);
    s_st = (pc_stream_t){ .ring = &s_ring, .inner = &pc_backend_mp3, .prebuffer = 32768 };
    snprintf(s_url, sizeof s_url, "%s", url); s_title[0] = 0;
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
    nsn_radio_end();
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
