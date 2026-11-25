// SPDX-License-Identifier: LGPL-2.1-or-later
#include "wifi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "pc/pc_kv.h"
#include "clock.h"
#include "settings.h"

static const char *TAG = "nsn_wifi";       /* not "wifi": that tag belongs to the driver, whose logs boot() quiets */
#define CFG "/sdcard/_player/config/wifi.txt"
#define KEY_SSID "wifi_ssid"
#define KEY_PASS "wifi_pass"

typedef enum { ST_OFF, ST_NOCRED, ST_CONNECTING, ST_CONNECTED, ST_WRONGPASS, ST_NOTFOUND } state_t;
typedef enum { SRC_NONE, SRC_NVS, SRC_CARD } src_t;

static bool s_inited, s_want, s_up;
static volatile state_t s_state = ST_OFF;
static volatile src_t s_src = SRC_NONE;
static volatile bool s_scanning;
static char s_ip[16];
static char s_ssid[33];                     /* the network in use (not secret) */
static TimerHandle_t s_retry;
static esp_netif_t *s_netif;
static SemaphoreHandle_t s_lock;            /* start / stop / set / forget / scan: console and BLE tasks may call at the same time */

static void lock(void) { if (s_lock) xSemaphoreTakeRecursive(s_lock, portMAX_DELAY); }
static void unlock(void) { if (s_lock) xSemaphoreGiveRecursive(s_lock); }

void nsn_wifi_boot(void)
{
    s_lock = xSemaphoreCreateRecursiveMutex();
    esp_log_level_set("wifi", ESP_LOG_ERROR);   /* the driver logs "connected with <ssid>" at info level, and rate tables at warning level */
}

static void retry_cb(TimerHandle_t t) { if (s_want && !s_scanning) esp_wifi_connect(); }

static void sntp_cb(struct timeval *tv) { nsn_clock_ntp_synced(); ESP_LOGI(TAG, "time from ntp"); }

/* Why the last attempt failed, from the disconnect event: bad passphrase shows as auth failure or a 4-way handshake that never finishes */
static state_t state_of_reason(int r)
{
    switch (r) {
    case WIFI_REASON_NO_AP_FOUND:
    case WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY:
    case WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD:
    case WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD:
        return ST_NOTFOUND;
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_MIC_FAILURE:
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
        return ST_WRONGPASS;
    default:
        return ST_CONNECTING;               /* beacon timeout, AP kicked us, ...: just trying again */
    }
}

static void on_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) { if (s_want) esp_wifi_connect(); }
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d = data;
        s_up = false; s_ip[0] = 0;
        if (s_want) {
            s_state = state_of_reason(d->reason);
            xTimerStart(s_retry, 0);        /* try again in 5 s, forever (a bedside box outlives router reboots) */
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = data;
        snprintf(s_ip, sizeof s_ip, IPSTR, IP2STR(&e->ip_info.ip));
        s_up = true; s_state = ST_CONNECTED;
        ESP_LOGI(TAG, "connected, ip %s", s_ip);                       /* never the network name */
        if (!esp_sntp_enabled()) {
            esp_sntp_config_t sc = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
            sc.sync_cb = sntp_cb;
            esp_netif_sntp_init(&sc);
        }
    }
}

/* The credentials to use: NVS first, the card file when nothing is stored. */
static src_t load_creds(char ssid[33], char pass[64])
{
    ssid[0] = pass[0] = 0;
    if (nsn_settings_get_str(KEY_SSID, ssid, 33) && *ssid) {      /* stored values were validated to fit */
        if (!nsn_settings_get_str(KEY_PASS, pass, 64)) pass[0] = 0;
        return SRC_NVS;
    }
    ssid[0] = 0;
    pc_kv_t kv[4];
    int n = pc_kv_load(CFG, kv, 4);
    const char *cs = pc_kv_get(kv, n, "ssid"), *cp = pc_kv_get(kv, n, "pass");
    src_t src = SRC_NONE;
    if (cs && *cs) { snprintf(ssid, 33, "%s", cs); snprintf(pass, 64, "%s", cp ? cp : ""); src = SRC_CARD; }
    memset(kv, 0, sizeof kv);
    return src;
}

/* driver up in station mode with no connection requested yet (the config is set by the caller) */
static esp_err_t driver_up(void)
{
    if (!s_inited) {
        esp_netif_init();
        esp_event_loop_create_default();                 /* ESP_ERR_INVALID_STATE if it exists: fine */
        s_netif = esp_netif_create_default_wifi_sta();
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, on_event, NULL);
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_event, NULL);
        s_retry = xTimerCreate("wifi_retry", pdMS_TO_TICKS(5000), pdFALSE, NULL, retry_cb);
        s_inited = true;
    }
    wifi_init_config_t ic = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t e = esp_wifi_init(&ic);
    if (e != ESP_OK) return e;
    esp_wifi_set_storage(WIFI_STORAGE_RAM);             /* the driver keeps nothing in its own NVS area; ours are the "nsn" keys */
    return esp_wifi_set_mode(WIFI_MODE_STA);
}

