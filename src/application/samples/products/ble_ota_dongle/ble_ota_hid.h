/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE OTA hid Config. \n
 *
 * History: \n
 * 2024-02-01, Create file. \n
 */

#ifndef BLE_OTA_HID_H
#define BLE_OTA_HID_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define USB_HID_OTA_MAX_KEY_LENTH      6
#define BLE_OTA_DONGLE_OK              0
#define BLE_OTA_DONGLE_FAILED          1

int32_t ble_ota_dongle_set_keyboard_report_desc_hid(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif