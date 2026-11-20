/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE mouse sample. \n
 *
 * History: \n
 * 2023-08-01, Create file. \n
 * 2024-09-12, Modify file with profiles. \n
 */

#include <stdint.h>
#include "securec.h"
#include "errcode.h"
#include "product.h"
#include "soc_osal.h"
#include "osal_debug.h"
#include "osal_addr.h"
#include "sle_common.h"
#include "sle_errcode.h"
#include "bts_le_gap.h"
#include "common_def.h"
#include "sle_device_manager.h"
#include "sle_connection_manager.h"
#include "sle_device_discovery.h"
#include "sle_ssap_server.h"
#include "sle_phone_server_adv.h"
#include "sle_ssap_client.h"
#include "sle_low_latency.h"
#include "sle_bas_server.h"
#include "sle_dis_server.h"
#include "sle_hid_server.h"
#include "sle_phone_server_dis.h"
#include "sle_phone_server_slem.h"
#include "sle_phone_server.h"

#define SLE_TASK_DELAY_8_MS    8
#define SLE_ADV_HANDLE_DEFAULT       1
#define SLE_CONN_INTERVAL_PARAM      1600 // 60 * 0.125 = 7.5ms

#define APP_UUID_LEN               2
#define BT_INDEX_4                 4
#define BT_INDEX_5                 5
#define BT_INDEX_0                 0

#define SLEM_IQ_DATALEN 512

static uint8_t g_app_uuid[APP_UUID_LEN] = {0x0, 0x0};
static uint8_t g_server_id = 0;

static uint8_t g_sle_conn_hdl = 0;
static uint32_t g_sle_pair_status = 0;
static uint8_t g_sle_server_conn_state = SLE_ACB_STATE_NONE;
static bool g_sle_ssap_cccd_ready = false;

/************** SLE 连接管理 *****************/
errcode_t get_g_sle_pair_state(uint32_t *pair_state)
{
    *pair_state = g_sle_pair_status;
    return ERRCODE_SLE_SUCCESS;
}

errcode_t get_g_sle_server_conn_state(uint8_t *conn_state)
{
    *conn_state = g_sle_server_conn_state;
    return ERRCODE_SLE_SUCCESS;
}

static void sle_connect_param_update_cbk(uint16_t conn_id, errcode_t status,
    const sle_connection_param_update_evt_t *param)
{
    osal_printk("%s connect up back conn_id:0x%02x, interval:0x%x, latency:0x%x, supervision:0x%x\r\n",
        SLE_MEASURE_DIS_LOG, conn_id, param->interval, param->latency, param->supervision);
    osal_printk("[uuid server] sle_connect_param_update_cbk:0x%x\r\n", status);
}

static void sle_connect_state_changed_cbk(uint16_t conn_id, const sle_addr_t *addr,
                                          sle_acb_state_t conn_state, sle_pair_state_t pair_state,
                                          sle_disc_reason_t disc_reason)
{
    unused(addr);
    unused(pair_state);
    unused(disc_reason);
    sle_connection_param_update_t con_param = {0};
    con_param.conn_id = conn_id;
    con_param.interval_max = SLE_CONN_INTERVAL_PARAM; // 每个slot 0.125ms  //当前为100ms 值会影响测距被cancel
    con_param.interval_min = SLE_CONN_INTERVAL_PARAM; // 每个slot 0.125ms
    con_param.max_latency = 0;
    con_param.supervision_timeout = 500; // 设置连接延迟500ms
    if (conn_state == SLE_ACB_STATE_CONNECTED) {
        ssap_exchange_info_t info = { 0 };
        info.mtu_size = SLEM_IQ_DATALEN;
        info.version = 1;
        sle_server_stop_adv();
        ssapc_exchange_info_req(0, conn_id, &info);
        sle_update_connect_param(&con_param);
        sle_server_slem_start(conn_id);
    } else if (conn_state == SLE_ACB_STATE_DISCONNECTED) {
        sle_hid_set_ssap_cccd_state(false);
        sle_low_latency_set(conn_id, false, SLE_LOW_LATENCY_4K);
        sle_start_announce(SLE_ADV_HANDLE_DEFAULT);
        sle_server_slem_set_disable(conn_id);
    }
    g_sle_server_conn_state = conn_state;
    g_sle_conn_hdl = conn_id;
}

