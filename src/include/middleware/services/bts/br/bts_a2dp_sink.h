/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: BT A2DP-SNK module.
 */

/**
 * @addtogroup bluetooth_bts_a2dp_sink A2DP SNK API
 * @ingroup    bluetooth
 * @{
 */

#ifndef BTS_A2DP_SINK_H
#define BTS_A2DP_SINK_H

#include "bts_def.h"
#include "bts_br_gap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief A2DP SNK codec priority.
 * @else
 * @brief A2DP SNK 编码格式优先级。
 * @endif
 */
#define A2DP_SNK_CODEC_PRIORITY_DISABLED (-1)
#define A2DP_SNK_CODEC_PRIORITY_DEFAULT 0
#define A2DP_SNK_CODEC_PRIORITY_HIGHEST (1000 * 1000)

/**
 * @if Eng
 * @brief Sample rate of A2DP SNK.
 * @else
 * @brief A2DP SNK 采样率。
 * @endif
 */
#define A2DP_SNK_CODEC_SAMPLE_RATE_NONE 0x00
#define A2DP_SNK_CODEC_SAMPLE_RATE_44100 (0x01 << 0)
#define A2DP_SNK_CODEC_SAMPLE_RATE_48000 (0x01 << 1)
#define A2DP_SNK_CODEC_SAMPLE_RATE_88200 (0x01 << 2)
#define A2DP_SNK_CODEC_SAMPLE_RATE_96000 (0x01 << 3)
#define A2DP_SNK_CODEC_SAMPLE_RATE_176400 (0x01 << 4)
#define A2DP_SNK_CODEC_SAMPLE_RATE_192000 (0x01 << 5)

/**
 * @if Eng
 * @brief Sample bit of A2DP SNK sample rate.
 * @else
 * @brief A2DP SNK 的采样比特位。
 * @endif
 */
#define A2DP_SNK_CODEC_BITS_PER_SAMPLE_NONE 0x00
#define A2DP_SNK_CODEC_BITS_PER_SAMPLE_16 (0x01 << 0)
#define A2DP_SNK_CODEC_BITS_PER_SAMPLE_24 (0x01 << 1)
#define A2DP_SNK_CODEC_BITS_PER_SAMPLE_32 (0x01 << 2)

/**
 * @if Eng
 * @brief Channel mode of A2DP SNK NONE.
 * @else
 * @brief A2DP SNK 声道模式NONE。
 * @endif
 */
#define A2DP_SNK_CODEC_CHANNEL_MODE_NONE 0x00

/**
 * @if Eng
 * @brief Channel mode of A2DP SNK MONO.
 * @else
 * @brief A2DP SNK 声道模式MONO。
 * @endif
 */
#define A2DP_SNK_CODEC_CHANNEL_MODE_MONO (0x01 << 0)
/**
 * @if Eng
 * @brief Channel mode of A2DP SNK STEREO.
 * @else
 * @brief A2DP SNK 声道模式STEREO。
 * @endif
 */
#define A2DP_SNK_CODEC_CHANNEL_MODE_STEREO (0x01 << 1)

/**
 * @if Eng
 * @brief Enum of A2DP SNK codec type.
 * @else
 * @brief A2DP SNK 编码格式类型。
 * @endif
 */
