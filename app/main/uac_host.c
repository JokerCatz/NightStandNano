// SPDX-License-Identifier: LGPL-2.1-or-later
#include "uac_host.h"
#include "keys.h"
#include "ratetest.h"
#include <string.h>
#include "esp_log.h"
#include "usb/usb_host.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

static const char *TAG = "uac";

#define ISO_PKTS      16            /* packets per transfer: 16 microframes = 2 ms at high speed */
#define ISO_XFERS     6             /* transfers in flight: ~12 ms of audio queued in the host */
#define UAC_SET_CUR   0x01
#define UAC_RANGE     0x02
#define CS_SAM_FREQ   0x01

typedef struct { uint8_t ifn, alt, ep; uint16_t mps; uint8_t channels, subslot, bits; } as_alt_t;

static usb_host_client_handle_t s_cli;
static usb_device_handle_t s_dev;
static uint8_t s_addr;
static QueueHandle_t s_q;                       /* device addresses (>0 attach, 0 = gone) from the client callback */
static SemaphoreHandle_t s_ctl_done;
static nsn_uac_info_t s_info;
static as_alt_t s_alt;
static uint8_t s_ac_if = 0, s_as_if = 1, s_clock = 0;
static volatile bool s_streaming;
static usb_transfer_t *s_xfer[ISO_XFERS];
static int s_inflight;
static nsn_pcm_pull_t s_pull; static void *s_pull_ctx;
static uint32_t s_rate, s_acc;
static volatile bool s_stop;
static volatile uint32_t s_done, s_bad, s_frames;   /* ISO transfers completed / failed; frames of the completed ones (the measured data rate) */
static struct { uint32_t mn, mx, step; } s_rng[16];   /* the clock's advertised sample rates (GET_RANGE) */
static int s_nrng;
static volatile bool s_last_set_ok; static volatile uint32_t s_last_clock;   /* the latest nsn_uac_play: SET_CUR accepted, the clock read back */
static int s_hid_if = -1, s_hid_rlen; static uint8_t s_hid_ep;
static usb_transfer_t *s_hid_xfer; static volatile bool s_hid_live, s_hid_busy; static uint8_t s_hid_bits;
static void hid_start(void);   /* the dongle's HID interface (headset line-control buttons), -1 = none */

/* ---- control transfers (synchronous, from the uac task, never from the client task) ---- */
static void ctl_cb(usb_transfer_t *t) { xSemaphoreGive(s_ctl_done); }

static esp_err_t ctl(uint8_t reqtype, uint8_t req, uint16_t val, uint16_t idx, void *data, uint16_t len)
{
    usb_transfer_t *t;
    esp_err_t e = usb_host_transfer_alloc(sizeof(usb_setup_packet_t) + len, 0, &t);
    if (e != ESP_OK) return e;
    usb_setup_packet_t *sp = (usb_setup_packet_t *)t->data_buffer;
    *sp = (usb_setup_packet_t){ reqtype, req, val, idx, len };
    if (!(reqtype & 0x80) && len) memcpy(t->data_buffer + sizeof *sp, data, len);
    t->num_bytes = sizeof *sp + len;
    t->device_handle = s_dev; t->bEndpointAddress = 0; t->callback = ctl_cb; t->context = NULL; t->timeout_ms = 1000;
    xSemaphoreTake(s_ctl_done, 0);
    e = usb_host_transfer_submit_control(s_cli, t);
    if (e == ESP_OK) {
        if (xSemaphoreTake(s_ctl_done, pdMS_TO_TICKS(1500)) != pdTRUE) e = ESP_ERR_TIMEOUT;
        else if (t->status != USB_TRANSFER_STATUS_COMPLETED) e = ESP_FAIL;
        else if ((reqtype & 0x80) && len) memcpy(data, t->data_buffer + sizeof *sp, t->actual_num_bytes - sizeof *sp < len ? t->actual_num_bytes - sizeof *sp : len);
    }
    usb_host_transfer_free(t);
    return e;
}

