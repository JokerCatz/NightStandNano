// SPDX-License-Identifier: LGPL-2.1-or-later
#include "bt_audio.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "player.h"
#include "events.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_audio_dec.h"
#include "esp_aac_dec.h"
#include "esp_sbc_dec.h"
#include "esp_audio_enc.h"
#include "esp_aac_enc.h"
#include "esp_sbc_enc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/ringbuf.h"
#include "settings.h"
#include "wifi.h"
#include "ldacBT.h"
#include "pc/pc_btcodec.h"

static const char *TAG = "bt";
#define DEV_NAME "NightStand Nano"
#define MAX_SEEN 12
#define IN_RING_BYTES (64 * 1024)          /* phone PCM waiting for the player, PSRAM: ~370 ms at 44.1 kHz */

typedef enum { ROLE_NONE, ROLE_SOURCE, ROLE_SINK } role_t;
typedef enum { ST_IDLE, ST_CONNECTING, ST_CONNECTED } st_t;

static volatile role_t s_role = ROLE_NONE;
static volatile st_t s_st = ST_IDLE;                 /* headphones link */
static volatile bool s_scanning, s_up, s_streaming, s_mode_ready, s_in_conn, s_in_streaming;
static uint8_t s_peer[6];
static nsn_bt_dev_t s_seen[MAX_SEEN]; static int s_nseen;
static nsn_a2dp_pull_t s_pull; static void *s_pull_ctx;
static SemaphoreHandle_t s_up_sem, s_down_sem;
static QueueHandle_t s_cmd_q;                        /* worker commands */
static esp_a2d_mcc_t s_mcc;                          /* negotiated codec of the current link */
static volatile uint32_t s_rate = 44100;
static volatile uint32_t s_gen;                      /* bumped whenever the encoder must restart */
static esp_a2d_conn_hdl_t s_hdl;
static uint16_t s_mtu = 600;
static void (*s_in_cb)(bool);
static TimerHandle_t s_pair_tmr;

enum { CMD_MODE_SPEAKER, CMD_MODE_HEADPHONES, CMD_START, CMD_SUSPEND };

/* ---- codec helpers ---- */
static bool mcc_is_ldac(const esp_a2d_mcc_t *m)
{
    return m->type == ESP_A2D_MCT_NON_A2DP && m->cie.vendor_info.vendor_id == ESP_A2D_LDAC_VENDOR_ID && m->cie.vendor_info.codec_id == ESP_A2D_LDAC_CODEC_ID;
}

static uint32_t mcc_rate(const esp_a2d_mcc_t *m, int *ch)
{
    if (mcc_is_ldac(m)) {
        uint8_t f = m->cie.vendor_info.value[0];
        *ch = m->cie.vendor_info.value[1] & ESP_A2D_LDAC_CH_MONO ? 1 : 2;
        return f & ESP_A2D_LDAC_SF_48K ? 48000 : f & ESP_A2D_LDAC_SF_44K ? 44100 : f & ESP_A2D_LDAC_SF_96K ? 96000 : 88200;
    }
    if (m->type == ESP_A2D_MCT_SBC) {
        *ch = (m->cie.sbc_info.ch_mode & ESP_A2D_SBC_CIE_CH_MODE_MONO) ? 1 : 2;
        uint8_t f = m->cie.sbc_info.samp_freq;
        return f & ESP_A2D_SBC_CIE_SF_48K ? 48000 : f & ESP_A2D_SBC_CIE_SF_44K ? 44100 : f & ESP_A2D_SBC_CIE_SF_32K ? 32000 : 16000;
    }
    *ch = (m->cie.m24_info.ch & ESP_A2D_M24_CIE_CH_1) && !(m->cie.m24_info.ch & ESP_A2D_M24_CIE_CH_2) ? 1 : 2;
    if (m->cie.m24_info.samp_freq2 & ESP_A2D_M24_CIE_SF2_48K) return 48000;
    return m->cie.m24_info.samp_freq1 & ESP_A2D_M24_CIE_SF1_44K ? 44100 : 32000;
}
const char *nsn_bt_codec_name(void) { return !s_up ? "" : mcc_is_ldac(&s_mcc) ? "LDAC" : s_mcc.type == ESP_A2D_MCT_SBC ? "SBC" : s_mcc.type == ESP_A2D_MCT_M24 ? "AAC" : ""; }

static void register_source_seps(void)
{
    esp_a2d_mcc_t aac = { .type = ESP_A2D_MCT_M24 }, sbc = { .type = ESP_A2D_MCT_SBC };
    aac.cie.m24_info.drc = ESP_A2D_M24_CIE_DRC_NS;
    aac.cie.m24_info.obj_type = ESP_A2D_M24_CIE_OBJ_TYPE_2_AAC_LC;
    aac.cie.m24_info.samp_freq1 = ESP_A2D_M24_CIE_SF1_44K;
    aac.cie.m24_info.samp_freq2 = ESP_A2D_M24_CIE_SF2_48K;
    aac.cie.m24_info.ch = ESP_A2D_M24_CIE_CH_2;
    aac.cie.m24_info.vbr = 0;
    aac.cie.m24_info.br1 = (256000 >> 16) & ESP_A2D_M24_CIE_BR1_MSK; aac.cie.m24_info.br2 = (256000 >> 8) & 0xFF; aac.cie.m24_info.br3 = 256000 & 0xFF;
    sbc.cie.sbc_info.samp_freq = ESP_A2D_SBC_CIE_SF_44K | ESP_A2D_SBC_CIE_SF_48K;
    sbc.cie.sbc_info.ch_mode = ESP_A2D_SBC_CIE_CH_MODE_STEREO | ESP_A2D_SBC_CIE_CH_MODE_JOINT_STEREO;
    sbc.cie.sbc_info.block_len = ESP_A2D_SBC_CIE_BLOCK_LEN_16;
    sbc.cie.sbc_info.num_subbands = ESP_A2D_SBC_CIE_NUM_SUBBANDS_8;
    sbc.cie.sbc_info.alloc_mthd = ESP_A2D_SBC_CIE_ALLOC_MTHD_LOUDNESS;
    sbc.cie.sbc_info.min_bitpool = 2; sbc.cie.sbc_info.max_bitpool = 53;
    esp_a2d_mcc_t ldac = { .type = ESP_A2D_MCT_NON_A2DP };
    ldac.cie.vendor_info.vendor_id = ESP_A2D_LDAC_VENDOR_ID;
    ldac.cie.vendor_info.codec_id = ESP_A2D_LDAC_CODEC_ID;
    ldac.cie.vendor_info.value[0] = ESP_A2D_LDAC_SF_44K | ESP_A2D_LDAC_SF_48K;        /* nothing above 48 kHz */
    ldac.cie.vendor_info.value[1] = ESP_A2D_LDAC_CH_STEREO;
    /* the stack offers our endpoints in seid order: best first (LDAC > AAC > SBC) */
    if (esp_a2d_source_register_stream_endpoint(0, &ldac) != ESP_OK) ESP_LOGW(TAG, "LDAC source endpoint failed");
    if (esp_a2d_source_register_stream_endpoint(1, &aac) != ESP_OK) ESP_LOGW(TAG, "AAC source endpoint failed");
    if (esp_a2d_source_register_stream_endpoint(2, &sbc) != ESP_OK) ESP_LOGW(TAG, "SBC source endpoint failed");
}

static void register_sink_seps(void)
{
    esp_a2d_mcc_t sbc = { .type = ESP_A2D_MCT_SBC }, aac = { .type = ESP_A2D_MCT_M24 };
    sbc.cie.sbc_info.samp_freq = ESP_A2D_SBC_CIE_SF_32K | ESP_A2D_SBC_CIE_SF_44K | ESP_A2D_SBC_CIE_SF_48K;
    sbc.cie.sbc_info.ch_mode = ESP_A2D_SBC_CIE_CH_MODE_MONO | ESP_A2D_SBC_CIE_CH_MODE_DUAL_CHANNEL | ESP_A2D_SBC_CIE_CH_MODE_STEREO | ESP_A2D_SBC_CIE_CH_MODE_JOINT_STEREO;
    sbc.cie.sbc_info.block_len = ESP_A2D_SBC_CIE_BLOCK_LEN_4 | ESP_A2D_SBC_CIE_BLOCK_LEN_8 | ESP_A2D_SBC_CIE_BLOCK_LEN_12 | ESP_A2D_SBC_CIE_BLOCK_LEN_16;
    sbc.cie.sbc_info.num_subbands = ESP_A2D_SBC_CIE_NUM_SUBBANDS_4 | ESP_A2D_SBC_CIE_NUM_SUBBANDS_8;
    sbc.cie.sbc_info.alloc_mthd = ESP_A2D_SBC_CIE_ALLOC_MTHD_SNR | ESP_A2D_SBC_CIE_ALLOC_MTHD_LOUDNESS;
    sbc.cie.sbc_info.min_bitpool = 2; sbc.cie.sbc_info.max_bitpool = 250;
    aac.cie.m24_info.drc = ESP_A2D_M24_CIE_DRC_NS;
    aac.cie.m24_info.obj_type = ESP_A2D_M24_CIE_OBJ_TYPE_2_AAC_LC | ESP_A2D_M24_CIE_OBJ_TYPE_4_AAC_LC;
    aac.cie.m24_info.samp_freq1 = ESP_A2D_M24_CIE_SF1_32K | ESP_A2D_M24_CIE_SF1_44K;
    aac.cie.m24_info.samp_freq2 = ESP_A2D_M24_CIE_SF2_48K;
    aac.cie.m24_info.ch = ESP_A2D_M24_CIE_CH_1 | ESP_A2D_M24_CIE_CH_2;
    aac.cie.m24_info.vbr = ESP_A2D_M24_CIE_VBR_SUPPORT;
    aac.cie.m24_info.br1 = 0x7F & ESP_A2D_M24_CIE_BR1_MSK; aac.cie.m24_info.br2 = 0xFF & ESP_A2D_M24_CIE_BR2_MSK; aac.cie.m24_info.br3 = 0xFF & ESP_A2D_M24_CIE_BR3_MSK;
    /* SBC first: simple sources read only the first endpoint; phones still pick AAC by their own preference */
    if (esp_a2d_sink_register_stream_endpoint(0, &sbc) != ESP_OK) ESP_LOGW(TAG, "SBC sink endpoint failed");
    if (esp_a2d_sink_register_stream_endpoint(1, &aac) != ESP_OK) ESP_LOGW(TAG, "AAC sink endpoint failed");
}

/* ======================= speaker mode: decode what a phone sends ======================= */
static QueueHandle_t s_raw_q;
static RingbufHandle_t s_in_rb;
static esp_audio_dec_handle_t s_dec;
static volatile bool s_dec_ok, s_mcc_new;
static int s_dec_ch = 2;
static volatile uint32_t s_in_rate = 44100;

static void sink_data_cb(esp_a2d_conn_hdl_t h, esp_a2d_audio_buff_t *b)
{
    if (!s_raw_q || xQueueSend(s_raw_q, &b, 0) != pdTRUE) esp_a2d_audio_buff_free(b);
}

static void dec_open(void)
{
    if (s_dec) { esp_audio_dec_close(s_dec); s_dec = NULL; }
    s_dec_ok = false;
    s_in_rate = mcc_rate(&s_mcc, &s_dec_ch);
    esp_audio_err_t r;
    if (s_mcc.type == ESP_A2D_MCT_SBC) {
        esp_sbc_dec_cfg_t c = ESP_SBC_DEC_CONFIG_DEFAULT();
        c.sbc_mode = ESP_SBC_MODE_STD; c.ch_num = (uint8_t)s_dec_ch; c.enable_plc = true;
        esp_audio_dec_cfg_t d = { .type = ESP_AUDIO_TYPE_SBC, .cfg = &c, .cfg_sz = sizeof c };
        esp_sbc_dec_register();
        r = esp_audio_dec_open(&d, &s_dec);
    } else {
        esp_aac_dec_cfg_t c = ESP_AAC_DEC_CONFIG_DEFAULT();
        c.sample_rate = s_in_rate; c.channel = (uint8_t)s_dec_ch; c.bits_per_sample = ESP_AUDIO_BIT16; c.no_adts_header = true;
        c.aac_plus_enable = (s_mcc.cie.m24_info.obj_type & (ESP_A2D_M24_CIE_OBJ_TYPE_4_HE_AAC | ESP_A2D_M24_CIE_OBJ_TYPE_4_HE_AAC_V2)) != 0;
        esp_audio_dec_cfg_t d = { .type = ESP_AUDIO_TYPE_AAC, .cfg = &c, .cfg_sz = sizeof c };
        esp_aac_dec_register();
        r = esp_audio_dec_open(&d, &s_dec);
    }
    s_dec_ok = r == ESP_AUDIO_ERR_OK;
    ESP_LOGI(TAG, "phone stream: %s %lu Hz %d ch, decoder %s", s_mcc.type == ESP_A2D_MCT_SBC ? "SBC" : "AAC", (unsigned long)s_in_rate, s_dec_ch, s_dec_ok ? "ok" : "FAILED");
}