static void sle_pair_complete_cbk(uint16_t conn_id, const sle_addr_t *addr, errcode_t status)
{
    osal_printk("[uuid server] pair complete conn_id:0x%02x, status:0x%x\r\n",
        conn_id, status);
    osal_printk("[uuid server] pair complete addr:%02x:**:**:**:%02x:%02x\r\n",
        addr->addr[BT_INDEX_0], addr->addr[BT_INDEX_4], addr->addr[BT_INDEX_5]);
    g_sle_pair_status = status;
}

static void sle_conn_register_cbks(void)
{
    sle_connection_callbacks_t conn_cbks = { 0 };
    conn_cbks.connect_state_changed_cb = sle_connect_state_changed_cbk;
    conn_cbks.pair_complete_cb = sle_pair_complete_cbk;
    conn_cbks.connect_param_update_cb = sle_connect_param_update_cbk;
    sle_connection_register_callbacks(&conn_cbks);
}

/************** SLE ssap 服务注册 *****************/
static uint8_t sle_get_server_id(void)
{
    return g_server_id;
}

int server_set_mtu(uint16_t mtu_size, uint16_t version)
{
    ssap_exchange_info_t info = {
        .mtu_size = mtu_size,
        .version = version
    };

    return ssaps_set_info(g_server_id, &info);
}

errcode_t sle_init_ssap_server(void)
{
    /* 注册ssap server，应用id使用全局配置 */
    sle_uuid_t app_uuid = {
        .len = sizeof(g_app_uuid),
    };

    errno_t sec_ret = memcpy_s(app_uuid.uuid, sizeof(app_uuid.uuid), g_app_uuid, sizeof(g_app_uuid));
    if (sec_ret != EOK) {
        osal_printk("[Error]:mouse app,sle reg ssap server fail,memcpy ret:%d\n", sec_ret);
        return ERRCODE_SLE_MEMCPY_FAIL;
    }

    errcode_t ret = ssaps_register_server(&app_uuid, &g_server_id);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("[Error]:mouse app,sle reg ssap server fail,reg ret:%x\n", ret);
        return ERRCODE_SLE_FAIL;
    }

    return ERRCODE_SLE_SUCCESS;
}

bool sle_hid_get_ssap_cccd_state(void)
{
    return g_sle_ssap_cccd_ready;
}

void sle_hid_set_ssap_cccd_state(bool state)
{
    g_sle_ssap_cccd_ready = state;
}

/************** SLE 服务注册和数据接口 *****************/
/* input report数据发送 */
errcode_t sle_send_notify_indicate(uint8_t *report, uint16_t len)
{
    if (report == NULL) {
        return ERRCODE_SLE_FAIL;
    }

    return sle_hid_server_send_input_report(sle_get_server_id(), g_sle_conn_hdl, report, len);
}

void sle_services_init(void)
{
    server_set_mtu(SLEM_IQ_DATALEN, 1);
    /* 初始化ssap server */
    errcode_t ret = sle_init_ssap_server();
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("[Error]:sle mouse init ssap server service failed:%d\n", ret);
        return;
    }

    /* 添加HID Server服务 */
    ret = sle_hid_server_init(sle_get_server_id());
    if (ret != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(sle_get_server_id());
        osal_printk("[Error]:sle mouse init hid server service failed:%d\n", ret);
        return;
    }

    /* 添加DIS Server服务 */
    ret = sle_dis_server_init(sle_get_server_id());
    if (ret != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(sle_get_server_id());
        osal_printk("[Error]:sle mouse init dis server service failed:%d\n", ret);
        return;
    }

    /* 添加BAS Server服务 */
    ret = sle_bas_server_init(sle_get_server_id());
    if (ret != ERRCODE_SLE_SUCCESS) {
        ssaps_unregister_server(sle_get_server_id());
        osal_printk("[Error]:sle mouse init bas server service failed:%d\n", ret);
    }
}

