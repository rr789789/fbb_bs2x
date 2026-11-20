/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: sle adv config for sle uart server. \n
 *
 * History: \n
 * 2023-07-17, Create file. \n
 */
#include "tcxo.h"
#include "securec.h"
#include "soc_osal.h"
#include "osal_debug.h"
#include "common_def.h"
#include "sle_common.h"
#include "slem_smooth.h"
#include "slem_alg_smooth_dis.h"
#include "sle_hadm_manager.h"
#include "sle_phone_server.h"
#include "sle_phone_server_dis.h"
#include "sle_phone_server_slem.h"

#define IQ_MONITORING 77
#define ERROR_IQ_VALUE 0x0

sle_phone_slem_caps_t g_slem_caps[SLEM_MAXNUM_CAPS];

sle_phone_slem_caps_t *get_slem_local_caps(void)
{
    return &g_slem_caps[SLEM_LOCAL_CAPS];
}

sle_phone_slem_caps_t *get_slem_remote_caps(void)
{
    return &g_slem_caps[SLEM_REMOTE_CAPS];
}

void sle_server_slem_standard_val_mod_clear(void)
{
    memset_s(&g_slem_caps[SLEM_LOCAL_CAPS], sizeof(sle_phone_slem_caps_t), 0, sizeof(sle_phone_slem_caps_t));
    memset_s(&g_slem_caps[SLEM_REMOTE_CAPS], sizeof(sle_phone_slem_caps_t), 0, sizeof(sle_phone_slem_caps_t));
}

errcode_t sle_server_slem_set_disable(uint16_t conn_id)
{
    osal_printk("[phone sample] SLEM SET DISABLE conn_id:%x.\r\n", conn_id);
    errcode_t ret = sle_set_channel_sounding_disable(conn_id);
    /* 解决测距使能之后，马上断链导致没有底层回调 */
    if ((ret != ERRCODE_SUCC) || (conn_id == SLEM_CONNET_INVAILD)) {
        osal_printk("slem set disable failed, ret = %d.\n", ret);
    }
    return ERRCODE_SUCC;
}

errcode_t sle_server_slem_set_enable(uint16_t conn_id)
{
    osal_printk("[phone sample] SLEM SET ENABLE conn_id:%x.\r\n", conn_id);
    errcode_t ret = sle_set_channel_sounding_enable(conn_id);
    /* 解决测距使能之后，马上断链导致没有底层回调 */
    if ((ret != ERRCODE_SUCC) || (conn_id == SLEM_CONNET_INVAILD)) {
        osal_printk("slem set enable failed, ret = %d.\n", ret);
    }
    return ERRCODE_SUCC;
}

errcode_t sle_server_slem_config_param(uint16_t conn_id)
{
    sle_set_channel_sounding_param_ex_t param = {
        .con_anchor_num = 1,
        .freq_space = 0, // 频率间隔， 0：1M， 1：2M [车钥匙应用]
        .is_cs_param_chg = 0, // 是否需要更改测距连接参数，0：不需要更改，1：需要更改（首次设置参数需置0）
        .refresh_rate = 1, // 刷新率，1：高频刷新，2：中频刷新；4：低频刷新
        .posalg_freq = 1,
        .glp_mode = 0,
        .acb_interval = 800,
        .cs_interval = 4000,
        .report_type = 1,
    };
    errcode_t ret = sle_set_channel_sounding_param_ex(conn_id, &param);
    if ((ret != ERRCODE_SUCC) || (conn_id == SLEM_CONNET_INVAILD)) {
        osal_printk("[phone sample] slem set param failed, ret = %d.\n", ret);
    }
    osal_printk("[phone sample] slem set param finish. refresh_rate:%d \n", param.refresh_rate);
    return ERRCODE_SUCC;
}