static void decode_task(void *arg)                      /* core 1 */
{
    uint32_t cap = 8192;
    uint8_t *out = heap_caps_malloc(cap, MALLOC_CAP_SPIRAM);
    int16_t *st = heap_caps_malloc(8192 * 2, MALLOC_CAP_SPIRAM);          /* stereo conversion of a decoded block */
    for (;;) {
        esp_a2d_audio_buff_t *b;
        if (xQueueReceive(s_raw_q, &b, portMAX_DELAY) != pdTRUE) continue;
        if (s_mcc_new) { s_mcc_new = false; dec_open(); }
        if (!s_dec_ok || !out || !st) { esp_a2d_audio_buff_free(b); continue; }
        esp_audio_dec_in_raw_t in = { .buffer = b->data, .len = b->data_len };
        while (in.len > 0) {
            esp_audio_dec_out_frame_t o = { .buffer = out, .len = cap };
            esp_audio_err_t r = esp_audio_dec_process(s_dec, &in, &o);
            if (r == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
                uint8_t *n = heap_caps_realloc(out, cap * 2, MALLOC_CAP_SPIRAM);
                if (!n) break;
                out = n; cap *= 2;
                continue;
            }
            if (r != ESP_AUDIO_ERR_OK) break;
            const int16_t *pcm = (const int16_t *)out;
            int frames = (int)(o.decoded_size / (2u * (unsigned)s_dec_ch));
            if (frames > 8192) frames = 8192;
            for (int i = 0; i < frames; i++) { st[2 * i] = pcm[i * s_dec_ch]; st[2 * i + 1] = pcm[i * s_dec_ch + (s_dec_ch > 1)]; }
            if (s_in_rb && frames) xRingbufferSend(s_in_rb, st, (size_t)frames * 4, 0);     /* full: drop (the reader keeps the fill centred) */
            in.buffer += in.consumed;
            in.len -= in.consumed;
        }
        esp_a2d_audio_buff_free(b);
    }
}

bool nsn_bt_in_connected(void) { return s_in_conn; }
bool nsn_bt_in_streaming(void) { return s_in_streaming; }
uint32_t nsn_bt_in_rate(void) { return s_in_rate; }
void nsn_bt_set_in_callback(void (*cb)(bool)) { s_in_cb = cb; }
size_t nsn_bt_in_fill(void) { return s_in_rb ? (IN_RING_BYTES - xRingbufferGetCurFreeSize(s_in_rb)) / 4 : 0; }

size_t nsn_bt_in_read(int16_t *dst, size_t frames)
{
    size_t want = frames * 4, got = 0;
    while (s_in_rb && got < want) {
        size_t n = 0;
        uint8_t *p = xRingbufferReceiveUpTo(s_in_rb, &n, 0, want - got);
        if (!p) break;
        memcpy((uint8_t *)dst + got, p, n);
        vRingbufferReturnItem(s_in_rb, p);
        got += n;
    }
    return got / 4;
}
void nsn_bt_in_flush(void) { int16_t tmp[512]; while (nsn_bt_in_read(tmp, 256)) ; }

/* ======================= LDAC ======================= */
static void ldac_set_level(HANDLE_LDAC_BT h, int lv)
{
    ldacBT_set_eqmid(h, lv == 0 ? LDACBT_EQMID_HQ : lv == 3 ? LDACBT_EQMID_MQ : LDACBT_EQMID_SQ);
    if (lv == 2) ldacBT_alter_eqmid_priority(h, LDACBT_EQMID_INC_CONNECTION);   /* SQ -> Q0 (492) */
}