/* ---- descriptor walk ---- */
static void parse_config(const uint8_t *p, size_t total)
{
    uint8_t cur_if = 0xff, cur_alt = 0, cur_cls = 0, cur_sub = 0, cur_neps = 0;
    as_alt_t cand = { 0 }; bool have_cand = false, best_ok = false;
    uint8_t usb_in_clock = 0;
    as_alt_t best = { 0 };
    for (size_t o = 0; o + 2 <= total && p[o] >= 2; o += p[o]) {
        const uint8_t *d = p + o; uint8_t len = d[0], type = d[1];
        if (type == 0x21 && len >= 9 && cur_cls == 0x03) { s_hid_if = cur_if; s_hid_rlen = d[7] | d[8] << 8; ESP_LOGI(TAG, "  HID descriptor on interface %d: report descriptor %d bytes", cur_if, s_hid_rlen); }
        if (type == 0x05 && len >= 7 && cur_cls == 0x03 && (d[2] & 0x80)) s_hid_ep = d[2];
        if (type == 0x05 && len >= 7 && cur_cls == 0x03) ESP_LOGI(TAG, "  HID endpoint 0x%02x attr 0x%02x mps %d interval %d", d[2], d[3], d[4] | d[5] << 8, d[6]);
        if (type == 0x04 && len >= 9) {                         /* interface */
            if (d[3] == 0) ESP_LOGI(TAG, "interface %d: class 0x%02x sub 0x%02x proto 0x%02x, %d endpoint(s)", d[2], d[5], d[6], d[7], d[4]);   /* what the device offers (HID = 0x03: line-control buttons) */
            if (have_cand && cand.ep && cand.channels) {         /* close the previous alt */
                bool pref = cand.subslot == 2 && cand.bits == 16 && cand.channels == 2;
                if (!best_ok || (pref && !(best.subslot == 2 && best.bits == 16))) { best = cand; best_ok = true; }
            }
            cur_if = d[2]; cur_alt = d[3]; cur_neps = d[4]; cur_cls = d[5]; cur_sub = d[6];
            have_cand = false;
            if (cur_cls == 1 && cur_sub == 2 && cur_neps == 1) { cand = (as_alt_t){ .ifn = cur_if, .alt = cur_alt }; have_cand = true; }
            if (cur_cls == 1 && cur_sub == 1) s_ac_if = cur_if;
        } else if (type == 0x24 && cur_cls == 1 && len >= 4) {   /* class-specific */
            uint8_t sub = d[2];
            if (cur_sub == 1 && sub == 0x02 && len >= 8 && (d[4] | d[5] << 8) == 0x0101) usb_in_clock = d[7];   /* input terminal, USB streaming */
            else if (cur_sub == 1 && sub == 0x0a && !s_clock) s_clock = d[3];                                      /* clock source */
            else if (cur_sub == 2 && have_cand && sub == 0x01 && len >= 11) cand.channels = d[10];                 /* AS_GENERAL */
            else if (cur_sub == 2 && have_cand && sub == 0x02 && len >= 6) { cand.subslot = d[4]; cand.bits = d[5]; }  /* FORMAT_TYPE I */
        } else if (type == 0x05 && have_cand && len >= 7 && !(d[2] & 0x80)) {   /* OUT endpoint of the candidate alt */
            cand.ep = d[2]; cand.mps = d[4] | (d[5] & 0x07) << 8;
        }
    }
    if (have_cand && cand.ep && cand.channels) {
        bool pref = cand.subslot == 2 && cand.bits == 16 && cand.channels == 2;
        if (!best_ok || (pref && !(best.subslot == 2 && best.bits == 16))) { best = cand; best_ok = true; }
    }
    if (usb_in_clock) s_clock = usb_in_clock;
    if (best_ok) { s_alt = best; s_as_if = best.ifn; s_info.channels = best.channels; s_info.subslot = best.subslot; s_info.bits = best.bits; }
    s_info.present = best_ok;
    ESP_LOGI(TAG, "playback alt=%d ep=0x%02x mps=%d %dch %d-bit in %d-byte slots, clock id %d, AC if %d AS if %d",
             s_alt.alt, s_alt.ep, s_alt.mps, s_alt.channels, s_alt.bits, s_alt.subslot, s_clock, s_ac_if, s_as_if);
}

