/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides CAN driver api \n
 *
 * History: \n
 * 2023-02-22, Create file. \n
 */
#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"
#include "common_def.h"
#include "can_porting.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup drivers_driver_can CAN
 * @ingroup  drivers_driver
 * @{
 */

#define build_can_fd_format_standard_data_frame(id, bit_rate_switch) { \
    .std_id = (id), \
    .extend_id = 0, \
    .rtr = 0, \
    .ide = 0, \
    .fdf = 1, \
    .brs = (bit_rate_switch), \
}

#define build_can_fd_format_extend_data_frame(id, ex_id, bit_rate_switch) { \
    .std_id = (id), \
    .extend_id = (ex_id), \
    .rtr = 0, \
    .ide = 1, \
    .fdf = 1, \
    .brs = (bit_rate_switch), \
}
#define build_can_standard_data_frame(id) { \
    .std_id = (id), \
    .extend_id = 0, \
    .rtr = 0, \
    .ide = 0, \
    .fdf = 0, \
    .brs = 0, \
}

#define build_can_extend_data_frame(id, ex_id) { \
    .std_id = (id), \
    .extend_id = (ex_id), \
    .rtr = 0, \
    .ide = 1, \
    .fdf = 0, \
    .brs = 0, \
}

#define build_can_standard_remote_frame(id) { \
    .std_id = (id), \
    .extend_id = 0, \
    .rtr = 1, \
    .ide = 0, \
    .fdf = 0, \
    .brs = 0, \
}

#define build_can_extend_remote_frame(id, ex_id) { \
    .std_id = (id), \
    .extend_id = (ex_id), \
    .rtr = 1, \
    .ide = 1, \
    .fdf = 0, \
    .brs = 0, \
}

/**
 * @if Eng
 * @brief  Standard CAN Baud rate.
 * @else
 * @brief  标准CAN协议波特率。
 * @endif
 */
typedef enum can_baud_rate {
    CAN_BPS_10K = 10,
    CAN_BPS_20K = 20,
    CAN_BPS_50K = 50,
    CAN_BPS_125K = 125,
    CAN_BPS_250K = 250,
    CAN_BPS_500K = 500,
    CAN_BPS_800K = 800,
    CAN_BPS_1M = 1000,
    CAN_BPS_MAX = 1000
} can_baud_rate_t;

/**
 * @if Eng
 * @brief  CAN-FD Data segment Baud rate.
 * @else
 * @brief  CAN-FD数据段波特率。
 * @endif
 */
typedef enum can_fd_baud_rate {
    CAN_FD_BPS_10K = 10,
    CAN_FD_BPS_20K = 20,
    CAN_FD_BPS_50K = 50,
    CAN_FD_BPS_125K = 125,
    CAN_FD_BPS_250K = 250,
    CAN_FD_BPS_500K = 500,
    CAN_FD_BPS_800K = 800,
    CAN_FD_BPS_1M = 1000,
    CAN_FD_BPS_2M = 2000,
    CAN_FD_BPS_4M = 4000,
    CAN_FD_BPS_5M = 5000,
    CAN_FD_BPS_8M = 8000,
    CAN_FD_BPS_10M = 10000,
    CAN_FD_BPS_MAX = 10000
} can_fd_baud_rate_t;

/**
 * @if Eng
 * @brief  CAN Sample point.
 * @else
 * @brief  CAN采样点。
 * @endif
 */
typedef enum can_sample_point {
    CAN_SAMPLE_PERCENT_75,
    CAN_SAMPLE_PERCENT_80,
    CAN_SAMPLE_PERCENT_85,
    CAN_SAMPLE_PERCENT_MAX
}can_sample_point_t;

/**
 * @if Eng
 * @brief  CAN Interrupt Status.
 * @else
 * @brief  CAN中断状态。
 * @endif
 */
typedef enum can_int_sts {
    CAN_FRAME_RECEIVED_INT,
    CAN_ERROR_PASSIVE_INT,
    CAN_BUS_OFF_INT,
} can_int_sts_t;

/**
 * @if Eng
 * @brief  CAN opreation mode.
 * @else
 * @brief  CAN 工作模式。
 * @endif
 */
typedef enum can_mode {
    NORMAL_OPREATION = 0x0,
    LOOP_BACK_MODE = 0x1
} can_mode_t;

/**
 * @if Eng
 * @brief  CAN filter type.
 * @else
 * @brief  CAN 滤波器格式。
 * @endif
 */
typedef enum filter_type {
    RANGE_FILTER = 0x0,
    DUAL_ID_FILTER,
    CLASSIC_FILTER = 0x3
} can_filter_type_t;

