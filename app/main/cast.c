// SPDX-License-Identifier: LGPL-2.1-or-later
/* Auracast broadcast source. The LE Audio parts (advertising set, BASE, stream creation, the 10 ms encode loop) follow JukeDial's
 * app/main/bt_svc_cast_tx.c and broadcast_on / broadcast_off in bt_svc.c; the audio comes from Nano's player through a pull callback. */
#include "cast.h"
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "bt_ctl.h"
#include "fx.h"
#include "pc/pc_fxapp.h"

static const char *TAG = "cast";

#if CONFIG_BT_BAP_BROADCAST_SOURCE
#include "esp_audio_enc.h"
#include "esp_lc3_enc.h"
#include "esp_ble_audio_bap_api.h"
#include "esp_ble_audio_bap_lc3_preset_defs.h"
#include "esp_ble_audio_common_api.h"
#include "esp_ble_audio_defs.h"
#include "esp_ble_audio_lc3_defs.h"
#include "esp_ble_audio_pbp_api.h"
#include "esp_gap_ble_api.h"

#define CAST_NAME "NightStand Nano"
#define CAST_ID 0x4E534E                         /* "NSN" */
#define CAST_ADV 0                               /* advertising set 0; the control service uses set 1 (bt_ctl.c) */
#define RATE 48000
#define NBYTE 120                                /* 48_4: 120 bytes per channel per 10 ms */

ESP_BLE_AUDIO_BAP_LC3_BROADCAST_PRESET_48_4_1_DEFINE(s_preset, ESP_BLE_AUDIO_LOCATION_FRONT_LEFT | ESP_BLE_AUDIO_LOCATION_FRONT_RIGHT,
                                                     ESP_BLE_AUDIO_CONTEXT_TYPE_MEDIA);
typedef struct { esp_ble_audio_bap_stream_t stream; uint16_t seq; volatile bool ready; } cs_t;
static cs_t s_cs[2];
static esp_ble_audio_bap_broadcast_source_t *s_src;
static bool s_inited;
static volatile bool s_on;
static SemaphoreHandle_t s_adv_sem, s_lock;
static esp_bt_status_t s_adv_st;
static esp_timer_handle_t s_tick;
static TaskHandle_t s_task;
static volatile nsn_cast_pull_t s_pull; static void *volatile s_pull_ctx;
static volatile uint32_t s_frames, s_under, s_lc3_us, s_send_err, s_gap_max;

static int cs_index(esp_ble_audio_bap_stream_t *st) { return st == &s_cs[0].stream ? 0 : st == &s_cs[1].stream ? 1 : -1; }
static void cs_started(esp_ble_audio_bap_stream_t *st) { int i = cs_index(st); if (i >= 0) s_cs[i].ready = true; ESP_LOGI(TAG, "stream %d started", i); }
static void cs_stopped(esp_ble_audio_bap_stream_t *st, uint8_t r) { int i = cs_index(st); if (i >= 0) s_cs[i].ready = false; ESP_LOGI(TAG, "stream %d stopped (%u)", i, r); }
static esp_ble_audio_bap_stream_ops_t s_cs_ops = { .started = cs_started, .stopped = cs_stopped };

