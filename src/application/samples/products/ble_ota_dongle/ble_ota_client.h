/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: BLE OTA sample of client. \n
 *
 * History: \n
 * 2026-07-16, Create file. \n
 */
#ifndef BLE_OTA_CLIENT_H
#define BLE_OTA_CLIENT_H

#include "stdbool.h"
#include "btsrv_gatt_client.h"
#include "bts_gatt_client.h"
#include "sdk_init.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

void ble_ota_client_init(gattc_notification_callback notification_cb, gattc_indication_callback indication_cb);
void ble_ota_start_scan(void);
uint8_t get_g_ble_enable_status(void);
uint16_t get_ble_ota_conn_id(void);
uint8_t get_ble_ota_client_id(void);
gattc_handle_value_t *get_ble_ota_send_param(void);
uint8_t get_ssap_find_ready(void);
uint8_t get_ssap_connect_param_update_ready(void);
uint8_t get_ble_ota_conn_state(void);
void set_ble_ota_set_address_state(bool state);
bool get_ble_ota_set_address_state(void);
void set_ble_ota_scan_state(bool state);
bool get_ble_ota_scan_state(void);
void get_sle_remote_addr(bd_addr_t *addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif