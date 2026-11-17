/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2021. All rights reserved.
 * Description: nfc hal core \n
 *
 * Date: 2021-08-20 \n
 */
/** @defgroup middleware_service_nfc_controller_hal_core NFC HAL Core
 * @ingroup middleware_service_nfc_controller
 * @{
 */
#ifndef NFC_HAL_CORE_H
#define NFC_HAL_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "nfc_error_code.h"
#include "nfc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_NFC_LISTEN
/**
 * @if Eng
 * @brief  Callback for upper layer to handle HCI message from NFCEE.
 * @param  [in]     buff  buffer for storing HCI message wrapped in NCI packet.
 * @param  [in]     len  data length in buffer.
 * @retval NFC_OK   Success.
 * @retval Other    Failure. For details, see @ref NfcErrorCode.
 * @else
 * @brief  上层注册的回调函数，用于处理从NFCEE过来的HCI命令或通知。
 * @param  [in]     buff  有NCI包封装的HCI信息的存储地址。
 * @param  [in]     len  封装的总数据字节长度。
 * @retval NFC_OK   成功。
 * @retval Other    失败。参考 @ref NfcErrorCode 。
 * @endif
 */
typedef NfcErrorCode (*FuncProcessHciFromNfcee)(uint8_t *buff, uint16_t len);
#endif

/**
 * @if Eng
 * @brief  NFC HAL core parameters structure.
 * @else
 * @brief  NFC硬件抽象层核心参数结构体。
 * @endif
 */
typedef struct NfcHalCoreParamsStruct {
    /** @if Eng  NCI control packet read operation timeout in millisecond.
     *  @else    NCI控制包读操作超时，单位毫秒。
     *  @endif */
    uint32_t readTimeout;
    /** @if Eng  NCI data packet read operation timeout in millisecond.
     *  @else    NCI数据包读操作超时，单位毫秒。
     *  @endif */
    uint32_t dataTimeout;
#ifdef HAVE_NFC_LISTEN
    /** @if Eng  Callback to handle HCI message from NFCEE. For details, see @ref FuncProcessHciFromNfcee.
     *  @else    上层注册的回调函数，用于处理从NFCEE过来的HCI命令或通知。参考 @ref FuncProcessHciFromNfcee 。
     *  @endif */
    FuncProcessHciFromNfcee processHciFromNfcee;
#endif
} NfcHalCoreParams;

#ifdef HAVE_NFC_POLL
#pragma pack(push, 1)
/**
 * @if Eng
 * @brief  NFC HAL T3T polling notification parameters structure.
 * @else
 * @brief  NFC硬件抽象层T3T轮询通知参数结构体。
 * @endif
 */
typedef struct NfcHalT3tPollNtfParamsStruct {
    /** @if Eng  NFCID2 array. For length, see @ref NFCID2_LEN.
     *  @else    NFCID2的存储数组。长度参考 @ref NFCID2_LEN 。
     *  @endif */
    uint8_t nfcid2[NFCID2_LEN];
    /** @if Eng  PAD0 parameter array. For length, see @ref T3T_PAD0_LEN.
     *  @else    PAD0的存储数组。长度参考 @ref T3T_PAD0_LEN 。
     *  @endif */
    uint8_t pad0[T3T_PAD0_LEN];
    /** @if Eng  PAD1 parameter array. For length, see @ref T3T_PAD1_LEN.
     *  @else    PAD1的存储数组。长度参考 @ref T3T_PAD1_LEN 。
     *  @endif */
    uint8_t pad1[T3T_PAD1_LEN];
    /** @if Eng  MRTI-check parameter.
     *  @else    MRTI-check参数。
     *  @endif */
    uint8_t mrtiCheck;
    /** @if Eng  MRTI-update parameter.
     *  @else    MRTI-update参数。
     *  @endif */
    uint8_t mrtiUpdate;
    /** @if Eng  PAD2 parameter.
     *  @else    PAD2参数。
     *  @endif */
    uint8_t pad2;
    /** @if Eng  RD parameter array. For length, see @ref T3T_RD_LEN.
     *  @else    RD的存储数组。长度参考 @ref T3T_RD_LEN 。
     *  @endif */
    uint8_t rd[T3T_RD_LEN];
} NfcHalT3tPollNtfParams;
#pragma pack(pop)

