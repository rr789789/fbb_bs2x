/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: nfc hal T5T tag operations \n
 *
 * Date: 2022-11-15 \n
 */
/** @defgroup middleware_service_nfc_controller_hal_t5t NFC HAL T5T
 * @ingroup middleware_service_nfc_controller
 * @{
 */
#ifndef NFC_HAL_T5T_H
#define NFC_HAL_T5T_H

#include <stdbool.h>
#include "nfc_error_code.h"
#include "nfc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Maximum number of bytes that can be stored in each block of T5T card.
 * @else
 * @brief  T5T卡每个块能存储的最大字节数。
 * @endif
 */
#define T5T_MAX_BLOCK_LEN           32

/**
 * @if Eng
 * @brief  Maximum number of bytes that can be read at one time from a T5T card.
 * @else
 * @brief  T5T卡单次读取的最大字节数。
 * @endif
 */
#define T5T_MAX_READ_LEN            254

/**
 * @if Eng
 * @brief  If the magic number is set to this value, the 1-byte address mode and extended commands are supported.
 * @else
 * @brief  魔法数字为此值时表示支持1字节地址模式及扩展命令。
 * @endif
 */
#define T5T_1BYTE_ADDR_MODE_SUPPORT 0xE1

/**
 * @if Eng
 * @brief  If the magic number is set to this value, the 2-byte address mode and extended commands are supported.
 * @else
 * @brief  魔法数字为此值时表示支持2字节地址模式及扩展命令。
 * @endif
 */
#define T5T_2BYTE_ADDR_MODE_SUPPORT 0xE2

/**
 * @if Eng
 * @brief  NFC HAL T5T request flag structure.
 * @else
 * @brief  NFC硬件抽象层T5T请求命令标志位结构体。
 * @endif
*/
typedef struct {
    /** @if Eng  T5T OPTION FLAG (OF). For read-alike cmd, it indicates whether read security status. \n
     *           For write-alike cmd, it indicates whether use special frame.
     *  @else    T5T OPTION FLAG (OF)标志位。对于读类命令，该标志位决定是否读取安全状态。 \n
     *           对于写类命令，决定是否使用特殊帧格式。
     *  @endif */
    bool isOptionFlag;
    /** @if Eng  Address Mode Selector. It indicates whether commands have UID. \n
     *           For select command & sleep V command, isAms should be true.
     *  @else    地址模式标志。决定命令中是否包含UID。 \n
     *           对于选定和睡眠命令，该标志必须为true。
     *  @endif */
    bool isAms;
    /** @if Eng  Selected Mode Selector. It indicates whether to work in selected mode.
     *  @else    选择模式标志。决定是否工作在选择模式。
     *  @endif */
    bool isSms;
} NfcHalT5TReqFlag;

/**
 * @if Eng
 * @brief  Nfc Hal T5T RW information structure. This structure contains read/write information used for framing. \n
 *         These params should be set valid according to different command type before used.
 * @else
 * @brief  NFC硬件抽象层T5T读写信息结构体。这个结构体包含用于组帧的读写信息。于使用前，对于不同命令，这些信息应当被正确设置。
 * @endif
*/
typedef struct {
    /** @if Eng  NFC HAL T5T request flag structure. For details, see @ref NfcHalT5TReqFlag.
     *  @else    NFC硬件抽象层T5T请求命令标志位结构体。参考 @ref NfcHalT5TReqFlag 。
     *  @endif */
    NfcHalT5TReqFlag reqFlag;
    /** @if Eng  UID array. This is optional based on isAms in @ref NfcHalT5TReqFlag. For length, see @ref UID_LEN.
     *  @else    UID的存储数组。UID是可选的，根据 @ref NfcHalT5TReqFlag 中的isAms来定。长度参考 @ref UID_LEN 。
     *  @endif */
    uint8_t uid[UID_LEN];
    /** @if Eng  Number of bytes stored in each block. For maximum length, see @ref T5T_MAX_BLOCK_LEN.
     *  @else    每个块存储的字节数。最大长度参考 @ref T5T_MAX_BLOCK_LEN 。
     *  @endif */
    uint8_t blockLen;
    /** @if Eng  Block index number of the block to be read/write.
     *  @else    需要读写的块的编号。
     *  @endif */
    uint16_t blockNo;
} NfcHalT5TRWInfo;

/**
 * @if Eng
 * @brief  Nfc Hal T5T read response structure.
 * @else
 * @brief  NFC硬件抽象层T5T读命令所收到回复内容结构体。
 * @endif
*/
typedef struct {
    /** @if Eng  Data buffer used to receive data.
     *  @else    用于接收数据的存储空间。
     *  @endif */
    uint8_t *data;
    /** @if Eng  Data buffer size.
     *  @else    用于接收数据的存储空间的总大小。
     *  @endif */
    uint16_t dataBuffSize;
    /** @if Eng  Length of received data.
     *  @else    所接收的数据大小。
     *  @endif */
    uint16_t dataLen;
    /** @if Eng  Status buffer used to receive response status.
     *  @else    用于接收回复的状态信息的存储空间。
     *  @endif */
    uint8_t *status;
    /** @if Eng  Status buffer size.
     *  @else    用于接收回复的状态信息的存储空间的总大小。
     *  @endif */
    uint16_t statusBuffSize;
    /** @if Eng  Received byte length of status. \n
     *           If isOptionFlag in @ref NfcHalT5TReqFlag is set to true, it should be equal to the number of blocks \n
     *           to be read because every block to be read has one byte of status information. \n
     *           If isOptionFlag in @ref NfcHalT5TReqFlag is set to false, it should be 0.
     *  @else    所接收的状态信息大小。 \n
     *           如果 @ref NfcHalT5TReqFlag 中的isOptionFlag 值为true，由于每个要读取的块对应一个字节的状态值，所以该值等于要读取的块数。 \n
     *           如果 @ref NfcHalT5TReqFlag 中的isOptionFlag 值为false，该值为0。
     *  @endif */
    uint16_t statusLen;
} NfcHalT5TReadRes;











#ifdef __cplusplus
}
#endif

#endif
/**
 * @}
 */
