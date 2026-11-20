/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: BLE ota server Config. \n
 *
 * History: \n
 * 2023-07-26, Create file. \n
 */
#ifndef BLE_OTA_SERVER_H
#define BLE_OTA_SERVER_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Service UUID */
#define BLE_OTA_UUID_SERVER_SERVICE 0xFF01
/* Characteristic UUID */
#define BLE_OTA_CHARACTERISTIC_UUID_TX 0xFF02

/* Client Characteristic Configuration UUID */
#define BLE_OTA_CLIENT_CHARACTERISTIC_CONFIGURATION 0x2902

void ble_ota_server_init(void);


errcode_t ble_ota_data_ack(uint16_t value_len, uint8_t *value);

typedef void (*ble_ota_chan_data_report) (uint8_t *data_ptr, uint16_t data_len);

void ble_ota_reg_chan_data_report_cbk(ble_ota_chan_data_report data_report);
errcode_t ble_ota_server_send_input_report(uint8_t *data, uint16_t len);
uint8_t ble_ota_get_connection_state(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif  /* __cplusplus */
#endif  /* __cplusplus */
#endif