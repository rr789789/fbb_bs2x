/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Provides sdio slave driver api \n
 *
 * History: \n
 * 2023-01-17, Create file. \n
 */

#ifndef SDIO_H
#define SDIO_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"
#include "sdio_porting.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup drivers_driver_sdio SDIO
 * @ingroup  drivers_driver
 * @{
 */

/**
 * @if Eng
 * @brief  SDIO message structure.
 * @else
 * @brief  SDIO消息结构体。
 * @endif
 */
typedef struct sdio_msg {
    uint32_t pending_msg;                       /*!< @if Eng Pending message.
                                                     @else   挂起中的消息 @endif */
    uint32_t sending_msg;                       /*!< @if Eng Sending message.
                                                     @else   发送中的消息 @endif */
} sdio_msg_t;

/**
 * @if Eng
 * @brief  SDIO works status.
 * @else
 * @brief  SDIO工作状态
 * @endif
 */
typedef enum {
    SDIO_CHANNEL_ERR    = 0x0,          /*!< @if Eng SDIO err.
                                         @else SDIO错误 @endif */
    SDIO_CHANNEL_RESET,                 /*!< @if Eng Reset SDIO.
                                         @else 重置SDIO @endif */
    SDIO_CHANNEL_INIT,                  /*!< @if Eng Initialize the SDIO.
                                         @else 初始化SDIO @endif */
    SDIO_CHANNEL_SLEEP,                 /*!< @if Eng Sleep the SDIO.
                                         @else SDIO睡眠状态 @endif */
    SDIO_CHANNEL_WAKE,                  /*!< @if Eng Wake the SDIO.
                                         @else 唤醒SDIO @endif */
    SDIO_CHANNEL_WORK,                  /*!< @if Eng SDIO work.
                                         @else SDIO工作状态 @endif */
    SDIO_CHANNEL_BUTT                   /*!< @if Eng SDIO status number.
                                         @else SDIO状态枚举数 @endif */
} sdio_channel_t;

/**
 * @if Eng
 * @brief  SDIO status informatio.
 * @else
 * @brief  SDIO状态信息。
 * @endif
 */
typedef struct sdio_status_info {
    uint8_t     allow_sleep;                    /*!< @if Eng Allow sleep.
                                                     @else   允许睡眠 @endif */
    uint8_t     tx_status;                      /*!< @if Eng Tx status.
                                                     @else   tx状态 @endif */
    uint8_t     sleep_status;                   /*!< @if Eng Sleep status.
                                                     @else   睡眠状态 @endif */
    sdio_channel_t  work_status;         /*!< @if Eng Work status.
                                                     @else   工作状态 @endif */
} sdio_status_info_t;

#if defined(CONFIG_SDIO_EXTEND_INFO)
/**
 * @if Eng
 * @brief  SDIO extend function structure.
 * @else
 * @brief  SDIO扩展功能结构体。
 * @endif
 */
typedef struct sdio_extendfunc {
    uint32_t                   int_stat;
    uint32_t                   msg_stat;
    uint32_t                   xfer_count;
    uint32_t                   credit_info;
    uint8_t                    comm_reg[SDIO_EXTENDREG_COUNT];
    int32_t                    valid_commreg_cnt;
    int8_t                     commreg_isvalid;
    int8_t                     credit_isvalid;
} sdio_extendfunc_t;
#endif

/**
 * @if Eng
 * @brief  SDIO status structure.
 * @else
 * @brief  SDIO状态结构体。
 * @endif
 */
typedef struct sdio_status {
    uint16_t          rd_arg_invalid_cnt;
    uint16_t          wr_arg_invlaid_cnt;
    uint16_t          unsupport_int_cnt;
    uint16_t          mem_int_cnt;
    uint16_t          fn1_wr_over;
    uint16_t          fn1_rd_over;
    uint16_t          fn1_rd_error;
    uint16_t          fn1_rd_start;
    uint16_t          fn1_wr_start;
    uint16_t          fn1_rst;
    uint16_t          fn1_msg_rdy;
    uint16_t          fn1_ack_to_arm_int_cnt;
    uint16_t          fn1_adma_end_int;
    uint16_t          fn1_suspend;
    uint16_t          fn1_resume;
    uint16_t          fn1_adma_int;
    uint16_t          fn1_adma_err;
    uint16_t          fn1_en_int;
    uint16_t          fn1_msg_isr;
    uint16_t          soft_reset_cnt;
} sdio_status_t;

/**
 * @if Eng
 * @brief  SDIO transfer channel structure.
 * @else
 * @brief  SDIO传输通道结构体。
 * @endif
 */
typedef struct sdio_chan_info {
    uint32_t                 send_data_len;
    uint16_t                 last_msg;
    uint16_t                 panic_forced_timeout;
    uint16_t                 chan_msg_cnt[D2H_MSG_COUNT];
} sdio_chan_info_t;

/**
 * @if Eng
 * @brief  SDIO information structure.
 * @else
 * @brief  SDIO信息结构体。
 * @endif
 */
typedef struct sdio_info {
    uint8_t                 volt_switch_flag;

    uint8_t                 host_to_device_msg_flag;

    uint16_t                reinit_times;
    uint16_t                gpio_int_times;
    uint16_t                pad;
    sdio_status_t           sdio_status;
    sdio_chan_info_t        chan_info;
    sdio_msg_t         sdio_msg_status;
} sdio_info_t;

/**
 * @if Eng
 * @brief  SDIO interrupt callback structure.
 * @else
 * @brief  SDIO中断回调函数结构体
 * @endif
 */
typedef struct sdio_callback_func {
    uint32_t (*read_start_callback)(uint32_t len, uint8_t *dma_tbl);    /*!< @if Eng HOST reading.
                                                                             @else DEVICE感知到HOST发起了读操作 @endif */
    uint32_t (*read_over_callback)(void);                               /*!< @if Eng HOST reading over.
                                                                             @else DEVICE感知到HOST读操作结束 @endif */
    void (*read_err_callback)(void);                                    /*!< @if Eng HOST read error.
                                                                             @else DEVICE感知到HOST读数据错误 @endif */
    uint32_t (*write_start_callback)(uint32_t len, uint8_t *dma_tbl);   /*!< @if Eng HOST writting.
                                                                             @else DEVICE感知到HOST发起了写操作 @endif */
    uint32_t (*write_over_callback)(void);                              /*!< @if Eng HOST write over.
                                                                             @else DEVICE感知到HOST写操作结束 @endif */
    void (*process_msg_callback)(uint32_t);                             /*!< @if Eng HOST getting message.
                                                                             @else DEVICE接收到HOST发来的消息 @endif */
    void (*soft_rst_callback)(void);                                    /*!< @if Eng HOST getting reset interruption.
                                                                             @else DEVICE接收到HOST发来的软复位 @endif */
} sdio_callback_func_t;

/**
 * @if Eng
 * @brief  use this callback to notify host msg event occurs.
 * @param  [in]  bus SDIO bus. see @ref sdio_bus_t
 * @else
 * @brief  通知host消息或数据事件存在的回调。
 * @param  [in]  bus SDIO通道， 参考 @ref sdio_bus_t
 * @endif
 */
typedef void (*notify_host_event_t)(sdio_bus_t bus);








#if defined(CONFIG_SDIO_EXTEND_INFO)

#endif

















#if defined(CONFIG_SDIO_EXTEND_INFO)

#endif

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
