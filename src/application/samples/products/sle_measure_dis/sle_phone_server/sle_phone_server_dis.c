/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: sle adv config for sle uart server. \n
 *
 * History: \n
 * 2023-07-17, Create file. \n
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "tcxo.h"
#include "securec.h"
#include "soc_osal.h"
#include "osal_debug.h"
#include "common_def.h"
#include "sle_phone_server.h"
#include "sle_common.h"
#include "slem_smooth.h"
#include "slem_alg_smooth_dis.h"
#include "sle_phone_server_slem.h"
#include "sle_phone_server_dis.h"

#define DIS_TOF_CALI_DEFAULT 1010
#define DIS_ALG_RSSI_LIMIT (-120)
#define DIS_ALG_THRESHOULD_COND2 20
#define DIS_ALG_R_START 2
#define DIS_TO_METERS 100

uint8_t g_next_idx;
uint32_t counter = 0;
uint8_t g_iq_debug_flag = 0;

uint8_t g_md_store_local_iq_complete;
unsigned long g_sle_server_dis_queue;

osal_event sle_phone_dis_evt;
sle_server_dis_msg_node_t g_msg_data;
sle_server_dis_stored_iq_data_t g_md_local_iq_data;
sle_server_dis_stored_iq_data_t g_md_remote_iq_data;
uint8_t g_sle_server_dis_conn_id = SLEM_CONNET_INVAILD;


void sle_server_dis_posalg_set_base_para(slem_alg_para_dis *alg_para, uint8_t key_id)
{
    unused(key_id);
    sle_phone_slem_caps_t *local_caps = get_slem_local_caps();
    sle_phone_slem_caps_t *remote_caps = get_slem_remote_caps();
    alg_para->calib_val = local_caps->phase_cali_offset_cm + ((float)remote_caps->phase_cali_offset_cm) / DIS_TO_METERS;
    alg_para->tof_calib = remote_caps->tof_cali_offset_m + DIS_TOF_CALI_DEFAULT;
    alg_para->ranging_method = METHOD_ADJ_R_END;
    if (g_iq_debug_flag) {
        osal_printk("[phone sample] calib_val: %d.%03d, tof_calib:%d \r\n", (int32_t)(alg_para->calib_val),
            ((uint32_t)(alg_para->calib_val * MEASURE_DIS_NUM_CARRY_1000) % MEASURE_DIS_NUM_CARRY_1000),
            (uint32_t)alg_para->tof_calib);
    }
}

void sle_server_dis_print_cal_dis(float dist_first, float dist_ori, float prob, uint32_t time)
{
    unused(dist_ori);
    unused(prob);
    osal_printk("[phone sample] SLEM get distance done. distance: %d.%03d, dist_ori:%d.%03d prob:%d ",
        (int32_t)(dist_first), ((uint32_t)(dist_first * MEASURE_DIS_NUM_CARRY_1000) % MEASURE_DIS_NUM_CARRY_1000),
        (int32_t)dist_ori, ((uint32_t)(dist_ori * MEASURE_DIS_NUM_CARRY_1000) % MEASURE_DIS_NUM_CARRY_1000), prob);
    osal_printk(" time:%d \r\n", time);
    osal_printk("------------------------------------------\r\n");
}

static void sle_server_dis_posalg_set_distance(slem_smoothed_dis_result *dis,
    sle_server_dis_profile_msg_dis_t *sle_server_dis_temp)
{
    sle_server_dis_temp->dist_first = dis->dis_smoothed;
    sle_server_dis_temp->dist_second = dis->dis_ori;
    sle_server_dis_temp->dist_double = dis->dis_slight_smoothed;
    sle_server_dis_temp->high = dis->height;
    sle_server_dis_temp->prob = dis->prob;
    sle_server_dis_temp->rssi = dis->rssi;
    sle_server_dis_temp->smooth_num = dis->smooth_num;
}

