/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: BLE GATT-Server adapter for the gateway. \n
 *
 * Initialization flow (mirrors ble_uuid_server):
 *   gateway task: gw_ble_adapter_init()
 *     -> registers BLE device-manager, GAP, and GATTS callbacks
 *     -> enable_ble()   (waits until BLE stack is ready)
 *     -> gatts_register_server()
 *     -> gatts_add_service()  (rest is driven by add_service_cbk chain)
 *     -> starts BLE advertising
 */
#include "common_def.h"
#include "securec.h"
#include "soc_osal.h"
#include "errcode.h"
#include "bts_def.h"
#include "bts_device_manager.h"
#include "bts_le_gap.h"
#include "bts_gatt_stru.h"
#include "bts_gatt_server.h"
#include "gateway_service.h"
#include "gw_ble_adapter.h"

/* -------------------------------------------------------------------------
 * BLE advertising constants
 * These are defined in sample-private headers (ble_server_adv.h) rather than
 * the common SDK headers, so we define the numeric values here directly.
 * ---------------------------------------------------------------------- */
#define BTH_GAP_BLE_ADV_HANDLE_DEFAULT              0x01
#define BTH_GAP_BLE_ADV_FOREVER_DURATION            0
#define BLE_ADV_CHANNEL_MAP_CH_DEFAULT              0x07
#define BLE_ADV_TYPE_CONNECTABLE_UNDIRECTED         0x00
#define BLE_ADV_FILTER_POLICY_SCAN_ANY_CONNECT_ANY  0x00
#define BLE_PUBLIC_DEVICE_ADDRESS                   0x00

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */
#define GW_BLE_LOG                  "[gw_ble] "
#define GW_BLE_UUID_SERVICE         0xAA01
#define GW_BLE_UUID_CHAR            0xEFEF  /* matches ble_uart_client BLE_UART_CHARACTER_CLIENT_UUID_TX */
#define GW_BLE_UUID_CCC             0x2902
#define GW_BLE_ADV_MIN_INTERVAL     0x20
#define GW_BLE_ADV_MAX_INTERVAL     0x30
#define GW_BLE_OCTET_BIT_LEN        8
#define GW_BLE_UUID_LEN             2
#define GW_BLE_ENABLE_POLL_MS       200
#define GW_BLE_STATUS_OK            1
#define GW_BLE_ADV_DATA_MAX_LEN     251

static uint8_t g_gw_ble_app_uuid[] = {0x56, 0x79};

/* Forward declarations for adv helpers used in callbacks */
static void gw_ble_set_adv_data(void);
static void gw_ble_start_adv(void);

/* -------------------------------------------------------------------------
 * Server state
 * ---------------------------------------------------------------------- */
static uint8_t  g_gw_ble_server_id            = 0;
static uint16_t g_gw_ble_notification_att_hdl = 0;
static uint16_t g_gw_ble_conn_hdl             = 0;
static volatile uint8_t g_gw_ble_status       = 0;

/* -------------------------------------------------------------------------
 * UUID helper
 * ---------------------------------------------------------------------- */
static void gw_ble_uuid_set(uint16_t uuid_data, bt_uuid_t *out)
{
    char uuid[GW_BLE_UUID_LEN] = {
        (uint8_t)(uuid_data >> GW_BLE_OCTET_BIT_LEN),
        (uint8_t)(uuid_data)
    };
    out->uuid_len = GW_BLE_UUID_LEN;
    (void)memcpy_s(out->uuid, out->uuid_len, uuid, GW_BLE_UUID_LEN);
}

/* -------------------------------------------------------------------------
 * GATTS service-build callback chain
 * ---------------------------------------------------------------------- */
static void gw_ble_service_add_cbk(uint8_t server_id, bt_uuid_t *uuid,
                                    uint16_t handle, errcode_t status)
{
    unused(status);
    bt_uuid_t svc_uuid = {0};
    gw_ble_uuid_set(GW_BLE_UUID_SERVICE, &svc_uuid);

    /* Only react to our own service UUID */
    if (uuid->uuid_len != svc_uuid.uuid_len ||
        memcmp(uuid->uuid, svc_uuid.uuid, uuid->uuid_len) != 0) {
        return;
    }
    osal_printk("%s service added srv_hdl=%d\r\n", GW_BLE_LOG, handle);

    /* Add notify+write characteristic */
    bt_uuid_t char_uuid = {0};
    gw_ble_uuid_set(GW_BLE_UUID_CHAR, &char_uuid);
    uint8_t init_val[] = {0x00, 0x00};
    gatts_add_chara_info_t chara = {0};
    chara.chara_uuid   = char_uuid;
    chara.properties   = GATT_CHARACTER_PROPERTY_BIT_NOTIFY |
                         GATT_CHARACTER_PROPERTY_BIT_WRITE  |
                         GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP;
    chara.permissions  = GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE;
    chara.value_len    = sizeof(init_val);
    chara.value        = init_val;
    gatts_add_characteristic(server_id, handle, &chara);
}

