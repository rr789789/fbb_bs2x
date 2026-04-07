/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE SSAP-Server adapter for the gateway. \n
 *
 * Initialization flow (mirrors sle_uart_server):
 *   app_run context: gw_sle_dev_register_cbks()
 *     -> registers sle_power_on_cbk  (-> enable_sle)
 *     -> registers sle_enable_cbk    (-> gw_sle_server_setup: adds SSAP service)
 *
 *   gateway task:   gw_sle_adapter_init()
 *     -> registers SSAP write/read callbacks
 *     -> registers connection-state callbacks
 *     -> registers announce-seek callbacks
 *     -> starts SLE advertisement
 */
#include "common_def.h"
#include "securec.h"
#include "soc_osal.h"
#include "sle_errcode.h"
#include "sle_device_manager.h"
#include "sle_connection_manager.h"
#include "sle_transmition_manager.h"
#include "sle_common.h"
#include "sle_device_discovery.h"
#include "sle_ssap_server.h"
#include "gateway_service.h"
#include "gw_sle_adapter.h"

/* -------------------------------------------------------------------------
 * SLE advertising constants and types
 * These are defined in sample-private headers rather than the common SDK
 * headers, so we define the numeric values here directly.
 * ---------------------------------------------------------------------- */
#define SLE_ADV_CHANNEL_MAP_DEFAULT     0x07

typedef enum {
    SLE_ADV_DATA_TYPE_DISCOVERY_LEVEL     = 0x01,
    SLE_ADV_DATA_TYPE_ACCESS_MODE         = 0x02,
    SLE_ADV_DATA_TYPE_TX_POWER_LEVEL      = 0x0C,
    SLE_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME = 0x0B,
} gw_sle_adv_data_type_t;

struct sle_adv_common_value {
    uint8_t length;
    uint8_t type;
    uint8_t value;
};

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */
#define GW_SLE_LOG              "[gw_sle] "
#define GW_SLE_ADV_HANDLE       1
#define GW_SLE_UUID_INDEX       14
#define GW_SLE_UUID_LEN         2

/* GW-specific SLE UUIDs (distinct from sle_uart 0x2222/0x2323) */
#define GW_SLE_UUID_SERVICE     0x4400
#define GW_SLE_UUID_PROPERTY    0x4401

#define GW_SLE_CONN_INTV_MIN    0x64   /* 12.5 ms in 125 us units */
#define GW_SLE_CONN_INTV_MAX    0x64
#define GW_SLE_ADV_INTV_MIN     0xC8   /* 25 ms */
#define GW_SLE_ADV_INTV_MAX     0xC8
#define GW_SLE_CONN_TIMEOUT     0x1F4  /* 5000 ms in 10 ms units */
#define GW_SLE_CONN_LATENCY     0x1F3
#define GW_SLE_ADV_TX_POWER     6
#define GW_SLE_ADV_DATA_LEN_MAX 251

/* 128-bit UUID base for gateway SLE service */
static uint8_t g_gw_sle_base[] = {
    0x38, 0xBE, 0xA8, 0x80, 0xFC, 0x70, 0x11, 0xEA,
    0xB7, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static uint8_t g_gw_sle_app_uuid[] = { 0x56, 0x78 };
static uint8_t g_gw_sle_adv_name[] = "sle_uart_server"; /* matches SLE_UART_SERVER_NAME in sle_uart_client */

/* -------------------------------------------------------------------------
 * Server handles
 * ---------------------------------------------------------------------- */
static uint8_t  g_gw_sle_server_id     = 0;
static uint16_t g_gw_sle_service_hdl   = 0;
static uint16_t g_gw_sle_property_hdl  = 0;

/* -------------------------------------------------------------------------
 * UUID helpers
 * ---------------------------------------------------------------------- */
static void gw_sle_uuid_set(uint16_t u2, sle_uuid_t *out)
{
    if (memcpy_s(out->uuid, SLE_UUID_LEN, g_gw_sle_base, SLE_UUID_LEN) != EOK) {
        out->len = 0;
        return;
    }
    out->len = GW_SLE_UUID_LEN;
    out->uuid[GW_SLE_UUID_INDEX]     = (uint8_t)(u2 & 0xFF);
    out->uuid[GW_SLE_UUID_INDEX + 1] = (uint8_t)((u2 >> 8) & 0xFF);
}

/* -------------------------------------------------------------------------
 * SSAP callbacks (write_request -> post DATA_RECEIVED to the service queue)
 * ---------------------------------------------------------------------- */
static void gw_ssaps_write_request_cbk(uint8_t server_id, uint16_t conn_id,
                                        ssaps_req_write_cb_t *write_cb, errcode_t status)
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
    msg.type         = GW_MSG_DATA_RECEIVED;
    msg.protocol     = GW_PROTO_SLE;
    msg.proto_conn_id = conn_id;
    msg.data_len     = copy_len;
    if (memcpy_s(msg.data, GW_DATA_MAX_LEN, write_cb->value, copy_len) != EOK) {
        osal_printk("%s write cbk memcpy fail\r\n", GW_SLE_LOG);
        return;
    }
    gw_service_post_msg(&msg);
}

static void gw_ssaps_read_request_cbk(uint8_t server_id, uint16_t conn_id,
                                       ssaps_req_read_cb_t *read_cb, errcode_t status)
{
    unused(server_id);
    unused(conn_id);
    unused(read_cb);
    unused(status);
}

static void gw_ssaps_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id,
                                      ssap_exchange_info_t *mtu_info, errcode_t status)
{
    osal_printk("%s mtu changed server_id=%d conn_id=%d mtu=%d status=0x%x\r\n",
                GW_SLE_LOG, server_id, conn_id, mtu_info->mtu_size, status);
}

/* -------------------------------------------------------------------------
 * SLE connection callbacks -> post CONNECTED/DISCONNECTED to service queue
 * ---------------------------------------------------------------------- */
static void gw_sle_connect_state_cbk(uint16_t conn_id, const sle_addr_t *addr,
                                      sle_acb_state_t conn_state,
                                      sle_pair_state_t pair_state,
                                      sle_disc_reason_t disc_reason)
{
    unused(pair_state);
    unused(disc_reason);
    gw_msg_t msg = {0};
    msg.protocol      = GW_PROTO_SLE;
    msg.proto_conn_id = conn_id;
    msg.attr_handle   = g_gw_sle_property_hdl;

    if (addr != NULL) {
        if (memcpy_s(msg.peer_addr, sizeof(msg.peer_addr),
                     addr->addr, sizeof(addr->addr)) != EOK) {
            osal_printk("%s connect cbk addr memcpy fail\r\n", GW_SLE_LOG);
        }
    }

    if (conn_state == SLE_ACB_STATE_CONNECTED) {
        osal_printk("%s connected conn_id=0x%x\r\n", GW_SLE_LOG, conn_id);
        msg.type = GW_MSG_CONN_CONNECTED;
        gw_service_post_msg(&msg);
    } else if (conn_state == SLE_ACB_STATE_DISCONNECTED) {
        osal_printk("%s disconnected conn_id=0x%x\r\n", GW_SLE_LOG, conn_id);
        msg.type = GW_MSG_CONN_DISCONNECTED;
        gw_service_post_msg(&msg);
        /* Restart advertising so new clients can connect */
        sle_start_announce(GW_SLE_ADV_HANDLE);
    }
}

static void gw_sle_pair_complete_cbk(uint16_t conn_id, const sle_addr_t *addr, errcode_t status)
{
    osal_printk("%s pair complete conn_id=0x%x status=0x%x addr=%02x:**:**:**:%02x:%02x\r\n",
                GW_SLE_LOG, conn_id, status, addr->addr[0], addr->addr[4], addr->addr[5]);
}

/* -------------------------------------------------------------------------
 * SSAP service/property setup (called from sle_enable_cbk)
 * ---------------------------------------------------------------------- */