static void read_rate_range(void)
{
    uint8_t buf[2 + 12 * 16] = { 0 };
    esp_err_t e = ctl(0xA1, UAC_RANGE, CS_SAM_FREQ << 8, (s_clock << 8) | s_ac_if, buf, sizeof buf);
    if (e != ESP_OK) { ESP_LOGW(TAG, "clock GET_RANGE failed: %s", esp_err_to_name(e)); return; }
    int n = buf[0] | buf[1] << 8; if (n > 16) n = 16;
    s_nrng = 0;
    uint32_t lo = 0xffffffff, hi = 0;
    for (int i = 0; i < n; i++) {
        const uint8_t *r = buf + 2 + i * 12;
        uint32_t mn = r[0] | r[1] << 8 | r[2] << 16 | (uint32_t)r[3] << 24, mx = r[4] | r[5] << 8 | r[6] << 16 | (uint32_t)r[7] << 24;
        ESP_LOGI(TAG, "  rate range %lu..%lu step %lu", (unsigned long)mn, (unsigned long)mx, (unsigned long)(r[8] | r[9] << 8));
        s_rng[s_nrng].mn = mn; s_rng[s_nrng].mx = mx; s_rng[s_nrng].step = r[8] | r[9] << 8 | r[10] << 16 | (uint32_t)r[11] << 24; s_nrng++;
        if (mn < lo) lo = mn;
        if (mx > hi) hi = mx;
    }
    s_info.rate_min = n ? lo : 0; s_info.rate_max = hi;
}

/* ---- ISO streaming ---- */
static void fill_and_submit(usb_transfer_t *t)
{
    const int ch = s_alt.channels, slot = s_alt.subslot;
    int total = 0, total_frames = 0; int16_t tmp[8 * 2];          /* a packet is at most ~48 frames at 384 kHz; 6 at 48 kHz */
    static int16_t big[64 * 2];
    uint8_t *out = t->data_buffer;
    for (int i = 0; i < ISO_PKTS; i++) {
        s_acc += s_rate;
        int frames = s_acc / 8000; s_acc %= 8000;
        if (frames > 64) frames = 64;
        int16_t *src = frames <= 8 ? tmp : big;
        size_t got = s_pull ? s_pull(src, frames, s_pull_ctx) : 0;
        if (got < (size_t)frames) memset(src + got * 2, 0, (frames - got) * 2 * sizeof(int16_t));
        int bytes = frames * ch * slot;
        for (int f = 0; f < frames; f++) {            /* stereo S16 in -> the DAC's slot format */
            for (int c = 0; c < ch; c++) {
                int16_t v = src[f * 2 + (c & 1)];
                if (slot == 2) { memcpy(out, &v, 2); out += 2; }
                else if (slot == 3) { out[0] = 0; out[1] = v & 0xff; out[2] = v >> 8; out += 3; }
                else { out[0] = out[1] = 0; out[2] = v & 0xff; out[3] = v >> 8; out += 4; }
            }
        }
        t->isoc_packet_desc[i].num_bytes = bytes;
        total += bytes; total_frames += frames;
    }
    t->num_bytes = total;
    t->context = (void *)(uintptr_t)total_frames;           /* iso_cb counts them when the transfer completes */
    if (usb_host_transfer_submit(t) == ESP_OK) s_inflight++;
}

static void iso_cb(usb_transfer_t *t)
{
    s_inflight--;
    s_done++;
    if (t->status != USB_TRANSFER_STATUS_COMPLETED) s_bad++;
    else s_frames += (uint32_t)(uintptr_t)t->context;
    if (s_stop || t->status == USB_TRANSFER_STATUS_NO_DEVICE || t->status == USB_TRANSFER_STATUS_CANCELED) return;
    if (t->status != USB_TRANSFER_STATUS_COMPLETED) ESP_LOGW(TAG, "iso status %d", t->status);
    fill_and_submit(t);
}