static void sle_server_init(void)
{
    sle_low_latency_tx_enable();
    sle_conn_register_cbks();
    sle_services_init();
    sle_server_adv_init();
    osal_printk("%s init ok\r\n", SLE_MEASURE_DIS_LOG);
}

/************** 样例初始化入口 *****************/
static void sle_power_on_cbk(uint8_t status)
{
    osal_printk("power on status:%d\r\n", status);
}

static void sle_enable_cbk(uint8_t status)
{
    osal_printk("enable status:%d\r\n", status);
    sle_server_init();
}

static void ssaps_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id,  ssap_exchange_info_t *mtu_size,
                                  errcode_t status)
{
    osal_printk("%s ssaps ssaps_mtu_changed_cbk callback server_id:0x%x, conn_id:0x%x, mtu_size:0x%x, status:0x%x\r\n",
                SLE_MEASURE_DIS_LOG, server_id, conn_id, mtu_size->mtu_size, status);
}

static void ssaps_start_service_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    osal_printk("%s start service cbk callback server_id:%d, handle:0x%x, status:0x%x\r\n", SLE_MEASURE_DIS_LOG,
                server_id, handle, status);
}

static void ssaps_add_service_cbk(uint8_t server_id, sle_uuid_t *uuid, uint16_t handle, errcode_t status)
{
    unused(uuid);
    osal_printk("%s add service cbk callback server_id:0x%x, handle:0x%x, status:0x%x\r\n", SLE_MEASURE_DIS_LOG,
                server_id, handle, status);
}

static void ssaps_add_property_cbk(uint8_t server_id, sle_uuid_t *uuid, uint16_t service_handle,
    uint16_t handle, errcode_t status)
{
    unused(uuid);
    osal_printk("%s add property cbk callback server_id:0x%x, service_handle:0x%x,handle:0x%x, status:0x%x\r\n",
                SLE_MEASURE_DIS_LOG, server_id, service_handle, handle, status);
}

static void ssaps_add_descriptor_cbk(uint8_t server_id, sle_uuid_t *uuid, uint16_t service_handle,
                                     uint16_t property_handle, errcode_t status)
{
    unused(uuid);
    osal_printk("%s add descriptor cbk callback server_id:0x%x, service_handle:0x%x, property_handle:0x%x, \
                status:0x%x\r\n", SLE_MEASURE_DIS_LOG, server_id, service_handle, property_handle, status);
}

static void ssaps_delete_all_service_cbk(uint8_t server_id, errcode_t status)
{
    osal_printk("%s delete all service callback server_id:0x%x, status:0x%x\r\n", SLE_MEASURE_DIS_LOG,
                server_id, status);
}

static void ssaps_server_read_request_cbk(uint8_t server_id, uint16_t conn_id, ssaps_req_read_cb_t *read_cb_para,
                                          errcode_t status)
{
    osal_printk("%s ssaps read request cbk callback server_id:0x%x, conn_id:0x%x, handle:0x%x, status:0x%x\r\n",
        SLE_MEASURE_DIS_LOG,
        server_id,
        conn_id,
        read_cb_para->handle,
        status);
}

static void ssaps_server_write_request_cbk(uint8_t server_id, uint16_t conn_id, ssaps_req_write_cb_t *write_cb_para,
                                           errcode_t status)
{
    unused(status);
    unused(conn_id);
    unused(server_id);
    if (write_cb_para == NULL || write_cb_para->value == NULL) {
        return;
    }
    osal_printk("sle vdt write recv data:%s\r\n", write_cb_para->value);
}