/**
 * @if Eng
 * @brief  NFC HAL T3T sense responses structure.
 * @else
 * @brief  NFC硬件抽象层T3T轮询感应回复参数结构体。
 * @endif
 */
typedef struct NfcHalT3tSensfResStruct {
    /** @if Eng  T3T polling notification parameters array. For details, see @ref NfcHalT3tPollNtfParams.
     *  @else    T3T轮询感应通知结构体数组。元素结构体参考 @ref NfcHalT3tPollNtfParams 。
     *  @endif */
    NfcHalT3tPollNtfParams resParams[T3T_POLL_SENSF_RES_MAX_SIZE];
    /** @if Eng  The number of received T3T polling notification parameters.
     *  @else    接收到的T3T轮询感应通知个数。
     *  @endif */
    uint8_t resNum;
} NfcHalT3tSensfRes;

/**
 * @if Eng
 * @brief  NFC HAL T3T polling command parameters structure.
 * @else
 * @brief  NFC硬件抽象层T3T轮询命令参数结构体。
 * @endif
 */
typedef struct NfcHalT3tPollCmdParamsStruct {
    /** @if Eng  The system code of T3T polling command.
     *  @else    T3T轮询命令的系统码。
     *  @endif */
    uint16_t sc;
    /** @if Eng  The request code of T3T polling command.
     *  @else    T3T轮询命令的请求码。
     *  @endif */
    uint8_t rc;
    /** @if Eng  The time slot number of T3T polling command.
     *  @else    T3T轮询命令的时隙个数。
     *  @endif */
    uint8_t tsn;
    /** @if Eng  The time in millisecond to wait fot T3T polling notification.
     *  @else    T3T轮询等待响应通知的超时时间，单位毫秒。
     *  @endif */
    uint16_t timeout;
} NfcHalT3tPollCmdParams;
#endif

/**
 * @if Eng
 * @brief  NFC technology A parameters structure.
 * @else
 * @brief  NFC技术A的参数结构体。
 * @endif
 */
typedef struct {
    /** @if Eng  Sense response array. For length, see @ref SENS_RES_LEN.
     *  @else    技术A的轮询感应回复。长度参考 @ref SENS_RES_LEN 。
     *  @endif */
    uint8_t sensRes[SENS_RES_LEN];
    /** @if Eng  The length of NFCID1.
     *  @else    NFCID1的长度。
     *  @endif */
    uint8_t nfcid1Len;
    /** @if Eng  NFCID1 buffer. For buffer max length, see @ref NFCID1_MAX_LEN.
     *  @else    NFCID1的存储空间。最大长度参考 @ref NFCID1_MAX_LEN 。
     *  @endif */
    uint8_t nfcid1[NFCID1_MAX_LEN];
    /** @if Eng  Select response.
     *  @else    技术A的选择回复。
     *  @endif */
    uint8_t selRes;
} NfcTechAParams;

/**
 * @if Eng
 * @brief  NFC technology B parameters structure.
 * @else
 * @brief  NFC技术B的参数结构体。
 * @endif
 */
typedef struct {
    /** @if Eng  The length of senseB response.
     *  @else    技术B的轮询感应回复长度。
     *  @endif */
    uint8_t sensbResLen;
    /** @if Eng  SenseB response buffer. First 4 bytes are NFCID0. For buffer max length, see @ref SENSB_RES_MAX_LEN.
     *  @else    技术B的轮询感应回复长度。头部4个字节是NFCID0。存储最大长度参考 @ref SENSB_RES_MAX_LEN 。
     *  @endif */
    uint8_t sensbRes[SENSB_RES_MAX_LEN];
} NfcTechBParams;

/**
 * @if Eng
 * @brief  NFC technology F parameters structure.
 * @else
 * @brief  NFC技术F的参数结构体。
 * @endif
 */