esp_err_t nsn_uac_play(uint32_t rate, nsn_pcm_pull_t pull, void *ctx)
{
    if (!s_info.present || !s_dev) return ESP_ERR_INVALID_STATE;
    if (s_streaming) return ESP_ERR_INVALID_STATE;
    s_last_set_ok = false; s_last_clock = 0;
    if ((rate + 7999) / 8000 * s_alt.channels * s_alt.subslot > s_alt.mps) {          /* a microframe's worth of this rate does not fit the endpoint */
        ESP_LOGE(TAG, "%lu Hz needs %lu bytes per microframe, the endpoint takes %d", (unsigned long)rate, (unsigned long)((rate + 7999) / 8000 * s_alt.channels * s_alt.subslot), s_alt.mps);
        return ESP_ERR_NOT_SUPPORTED;
    }
    uint8_t r[4] = { rate, rate >> 8, rate >> 16, rate >> 24 };
    esp_err_t e = ctl(0x21, UAC_SET_CUR, CS_SAM_FREQ << 8, (s_clock << 8) | s_ac_if, r, 4);
    if (e != ESP_OK) { ESP_LOGE(TAG, "set rate %lu failed: %s", (unsigned long)rate, esp_err_to_name(e)); return e; }
    s_last_set_ok = true;
    uint8_t chk[4] = { 0 };
    if (ctl(0xA1, UAC_SET_CUR, CS_SAM_FREQ << 8, (s_clock << 8) | s_ac_if, chk, 4) == ESP_OK) {
        s_last_clock = chk[0] | chk[1] << 8 | chk[2] << 16 | (uint32_t)chk[3] << 24;
        ESP_LOGI(TAG, "clock now %lu Hz", (unsigned long)s_last_clock);
    }
    e = usb_host_interface_claim(s_cli, s_dev, s_as_if, s_alt.alt);
    if (e != ESP_OK) { ESP_LOGE(TAG, "claim AS interface failed: %s", esp_err_to_name(e)); return e; }
    s_pull = pull; s_pull_ctx = ctx; s_rate = rate; s_acc = 0; s_stop = false; s_inflight = 0;
    for (int i = 0; i < ISO_XFERS; i++) {
        if (usb_host_transfer_alloc(ISO_PKTS * ((rate / 8000 + 1) * s_alt.channels * s_alt.subslot), ISO_PKTS, &s_xfer[i]) != ESP_OK) { e = ESP_ERR_NO_MEM; break; }
        usb_transfer_t *t = s_xfer[i];
        t->device_handle = s_dev; t->bEndpointAddress = s_alt.ep; t->callback = iso_cb; t->context = NULL; t->timeout_ms = 0;
    }
    if (e != ESP_OK) { nsn_uac_stop(); return e; }
    s_streaming = true;
    for (int i = 0; i < ISO_XFERS; i++) fill_and_submit(s_xfer[i]);
    ESP_LOGI(TAG, "streaming %lu Hz, %d transfers in flight", (unsigned long)rate, s_inflight);
    return ESP_OK;
}

void nsn_uac_stop(void)
{
    s_stop = true;
    if (s_dev) { usb_host_endpoint_halt(s_dev, s_alt.ep); usb_host_endpoint_flush(s_dev, s_alt.ep); }
    for (int i = 0; i < 100 && s_inflight > 0; i++) vTaskDelay(pdMS_TO_TICKS(5));
    for (int i = 0; i < ISO_XFERS; i++) if (s_xfer[i]) { usb_host_transfer_free(s_xfer[i]); s_xfer[i] = NULL; }
    if (s_dev && s_streaming) { usb_host_endpoint_clear(s_dev, s_alt.ep); usb_host_interface_release(s_cli, s_dev, s_as_if); }
    s_streaming = false;
}

