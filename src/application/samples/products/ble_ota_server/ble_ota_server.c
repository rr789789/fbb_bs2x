/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: BLE OTA Server Source. \n
 *
 * History: \n
 * 2023-07-26, Create file. \n
 */
#include "osal_addr.h"
#include "osal_debug.h"
#include "product.h"
#include "securec.h"
#include "errcode.h"
#include "bts_def.h"
#include "bts_device_manager.h"
#include "bts_gatt_stru.h"
#include "bts_gatt_server.h"
#include "bts_le_gap.h"
#include "dfx_channel.h"
#include "ble_ota_server_adv.h"
#include "ble_ota_server.h"

/* ota gatt server id */
#define BLE_OTA_SERVER_ID 			1
/* ota ble connect id */
#define BLE_SINGLE_LINK_CONNECT_ID 	1
/* octets of 16 bits ota */
#define OTA16_LEN 					2
/* invalid attribute handle */
#define INVALID_ATT_HDL 			0
/* invalid server ID */
#define INVALID_SERVER_ID 			0

#define BLE_OTA_SERVER_LOG "[ble ota server]"
#define BLE_OTA_SERVER_ERROR "[ble ota server error]"
#define BLE_OTA_SERVICE_NUM 3

#define MAXTXOCTETS 251
#define MAXTXTIME 2120

static uint16_t g_ble_ota_conn_id;
static uint8_t g_ble_ota_name_value[] = { 'b', 'l', 'e', '_', 'u', 'a', 'r', 't', '\0' };
static uint8_t g_ota_server_app_uuid[] = { 0x00, 0x00 };
static uint8_t g_ble_ota_server_addr[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
static uint8_t g_server_id = INVALID_SERVER_ID;
static uint8_t g_connection_state = 0;
static uint16_t g_notify_indicate_handle = 0;
static uint8_t g_service_num = 0;

/* 将uint16的uuid数字转化为bt_uuid_t */
static void bts_data_to_uuid_len2(uint16_t uuid_data, bt_uuid_t *out_uuid)
{
    out_uuid->uuid_len = OTA16_LEN;
    out_uuid->uuid[0] = (uint8_t)(uuid_data >> 8); /* 8: octet bit num */
    out_uuid->uuid[1] = (uint8_t)(uuid_data);
}

/* 创建服务 */
static void ble_ota_add_service(void)
{
    bt_uuid_t ota_service_uuid = { 0 };
    bts_data_to_uuid_len2(BLE_OTA_UUID_SERVER_SERVICE, &ota_service_uuid);
    gatts_add_service(BLE_OTA_SERVER_ID, &ota_service_uuid, true);
}

/* 添加OTA服务的所有特征和描述符 */
static void ble_ota_add_tx_characters_and_descriptors(uint8_t server_id, uint16_t srvc_handle)
{
    osal_printk("%s TX characters:%d srv_handle:%d \n", BLE_OTA_SERVER_LOG, server_id, srvc_handle);
    bt_uuid_t characters_uuid = { 0 };
    uint8_t characters_value[] = { 0x12, 0x34 };
    bts_data_to_uuid_len2(BLE_OTA_CHARACTERISTIC_UUID_TX, &characters_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = characters_uuid;
    character.properties = GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP | GATT_CHARACTER_PROPERTY_BIT_NOTIFY;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE;
    character.value_len = sizeof(characters_value);
    character.value = characters_value;
    gatts_add_characteristic(server_id, srvc_handle, &character);
    osal_printk("%s characters_uuid:%2x %2x\n", BLE_OTA_SERVER_LOG, characters_uuid.uuid[0], characters_uuid.uuid[1]);

    static uint8_t ccc_val[] = { 0x01, 0x00 }; // notify
    bt_uuid_t ccc_uuid = { 0 };
    bts_data_to_uuid_len2(BLE_OTA_CLIENT_CHARACTERISTIC_CONFIGURATION, &ccc_uuid);
    gatts_add_desc_info_t descriptor;
    descriptor.desc_uuid = ccc_uuid;
    descriptor.permissions = GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE;
    descriptor.value_len = sizeof(ccc_val);
    descriptor.value = ccc_val;
    gatts_add_descriptor(server_id, srvc_handle, &descriptor);
    osal_printk("%s ccc_uuid:%2x %2x\n", BLE_OTA_SERVER_LOG, characters_uuid.uuid[0], characters_uuid.uuid[1]);
}


bool bts_ota_compare_uuid(bt_uuid_t *uuid1, bt_uuid_t *uuid2)
{
    if (uuid1->uuid_len != uuid2->uuid_len) {
        return false;
    }
    if (memcmp(uuid1->uuid, uuid2->uuid, uuid1->uuid_len) != 0) {
        return false;
    }
    return true;
}

/* 服务添加回调 */
static void ble_ota_server_service_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t handle, errcode_t status)
{
    osal_printk("%s add characters_and_descriptors cbk service:%d, srv_handle:%d, uuid_len:%d, status:%d, uuid:",
                BLE_OTA_SERVER_LOG, server_id, handle, uuid->uuid_len, status);
    for (int8_t i = 0; i < uuid->uuid_len; i++) {
        osal_printk("0x%02x ", uuid->uuid[i]);
    }
    osal_printk("\n");
    ble_ota_add_tx_characters_and_descriptors(server_id, handle);
    gatts_start_service(server_id, handle);
}

/* 特征添加回调 */
static void ble_ota_server_characteristic_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
                                                  gatts_add_character_result_t *result, errcode_t status)
{
    osal_printk("%s add character cbk service:%d service_hdl: %d char_hdl: %d char_val_hdl: %d uuid_len: %d \n",
                BLE_OTA_SERVER_LOG, server_id, service_handle, result->handle, result->value_handle, uuid->uuid_len);
    osal_printk("uuid:");
    for (int8_t i = 0; i < uuid->uuid_len; i++) {
        osal_printk("0x%02x ", uuid->uuid[i]);
    }
    bt_uuid_t characters_cbk_uuid = { 0 };
    bts_data_to_uuid_len2(BLE_OTA_CHARACTERISTIC_UUID_TX, &characters_cbk_uuid);
    characters_cbk_uuid.uuid_len = uuid->uuid_len;
    if (bts_ota_compare_uuid(uuid, &characters_cbk_uuid)) {
        g_notify_indicate_handle = result->value_handle;
    }
    osal_printk("%s status:%d indicate_handle:%d\n", BLE_OTA_SERVER_LOG, status, g_notify_indicate_handle);
}