errcode_t sle_server_slem_start(uint16_t conn_id)
{
    errcode_t ret = sle_server_slem_config_param(conn_id);
    if (ret != 0) {
        osal_printk("[phone sample] sle_server_slem_config_param error\r\n");
    }

    ret = sle_read_local_channel_sounding_caps();
    if (ret != 0) {
        osal_printk("[phone sample] sle_read_local_channel_sounding_caps error\r\n");
    }

    ret = sle_read_remote_channel_sounding_caps(conn_id);
    if (ret != 0) {
        osal_printk("[phone sample] sle_read_remote_channel_sounding_caps error\r\n");
    }

    return ret;
}

void sle_channel_sounding_local_iq_report_cbk(uint16_t conn_id, sle_channel_sounding_iq_report_t *report)
{
    unused(conn_id);
    unused(report);
    if (report->i_data[IQ_MONITORING] == ERROR_IQ_VALUE && report->q_data[IQ_MONITORING] == ERROR_IQ_VALUE) {
        return ;
    }

    if (report->tx_tt_id == SLEM_G_POINT && report->rx_tt_id == SLEM_T1_POINT) {
        sle_server_dis_msg_store(conn_id, SLEM_MSG_LOCAL_IQ, report);
    } else if (report->tx_tt_id == SLEM_T1_POINT && report->rx_tt_id == SLEM_G_POINT) {
        sle_server_dis_msg_store(conn_id, SLEM_MSG_REMOTE_IQ, report);
    } else {
        osal_printk("[phone sample] error target value!!! tx_id :%d, rx_id:%d\r\n",
            report->tx_tt_id, report->rx_tt_id);
    }
}

void sle_channel_sounding_set_config_cbk(errcode_t status)
{
    unused(status);
    osal_printk("[phone sample] sle_channel_sounding_set_config_cbk status:%d\r\n", status);
}

void sle_channel_sounding_state_changed_cbk(uint8_t slem_status, errcode_t status)
{
    unused(slem_status);
    unused(status);
    osal_printk("[phone sample] sle_channel_sounding_state_changed_cbk slem_status:%d\r\n", slem_status);
}

void sle_read_local_channel_sounding_cbk(sle_channel_sounding_caps_t *caps,
    errcode_t status)
{
    unused(caps);
    unused(status);
    sle_phone_slem_caps_t *local_caps = (sle_phone_slem_caps_t*)caps;
    sle_server_slem_standard_val_mod_clear();
    
    if (memcpy_s(&g_slem_caps[SLEM_LOCAL_CAPS], sizeof(sle_phone_slem_caps_t), local_caps, sizeof(sle_phone_slem_caps_t)) != EOK) {
        osal_printk("[phone sample] cpy local error! \r\n");
    }
}

void sle_read_remote_channel_sounding_cbk(uint16_t conn_id, sle_channel_sounding_caps_t *caps,
    errcode_t status)
{
    unused(conn_id);
    unused(caps);
    unused(status);
    sle_phone_slem_caps_t *remote_caps = (sle_phone_slem_caps_t*)caps;
    if (memcpy_s(&g_slem_caps[SLEM_REMOTE_CAPS], sizeof(sle_phone_slem_caps_t), remote_caps, sizeof(sle_phone_slem_caps_t)) != EOK) {
        osal_printk("[phone sample] cpy remote error! \r\n");
    }

    errcode_t ret = sle_server_slem_set_enable(conn_id);
    if (ret != 0) {
        osal_printk("[phone sample] sle_server_slem_set_enable error\r\n");
    }
}

errcode_t sle_server_slem_init(void)
{
    sle_server_dis_init();
    sle_hadm_callbacks_t slem_hadm_cbk = { 0 };
    slem_hadm_cbk.cs_set_config_cb = sle_channel_sounding_set_config_cbk;
    slem_hadm_cbk.cs_state_changed_cb = sle_channel_sounding_state_changed_cbk;
    slem_hadm_cbk.read_local_cs_caps_cb = sle_read_local_channel_sounding_cbk;
    slem_hadm_cbk.read_remote_cs_caps_cb = sle_read_remote_channel_sounding_cbk;
    slem_hadm_cbk.cs_iq_report_cb = sle_channel_sounding_local_iq_report_cbk;
    return sle_hadm_register_callbacks(&slem_hadm_cbk);
}