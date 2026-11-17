/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved. \n
 *
 * Description: Provides cipher aead driver header \n
 * Author:  \n
 * History: \n
 * 2023-03-14, Create file. \n
 */
#ifndef CIPHER_AEAD_H
#define CIPHER_AEAD_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @defgroup security_unified_cipher_aead Cipher AEAD
 * @ingroup  drivers_driver_security_unified
 * @{
 */



/**
 * @if Eng
 * @brief  GCM create channel handle.
 * @param  [out]   handle Pointer to the created channel handle.
 * @param  [in]    key Encryption/Decryption key.
 * @param  [in]    key_len Encryption/Decryption key length in byte. Only support 16,24,32.
 * @param  [in]    keyslot_handle The keyslot handle which store the encryption/decryption key.
 * @param  [in]    iv The initial vector.
 * @param  [in]    iv_len The initial vector lenght in byte.
 * @param  [in]    aad The additional authentication message. NULL is support.
 * @param  [in]    aad_len The additional authentication message length in byte.
 * @param  [in]    tag_len The expected tag length in byte.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @note   There are two methods to set the key:
            1）Using key/key_len, in which case, the keyslot_handle is set to 0.
            2) Calling keyslot api to create one keyslot_handle and set it, in which case,
                the key is set to NULL and key_len is 0.
            When keyslot is supported, you must use method 2, otherwise you should use method 1.
 * @else
 * @brief  GCM 创建通道句柄.
 * @param  [out]   handle 指向创建的通道句柄的指针.
 * @param  [in]    key 加解密密钥.
 * @param  [in]    key_len 加解密密钥长度，单位是 Byte. 只支持 16,24,32.
 * @param  [in]    keyslot_handle 保存加解密密钥的 keyslot 句柄.
 * @param  [in]    iv 初始向量
 * @param  [in]    iv_len 初始向量长度，单位是 Byte.
 * @param  [in]    aad 附加认证信息，支持传入为 NULL
 * @param  [in]    aad_len 附加认证信息，单位是 Byte.
 * @param  [in]    tag_len 期望的 tag 长度，单位是 Byte.
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败，参考 @ref errcode_t 。
 * @note   有两种方法设置密钥:
            1）使用 key/key_len 组合，在这种情况下，keyslot_handle 传入 0.
            2）使用 keyslot 接口创建 keyslot 句柄传入，在这种情况下，key 传入为 NULL，key_len 传入为 0.
            若支持 keyslot，则必须使用方法2，否则只能使用方法1.
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
