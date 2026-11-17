/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: BT HFP HF module.
 */

/**
 * @defgroup bluetooth_bts_hfp_hf HFP HF API
 * @ingroup  bluetooth
 * @{
 */

#ifndef BTS_HFP_HF_H
#define BTS_HFP_HF_H

#include <stdbool.h>
#include "bts_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Max value of volume.
 * @else
 * @brief 最大音量值范围。
 * @endif
 */
#define HFP_HF_MAX_VOLUME 15

/**
 * @if Eng
 * @brief No in band tong.
 * @else
 * @brief 非带内铃声。
 * @endif
 */
#define HFP_HF_NO_IN_BAND_TONG 0x00

/**
 * @if Eng
 * @brief In band tong.
 * @else
 * @brief 带内铃声。
 * @endif
 */
#define HFP_HF_IN_BAND_TONG 0x01

/**
 * @if Eng
 * @brief Incoming call ringing.
 * @else
 * @brief 来电振铃。
 * @endif
 */
#define HFP_HF_INCOMING_CALL 0x10

/**
 * @if Eng
 * @brief Terminate local generated ring tone or the in-band ring tone.
 * @else
 * @brief 终止本地生成的振铃音或带内振铃音。
 * @endif
 */
#define HFP_HF_TERMINATE_LOCAL_RINGTONE 0x20

/**
 * @if Eng
 * @brief Enum of phone calling action policy.
 * @else
 * @brief 接受一路呼叫时对其他呼叫的操作策略。
 * @endif
 */
typedef enum {
    HFP_HF_ACCEPT_CALL_ACTION_NONE = 0x00,  /*!< @if Eng HFP HF phone calling action of none
                                                      @else 接受呼叫后无操作 @endif */
    HFP_HF_ACCEPT_CALL_ACTION_HOLD = 0x01,  /*!< @if Eng HFP HF phone calling action of hold
                                                      @else 接受呼叫保持当前通话 @endif */
    HFP_HF_ACCEPT_CALL_ACTION_FINISH = 0x02 /*!< @if Eng HFP HF phone calling action of finish
                                                      @else 接受呼叫后结束当前通话 @endif */
} hfp_hf_accept_call_action_type_t;

/**
 * @if Eng
 * @brief Enum of Phone call state type.
 * @else
 * @brief 电话呼叫状态类型。
 * @endif
 */
typedef enum {
    HFP_HF_CALL_STATE_ACTIVE = 0, /*!< @if Eng HFP phone calling state of active @else HFP HF正在通话 @endif */
    HFP_HF_CALL_STATE_HELD,       /*!< @if Eng HFP phone calling state of hold @else HFP HF呼叫保持通话 @endif */
    HFP_HF_CALL_STATE_DIALING,    /*!< @if Eng HFP phone calling state of dialing @else HFP HF呼叫正在拨号 @endif */
    HFP_HF_CALL_STATE_ALERTING,   /*!< @if Eng HFP phone calling state of alerting @else HFP HF呼叫正在响铃 @endif */
    HFP_HF_CALL_STATE_INCOMING,   /*!< @if Eng HFP phone calling state of incoming @else HFP HF呼叫接入 @endif */
    HFP_HF_CALL_STATE_WAITING,    /*!< @if Eng HFP phone calling state of waiting @else HFP HF呼叫等待 @endif */
    HFP_HF_CALL_STATE_RESPONSE_HELD, /*!< @if Eng HFP phone calling state of respond hold
                                          @else HFP HF呼叫响应保持 @endif */
    HFP_HF_CALL_STATE_FINISHED       /*!< @if Eng HFP phone calling state of finshed @else HFP HF呼叫结束 @endif */
} hfp_hf_call_state_t;

/**
 * @if Eng
 * @brief Struct of Phone call info.
 * @else
 * @brief 定义电话呼叫信息。
 * @endif
 */
typedef struct {
    bd_addr_t bd_addr;        /*!< @if Eng Remote device address @else  远端设备地址 @endif */
    int id;                   /*!< @if Eng Call index @else 编号 @endif */
    int state;                /*!< @if Eng Call state see hfp_hf_call_state_t @else  电话呼叫状态 @endif */
    char *number;             /*!< @if Eng Call number @else  电话号码 @endif */
    unsigned char number_len; /*!< @if Eng Call number length @else  电话号码位数 @endif */
    bt_uuid_t uuid;           /*!< @if Eng Call uuid @else  电话服务uuid @endif */
    bool multi_party;         /*!< @if Eng Is multiparty flag @else 多方电话 @endif */
    bool outgoing;            /*!< @if Eng Incoming/outing flag @else 呼入/呼出 @endif */
    bool in_band_ring;        /*!< @if Eng Is inband-ring flag @else  inband-ring模式 @endif */
    long creation_time;       /*!< @if Eng call start time @else  呼叫开始时间 @endif */
} hfp_hf_calls_info_t;

/**
 * @if Eng
 * @brief Enum of codec info.
 * @else
 * @brief 定义codec编码信息。
 * @endif
 */
typedef enum {
    HFP_HF_CODEC_ID_CVSD = 0x01, /*!< @if Eng HFP HF codec type of CVSD @else  HFP HF 编码格式为CVSD @endif */
    HFP_HF_CODEC_ID_MSBC,        /*!< @if Eng HFP HF codec type of MSBC @else  HFP HF 编码格式为MSBC @endif */
} hfp_hf_codec_id_t;

/**
 * @if Eng
 * @brief Callback invoked in response to hfp connect state changed.
 * @par Description: When registered, the bts uses this callback to deliver hfp connect state after hfp connect updated.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss. See @ref bd_addr_t
 * @param  [in]  state    Connect state of hfp. See {@link profile_connect_state_t}
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief hfp连接状态更新回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次hfp连接状态更新后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  state    hfp的连接状态。见 {@link profile_connect_state_t}
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_conn_state_changed_callback)(const bd_addr_t *bd_addr, profile_connect_state_t state);

/**
 * @if Eng
 * @brief Callback invoked in response to sco connect state changed.
 * @par Description: When registered, the bts uses this callback to deliver sco connect stat after sco connect updated.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  state    Connect state of sco. See {@link hfp_sco_connect_state_t}
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief sco连接状态更新回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次sco连接状态更新后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  state    sco的连接状态。见 {@link hfp_sco_connect_state_t}.
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_sco_conn_state_changed_callback)(const bd_addr_t *bd_addr, hfp_sco_connect_state_t state);

/**
 * @if Eng
 * @brief Callback invoked in response to receive AG charge change indication.
 * @par Description: When registered, the bts uses this callback to deliver AG charge change indication.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr        A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  battery_level  Power level, range 0-5.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 收到AG电量变化指示回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次收到AG电量变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr        对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  battery_level  电量等级，范围0-5。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_battery_level_changed_callback)(const bd_addr_t *bd_addr, int battery_level);

/**
 * @if Eng
 * @brief Callback invoked in response to receive AG signal strength change indication.
 * @par Description: When registered, the bts uses this callback to deliver AG signal strength change indication.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr          A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  signal_strength  Signal level, range 0-5.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 收到AG信号强度变化指示回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次收到AG信号强度变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr          对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  signal_strength  信号强度等级，范围0-5。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_signal_strength_changed_callback)(const bd_addr_t *bd_addr, int signal_strength);

/**
 * @if Eng
 * @brief Callback invoked in response to receive AG registration status change indication.
 * @par Description: When registered, the bts uses this callback to deliver a2dp playing stat after playing state
 *                   updated.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  status   Registration status, range 0-1.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 收到AG注册状态变化指示回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次收到AG注册状态变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  status   注册状态，取值为0或1。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_registration_status_changed_callback)(const bd_addr_t *bd_addr, int status);

/**
 * @if Eng
 * @brief Callback invoked in response to receive AG roaming status change indication.
 * @par Description: When registered, the bts uses this callback to deliver AG roaming status after roaming state
 *                   updated.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  status   Roaming status, range 0-1.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 收到AG漫游状态变化指示回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次收到AG漫游状态变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  status   漫游状态，取值为0或1。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_roaming_status_changed_callback)(const bd_addr_t *bd_addr, int status);

/**
 * @if Eng
 * @brief Callback invoked in response to query AG operators information after receiving the AG reply.
 * @par Description: When registered,the bts uses this callback to deliver AG operators information
 *                   after receiving the AG reply.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  name     Operator name.
 * @param  [in]  len      The length of operator name.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 查询AG运营商信息收到AG回复后的回调函数。
 * @par 注册该回调函数之后，BTS每次收到查询AG运营商信息,在收到AG回复后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  name     运营商名字。
 * @param  [in]  len      运营商名字长度。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_operator_selection_callback)(const bd_addr_t *bd_addr, unsigned char *name, unsigned int len);

/**
 * @if Eng
 * @brief Callback invoked in response to query AG users phone number after receiving the AG reply.
 * @par Description: When registered,the bts uses this callback to deliver AG user number information
 *                   after receiving the AG reply.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  number   Phone number.
 * @param  [in]  len      The length of phone number.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 查询AG用户号码收到AG回复后的回调函数。
 * @par 注册该回调函数之后，BTS每次收到查询AG用户号码信息,在收到AG回复后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  number   电话号码。
 * @param  [in]  len      电话号码长度。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_subscriber_number_callback)(const bd_addr_t *bd_addr, unsigned char *number, unsigned int len);

/**
 * @if Eng
 * @brief Callback invoked in response to recognize Speech status changes.
 * @par Description: When registered,the bts uses this callback to deliver Speech status information after being
 *                   recognized.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  status   Voice recognition status, range 0-1.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 语音识别状态变化回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次语音识别状态变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  status   语音识别状态, 取值范围：0到1。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_voice_recognition_status_changed_callback)(const bd_addr_t *bd_addr, int status);

/**
 * @if Eng
 * @brief Callback invoked in response to recognize In-band Ring tone status changes.
 * @par Description: When registered,the bts uses this callback to deliver In-band Ring tone status information
 *                   after In-band Ring tone status changes.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  status   In-band Ring tone status value range: 0-1.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 带内铃声状态变化回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次带内铃声状态变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  status   带内铃声状态 取值范围：0到1。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_in_band_ring_tone_changed_callback)(const bd_addr_t *bd_addr, int status);

/**
 * @if Eng
 * @brief Callback invoked in response to adjust HF volume at AG end.
 * @par Description: When registered,the bts uses this callback to deliver HF volume information
 *                   after HF volume status changes.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  type     Adjust HF volume type. See {@link hfp_volume_type_t}
 * @param  [in]  volume   Acoustic gain, range 0-15.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief AG端调整HF音量回调函数。
 * @par 注册该回调函数之后，BTS每次AG端调整HF音量后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  type     调整HF音量类型。参考 {@link hfp_volume_type_t}
 * @param  [in]  volume   音量增益，范围0-15。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_volume_changed_callback)(const bd_addr_t *bd_addr, hfp_volume_type_t type, unsigned char volume);

/**
 * @if Eng
 * @brief Callback invoked in response to receive call status change indication.
 * @par Description: When registered,the bts uses this callback to deliver AG Call Status
 *                   after the call status changes.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  call     Call status information. See {@link hfp_hf_calls_info_t}
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 通话状态变化回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次收到通话状态变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  call     通话状态信息，参考 {@link hfp_hf_calls_info_t}。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_call_changed_callback)(const bd_addr_t *bd_addr, const hfp_hf_calls_info_t *call);

/**
 * @if Eng
 * @brief Callback invoked in response to dial failure.
 * @par Description: When registered,the bts uses this callback to deliver dialing error code information
 *                   after dialing failure.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  code     Dialing error code information.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief 拨号错误回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次收到拨号失败后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  code     拨号错误码。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_dial_error_code_callback)(const bd_addr_t *bd_addr, int code);

/**
 * @if Eng
 * @brief Callback invoked in response to update codec.
 * @par Description: When registered,the bts uses this callback to deliver codec change information
 *                   after codec changed.
 * @attention  1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention  2. The memories of <devices> are requested and freed by the bts automatically.
 * @param  [in]  bd_addr  A poniter of the peer bluetooth device addresss.See @ref bd_addr_t
 * @param  [in]  codec    Codec information.
 * @retval void  No return value.
 * @par Dependency:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @else
 * @brief codec编码信息变化回调函数。
 * @par 说明: 注册该回调函数之后，BTS每次codec编码格式变化后调用该接口反馈结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  bd_addr  对端蓝牙设备的地址信息。见 @ref bd_addr_t
 * @param  [in]  codec    编码信息。
 * @retval void  无返回值。
 * @par 依赖:
 *            @li bts_hfp_hf.h
 * @see hfp_hf_callbacks_t
 * @endif
 */
