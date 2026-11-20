/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: BLE OTA Server adv config. \n
 *
 * History: \n
 * 2023-07-26, Create file. \n
 */

#ifndef BLE_OTA_SERVER_ADV_H
#define BLE_OTA_SERVER_ADV_H

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef enum {
    BLE_PUBLIC_DEVICE_ADDRESS,
    BLE_RANDOM_DEVICE_ADDRESS,
    BLE_PUBLIC_IDENTITY_ADDRESS,
    BLE_RANDOM_STATIC_IDENTITY_ADDRESS
} ble_address_t;

uint8_t ble_ota_set_adv_data(void);
uint8_t ble_ota_start_adv(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif