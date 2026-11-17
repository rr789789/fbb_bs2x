/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: BT SPP module.
 */

/**
 * @defgroup bluetooth_bts_spp SPP API
 * @ingroup  bluetooth
 * @{
 */

#ifndef BTS_SPP_H
#define BTS_SPP_H

#include <stdbool.h>
#include "bts_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  SPP is disconnected.
 * @else
 * @brief  SPP连接已经断开。
 * @endif
 */
#define SPP_READ_SOCKET_CLOSED (0)

/**
 * @if Eng
 * @brief  Read data failed.
 * @else
 * @brief  读取数据失败。
 * @endif
 */
#define SPP_READ_FAILED (-1)

/**
 * @if Eng
 * @brief  Write data failed.
 * @else
 * @brief  写数据失败。
 * @endif
 */
#define SPP_WRITE_FAILED (-1)

/**
 * @if Eng
 * @brief  Invalid SPP server/client ID.
 * @else
 * @brief  非法SPP Server 或 Client ID。
 * @endif
 */
#define SPP_INVALID_ID (-1)

/**
 * @if Eng
 * @brief  Enum of SPP socket type.
 * @else
 * @brief  SPP socket类型定义枚举。
 * @endif
 */
typedef enum {
    SPP_SOCKET_RFCOMM = 0x0,    /*!< @if Eng RFCOMM
                                     @else   RFCOMM @endif */
} spp_socket_type_t;

/**
 * @if Eng
 * @brief  Parameters for create SPP socket.
 * @else
 * @brief  创建SPP socket的参数。
 * @endif
 */
typedef struct {
    bt_uuid_t uuid;                /*!< @if Eng UUID of service.
                                        @else   SPP socket对应的服务UUID。 @endif */
    spp_socket_type_t socket_type; /*!< @if Eng SPP socket type
                                        @else SPP socket类型。@endif */
                                        @else 是否加密（目前此参数未使用，默认加密）。@endif */
} spp_create_socket_para_t;









/**
 * @if Eng
 * @brief Callback function for connection event.
 * @par Description: When registered, this callback reports connection event to upper layer application.
 * @param  [in]  client_id The relative ID used to identify the current client socket.
 * @param  [in]  state    Connect state of spp. See {@link profile_connect_state_t}
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_spp.h
 * @see spp_client_callbacks_t
 * @else
 * @brief 连接变化事件回调函数。
 * @par 说明: 注册该回调函数之后，bluetooth调用该回调函数将连接事件上报给上层。
 * @param  [in]  client_id 入参，用来指定当前操作Client的ID。
 * @param  [in]  state    spp的连接状态。见 {@link profile_connect_state_t}
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_spp.h
 * @see spp_callbacks_t
 * @endif
 */
typedef void (*spp_conn_state_changed_callback)(uint8_t client_id, profile_connect_state_t state);

/**
 * @if Eng
 * @brief Callback function for data reception.
 * @par Description: When registered, this callback reports received data to upper layer application.
 * @param  [in]  client_id The relative ID used to identify the current client socket.
 * @param  [in]  data     Received data.
 * @param  [in]  data_len      Length of data.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_spp.h
 * @see spp_client_callbacks_t
 * @else
 * @brief 接收到数据回调函数。
 * @par 说明: 注册该回调函数之后，bluetooth调用该回调函数将接收到的数据上报给上层。
 * @param  [in]  client_id 入参，用来指定当前操作Client的ID。
 * @param  [in]  data     接收到的数据。
 * @param  [in]  data_len      数据长度。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_spp.h
 * @see spp_callbacks_t
 * @endif
 */
typedef void (*spp_receive_data_callback)(uint8_t client_id, uint8_t *data, uint32_t data_len);

/**
 * @if Eng
 * @brief Struct of SPP callback functions.
 * @else
 * @brief SPP回调函数接口定义。
 * @endif
 */
typedef struct {
    spp_receive_data_callback receive_data_cb;
    spp_conn_state_changed_callback conn_state_changed_cb;
} spp_callbacks_t;


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
