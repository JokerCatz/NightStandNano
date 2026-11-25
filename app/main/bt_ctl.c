// SPDX-License-Identifier: LGPL-2.1-or-later
#include "bt_ctl.h"
#include <string.h>
#include "esp_log.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatt_defs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "cmd.h"
#include "spectrum.h"

static const char *TAG = "ble";
#define APP_ID 0x4e
#define NAME "NightStand Nano"

/* 128-bit UUIDs, little endian on the wire: 7f2a000N-a1c4-4e53-9a31-6e73616e6f00 */
#define UUID128(n) { 0x00, 0x6f, 0x6e, 0x61, 0x73, 0x6e, 0x31, 0x9a, 0x53, 0x4e, 0xc4, 0xa1, (n), 0x00, 0x2a, 0x7f }
static uint8_t u_svc[16] = UUID128(0x01), u_cmd[16] = UUID128(0x02), u_evt[16] = UUID128(0x03), u_spec[16] = UUID128(0x04);

enum { IDX_SVC, IDX_CMD_DECL, IDX_CMD_VAL, IDX_EVT_DECL, IDX_EVT_VAL, IDX_EVT_CCC, IDX_SPEC_DECL, IDX_SPEC_VAL, IDX_SPEC_CCC, IDX_N };

static uint16_t s_h[IDX_N];
static esp_gatt_if_t s_if = ESP_GATT_IF_NONE;
static uint16_t s_conn = 0xffff, s_mtu = 23;
static volatile bool s_evt_on, s_spec_on, s_congested;
static QueueHandle_t s_q;
static uint8_t s_seq;

#ifdef CONFIG_NSN_BLE_OPEN
#define PERM_W ESP_GATT_PERM_WRITE
#define PERM_R ESP_GATT_PERM_READ
#else
#define PERM_W ESP_GATT_PERM_WRITE_ENCRYPTED
#define PERM_R ESP_GATT_PERM_READ_ENCRYPTED
#endif

static const uint16_t primary_uuid = ESP_GATT_UUID_PRI_SERVICE, decl_uuid = ESP_GATT_UUID_CHAR_DECLARE, ccc_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t prop_cmd = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR, prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                     prop_spec = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_READ;
static uint8_t v_cmd[160], v_evt[20], v_spec[2 + NSN_BANDS], v_ccc_evt[2], v_ccc_spec[2];

static const esp_gatts_attr_db_t db[IDX_N] = {
    [IDX_SVC] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)&primary_uuid, ESP_GATT_PERM_READ, 16, 16, u_svc } },
    [IDX_CMD_DECL] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)&decl_uuid, ESP_GATT_PERM_READ, 1, 1, (uint8_t *)&prop_cmd } },
    [IDX_CMD_VAL] = { { ESP_GATT_RSP_BY_APP }, { ESP_UUID_LEN_128, u_cmd, PERM_W, sizeof v_cmd, 0, v_cmd } },
    [IDX_EVT_DECL] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)&decl_uuid, ESP_GATT_PERM_READ, 1, 1, (uint8_t *)&prop_notify } },
    [IDX_EVT_VAL] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_128, u_evt, PERM_R, sizeof v_evt, 0, v_evt } },
    [IDX_EVT_CCC] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)&ccc_uuid, PERM_R | PERM_W, 2, 2, v_ccc_evt } },
    [IDX_SPEC_DECL] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)&decl_uuid, ESP_GATT_PERM_READ, 1, 1, (uint8_t *)&prop_spec } },
    [IDX_SPEC_VAL] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_128, u_spec, PERM_R, sizeof v_spec, sizeof v_spec, v_spec } },
    [IDX_SPEC_CCC] = { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)&ccc_uuid, PERM_R | PERM_W, 2, 2, v_ccc_spec } },
};

/* Advertising: a legacy (31-byte, connectable) advertisement sent through the extended-advertising API, because the build has the
 * BLE 5.0 feature set (Auracast needs it) and IDF does not allow the 4.2 advertising API next to it. Phones and Web Bluetooth see an
 * ordinary advertisement. Advertising set 1: set 0 belongs to the Auracast broadcaster. */