/* Wi-Fi shares the radio: the ABR caps LDAC lower while it is on (JukeDial's measured rule, pc_ldac_abr_cap) */
static void ldac_stream(uint32_t gen)
{
    const uint32_t rate = s_rate;
    if (!pc_btc_fits_mtu(PC_BTC_LDAC, s_mtu)) { ESP_LOGE(TAG, "LDAC needs mtu >= %d, link mtu=%u", PC_LDAC_MTU_MIN, s_mtu); vTaskDelay(pdMS_TO_TICKS(1000)); return; }
    pc_ldac_abr_t abr;
    int max_kbps = 990;
    pc_ldac_abr_init(&abr, nsn_wifi_enabled(), max_kbps);
    int lv = abr.level;
    HANDLE_LDAC_BT h = ldacBT_get_handle();
    if (!h || ldacBT_init_handle_encode(h, s_mtu, LDACBT_EQMID_SQ, LDACBT_CHANNEL_MODE_STEREO, LDACBT_SMPL_FMT_S16, (int)rate)) {
        ESP_LOGE(TAG, "LDAC init failed err=%d mtu=%u", h ? ldacBT_get_error_code(h) : -1, s_mtu);
        if (h) ldacBT_free_handle(h);
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }
    ldac_set_level(h, lv);
    ESP_LOGI(TAG, "encoder LDAC %lu Hz, level %d kbps (cap %d, wifi %d), mtu %u", (unsigned long)rate, pc_ldac_kbps(pc_ldac_level(lv)->frame_bytes, rate), max_kbps, nsn_wifi_enabled(), s_mtu);
    int16_t *pcm = heap_caps_malloc(2 * LDACBT_ENC_LSU * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    uint8_t *out = heap_caps_malloc(LDACBT_MAX_NBYTES, MALLOC_CAP_SPIRAM);
    uint32_t ts = 0, sent = 0, fails = 0, lost_total = 0, errs = 0, drops0 = 0;
    esp_a2d_source_get_tx_stats(NULL, &drops0);
    int64_t t0 = esp_timer_get_time(); uint64_t samples = 0;
    while (pcm && out && s_streaming && gen == s_gen && s_role == ROLE_SOURCE) {
        size_t got = s_pull ? s_pull(pcm, LDACBT_ENC_LSU, s_pull_ctx) : 0;
        if (got < LDACBT_ENC_LSU) memset(pcm + got * 2, 0, (LDACBT_ENC_LSU - got) * 4);
        int used = 0, sz = 0, nf = 0;
        int r = ldacBT_encode(h, pcm, &used, out, &sz, &nf);
        if (r) { if (errs++ < 5) ESP_LOGW(TAG, "LDAC encode error 0x%x", ldacBT_get_error_code(h)); }
        else if (sz > 0) {
            uint32_t f0 = fails;
            esp_a2d_audio_buff_t *b = esp_a2d_audio_buff_alloc((uint16_t)sz);
            if (b) {
                memcpy(b->data, out, (size_t)sz); b->data_len = (uint16_t)sz; b->number_frame = (uint16_t)nf; b->timestamp = ts;
                if (esp_a2d_source_audio_data_send(s_hdl, b) != ESP_OK) { esp_a2d_audio_buff_free(b); fails++; } else sent++;
            } else fails++;
            ts += (uint32_t)nf * LDACBT_ENC_LSU;
            uint16_t q = 0; uint32_t drops = drops0;
            esp_a2d_source_get_tx_stats(&q, &drops);
            int lost = (int)(drops - drops0) + (int)(fails - f0);
            drops0 = drops; lost_total += (uint32_t)lost;
            int nl = pc_ldac_abr_update(&abr, (uint32_t)(esp_timer_get_time() / 1000), q, lost, nsn_wifi_enabled(), max_kbps);
            if (nl != lv) {
                ESP_LOGI(TAG, "LDAC abr %d -> %d kbps (queue %d, lost %d)", pc_ldac_kbps(pc_ldac_level(lv)->frame_bytes, rate), pc_ldac_kbps(pc_ldac_level(nl)->frame_bytes, rate), q, lost);
                ldac_set_level(h, nl); lv = nl;
            }
        }
        samples += LDACBT_ENC_LSU;                                       /* real-time pacing from the sample count; a stall over 100 ms is not made up in a burst */
        int64_t due = t0 + (int64_t)(samples * 1000000ull / rate), now = esp_timer_get_time();
        if (now - due > 100000) { t0 = now; samples = 0; }
        else if (due - now > 3000) vTaskDelay(pdMS_TO_TICKS((due - now) / 1000));
    }
    ESP_LOGI(TAG, "LDAC end: sent %lu, send failures %lu, lost %lu, abr down/up %lu/%lu", (unsigned long)sent, (unsigned long)fails, (unsigned long)lost_total, (unsigned long)abr.downs, (unsigned long)abr.ups);
    heap_caps_free(pcm); heap_caps_free(out);
    ldacBT_free_handle(h);
}

/* offline check of the encoder (no headphones needed): 1 s of a tone at each quality, frame sizes and the 0xAA sync byte */
void nsn_bt_ldac_selftest(char *res, int n)
{
    int pos = 0;
    HANDLE_LDAC_BT h = ldacBT_get_handle();
    int16_t *pcm = heap_caps_malloc(2 * LDACBT_ENC_LSU * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    uint8_t *out = heap_caps_malloc(LDACBT_MAX_NBYTES, MALLOC_CAP_SPIRAM);
    if (!h || !pcm || !out) { snprintf(res, n, "ldactest: no memory"); goto done; }
    for (int lv = 0; lv < 4; lv++) {
        if (ldacBT_init_handle_encode(h, 679, LDACBT_EQMID_SQ, LDACBT_CHANNEL_MODE_STEREO, LDACBT_SMPL_FMT_S16, 44100)) { pos += snprintf(res + pos, n - pos, "init failed (lv %d) ", lv); break; }
        ldac_set_level(h, lv);
        uint32_t bytes = 0, packets = 0, bad = 0; float ph = 0;
        for (int blk = 0; blk < 44100 / LDACBT_ENC_LSU; blk++) {
            for (int i = 0; i < LDACBT_ENC_LSU; i++) { pcm[2 * i] = pcm[2 * i + 1] = (int16_t)(8000 * sinf(ph)); ph += 6.2831853f * 1000.0f / 44100.0f; }
            int used = 0, sz = 0, nf = 0;
            if (ldacBT_encode(h, pcm, &used, out, &sz, &nf)) { bad++; continue; }
            if (sz > 0) { packets++; bytes += (uint32_t)sz; if (out[0] != 0xAA) bad++; }
        }
        ldacBT_close_handle(h);
        pos += snprintf(res + pos, n - pos, "lv%d: %lu packets %lu B (%lu kbps) bad %lu; ", lv, (unsigned long)packets, (unsigned long)bytes, (unsigned long)(bytes * 8 / 1000), (unsigned long)bad);
    }
done:
    if (h) ldacBT_free_handle(h);
    heap_caps_free(pcm); heap_caps_free(out);
}

/* ======================= headphones mode: encode our audio ======================= */
static void encode_task(void *arg)                      /* core 1 */
{
    for (;;) {
        while (!s_streaming || s_role != ROLE_SOURCE) vTaskDelay(pdMS_TO_TICKS(20));
        uint32_t gen = s_gen;
        if (mcc_is_ldac(&s_mcc)) { s_rate = ({ int ch; mcc_rate(&s_mcc, &ch); }); ldac_stream(gen); continue; }
        bool aac = s_mcc.type == ESP_A2D_MCT_M24;
        int ch; uint32_t rate = mcc_rate(&s_mcc, &ch); s_rate = rate;
        esp_audio_enc_handle_t enc = NULL; esp_audio_err_t r;
        if (aac) {
            esp_aac_enc_config_t c = ESP_AAC_ENC_CONFIG_DEFAULT();
            c.sample_rate = (int)rate; c.channel = 2; c.bits_per_sample = 16; c.bitrate = 256000; c.adts_used = false;
            esp_audio_enc_config_t ec = { .type = ESP_AUDIO_TYPE_AAC, .cfg = &c, .cfg_sz = sizeof c };
            esp_aac_enc_register();
            r = esp_audio_enc_open(&ec, &enc);
        } else {
            esp_sbc_enc_config_t c = ESP_SBC_STD_ENC_CONFIG_DEFAULT();
            c.sample_rate = rate; c.ch_mode = ESP_SBC_CH_MODE_JOINT_STEREO; c.allocation_method = ESP_SBC_ALLOC_LOUDNESS;
            c.bitpool = s_mcc.cie.sbc_info.max_bitpool < 53 ? s_mcc.cie.sbc_info.max_bitpool : 53;
            c.block_length = 16; c.sub_bands_num = 8;
            esp_audio_enc_config_t ec = { .type = ESP_AUDIO_TYPE_SBC, .cfg = &c, .cfg_sz = sizeof c };
            esp_sbc_enc_register();
            r = esp_audio_enc_open(&ec, &enc);
        }
        int in_sz = 0, out_sz = 0;
        if (r == ESP_AUDIO_ERR_OK) esp_audio_enc_get_frame_size(enc, &in_sz, &out_sz);
        ESP_LOGI(TAG, "encoder %s %lu Hz in=%d out=%d rc=%d", aac ? "AAC" : "SBC", (unsigned long)rate, in_sz, out_sz, r);
        if (r != ESP_AUDIO_ERR_OK || in_sz <= 0) { if (enc) esp_audio_enc_close(enc); vTaskDelay(pdMS_TO_TICKS(500)); continue; }
        int fs = in_sz / 4;
        uint8_t *pcm = heap_caps_malloc((size_t)in_sz, MALLOC_CAP_SPIRAM), *frame = heap_caps_malloc((size_t)out_sz, MALLOC_CAP_SPIRAM), *pkt = heap_caps_malloc(2048, MALLOC_CAP_SPIRAM);
        uint32_t ts = 0, sent = 0, fails = 0;
        int64_t next = esp_timer_get_time();
        while (s_streaming && gen == s_gen && s_role == ROLE_SOURCE && pcm && frame && pkt) {
            int nf = 0, len = 0, cap = aac ? 1 : 15, maxlen = (int)s_mtu - (aac ? 20 : 13);
            while (nf < cap) {
                size_t got = s_pull ? s_pull((int16_t *)pcm, (size_t)fs, s_pull_ctx) : 0;
                if (got < (size_t)fs) memset(pcm + got * 4, 0, (size_t)(fs - got) * 4);
                esp_audio_enc_in_frame_t in = { .buffer = pcm, .len = (uint32_t)in_sz };
                esp_audio_enc_out_frame_t o = { .buffer = frame, .len = (uint32_t)out_sz };
                if (esp_audio_enc_process(enc, &in, &o) != ESP_AUDIO_ERR_OK || !o.encoded_bytes) break;
                if (len + (int)o.encoded_bytes > maxlen || len + (int)o.encoded_bytes > 2048) { if (!nf) break; }
                memcpy(pkt + len, frame, o.encoded_bytes);
                len += (int)o.encoded_bytes; nf++;
                if (len + (int)o.encoded_bytes > maxlen) break;
            }
            if (!nf) { vTaskDelay(1); continue; }
            esp_a2d_audio_buff_t *b = esp_a2d_audio_buff_alloc((uint16_t)len);
            if (b) {
                memcpy(b->data, pkt, (size_t)len);
                b->data_len = (uint16_t)len; b->number_frame = (uint16_t)nf; b->timestamp = ts;
                if (esp_a2d_source_audio_data_send(s_hdl, b) != ESP_OK) { esp_a2d_audio_buff_free(b); fails++; } else sent++;
            }
            ts += (uint32_t)(nf * fs);
            next += (int64_t)nf * fs * 1000000 / rate;                    /* real time */
            int64_t wait = next - esp_timer_get_time();
            if (wait > 2000) vTaskDelay(pdMS_TO_TICKS(wait / 1000));
            else if (wait < -200000) next = esp_timer_get_time();         /* far behind (a stall): do not burst */
        }
        if (fails) ESP_LOGW(TAG, "encoder end: sent %lu, send failures %lu", (unsigned long)sent, (unsigned long)fails);
        if (enc) esp_audio_enc_close(enc);
        heap_caps_free(pcm); heap_caps_free(frame); heap_caps_free(pkt);
    }
}

/* ======================= callbacks ======================= */
static void gap_cb(esp_bt_gap_cb_event_t ev, esp_bt_gap_cb_param_t *p)
{
    switch (ev) {
    case ESP_BT_GAP_DISC_RES_EVT: {
        uint32_t cod = 0; int8_t rssi = -127; char name[33] = "";
        for (int i = 0; i < p->disc_res.num_prop; i++) {
            esp_bt_gap_dev_prop_t *d = &p->disc_res.prop[i];
            if (d->type == ESP_BT_GAP_DEV_PROP_COD) cod = *(uint32_t *)d->val;
            else if (d->type == ESP_BT_GAP_DEV_PROP_RSSI) rssi = *(int8_t *)d->val;
            else if (d->type == ESP_BT_GAP_DEV_PROP_BDNAME) { int n = d->len > 32 ? 32 : d->len; memcpy(name, d->val, n); name[n] = 0; }
            else if (d->type == ESP_BT_GAP_DEV_PROP_EIR) {
                uint8_t l = 0; uint8_t *n = esp_bt_gap_resolve_eir_data(d->val, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &l);
                if (!n) n = esp_bt_gap_resolve_eir_data(d->val, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &l);
                if (n) { if (l > 32) l = 32; memcpy(name, n, l); name[l] = 0; }
            }
        }
        int k = -1;
        for (int i = 0; i < s_nseen; i++) if (!memcmp(s_seen[i].bda, p->disc_res.bda, 6)) k = i;
        if (k < 0 && s_nseen < MAX_SEEN) k = s_nseen++;
        if (k >= 0) {
            memcpy(s_seen[k].bda, p->disc_res.bda, 6); if (name[0]) strlcpy(s_seen[k].name, name, sizeof s_seen[k].name);
            s_seen[k].rssi = rssi; s_seen[k].cod = cod;
            s_seen[k].audio = esp_bt_gap_is_valid_cod(cod) && (esp_bt_gap_get_cod_srvc(cod) & ESP_BT_COD_SRVC_RENDERING);
        }
        break;
    }
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
        s_scanning = p->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED;
        ESP_LOGI(TAG, "discovery %s, %d device(s)", s_scanning ? "started" : "stopped", s_nseen);
        break;
    case ESP_BT_GAP_AUTH_CMPL_EVT:
        ESP_LOGI(TAG, "auth %s: %s", p->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS ? "ok" : "FAILED", p->auth_cmpl.device_name);
        break;
    case ESP_BT_GAP_CFM_REQ_EVT:
        esp_bt_gap_ssp_confirm_reply(p->cfm_req.bda, true);       /* just works / numeric compare: accept (no screen) */
        break;
    case ESP_BT_GAP_PIN_REQ_EVT: { esp_bt_pin_code_t pin = { '0', '0', '0', '0' }; esp_bt_gap_pin_reply(p->pin_req.bda, true, 4, pin); break; }
    default: break;
    }
}

static void a2d_cb(esp_a2d_cb_event_t ev, esp_a2d_cb_param_t *p)
{
    if (ev == ESP_A2D_PROF_STATE_EVT) {
        if (p->a2d_prof_stat.init_state == ESP_A2D_DEINIT_SUCCESS) { if (s_down_sem) xSemaphoreGive(s_down_sem); return; }
        if (s_role == ROLE_SOURCE) register_source_seps(); else register_sink_seps();
        if (s_up_sem) xSemaphoreGive(s_up_sem);
        return;
    }
    if (s_role == ROLE_SINK) {
        switch (ev) {
        case ESP_A2D_CONNECTION_STATE_EVT:
            if (p->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
                s_in_conn = true; memcpy(s_peer, p->conn_stat.remote_bda, 6);
                esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);   /* one phone at a time */
                ESP_LOGI(TAG, "phone connected");
            } else if (p->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
                bool was = s_in_streaming;
                s_in_conn = false; s_in_streaming = false;
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
                ESP_LOGI(TAG, "phone gone");
                if (was && s_in_cb) s_in_cb(false);
            }
            break;
        case ESP_A2D_AUDIO_STATE_EVT: {
            bool on = p->audio_stat.state == ESP_A2D_AUDIO_STATE_STARTED;
            if (on != s_in_streaming) { s_in_streaming = on; ESP_LOGI(TAG, "phone %s", on ? "playing" : "paused"); if (s_in_cb) s_in_cb(on); }
            break;
        }
        case ESP_A2D_AUDIO_CFG_EVT: s_mcc = p->audio_cfg.mcc; s_mcc_new = true; { int ch; s_in_rate = mcc_rate(&s_mcc, &ch); } break;
        default: break;
        }
        return;
    }
    switch (ev) {                                                   /* source (headphones) */
    case ESP_A2D_CONNECTION_STATE_EVT:
        if (p->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
            s_st = ST_CONNECTED; s_hdl = p->conn_stat.conn_hdl; memcpy(s_peer, p->conn_stat.remote_bda, 6);
            if (p->conn_stat.audio_mtu) s_mtu = p->conn_stat.audio_mtu;
            ESP_LOGI(TAG, "a2dp connected, mtu %u", s_mtu);
        } else if (p->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            s_st = ST_IDLE; s_streaming = false; s_gen++;
            ESP_LOGI(TAG, "a2dp disconnected (reason %d)", p->conn_stat.disc_rsn);
        }
        break;
    case ESP_A2D_AUDIO_STATE_EVT:
        s_streaming = p->audio_stat.state == ESP_A2D_AUDIO_STATE_STARTED; s_gen++;
        ESP_LOGI(TAG, "audio %s", s_streaming ? "started" : "stopped/suspended");
        break;
    case ESP_A2D_AUDIO_CFG_EVT: {
        s_mcc = p->audio_cfg.mcc; s_gen++;
        int ch; s_rate = mcc_rate(&s_mcc, &ch);
        ESP_LOGI(TAG, "codec %s %lu Hz", nsn_bt_codec_name(), (unsigned long)s_rate);
        break;
    }
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
        if (p->media_ctrl_stat.status != ESP_A2D_MEDIA_CTRL_ACK_SUCCESS) { ESP_LOGW(TAG, "media ctrl %d -> status %d", p->media_ctrl_stat.cmd, p->media_ctrl_stat.status); break; }
        if (p->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY) esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START);
        break;
    default: break;
    }
}

