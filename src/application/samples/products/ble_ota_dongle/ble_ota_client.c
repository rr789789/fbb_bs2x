/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: BLE OTA sample of client. \n
 *
 * History: \n
 * 2024-02-01, Create file. \n
 */
#include "securec.h"
#include "string.h"
#include "common_def.h"
#include "osal_debug.h"
#include "osal_task.h"
#include "bts_le_gap.h"
#include "bts_device_manager.h"
#include "gadget/f_hid.h"
#include "ble_ota_cmd.h"
#include "ble_ota_cmd_handler.h"
#include "ble_ota_client.h"

#define BLE_MTU_SIZE_DEFAULT                        CONFIG_SLE_MTU_LENGTH
#define BLE_WAIT_BLE_CORE_READY_MS                  1000
#define BLE_SERVER_ADDR_RPT_LEN                     20
#define OTA16_LEN 					2
#define BLE_SERVER_ADDR_INDEX                       12
#define BLE_SERVER_DATA_RPT_LEN                     64
#define BLE_SERVER_DATA_INDEX                       12
#define BLE_SERVER_RPT_LEN_INDEX                    7
#define BLE_SERVER_DATA_LEN_INDEX                   10
#define BLE_SERVER_8_BIT_SHIFT                      8
#define BLE_OTA_DONGLE_LOG                          "[ble ota dongle]"
/* client id, invalid client id is "0" */
uint8_t g_ota_client_id = 0;
/* ota client app uuid for test */
static bt_uuid_t g_client_app_uuid = { 2, { 0 } };

static uint16_t g_ble_ota_conn_id = 0;
static uint8_t g_ssap_find_ready = 0;
static uint8_t g_ssap_connect_param_update_ready = 0;
static uint8_t g_ssap_connect_state = 0;
static uint8_t g_ble_enable_status = 0;
static bts_dev_manager_callbacks_t dev_cb = { 0 };
static gap_ble_callbacks_t gap_cb = { 0 };
static gattc_callbacks_t cb = { 0 };
static bd_addr_t g_ble_ota_remote_addr = {
    .addr = { 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 },
    .type = 0
};
static gattc_handle_value_t g_ble_ota_send_param = { 0 };
static gap_ble_conn_state_t g_ble_ota_conn_state = GAP_BLE_STATE_DISCONNECTED;
static gap_ble_pair_state_t g_ble_ota_pair_state = GAP_BLE_PAIR_NONE;

static bool g_addr_is_set = false;
static bool g_scan_state = false;

static uint8_t g_sever_addr_rpt[BLE_SERVER_ADDR_RPT_LEN] = { BLE_LINK_FRAME_HEAD_FLAG, 0x0, 0x0, 0x0, 0x0,
                                                             0x01, 0x0C, 0x09, 0x00, 0x03, 0x06, 0x00 };

#define BLE_OTA_CHARACTER_CLIENT_UUID_TX                  0xFF02

uint16_t get_ble_ota_conn_id(void)
{
    return g_ble_ota_conn_id;
}

uint8_t get_ble_ota_client_id(void)
{
    return g_ota_client_id;
}
void set_ble_ota_set_address_state(bool state)
{
    g_addr_is_set = state;
}

bool get_ble_ota_set_address_state(void)
{
    return g_addr_is_set;
}

void set_ble_ota_scan_state(bool state)
{
    g_scan_state = state;
}

bool get_ble_ota_scan_state(void)
{
    return g_scan_state;
}

uint8_t get_ble_ota_conn_state(void)
{
    return g_ble_ota_conn_state;
}

gattc_handle_value_t *get_ble_ota_send_param(void)
{
    return &g_ble_ota_send_param;
}

uint8_t get_g_ble_enable_status(void)
{
    return g_ble_enable_status;
}

uint8_t get_ssap_find_ready(void)
{
    return g_ssap_find_ready;
}
uint8_t get_ssap_connect_param_update_ready(void)
{
    return g_ssap_connect_param_update_ready;
}

static uint16_t g_ota_scan_interval = 0x48;
static uint16_t g_ota_scan_window = 0x48;
static uint8_t g_ota_scan_type = 0x00;
static uint8_t g_ota_scan_phy = 0x01;
static uint8_t g_ota_scan_filter_policy = 0x00;
void ble_ota_start_scan(void)
{
    gap_ble_scan_params_t ble_ota_scan_params = { 0 };
    ble_ota_scan_params.scan_interval = g_ota_scan_interval;
    ble_ota_scan_params.scan_window = g_ota_scan_window;
    ble_ota_scan_params.scan_type = g_ota_scan_type;
    ble_ota_scan_params.scan_phy = g_ota_scan_phy;
    ble_ota_scan_params.scan_filter_policy = g_ota_scan_filter_policy;
    gap_ble_set_scan_parameters(&ble_ota_scan_params);
    gap_ble_start_scan();
}