static void gw_ble_char_add_cbk(uint8_t server_id, bt_uuid_t *uuid,
                                 uint16_t service_handle,
                                 gatts_add_character_result_t *result,
                                 errcode_t status)
{
    unused(uuid);
    unused(status);
    osal_printk("%s char added char_hdl=0x%x val_hdl=0x%x\r\n",
                GW_BLE_LOG, result->handle, result->value_handle);
    g_gw_ble_notification_att_hdl = result->value_handle;

    /* Add CCC descriptor */
    bt_uuid_t ccc_uuid = {0};
    gw_ble_uuid_set(GW_BLE_UUID_CCC, &ccc_uuid);
    uint8_t ccc_val[] = {0x00, 0x00};
    gatts_add_desc_info_t desc = {0};
    desc.desc_uuid   = ccc_uuid;
    desc.permissions = GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE;
    desc.value_len   = sizeof(ccc_val);
    desc.value       = ccc_val;
    gatts_add_descriptor(server_id, service_handle, &desc);
}

static void gw_ble_desc_add_cbk(uint8_t server_id, bt_uuid_t *uuid,
                                 uint16_t service_handle, uint16_t handle,
                                 errcode_t status)
{
    unused(uuid);
    unused(handle);
    unused(status);
    osal_printk("%s descriptor added, starting service\r\n", GW_BLE_LOG);
    gatts_start_service(server_id, service_handle);
}

static void gw_ble_service_start_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    osal_printk("%s service started server_id=%d hdl=%d status=%d\r\n",
                GW_BLE_LOG, server_id, handle, status);
}

/* -------------------------------------------------------------------------
 * GATTS read/write callbacks -> forward data to the service queue
 * ---------------------------------------------------------------------- */
static void gw_ble_write_cbk(uint8_t server_id, uint16_t conn_id,
                              gatts_req_write_cb_t *write_cb, errcode_t status)
{
    unused(server_id);
    unused(status);
    if (write_cb == NULL || write_cb->value == NULL || write_cb->length == 0) {
        return;
    }
    uint16_t copy_len = write_cb->length;
    if (copy_len > GW_DATA_MAX_LEN) {
        copy_len = GW_DATA_MAX_LEN;
    }
    gw_msg_t msg = {0};
    msg.type          = GW_MSG_DATA_RECEIVED;
    msg.protocol      = GW_PROTO_BLE;
    msg.proto_conn_id = conn_id;
    msg.data_len      = copy_len;
    if (memcpy_s(msg.data, GW_DATA_MAX_LEN, write_cb->value, copy_len) != EOK) {
        osal_printk("%s write cbk memcpy fail\r\n", GW_BLE_LOG);
        return;
    }
    gw_service_post_msg(&msg);
}

static void gw_ble_read_cbk(uint8_t server_id, uint16_t conn_id,
                             gatts_req_read_cb_t *read_cb, errcode_t status)
{
    unused(server_id);
    unused(conn_id);
    unused(read_cb);
    unused(status);
}

static void gw_ble_mtu_cbk(uint8_t server_id, uint16_t conn_id,
                            uint16_t mtu_size, errcode_t status)
{
    osal_printk("%s mtu changed server_id=%d conn_id=%d mtu=%d status=0x%x\r\n",
                GW_BLE_LOG, server_id, conn_id, mtu_size, status);
}

/* -------------------------------------------------------------------------
 * GAP connection/advertising callbacks -> post CONNECTED/DISCONNECTED
 * ---------------------------------------------------------------------- */