void sle_server_dis_posalg_get_distance(sle_server_dis_profile_msg_dis_t *sle_server_dis_temp,
                                        sle_channel_sounding_iq_trans_t *remote_iq_data)
{
    unused(remote_iq_data);
    slem_alg_para_dis *alg_para = (slem_alg_para_dis *)osal_kmalloc(sizeof(slem_alg_para_dis), 0);

    sle_server_dis_posalg_set_base_para(alg_para, 0);
    alg_para->rssi_rtd = g_md_local_iq_data.rssi;
    alg_para->rssi_dut = g_md_remote_iq_data.rssi;
    alg_para->tof_rtd = g_md_local_iq_data.tof_result;
    alg_para->tof_dut = g_md_remote_iq_data.tof_result;
    alg_para->iq_rtd = (slem_alg_iq *)&(g_md_local_iq_data.data[0]);
    alg_para->iq_dut = (slem_alg_iq *)&(g_md_remote_iq_data.data[0]);
    g_md_store_local_iq_complete = false;

    slem_para_pair para_limit = {.rssi_limit = DIS_ALG_RSSI_LIMIT,
                                 .threshold_cond2 = DIS_ALG_THRESHOULD_COND2,
                                 .r_start = DIS_ALG_R_START};
    alg_para->para_limit = para_limit;
    alg_para->key_id = 0;
    /* remote iq 是 dut， local iq 是 rtd */
    uint32_t time_start = (uint32_t)uapi_tcxo_get_ms();

    alg_para->cur_count = counter;
    alg_para->cur_time = time_start;
    slem_smoothed_dis_result result_dist;
    errcode_slem ret = slem_alg_calc_smoothed_dis(&result_dist, alg_para);
    if (ret == ERRCODE_SLEM_SUCCESS) {
        counter++;
        uint32_t time_end = (uint32_t)uapi_tcxo_get_ms();
        sle_server_dis_print_cal_dis(result_dist.dis_smoothed, result_dist.dis_ori,
            result_dist.prob, time_end - time_start);
        sle_server_dis_posalg_set_distance(&result_dist, sle_server_dis_temp);
    }

    osal_kfree(alg_para);
}

void sle_server_dis_print_iq_data(uint8_t num, uint8_t role, sle_channel_sounding_iq_trans_t *report)
{
    if (!g_iq_debug_flag) {
        return;
    }
    osal_printk("[iq_data]%d,%d,%d,%d,0x%x,",
                num, role, report->timestamp_sn, report->samp_cnt, report->rssi);
    sle_channel_sounding_qte_trans_t *data = report->data;
    uint16_t i = 0;
    while (i < POSALG_DATA_NUM) {
        osal_printk("[%d]:0x%x,0x%x", i, data[i].i_data, data[i].q_data);
        i++;
        if (i == POSALG_DATA_NUM) {
            osal_printk(".\r\n");
        } else {
            osal_printk(";");
        }
    }
}

errcode_t sle_server_dis_store_local_iq(sle_channel_sounding_iq_report_t *report)
{
    if (report->report_idx == 0) {
        g_md_local_iq_data.samp_cnt = 0;
        g_md_local_iq_data.rssi = report->rssi[0];
        g_md_local_iq_data.es_sn = report->es_sn;
        g_md_local_iq_data.timestamp_sn = report->timestamp_sn;
        g_md_local_iq_data.tof_result = report->tof_result;
        g_next_idx = 0;
    }
    if ((report->report_idx >= MEASURE_DIS_IQ_REPORT_CNT_MAX ||
         report->timestamp_sn != g_md_local_iq_data.timestamp_sn) ||
        (g_next_idx != report->report_idx)) {
        osal_printk("[phone sample] STORE LOCAL IQ FAIL. report_idx:%d, timestamp:%d, g_next_idx:%d. \r\n",
                    report->report_idx, report->timestamp_sn, g_next_idx);
        return ERRCODE_INVALID_PARAM;
    }
    g_md_local_iq_data.samp_cnt += report->samp_cnt;
    uint8_t offset = report->report_idx * SLE_CS_IQ_REPORT_COUNT;
    for (uint8_t i = 0; i < report->samp_cnt; i++) {
        g_md_local_iq_data.data[offset + i].i_data = report->i_data[i];
        g_md_local_iq_data.data[offset + i].q_data = report->q_data[i];
    }
    g_next_idx = report->report_idx + 1;

    return ERRCODE_SUCC;
}

errcode_t sle_server_dis_proc_local_iq(uint16_t conn_id, sle_channel_sounding_iq_report_t *report)
{
    unused(conn_id);
    errcode_t ret = ERRCODE_SUCC;

    ret = sle_server_dis_store_local_iq(report);
    if (ret != ERRCODE_SUCC) {
        return ERRCODE_INVALID_PARAM;
    }

    if (report->report_idx + 1 == MEASURE_DIS_IQ_REPORT_CNT_MAX) {
        g_md_store_local_iq_complete = true;
        /* 如果是钥匙则发送给从锚点 */
        sle_server_dis_print_iq_data(0, 0, (sle_channel_sounding_iq_trans_t *)(&g_md_local_iq_data));
    }
    return ret;
}