/* 描述符添加回调 */
static void ble_ota_server_descriptor_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
                                              uint16_t handle, errcode_t status)
{
    osal_printk("%s service:%d service_hdl: %d desc_hdl: %d uuid_len: %d \n",
                BLE_OTA_SERVER_LOG, server_id, service_handle, handle, uuid->uuid_len);
    osal_printk("uuid:");
    for (int8_t i = 0; i < uuid->uuid_len; i++) {
        osal_printk("0x%02x ", (uint8_t)uuid->uuid[i]);
    }
    osal_printk("%s status:%d\n", BLE_OTA_SERVER_LOG, status);
}

errcode_t ble_ota_data_ack(uint16_t value_len, uint8_t *value)
{
    if (value_len == 0 || value == NULL) {
        return ERRCODE_BT_PARAM_ERR;
    }
    gatts_ntf_ind_t param = { 0 };
    uint16_t conn_id = g_ble_ota_conn_id;
    param.attr_handle = g_notify_indicate_handle;
    param.value_len = value_len;
    param.value = value;
    errcode_t ret = gatts_notify_indicate(BLE_OTA_SERVER_ID, conn_id, &param);
    if (ret != ERRCODE_BT_SUCCESS) {
        return ERRCODE_BT_FAIL;
    }
    return ERRCODE_BT_SUCCESS;
}

/* 开始服务回调 */
static void ble_ota_server_service_start_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    g_service_num++;
    if ((g_service_num == BLE_OTA_SERVICE_NUM) && (status == 0)) {
        osal_printk("%s start service cbk , start adv\n", BLE_OTA_SERVER_LOG);
        ble_ota_set_adv_data();
        ble_ota_start_adv();
    }
    diag_ble_ota_service_ack_register(ble_ota_data_ack);
    osal_printk("%s start service:%2d service_hdl: %d status: %d\n",
                BLE_OTA_SERVER_LOG, server_id, handle, status);
}

