/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: dfx uart channel
 * This file should be changed only infrequently and with great care.
 */
#ifndef DFX_CHANNEL_H
#define DFX_CHANNEL_H

#include <stdint.h>
#include <diag.h>
#include "errcode.h"

#if defined(CONFIG_SUPPORT_LOG_CUSTOM_OUTPUT)
typedef void (*diag_custom_output_func_t)(uint8_t *data, uint8_t length);
typedef void (*diag_trigger_func_t)(void);
#endif  /* CONFIG_SUPPORT_LOG_CUSTOM_OUTPUT */
#if defined(CONFIG_BT_OTA_SERVICE_ENABLE) && (CONFIG_BT_OTA_SERVICE_ENABLE == 1)
/**
 * @if Eng
 * @brief  Definition of the callback function for sending OTA GATT data.
 * @param [in] value data.
 * @param [in] value_len data length.
 * @retval error code.
 * @else
 * @brief  ota gatt数据发送回调函数定义。
 * @param [in] value 数据。
 * @param [in] value_len 数据长度。
 * @retval error code.
 * @endif
 */
typedef errcode_t (*ble_ota_service_ack) (uint16_t value_len, uint8_t *value);

/**
 * @if Eng
 * @brief Callback for sending OTA data from the server.
 * @else
 * @brief 服务端发送ota数据回调。
 * @endif
 */
void diag_ble_ota_service_ack_register(ble_ota_service_ack ack_data);

/**
 * @if Eng
 * @brief  Definition of the callback function for sending OTA GATT data.
 * @param [in] data data.
 * @param [in] size data length.
 * @retval error code.
 * @else
 * @brief  gatt ota 包接收处理函数。
 * @param [in] data 数据。
 * @param [in] size 数据长度。
 * @retval error code.
 * @endif
 */
errcode_t diag_channel_rx_bt_service_ota_data(uint8_t *data, uint16_t size);
#endif

#if defined(CONFIG_SLE_UPG_ENABLE) && (CONFIG_SLE_UPG_ENABLE== 1)
/**
 * @if Eng
 * @brief  Definition of the callback function for sending OTA SSAP data.
 * @param [in] value data.
 * @param [in] value_len data length.
 * @retval error code.
 * @else
 * @brief  ota gatt数据发送回调函数定义。
 * @param [in] value 数据。
 * @param [in] value_len 数据长度。
 * @retval error code.
 * @endif
 */
typedef errcode_t (*sle_ota_service_ack) (uint16_t value_len, uint8_t *value);

/**
 * @if Eng
 * @brief Callback for sending OTA data from the server.
 * @else
 * @brief 服务端发送ota数据回调。
 * @endif
 */
void diag_sle_ota_service_ack_register(sle_ota_service_ack ack_data);

/**
 * @if Eng
 * @brief  Definition of the callback function for sending OTA SSAP data.
 * @param [in] data data.
 * @param [in] size data length.
 * @retval error code.
 * @else
 * @brief  ssap ota 包接收处理函数。
 * @param [in] data 数据。
 * @param [in] size 数据长度。
 * @retval error code.
 * @endif
 */
void diag_channel_rx_sle_data(uint8_t *data, uint16_t size);
#endif
void diag_uart_rx_proc(uint8_t *buffer, uint16_t length);
errcode_t diag_register_channel(void);

#if defined(CONFIG_SUPPORT_LOG_CUSTOM_OUTPUT)
void uapi_diag_register_custom_output(diag_custom_output_func_t func);
void uapi_diag_set_dest_id(uint8_t dest_id);
void uapi_diag_switch_custom_output(bool en);
bool uapi_diag_get_custom_output_status(void);

bool uapi_diag_block_check(void);
void uapi_diag_block_clear(void);
void uapi_diag_register_trigger_func(diag_trigger_func_t func);
void uapi_diag_switch_trigger_enable(bool en);
void uapi_diag_enable_trans(uint8_t id, uint8_t count);

#endif  /* CONFIG_SUPPORT_LOG_CUSTOM_OUTPUT */
#endif