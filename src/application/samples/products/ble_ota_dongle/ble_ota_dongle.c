/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE OTA Dongle Source. \n
 *
 * History: \n
 * 2024-02-01, Create file. \n
 */
#include "securec.h"
#include "chip_io.h"
#include "common_def.h"
#include "app_init.h"
#include "gadget/f_hid.h"
#include "osal_debug.h"
#include "osal_task.h"
#include "osal_addr.h"
#include "implementation/usb_init.h"
#include "sle_connection_manager.h"
#include "sle_ssap_client.h"
#include "ble_ota_client.h"
#include "ble_ota_hid.h"
#include "ble_ota_cmd.h"
#include "ble_ota_cmd_handler.h"

#define BLE_OTA_DONGLE_TASK_STACK_SIZE      0x1000
#define BLE_OTA_DONGLE_TASK_PRIO            24
#define BLE_OTA_DONGLE_TASK_DELAY_MS        2000
#define USB_HID_INIT_DELAY_MS               (500UL)
#define USB_KEYBOARD_REPORTER_LEN           9
#define USB_MOUSE_REPORTER_LEN              5
#define USB_CONSUMER_REPORTER_LEN           3
#define BLE_KRYBOARD_USB_MANUFACTURER       { 'H', 0, 'H', 0, 'H', 0, 'H', 0, 'l', 0, 'i', 0, 'c', 0, 'o', 0, 'n', 0 }
#define BLE_KRYBOARD_USB_MANUFACTURER_LEN   20
#define BLE_KRYBOARD_USB_PRODUCT    { 'H', 0, 'H', 0, '6', 0, '6', 0, '6', 0, '6', 0, ' ', 0, 'U', 0, 'S', 0, 'B', 0 }
#define BLE_KRYBOARD_USB_PRODUCT_LEN        22
#define BLE_KRYBOARD_USB_SERIAL             { '2', 0, '0', 0, '2', 0, '0', 0, '0', 0, '6', 0, '2', 0, '4', 0 }
#define BLE_KRYBOARD_USB_SERIAL_LEN         16
#define RECV_MAX_LENGTH                     256
#define USB_RECV_STACK_SIZE                 0x800
#define BLE_OTA_RESPONSE_HEADER_LEN         26
#define BLE_OTA_RESPONSE_LEN                128
#define BLE_OTA_RPT_DATA_LEN_INDEX          24
#define BLE_OTA_DONGLE_LOG                  "[ble ota dongle]"
#define BLE_OTA_8_BIT_SHIFT                 8
#define BLE_OTA_RPT_SERVICEID               0X03
#define BLE_OTA_RPT_COMMANDID               0X07

static bool g_ble_ota_dongle_inited = false;
static uint32_t g_ble_ota_dongle_hid_keyboard_index = 0;
static uint8_t g_ble_ota_rpt_body[BLE_OTA_RESPONSE_LEN] = { 0x1, 0x1, 0x0, 0x0, 0x2, 0x2, 0x0, 0xa, 0x0, 0x3,
                                                            0x2, 0x0, 0xa, 0x0, 0x4, 0x1, 0x0, 0x0, 0x5, 0x2,
                                                            0x0, 0xd, 0x0, 0x6, 0x0, 0x0};

static uint8_t ble_ota_dongle_init_internal(device_type dtype)
{
    if (g_ble_ota_dongle_inited) {
        return BLE_OTA_DONGLE_OK;
    }

    const char manufacturer[BLE_KRYBOARD_USB_MANUFACTURER_LEN] = BLE_KRYBOARD_USB_MANUFACTURER;
    struct device_string str_manufacturer = {
        .str = manufacturer,
        .len = BLE_KRYBOARD_USB_MANUFACTURER_LEN
    };

    const char product[BLE_KRYBOARD_USB_PRODUCT_LEN] = BLE_KRYBOARD_USB_PRODUCT;
    struct device_string str_product = {
        .str = product,
        .len = BLE_KRYBOARD_USB_PRODUCT_LEN
    };

    const char serial[BLE_KRYBOARD_USB_SERIAL_LEN] = BLE_KRYBOARD_USB_SERIAL;
    struct device_string str_serial_number = {
        .str = serial,
        .len = BLE_KRYBOARD_USB_SERIAL_LEN
    };

    struct device_id dev_id = {
        .vendor_id = 0x1111,
        .product_id = 0x0009,
        .release_num = 0x0800
    };

    if (dtype == DEV_HID) {
        g_ble_ota_dongle_hid_keyboard_index = ble_ota_dongle_set_keyboard_report_desc_hid();
        ble_ota_set_hid_index(g_ble_ota_dongle_hid_keyboard_index);
    }

    if (usbd_set_device_info(dtype, &str_manufacturer, &str_product, &str_serial_number, dev_id) != 0) {
        osal_printk("%s set device info fail!\r\n", BLE_OTA_DONGLE_LOG);
        return BLE_OTA_DONGLE_FAILED;
    }

    if (usb_init(DEVICE, dtype) != 0) {
        osal_printk("%s usb_init failed!\r\n", BLE_OTA_DONGLE_LOG);
        return BLE_OTA_DONGLE_FAILED;
    }
    g_ble_ota_dongle_inited = true;
    return BLE_OTA_DONGLE_OK;
}

static uint8_t ble_ota_dongle_init(void)
{
    if (!g_ble_ota_dongle_inited) {
        if (ble_ota_dongle_init_internal(DEV_HID) != BLE_OTA_DONGLE_OK) {
            return BLE_OTA_DONGLE_FAILED;
        }
        osal_msleep(USB_HID_INIT_DELAY_MS);
    }
    return BLE_OTA_DONGLE_OK;
}

static void ble_ota_fhid_send_data_one(uint8_t service_id, uint8_t command_id, uint16_t body_len, uint8_t *body)
{
    uint8_t send_buff[BLE_LINK_MAX_FRAME_LEN] = {0};
    ble_ota_frame_header_t *head = (ble_ota_frame_header_t *)&send_buff;
    head->flag = BLE_LINK_FRAME_HEAD_FLAG;
    head->version = 0;
    head->total_frame = 1;
    head->frame_seq = 0;
    head->service_id = service_id;
    head->command_id = command_id;
    head->body_len[0] = body_len;
    head->body_len[1]= body_len >> BLE_OTA_8_BIT_SHIFT;
    /* 不需要分帧发送 */
    uint8_t *body_buff = (uint8_t *)((uintptr_t)send_buff + sizeof(ble_ota_frame_header_t));
    if (memcpy_s(body_buff, BLE_LINK_FRAME_BODY_MAX_LEN, body, body_len) != EOK) {
        osal_printk("%s memcpy body fail.\r\n", BLE_OTA_DONGLE_LOG);
        return;
    }
    uint16_t send_len = body_len + BLE_LINK_FRAME_HEAD_LEN + BLE_LINK_FRAME_PAYLOAD_HEAD_LEN + BLE_LINK_FRAME_MIC_LEN;
    int32_t ret = fhid_send_data(ble_ota_get_hid_index(), (char *)send_buff, send_len);
    if (ret == -1) {
        osal_printk("%s hid send data falied! ret:%d\n", BLE_OTA_DONGLE_LOG, ret);
        return;
    }
}

static void ble_ota_fhid_send_data_more(uint8_t service_id, uint8_t command_id, uint16_t body_len, uint8_t *body)
{
    uint8_t send_buff[BLE_LINK_MAX_FRAME_LEN] = {0};
    ble_ota_frame_header_t *head = (ble_ota_frame_header_t *)&send_buff;
    head->flag = BLE_LINK_FRAME_HEAD_FLAG;
    head->version = 0;
    head->service_id = service_id;
    head->command_id = command_id;
    head->body_len[0] = body_len;
    head->body_len[1]= body_len >> BLE_OTA_8_BIT_SHIFT;
    /* 分帧发送 */
    uint16_t payload_len_send = body_len + BLE_LINK_FRAME_PAYLOAD_HEAD_LEN;
    head->total_frame = ((payload_len_send % BLE_LINK_FRAME_PAYLOAD_MAX_LEN) ? 1 : 0)
                        + (payload_len_send / BLE_LINK_FRAME_PAYLOAD_MAX_LEN);
    /* 第一帧 */
    head->frame_seq = 0;
    uint8_t *body_buff = (uint8_t *)((uintptr_t)send_buff + sizeof(ble_ota_frame_header_t));
    if (memcpy_s(body_buff, BLE_LINK_FRAME_BODY_MAX_LEN, body, BLE_LINK_FRAME_BODY_MAX_LEN) != EOK) {
        osal_printk("%s memcpy body fail.\r\n", BLE_OTA_DONGLE_LOG);
        return;
    }
    int32_t ret = fhid_send_data(ble_ota_get_hid_index(), (char *)send_buff, BLE_LINK_MAX_FRAME_LEN);
    if (ret == -1) {
        osal_printk("%s hid send data(seq %d) falied! ret:%d\n", BLE_OTA_DONGLE_LOG, head->frame_seq, ret);
        return;
    }
    uint16_t body_offset = BLE_LINK_FRAME_BODY_MAX_LEN;
    uint16_t left_body_len = body_len - BLE_LINK_FRAME_BODY_MAX_LEN;
    /* 剩余帧 */
    for (uint8_t i = 1; i < head->total_frame ; i++) {
        head->frame_seq = i;
        uint8_t *payload_buff = (uint8_t *)((uintptr_t)send_buff + BLE_LINK_FRAME_HEAD_LEN);
        uint8_t *payload_data = (uint8_t *)((uintptr_t)body + body_offset);
        uint16_t body_send_len = (left_body_len > BLE_LINK_FRAME_BODY_MAX_LEN) ?
                                  BLE_LINK_FRAME_BODY_MAX_LEN : left_body_len;
        (void)memset_s(payload_buff, BLE_LINK_FRAME_PAYLOAD_MAX_LEN, 0, BLE_LINK_FRAME_PAYLOAD_MAX_LEN);
        if (memcpy_s(payload_buff, BLE_LINK_FRAME_PAYLOAD_MAX_LEN, payload_data, body_send_len) != EOK) {
            osal_printk("%s memcpy %d seq of body fail.\r\n", BLE_OTA_DONGLE_LOG, head->frame_seq);
            return;
        }
        uint16_t frame_send_len = body_send_len + BLE_LINK_FRAME_MIC_LEN + BLE_LINK_FRAME_HEAD_LEN;
        ret = fhid_send_data(ble_ota_get_hid_index(), (char *)send_buff, frame_send_len);
        if (ret == -1) {
            osal_printk("%s hid send data(seq %d) falied! ret:%d\n", BLE_OTA_DONGLE_LOG, head->frame_seq, ret);
            return;
        }
        body_offset += body_send_len;
        left_body_len -= body_send_len;
    }
}