#define ADV_SET 1
static uint8_t s_adv[3 + 18];            /* flags + complete list of 128-bit services */
static uint8_t s_rsp[2 + sizeof NAME - 1];   /* scan response: the name */
static esp_ble_gap_ext_adv_params_t s_advp = {
    .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_LEGACY_IND, .interval_min = 0x40, .interval_max = 0x80, .channel_map = ADV_CHNL_ALL,
    .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, .primary_phy = ESP_BLE_GAP_PHY_1M, .max_skip = 0,
    .secondary_phy = ESP_BLE_GAP_PHY_1M, .sid = ADV_SET, .scan_req_notif = false, .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .tx_power = EXT_ADV_TX_PWR_NO_PREFERENCE,
};
static esp_ble_gap_ext_adv_t s_ext = { ADV_SET, 0, 0 };
static void (*s_gap_hook)(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *);
void nsn_ble_set_gap_hook(void (*hook)(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *)) { s_gap_hook = hook; }

static volatile bool s_quiet;                 /* nsn_ble_quiesce: no advertising, no connection */
static void start_adv(void) { if (!s_quiet) esp_ble_gap_ext_adv_start(1, &s_ext); }

/* one GAP callback for the whole stack: ours are the events of advertising set 1, everything else goes to the hook (the broadcaster) */
static void gap_cb(esp_gap_ble_cb_event_t ev, esp_ble_gap_cb_param_t *p)
{
    switch (ev) {
    case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT:
        if (p->ext_adv_set_params.instance == ADV_SET) { esp_ble_gap_config_ext_adv_data_raw(ADV_SET, sizeof s_adv, s_adv); return; }
        break;
    case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
        if (p->ext_adv_data_set.instance == ADV_SET) { esp_ble_gap_config_ext_scan_rsp_data_raw(ADV_SET, sizeof s_rsp, s_rsp); return; }
        break;
    case ESP_GAP_BLE_EXT_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        if (p->scan_rsp_set.instance == ADV_SET) { start_adv(); return; }
        break;
    case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
        if (p->ext_adv_start.instance_num == 1 && p->ext_adv_start.instance[0] == ADV_SET) {
            ESP_LOGI(TAG, "advertising %s", p->ext_adv_start.status == ESP_BT_STATUS_SUCCESS ? "on" : "FAILED");
            return;
        }
        break;
    default: break;
    }
    if (s_gap_hook) s_gap_hook(ev, p);
}

/* ---- notifications ---- */
static void notify(uint16_t handle, const uint8_t *d, uint16_t n)
{
    if (s_if == ESP_GATT_IF_NONE || s_conn == 0xffff) return;
    esp_ble_gatts_send_indicate(s_if, s_conn, handle, n, (uint8_t *)d, false);
}

static void ble_out(void *ctx, const char *text)
{
    if (!s_evt_on) return;
    char line[220]; int n = snprintf(line, sizeof line, "%s\n", text);
    if (n > (int)sizeof line - 1) n = sizeof line - 1;
    int chunk = s_mtu > 23 ? s_mtu - 3 : 20;
    for (int off = 0; off < n; off += chunk) {
        int len = n - off < chunk ? n - off : chunk;
        for (int i = 0; i < 100 && s_congested; i++) vTaskDelay(pdMS_TO_TICKS(5));
        notify(s_h[IDX_EVT_VAL], (const uint8_t *)line + off, len);
        vTaskDelay(pdMS_TO_TICKS(6));            /* the stack queues a limited number of notifications */
    }
}

static void cmd_task(void *arg)
{
    char line[160];
    for (;;) if (xQueueReceive(s_q, line, portMAX_DELAY) == pdTRUE) nsn_cmd_exec(line, ble_out, NULL);
}

static void spec_frame(const uint8_t *bands)
{
    if (!s_spec_on || s_conn == 0xffff || s_congested) return;
    uint8_t f[2 + NSN_BANDS];
    f[0] = s_seq++; f[1] = NSN_BANDS; memcpy(f + 2, bands, NSN_BANDS);
    uint16_t n = sizeof f; if (s_mtu > 23 && n > s_mtu - 3) n = s_mtu - 3; else if (s_mtu <= 23 && n > 20) n = 20;
    notify(s_h[IDX_SPEC_VAL], f, n);
}