typedef struct {
    /** @if Eng  Technology F Bit rate.
     *  @else    技术F的速率。
     *  @endif */
    uint8_t bitRate;
    /** @if Eng  The length of senseF response.
     *  @else    技术F的轮询感应回复长度。
     *  @endif */
    uint8_t sensfResLen;
    /** @if Eng  SenseF response buffer. First 8 bytes are NFCID2. For buffer max length, see @ref SENSF_RES_MAX_LEN.
     *  @else    技术F的轮询感应回复长度。头部8个字节是NFCID2。存储最大长度参考 @ref SENSF_RES_MAX_LEN 。
     *  @endif */
    uint8_t sensfRes[SENSF_RES_MAX_LEN];
} NfcTechFParams;

/**
 * @if Eng
 * @brief  NFC technology V parameters structure.
 * @else
 * @brief  NFC技术V的参数结构体。
 * @endif
 */
typedef struct {
    /** @if Eng  Response Flag.
     *  @else    回复的标记字节信息。
     *  @endif */
    uint8_t resFlag;
    /** @if Eng  DSFID parameter.
     *  @else    DSFID参数信息。
     *  @endif */
    uint8_t dsfid;
    /** @if Eng  UID buffer. For buffer length, see @ref UID_LEN.
     *  @else    UID信息。长度参考 @ref UID_LEN 。
     *  @endif */
    uint8_t uid[UID_LEN];
} NfcTechVParams;

/**
 * @if Eng
 * @brief  NFC technology parameters union.
 * @else
 * @brief  NFC技术的参数联合体。
 * @endif
 */
typedef union {
    /** @if Eng  Technology A parameters. For details, see @ref NfcTechAParams.
     *  @else    技术A的参数信息。参考 @ref NfcTechAParams 。
     *  @endif */
    NfcTechAParams a;
    /** @if Eng  Technology B parameters. For details, see @ref NfcTechBParams.
     *  @else    技术B的参数信息。参考 @ref NfcTechBParams 。
     *  @endif */
    NfcTechBParams b;
    /** @if Eng  Technology F parameters. For details, see @ref NfcTechFParams.
     *  @else    技术F的参数信息。参考 @ref NfcTechFParams 。
     *  @endif */
    NfcTechFParams f;
    /** @if Eng  Technology V parameters. For details, see @ref NfcTechVParams.
     *  @else    技术V的参数信息。参考 @ref NfcTechVParams 。
     *  @endif */
    NfcTechVParams v;
} NfcTechParams;

/**
 * @if Eng
 * @brief  NFC interface activation information structure.
 * @else
 * @brief  NFC接口激活信息结构体。
 * @endif
 */
typedef struct {
    /** @if Eng  NCI interface. For details, see @ref NFC_NCI_RF_INTERFACE_ENUM.
     *  @else    NCI接口。参考 @ref NFC_NCI_RF_INTERFACE_ENUM 。
     *  @endif */
    uint8_t interface;
    /** @if Eng  NCI protocol. For details, see @ref NFC_NCI_RF_PROTOCOL_ENUM.
     *  @else    NCI协议。参考 @ref NFC_NCI_RF_PROTOCOL_ENUM 。
     *  @endif */
    uint8_t protocol;
    /** @if Eng  NCI technology mode. For details, see @ref NFC_NCI_RF_TECH_MODE_ENUM.
     *  @else    NCI技术模式。参考 @ref NFC_NCI_RF_TECH_MODE_ENUM 。
     *  @endif */
    uint8_t techMode;
    /** @if Eng  NFC technology parameters union. For details, see @ref NfcTechParams.
     *  @else    NFC技术的参数联合体。参考 @ref NfcTechParams 。
     *  @endif */
    NfcTechParams techParams;
} NfcIntfActInfo;



















#ifdef HAVE_NFC_POLL





#endif // HAVE_NFC_POLL

#ifdef HAVE_NFC_LISTEN




#endif // HAVE_NFC_LISTEN

#ifdef __cplusplus
}
#endif

#endif // NFC_HAL_CORE_H
/**
 * @}
 */