/* GAP events that are not the control service's (bt_ctl.c forwards them): each advertising step answers through the semaphore */
static void gap_hook(esp_gap_ble_cb_event_t ev, esp_ble_gap_cb_param_t *p)
{
    switch (ev) {
    case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT: s_adv_st = p->ext_adv_set_params.status; break;
    case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT: s_adv_st = p->ext_adv_data_set.status; break;
    case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT: s_adv_st = p->ext_adv_start.status; break;
    case ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT: s_adv_st = p->ext_adv_stop.status; break;
    case ESP_GAP_BLE_PERIODIC_ADV_SET_PARAMS_COMPLETE_EVT: s_adv_st = p->peroid_adv_set_params.status; break;
    case ESP_GAP_BLE_PERIODIC_ADV_DATA_SET_COMPLETE_EVT: s_adv_st = p->period_adv_data_set.status; break;
    case ESP_GAP_BLE_PERIODIC_ADV_START_COMPLETE_EVT: s_adv_st = p->period_adv_start.status; break;
    case ESP_GAP_BLE_PERIODIC_ADV_STOP_COMPLETE_EVT: s_adv_st = p->period_adv_stop.status; break;
    case ESP_GAP_BLE_EXT_ADV_SET_REMOVE_COMPLETE_EVT: s_adv_st = p->ext_adv_remove.status; break;
    default: return;
    }
    if (s_adv_sem) xSemaphoreGive(s_adv_sem);
}
#define ADV_CALL(x) do { esp_err_t _e = (x); if (_e || xSemaphoreTake(s_adv_sem, pdMS_TO_TICKS(2000)) != pdTRUE || s_adv_st) { \
        ESP_LOGE(TAG, "adv step %s failed e=%d st=%d", #x, _e, s_adv_st); return -1; } } while (0)

static int adv_start(void)
{
    static esp_ble_gap_ext_adv_params_t ep = {
        .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_NONCONN_NONSCANNABLE_UNDIRECTED,
        .interval_min = ESP_BLE_GAP_ADV_ITVL_MS(200), .interval_max = ESP_BLE_GAP_ADV_ITVL_MS(200),
        .channel_map = ADV_CHNL_ALL, .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
        .primary_phy = ESP_BLE_GAP_PHY_1M, .max_skip = 0, .secondary_phy = ESP_BLE_GAP_PHY_2M,
        .sid = CAST_ADV, .scan_req_notif = false, .own_addr_type = BLE_ADDR_TYPE_PUBLIC, .tx_power = 127,
    };
    static esp_ble_gap_periodic_adv_params_t pp = {
        .interval_min = ESP_BLE_GAP_PERIODIC_ADV_ITVL_MS(100), .interval_max = ESP_BLE_GAP_PERIODIC_ADV_ITVL_MS(100), .properties = 0,
    };
    /* extended advertising: Broadcast Audio Announcement (UUID + broadcast id), Public Broadcast Announcement (Auracast, high quality,
     * unencrypted), Broadcast Name */
    uint8_t ext[64]; int n = 0;
    ext[n++] = 6; ext[n++] = 0x16;
    ext[n++] = ESP_BLE_AUDIO_UUID_BROADCAST_AUDIO_VAL & 0xFF; ext[n++] = ESP_BLE_AUDIO_UUID_BROADCAST_AUDIO_VAL >> 8;
    ext[n++] = CAST_ID & 0xFF; ext[n++] = (CAST_ID >> 8) & 0xFF; ext[n++] = (CAST_ID >> 16) & 0xFF;
    NET_BUF_SIMPLE_DEFINE(pba, ESP_BLE_AUDIO_PBP_MIN_PBA_SIZE + 8);
    if (esp_ble_audio_pbp_get_announcement(NULL, 0, ESP_BLE_AUDIO_PBP_ANNOUNCEMENT_FEATURE_HIGH_QUALITY, &pba) == ESP_OK) {
        ext[n++] = (uint8_t)(1 + pba.len); ext[n++] = 0x16;
        memcpy(ext + n, pba.data, pba.len); n += pba.len;
    }
    ext[n++] = sizeof CAST_NAME; ext[n++] = 0x30;
    memcpy(ext + n, CAST_NAME, sizeof CAST_NAME - 1); n += sizeof CAST_NAME - 1;
    /* periodic advertising: the BASE */
    NET_BUF_SIMPLE_DEFINE(base, 128);
    if (esp_ble_audio_bap_broadcast_source_get_base(s_src, &base)) return -1;
    uint8_t per[140]; int m = 0;
    per[m++] = (uint8_t)(1 + base.len); per[m++] = 0x16;
    memcpy(per + m, base.data, base.len); m += base.len;
    static esp_ble_gap_ext_adv_t inst[1] = { { CAST_ADV, 0, 0 } };
    ADV_CALL(esp_ble_gap_ext_adv_set_params(CAST_ADV, &ep));
    ADV_CALL(esp_ble_gap_config_ext_adv_data_raw(CAST_ADV, (uint16_t)n, ext));
    ADV_CALL(esp_ble_gap_periodic_adv_set_params(CAST_ADV, &pp));
#if CONFIG_BT_BLE_FEAT_PERIODIC_ADV_ENH
    ADV_CALL(esp_ble_gap_config_periodic_adv_data_raw(CAST_ADV, (uint16_t)m, per, false));
    ADV_CALL(esp_ble_gap_periodic_adv_start(CAST_ADV, true));
#else
    ADV_CALL(esp_ble_gap_config_periodic_adv_data_raw(CAST_ADV, (uint16_t)m, per));
    ADV_CALL(esp_ble_gap_periodic_adv_start(CAST_ADV));
#endif
    ADV_CALL(esp_ble_gap_ext_adv_start(1, inst));
    return 0;
}

static void tick_cb(void *arg) { (void)arg; if (s_task) xTaskNotifyGive(s_task); }

/* every 10 ms: 480 frames from the player -> left / right LC3 (120 bytes each) -> one BIS each */
static void cast_task(void *arg)
{
    (void)arg;
    esp_audio_enc_handle_t enc[2] = { 0 };
    static int16_t pcm[480 * 2], ch[2][480];
    static uint8_t out[2][160];
    uint32_t enc_us = 0, enc_n = 0; int64_t last = 0;
    esp_lc3_enc_register();
    for (int c = 0; c < 2; c++) {
        esp_lc3_enc_config_t lc = ESP_LC3_ENC_CONFIG_DEFAULT();
        lc.sample_rate = RATE; lc.channel = 1; lc.frame_dms = 100; lc.nbyte = NBYTE;
        esp_audio_enc_config_t ec = { .type = ESP_AUDIO_TYPE_LC3, .cfg = &lc, .cfg_sz = sizeof lc };
        if (esp_audio_enc_open(&ec, &enc[c]) != ESP_AUDIO_ERR_OK) ESP_LOGE(TAG, "LC3 encoder open failed");
    }
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (!s_on) { last = 0; continue; }
        int64_t now = esp_timer_get_time();
        if (last && (uint32_t)(now - last) > s_gap_max) s_gap_max = (uint32_t)(now - last);
        last = now;
        const int fr = RATE / 100;
        nsn_cast_pull_t pull = s_pull;
        size_t got = pull ? pull(pcm, (size_t)fr, s_pull_ctx) : 0;
        if (got < (size_t)fr) { memset(pcm + got * 2, 0, ((size_t)fr - got) * 4); if (pull) s_under++; }
        for (int i = 0; i < fr; i++) { ch[0][i] = pcm[2 * i]; ch[1][i] = pcm[2 * i + 1]; }
        for (int c = 0; c < 2; c++) {
            if (!enc[c] || !s_cs[c].ready) continue;
            esp_audio_enc_in_frame_t in = { .buffer = (uint8_t *)ch[c], .len = (uint32_t)(fr * 2) };
            esp_audio_enc_out_frame_t o = { .buffer = out[c], .len = sizeof out[c] };
            int64_t te = esp_timer_get_time();
            esp_audio_err_t er = esp_audio_enc_process(enc[c], &in, &o);
            enc_us += (uint32_t)(esp_timer_get_time() - te); enc_n++;
            if (er != ESP_AUDIO_ERR_OK) continue;
            if (esp_ble_audio_bap_stream_send(&s_cs[c].stream, out[c], (uint16_t)o.encoded_bytes, s_cs[c].seq++)) s_send_err++;
        }
        if (++s_frames % 200 == 0 && enc_n) { s_lc3_us = enc_us / enc_n; enc_us = enc_n = 0; }
    }
}

