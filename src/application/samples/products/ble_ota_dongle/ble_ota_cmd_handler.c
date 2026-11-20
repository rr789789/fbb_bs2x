/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE OTA Recv Source. \n
 *
 * History: \n
 * 2024-02-01, Create file. \n
 */
#include "securec.h"
#include "common_def.h"
#include "bts_le_gap.h"
#include "bts_gatt_client.h"
#include "bts_device_manager.h"
#include "ble_ota_client.h"
#include "gadget/f_hid.h"
#include "ble_ota_cmd.h"
#include "osal_debug.h"
#include "osal_task.h"
#include "ble_ota_cmd_handler.h"

#define BT_INDEX_0                 0
#define BT_INDEX_1                 1
#define BT_INDEX_2                 2
#define BT_INDEX_3                 3
#define BT_INDEX_4                 4
#define BT_INDEX_5                 5

#define WRITE_REQ_DATA_LEN_INDEX    33
#define WRITE_REQ_DATA_INDEX        35
#define DEVICE_ADDR_RPT_LEN         24
#define RPT_TYPE_INDEX              12
#define RPT_ADDR_INDEX              16
#define BLE_OTA_CONNECT_DELAY        500
#define DIAG_FRAME_DATA_OFFSET      4
#define FRAME_8_BIT_SHIFT           8
#define CONNECT_TRY_TIME            20

#pragma pack(1)
struct ble_link_frame {
    uint8_t flag;
    uint8_t version;
    uint8_t total_frame;
    uint8_t frame_seq;
    uint8_t rsv;
    uint8_t service_id;
    uint8_t command_id;
    uint16_t body_len;
    uint8_t body[0];
} ble_link_frame_t;

struct ble_link_frame_tlv {
    uint8_t type;
    uint16_t length;
    uint8_t value[0];
} ble_link_frame_tlv_t;
#pragma pack()

static uint32_t g_ble_ota_hid_index = 0;
static uint8_t g_ble_ota_rpt[DEVICE_ADDR_RPT_LEN] = { BLE_LINK_FRAME_HEAD_FLAG, 0x0, 0x0, 0x0, 0x0,
                                                      0x1, 0x4, 0xd, 0x0, 0x1, 0x1, 0x0, 0x0, 0x2, 0x6,
                                                      0x0, 0x0 };

void ble_ota_set_hid_index(uint32_t index)
{
    g_ble_ota_hid_index = index;
}

uint32_t ble_ota_get_hid_index(void)
{
    return g_ble_ota_hid_index;
}

static void ble_ota_send_common_response(uint8_t service_id, uint8_t command_id, errcode_t ret)
{
    uint8_t ret_code = 0;
    if (ret != ERRCODE_SUCC) {
        ret_code = 1;
    }
    ble_ota_response_frame_t response = { 0 };
    response.flag = BLE_LINK_FRAME_HEAD_FLAG;
    response.service_id = service_id;
    response.command_id = command_id;
    response.body_len[0] = 0x04;
    response.body_len[1] = 0x00;
    response.type = 0x7F;
    response.len[0] = 0x01;
    response.len[1] = 0x00;
    response.errorcode = ret_code;
    fhid_send_data(g_ble_ota_hid_index, (char *)&response, sizeof(ble_ota_response_frame_t));
}

