/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: SLE ADV Config. \n
 *
 * History: \n
 * 2023-07-17, Create file. \n
 */

#ifndef SLE_SERVER_SLEM_H
#define SLE_SERVER_SLEM_H

#include "cmsis_os2.h"
#include "errcode.h"
#include "sle_hadm_manager.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define SLEM_CONNET_INVAILD 0xFF

typedef enum slem_caps_number {
    SLEM_LOCAL_CAPS                 = 0x00,
    SLEM_REMOTE_CAPS                = 0x01,
    SLEM_MAXNUM_CAPS                = 0x02
} slem_caps_number_t;

typedef struct {
    uint32_t measurement_signal_capability_supported;    // 测量信号能力支持；
    uint32_t measurement_reporting_capability_supported; // 测量报告能力支持;
    uint8_t mutil_antennas_supported;                    // 多天线支持；
    uint8_t mutil_antennas_switching_interval;           // 多天线切换间隔；
    uint8_t type1_min_time_ip1;                          // 类型1最小时间间隔_ip1；
    uint8_t type1_min_time_ip2;                          // 类型1最小时间间隔_ip2；
    uint8_t type1_min_time_ip3;                          // 类型1最小时间间隔_ip3；
    uint8_t type1_min_time_ip4;                          // 类型1最小时间间隔_ip4；
    uint16_t type1_min_time_inter_event;                 // 类型1事件间最小时间间隔；
    uint16_t type2_min_time_inter_event;                 // 类型2事件间最小时间间隔；
    uint16_t min_time_initialze_inter_event;             // 初始化事件间最小时间间隔；
    uint16_t type1_min_time_intra_event;                 // 类型1事件内最小时间间隔；
    uint16_t type2_min_time_intra_event;                 // 类型2事件内最小时间间隔；
    uint16_t min_time_initialze_intra_event;             // 初始化事件内最小时间间隔；
    uint16_t min_time_intra_event_group;                 // 事件组内最小时间间隔；
    uint16_t phase_cali_offset_cm;                       // 相位校准偏移量（厘米）；
    uint16_t tof_cali_offset_m;                          // 飞行时间校准偏移量（米）；
} sle_phone_slem_caps_t;

enum slem_phone_id {
    SLEM_G_POINT = 0,
    SLEM_T1_POINT = 1,
};

errcode_t sle_server_slem_init(void);
sle_phone_slem_caps_t *get_slem_local_caps(void);
sle_phone_slem_caps_t *get_slem_remote_caps(void);
errcode_t sle_server_slem_start(uint16_t conn_id);
errcode_t sle_server_slem_set_enable(uint16_t conn_id);
errcode_t sle_server_slem_set_disable(uint16_t conn_id);
errcode_t sle_server_slem_config_param(uint16_t conn_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif