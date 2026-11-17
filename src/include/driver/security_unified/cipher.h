/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved. \n
 *
 * Description: Provides cipher driver header \n
 * Author:  \n
 * History: \n
 * 2023-03-14, Create file. \n
 */
#ifndef CIPHER_H
#define CIPHER_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @defgroup security_unified_cipher Cipher
 * @ingroup  drivers_driver_security_unified
 * @{
 */

/**
 * @if Eng
 * @brief Symmetric encryption channel type
 * @else
 * @brief 对称加密通道类型
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_TYPE_NORMAL = 0x0,
    UAPI_DRV_CIPHER_SYMC_TYPE_REG,
    UAPI_DRV_CIPHER_SYMC_TYPE_MAX,
    UAPI_DRV_CIPHER_SYMC_TYPE_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_type_t;

/**
 * @if Eng
 * @brief Symmetrical encryption algorithm type
 * @note  TDES is not secure, and we advise not to use it.
 * @else
 * @brief 对称加密算法类型
 * @note  TDES不安全，不建议使用
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_ALG_TDES = 0x0,
    UAPI_DRV_CIPHER_SYMC_ALG_AES = 0x1,
    UAPI_DRV_CIPHER_SYMC_ALG_SM4 = 0x2,
    UAPI_DRV_CIPHER_SYMC_ALG_LEA = 0x3,
    UAPI_DRV_CIPHER_SYMC_ALG_DMA = 0x4,
    UAPI_DRV_CIPHER_SYMC_ALG_MAX,
    UAPI_DRV_CIPHER_SYMC_ALG_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_alg_t;

/**
 * @if Eng
 * @brief Symmetric encryption algorithm worke mode
 * @note  ECB is not secure, and we advise not to use it.
 * @else
 * @brief 对称加密算法工作模式
 * @note  ECB模式不安全，不建议使用
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_ECB = 0x0,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_CBC,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_CTR,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_OFB,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_CFB,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_CCM,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_GCM,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_CBC_MAC,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_CMAC,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_MAX,
    UAPI_DRV_CIPHER_SYMC_WORK_MODE_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_work_mode_t;

/**
 * @if Eng
 * @brief Symmetric encryption key length
 * @else
 * @brief 对称加密密钥长度
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_KEY_64BIT =  0x0,
    UAPI_DRV_CIPHER_SYMC_KEY_128BIT = 0x1,
    UAPI_DRV_CIPHER_SYMC_KEY_192BIT = 0x2,
    UAPI_DRV_CIPHER_SYMC_KEY_256BIT = 0x3,
    UAPI_DRV_CIPHER_SYMC_KEY_LENGTH_MAX,
    UAPI_DRV_CIPHER_SYMC_KEY_LENGTH_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_key_length_t;

/**
 * @if Eng
 * @brief Symmetric encryption key parity
 * @else
 * @brief 对称算法密钥奇偶性
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_KEY_EVEN =  0x0,
    UAPI_DRV_CIPHER_SYMC_KEY_ODD  =  0x1,
    UAPI_DRV_CIPHER_SYMC_KEY_PARITY_MAX,
    UAPI_DRV_CIPHER_SYMC_KEY_PARITY_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_key_parity_t;

/**
 * @if Eng
 * @brief Symmetric encryption key bit width
 * @else
 * @brief 对称算法密钥位宽
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_BIT_WIDTH_1BIT = 0x0,
    UAPI_DRV_CIPHER_SYMC_BIT_WIDTH_8BIT = 0x1,
    UAPI_DRV_CIPHER_SYMC_BIT_WIDTH_64BIT = 0x2,
    UAPI_DRV_CIPHER_SYMC_BIT_WIDTH_128BIT = 0x3,
    UAPI_DRV_CIPHER_SYMC_BIT_WIDTH_MAX,
    UAPI_DRV_CIPHER_SYMC_BIT_WIDTH_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_bit_width_t;

/**
 * @if Eng
 * @brief Symmetric encryption iv change flag
 * @else
 * @brief 对称算法初始值标志
 * @endif
 */
typedef enum {
    UAPI_DRV_CIPHER_SYMC_IV_DO_NOT_CHANGE = 0,
    UAPI_DRV_CIPHER_SYMC_IV_CHANGE_ONE_PKG,
    UAPI_DRV_CIPHER_SYMC_IV_CHANGE_ALL_PKG,
    /* GCM. */
    UAPI_DRV_CIPHER_SYMC_GCM_IV_DO_NOT_CHANGE,
    UAPI_DRV_CIPHER_SYMC_GCM_IV_CHANGE_START,
    UAPI_DRV_CIPHER_SYMC_GCM_IV_CHANGE_UPDATE,
    UAPI_DRV_CIPHER_SYMC_GCM_IV_CHANGE_FINISH,
    /* CCM. */
    UAPI_DRV_CIPHER_SYMC_CCM_IV_DO_NOT_CHANGE,
    UAPI_DRV_CIPHER_SYMC_CCM_IV_CHANGE_START,
    UAPI_DRV_CIPHER_SYMC_CCM_IV_CHANGE_UPDATE,
    UAPI_DRV_CIPHER_SYMC_CCM_IV_CHANGE_FINISH,
    UAPI_DRV_CIPHER_SYMC_IV_CHANGE_MAX,
    UAPI_DRV_CIPHER_SYMC_IV_CHANGE_INVALID = 0xffffffff,
} uapi_drv_cipher_symc_iv_change_type_t;

/**
 * @if Eng
 * @brief  Security attribute selection for the buffer
 * @else
 * @brief  缓冲区的安全属性选择
 * @endif
 */
typedef enum uapi_drv_cipher_buffer_secure {
    UAPI_DRV_CIPHER_BUF_NONSECURE,
    UAPI_DRV_CIPHER_BUF_SECURE,
} uapi_drv_cipher_buffer_secure_t;

/**
 * @if Eng
 * @brief Hash algorithm type
 * @note  SHA1 and SHA224 are not secure, and we advise not to use them.
 * @else
 * @brief hash算法类型
 * @note  SHA1和SHA224不安全，不建议使用
 * @endif
 */
typedef enum uapi_drv_hash_type {
    UAPI_DRV_CIPHER_HASH_TYPE_SHA1 = 0xf690a0,
    UAPI_DRV_CIPHER_HASH_TYPE_SHA224 = 0x10690e0,
    UAPI_DRV_CIPHER_HASH_TYPE_SHA256 = 0x1169100,
    UAPI_DRV_CIPHER_HASH_TYPE_SHA384 = 0x127a180,
    UAPI_DRV_CIPHER_HASH_TYPE_SHA512 = 0x137a200,
    UAPI_DRV_CIPHER_HASH_TYPE_SM3 = 0x2169100,

    UAPI_DRV_CIPHER_HASH_TYPE_HMAC_SHA1 = 0x10f690a0,
    UAPI_DRV_CIPHER_HASH_TYPE_HMAC_SHA224 = 0x110690e0,
    UAPI_DRV_CIPHER_HASH_TYPE_HMAC_SHA256 = 0x11169100,
    UAPI_DRV_CIPHER_HASH_TYPE_HMAC_SHA384 = 0x1127a180,
    UAPI_DRV_CIPHER_HASH_TYPE_HMAC_SHA512 = 0x1137a200,
    UAPI_DRV_CIPHER_HASH_TYPE_HMAC_SM3 = 0x12169100,

    UAPI_DRV_CIPHER_HASH_TYPE_INVALID = 0xffffffff,
} uapi_drv_cipher_hash_type_t;

/**
 * @if Eng
 * @brief  Attribute structure of the buffer
 * @else
 * @brief  缓冲区的属性结构体
 * @endif
 */
typedef struct uapi_drv_cipher_buf_attr {
    uint64_t uapi_mem_handle;       /*!< @if Eng This parameter is not used.
                                         @else   该参数未使用。 @endif */
    uint64_t addr_offset;           /*!< @if Eng This parameter is not used.
                                         @else   该参数未使用。 @endif */
    void *kapi_mem_handle;          /*!< @if Eng This parameter is not used.
                                         @else   该参数未使用。 @endif */
    uintptr_t phys_addr;            /*!< @if Eng Physical address of the buffer.
                                         @else   缓冲区的物理地址。 @endif */
    void *virt_addr;                /*!< @if Eng Indicates the CPU address of the buffer.
                                         @else   缓冲区的cpu地址。 @endif */
    uapi_drv_cipher_buffer_secure_t buf_sec; /*!< @if Eng Buffer security attribute.
                                                  @else   缓冲区安全属性。 @endif */
} uapi_drv_cipher_buf_attr_t;

/**
 * @if Eng
 * @brief  Symmetric encryption parameter attribute structure
 * @else
 * @brief  对称加密参数属性结构体
 * @endif
 */
typedef struct {
    uapi_drv_cipher_symc_alg_t symc_alg;        /*!< @if Eng Symmetric encryption algorithm.
                                                     @else   对称加密算法。 @endif */
    uapi_drv_cipher_symc_work_mode_t work_mode; /*!< @if Eng Symmetric encryption word mode.
                                                     @else   对称加密算法工作模式。 @endif */
    uapi_drv_cipher_symc_type_t symc_type;      /*!< @if Eng Symmetric encryption channel type.
                                                     @else   对称加密通道类型。 @endif */
    bool is_long_term;                          /*!< @if Eng Long or short term channel occupancy flag.
                                                     @else   长短期通道占用标志。 @endif */
} uapi_drv_cipher_symc_attr_t;

/**
 * @if Eng
 * @brief  Parameter structure of CCM and GCM
 * @else
 * @brief  CCM和GCM工作模式的参数结构体
 * @endif
 */
typedef struct {
    uapi_drv_cipher_buf_attr_t aad_buf;    /*!< @if Attribute structure of the buffer.
                                                @else 缓冲区的属性结构体。 @endif */
    uint32_t aad_len;                      /*!< @if Addition data length in bytes.
                                                @else 附加信息的字节长度。 @endif */
    uint32_t data_len;                     /*!< @if Crypto data length in bytes.
                                                @else 加密数据的字节长度。 @endif */
    uint32_t tag_len;                      /*!< @if Tag length in bytes.
                                                @else 标签的字节长度。 @endif */
} uapi_drv_cipher_symc_config_aes_ccm_gcm_t;

#define UAPI_DRV_CIPHER_IV_LEN_IN_BYTES 16

/**
 * @if Eng
 * @brief  Parameters of symmetric encryption algorithm
 * @else
 * @brief  对称加密算法的参数
 * @endif
 */
typedef struct {
    uapi_drv_cipher_symc_alg_t symc_alg;                    /*!< @if Eng Symmetric encryption algorithm.
                                                                 @else   对称加密算法。 @endif */
    uapi_drv_cipher_symc_work_mode_t work_mode;             /*!< @if Eng Symmetric encryption word mode.
                                                                 @else   对称加密算法工作模式。 @endif */
    uapi_drv_cipher_symc_key_length_t symc_key_length;      /*!< @if Eng Symmetric encryption key length.
                                                                 @else   对称加密密钥长度。 @endif */
    uapi_drv_cipher_symc_key_parity_t key_parity;           /*!< @if Eng Symmetric encryption key parity.
                                                                 @else   对称算法密钥奇偶性。 @endif */
    uapi_drv_cipher_symc_bit_width_t symc_bit_width;        /*!< @if Eng Symmetric encryption key bit width.
                                                                 @else   对称算法密钥位宽。 @endif */
    uapi_drv_cipher_symc_iv_change_type_t iv_change_flag;   /*!< @if Eng Symmetric encryption iv change flag.
                                                                 @else   对称算法密钥位宽。 @endif */
    uint8_t iv[UAPI_DRV_CIPHER_IV_LEN_IN_BYTES];            /*!< @if Eng Symmetric encryption iv.
                                                                 @else   对称加密算法初始值。 @endif */
    uint32_t iv_length;                                     /*!< @if Eng Symmetric encryption iv length.
                                                                 @else   对称加密算法初始值长度。 @endif */
    void *param;                                            /*!< @if Eng Symmetric encryption ccm and gam parameter.
                                                                 @else   对称加密CCM和GCM工作模式的参数。 @endif */
} uapi_drv_cipher_symc_ctrl_t;

/**
 * @if Eng
 * @brief  Control parameters of symmetric encryption algorithm
 * @else
 * @brief  Attribute structure of cbc_mac and cmac
 * @endif
 */
typedef struct {
    bool is_long_term;                                  /*!< @if Eng Long or short term channel occupancy flag.
                                                             @else   长短期通道占用标志。 @endif */
    uapi_drv_cipher_symc_alg_t symc_alg;                /*!< @if Eng Symmetric encryption algorithm.
                                                             @else   对称加密算法。 @endif */
    uapi_drv_cipher_symc_work_mode_t work_mode;         /*!< @if Eng Symmetric encryption word mode.
                                                             @else   对称加密算法工作模式。 @endif */
    uapi_drv_cipher_symc_key_length_t symc_key_length;  /*!< @if Eng Symmetric encryption key length.
                                                             @else   对称加密密钥长度。 @endif */
    uint32_t keyslot_chn;                               /*!< @if Eng Keyslot channel id.
                                                             @else   Key槽ID。 @endif */
} uapi_drv_cipher_symc_mac_attr_t;

/**
 * @if Eng
 * @brief Hash channel and algorithm parameter structure
 * @else
 * @brief hash通道及算法参数结构体
 * @endif
 */
typedef struct uapi_drv_hash_attr {
    uint8_t *key;               /*!< @if Eng Pointer to the key used by the HMAC algorithm,
                                        This parameter is valid only when the HMAC algorithm
                                        is used and is_keyslot is set to false.
                                     @else   指向hmac算法使用密钥的指针，
                                        当使用hmac算法且is_keyslot为false时该参数生效 @endif */
    uint32_t key_len;           /*!< @if Eng Key length, in bytes.
                                        When the algorithm is HMAC-SHA1, SHA224, SHA256, or SM3,
                                        the value cannot exceed 64 bytes.
                                        When the algorithm is HMAC-SHA384 or SHA512, the value cannot exceed 128 bytes.
                                     @else   密钥长度，单位为字节。当算法为HMAC-SHA1/SHA224/SHA256/SM3时，不超过64字节；
                                        当算法为HMAC-SHA384/SHA512时，不超过128字节。 @endif */
    uint32_t keyslot_handle;    /*!< @if Eng Keyslot channel handle used by the HMAC algorithm.
                                            This parameter is valid only when is_keyslot is set to true.
                                     @else   hmac算法使用的keyslot通道句柄，当is_keyslot为true时该参数生效。 @endif */
    uapi_drv_cipher_hash_type_t hash_type;  /*!< @if Eng Indicates the hash algorithm type.
                                                    For details, see @ref uapi_drv_cipher_hash_type_t
                                                 @else   hash算法类型。参考 @ref uapi_drv_cipher_hash_type_t @endif */
    bool is_keyslot;    /*!< @if Eng Indicates whether the HMAC algorithm uses keyslot.
                             @else   指示hmac算法是否使用keyslot 。 @endif */
    bool is_long_term;  /*!< @if Eng Indicates whether the created channel is a long-term or short-term channel.
                                A maximum of two long-term channels and eight short-term channels can be created.
                             @else   指示创建的通道为长期通道还是短期通道。最大支持创建2个长期通道和8个短期通道。 @endif */
} uapi_drv_cipher_hash_attr_t;

#define UAPI_DRV_CIPHER_HASH_RESULT_SIZE_MAX_IN_WORD 16      // for SHA-512
#define UAPI_DRV_CIPHER_HASH_BLOCK_SIZE_MAX 128              // for SHA-512

/**
 * @if Eng
 * @brief Structure of the intermediate hash calculation result
 * @note  This structure does not need to be constructed by users.
            It is obtained by calling the uapi_drv_cipher_hash_get interface and
            set by calling the uapi_drv_cipher_hash_set interface.
 * @else
 * @brief hash计算中间结果结构体
 * @note  该结构体的内容无需用户构造，通过uapi_drv_cipher_hash_get接口获取，并通过uapi_drv_cipher_hash_set接口设置。
 * @endif
 */
typedef struct uapi_drv_cipher_hash_clone_ctx {
    uint32_t length[2]; /*!< @if Eng Indicates the length of the currently processed data, in bits.
                             @else   保存当前已处理的数据长度，单位是位。 @endif */
    uint32_t state[UAPI_DRV_CIPHER_HASH_RESULT_SIZE_MAX_IN_WORD];  /*!< @if Eng Saving calculation intermediate results.
                                                                 @else   保存计算中间结果。 @endif */
    uint32_t tail_len;  /*!< @if Eng Length of the remaining unaligned tail data since the last calculation.
                             @else   上次计算后剩余未对齐的尾部数据长度。 @endif */
    uapi_drv_cipher_hash_type_t hash_type;  /*!< @if Eng Algorithm used for hash calculation.
                                                 @else   hash计算使用的算法。 @endif */
    uint8_t o_key_pad[UAPI_DRV_CIPHER_HASH_BLOCK_SIZE_MAX];  /*!< @if Eng Save o_key_pad,
                                                                    which is used by the HMAC algorithm.
                                                           @else   保存o_key_pad，hmac 算法使用。 @endif */
    uint8_t i_key_pad[UAPI_DRV_CIPHER_HASH_BLOCK_SIZE_MAX];  /*!< @if Eng Save i_key_pad,
                                                                    which is used by the HMAC algorithm.
                                                           @else   保存i_key_pad，hmac 算法使用。 @endif */
    uint8_t tail[UAPI_DRV_CIPHER_HASH_BLOCK_SIZE_MAX]; /*!< @if Eng Unaligned tail data remaining
                                                                    since last calculation.
                                                     @else   上次计算后剩余未对齐的尾部数据。 @endif */
} uapi_drv_cipher_hash_clone_ctx_t;

/**
 * @if Eng
 * @brief Parameter structure of PBKDF2
 * @else
 * @brief PBKDF2算法的参数结构体
 * @endif
 */
typedef struct {
    uapi_drv_cipher_hash_type_t hash_type;  /*!< @if The type of hash algorithm used by PBKDF2.
                                                 @else   PBKDF2使用的hash算法类型。 @endif */
    uint8_t *password;      /*!< @if Password for PBKDF2.
                                 @else   PBKDF2的输入口令。 @endif */
    uint32_t plen;          /*!< @if Password length of PBKDF2.
                                 @else   PBKDF2的输入口令长度。 @endif */
    uint8_t *salt;          /*!< @if Salt of PBKDF2.
                                 @else   PBKDF2的盐值。 @endif */
    uint32_t slen;          /*!< @if Salt of PBKDF2.
                                 @else   PBKDF2的盐值长度。 @endif */
    uint16_t count;         /*!< @if Iterations of PBKDF2.
                                 @else   PBKDF2的迭代次数。 @endif */
} uapi_drv_cipher_kdf_pbkdf2_param_t;

/**
 * @if Eng
 * @brief Parameter structure of key extract
 * @else
 * @brief 密钥提取的参数结构体
 * @endif
 */
typedef struct {
    uapi_drv_cipher_hash_type_t hmac_type;    /*!< @if Type of the HMAC algorithm for extracting keys.
                                                   @else   提取密钥的HMAC算法类型。 @endif */
    uint8_t *salt;    /*!< @if Extract the salt value of the key.
                           @else   提取密钥的盐值。 @endif */
    uint32_t salt_length;    /*!< @if Extract the salt length of the key.
                                  @else   提取密钥的盐值长度。 @endif */
    uint8_t *ikm;           /*!< @if Extract the input key material of the key.
                                 @else   提取密钥的原始密钥材料。 @endif */
    uint32_t ikm_length;    /*!< @if Length of the input key material of the extracted key.
                                 @else   提取密钥的原始密钥材料长度。 @endif */
} uapi_drv_cipher_hkdf_extract_t;

/**
 * @if Eng
 * @brief Parameter structure of key extend
 * @else
 * @brief 密钥拓展的参数结构体
 * @endif
 */
typedef struct {
    uapi_drv_cipher_hash_type_t hmac_type;    /*!< @if HMAC algorithm type of the extended key.
                                                   @else   拓展密钥的HMAC算法类型。 @endif */
    uint8_t *prk;    /*!< @if Pseudo-random key of the extended key.
                          @else   拓展密钥的伪随机密钥。 @endif */
    uint32_t prk_length;    /*!< @if Pseudo-random key length of the extended key.
                                 @else   拓展密钥的伪随机密钥长度。 @endif */
    uint8_t *info;    /*!< @if Information about the extended key.
                           @else   拓展密钥的信息。 @endif */
    uint32_t info_length;   /*!< @if Length of extended key information.
                                 @else   拓展密钥的信息长度。 @endif */
} uapi_drv_cipher_hkdf_expand_t;

/**
 * @if Eng
 * @brief Parameter structure of HKDF
 * @else
 * @brief HKDF参数结构体
 * @endif
 */
typedef struct {
    uapi_drv_cipher_hash_type_t hmac_type;    /*!< @if HMAC algorithm type of the extended key.
                                                   @else   提起、拓展密钥的HMAC算法类型。 @endif */
    uint8_t *salt;    /*!< @if Extract the salt value of the key.
                           @else   提取密钥的盐值。 @endif */
    uint32_t salt_length;    /*!< @if Extract the salt length of the key.
                                  @else   提取密钥的盐值长度。 @endif */
    uint8_t *ikm;    /*!< @if Extract the input key material of the key.
                          @else   提取密钥的原始密钥材料。 @endif */
    uint32_t ikm_length;    /*!< @if Length of the input key material of the extracted key.
                                 @else   提取密钥的原始密钥材料长度。 @endif */
    uint8_t *info;    /*!< @if Information about the extended key.
                           @else   拓展密钥的信息。 @endif */
    uint32_t info_length;    /*!< @if Length of extended key information.
                                  @else   拓展密钥的信息长度。 @endif */
} uapi_drv_cipher_hkdf_t;





/**
 * @if Eng
 * @brief  Set algorithm parameters.
 * @param  [in]  symc_handle The number of the channel to be set.
 * @param  [in]  symc_ctrl Parameters of symmetric encryption algorithm.
                            For details, see @ref uapi_drv_cipher_symc_ctrl_t
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @else
 * @brief  设置算法参数。
 * @param  [in]  symc_handle 要设置的通道号。
 * @param  [in]  symc_ctrl 对称加密算法的参数，参考 @ref uapi_drv_cipher_symc_attr_t  。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败，参考 @ref errcode_t 。
 * @endif
 */
















/**
 * @if Eng
 * @brief  Sets the intermediate result of hash calculation.
 * @note   The hash handle must have been created.
 * @param  [in]    hash_handle Handle of the created hash channel.
 * @param  [in]    hash_clone_ctx Pointer to the intermediate result structure of hash calculation.
                                    For details, see @ref uapi_drv_cipher_hash_clone_ctx_t
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @else
 * @brief  设置HASH计算中间结果。
 * @note   必须已经创建了hash句柄。
 * @param  [in]    hash_handle 已创建的hash通道句柄。
 * @param  [in]   hash_clone_ctx 指向Hash计算中间结果结构体的指针。参考 @ref uapi_drv_cipher_hash_clone_ctx_t 。
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