/**
 * @if Eng
 * @brief  CAN controller status.
 * @else
 * @brief  CAN 控制器状态。
 * @endif
 */
typedef enum can_controller_sts {
    CAN_STS_ERROR_ACTIVE,
    CAN_STS_ERROR_PASSIVE,
    CAN_STS_BUS_OFF,
    CAN_BUS_NOT_INIT
} can_controller_sts_t;

/**
 * @if Eng
 * @brief  Nominal and Data bit time config.
 * @else
 * @brief  配置标称位时间和数据段位时间。
 * @endif
 */
typedef struct can_global_config {
    uint32_t std_baud_rate;            /*!< @if Eng Standard CAN Baud rate. @ref can_baud_rate_t
                                            @else 标准CAN协议波特率。 @ref can_baud_rate_t @endif */
    uint32_t fd_baud_rate;             /*!< @if Eng CAN-FD Data segment Baud rate. @ref can_fd_baud_rate_t
                                            @else CAN-FD数据段波特率。 @ref can_fd_baud_rate_t @endif */
    uint8_t point;                     /*!< @if Eng CAN Sample point. @ref can_sample_point_t
                                            @else CAN采样点。 @ref can_sample_point_t @endif */
    uint8_t can_mode;                  /*!< @if Eng CAN opreation mode. @ref can_sample_point_t
                                            @else CAN采样点。 @ref can_sample_point_t @endif */
} can_global_config_t;

/**
 * @if Eng
 * @brief  CAN filter structure.
 * @else
 * @brief  CAN滤波结构。
 * @endif
 */
typedef struct can_filter {
    uint32_t fid1;                     /*!< @if Eng First ID of the filter.
                                            @else   滤波器的首要ID @endif */
    uint32_t fid2;                     /*!< @if Eng Second ID of the filter, which can be used as a mask,
                                                    secondary identifier, or right boundary.
                                            @else   滤波器的次要ID，可作为掩码、次要标识符或右边界。 @endif */
    uint8_t fifo_id;                   /*!< @if Eng Index of the Rx-FIFO bound to the filter.
                                                    Interrupt priorities are sorted by index in ASC order.
                                            @else   滤波器绑定的Rx-FIFO索引。中断优先级按索引升序排列。 @endif */
    uint8_t filter_type;               /*!< @if Eng Filter type.
                                            @else   滤波器模式 @endif */
} can_filter_t;

typedef struct can_filter_list {
    can_filter_t *filters;             /*!< @if Eng Filter list.
                                            @else 滤波器列表。 @endif */
    uint8_t filter_num;                /*!< @if Eng Filter number.
                                            @else 滤波器个数。 @endif */
} can_filter_list_t;

/**
 * @if Eng
 * @brief  Structure of the CAN frame Arbitration field.
 * @else
 * @brief  CAN帧仲裁域结构。
 * @endif
 */
typedef struct can_msg_attr {
    uint32_t std_id;                   /*!< @if Eng Standard identifier.
                                            @else   标准标识符。 @endif */
    uint32_t extend_id;                /*!< @if Eng Extra identifier.
                                            @else   扩展标识符。 @endif */
    uint8_t rtr;                       /*!< @if Eng Remote frame flag.
                                            @else   远程帧标志。 @endif */
    uint8_t ide;                       /*!< @if Eng Extended format flag.
                                            @else   扩展格式标志。 @endif */
    uint8_t fdf;                       /*!< @if Eng FD format flag.
                                            @else   FD帧格式标志。 @endif */
    uint8_t brs;                       /*!< @if Eng Bit rate switch.
                                            @else   波特率切换标志。 @endif */
} can_msg_attr_t;

/**
 * @if Eng
 * @brief  CAN interrupt callback function.
 * @param  [in]  bus CAN bus line select.
 * @param  [in]  status CAN Interrupt Status. @ref can_int_sts_t
 * @param  [in]  received_msg Arbitration field of the received CAN frame.
 * @param  [in]  data Received data.
 * @param  [in]  length Received data length.
 * @else
 * @brief  CAN中断回调函数。
 * @param  [in]  bus CAN总线选择。
 * @param  [in]  status CAN中断状态。 @ref can_int_sts_t
 * @param  [in]  received_msg 接收到的CAN帧仲裁域。
 * @param  [in]  data 接收到的数据。
 * @param  [in]  length 接收到的数据长度。
 * @endif
 */
typedef void (*can_callback_t)(can_int_sts_t status, can_msg_attr_t *received_msg,
                               uint8_t *data, uint8_t length);










/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