typedef void (*hfp_hf_codec_changed_callback)(const bd_addr_t *bd_addr, int codec);

/**
 * @if Eng
 * @brief Struct of hfp hf callback function.
 * @else
 * @brief HFP HF回调函数接口定义。
 * @endif
 */
typedef struct {
    hfp_hf_conn_state_changed_callback conn_state_changed_cb;
    hfp_hf_sco_conn_state_changed_callback sco_conn_state_changed_cb;
    hfp_hf_battery_level_changed_callback battery_level_changed_cb;
    hfp_hf_signal_strength_changed_callback signal_strength_changed_cb;
    hfp_hf_registration_status_changed_callback registration_status_changed_cb;
    hfp_hf_roaming_status_changed_callback roaming_status_changed_cb;
    hfp_hf_operator_selection_callback operator_selection_cb;
    hfp_hf_subscriber_number_callback subscriber_number_cb;
    hfp_hf_voice_recognition_status_changed_callback voice_recognition_status_changed_cb;
    hfp_hf_in_band_ring_tone_changed_callback in_band_ring_tone_changed_cb;
    hfp_hf_volume_changed_callback volume_changed_cb;
    hfp_hf_call_changed_callback call_changed_cb;
    hfp_hf_dial_error_code_callback dial_error_code_cb;
    hfp_hf_codec_changed_callback codec_changed_cb;
} hfp_hf_callbacks_t;

















#ifdef __cplusplus
}
#endif

#endif /* end of bts_hfp.h */