static errcode_t gw_sle_server_setup(void)
{
    errcode_t ret;
    sle_uuid_t app_uuid = {0};
    sle_uuid_t svc_uuid = {0};
    ssaps_property_info_t prop  = {0};
    ssaps_desc_info_t     desc  = {0};

    /* Register application UUID */
    app_uuid.len = sizeof(g_gw_sle_app_uuid);
    if (memcpy_s(app_uuid.uuid, app_uuid.len,
                 g_gw_sle_app_uuid, sizeof(g_gw_sle_app_uuid)) != EOK) {
        return ERRCODE_FAIL;
    }
    ssaps_register_server(&app_uuid, &g_gw_sle_server_id);

    /* Add service */
    gw_sle_uuid_set(GW_SLE_UUID_SERVICE, &svc_uuid);
    ret = ssaps_add_service_sync(g_gw_sle_server_id, &svc_uuid, 1, &g_gw_sle_service_hdl);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s add service fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        ssaps_unregister_server(g_gw_sle_server_id);
        return ERRCODE_FAIL;
    }

    /* Add notify/write property */
    prop.permissions        = SSAP_PERMISSION_READ | SSAP_PERMISSION_WRITE;
    prop.operate_indication = SSAP_OPERATE_INDICATION_BIT_READ |
                              SSAP_OPERATE_INDICATION_BIT_WRITE |
                              SSAP_OPERATE_INDICATION_BIT_NOTIFY;
    gw_sle_uuid_set(GW_SLE_UUID_PROPERTY, &prop.uuid);
    uint8_t init_val[2] = {0};
    prop.value     = init_val;
    prop.value_len = sizeof(init_val);
    ret = ssaps_add_property_sync(g_gw_sle_server_id, g_gw_sle_service_hdl,
                                  &prop, &g_gw_sle_property_hdl);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s add property fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        ssaps_unregister_server(g_gw_sle_server_id);
        return ERRCODE_FAIL;
    }

    /* Add CCC descriptor */
    uint8_t ccc_val[] = {0x01, 0x00};
    desc.permissions        = SSAP_PERMISSION_READ | SSAP_PERMISSION_WRITE;
    desc.type               = SSAP_DESCRIPTOR_CLIENT_CONFIGURATION;
    desc.operate_indication = SSAP_OPERATE_INDICATION_BIT_READ |
                              SSAP_OPERATE_INDICATION_BIT_WRITE |
                              SSAP_OPERATE_INDICATION_BIT_DESCRIPTOR_CLIENT_CONFIGURATION_WRITE;
    desc.value     = ccc_val;
    desc.value_len = sizeof(ccc_val);
    ret = ssaps_add_descriptor_sync(g_gw_sle_server_id, g_gw_sle_service_hdl,
                                    g_gw_sle_property_hdl, &desc);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s add descriptor fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        ssaps_unregister_server(g_gw_sle_server_id);
        return ERRCODE_FAIL;
    }

    ret = ssaps_start_service(g_gw_sle_server_id, g_gw_sle_service_hdl);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s start service fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        return ERRCODE_FAIL;
    }
    osal_printk("%s server setup ok server_id=%d svc_hdl=0x%x prop_hdl=0x%x\r\n",
                GW_SLE_LOG, g_gw_sle_server_id, g_gw_sle_service_hdl, g_gw_sle_property_hdl);
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * SLE device-manager callbacks (power-on / enable)
 * ---------------------------------------------------------------------- */
static void gw_sle_announce_enable_cbk(uint32_t announce_id, errcode_t status)
{
    osal_printk("%s announce enable id=%d status=0x%x\r\n", GW_SLE_LOG, announce_id, status);
}

static void gw_sle_announce_disable_cbk(uint32_t announce_id, errcode_t status)
{
    osal_printk("%s announce disable id=%d status=0x%x\r\n", GW_SLE_LOG, announce_id, status);
}

static void gw_sle_announce_terminal_cbk(uint32_t announce_id)
{
    osal_printk("%s announce terminal id=%d\r\n", GW_SLE_LOG, announce_id);
}

static void gw_sle_power_on_cbk(uint8_t status)
{
    osal_printk("%s power on status=%d\r\n", GW_SLE_LOG, status);
    enable_sle();
}

static void gw_sle_enable_cbk(uint8_t status)
{
    osal_printk("%s sle enabled status=%d\r\n", GW_SLE_LOG, status);
    gw_sle_server_setup();
}

errcode_t gw_sle_dev_register_cbks(void)
{
    sle_dev_manager_callbacks_t dev_cbks = {0};
    dev_cbks.sle_power_on_cb = gw_sle_power_on_cbk;
    dev_cbks.sle_enable_cb   = gw_sle_enable_cbk;
    errcode_t ret = sle_dev_manager_register_callbacks(&dev_cbks);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s dev_register_cbks fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        return ERRCODE_FAIL;
    }
#if (CORE_NUMS < 2)
    enable_sle();
#endif
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * Advertisement helpers
 * ---------------------------------------------------------------------- */
static void gw_sle_set_adv_params(void)
{
    sle_announce_param_t param = {0};
    uint8_t zero_addr[SLE_ADDR_LEN] = {0};
    param.announce_mode      = SLE_ANNOUNCE_MODE_CONNECTABLE_SCANABLE;
    param.announce_handle    = GW_SLE_ADV_HANDLE;
    param.announce_gt_role   = SLE_ANNOUNCE_ROLE_T_CAN_NEGO;
    param.announce_level     = SLE_ANNOUNCE_LEVEL_NORMAL;
    param.announce_channel_map  = SLE_ADV_CHANNEL_MAP_DEFAULT;
    param.announce_interval_min = GW_SLE_ADV_INTV_MIN;
    param.announce_interval_max = GW_SLE_ADV_INTV_MAX;
    param.conn_interval_min     = GW_SLE_CONN_INTV_MIN;
    param.conn_interval_max     = GW_SLE_CONN_INTV_MAX;
    param.conn_max_latency      = GW_SLE_CONN_LATENCY;
    param.conn_supervision_timeout = GW_SLE_CONN_TIMEOUT;
    param.own_addr.type = 0;
    (void)memcpy_s(param.own_addr.addr, SLE_ADDR_LEN, zero_addr, SLE_ADDR_LEN);
    sle_set_announce_param(GW_SLE_ADV_HANDLE, &param);
}

static void gw_sle_set_adv_data(void)
{
    uint8_t adv_data[GW_SLE_ADV_DATA_LEN_MAX]     = {0};
    uint8_t scan_rsp[GW_SLE_ADV_DATA_LEN_MAX]     = {0};
    sle_announce_data_t data = {0};
    uint16_t adv_idx  = 0;
    uint16_t scan_idx = 0;

    /* ADV data: discovery level + access mode */
    struct sle_adv_common_value disc = {
        .length = sizeof(struct sle_adv_common_value) - 1,
        .type   = SLE_ADV_DATA_TYPE_DISCOVERY_LEVEL,
        .value  = SLE_ANNOUNCE_LEVEL_NORMAL,
    };
    if (memcpy_s(&adv_data[adv_idx], GW_SLE_ADV_DATA_LEN_MAX,
                 &disc, sizeof(disc)) == EOK) {
        adv_idx += sizeof(disc);
    }
    struct sle_adv_common_value acc = {
        .length = sizeof(struct sle_adv_common_value) - 1,
        .type   = SLE_ADV_DATA_TYPE_ACCESS_MODE,
        .value  = 0,
    };
    if (memcpy_s(&adv_data[adv_idx], (uint16_t)(GW_SLE_ADV_DATA_LEN_MAX - adv_idx),
                 &acc, sizeof(acc)) == EOK) {
        adv_idx += sizeof(acc);
    }

    /* Scan response: tx power + local name */
    struct sle_adv_common_value tx_pwr = {
        .length = sizeof(struct sle_adv_common_value) - 1,
        .type   = SLE_ADV_DATA_TYPE_TX_POWER_LEVEL,
        .value  = GW_SLE_ADV_TX_POWER,
    };
    if (memcpy_s(&scan_rsp[scan_idx], GW_SLE_ADV_DATA_LEN_MAX,
                 &tx_pwr, sizeof(tx_pwr)) == EOK) {
        scan_idx += sizeof(tx_pwr);
    }
    uint8_t name_len = (uint8_t)(sizeof(g_gw_sle_adv_name) - 1);
    if ((scan_idx + 2 + name_len) <= GW_SLE_ADV_DATA_LEN_MAX) {
        scan_rsp[scan_idx++] = name_len + 1;
        scan_rsp[scan_idx++] = SLE_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME;
        if (memcpy_s(&scan_rsp[scan_idx],
                     (uint16_t)(GW_SLE_ADV_DATA_LEN_MAX - scan_idx),
                     g_gw_sle_adv_name, name_len) == EOK) {
            scan_idx += name_len;
        }
    }

    data.announce_data     = adv_data;
    data.announce_data_len = adv_idx;
    data.seek_rsp_data     = scan_rsp;
    data.seek_rsp_data_len = scan_idx;
    sle_set_announce_data(GW_SLE_ADV_HANDLE, &data);
}