typedef enum {
    A2DP_SNK_CODEC_TYPE_SBC = 0x00,        /*!< @if Eng SBC codec type.
                                            @else   编码格式类型为SBC。 @endif */
    A2DP_SNK_CODEC_TYPE_AAC,               /*!< @if Eng AAC codec type.
                                            @else   编码格式类型为AAC。 @endif */
    A2DP_SNK_CODEC_TYPE_APTX,              /*!< @if Eng APTX codec type.
                                            @else   编码格式类型为APTX。 @endif */
    A2DP_SNK_CODEC_TYPE_APTX_HD,           /*!< @if Eng APTX_HD codec type.
                                            @else   编码格式类型为APTX_HD。 @endif */
    A2DP_SNK_CODEC_TYPE_LDAC,              /*!< @if Eng LDAC codec type.
                                            @else   编码格式类型为LDAC。 @endif */
    A2DP_SNK_CODEC_TYPE_LHDC,              /*!< @if Eng LHDC codec type.
                                            @else   编码格式类型为LHDC。 @endif */
    A2DP_SNK_CODEC_TYPE_LC3,               /*!< @if Eng LC3 codec type.
                                            @else   编码格式类型为LC3。 @endif */
    A2DP_SNK_CODEC_TYPE_L2HC,              /*!< @if Eng L2HC codec type.
                                            @else   编码格式类型为L2HC。 @endif */
    A2DP_SNK_CODEC_TYPE_INVALID = 0xF4240, /*!< @if Eng Invalid codec type.
                                            @else   无效编码格式类型。 @endif */
} a2dp_snk_codec_type_t;

/**
 * @if Eng
 * @brief Enum of A2DP SNK optional codec support state define.
 * @else
 * @brief A2DP SNK 配置编码格式支持状态。
 * @endif
 */
typedef enum {
    A2DP_SNK_OPTIONAL_CODEC_NOT_SUPPORT = 0x00, /*!< @if Eng Not support optional codec.
                                                 @else   不支持设置编码格式类型。 @endif */
    A2DP_SNK_OPTIONAL_CODEC_SUPPORT,            /*!< @if Eng Support optional codec.
                                                 @else   支持设置编码格式类型。 @endif */
    A2DP_SNK_OPTIONAL_CODEC_SUPPORT_UNKNOWN,    /*!< @if Eng Support user define codec.
                                                 @else   支持用户定义编码格式类型。 @endif */
} a2dp_snk_optional_codec_support_state_t;

/**
 * @if Eng
 * @brief Enum of A2DP SNK codec configuration information.
 * @else
 * @brief A2DP SNK 配置编码格式参数。
 * @endif
 */
typedef struct {
    int codec_priority;            /*!< @if Eng Codec priority.
                                        @else   编码格式优先级。 @endif */
    unsigned int codec_type;       /*!< @if Eng Codec type.
                                        @else   编码格式类型。 @endif */
    unsigned char sample_rate;     /*!< @if Eng Codec sample rate.
                                        @else   编码器采样率。 @endif */
    unsigned char bits_per_sample; /*!< @if Eng Codec bits per sampl.
                                        @else   采样比特位。 @endif */
    unsigned char channel_mode;    /*!< @if Eng Codec channel mode.
                                        @else   声道模式。 @endif */
    unsigned long codec_specific1; /*!< @if Eng Codec specific value 1.
                                        @else   自定义参数1。 @endif */
    unsigned long codec_specific2; /*!< @if Eng Codec specific value 2.
                                        @else   自定义参数2。 @endif */
    unsigned long codec_specific3; /*!< @if Eng Codec specific value 3.
                                        @else   自定义参数3。 @endif */
    unsigned long codec_specific4; /*!< @if Eng Codec specific value 4.
                                        @else   自定义参数4。 @endif */
} a2dp_snk_codec_info_t;

/**
 * @if Eng
 * @brief  Callback invoked in response to A2DP SNK connection state changed.
 * @par Description:
 *           When registered, the bts uses this callback to deliver A2DP SNK connect stat info updated
 *           after connection state updated.
 * @attention  1. This function is called in bts context, should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr        A poniter of the peer bluetooth device addresss. See @ref bd_addr_t
 * @param  [in]  conn_state     A2DP SNK connection state. See @ref int
 * @retval No return value. See  @ref void
 * @else
 * @brief  A2DP SNK 连接状态改变回调函数。
 * @par 说明:
 *           注册该回调函数之后，BTS每次完成A2DP SNK连接状态更新后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr        对端蓝牙设备的地址信息。参考 @ref bd_addr_t
 * @param  [in]  conn_state     A2DP SNK 的连接状态。参考 @ref int
 * @retval 无返回值。 参考 @ref void
 * @endif
 * @par Dependency:
 * @li bts_a2dp_snk_source.h
 * @see  a2dp_snk_callbacks_t
 */