static errcode_t ble_ota_enable_ble(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_disable_ble(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = disable_ble();
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_set_device_addr(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(length);
    unused(buffer);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_get_device_addr(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(service_id);
    unused(command_id);
    unused(buffer);
    unused(length);
    bd_addr_t local_address = { 0 };
    gap_ble_set_local_addr(&local_address);
    g_ble_ota_rpt[RPT_TYPE_INDEX] = local_address.type;
    for (uint8_t i = 0; i < BLE_ADDR_LEN; i++) {
        g_ble_ota_rpt[RPT_ADDR_INDEX + i] = (uint8_t)local_address.addr[i];
    }
    fhid_send_data(g_ble_ota_hid_index, (char *)g_ble_ota_rpt, DEVICE_ADDR_RPT_LEN);
    return ERRCODE_SUCC;
}

static errcode_t ble_ota_set_device_name(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_get_device_name(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_set_announce_data(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_start_announce(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_end_announce(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_set_scan_param(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_upload_scan_result(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_device_start_scan(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    set_ble_ota_scan_state(true);
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_stop_scan(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    set_ble_ota_scan_state(false);
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_send_data(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    uint16_t len = (buffer[WRITE_REQ_DATA_LEN_INDEX]) + (buffer[WRITE_REQ_DATA_LEN_INDEX + 1] << FRAME_8_BIT_SHIFT);
    uint8_t *data = buffer + WRITE_REQ_DATA_INDEX;
    gattc_handle_value_t *ble_micro_send_param = get_ble_ota_send_param();
    ble_micro_send_param->data_len = len;
    ble_micro_send_param->data = data;
    gattc_write_cmd(get_ble_ota_client_id(), get_ble_ota_conn_id(), ble_micro_send_param);
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

errcode_t ble_ota_discover_device_service(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    errcode_t ret = ERRCODE_SUCC;
    switch (command_id) {
        case COMMAND_ID_ENABLE_SLE:
            ret = ble_ota_enable_ble(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_DISABLE:
            ret = ble_ota_disable_ble(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_SET_DEVICE_ADDR:
            ret = ble_ota_set_device_addr(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_GET_DEVICE_ADDR:
            ret = ble_ota_get_device_addr(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_SET_DEVICE_NAME:
            ret = ble_ota_set_device_name(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_GET_DEVICE_NAME:
            ret = ble_ota_get_device_name(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_SET_ANNOUNCE_DATA:
            ret = ble_ota_set_announce_data(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_SET_START_ANNOUNCE:
            ret = ble_ota_start_announce(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_SET_END_ANNOUNCE:
            ret = ble_ota_end_announce(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_SET_SCAN_PARAM:
            ret = ble_ota_set_scan_param(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_UPLOAD_SCAN_RESULT:
            ret = ble_ota_upload_scan_result(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_START_SCAN:
            ret = ble_ota_device_start_scan(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_STOP_SCAN:
            ret = ble_ota_stop_scan(service_id, command_id, buffer, length);
            break;
        default:
            break;
    }
    return ret;
}

static errcode_t ble_ota_connect(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    uint8_t delay_num = 0;
    bd_addr_t connect_addr = {0};
    struct ble_link_frame *ble_link_frame = (struct ble_link_frame *)buffer;
    struct ble_link_frame_tlv *ble_link_body = (struct ble_link_frame_tlv *)ble_link_frame->body;
    uint16_t body_offset = 0;
    while (body_offset < ble_link_frame->body_len) {
        if (ble_link_body->type == 0x01) {
            connect_addr.type = ble_link_body->value[0];
            osal_printk("tyep %d ", connect_addr.type);
        } else if (ble_link_body->type == 0x02) {
            memcpy_s(connect_addr.addr, sizeof(connect_addr.addr), ble_link_body->value, sizeof(connect_addr.addr));
            osal_printk("addr %02x:%02x:%02x:%02x:%02x:%02x\r\n", connect_addr.addr[BT_INDEX_0],
                connect_addr.addr[BT_INDEX_1], connect_addr.addr[BT_INDEX_2], connect_addr.addr[BT_INDEX_3],
                connect_addr.addr[BT_INDEX_4], connect_addr.addr[BT_INDEX_5]);
        }

        body_offset = body_offset + offsetof(struct ble_link_frame_tlv, value) + ble_link_body->length;
        ble_link_body = (struct ble_link_frame_tlv *)&ble_link_frame->body[body_offset];
    }

    gap_ble_connect_remote_device(&connect_addr);
    osal_printk("ota connect addr %d,%02x:%02x:%02x:%02x:%02x:%02x\r\n", connect_addr.type,
                connect_addr.addr[BT_INDEX_0], connect_addr.addr[BT_INDEX_1], connect_addr.addr[BT_INDEX_2],
                connect_addr.addr[BT_INDEX_3], connect_addr.addr[BT_INDEX_4], connect_addr.addr[BT_INDEX_5]);
    set_ble_ota_set_address_state(true);
    while (get_ssap_find_ready() == 0) {
        osal_msleep(BLE_OTA_CONNECT_DELAY);
        if ((delay_num++) >= CONNECT_TRY_TIME) {
            osal_printk("connect timeout ");
            set_ble_ota_set_address_state(false);
            return ret;
        }
    }
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

static errcode_t ble_ota_disconnect(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(service_id);
    unused(command_id);
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    bd_addr_t connect_addr = {0};
    struct ble_link_frame *ble_link_frame = (struct ble_link_frame *)buffer;
    struct ble_link_frame_tlv *ble_link_body = (struct ble_link_frame_tlv *)ble_link_frame->body;
    uint16_t body_offset = 0;
    while (body_offset < ble_link_frame->body_len) {
        if (ble_link_body->type == 0x01) {
            connect_addr.type = ble_link_body->value[0];
            osal_printk("tyep %d ", connect_addr.type);
        } else if (ble_link_body->type == 0x02) {
            memcpy_s(connect_addr.addr, sizeof(connect_addr.addr), ble_link_body->value, sizeof(connect_addr.addr));
            osal_printk("addr %02x:%02x:%02x:%02x:%02x:%02x\r\n", connect_addr.addr[BT_INDEX_0],
                connect_addr.addr[BT_INDEX_1], connect_addr.addr[BT_INDEX_2], connect_addr.addr[BT_INDEX_3],
                connect_addr.addr[BT_INDEX_4], connect_addr.addr[BT_INDEX_5]);
        }

        body_offset = body_offset + offsetof(struct ble_link_frame_tlv, value) + ble_link_body->length;
        ble_link_body = (struct ble_link_frame_tlv *)&ble_link_frame->body[body_offset];
    }
    gap_ble_disconnect_remote_device(&connect_addr);
    set_ble_ota_set_address_state(false);
    ble_ota_send_common_response(service_id, command_id, ret);
    return ret;
}

errcode_t ble_ota_get_conn_status(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    ble_ota_get_conn_status_rsp_t response = { 0 };
    response.flag = BLE_LINK_FRAME_HEAD_FLAG;
    response.service_id = service_id;
    response.command_id = command_id;
    /*
    |   1Byte   |   2Bytes   | 1Bytes |
    |   Type    |   Length   | Value  |
    */
    /* length of Type Length and Value */
    response.body_len[0] = 0x04;
    response.body_len[1] = 0x00;
    /* type of conn_status */
    response.type = 0x01;
    /* length of Value */
    response.len[0] = 0x01;
    response.len[1] = 0x00;
    response.conn_status = get_ble_ota_conn_state();
    int32_t fhid_ret = fhid_send_data(g_ble_ota_hid_index, (char *)&response, sizeof(ble_ota_get_conn_status_rsp_t));
    if (fhid_ret == -1) {
        osal_printk("send link manager response falied! ret:%d\n", fhid_ret);
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}

errcode_t ble_ota_manage_connection_service(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    switch (command_id) {
        case COMMAND_ID_SEND_LINK_REQ:
            ret = ble_ota_connect(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_BREAK_LINK_REQ:
            ret = ble_ota_disconnect(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_UPG_LINK_PARAM:
        case COMMAND_ID_SEND_BOUND_REQ:
        case COMMAND_ID_DELETE_BOUND:
        case COMMAND_ID_DELETE_ALL_BOUND:
        case COMMAND_ID_GET_BOUND_DEVICE_NUM:
        case COMMAND_ID_GET_BOUND_DEVICE:
        case COMMAND_ID_GET_BOUND_STATUS:
        case COMMAND_ID_GET_DEVICE_RSSI:
        case COMMAND_ID_GET_ACB_PARAM:
        case COMMAND_ID_SET_PHY_PARAM:
        case COMMAND_ID_SET_POWRER_MAXIMUM:
        case COMMAND_ID_SEND_LINK_MANAGEMENT_CHECK:
            break;
        case COMMAND_ID_GET_CONN_STATUS:
            ret = ble_ota_get_conn_status(service_id, command_id, buffer, length);
            break;
        default:
            break;
    }
    return ret;
}

errcode_t ble_ota_manage_ssap_client(uint8_t service_id, uint8_t command_id, uint8_t *buffer, uint16_t length)
{
    unused(buffer);
    unused(length);
    errcode_t ret = ERRCODE_SUCC;
    switch (command_id) {
        case COMMAND_ID_REGISTER_CLIENT:
        case COMMAND_ID_UNREGISTER_CLIENT:
        case COMMAND_ID_SEARCH_SERVER_DESCRIPTION:
        case COMMAND_ID_UUID_READ_REQ:
        case COMMAND_ID_HANDLE_READ_REQ:
        case COMMAND_ID_WRITE_REQ:
        case COMMAND_ID_WRITE_CMD:
            ret = ble_ota_send_data(service_id, command_id, buffer, length);
            break;
        case COMMAND_ID_EXCHANGE_INFO_REQ:
            break;
        default:
            break;
    }
    return ret;
}
