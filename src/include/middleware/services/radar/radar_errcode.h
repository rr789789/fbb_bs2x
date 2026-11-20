/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2025-2025. All rights reserved.
 * Description: header file of radar errcode
 */


/**
 * @defgroup  middleware_service_radar_errcode Radar Errcode
 * @ingroup middleware_service_radar
 * @{
 */

#ifndef RADAR_ERRCODE_H
#define RADAR_ERRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  radar error code base.
 * @else
 * @brief  雷达错误码起始。
 * @endif
 */
#define ERRCODE_RADAR_BASE 0x8000A000

/**
 * @if Eng
 * @brief  radar error code base.
 * @else
 * @brief  雷达错误码结束。
 * @endif
 */
#define ERRCODE_RADAR_END 0x8000A0FF

/**
 * @if Eng
 * @brief  radar client error code base.
 * @else
 * @brief  雷达client错误码起始。
 * @endif
 */
#define ERRCODE_RADAR_CLIENT_BASE 0x8000A100

/**
 * @if Eng
 * @brief  radar client error code base.
 * @else
 * @brief  雷达client UAPI 错误码起始。
 * @endif
 */
#define ERRCODE_RADAR_CLIENT_UAPI_BASE 0x8000A11e

/**
 * @if Eng
 * @brief  radar Client error code base.
 * @else
 * @brief  雷达client 软件流程错误码起始。
 * @endif
 */
#define ERRCODE_RADAR_CLIENT_SW_BASE 0x8000A183

/**
 * @if Eng
 * @brief  radar Client error code base.
 * @else
 * @brief  雷达client 硬件错误码起始。
 * @endif
 */
#define ERRCODE_RADAR_CLIENT_HW_BASE 0x8000A1C8

/**
 * @if Eng
 * @brief  radar Client error code base.
 * @else
 * @brief  雷达client错误码结束。
 * @endif
 */
#define ERRCODE_RADAR_CLIENT_END 0x8000A1FF

/**
 * @if Eng
 * @brief  radar client error code. If errcode in [0x80009600,0x800097FF], please check @ref ErrcodeSlpClient
 * @else
 * @brief  雷达client错误码。如果错误码在0x80009600~0x800097FF范围内，请查阅 @ref ErrcodeSlpClient
 * @endif
 */
