/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Gateway demo -- automatic bidirectional routing between the first
 *              SLE connection and the first BLE connection. \n
 */
#ifndef GATEWAY_DEMO_H
#define GATEWAY_DEMO_H

#include "gateway_service.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Called by the service layer when a new connection is established.
 *        Automatically pairs the first SLE and first BLE connection.
 */
void gateway_demo_on_connect(const gw_conn_ctx_t *conn);

/**
 * @brief Called by the service layer when a connection is terminated.
 *        Removes the associated route.
 */
void gateway_demo_on_disconnect(const gw_conn_ctx_t *conn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* GATEWAY_DEMO_H */
