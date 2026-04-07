/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: BLE adapter API for the gateway (GATT Server mode). \n
 */
#ifndef GW_BLE_ADAPTER_H
#define GW_BLE_ADAPTER_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Register BLE callbacks, wait for BLE enable, register GATT server,
 *        add service, and start BLE advertising.
 *        Called from the gateway task after gw_service_init().
 */
errcode_t gw_ble_adapter_init(void);

/**
 * @brief Send data to a connected BLE peer via GATT notification.
 */
errcode_t gw_ble_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                               const uint8_t *data, uint16_t len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* GW_BLE_ADAPTER_H */