static void avrc_ct_cb(esp_avrc_ct_cb_event_t ev, esp_avrc_ct_cb_param_t *p)
{
    if (ev == ESP_AVRC_CT_CONNECTION_STATE_EVT) ESP_LOGI(TAG, "avrcp %s", p->conn_stat.connected ? "connected" : "disconnected");
}

/* Headphone buttons arrive as AVRCP pass-through commands (we are the target when sending music). */
static void avrc_tg_cb(esp_avrc_tg_cb_event_t ev, esp_avrc_tg_cb_param_t *p)
{
    if (ev != ESP_AVRC_TG_PASSTHROUGH_CMD_EVT || p->psth_cmd.key_state != ESP_AVRC_PT_CMD_STATE_PRESSED) return;
    nsn_player_info_t in; nsn_player_info(&in);
    const char *did = NULL;                          /* what the box did; NULL = a key it does not act on (still recorded: it tells what the headphones send) */
    switch (p->psth_cmd.key_code) {
    case ESP_AVRC_PT_CMD_PLAY: nsn_player_pause(false); did = "play"; break;
    case ESP_AVRC_PT_CMD_PAUSE: nsn_player_pause(true); did = "pause"; break;
    case ESP_AVRC_PT_CMD_STOP: nsn_player_pause(true); did = "pause"; break;
    case ESP_AVRC_PT_CMD_FORWARD: nsn_player_skip(1); did = "next"; break;
    case ESP_AVRC_PT_CMD_BACKWARD: nsn_player_skip(-1); did = "prev"; break;
    case ESP_AVRC_PT_CMD_VOL_UP: nsn_player_set_volume(nsn_player_volume() + 5 > 100 ? 100 : nsn_player_volume() + 5); did = "vol+"; break;
    case ESP_AVRC_PT_CMD_VOL_DOWN: nsn_player_set_volume(nsn_player_volume() < 5 ? 0 : nsn_player_volume() - 5); did = "vol-"; break;
    default: break;
    }
    ESP_LOGI(TAG, "avrcp key 0x%02x -> %s", p->psth_cmd.key_code, did ? did : "ignored");
    nsn_event("bt 0x%02x %s", p->psth_cmd.key_code, did ? did : "ignored");
}

/* ======================= role switching (worker task) ======================= */
static void set_scan_for_role(void)
{
    int bonds = esp_bt_gap_get_bond_device_num();
    if (s_role == ROLE_SINK) esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, bonds <= 0 ? ESP_BT_GENERAL_DISCOVERABLE : ESP_BT_NON_DISCOVERABLE);   /* nobody paired yet: let the first phone find us */
    else esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
}

static void role_down(void)
{
    if (s_role == ROLE_NONE) return;
    s_streaming = false; s_gen++; s_mode_ready = false;
    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
    if (s_role == ROLE_SOURCE && s_st == ST_CONNECTED) esp_a2d_source_disconnect(s_peer);
    if (s_role == ROLE_SINK && s_in_conn) esp_a2d_sink_disconnect(s_peer);
    for (int i = 0; i < 70 && (s_st == ST_CONNECTED || s_in_conn); i++) vTaskDelay(pdMS_TO_TICKS(100));   /* the link down first: a deinit during a slow disconnect times out */
    vTaskDelay(pdMS_TO_TICKS(300));
    esp_avrc_ct_deinit();
    esp_avrc_tg_deinit();
    vTaskDelay(pdMS_TO_TICKS(100));
    xSemaphoreTake(s_down_sem, 0);
    if (s_role == ROLE_SOURCE) esp_a2d_source_deinit(); else esp_a2d_sink_deinit();
    if (xSemaphoreTake(s_down_sem, pdMS_TO_TICKS(10000)) != pdTRUE) ESP_LOGW(TAG, "a2dp deinit timeout");
    s_role = ROLE_NONE; s_st = ST_IDLE; s_in_conn = false; s_in_streaming = false;
}