/* -------------------------------------------------------------------------
 * gw_sle_adapter_init -- called from the gateway task
 * ---------------------------------------------------------------------- */
errcode_t gw_sle_adapter_init(void)
{
    errcode_t ret;

    /* Register SSAP callbacks */
    ssaps_callbacks_t ssaps_cbk = {0};
    ssaps_cbk.read_request_cb  = gw_ssaps_read_request_cbk;
    ssaps_cbk.write_request_cb = gw_ssaps_write_request_cbk;
    ssaps_cbk.mtu_changed_cb   = gw_ssaps_mtu_changed_cbk;
    ret = ssaps_register_callbacks(&ssaps_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s ssaps_register_callbacks fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        return ERRCODE_FAIL;
    }

    /* Register connection callbacks */
    sle_connection_callbacks_t conn_cbk = {0};
    conn_cbk.connect_state_changed_cb = gw_sle_connect_state_cbk;
    conn_cbk.pair_complete_cb         = gw_sle_pair_complete_cbk;
    ret = sle_connection_register_callbacks(&conn_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s connection_register_callbacks fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        return ERRCODE_FAIL;
    }

    /* Register announce callbacks */
    sle_announce_seek_callbacks_t seek_cbk = {0};
    seek_cbk.announce_enable_cb   = gw_sle_announce_enable_cbk;
    seek_cbk.announce_disable_cb  = gw_sle_announce_disable_cbk;
    seek_cbk.announce_terminal_cb = gw_sle_announce_terminal_cbk;
    ret = sle_announce_seek_register_callbacks(&seek_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s announce_register_callbacks fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        return ERRCODE_FAIL;
    }

    /* Start advertising */
    gw_sle_set_adv_params();
    gw_sle_set_adv_data();
    ret = sle_start_announce(GW_SLE_ADV_HANDLE);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s sle_start_announce fail ret=0x%x\r\n", GW_SLE_LOG, ret);
        return ERRCODE_FAIL;
    }
    osal_printk("%s adapter init ok\r\n", GW_SLE_LOG);
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * Send data to a connected SLE peer
 * ---------------------------------------------------------------------- */
errcode_t gw_sle_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                               const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return ERRCODE_FAIL;
    }
    ssaps_ntf_ind_t param = {0};
    param.handle     = attr_handle;
    param.type       = SSAP_PROPERTY_TYPE_VALUE;
    param.value      = (uint8_t *)(uintptr_t)data; /* non-const cast, stack owns data */
    param.value_len  = len;

    errcode_t ret = ssaps_notify_indicate(g_gw_sle_server_id, conn_id, &param);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s send fail conn_id=%d ret=0x%x\r\n", GW_SLE_LOG, conn_id, ret);
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}
