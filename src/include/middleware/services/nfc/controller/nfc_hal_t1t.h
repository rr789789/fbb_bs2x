/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: nfc hal T1T tag operations \n
 *
 * Date: 2022-11-11 \n
 */
/** @defgroup middleware_service_nfc_controller_hal_t1t NFC HAL T1T
 * @ingroup middleware_service_nfc_controller
 * @{
 */
#ifndef NFC_HAL_T1T_H
#define NFC_HAL_T1T_H

#include "nfc_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  The UID-echo frame length (4 lower bytes of UID) for T1T Tag.
 * @else
 * @brief  T1T卡片命令中携带的UID-echo部分的长度，是取UID的4个低字节。
 * @endif
 */
#define T1T_UID_ECHO_LEN                4

/**
 * @if Eng
 * @brief  The data length of T1T RID response.
 * @else
 * @brief  T1T读标志信息操作的回复的数据长度。
 * @endif
 */
#define T1T_RID_RES_LEN                 6

/**
 * @if Eng
 * @brief  The data length of T1T RALL response.
 * @else
 * @brief  T1T读所有静态存储空间内容的操作的回复的数据长度。
 * @endif
 */
#define T1T_RALL_RES_LEN                122

/**
 * @if Eng
 * @brief  The data length of T1T Read-Segment response.
 * @else
 * @brief  T1T读数据段操作回复的数据长度。
 * @endif
 */
#define T1T_RSEG_RES_DATA_LEN           128

/**
 * @if Eng
 * @brief  The data length of T1T Read 8 bytes response.
 * @else
 * @brief  T1T读8字节操作的回复的数据长度。
 * @endif
 */
#define T1T_READ8_RES_DATA_LEN          8

/**
 * @if Eng
 * @brief  The data byte length in T1T Write-Erase 8 bytes command.
 * @else
 * @brief  T1T带擦除的写8字节的命令里携带的数据长度。
 * @endif
 */
#define T1T_WRITE_E8_DATA_LEN           8

/**
 * @if Eng
 * @brief  The data byte length in T1T Write-No-Erase 8 bytes command.
 * @else
 * @brief  T1T不带擦除的写8字节的命令里携带的数据长度。
 * @endif
 */
#define T1T_WRITE_NE8_DATA_LEN          8

/**
 * @if Eng
 * @brief  Make address byte with block and byte index for T1T static memory.
 * @else
 * @brief  对于T1T静态存储空间，根据块号和块内字节序号，生成地址字节。
 * @endif
 */
#define MAKE_ADD(blk, byte)             ((((blk) & 0x0F) << 3) | ((byte) & 0x07))

/**
 * @if Eng
 * @brief  Make address byte with segment index for T1T dynamic memory.
 * @else
 * @brief  对于T1T动态存储空间，根据段序号，生成地址字节。
 * @endif
 */
#define MAKE_ADDS(seg)                  (((seg) & 0x0F) << 4)

/**
 * @if Eng
 * @brief  Make address byte with block index for T1T dynamic memory.
 * @else
 * @brief  对于T1T动态存储空间，根据块序号，生成地址字节。
 * @endif
 */
#define MAKE_ADD8(blk)                  (blk)

/**
 * @if Eng
 * @brief  T1T RID response structure.
 * @else
 * @brief  T1T RID命令的响应回复数据结构体。
 * @endif
 */
typedef struct {
    /** @if Eng  T1T header ROM byte 0.
     *  @else    T1T存储空间头部的字节0。
     *  @endif */
    uint8_t hr0;
    /** @if Eng  T1T header ROM byte 1.
     *  @else    T1T存储空间头部的字节1。
     *  @endif */
    uint8_t hr1;
    /** @if Eng  T1T UID-echo 4 bytes array.
     *  @else    T1T UID低四字节。
     *  @endif */
    uint8_t uid[T1T_UID_ECHO_LEN];
} NfcHalT1TRidRes;










#ifdef __cplusplus
}
#endif

#endif
/**
 * @}
 */
