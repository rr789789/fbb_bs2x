/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved. \n
 *
 * Description: Provides pke driver header \n
 * Author:  \n
 * History: \n
 * 2023-03-14, Create file. \n
 */
#ifndef UNIFIED_CIPHER_PKE_H
#define UNIFIED_CIPHER_PKE_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @defgroup security_unified_pke PKE
 * @ingroup  drivers_driver_security_unified
 * @{
 */

/**
 * @if Eng
 * @brief ECC curve type
 * @note  TDES is not secure, and we advise not to use it.
 * @else
 * @brief ECC曲线类型
 * @note  TDES不安全，不建议使用。
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_RFC5639_P256 = 0,      /* RFC 5639 - Brainpool P256/384/512 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_RFC5639_P384,          /* RFC 5639 - Brainpool P256/384/512 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_RFC5639_P512,          /* RFC 5639 - Brainpool P256/384/512 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_FIPS_P256K,            /* NIST FIPS 186-4 P192/224/256/384/521, suggest not to use */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_FIPS_P192R,            /* NIST FIPS 186-4 P192/224/256/384/521, suggest not to use */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_FIPS_P224R,            /* NIST FIPS 186-4 P192/224/256/384/521, suggest not to use */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_FIPS_P256R,            /* NIST FIPS 186-4 P192/224/256/384/521, suggest not to use */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_FIPS_P384R,            /* NIST FIPS 186-4 P192/224/256/384/521, suggest not to use */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_FIPS_P521R,            /* NIST FIPS 186-4 P192/224/256/384/521, suggest not to use */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_RFC7748,               /* RFC 7748 - Curve25519 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_RFC7748_448,           /* RFC 7748 - Curve448 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_RFC8032,               /* RFC 8032 - ED25519 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_SM2,                   /* GMT 0003.2-2012 */
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_MAX,
    UAPI_DRV_CIPHER_PKE_ECC_TYPE_INVALID = 0xffffffff,
} uapi_drv_cipher_pke_ecc_curve_type_t;

/**
 * @if Eng
 * @brief Padding mode of the RSA algorithm
 * @note  PKCS1_V15 is not secure, and we advise not to use it.
 * @else
 * @brief RSA算法填充方式
 * @note  PKCS1_V15不安全，不建议使用。
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_PKE_RSA_SCHEME_PKCS1_V15 = 0x00,    /* not security, suggest not to use */
    UAPI_DRV_CIPHER_PKE_RSA_SCHEME_PKCS1_V21,
    UAPI_DRV_CIPHER_PKE_RSA_SCHEME_MAX,
    UAPI_DRV_CIPHER_PKE_RSA_SCHEME_INVALID = 0xffffffff,
} uapi_drv_cipher_pke_rsa_scheme_t;

/**
 * @if Eng
 * @brief Hash algorithm type used for RSA padding
 * @note  SHA1 and SHA224 is not secure, and we advise not to use it.
 * @else
 * @brief RSA填充使用的hash算法类型
 * @note  SHA1和SHA224不安全，不建议使用。
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_SHA1 = 0x00,  /* not security, suggest not to use */
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_SHA224,
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_SHA256,
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_SHA384,
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_SHA512,
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_SM3,
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_MAX,
    UAPI_DRV_CIPHER_PKE_HASH_TYPE_INVALID = 0xffffffff,
} uapi_drv_cipher_pke_hash_type_t;

/**
 * @if Eng
 * @brief Buffer security attribute for RSA input messages
 * @else
 * @brief RSA输入消息的缓冲区安全属性
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_PKE_BUF_NONSECURE = 0x00,
    UAPI_DRV_CIPHER_PKE_BUF_SECURE,
    UAPI_DRV_CIPHER_PKE_BUF_INVALID = 0xffffffff,
} uapi_drv_cipher_pke_buffer_secure_t;

/**
 * @if Eng
 * @brief  Common PKE data structure
 * @else
 * @brief PKE通用数据结构体
 * @endif
 */
typedef struct {
    uint32_t  length;   /*!< @if Eng PKE common data buffer length.
                             @else   PKE通用数据缓冲区长度。 @endif */
    uint8_t  *data;     /*!< @if Eng PKE common data buffer.
                             @else   PKE通用数据缓冲区。 @endif */
} uapi_drv_cipher_pke_data_t;

/**
 * @if Eng
 * @brief  ECC public key structure
 * @else
 * @brief ECC公钥结构体
 * @endif
 */
typedef struct {
    uint8_t *x;    /*!< @if Eng X coordinates of the generated public key, the caller ensures it is padded with leading
                                zeros if the effective size of this key is smaller than ecc key size.
                        @else   公钥的X坐标，调用方确保如果此密钥的有效大小小于ecc密钥大小，则用前导零填充。 @endif */
    uint8_t *y;    /*!< @if Eng Y coordinates of the generated public key, the caller ensures it is padded with leading
                                zeros if the effective size of this key is smaller than ecc key size.
                        @else   公钥的Y坐标，调用方确保如果此密钥的有效大小小于ecc密钥大小，则用前导零填充。 @endif */
    uint32_t length;    /*!< @if Eng ECC public key length.
                             @else   RCC公钥长度。 @endif */
} uapi_drv_cipher_pke_ecc_point_t;

/**
 * @if Eng
 * @brief  ECC signature structure
 * @else
 * @brief ECC签名结构体
 * @endif
 */
typedef struct {
    uint8_t *r;    /*!< @if Eng ECC signature R.
                        @else   ECC签名值R。 @endif */
    uint8_t *s;    /*!< @if Eng ECC signature S.
                        @else   ECC签名值S。 @endif */
    uint32_t length;    /*!< @if Eng Length of the ECC signature.
                             @else   ECC签名数据长度。 @endif */
} uapi_drv_cipher_pke_ecc_sig_t;

/**
 * @if Eng
 * @brief  ECC input message structure
 * @else
 * @brief ECC输入消息结构体
 * @endif
 */
typedef struct {
    uint32_t  length;    /*!< @if Eng Length of the ECC input message buffer.
                              @else   ECC输入消息缓冲区长度。 @endif */
    uint8_t  *data;      /*!< @if Eng ECC input message buffer.
                             @else   ECC输入消息缓冲区。 @endif */
    uapi_drv_cipher_pke_buffer_secure_t buf_sec;
} uapi_drv_cipher_pke_msg_t;

/**
 * @if Eng
 * @brief  RSA private key structure
 * @else
 * @brief RSA私钥结构体
 * @endif
 */
typedef struct {
    uint8_t *n;          /*!< @if Eng RSA public modulus.
                              @else   RSA秘钥参数n。 @endif */
    uint8_t *e;          /*!< @if Eng public exponent.
                              @else   RSA公钥参数e。 @endif */
    uint8_t *d;          /*!< @if Eng private exponent.
                              @else   RSA私钥参数d。 @endif */
    uint8_t *p;          /*!< @if Eng 1st prime factor.
                              @else   RSA第一素数因子。 @endif */
    uint8_t *q;          /*!< @if Eng 2nd prime factor.
                              @else   RSA第二素数因子。 @endif */
    uint8_t *dp;         /*!< @if Eng D % (P - 1).
                              @else   D % (P - 1)的结果。 @endif */
    uint8_t *dq;         /*!< @if Eng D % (Q - 1).
                              @else   D % (Q - 1)的结果。 @endif */
    uint8_t *qp;         /*!< @if Eng 1 / (Q % P).
                              @else   1 / (Q % P)的结果。 @endif */
    uint16_t n_len;      /*!< @if Eng length of public modulus.
                              @else   RSA秘钥参数n的长度。 @endif */
    uint16_t e_len;      /*!< @if Eng length of public exponent.
                              @else   RSA公钥参数e的长度。 @endif */
    uint16_t d_len;      /*!< @if Eng length of private exponent.
                              @else   RSA私钥参数d的长度。 @endif */
    uint16_t p_len;      /*!< @if Eng length of 1st prime factor,should be half of u16NLen.
                              @else   RSA第一素因子的长度，应该是u16NLen的一半。 @endif */
    uint16_t q_len;      /*!< @if Eng length of 2nd prime factor,should be half of u16NLen.
                              @else   RSA第二素因子的长度，应该是u16NLen的一半。 @endif */
    uint16_t dp_len;     /*!< @if Eng length of D % (P - 1),should be half of u16NLen.
                              @else   D % (P - 1)结果的长度，应该是u16NLen的一半。 @endif */
    uint16_t dq_len;     /*!< @if Eng length of D % (Q - 1),should be half of u16NLen.
                              @else   D % (Q - 1)结果的长度，应该是u16NLen的一半。 @endif */
    uint16_t qp_len;     /*!< @if Eng length of 1 / (Q % P),should be half of u16NLen.
                              @else   1 / (Q % P)结果的长度，应该是u16NLen的一半。 @endif */
} uapi_drv_cipher_pke_rsa_priv_key_t;

/**
 * @if Eng
 * @brief  RSA public key structure.
 * @else
 * @brief RSA公钥结构体。
 * @endif
 */
typedef struct {
    uint8_t  *n;            /*!< @if Eng private exponent.
                                 @else   RSA私钥参数d。 @endif */
    uint8_t  *e;            /*!< @if Eng public exponent.
                                 @else   RSA公钥参数e。 @endif */
    uint16_t len;           /*!< @if Eng RSA public key length.
                                 @else   RSA公钥长度。 @endif */
} uapi_drv_cipher_pke_rsa_pub_key_t;
























/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