static void role_up(role_t r)
{
    s_role = r;
    esp_avrc_ct_init();
    esp_avrc_ct_register_callback(avrc_ct_cb);
    esp_avrc_tg_init();
    esp_avrc_tg_register_callback(avrc_tg_cb);
    {   /* the keys we act on */
        esp_avrc_psth_bit_mask_t m; memset(&m, 0, sizeof m);
        static const int keys[] = { ESP_AVRC_PT_CMD_PLAY, ESP_AVRC_PT_CMD_PAUSE, ESP_AVRC_PT_CMD_STOP, ESP_AVRC_PT_CMD_FORWARD, ESP_AVRC_PT_CMD_BACKWARD, ESP_AVRC_PT_CMD_VOL_UP, ESP_AVRC_PT_CMD_VOL_DOWN };
        for (unsigned i = 0; i < sizeof keys / sizeof *keys; i++) esp_avrc_psth_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &m, keys[i]);
        esp_avrc_tg_set_psth_cmd_filter(ESP_AVRC_PSTH_FILTER_SUPPORTED_CMD, &m);
    }
    xSemaphoreTake(s_up_sem, 0);
    esp_err_t e;
    if (r == ROLE_SOURCE) e = esp_a2d_source_init(); else e = esp_a2d_sink_init();
    if (e != ESP_OK) { ESP_LOGE(TAG, "a2dp init failed: %s", esp_err_to_name(e)); s_role = ROLE_NONE; return; }
    if (xSemaphoreTake(s_up_sem, pdMS_TO_TICKS(3000)) != pdTRUE) ESP_LOGW(TAG, "a2dp init timeout: no endpoints");
    if (r == ROLE_SINK) esp_a2d_sink_register_audio_data_callback(sink_data_cb);
    set_scan_for_role();
    s_mode_ready = true;
    ESP_LOGI(TAG, "%s mode ready", r == ROLE_SINK ? "speaker (A2DP sink)" : "headphones (A2DP source)");
}

static void worker(void *arg)
{
    uint8_t c;
    for (;;) {
        if (xQueueReceive(s_cmd_q, &c, portMAX_DELAY) != pdTRUE) continue;
        switch (c) {
        case CMD_MODE_SPEAKER: if (s_role != ROLE_SINK) { role_down(); role_up(ROLE_SINK); } break;
        case CMD_MODE_HEADPHONES: if (s_role != ROLE_SOURCE) { role_down(); role_up(ROLE_SOURCE); } break;
        case CMD_START: if (s_role == ROLE_SOURCE) esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY); break;
        case CMD_SUSPEND: if (s_role == ROLE_SOURCE) esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_SUSPEND); break;
        }
    }
}

static void pair_end(TimerHandle_t t) { if (s_role == ROLE_SINK) set_scan_for_role(); }
void nsn_bt_pairable(int seconds)
{
    if (s_role != ROLE_SINK) return;
    if (seconds > 0) {
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        xTimerChangePeriod(s_pair_tmr, pdMS_TO_TICKS(seconds * 1000), 0);
        xTimerStart(s_pair_tmr, 0);
    } else { xTimerStop(s_pair_tmr, 0); set_scan_for_role(); }
}