errcode_t sle_server_dis_store_remote_iq(sle_channel_sounding_iq_report_t *report)
{
    if (report->report_idx == 0) {
        g_md_remote_iq_data.samp_cnt = 0;
        g_md_remote_iq_data.rssi = report->rssi[0];
        g_md_remote_iq_data.es_sn = g_md_local_iq_data.es_sn;
        g_md_remote_iq_data.timestamp_sn = report->timestamp_sn;
        g_md_remote_iq_data.tof_result = report->tof_result;
        g_next_idx = 0;
    }
    if ((report->report_idx >= MEASURE_DIS_IQ_REPORT_CNT_MAX ||
         report->timestamp_sn != g_md_remote_iq_data.timestamp_sn) ||
        (g_next_idx != report->report_idx)) {
        osal_printk("[phone sample] STORE REMOTE IQ FAIL. report_idx:%d, timestamp:%d, g_next_idx:%d. \r\n",
                    report->report_idx, report->timestamp_sn, g_next_idx);
        return ERRCODE_INVALID_PARAM;
    }
    g_md_remote_iq_data.samp_cnt += report->samp_cnt;
    uint8_t offset = report->report_idx * SLE_CS_IQ_REPORT_COUNT;
    for (uint8_t i = 0; i < report->samp_cnt; i++) {
        g_md_remote_iq_data.data[offset + i].i_data = report->i_data[i];
        g_md_remote_iq_data.data[offset + i].q_data = report->q_data[i];
    }
    g_next_idx = report->report_idx + 1;

    return ERRCODE_SUCC;
}

errcode_t sle_server_dis_proc_remote_iq(uint16_t conn_id, sle_channel_sounding_iq_report_t *report)
{
    unused(conn_id);
    unused(report);
    // uint8_t num = 0; /* SLAVE只有一路MEASURE_DIS业务 */
    sle_server_dis_store_remote_iq(report);
    sle_server_dis_print_iq_data(0, 1, (sle_channel_sounding_iq_trans_t *)(&g_md_remote_iq_data));
    // report->rssi
    if (abs((int32_t)(g_md_local_iq_data.timestamp_sn - g_md_remote_iq_data.timestamp_sn)) >=
            MEASURE_DIS_TIMESTAMP_DIFF_MAX) {
        osal_printk("local:%d, remote:%d. \r\n", g_md_local_iq_data.timestamp_sn, g_md_remote_iq_data.timestamp_sn);
        return ERRCODE_INVALID_PARAM;
    }

    if (g_md_store_local_iq_complete) {
        g_md_store_local_iq_complete = false;
        sle_server_dis_profile_msg_dis_t sle_server_dis_dis_temp = {
            g_md_remote_iq_data.timestamp_sn, 0, 0, 0, 0, 0, 0, 0};
        sle_server_dis_posalg_get_distance(&sle_server_dis_dis_temp,
            (sle_channel_sounding_iq_trans_t *)(&g_md_remote_iq_data));
    }

    osal_printk("[phone sample] rssi = %d, local ts = %d, remote ts = %d.  differential_value[%d]\r\n", g_md_remote_iq_data.rssi,
                g_md_local_iq_data.timestamp_sn, g_md_remote_iq_data.timestamp_sn,
                abs((int32_t)(g_md_local_iq_data.timestamp_sn - g_md_remote_iq_data.timestamp_sn)));

    return ERRCODE_SUCC;
}

errcode_t sle_server_dis_msg_add(sle_server_dis_msg_node_t *msg)
{
    errcode_t ret = ERRCODE_FAIL;

    do {
        if ((msg == NULL) || (g_sle_server_dis_queue == 0)) {
            osal_printk("sle_server_dis_msg_add 1. \r\n");
            break;
        }
        if (osal_msg_queue_write_copy(g_sle_server_dis_queue,
                                      (void *)msg, sizeof(sle_server_dis_msg_node_t), 0) != OSAL_SUCCESS) {
            osal_printk("sle_server_dis_msg_add 2. \r\n");
            ret = ERRCODE_FAIL;
            break;
        }
        if (osal_event_write(&sle_phone_dis_evt, MEASURE_DIS_MSG_EVENT) != OSAL_SUCCESS) {
            osal_printk("sle_server_dis_msg_add 3. \r\n");
            ret = ERRCODE_FAIL;
            break;
        }
        ret = ERRCODE_SUCC;
    } while (0);

    if (ret != ERRCODE_SUCC) {
        uint8_t msg_num = osal_msg_queue_get_msg_num(g_sle_server_dis_queue);
        osal_printk("MSG ADD FAIL. msg_num:%d ret:%d. \r\n", msg_num, ret);
    }

    return ret;
}