static void ble_ota_receive_write_req_cbk(uint8_t server_id, uint16_t conn_id, gatts_req_write_cb_t *write_cb_para,
                                          errcode_t status)
{
    unused(server_id);
    unused(conn_id);
    unused(status);
    if (write_cb_para->handle == g_notify_indicate_handle) {
        diag_channel_rx_bt_service_ota_data(write_cb_para->value, write_cb_para->length);
    }
}

static void ble_ota_receive_read_req_cbk(uint8_t server_id, uint16_t conn_id, gatts_req_read_cb_t *read_cb_para,
    errcode_t status)
{
    osal_printk("%s ReceiveReadReq--server_id:%d conn_id:%d\n", BLE_OTA_SERVER_LOG, server_id, conn_id);
    osal_printk("%s request_id:%d, att_handle:%d offset:%d, need_rsp:%d, is_long:%d\n",
                BLE_OTA_SERVER_LOG, read_cb_para->request_id, read_cb_para->handle, read_cb_para->offset,
                read_cb_para->need_rsp, read_cb_para->is_long);
    osal_printk("%s status:%d\n", BLE_OTA_SERVER_LOG, status);
}

static void ble_ota_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status)
{
    osal_printk("%s MtuChanged--server_id:%d conn_id:%d\n", BLE_OTA_SERVER_LOG, server_id, conn_id);
    osal_printk("%s mtusize:%d, status:%d\n", BLE_OTA_SERVER_LOG, mtu_size, status);
    gap_le_set_data_length_t param = {0};
    param.conn_handle = conn_id;
    param.maxtxoctets = MAXTXOCTETS;
    param.maxtxtime = MAXTXTIME;
    errcode_t ret = gap_ble_set_data_length(&param);
    if (ret != ERRCODE_BT_SUCCESS) {
        osal_printk("%s set data length failed ret = %d\n", BLE_OTA_SERVER_ERROR, ret);
    }
}

static void ble_ota_server_adv_enable_cbk(uint8_t adv_id, adv_status_t status)
{
    osal_printk("%s adv enable cbk adv_id:%d status:%d\n", BLE_OTA_SERVER_LOG, adv_id, status);
}

static void ble_ota_server_adv_disable_cbk(uint8_t adv_id, adv_status_t status)
{
    osal_printk("%s adv disable adv_id: %d, status:%d\n", BLE_OTA_SERVER_LOG, adv_id, status);
}

void ble_ota_server_connect_change_cbk(uint16_t conn_id, bd_addr_t *addr, gap_ble_conn_state_t conn_state,
                                       gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason)
{
    g_ble_ota_conn_id = conn_id;
    g_connection_state = (uint8_t)conn_state;
    osal_printk("%s connect state change conn_id: %d, status: %d, pair_status:%d, disc_reason 0x%x\n",
                BLE_OTA_SERVER_LOG, conn_id, conn_state, pair_state, disc_reason);
    osal_printk("addr:\n");
    for (uint8_t i = 0; i < BD_ADDR_LEN; i++) {
        osal_printk("%2x", addr->addr[i]);
    }
    osal_printk("\n");
    if (conn_state == GAP_BLE_STATE_CONNECTED) {
        return;
    } else if (conn_state == GAP_BLE_STATE_DISCONNECTED) {
        ble_ota_set_adv_data();
        ble_ota_start_adv();
    }
}
void ble_ota_server_pair_result_cb(uint16_t conn_id, const bd_addr_t *addr, errcode_t status)
{
    osal_printk("%s pair result conn_id: %d, status: %d\n",
                BLE_OTA_SERVER_LOG, conn_id, status);
    osal_printk("addr:\n");
    for (uint8_t i = 0; i < BD_ADDR_LEN; i++) {
        osal_printk("%2x", addr->addr[i]);
    }
    osal_printk("\n");
}

void ble_ota_server_power_on_cbk(uint8_t status)
{
    osal_printk("ble power on: %d\n", status);
    enable_ble();
}

