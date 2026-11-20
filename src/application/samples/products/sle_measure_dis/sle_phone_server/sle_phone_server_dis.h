/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: SLE ADV Config. \n
 *
 * History: \n
 * 2023-07-17, Create file. \n
 */

#ifndef SLE_MEASURE_DIS_H
#define SLE_MEASURE_DIS_H

#include "cmsis_os2.h"
#include "errcode.h"
#include "sle_hadm_manager.h"

#define KEY_CONNECT_DELAY_TIME 500
#define MEASURE_DIS_INVALID (-1)
#define MEASURE_DIS_RSSI_INVALID (-128)
#define MEASURE_DIS_HIGH_INVALID (-10)
#define MEASURE_DIS_IQ_REPORT_CNT_MAX 1
#define POSALG_DATA_NUM 79
#define IQ_DATA_MAX (MEASURE_DIS_IQ_REPORT_CNT_MAX * SLE_CS_IQ_REPORT_COUNT)
#define POSALG_RSSI_OFFSET 256
#define POSALG_CALIB_VAL 0.8
#define POSALG_IQ_FLIP_BARRIER 1023
#define POSALG_IQ_FLIP_OFFSET 2048
#define MEASURE_DIS_TIMESTAMP_DIFF_MAX 20

#define MEASURE_DIS_NUM_CARRY_10_7     10000000
#define MEASURE_DIS_NUM_CARRY_1000     1000
#define MEASURE_DIS_NUM_CARRY_100      100
#define MEASURE_DIS_NUM_CARRY_10       10
#define MEASURE_DIS_NUM_CARRY_2        2

#define MEASURE_DIS_MSG_EVENT 1
#define MEASURE_DIS_MSG_QUEUE_SIZE 30

#define SLEM_CONNET_INVAILD 0xFF
typedef enum {
    SLEM_MSG_LOCAL_IQ,
    SLEM_MSG_REMOTE_IQ,
} slem_msg_node_type_t;

typedef struct {
    int key_num;
    int key_num_all;
} sle_server_dis_keys_info;

typedef struct {
    uint32_t                  sn;                               /*!< 测量SN */
    float                     dist_first;                       /*!< 第一个测距值 */
    float                     dist_second;                      /*!< 第二个测距值 */
    float                     dist_double;                      /*!< 第二个测距值 */
    float                     rssi;                             /*!< 信号强度 */
    float                     high;                             /*!< 置信度相关系数 */
    float                     prob;                             /*!< 置信度 */
    unsigned char             smooth_num;                       /*!<有效平滑次数 */
} sle_server_dis_profile_msg_dis_t;

typedef struct {
    uint16_t i_data;
    uint16_t q_data;
} sle_server_dis_stored_qte_trans_t;

typedef struct {
    uint8_t samp_cnt;
    uint8_t rssi;
    uint16_t es_sn;
    uint32_t timestamp_sn;
    sle_server_dis_stored_qte_trans_t data[IQ_DATA_MAX];
    uint32_t tof_result;
} sle_server_dis_stored_iq_data_t;

typedef struct {
    uint16_t i_data;
    uint16_t q_data;
} sle_channel_sounding_qte_trans_t;

typedef struct {
    uint8_t samp_cnt;
    uint8_t rssi;
    uint16_t es_sn;
    uint32_t timestamp_sn;
    sle_channel_sounding_qte_trans_t data[IQ_DATA_MAX];
    uint32_t tof_result;
} sle_channel_sounding_iq_trans_t;

typedef struct {
    uint16_t            conn_id;                          /*!< 连接句柄 */
    uint16_t            type;                             /*!< 消息类型 */
    uint16_t            result;                           /*!< 执行结果 */
    void                *data;
} sle_server_dis_msg_node_t;

#define check_rc_return_rc(rc, err)                                                             \
    do {                                                                                         \
        if ((rc) != ERRCODE_SUCC) {                                                              \
            osal_printk("CARKEY ERROR: %s fail!: call %s return 0x%x!\n", err, __FUNCTION__, rc);   \
        }                                                                                        \
    } while (0)

#define g_log_level 63
/**
 * @brief 通过串口打印日志。
 * @par 通过串口打印日志。
 * @param[in] level 日志级别。
 * @param[in] fmt 格式化字符串。
 * @param[in] args 输出内容。
 */
#define carkey_log_uart(level, format, args...)             \
do {                                                        \
    if (level & g_log_level) {                              \
        osal_printk(format, ##args);                        \
    }                                                       \
} while (0)
/**
 * @brief iq上报结构体。
 */


extern osal_event sle_phone_dis_evt;

osal_event* get_sle_server_dis_evt(void);
void sle_server_dis_init(void);
void sle_server_dis_msg_proc(void);
errcode_t sle_server_dis_reg_callbacks(void);
errcode_t sle_server_dis_match_msg(sle_server_dis_msg_node_t *msg_node);
errcode_t sle_server_dis_remote_iq(uint16_t len, uint8_t *value);
errcode_t sle_server_dis_msg_store(uint16_t conn_id, slem_msg_node_type_t msg_type,
    sle_channel_sounding_iq_report_t *report);
void sle_server_dis_enable(uint16_t conn_id);

#endif