static void ble_ota_notification_cb(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data, errcode_t status)
{
    unused(client_id);
    unused(conn_id);
    unused(status);
    if (data == NULL || data->data_len == 0 || data->data == NULL) {
        return;
    }

    g_ble_ota_rpt_body[BLE_OTA_RPT_DATA_LEN_INDEX] = data->data_len;
    g_ble_ota_rpt_body[BLE_OTA_RPT_DATA_LEN_INDEX + 1] = data->data_len >> BLE_OTA_8_BIT_SHIFT;
    if (data->data_len > (sizeof(g_ble_ota_rpt_body) - BLE_OTA_RESPONSE_HEADER_LEN)) {
        osal_printk("%s data length exceeds buffer limit!\r", BLE_OTA_DONGLE_LOG);
        return;
    }
    for (uint32_t i = 0; i < data->data_len; i++) {
        g_ble_ota_rpt_body[BLE_OTA_RESPONSE_HEADER_LEN + i] = data->data[i];
    }
    uint16_t body_len = BLE_OTA_RESPONSE_HEADER_LEN + data->data_len;
    if (body_len <= BLE_LINK_FRAME_BODY_MAX_LEN) {
        ble_ota_fhid_send_data_one(BLE_OTA_RPT_SERVICEID, BLE_OTA_RPT_COMMANDID, body_len, g_ble_ota_rpt_body);
        return;
    }
    ble_ota_fhid_send_data_more(BLE_OTA_RPT_SERVICEID, BLE_OTA_RPT_COMMANDID, body_len, g_ble_ota_rpt_body);
}

static void ble_ota_indication_cb(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data, errcode_t status)
{
    unused(client_id);
    unused(conn_id);
    unused(status);
    if (data == NULL || data->data_len == 0 || data->data == NULL) {
        return;
    }
}

static void *ble_ota_dongle_task(const char *arg)
{
    unused(arg);
    uint8_t ret;

    osal_printk("%s enter ble_ota_dongle_task\r\n", BLE_OTA_DONGLE_LOG);
    /* ble ota dongle init */
    ret = ble_ota_dongle_init();
    if (ret != BLE_OTA_DONGLE_OK) {
        osal_printk("%s ble_ota_dongle_init fail! ret = %d\r\n", BLE_OTA_DONGLE_LOG, ret);
    }

    /* ble ota client init */
    ble_ota_client_init(ble_ota_notification_cb, ble_ota_indication_cb);
    /* delay for param update complete */
    osal_msleep(BLE_OTA_DONGLE_TASK_DELAY_MS);
    uint8_t recv_hid_data[RECV_MAX_LENGTH];
    while (1) {
        int32_t ret = fhid_recv_data(g_ble_ota_dongle_hid_keyboard_index, (char*)recv_hid_data, RECV_MAX_LENGTH);
        if (ret <= 0) {
            continue;
        }
        ble_ota_recv_handler(recv_hid_data, ret);
    }
    return NULL;
}

static void ble_ota_dongle_entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)ble_ota_dongle_task, 0, "SleOtaDongleTask",
                                      BLE_OTA_DONGLE_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, BLE_OTA_DONGLE_TASK_PRIO);
    }
    osal_kthread_unlock();
}

/* Run the ble_ota_dongle_entry. */
app_run(ble_ota_dongle_entry);