typedef void (*a2dp_snk_connect_state_changed_callback)(bd_addr_t *bd_addr, int conn_state);

/**
 * @if Eng
 * @brief  Callback invoked in response to A2DP SNK request to start an audio stream.
 * @par Description:
 *           When registered, the bts uses this callback to deliver A2DP SNK start stream request
 *           when received a request to start an audio stream.
 * @attention  1. This function is called in bts context, should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr        A poniter of the peer bluetooth device addresss. See @ref bd_addr_t
 * @retval No return value. See  @ref void
 * @else
 * @brief  aA2DP SNK 请求流开始回调函数。
 * @par 说明:
 *           注册该回调函数之后，BTS每次收到A2DP SNK请求流开始时调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr        对端蓝牙设备的地址信息。参考 @ref bd_addr_t
 * @retval 无返回值。 参考 @ref void
 * @endif
 * @par Dependency:
 * @li bts_a2dp_snk_source.h
 * @see  a2dp_snk_callbacks_t
 */
typedef void (*a2dp_snk_start_stream_request_callback)(bd_addr_t *bd_addr);

/**
 * @if Eng
 * @brief  Callback invoked in response to A2DP SNK playing state changed.
 * @par Description:
 *           When registered, the bts uses this callback to deliver A2DP SNK playing stat after playing state updated.
 * @attention  1. This function is called in bts context, should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr            A poniter of the peer bluetooth device addresss. See @ref bd_addr_t
 * @param  [in]  playing_state      A2DP SNK playingState state. See @ref int
 * @retval No return value. See  @ref void
 * @else
 * @brief A2dp snk播放状态改变回调函数。
 * @par 说明:
 *           注册该回调函数之后，BTS每次完成A2DP SNK播放状态更新后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr            对端蓝牙设备的地址信息。参考 @ref bd_addr_t
 * @param  [in]  playing_state      A2DP SNK的播放状态。参考 @ref int
 * @retval 无返回值。参考 @ref void
 * @endif
 * @par Dependency:
 * @li bts_a2dp_snk_source.h
 * @see  a2dp_snk_callbacks_t
 */
typedef void (*a2dp_snk_playing_state_changed_callback)(bd_addr_t *bd_addr, int playing_state);

/**
 * @if Eng
 * @brief  Callback invoked in response to a2dp codec information changed.
 * @par Description:
 *           When registered, the bts uses this callback to deliver a2dp codec information
 *           after codec information changed.
 * @attention  1. This function is called in bts context, should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr    A poniter of the peer bluetooth device addresss. See @ref bd_addr_t
 * @param  [in]  info       A poniter of device's codec information. See @ref a2dp_snk_codec_info_t
 * @retval No return value. See @ref void
 * @else
 * @brief  A2dp 编码格式改变回调函数。
 * @par 说明:
 *           注册该回调函数之后，BTS每次完成a2dp状态更新后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr    对端蓝牙设备的地址信息。参考 @ref bd_addr_t
 * @param  [in]  info       A2dp 的编码格式信息。参考 @ref a2dp_snk_codec_info_t
 * @retval 无返回值。参考 @ref void
 * @endif
 * @par Dependency:
 * @li bts_a2dp_snk_source.h
 * @see  a2dp_snk_callbacks_t
 */
typedef void (*a2dp_snk_configuration_changed_callback)(bd_addr_t *bd_addr, a2dp_snk_codec_info_t *info);

/**
 * @if Eng
 * @brief Struct of a2dp snk callback function.
 * @else
 * @brief A2dp snk回调接口定义。
 * @endif
 */
typedef struct {
    a2dp_snk_connect_state_changed_callback connectstate_changed_cb;
    a2dp_snk_start_stream_request_callback start_stream_request_cb;
    a2dp_snk_playing_state_changed_callback playing_state_changed_cb;
    a2dp_snk_configuration_changed_callback configuration_changed_cb;
} a2dp_snk_callbacks_t;












/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* BTS_A2DP_SINK_H */
