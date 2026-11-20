/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023. All rights reserved.
 */

/**
 * @defgroup sle_srv_ota API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_RCU_OTA_SERVER_H
#define SLE_RCU_OTA_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Property Property */
#define SLE_OTA_PROPERTY_PERMISSION  SSAP_PERMISSION_AUTHORIZATION_NEED
#define SLE_OTA_DESC_PERMISSION  (SSAP_PERMISSION_READ | SSAP_PERMISSION_WRITE)
#define SLE_OTA_OPINDICAT   (SSAP_OPERATE_INDICATION_BIT_READ | SSAP_OPERATE_INDICATION_BIT_WRITE_NO_RSP | \
    SSAP_OPERATE_INDICATION_BIT_NOTIFY)
#define SLE_OTA_DESC_OPINDICAT (SSAP_OPERATE_INDICATION_BIT_READ | SSAP_OPERATE_INDICATION_BIT_WRITE_NO_RSP | \
    SSAP_OPERATE_INDICATION_BIT_WRITE)
/* Service UUID */
#define SLE_OTA_SERVER_SERVICE        0xFF01

/* Property UUID */
#define SLE_OTA_SERVER_PROPERTY     0xFF02


#define OCTET_BIT_LEN   8
#define UUID_LEN_2      2

errcode_t sle_ota_service_init(uint8_t server_id);
#ifdef __cplusplus
}
#endif
#endif /* SLE_SRV_OTA_H */

/**
* @}
*/