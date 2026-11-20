/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023. All rights reserved.
 * Description: sle ota server.
 */
#include "securec.h"
#include "errcode.h"
#include "osal_addr.h"
#include "sle_common.h"
#include "sle_errcode.h"
#include "sle_ssap_server.h"
#include "dfx_channel.h"
#include "osal_debug.h"
#include "sle_rcu_ota_server.h"

#define OCTET_BIT_LEN   8
#define UUID_LEN_2      2

#undef THIS_FILE_ID
#define THIS_FILE_ID BTH_SLE_OTA
#define SLE_OTA_SERVER_LOG "[sle ota server]"
#define encode2byte_little(_ptr, data) \
    do { \
        *(uint8_t *)((_ptr) + 1) = (uint8_t)((data) >> 8); \
        *(uint8_t *)(_ptr) = (uint8_t)(data); \
    } while (0)

/* sle server id */
uint8_t g_ota_server_id = 1;
/* sle ota service handle */
uint16_t g_ota_service_handle = 0;
/* sle ota property handle */
uint16_t g_ota_property_handle = 0xFFFF;
/* sle ota connect id */
uint16_t g_ota_conn_id = 0;

char sle_ota_app_uuid[UUID_LEN_2] = {0x0, 0x0};
uint8_t g_desc_value[] = {0x01, 0x00};
char ota_property_value[OCTET_BIT_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

static uint8_t sle_uuid_base[] = { 0x37, 0xBE, 0xA8, 0x80, 0xFC, 0x70, 0x11, 0xEA, \
                                   0xB7, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static void sle_uuid_setu2(uint16_t u2, sle_uuid_t *out)
{
    (void)memcpy_s(out->uuid, SLE_UUID_LEN, sle_uuid_base, SLE_UUID_LEN);
    out->len = UUID_LEN_2;
    encode2byte_little(&out->uuid[14], u2);
}

void sle_ota_receive_data(uint16_t conn_id, uint16_t request_id, uint16_t data_len, uint8_t *data)
{
    ssaps_send_rsp_t rsp_param = { 0 };
    g_ota_conn_id = conn_id;
    diag_channel_rx_sle_data(data, data_len);
    rsp_param.request_id = request_id;
    rsp_param.status = 0;
    rsp_param.value_len = 0;
    if (ssaps_send_response(g_ota_server_id, conn_id, &rsp_param) != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s ota receive data rsp fail!\r\n", SLE_OTA_SERVER_LOG);
    };
}

errcode_t sle_ota_data_ack(uint16_t value_len, uint8_t *value)
{
    if (value_len == 0 || value == NULL) {
        osal_printk("%s ota data ack param error\r\n", SLE_OTA_SERVER_LOG);
        return ERRCODE_SLE_PARAM_ERR;
    }
    ssaps_ntf_ind_t param = {0};
    param.value = value;
    param.handle = g_ota_property_handle;
    param.type = SSAP_PROPERTY_TYPE_VALUE;
    param.value_len = value_len;

    errcode_t ret = ssaps_notify_indicate(g_ota_server_id, g_ota_conn_id, &param);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s ota data ack failed\r\n", SLE_OTA_SERVER_LOG);
        return ERRCODE_SLE_FAIL;
    }
    return ERRCODE_SLE_SUCCESS;
}

static void sle_ota_ssaps_write_req_cbk(uint8_t server_id, uint16_t conn_id, ssaps_req_write_cb_t *write_cb_para,
                                        errcode_t status)
{
    unused(server_id);
    unused(conn_id);
    unused(status);
    if (write_cb_para->handle == g_ota_property_handle) {
        sle_ota_receive_data(conn_id, write_cb_para->request_id, write_cb_para->length, write_cb_para->value);
    }
}

static errcode_t sle_ota_server_service_add(void)
{
    errcode_t ret;
    sle_uuid_t service_uuid = {0};
    bool is_primary = true;
    sle_uuid_setu2(SLE_OTA_SERVER_SERVICE, &service_uuid);
    ssaps_callbacks_t ota_srv_cbk = {
        .write_request_cb      = sle_ota_ssaps_write_req_cbk,
    };
    ret = ssaps_register_multi_callbacks(g_ota_server_id, &service_uuid, &ota_srv_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s ota server init failed,reg callback ret:%u,server id:%u\n", SLE_OTA_SERVER_LOG, ret, g_ota_server_id);
        return ret;
    }
    ret = ssaps_add_service_sync(g_ota_server_id, &service_uuid, is_primary, &g_ota_service_handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s sle ota add service fail, ret:%x\r\n", SLE_OTA_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    diag_sle_ota_service_ack_register(sle_ota_data_ack);
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_ota_server_property_add(void)
{
    errcode_t ret;
    ssaps_property_info_t property = {0};

    property.permissions = SLE_OTA_PROPERTY_PERMISSION;
    property.operate_indication = SLE_OTA_OPINDICAT;
    sle_uuid_setu2(SLE_OTA_SERVER_PROPERTY, &property.uuid);
    property.value = osal_vmalloc(sizeof(ota_property_value));
    if (property.value == NULL) {
        osal_printk("%s sle property mem fail\r\n", SLE_OTA_SERVER_LOG);
        return ERRCODE_SLE_FAIL;
    }
    if (memcpy_s(property.value, sizeof(ota_property_value), ota_property_value,
        sizeof(ota_property_value)) != EOK) {
        osal_vfree(property.value);
        osal_printk("%s sle property mem cpy fail\r\n", SLE_OTA_SERVER_LOG);
        return ERRCODE_SLE_FAIL;
    }
    ret = ssaps_add_property_sync(g_ota_server_id, g_ota_service_handle, &property,  &g_ota_property_handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s sle ota add property fail, ret:%x\r\n", SLE_OTA_SERVER_LOG, ret);
        osal_vfree(property.value);
        return ERRCODE_SLE_FAIL;
    }

    ssaps_desc_info_t desc = {0};
    desc.permissions = SLE_OTA_DESC_PERMISSION;
    desc.operate_indication = SLE_OTA_DESC_OPINDICAT;
    desc.type = SSAP_DESCRIPTOR_CLIENT_CONFIGURATION;
    desc.value_len = sizeof(g_desc_value);
    desc.value = g_desc_value;

    ret = ssaps_add_descriptor_sync(g_ota_server_id, g_ota_service_handle, g_ota_property_handle, &desc);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s sle ota add descriptor fail, ret:%x\r\n", SLE_OTA_SERVER_LOG, ret);
        osal_vfree(property.value);
        return ERRCODE_SLE_FAIL;
    }
    osal_vfree(property.value);
    return ERRCODE_SLE_SUCCESS;
}

static errcode_t sle_ota_service_start(void)
{
    errcode_t ret;
    osal_printk("%s sle uuid add service in\r\n", SLE_OTA_SERVER_LOG);
    if (sle_ota_server_service_add() != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(g_ota_server_id);
        return ERRCODE_SLE_FAIL;
    }
    if (sle_ota_server_property_add() != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(g_ota_server_id);
        return ERRCODE_SLE_FAIL;
    }

    ret = ssaps_start_service(g_ota_server_id, g_ota_service_handle);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s sle ota add service fail, ret:%x\r\n", SLE_OTA_SERVER_LOG, ret);
        return ERRCODE_SLE_FAIL;
    }
    osal_printk("%s sle ota add service out\r\n", SLE_OTA_SERVER_LOG);
    return ERRCODE_SLE_SUCCESS;
}

/* 初始化ota service */
errcode_t sle_ota_service_init(uint8_t server_id)
{
    g_ota_server_id = server_id;
    sle_ota_service_start();
    osal_printk("%s init ok\r\n", SLE_OTA_SERVER_LOG);
    return ERRCODE_SLE_SUCCESS;
}