static void ble_ota_client_sample_ble_enable_cbk(uint8_t status)
{
    osal_msleep(BLE_WAIT_BLE_CORE_READY_MS);
    if (status != 0) {
        osal_printk("%s ble_ota_client_sample_ble_enable_cbk,status error\r\n", BLE_OTA_DONGLE_LOG);
    } else {
        uint8_t local_addr[BD_ADDR_LEN] = {0x0b, 0x01, 0x02, 0x03, 0x04, 0x05};
        bd_addr_t local_address;
        local_address.type = 0;
        (void)memcpy_s(local_address.addr, BD_ADDR_LEN, local_addr, BD_ADDR_LEN);
        if (gap_ble_set_local_addr(&local_address) != ERRCODE_SUCC) {
            osal_printk("%s set local addr error!\r\n", BLE_OTA_DONGLE_LOG);
            return;
        }
        gattc_register_client(&g_client_app_uuid, &g_ota_client_id);
        osal_printk("%s enter callback of sle enable,start scan!\r\n", BLE_OTA_DONGLE_LOG);
        ble_ota_start_scan();
        g_ble_enable_status = 1;
    }
}


static void ble_ota_client_sample_seek_result_info_cbk(gap_scan_result_data_t *seek_result_data)
{
    if (seek_result_data == NULL || seek_result_data->adv_data == NULL) {
        osal_printk("seek_result_data error!\r\n");
        return;
    }
    osal_printk("seek_result:%x %x!\r\n,", seek_result_data->addr.addr[0], seek_result_data->addr.addr[1]);
    if (memcmp(g_ble_ota_remote_addr.addr, seek_result_data->addr.addr, BD_ADDR_LEN) == 0) {
        gap_ble_connect_remote_device(&g_ble_ota_remote_addr);
    }
    if (get_ble_ota_scan_state() == true) {
        for (uint8_t i = 0; i < BD_ADDR_LEN; i++) {
            g_sever_addr_rpt[BLE_SERVER_ADDR_INDEX + i] = (uint8_t)seek_result_data->addr.addr[i];
        }
        fhid_send_data(ble_ota_get_hid_index(), (char *)g_sever_addr_rpt, BLE_SERVER_ADDR_RPT_LEN);
    }
}

void ble_ota_client_sample_connect_state_changed_cbk(uint16_t conn_id, bd_addr_t *addr,
                                                     gap_ble_conn_state_t conn_state,
                                                     gap_ble_pair_state_t pair_state,
                                                     gap_ble_disc_reason_t disc_reason)
{
    unused(addr);
    unused(pair_state);
    osal_printk("%s conn state changed,connect_state:%d, disc_reason:%d\r\n", BLE_OTA_DONGLE_LOG,
                conn_state, disc_reason);
    g_ble_ota_conn_id = conn_id;
    g_ble_ota_conn_state = conn_state;
    g_ble_ota_pair_state = pair_state;
    ble_ota_get_conn_status(SERVICE_ID_SERVICE_CONNECT, COMMAND_ID_GET_CONN_STATUS, NULL, 0);
    if (conn_state == GAP_BLE_STATE_CONNECTED) {
        osal_printk("%s BLE_ACL_STATE_CONNECTED\r\n", BLE_OTA_DONGLE_LOG);
        g_ssap_connect_state = 1;
        gap_ble_stop_scan();
        gap_ble_pair_remote_device(addr);
        gap_conn_param_update_t params = {0};
        params.conn_handle = conn_id;
        params.interval_min = 0x48;
        params.interval_max = 0x48;
        params.slave_latency = 0;
        params.timeout_multiplier = 0x1f4;
        gap_ble_connect_param_update(&params);
        osal_printk("ble_update_connect_param\r\n");
    } else if (conn_state == GAP_BLE_STATE_DISCONNECTED) {
        osal_printk("%s BLE_ACL_STATE_DISCONNECTED\r\n", BLE_OTA_DONGLE_LOG);
        g_ssap_connect_state = 0;
        (void)memcpy_s(&g_ble_ota_remote_addr, sizeof(bd_addr_t), addr, sizeof(bd_addr_t));
        set_ble_ota_scan_state(true);
        ble_ota_start_scan();
        g_ssap_find_ready = 0;
    } else {
    }
}

static void ble_ota_client_param_update_req_cbk(uint16_t conn_id, errcode_t status,
                                                const gap_ble_conn_param_update_t *param)
{
    unused(conn_id);
    unused(status);
    unused(param);
    g_ssap_connect_param_update_ready = 1;
}

void  ble_ota_client_sample_pair_complete_cbk(uint16_t conn_id, const bd_addr_t *addr, errcode_t status)
{
    unused(conn_id);
    unused(addr);
    unused(status);
    osal_printk("pair status %d\r\n", status);
    if (status == 0) {
        g_ssap_connect_param_update_ready = 1;
        gattc_exchange_mtu_req(1, conn_id, BLE_MTU_SIZE_DEFAULT);
    }
}

static void ble_ota_client_sample_seek_cbk_register(void)
{
    gap_cb.scan_result_cb = ble_ota_client_sample_seek_result_info_cbk;
    gap_cb.conn_state_change_cb = ble_ota_client_sample_connect_state_changed_cbk;
    gap_cb.conn_param_update_cb = ble_ota_client_param_update_req_cbk;
    gap_cb.pair_result_cb = ble_ota_client_sample_pair_complete_cbk;
    dev_cb.ble_enable_cb = ble_ota_client_sample_ble_enable_cbk;
    bts_dev_manager_register_callbacks(&dev_cb);
    gap_ble_register_callbacks(&gap_cb);
}
static void ble_ota_client_sample_exchange_info_cbk(uint8_t client_id, uint16_t conn_id,
                                                    uint16_t mtu_size, errcode_t status)
{
    unused(conn_id);
    unused(client_id);
    unused(mtu_size);
    unused(status);
    osal_printk("exchange info status %d\r\n", status);
    bt_uuid_t service_uuid = { 0 }; /* uuid length is zero, discover all service */
    gattc_discovery_service(client_id, conn_id, &service_uuid);
}

static void ble_ota_client_sample_find_structure_cbk(uint8_t client_id, uint16_t conn_id,
                                                     gattc_discovery_service_result_t *service, errcode_t status)
{
    unused(conn_id);
    unused(client_id);
    unused(service);
    unused(status);
        osal_printk("Discovery service callback client:%d conn_id:%d\n", client_id, conn_id);
    osal_printk(" start handle:%d end handle:%d uuid_len:%d uuid:\n",
                service->start_hdl, service->end_hdl, service->uuid.uuid_len);
    gattc_discovery_character_param_t param = { 0 };
    param.service_handle = service->start_hdl;
    param.uuid.uuid_len = service->uuid.uuid_len; /* uuid length is zero, discover all character */
    if (memcpy_s(param.uuid.uuid, param.uuid.uuid_len, service->uuid.uuid, service->uuid.uuid_len) != 0) {
        osal_printk("%s memcpy error\n", BLE_OTA_DONGLE_LOG);
    }
    gattc_discovery_character(client_id, conn_id, &param);
}
static void bts_data_to_uuid_len2(uint16_t uuid_data, bt_uuid_t *out_uuid)
{
    out_uuid->uuid_len = OTA16_LEN;
    out_uuid->uuid[0] = (uint8_t)(uuid_data >> 8); /* 8: octet bit num */
    out_uuid->uuid[1] = (uint8_t)(uuid_data);
}

static void ble_ota_client_sample_find_property_cbk(uint8_t client_id, uint16_t conn_id,
                                                    gattc_discovery_character_result_t *property, errcode_t status)
{
    unused(conn_id);
    unused(client_id);
    unused(property);
    unused(status);
        bt_uuid_t write_uuid = { 0 };
    bts_data_to_uuid_len2(BLE_OTA_CHARACTER_CLIENT_UUID_TX, &write_uuid);
    write_uuid.uuid_len = BT_UUID_MAX_LEN;
    if (memcmp(property->uuid.uuid, write_uuid.uuid, property->uuid.uuid_len) == 0) {
        g_ble_ota_send_param.handle = property->value_handle;
        g_ssap_find_ready = 1;
        osal_printk("ota prop handle 0x%02x\r\n", g_ble_ota_send_param.handle);
    }
}

static void ble_ota_client_sample_find_structure_cmp_cbk(uint8_t client_id, uint16_t conn_id,
                                                         gattc_discovery_character_param_t *structure_result,
                                                         errcode_t status)
{
    unused(conn_id);
    unused(client_id);
    unused(structure_result);
    unused(status);
}

static void ble_ota_client_sample_ssapc_cbk_register(gattc_notification_callback notification_cb,
                                                     gattc_indication_callback indication_cb)
{
    cb.mtu_changed_cb = ble_ota_client_sample_exchange_info_cbk;
    cb.discovery_svc_cb = ble_ota_client_sample_find_structure_cbk;
    cb.discovery_chara_cb = ble_ota_client_sample_find_property_cbk;
    cb.discovery_chara_cmp_cb = ble_ota_client_sample_find_structure_cmp_cbk;
    cb.notification_cb = notification_cb;
    cb.indication_cb = indication_cb;
    gattc_register_callbacks(&cb);
}

void ble_ota_client_init(gattc_notification_callback notification_cb, gattc_indication_callback indication_cb)
{
    ble_ota_client_sample_seek_cbk_register();
    ble_ota_client_sample_ssapc_cbk_register(notification_cb, indication_cb);
    enable_ble();
}
