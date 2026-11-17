/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: Provides SHA256 header
 * Author: 
 * Create: 2023-12-13
*/

#ifndef SECURITY_SHA256_H
#define SECURITY_SHA256_H

#include <stdint.h>
#include "errcode.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @defgroup security_unified_sha256 SHA256
 * @ingroup  drivers_driver_security_unified
 * @{
 */

#ifndef SHA256_HASH_SIZE
#define SHA256_HASH_SIZE    32
#endif


/**
 * @if Eng
 * @brief  Sha256 calculation
 * @note   A sha256 handle must have been created before this API is called.


/**
 * @if Eng
 * @brief  Sha256 calculation
 * @param  [in]    buf Source buffer pointer.
 * @param  [in]    len Source buffer size.
 * @param  [out]   out Pointer to the buffer address for storing the summary information.
 * @param  [inout] out_len Pointer to the size of the buffer for storing the summary information.
                           The input is the buffer length, and the output is the actual digest length.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @else
 * @brief  Sha256计算
 * @param  [in]    buf 源缓冲区指针。
 * @param  [in]    len 缓冲区大小。
 * @param  [out]   out 存储摘要信息的缓冲区地址指针。
 * @param  [in]    out_len 存储摘要信息的缓冲区大小指针。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败，参考 @ref errcode_t 。
 * @endif
 */


/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