static int create(void)
{
    static uint8_t left[] = { ESP_BLE_AUDIO_CODEC_DATA(ESP_BLE_AUDIO_CODEC_CFG_CHAN_ALLOC, BT_BYTES_LIST_LE32(ESP_BLE_AUDIO_LOCATION_FRONT_LEFT)) };
    static uint8_t right[] = { ESP_BLE_AUDIO_CODEC_DATA(ESP_BLE_AUDIO_CODEC_CFG_CHAN_ALLOC, BT_BYTES_LIST_LE32(ESP_BLE_AUDIO_LOCATION_FRONT_RIGHT)) };
    esp_ble_audio_bap_broadcast_source_stream_param_t sp[2];
    esp_ble_audio_bap_broadcast_source_subgroup_param_t sg = { .params_count = 2, .params = sp, .codec_cfg = &s_preset.codec_cfg };
    for (int i = 0; i < 2; i++) {
        sp[i].stream = &s_cs[i].stream;
        sp[i].data = i ? right : left;
        sp[i].data_len = i ? sizeof right : sizeof left;
        esp_ble_audio_bap_stream_cb_register(sp[i].stream, &s_cs_ops);
    }
    esp_ble_audio_bap_broadcast_source_param_t cp = { .params_count = 1, .params = &sg, .qos = &s_preset.qos,
                                                      .encryption = false, .packing = ESP_BLE_ISO_PACKING_SEQUENTIAL };
    return esp_ble_audio_bap_broadcast_source_create(&cp, &s_src);
}