static void gatts_cb(esp_gatts_cb_event_t ev, esp_gatt_if_t gif, esp_ble_gatts_cb_param_t *p)
{
    switch (ev) {
    case ESP_GATTS_REG_EVT:
        s_if = gif;
        esp_ble_gap_set_device_name(NAME);
        s_adv[0] = 2; s_adv[1] = 0x01; s_adv[2] = 0x06;                       /* flags: general discoverable (dual-mode device) */
        s_adv[3] = 17; s_adv[4] = 0x07; memcpy(s_adv + 5, u_svc, 16);
        s_rsp[0] = sizeof NAME; s_rsp[1] = 0x09; memcpy(s_rsp + 2, NAME, sizeof NAME - 1);
        esp_ble_gap_ext_adv_set_params(ADV_SET, &s_advp);
        esp_ble_gatts_create_attr_tab(db, gif, IDX_N, 0);
        break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        if (p->add_attr_tab.status == ESP_GATT_OK && p->add_attr_tab.num_handle == IDX_N) {
            memcpy(s_h, p->add_attr_tab.handles, sizeof s_h);
            esp_ble_gatts_start_service(s_h[IDX_SVC]);
            ESP_LOGI(TAG, "GATT service up (%d attributes)", IDX_N);
        } else ESP_LOGE(TAG, "attr table failed: status %d n=%d", p->add_attr_tab.status, p->add_attr_tab.num_handle);
        break;
    case ESP_GATTS_CONNECT_EVT:
        s_conn = p->connect.conn_id; s_mtu = 23; s_evt_on = s_spec_on = false;
        ESP_LOGI(TAG, "connected, conn %d", s_conn);
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "disconnected (reason 0x%x)", p->disconnect.reason);
        s_conn = 0xffff; s_evt_on = s_spec_on = s_congested = false;
        start_adv();
        break;
    case ESP_GATTS_MTU_EVT:
        s_mtu = p->mtu.mtu; ESP_LOGI(TAG, "mtu %d", s_mtu);
        break;
    case ESP_GATTS_CONGEST_EVT:
        s_congested = p->congest.congested;
        break;
    case ESP_GATTS_WRITE_EVT: {
        uint16_t h = p->write.handle;
        if (h == s_h[IDX_CMD_VAL]) {
            char line[160]; int n = p->write.len < 159 ? p->write.len : 159;
            memcpy(line, p->write.value, n); line[n] = 0;
            if (xQueueSend(s_q, line, 0) != pdTRUE) ESP_LOGW(TAG, "command queue full");
        } else if (h == s_h[IDX_EVT_CCC] && p->write.len == 2) {
            s_evt_on = p->write.value[0] & 1;
        } else if (h == s_h[IDX_SPEC_CCC] && p->write.len == 2) {
            s_spec_on = p->write.value[0] & 1;
            ESP_LOGI(TAG, "spectrum notifications %s", s_spec_on ? "on" : "off");
        }
        if (p->write.need_rsp) esp_ble_gatts_send_response(gif, p->write.conn_id, p->write.trans_id, ESP_GATT_OK, NULL);
        break;
    }
    default: break;
    }
}

esp_err_t nsn_ble_start(void)
{
    s_q = xQueueCreate(4, 160);
    xTaskCreate(cmd_task, "ble_cmd", 6144, NULL, 4, NULL);
    esp_ble_gatt_set_local_mtu(247);
    esp_ble_gap_register_callback(gap_cb);
    esp_ble_gatts_register_callback(gatts_cb);
    nsn_spec_start(spec_frame);
    return esp_ble_gatts_app_register(APP_ID);
}

/* No client and no advertising for a moment (true), then back to normal (false). LE Audio must be initialised while no GATT connection
 * exists: its start-up registers a GATT app whose connect callback posts to a queue that does not exist yet (IDF v6.1 esp_ble_iso,
 * gatts_connect_event_handler -> bt_le_iso_task_post; it asserted on the board 2026-11-25). The client simply reconnects afterwards. */
void nsn_ble_quiesce(bool on)
{
    if (!on) { s_quiet = false; start_adv(); return; }
    s_quiet = true;
    uint8_t inst = ADV_SET;
    esp_ble_gap_ext_adv_stop(1, &inst);
    if (s_conn != 0xffff) esp_ble_gatts_close(s_if, s_conn);
    for (int i = 0; i < 300 && s_conn != 0xffff; i++) vTaskDelay(pdMS_TO_TICKS(10));
    vTaskDelay(pdMS_TO_TICKS(200));
}
bool nsn_ble_connected(void) { return s_conn != 0xffff; }