esp_err_t nsn_wifi_start(void)
{
    lock();
    if (s_want) { unlock(); return ESP_OK; }
    char ssid[33], pass[64];
    src_t src = load_creds(ssid, pass);
    s_src = src; snprintf(s_ssid, sizeof s_ssid, "%s", ssid);
    if (src == SRC_NONE) {
        s_state = ST_NOCRED;
        ESP_LOGI(TAG, "no credentials (nothing stored, no wifi.txt on the card): Wi-Fi stays off");
        unlock();
        return ESP_ERR_NOT_FOUND;
    }
    esp_err_t e = driver_up();
    if (e != ESP_OK) { unlock(); return e; }
    wifi_config_t wc = { 0 };
    memcpy(wc.sta.ssid, ssid, strlen(ssid));
    memcpy(wc.sta.password, pass, strlen(pass));
    wc.sta.threshold.authmode = *pass ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    esp_wifi_set_config(WIFI_IF_STA, &wc);
    memset(&wc, 0, sizeof wc); memset(pass, 0, sizeof pass);   /* do not leave the secrets on the stack */
    s_want = true; s_state = ST_CONNECTING;
    e = esp_wifi_start();
    ESP_LOGI(TAG, "start (%s): %s", src == SRC_NVS ? "stored" : "card", esp_err_to_name(e));
    unlock();
    return e;
}

void nsn_wifi_stop(void)
{
    lock();
    if (s_want) {
        s_want = false; s_up = false; s_ip[0] = 0; s_state = ST_OFF;
        if (s_retry) xTimerStop(s_retry, 0);
        esp_wifi_disconnect(); esp_wifi_stop(); esp_wifi_deinit();
    } else if (s_state != ST_NOCRED) s_state = ST_OFF;
    unlock();
}

bool nsn_wifi_connected(void) { return s_up; }
bool nsn_wifi_enabled(void) { return s_want; }

static bool text_ok(const char *s)
{
    for (; *s; s++) if ((unsigned char)*s < 0x20 || *s == 0x7f) return false;
    return true;
}

const char *nsn_wifi_set(const char *ssid, const char *pass)
{
    size_t ls = strlen(ssid), lp = strlen(pass);
    if (ls < 1 || ls > 32) return "ssid must be 1-32 bytes";
    if (lp != 0 && (lp < 8 || lp > 63)) return "password must be empty or 8-63 bytes";
    if (!text_ok(ssid) || !text_ok(pass)) return "control characters";
    lock();
    const char *err = NULL;
    if (!nsn_settings_set_str(KEY_PASS, pass) || !nsn_settings_set_str(KEY_SSID, ssid)) err = "cannot store";
    if (!err) { nsn_wifi_stop(); nsn_wifi_start(); }
    unlock();
    return err;
}

void nsn_wifi_forget(void)
{
    lock();
    nsn_settings_erase(KEY_SSID); nsn_settings_erase(KEY_PASS);
    nsn_wifi_stop();
    nsn_wifi_start();                                   /* the card file, if any */
    unlock();
}

static int by_rssi(const void *a, const void *b) { return ((const wifi_ap_record_t *)b)->rssi - ((const wifi_ap_record_t *)a)->rssi; }

int nsn_wifi_scan(nsn_ap_t *out, int max)
{
    if (__atomic_exchange_n(&s_scanning, true, __ATOMIC_ACQ_REL)) return -1;
    lock();
    bool temp = !s_want;                                /* not connected by choice: bring the driver up just for the scan */
    int res = -2;
    wifi_ap_record_t *ap = heap_caps_malloc(40 * sizeof *ap, MALLOC_CAP_SPIRAM);
    if (ap && (!temp || (driver_up() == ESP_OK && esp_wifi_start() == ESP_OK))) {
        if (esp_wifi_scan_start(NULL, true) == ESP_OK) {
            uint16_t n = 40;
            if (esp_wifi_scan_get_ap_records(&n, ap) == ESP_OK) {
                qsort(ap, n, sizeof *ap, by_rssi);
                res = 0;
                for (int i = 0; i < n && res < max; i++) {
                    const char *name = (const char *)ap[i].ssid;
                    if (!*name) continue;               /* hidden network */
                    bool dup = false;
                    for (int j = 0; j < res; j++) if (!strcmp(out[j].ssid, name)) dup = true;     /* mesh / repeaters: keep the strongest */
                    if (dup) continue;
                    out[res].rssi = ap[i].rssi; out[res].open = ap[i].authmode == WIFI_AUTH_OPEN;
                    snprintf(out[res].ssid, sizeof out[res].ssid, "%s", name);
                    res++;
                }
            }
        }
        if (temp) { esp_wifi_stop(); esp_wifi_deinit(); }
    } else if (temp && ap) esp_wifi_deinit();           /* driver_up() may have got half way */
    free(ap);
    unlock();
    __atomic_store_n(&s_scanning, false, __ATOMIC_RELEASE);
    return res;
}

void nsn_wifi_status(char *out, int n)
{
    int rssi = 0; wifi_ap_record_t ap;
    if (s_up && esp_wifi_sta_get_ap_info(&ap) == ESP_OK) rssi = ap.rssi;
    time_t t = time(NULL); struct tm tm; localtime_r(&t, &tm);
    static const char *const words[] = { "off", "no credentials", "connecting", "connected", "wrong password", "not found" };
    char esc[72]; size_t j = 0;                         /* JSON-style: backslash before quotes and backslashes */
    for (const char *s = s_ssid; *s && j < sizeof esc - 3; s++) { if (*s == '"' || *s == '\\') esc[j++] = '\\'; esc[j++] = *s; }
    esc[j] = 0;
    snprintf(out, n, "wifi %s ip=%s rssi=%d time=%s%04d-%02d-%02d %02d:%02d:%02d src=%s ssid=\"%s\"", words[s_state], s_ip, rssi,
             nsn_clock_valid() ? "" : "(unsynced) ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
             (const char *[]){ "none", "nvs", "card" }[s_src], esc);
}