bool nsn_uac_playing(void) { return s_streaming; }
void nsn_uac_stats(uint32_t *done, uint32_t *bad) { *done = s_done; *bad = s_bad; }
uint32_t nsn_uac_frames(void) { return s_frames; }
void nsn_uac_last_clock(bool *set_ok, uint32_t *readback) { *set_ok = s_last_set_ok; *readback = s_last_clock; }
bool nsn_uac_rates_known(void) { return s_nrng > 0; }
bool nsn_uac_rate_advertised(uint32_t r)
{
    for (int i = 0; i < s_nrng; i++)
        if (r >= s_rng[i].mn && r <= s_rng[i].mx && (r == s_rng[i].mn || r == s_rng[i].mx || (s_rng[i].step && (r - s_rng[i].mn) % s_rng[i].step == 0))) return true;
    return false;
}
void nsn_uac_info(nsn_uac_info_t *out) { *out = s_info; }

/* ---- host plumbing ---- */
static void lib_task(void *arg)
{
    for (;;) {
        uint32_t flags;
        usb_host_lib_handle_events(portMAX_DELAY, &flags);
        if (flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) usb_host_device_free_all();
    }
}

static void client_cb(const usb_host_client_event_msg_t *m, void *arg)
{
    uint8_t v = m->event == USB_HOST_CLIENT_EVENT_NEW_DEV ? m->new_dev.address : 0;
    xQueueSend(s_q, &v, 0);
}

static void client_task(void *arg)
{
    for (;;) usb_host_client_handle_events(s_cli, portMAX_DELAY);
}

static void on_attach(uint8_t addr)
{
    if (usb_host_device_open(s_cli, addr, &s_dev) != ESP_OK) { ESP_LOGE(TAG, "open dev %d failed", addr); return; }
    s_addr = addr;
    memset(&s_info, 0, sizeof s_info); s_clock = 0;
    const usb_device_desc_t *dd; usb_device_info_t di; const usb_config_desc_t *cd;
    usb_host_get_device_descriptor(s_dev, &dd); usb_host_device_info(s_dev, &di);
    s_info.vid = dd->idVendor; s_info.pid = dd->idProduct;
    if (di.str_desc_product) for (int i = 0; i < 47 && i < (di.str_desc_product->bLength - 2) / 2; i++) s_info.product[i] = di.str_desc_product->wData[i] < 128 ? di.str_desc_product->wData[i] : '?';
    ESP_LOGI(TAG, "device %04x:%04x '%s'", s_info.vid, s_info.pid, s_info.product);
    s_hid_if = -1; s_hid_rlen = 0; s_hid_ep = 0;
    if (usb_host_get_active_config_descriptor(s_dev, &cd) == ESP_OK) parse_config((const uint8_t *)cd, cd->wTotalLength);
    if (s_hid_if >= 0 && s_hid_rlen > 0 && s_hid_rlen <= 256) {          /* what the buttons report: dump the HID report descriptor */
        static uint8_t rd[256]; char hex[3 * 32 + 1];
        if (ctl(0x81, 0x06, 0x2200, (uint16_t)s_hid_if, rd, (uint16_t)s_hid_rlen) == ESP_OK)
            for (int o = 0; o < s_hid_rlen; o += 32) {
                int k = 0;
                for (int i = o; i < o + 32 && i < s_hid_rlen; i++) k += sprintf(hex + k, "%02x ", rd[i]);
                ESP_LOGI(TAG, "  HID report descriptor: %s", hex);
            }
        else ESP_LOGW(TAG, "  HID report descriptor: read failed");
    }
    s_nrng = 0;
    if (s_info.present) read_rate_range();
    nsn_ratetest_attach(s_info.vid, s_info.pid);
    hid_start();
}

/* ---- headset buttons: the dongle's HID interrupt endpoint. Report id 2, one byte: bit0 volume up, bit1 volume down, bit2 play/pause,
 * bit3 voice command (read from the CX31993's report descriptor; the layout Android's USB headset spec asks for) ---- */