errcode_t sle_server_dis_msg_store(uint16_t conn_id, slem_msg_node_type_t msg_type,
                                   sle_channel_sounding_iq_report_t *report)
{
    errcode_t ret = ERRCODE_FAIL;
    sle_channel_sounding_iq_report_t *report_local = NULL;
    uint32_t len = sizeof(sle_channel_sounding_iq_report_t);
    report_local = (sle_channel_sounding_iq_report_t *)osal_kmalloc(len, 0);
    if (report_local == NULL) {
        return ret;
    }
    if (memcpy_s(report_local, sizeof(sle_channel_sounding_iq_report_t), report, len) != EOK) {
        osal_kfree(report_local);
        return ret;
    }
    sle_server_dis_msg_node_t msg_node = {conn_id, msg_type, 0, report_local};
    ret = sle_server_dis_msg_add(&msg_node);
    if (ret != ERRCODE_SUCC) {
        osal_kfree(report_local);
    }
    
    return ret;
}

void sle_server_dis_msg_proc(void)
{
    sle_server_dis_msg_node_t *msg_node = NULL;
    uint32_t msg_data_size = sizeof(sle_server_dis_msg_node_t);
    errcode_t ret = ERRCODE_SUCC;
    ret = osal_msg_queue_read_copy(g_sle_server_dis_queue, &g_msg_data, &msg_data_size, 0);
    if (ret != ERRCODE_SUCC) {
        return;
    }
    msg_node = &g_msg_data;
    ret = sle_server_dis_match_msg(msg_node);
    if (ret != ERRCODE_SUCC) {
        osal_printk("MSG PROC ERROR type:%d \r\n", msg_node->type);
    }
}

errcode_t sle_server_dis_recv_local_iq(sle_server_dis_msg_node_t *msg_node)
{
    errcode_t ret = sle_server_dis_proc_local_iq(msg_node->conn_id,
        (sle_channel_sounding_iq_report_t *)(msg_node->data));
    osal_kfree(msg_node->data);
    osal_event_write(&sle_phone_dis_evt, MEASURE_DIS_MSG_EVENT);
    return ret;
}

errcode_t sle_server_dis_recv_remote_iq(sle_server_dis_msg_node_t *msg_node)
{
    errcode_t ret = sle_server_dis_proc_remote_iq(msg_node->conn_id,
        (sle_channel_sounding_iq_report_t *)(msg_node->data));
    osal_kfree(msg_node->data);
    osal_event_write(&sle_phone_dis_evt, MEASURE_DIS_MSG_EVENT);
    return ret;
}

errcode_t sle_server_dis_match_msg(sle_server_dis_msg_node_t *msg_node)
{
    errcode_t ret = ERRCODE_FAIL;
    switch (msg_node->type) {
        case SLEM_MSG_LOCAL_IQ:
            ret = sle_server_dis_recv_local_iq(msg_node);
            break;
        case SLEM_MSG_REMOTE_IQ:
            ret = sle_server_dis_recv_remote_iq(msg_node);
            break;
        default:
            break;
    }
    return ret;
}

errcode_t sle_server_dis_remote_iq(uint16_t len, uint8_t *value)
{
    unused(len);
    errcode_t ret = ERRCODE_FAIL;
    sle_channel_sounding_iq_trans_t *report = NULL;
    report = (sle_channel_sounding_iq_trans_t *)osal_kmalloc(sizeof(sle_channel_sounding_iq_trans_t), 0);
    if (report == NULL) {
        return ERRCODE_MALLOC;
    }
    if (memcpy_s(report, sizeof(sle_channel_sounding_iq_trans_t), value, len) != EOK) {
        osal_kfree(report);
        return ERRCODE_MEMCPY;
    }
    osal_printk("RECEIVE REMOTE IQ. timestamp_sn:%d \r\n", report->timestamp_sn);
    sle_server_dis_msg_node_t msg_node = {g_sle_server_dis_conn_id, SLEM_MSG_REMOTE_IQ, 0, report};
    ret = sle_server_dis_msg_add(&msg_node);
    if (ret != ERRCODE_SUCC) {
        osal_kfree(report);
    }

    return ret;
}

void sle_server_dis_init(void)
{
    if (osal_event_init(&sle_phone_dis_evt) != ERRCODE_SUCC) {
        osal_printk("touch osal_event_init fail! \r\n");
        return;
    }

    if (osal_msg_queue_create("sle_server_dis_queue", MEASURE_DIS_MSG_QUEUE_SIZE,
                              &g_sle_server_dis_queue, 0, sizeof(sle_server_dis_msg_node_t)) != ERRCODE_SUCC) {
        osal_printk("osal_queue_init init failed \r\n");
        return;
    }
}