static void gw_ble_connect_cbk(uint16_t conn_id, bd_addr_t *addr,
                                gap_ble_conn_state_t conn_state,
                                gap_ble_pair_state_t pair_state,
                                gap_ble_disc_reason_t disc_reason)
{
    unused(pair_state);
    unused(disc_reason);
    gw_msg_t msg = {0};
    msg.protocol      = GW_PROTO_BLE;
    msg.proto_conn_id = conn_id;
    msg.attr_handle   = g_gw_ble_notification_att_hdl;

    if (addr != NULL) {
        if (memcpy_s(msg.peer_addr, sizeof(msg.peer_addr),
                     addr->addr, BD_ADDR_LEN) != EOK) {
            osal_printk("%s connect cbk addr memcpy fail\r\n", GW_BLE_LOG);
        }
    }

    if (conn_state == GAP_BLE_STATE_CONNECTED) {
        osal_printk("%s connected conn_id=%d\r\n", GW_BLE_LOG, conn_id);
        g_gw_ble_conn_hdl = conn_id;
        msg.type = GW_MSG_CONN_CONNECTED;
        gw_service_post_msg(&msg);
    } else if (conn_state == GAP_BLE_STATE_DISCONNECTED) {
        osal_printk("%s disconnected conn_id=%d\r\n", GW_BLE_LOG, conn_id);
        msg.type = GW_MSG_CONN_DISCONNECTED;
        gw_service_post_msg(&msg);
        /* Restart advertising so new clients can connect */
        gw_ble_set_adv_data();
        gw_ble_start_adv();
    }
}

static void gw_ble_enable_cbk(uint8_t status)
{
    osal_printk("%s ble enabled status=%d\r\n", GW_BLE_LOG, status);
    g_gw_ble_status = GW_BLE_STATUS_OK;
}

static void gw_ble_adv_enable_cbk(uint8_t adv_id, adv_status_t status)
{
    osal_printk("%s adv enabled adv_id=%d status=%d\r\n", GW_BLE_LOG, adv_id, status);
}

static void gw_ble_adv_disable_cbk(uint8_t adv_id, adv_status_t status)
{
    osal_printk("%s adv disabled adv_id=%d status=%d\r\n", GW_BLE_LOG, adv_id, status);
}

/* -------------------------------------------------------------------------
 * Register all callbacks
 * ---------------------------------------------------------------------- */