static void hid_cb(usb_transfer_t *t)
{
    if (t->status == USB_TRANSFER_STATUS_COMPLETED && s_hid_live) {
        if (t->actual_num_bytes >= 2 && t->data_buffer[0] == 2) {
            uint8_t b = t->data_buffer[1], ch = b ^ s_hid_bits;
            s_hid_bits = b;
            if (ch & 1) nsn_keys_event(NSN_KEY_PLUS, b & 1);
            if (ch & 2) nsn_keys_event(NSN_KEY_MINUS, b & 2);
            if (ch & 4) nsn_keys_event(NSN_KEY_CENTER, b & 4);
        }
        if (usb_host_transfer_submit(t) == ESP_OK) return;
    }
    s_hid_busy = false;                                    /* device gone, cancelled or failed: the reader stops */
}
static void hid_start(void)
{
    if (s_hid_if < 0 || !s_hid_ep) return;
    if (usb_host_interface_claim(s_cli, s_dev, (uint8_t)s_hid_if, 0) != ESP_OK) { ESP_LOGW(TAG, "HID interface claim failed"); s_hid_if = -1; return; }
    if (usb_host_transfer_alloc(64, 0, &s_hid_xfer) != ESP_OK) { usb_host_interface_release(s_cli, s_dev, (uint8_t)s_hid_if); s_hid_if = -1; return; }
    s_hid_xfer->device_handle = s_dev; s_hid_xfer->bEndpointAddress = s_hid_ep; s_hid_xfer->callback = hid_cb; s_hid_xfer->num_bytes = 64;
    s_hid_bits = 0; s_hid_live = true; s_hid_busy = true;
    if (usb_host_transfer_submit(s_hid_xfer) != ESP_OK) { s_hid_busy = false; s_hid_live = false; ESP_LOGW(TAG, "HID read failed to start"); }
    else ESP_LOGI(TAG, "headset buttons: listening on endpoint 0x%02x", s_hid_ep);
}
static void hid_stop(void)
{
    if (!s_hid_xfer) return;
    s_hid_live = false;
    for (int i = 0; i < 50 && s_hid_busy; i++) vTaskDelay(pdMS_TO_TICKS(10));
    for (int k = 0; k < NSN_KEY_N; k++) nsn_keys_event(k, false);            /* nothing stays pressed */
    if (s_dev && s_hid_if >= 0) usb_host_interface_release(s_cli, s_dev, (uint8_t)s_hid_if);
    if (!s_hid_busy) usb_host_transfer_free(s_hid_xfer);
    s_hid_xfer = NULL; s_hid_if = -1;
}

static void on_gone(void)
{
    hid_stop();
    if (s_streaming) nsn_uac_stop();
    if (s_dev) { usb_host_device_close(s_cli, s_dev); s_dev = NULL; }
    memset(&s_info, 0, sizeof s_info); s_nrng = 0;
    ESP_LOGI(TAG, "device gone");
}

static void uac_task(void *arg)
{
    uint8_t v;
    for (;;) {
        if (xQueueReceive(s_q, &v, portMAX_DELAY) != pdTRUE) continue;
        if (v) { if (s_dev) on_gone(); on_attach(v); } else on_gone();
    }
}

esp_err_t nsn_uac_start(void)
{
    s_q = xQueueCreate(4, 1); s_ctl_done = xSemaphoreCreateBinary();
    const usb_host_config_t hc = { .intr_flags = ESP_INTR_FLAG_LEVEL1 };
    esp_err_t e = usb_host_install(&hc);
    if (e != ESP_OK) return e;
    const usb_host_client_config_t cc = { .is_synchronous = false, .max_num_event_msg = 5, .async = { .client_event_callback = client_cb } };
    e = usb_host_client_register(&cc, &s_cli);
    if (e != ESP_OK) return e;
    xTaskCreate(lib_task, "usb_lib", 4096, NULL, 10, NULL);
    xTaskCreate(client_task, "usb_cli", 4096, NULL, 9, NULL);
    xTaskCreate(uac_task, "uac", 6144, NULL, 5, NULL);
    return ESP_OK;
}
bool nsn_uac_info_present(void) { return s_info.present && s_dev != NULL; }