static void memline(const char *what)
{
    ESP_LOGI(TAG, "%s: internal free %u, largest %u", what, (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

esp_err_t nsn_cast_start(void)
{
    if (!s_lock) s_lock = xSemaphoreCreateMutex();
    xSemaphoreTake(s_lock, portMAX_DELAY);
    esp_err_t e = ESP_OK;
    if (s_on) goto done;
    memline("before");
    if (!s_inited) {
        s_adv_sem = xSemaphoreCreateBinary();
        nsn_ble_set_gap_hook(gap_hook);
        nsn_ble_quiesce(true);                         /* LE Audio must come up with no GATT client connected (see bt_ctl.c) */
        e = esp_ble_audio_common_init(NULL);
        if (e) { ESP_LOGE(TAG, "common init %d", e); nsn_ble_quiesce(false); goto done; }
        static esp_ble_audio_bap_broadcast_source_cb_t cb = { 0 };
        esp_ble_audio_bap_broadcast_source_register_cb(&cb);
        if (!(e = create())) e = esp_ble_audio_common_start(NULL);
        nsn_ble_quiesce(false);
        if (e) { ESP_LOGE(TAG, "create / common start %d", e); goto done; }
        xTaskCreatePinnedToCore(cast_task, "cast", 10 * 1024, NULL, configMAX_PRIORITIES - 4, &s_task, 0);
        const esp_timer_create_args_t ta = { .callback = tick_cb, .name = "cast" };
        esp_timer_create(&ta, &s_tick);
        s_inited = true;
        memline("LE Audio up");
    } else if (!s_src && (e = create())) goto done;
    if (adv_start()) { e = ESP_FAIL; goto done; }
    esp_ble_audio_bap_broadcast_adv_info_t info = { .adv_handle = CAST_ADV };
    e = esp_ble_audio_bap_broadcast_adv_add(&info);
    if (!e) e = esp_ble_audio_bap_broadcast_source_start(s_src, CAST_ADV);
    if (e) { ESP_LOGE(TAG, "start %d", e); goto done; }
    fx_mode_set(PC_FXM_CAST, true);                    /* LC3 shares core 1 with the decoder: the heavy fx apps step aside (round's rule) */
    s_frames = s_under = s_send_err = s_gap_max = 0;
    s_on = true;
    esp_timer_start_periodic(s_tick, 10000);
    memline("broadcasting");
    ESP_LOGI(TAG, "broadcasting \"%s\" (LC3 48 kHz stereo, 2 BIS)", CAST_NAME);
done:
    xSemaphoreGive(s_lock);
    return e;
}

void nsn_cast_stop(void)
{
    if (!s_lock || !s_on) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_on = false;
    fx_mode_set(PC_FXM_CAST, false);
    if (s_tick) esp_timer_stop(s_tick);
    if (s_src) {
        esp_ble_audio_bap_broadcast_source_stop(s_src);
        vTaskDelay(pdMS_TO_TICKS(200));
        esp_ble_audio_bap_broadcast_source_delete(s_src);
        s_src = NULL;
    }
    esp_ble_audio_bap_broadcast_adv_info_t info = { .adv_handle = CAST_ADV };
    esp_ble_audio_bap_broadcast_adv_delete(&info);       /* or the next start's adv_add fails (JukeDial 2026-11-09) */
    xSemaphoreTake(s_adv_sem, 0);
    if (esp_ble_gap_periodic_adv_stop(CAST_ADV) == ESP_OK) xSemaphoreTake(s_adv_sem, pdMS_TO_TICKS(1000));
    uint8_t h = CAST_ADV;
    if (esp_ble_gap_ext_adv_stop(1, &h) == ESP_OK) xSemaphoreTake(s_adv_sem, pdMS_TO_TICKS(1000));
    if (esp_ble_gap_ext_adv_set_remove(CAST_ADV) == ESP_OK) xSemaphoreTake(s_adv_sem, pdMS_TO_TICKS(1000));
    for (int i = 0; i < 2; i++) s_cs[i].ready = false;
    ESP_LOGI(TAG, "broadcast stopped");
    xSemaphoreGive(s_lock);
}

bool nsn_cast_on(void) { return s_on; }
uint32_t nsn_cast_rate(void) { return RATE; }
void nsn_cast_set_source(nsn_cast_pull_t pull, void *ctx) { s_pull = NULL; s_pull_ctx = ctx; s_pull = pull; }
void nsn_cast_status(char *out, int n)
{
    if (!s_on) { snprintf(out, n, "cast: off%s", s_inited ? " (LE Audio stays loaded until reboot)" : ""); return; }
    snprintf(out, n, "cast: on \"%s\" frames=%lu underflows=%lu lc3_us_per_ch=%lu send_err=%lu gap_max_us=%lu streams=%d%d", CAST_NAME,
             (unsigned long)s_frames, (unsigned long)s_under, (unsigned long)s_lc3_us, (unsigned long)s_send_err, (unsigned long)s_gap_max,
             s_cs[0].ready, s_cs[1].ready);
}
#else
esp_err_t nsn_cast_start(void) { ESP_LOGE(TAG, "built without the Auracast broadcast source"); return ESP_ERR_NOT_SUPPORTED; }
void nsn_cast_stop(void) {}
bool nsn_cast_on(void) { return false; }
uint32_t nsn_cast_rate(void) { return 48000; }
void nsn_cast_set_source(nsn_cast_pull_t pull, void *ctx) { (void)pull; (void)ctx; }
void nsn_cast_status(char *out, int n) { snprintf(out, n, "cast: not in this build"); }
#endif