typedef enum {
    ERRCODE_RC_SUCCESS = 0,                                 /*!< @if Eng error code of success
                                                                @else   执行成功错误码 @endif */
    ERRCODE_RC_MALLOC_FAILED = ERRCODE_RADAR_CLIENT_BASE,   /*!< @if Eng error code of malloc failed
                                                                @else   0x8000A100: 申请动态内存失败错误码 @endif */
    ERRCODE_RC_QUEUE_WRITE_FAILED,                          /*!< @if Eng error code of message queue write failed
                                                                @else   0x8000A101: 消息队列写入失败错误码 @endif */
    ERRCODE_RC_POWER_ON_FAILED,                             /*!< @if Eng error code of power on failed
                                                                @else   0x8000A102: 上电加载失败错误码 @endif */
    ERRCODE_RC_POWERED_ON,                                  /*!< @if Eng error code of is powerd on
                                                                @else   0x8000A103: 已经上电加载错误码 @endif */
    ERRCODE_RC_POWERED_OFF,                                 /*!< @if Eng error code of is powerd off
                                                                @else   0x8000A104: 已经下电错误码 @endif */
    ERRCODE_RC_RADAR_ENABLED,                               /*!< @if Eng error code of SLP radar enabled
                                                                @else   0x8000A105: 雷达已经使能错误码 @endif */
    ERRCODE_RC_RADAR_DISABLED,                              /*!< @if Eng error code of SLP disabled
                                                                @else   0x8000A106: 雷达已经关闭错误码 @endif */
    ERRCODE_RC_PTR_NULL,                                    /*!< @if Eng error code of ptr is NULL
                                                                @else   0x8000A107: 雷达指针为空 @endif */
    ERRCODE_RC_PARA_INVALID,                                /*!< @if Eng error code of para is invalid
                                                                @else   0x8000A108: 雷达参数不合规 @endif */
    ERRCODE_RC_POWERING_ON,                                 /*!< @if Eng error code of is powerd on
                                                                @else   0x8000A109: 正在上电加载错误码 @endif */
    ERRCODE_RC_POWERING_OFF,                                /*!< @if Eng error code of is powerd on
                                                                @else   0x8000A110: 正在下电加载错误码 @endif */
    ERRCODE_RC_STATUS_ERROR = ERRCODE_RADAR_CLIENT_UAPI_BASE, /*!< @if Eng Startup parameter exception
                                                                @else   0x8000A11E: 启动参数异常 @endif */
    ERRCODE_RC_CALL_BACK,                                   /*!< @if Eng Callback registration exception
                                                                @else   0x8000A11F: 回调注册异常 @endif */
    ERRCODE_RC_CHANNEL_IDX,                                 /*!< @if Eng Channel index
                                                                @else   0x8000A120: 信道索引 @endif */
    ERRCODE_RC_GPIO_PINMUX,                                 /*!< @if Eng Switch GPIO PINMUX configuration
                                                                @else   0x8000A121: 开关GPIO PINMUX配置 @endif */
    ERRCODE_RC_ANT_CHANNEL_NUM,                             /*!< @if Eng Antenna pair effective value controllable
                                                                @else   0x8000A122: 天线对有效值可控 @endif */
    ERRCODE_RC_ANT_CODE,                                    /*!< @if Eng Antenna code word configurable
                                                                @else   0x8000A123: 天线码字可配 @endif */
    ERRCODE_RC_TX_POWER,                                    /*!< @if Eng Transmission power
                                                                @else   0x8000A124: 发射功率 @endif */
    ERRCODE_RC_AGC_LNA,                                     /*!< @if Eng AGC gain configuration LNA
                                                                @else   0x8000A125: AGC增益配置LNA @endif */
    ERRCODE_RC_AGC_VGA,                                     /*!< @if Eng AGC gain configuration VGA
                                                                @else   0x8000A126: AGC增益配置VGA @endif */
    ERRCODE_RC_ANT_SW_CTRL,                                 /*!< @if Eng RF switch control pin
                                                                @else   0x8000A127: 射频开关控制管脚 @endif */
    ERRCODE_RC_SUBFRAME_PERIOD,                             /*!< @if Eng Subframe interval
                                                                @else   0x8000A128: 子帧间隔 @endif */
    ERRCODE_RC_ANT_SWITCH_INTERVAL,                         /*!< @if Eng Radar antenna pair switching interval
                                                                @else   0x8000A129: 雷达天线对切换间隔 @endif */
    ERRCODE_RC_START_BIN,                                   /*!< @if Eng Radar raw data parameter: start bin index
                                                                @else   0x8000A12A: 雷达原始数据参数：起始bin索引 @endif */
    ERRCODE_RC_END_BIN,                                     /*!< @if Eng Radar raw data parameter: start end index
                                                                @else   0x8000A12B: 雷达原始数据参数：起始end索引 @endif */
    ERRCODE_RC_TX_WAVE_SPREAD_FACTOR,                       /*!< @if Eng Radar waveform parameter: waveform sequence spreading factor
                                                                @else   0x8000A12C: 雷达波形参数：波形序列扩频因子 @endif */
    ERRCODE_RC_TX_WAVE_ACC_RSHIFT_BIT,                      /*!< @if Eng Radar waveform parameter: accumulation bit width protection right shift bits
                                                                @else   0x8000A12D: 雷达波形参数：累加位宽保护右移位数 @endif */
    ERRCODE_RC_TX_WAVE_ACC_NUM,                             /*!< @if Eng Radar waveform parameter: accumulation times
                                                                @else   0x8000A12E: 雷达波形参数：累加次数 @endif */
    ERRCODE_RC_TX_WAVE_CORR_DIV,                            /*!< @if Eng Radar waveform parameter: correlation divisor factor
                                                                @else   0x8000A12F: 雷达波形参数：相关后除数因子 @endif */
    ERRCODE_RC_TX_WAVE_TX_WAVE_LEN,                         /*!< @if Eng Radar waveform parameter: transmission waveform length
                                                                @else   0x8000A130: 雷达波形参数：发射波形长度 @endif */
    ERRCODE_RC_TX_WAVE_TX_WAVE_BITS,                        /*!< @if Eng Radar waveform parameter: ternary code sequence before waveform spreading
                                                                @else   0x8000A131: 雷达波形参数：波形扩频前的三元码序列 @endif */
    ERRCODE_RC_RADAR_VERSION_NOT_INIT,                      /*!< @if Eng Radar version not initialized
                                                                @else   0x8000A132: 雷达版本未初始化 @endif */
    ERRCODE_RC_NV_WRITE_FAILED,                             /*!< @if Eng Radar nv write failed
                                                                @else   0x8000A133: 雷达写NV失败 @endif */
    ERRCODE_RC_NV_READ_FAILED,                              /*!< @if Eng Radar nv read failed
                                                                @else   0x8000A134: 雷达读NV失败 @endif */
    ERRCODE_RC_START_UP = ERRCODE_RADAR_CLIENT_SW_BASE,     /*!< @if Eng Startup exception
                                                                @else   0x8000A183: 启动异常 @endif */
    ERRCODE_RC_CLOSE,                                       /*!< @if Eng Shutdown exception
                                                                @else   0x8000A184: 关闭异常 @endif */
    ERRCODE_RC_FRAME_BUFFER_FULL,                           /*!< @if Eng Report data buffer full, buffer full, frame loss
                                                                @else   0x8000A185: 上报数据缓冲满，bufferfull，丢帧 @endif */
    ERRCODE_RC_RAINGING_DURATION,                           /*!< @if Eng Coexistence pointing interval exception
                                                                @else   0x8000A186: 共存指向间隔异常 @endif */
    ERRCODE_RC_TX_AHEAD_READ_ADC,                           /*!< @if Eng Radar TX transmission before reading ADC data
                                                                @else   0x8000A187: 雷达TX发送在读取ADC数据前 @endif */
    ERRCODE_RC_AGC_CALI_FAILED,                             /*!< @if Eng AGC cali failed
                                                                @else   0x8000A188: AGC校准异常 @endif */
    ERRCODE_RC_PC_PEAK_OVERFLOWED,                          /*!< @if Eng radar pulse compression peak value overflowed
                                                                @else   0x8000A189: 雷达脉压峰值溢出 @endif */
    ERRCODE_RC_EEPROM_READ_FAILED,                          /*!< @if Eng radar read cali res from eeprom failed
                                                                @else   0x8000A18A: 雷达从eeprom中读取校准结果时发生错误 @endif */
    ERRCODE_RC_IMAGE_CHECK_FAILED,                          /*!< @if Eng radar firmware image check failed
                                                                @else   0x8000A18B: 雷达固件镜像校验错误 @endif */
    ERRCODE_RC_MAX = ERRCODE_RADAR_CLIENT_END,              /*!< @if Eng maximum of error code
                                                                @else   0x8000A1FF: RC错误码最大值 @endif */
} errcode_radar_client_t;

#ifdef __cplusplus
}
#endif
#endif

/**
 * @}
 */