static errcode_t gw_ble_register_callbacks(void)
{
    bts_dev_manager_callbacks_t dev_cb  = {0};
    gap_ble_callbacks_t         gap_cb  = {0};
    gatts_callbacks_t           svc_cb  = {0};
    errcode_t ret = 0;

    dev_cb.ble_enable_cb           = gw_ble_enable_cbk;
    gap_cb.conn_state_change_cb    = gw_ble_connect_cbk;
    gap_cb.start_adv_cb            = gw_ble_adv_enable_cbk;
    gap_cb.stop_adv_cb             = gw_ble_adv_disable_cbk;
    svc_cb.add_service_cb          = gw_ble_service_add_cbk;
    svc_cb.add_characteristic_cb   = gw_ble_char_add_cbk;
    svc_cb.add_descriptor_cb       = gw_ble_desc_add_cbk;
    svc_cb.start_service_cb        = gw_ble_service_start_cbk;
    svc_cb.read_request_cb         = gw_ble_read_cbk;
    svc_cb.write_request_cb        = gw_ble_write_cbk;
    svc_cb.mtu_changed_cb          = gw_ble_mtu_cbk;

    ret |= bts_dev_manager_register_callbacks(&dev_cb);
    ret |= gap_ble_register_callbacks(&gap_cb);
    if (ret != ERRCODE_BT_SUCCESS) {
        osal_printk("%s reg gap cbk fail\r\n", GW_BLE_LOG);
        return ERRCODE_FAIL;
    }
    ret = gatts_register_callbacks(&svc_cb);
    if (ret != ERRCODE_BT_SUCCESS) {
        osal_printk("%s reg gatts cbk fail\r\n", GW_BLE_LOG);
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * BLE advertisement helpers
 * ---------------------------------------------------------------------- */
static uint8_t g_gw_ble_local_name[] = {'b', 'l', 'e', '_', 'g', 'w'};

static uint16_t gw_ble_build_adv_data(uint8_t *buf, uint8_t max_len)
{
    uint8_t idx = 0;
    /* Flags */
    if (idx + 3 <= max_len) {
        buf[idx++] = 2;
        buf[idx++] = 0x01; /* AD type: Flags */
        buf[idx++] = 0x06; /* LE General Discoverable | BR/EDR Not Supported */
    }
    /* Shortened local name */
    uint8_t name_len = sizeof(g_gw_ble_local_name);
    if (idx + 2 + name_len <= max_len) {
        buf[idx++] = name_len + 1;
        buf[idx++] = 0x08; /* AD type: Shortened Local Name */
        (void)memcpy_s(&buf[idx], max_len - idx, g_gw_ble_local_name, name_len);
        idx += name_len;
    }
    return idx;
}

static void gw_ble_set_adv_data(void)
{
    uint8_t adv_buf[GW_BLE_ADV_DATA_MAX_LEN] = {0};
    uint16_t adv_len = gw_ble_build_adv_data(adv_buf, GW_BLE_ADV_DATA_MAX_LEN);
    gap_ble_config_adv_data_t cfg = {0};
    cfg.adv_data        = adv_buf;
    cfg.adv_length      = adv_len;
    cfg.scan_rsp_data   = NULL;
    cfg.scan_rsp_length = 0;
    gap_ble_set_adv_data(BTH_GAP_BLE_ADV_HANDLE_DEFAULT, &cfg);
}

static void gw_ble_start_adv(void)
{
    gap_ble_adv_params_t param = {0};
    param.min_interval        = GW_BLE_ADV_MIN_INTERVAL;
    param.max_interval        = GW_BLE_ADV_MAX_INTERVAL;
    param.duration            = BTH_GAP_BLE_ADV_FOREVER_DURATION;
    param.channel_map         = BLE_ADV_CHANNEL_MAP_CH_DEFAULT;
    param.adv_type            = BLE_ADV_TYPE_CONNECTABLE_UNDIRECTED;
    param.adv_filter_policy   = BLE_ADV_FILTER_POLICY_SCAN_ANY_CONNECT_ANY;
    param.peer_addr.type      = BLE_PUBLIC_DEVICE_ADDRESS;
    (void)memset_s(&param.peer_addr.addr, BD_ADDR_LEN, 0, BD_ADDR_LEN);
    gap_ble_set_adv_param(BTH_GAP_BLE_ADV_HANDLE_DEFAULT, &param);
    gap_ble_start_adv(BTH_GAP_BLE_ADV_HANDLE_DEFAULT);
}

/* -------------------------------------------------------------------------
 * gw_ble_adapter_init -- called from the gateway task
 * ---------------------------------------------------------------------- */
errcode_t gw_ble_adapter_init(void)
{
    errcode_t ret = gw_ble_register_callbacks();
    if (ret != ERRCODE_SUCC) {
        return ret;
    }

    enable_ble();
    /* Poll until BLE stack is ready */
    while (g_gw_ble_status != GW_BLE_STATUS_OK) {
        osal_msleep(GW_BLE_ENABLE_POLL_MS);
    }

    /* Register GATT server */
    bt_uuid_t app_uuid = {0};
    app_uuid.uuid_len = sizeof(g_gw_ble_app_uuid);
    if (memcpy_s(app_uuid.uuid, app_uuid.uuid_len,
                 g_gw_ble_app_uuid, sizeof(g_gw_ble_app_uuid)) != EOK) {
        return ERRCODE_FAIL;
    }
    gatts_register_server(&app_uuid, &g_gw_ble_server_id);

    /* Trigger service-build chain (completes via callbacks) */
    bt_uuid_t svc_uuid = {0};
    gw_ble_uuid_set(GW_BLE_UUID_SERVICE, &svc_uuid);
    gatts_add_service(g_gw_ble_server_id, &svc_uuid, true);

    /* Start advertising */
    gw_ble_set_adv_data();
    gw_ble_start_adv();

    osal_printk("%s adapter init ok\r\n", GW_BLE_LOG);
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * Send data to a connected BLE peer via GATT notification
 * ---------------------------------------------------------------------- */
errcode_t gw_ble_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                               const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return ERRCODE_FAIL;
    }
    gatts_ntf_ind_t param = {0};
    param.attr_handle = attr_handle;
    param.value_len   = len;
    param.value       = osal_vmalloc(len);
    if (param.value == NULL) {
        osal_printk("%s send vmalloc fail\r\n", GW_BLE_LOG);
        return ERRCODE_FAIL;
    }
    if (memcpy_s(param.value, len, data, len) != EOK) {
        osal_vfree(param.value);
        return ERRCODE_FAIL;
    }
    errcode_t ret = gatts_notify_indicate(g_gw_ble_server_id, conn_id, &param);
    osal_vfree(param.value);
    if (ret != ERRCODE_BT_SUCCESS) {
        osal_printk("%s send fail conn_id=%d ret=0x%x\r\n", GW_BLE_LOG, conn_id, ret);
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}