static void ssapc_mtu_changed_cbk(uint8_t client_id, uint16_t conn_id, ssap_exchange_info_t *param, errcode_t status)
{
    osal_printk("%s ssaps ssapc_mtu_changed_cbk callback client_id:0x%x, conn_id:0x%x, mtu_size:0x%x, status:0x%x\r\n",
                SLE_MEASURE_DIS_LOG, client_id, conn_id, param->mtu_size, status);
}

static void ssapc_read_cfm_cbk(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *read_data,
    errcode_t status)
{
    osal_printk("%s ssaps ssapc_read_cfm_callback client_id:0x%x, conn_id:0x%x, read_data->handle:0x%x, status:0x%x\r\n",
                SLE_MEASURE_DIS_LOG, client_id, conn_id, read_data->handle, status);
}

static void ssapc_write_cfm_cbk(uint8_t client_id, uint16_t conn_id, ssapc_write_result_t *write_result,
    errcode_t status)
{
    osal_printk("%s ssaps ssapc_write_cfm_callback client_id:0x%x, conn_id:0x%x, write_result->handle:0x%x, status:0x%x\r\n",
                SLE_MEASURE_DIS_LOG, client_id, conn_id, write_result->handle, status);
}

static void ssapc_notification_cbk(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data,
    errcode_t status)
{
    osal_printk("%s ssaps ssapc_notification_callback callback client_id:0x%x, conn_id:0x%x, data->data_len:0x%x, status:0x%x\r\n",
                SLE_MEASURE_DIS_LOG, client_id, conn_id, data->data_len, status);
}

static errcode_t sle_ssaps_register_cbks(void)
{
    errcode_t ret;
    sle_dev_manager_callbacks_t dev_mgr_cbks = { 0 };
    dev_mgr_cbks.sle_power_on_cb = sle_power_on_cbk;
    dev_mgr_cbks.sle_enable_cb = sle_enable_cbk;
    if (sle_dev_manager_register_callbacks(&dev_mgr_cbks) != ERRCODE_BT_SUCCESS) {
        osal_printk("register sle_enable failed\r\n");
    }

    ssaps_callbacks_t ssaps_cbk = { 0 };
    ssaps_cbk.add_service_cb = ssaps_add_service_cbk;
    ssaps_cbk.add_property_cb = ssaps_add_property_cbk;
    ssaps_cbk.add_descriptor_cb = ssaps_add_descriptor_cbk;
    ssaps_cbk.start_service_cb = ssaps_start_service_cbk;
    ssaps_cbk.delete_all_service_cb = ssaps_delete_all_service_cbk;
    ssaps_cbk.mtu_changed_cb = ssaps_mtu_changed_cbk;
    ssaps_cbk.read_request_cb = ssaps_server_read_request_cbk;
    ssaps_cbk.write_request_cb = ssaps_server_write_request_cbk;
    ret = ssaps_register_callbacks(&ssaps_cbk);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("%s sle_ssaps_register_cbks,ssaps_register_callbacks fail :%x\r\n", SLE_MEASURE_DIS_LOG,
                    ret);
        return ret;
    }
    
    ssapc_callbacks_t ssapc_cbk = { 0 };
    ssapc_cbk.exchange_info_cb = ssapc_mtu_changed_cbk;
    ssapc_cbk.read_cfm_cb = ssapc_read_cfm_cbk;
    ssapc_cbk.write_cfm_cb = ssapc_write_cfm_cbk;
    ssapc_cbk.notification_cb = ssapc_notification_cbk;
    ssapc_register_callbacks(&ssapc_cbk);

    sle_server_slem_init();

    return ERRCODE_SLE_SUCCESS;
}

void sle_dev_cb_register(void)
{
    osal_printk("sle_dev_cb_register CORE_NUMS:%d\r\n", CORE_NUMS);
    sle_ssaps_register_cbks();
    enable_sle();
}