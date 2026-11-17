/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: nfc hal T2T tag operations \n
 *
 * Date: 2022-11-15 \n
 */
/** @defgroup middleware_service_nfc_controller_hal_t2t NFC HAL T2T
 * @ingroup middleware_service_nfc_controller
 * @{
 */
#ifndef NFC_HAL_T2T_H
#define NFC_HAL_T2T_H

#include "nfc_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Number of bytes that can be read at one time from a T2T card.
 * @else
 * @brief  T2T卡每次读取的字节数。
 * @endif
 */
#define T2T_READ_DATA_LEN   16

/**
 * @if Eng
 * @brief  Number of bytes that can be written at one time to a T2T card.
 * @else
 * @brief  T2T卡每次写入的字节数。
 * @endif
 */
#define T2T_WRITE_DATA_LEN  4




#ifdef __cplusplus
}
#endif

#endif
/**
 * @}
 */
