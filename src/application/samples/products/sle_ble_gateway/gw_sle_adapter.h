/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE adapter API for the gateway (SSAP Server mode). \n
 */
#ifndef GW_SLE_ADAPTER_H
#define GW_SLE_ADAPTER_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Register SLE device-manager callbacks (power-on / enable).
 *        MUST be called from app_run context, before task creation.
 */
errcode_t gw_sle_dev_register_cbks(void);

/**
 * @brief Register SLE SSAP-server, connection and announce callbacks,
 *        then start SLE advertising.
 *        Called from the gateway task after gw_service_init().
 */
errcode_t gw_sle_adapter_init(void);

/**
 * @brief Send data to a connected SLE peer via SSAP notify/indicate.
 */
errcode_t gw_sle_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                               const uint8_t *data, uint16_t len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* GW_SLE_ADAPTER_H */
