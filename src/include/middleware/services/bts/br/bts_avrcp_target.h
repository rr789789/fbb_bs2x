/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: BT AVRCP target module.
 */

/**
 * @defgroup bluetooth_bts_avrcp_target AVRCP TG API
 * @ingroup  bluetooth
 * @{
 */

#ifndef BTS_AVRCP_TARGET_H
#define BTS_AVRCP_TARGET_H

#include "bts_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Callback invoked in response to avrcp tg connection state changed.
 * @par Description:
 *           When registered,the bts uses this callback to deliver avrcp tg connect stat info updated
 *           after connection state updated.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bdAddr Type  #BdAddr *, a poniter of the peer bluetooth device addresss.
 * @param  [in]  state  Type  #BtProfileConnectState avtcp tg connection state.
 * @retval void  no return value.
 * @par Dependency:
 * @li bts_avrcp_target.h
 * @see  BtAvrcpTgCallbacks
 * @else
 * @brief  avrcp tg连接状态改变回调函数。
 * @par 说明:注册该回调函数之后，BTS每次完成avrcp tg状态更新后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bdAddr Type  #BdAddr *, 对端蓝牙设备的地址信息。
 * @param  [in]  state  Type  #BtProfileConnectState ,avrcp tg连接状态，参考 {@link BtProfileConnectState}。
 * @retval void  无返回值。
 * @par 依赖:
 * @li bts_avrcp_target.h
 * @see  BtAvrcpTgCallbacks
 * @endif
 */
typedef void (*avrcp_tg_conn_state_changed_callback)(const bd_addr_t *bd_addr, profile_connect_state_t state);

/**
 * @if Eng
 * @brief Struct of avrcp tg callback function.
 * @else
 * @brief avrcp tg回调接口定义。
 * @endif
 */
typedef struct {
    avrcp_tg_conn_state_changed_callback conn_state_changed_cb;
} avrcp_tg_callbacks_t;















/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
/** @} */