void ble_ota_server_enable_cbk(uint8_t status)
{
    osal_printk("ble enable: %d\n", status);
    errcode_t ret = 0;
    bt_uuid_t app_uuid = { 0 };
    bd_addr_t ble_addr = { 0 };
    app_uuid.uuid_len = sizeof(g_ota_server_app_uuid);
    if (memcpy_s(app_uuid.uuid, app_uuid.uuid_len, g_ota_server_app_uuid, sizeof(g_ota_server_app_uuid)) != EOK) {
        osal_printk("%s add server app uuid memcpy failed\n", BLE_OTA_SERVER_ERROR);
        return;
    }
    ble_addr.type = BLE_PUBLIC_DEVICE_ADDRESS;
    if (memcpy_s(ble_addr.addr, BD_ADDR_LEN, g_ble_ota_server_addr, sizeof(g_ble_ota_server_addr)) != EOK) {
        osal_printk("%s add server app addr memcpy failed\n", BLE_OTA_SERVER_ERROR);
        return;
    }
    gap_ble_set_local_name(g_ble_ota_name_value, sizeof(g_ble_ota_name_value));
    gap_ble_set_local_addr(&ble_addr);
    ret = gatts_register_server(&app_uuid, &g_server_id);
    if ((ret != ERRCODE_BT_SUCCESS) || (g_server_id == INVALID_SERVER_ID)) {
        osal_printk("%s add server failed\r\n", BLE_OTA_SERVER_ERROR);
        return;
    }
    ble_ota_add_service(); /* 添加ota服务 */
    osal_printk("%s beginning add service\r\n", BLE_OTA_SERVER_LOG);
    bth_ota_init();
}

static errcode_t ble_ota_server_register_callbacks(void)
{
    bts_dev_manager_callbacks_t dev_mgr_cb = { 0 };
    gap_ble_callbacks_t gap_cb = { 0 };
    gatts_callbacks_t service_cb = { 0 };

    dev_mgr_cb.power_on_cb = ble_ota_server_power_on_cbk;
    dev_mgr_cb.ble_enable_cb = ble_ota_server_enable_cbk;
    gap_cb.start_adv_cb = ble_ota_server_adv_enable_cbk;
    gap_cb.conn_state_change_cb = ble_ota_server_connect_change_cbk;
    gap_cb.stop_adv_cb = ble_ota_server_adv_disable_cbk;
    gap_cb.pair_result_cb = ble_ota_server_pair_result_cb;
    errcode_t ret = gap_ble_register_callbacks(&gap_cb);
    ret |= bts_dev_manager_register_callbacks(&dev_mgr_cb);
    if (ret != ERRCODE_BT_SUCCESS) {
        osal_printk("%s reg gap cbk failed ret = %d\n", BLE_OTA_SERVER_ERROR, ret);
        return ret;
    }

    service_cb.add_service_cb = ble_ota_server_service_add_cbk;
    service_cb.add_characteristic_cb = ble_ota_server_characteristic_add_cbk;
    service_cb.add_descriptor_cb = ble_ota_server_descriptor_add_cbk;
    service_cb.start_service_cb = ble_ota_server_service_start_cbk;
    service_cb.read_request_cb = ble_ota_receive_read_req_cbk;
    service_cb.write_request_cb = ble_ota_receive_write_req_cbk;
    service_cb.mtu_changed_cb = ble_ota_mtu_changed_cbk;
    ret = gatts_register_callbacks(&service_cb);
    if (ret != ERRCODE_BT_SUCCESS) {
        osal_printk("%s reg service cbk failed ret = %d\n", BLE_OTA_SERVER_ERROR, ret);
        return ret;
    }
#if (CORE_NUMS < 2)
    enable_ble();
#endif
    return ret;
}

void ble_ota_server_init(void)
{
    ble_ota_server_register_callbacks();
}

/* device向host发送数据：input report */
errcode_t ble_ota_server_send_input_report(uint8_t *data, uint16_t len)
{
    gatts_ntf_ind_t param = { 0 };
    uint16_t conn_id = g_ble_ota_conn_id;
    param.attr_handle = g_notify_indicate_handle;
    param.value_len = len;
    param.value = data;
    osal_printk("%s send input report indicate_handle:%d\n", BLE_OTA_SERVER_LOG, g_notify_indicate_handle);
    gatts_notify_indicate(BLE_OTA_SERVER_ID, conn_id, &param);
    return ERRCODE_BT_SUCCESS;
}

uint8_t ble_ota_get_connection_state(void)
{
    return g_connection_state;
}