esp_err_t nsn_bt_init(void)
{
    if (s_up) return ESP_OK;
    esp_bt_controller_config_t bc = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t e;
    if ((e = esp_bt_controller_init(&bc)) != ESP_OK) return e;
    if ((e = esp_bt_controller_enable(ESP_BT_MODE_BTDM)) != ESP_OK) return e;
    esp_bluedroid_config_t c = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    if ((e = esp_bluedroid_init_with_cfg(&c)) != ESP_OK) return e;
    if ((e = esp_bluedroid_enable()) != ESP_OK) return e;
    esp_bt_gap_set_device_name(DEV_NAME);
    esp_bt_sp_param_t pt = ESP_BT_SP_IOCAP_MODE; esp_bt_io_cap_t io = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(pt, &io, sizeof io);
    esp_bt_pin_code_t pin = { 0 }; esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_VARIABLE, 0, pin);
    esp_bt_gap_register_callback(gap_cb);
    esp_a2d_register_callback(a2d_cb);
    s_up_sem = xSemaphoreCreateBinary(); s_down_sem = xSemaphoreCreateBinary();
    s_cmd_q = xQueueCreate(6, 1);
    s_raw_q = xQueueCreate(32, sizeof(esp_a2d_audio_buff_t *));
    s_in_rb = xRingbufferCreateWithCaps(IN_RING_BYTES, RINGBUF_TYPE_BYTEBUF, MALLOC_CAP_SPIRAM);
    s_pair_tmr = xTimerCreate("pair", pdMS_TO_TICKS(1000), pdFALSE, NULL, pair_end);
    xTaskCreatePinnedToCore(worker, "bt_work", 6144, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCoreWithCaps(decode_task, "bt_dec", 16 * 1024, NULL, configMAX_PRIORITIES - 4, NULL, 1, MALLOC_CAP_SPIRAM);
    xTaskCreatePinnedToCoreWithCaps(encode_task, "bt_enc", 16 * 1024, NULL, configMAX_PRIORITIES - 4, NULL, 1, MALLOC_CAP_SPIRAM);
    s_up = true;
    const uint8_t *a = esp_bt_dev_get_address();
    ESP_LOGI(TAG, "up, address %02x:%02x:%02x:%02x:%02x:%02x", a[0], a[1], a[2], a[3], a[4], a[5]);
    return nsn_bt_set_mode(g_set.bt_mode ? NSN_BT_HEADPHONES : NSN_BT_SPEAKER);
}

esp_err_t nsn_bt_set_mode(nsn_bt_mode_t m)
{
    if (!s_up) return ESP_ERR_INVALID_STATE;
    g_set.bt_mode = m == NSN_BT_HEADPHONES; nsn_settings_save();
    uint8_t c = m == NSN_BT_HEADPHONES ? CMD_MODE_HEADPHONES : CMD_MODE_SPEAKER;
    return xQueueSend(s_cmd_q, &c, 0) == pdTRUE ? ESP_OK : ESP_FAIL;
}
nsn_bt_mode_t nsn_bt_mode(void) { return g_set.bt_mode ? NSN_BT_HEADPHONES : NSN_BT_SPEAKER; }
bool nsn_bt_mode_ready(void) { return s_mode_ready; }

int nsn_bt_forget_all(void)
{
    int n = esp_bt_gap_get_bond_device_num();
    if (n <= 0) return 0;
    esp_bd_addr_t *list = malloc(n * sizeof(esp_bd_addr_t));
    if (!list) return -1;
    int got = n;
    if (esp_bt_gap_get_bond_device_list(&got, list) == ESP_OK) for (int i = 0; i < got; i++) esp_bt_gap_remove_bond_device(list[i]);
    free(list);
    return got;
}

/* ---- headphones-mode API ---- */
esp_err_t nsn_bt_scan(void)
{
    if (!s_up) return ESP_ERR_INVALID_STATE;
    s_nseen = 0; memset(s_seen, 0, sizeof s_seen);
    return esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 6, 0);
}
bool nsn_bt_scanning(void) { return s_scanning; }
int nsn_bt_scan_results(nsn_bt_dev_t *out, int max) { int n = s_nseen < max ? s_nseen : max; memcpy(out, s_seen, n * sizeof *out); return n; }
esp_err_t nsn_bt_connect(const uint8_t bda[6])
{
    if (!s_up || s_role != ROLE_SOURCE || !s_mode_ready) return ESP_ERR_INVALID_STATE;
    if (s_scanning) esp_bt_gap_cancel_discovery();
    memcpy(s_peer, bda, 6); s_st = ST_CONNECTING;
    return esp_a2d_source_connect(s_peer);
}
esp_err_t nsn_bt_disconnect(void) { return s_role == ROLE_SOURCE && s_st == ST_CONNECTED ? esp_a2d_source_disconnect(s_peer) : ESP_ERR_INVALID_STATE; }
void nsn_bt_set_source(nsn_a2dp_pull_t pull, void *ctx) { s_pull_ctx = ctx; s_pull = pull; }
esp_err_t nsn_bt_start(void) { uint8_t c = CMD_START; return xQueueSend(s_cmd_q, &c, 0) == pdTRUE ? ESP_OK : ESP_FAIL; }
esp_err_t nsn_bt_suspend(void) { uint8_t c = CMD_SUSPEND; return xQueueSend(s_cmd_q, &c, 0) == pdTRUE ? ESP_OK : ESP_FAIL; }
bool nsn_bt_streaming(void) { return s_streaming; }
bool nsn_bt_connected(void) { return s_role == ROLE_SOURCE && s_st == ST_CONNECTED; }
uint32_t nsn_bt_rate(void) { return s_rate; }
void nsn_bt_peer(uint8_t out[6]) { memcpy(out, s_peer, 6); }

void nsn_bt_status(char *out, int n)
{
    static const char *nm[] = { "idle", "connecting", "connected" };
    if (s_role == ROLE_SINK)
        snprintf(out, n, "bt speaker: phone %s%s, %s in-fill=%u frames", s_in_conn ? "connected" : "not connected", s_in_streaming ? " (playing)" : "", nsn_bt_codec_name(), (unsigned)nsn_bt_in_fill());
    else if (s_role == ROLE_SOURCE)
        snprintf(out, n, "bt headphones: %s %02x:%02x:%02x:%02x:%02x:%02x %s %s %luHz", nm[s_st], s_peer[0], s_peer[1], s_peer[2], s_peer[3], s_peer[4], s_peer[5],
                 s_streaming ? "streaming" : "not streaming", nsn_bt_codec_name(), (unsigned long)s_rate);
    else snprintf(out, n, "bt: switching roles%s", s_up ? "" : " (stack down)");
}
