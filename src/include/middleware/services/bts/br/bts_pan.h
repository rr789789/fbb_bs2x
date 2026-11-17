/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: BT PAN module.
 */

/**
 * @defgroup bluetooth_bts_pan PAN API
 * @ingroup  bluetooth
 * @{
 */
#ifndef BTS_PAN_H
#define BTS_PAN_H

#include "bts_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Enum of pan net state.
 * @else
 * @brief Pan 网络状态
 * @endif
 */
typedef enum {
    PAN_NET_STATE_OFF,                      /*!< @if Eng Offline. @else 离线 @endif */
    PAN_NET_STATE_ON,                       /*!< @if Eng Online. @else 在线 @endif */
} pan_net_state_t;





/**
 * @if Eng
 * @brief The call back of net state change.
 * @par Description:
 *           The call back of net state change.
 * @param  [in]  net_state    See @ref pan_net_state_t
 * @retval No return value. See @ref void
 * @else
 * @brief  pan 网络连接状态通知
 * @par 说明:
 *           注册该回调函数之后，pan服务网络连接状态反馈上层应用。
 * @param  [in]  net_state    参考 @ref pan_net_state_t
 * @retval 无返回值。参考 @ref void
 * @endif
 * @par Dependency:
 *            @li pan_service.h
 */
typedef void (*pan_net_state_callback)(unsigned char net_state);

/**
 * @if Eng
 * @brief The call back of ethernet.
 * @par Description:The call back of ethernet.
 * @param  [in]  packet  a pointer to net data.
 * @param  [in]  packet_length  the length of net data.
 * @retval No return value. See @ref void
 * @else
 * @brief  读取网络数据包通知
 * @par 说明:注册该回调函数之后，读取网络数据包结果反馈上层应用。
 * @param  [in]  packet  指针，指向网络数据包所在内存地址.
 * @param  [in]  packet_length  网络数据包长度.
 * @retval 无返回值。参考 @ref void
 * @endif
 * @par Dependency:
 *            @li pan_service.h
 */
typedef void (*pan_rpt_data_callback)(unsigned char *packet, unsigned short packet_length);

/**
 * @if Eng
 * @brief Struct of pan callback function.
 * @else
 * @brief pan回调接口定义。
 * @endif
 */
typedef struct {
    pan_net_state_callback net_state_cb;
    pan_rpt_data_callback rpt_data_cb;
} pan_callbacks_t;



/**
 * @}
 */
#ifdef __cplusplus
}
#endif
#endif /* end of bts_pan.h */
