/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 * Description: BT GAP module.
 */

/**
 * @defgroup bluetooth_bts_gap BR GAP API
 * @ingroup  bluetooth
 * @{
 */


#ifndef BTS_BR_GAP_H
#define BTS_BR_GAP_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"
#include "bts_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Limited Discoverability.
 * @else
 * @brief 蓝牙受限可发现设备类型。
 * @endif
 */
#define SERVICE_LIMITED_DISCOVERABILITY 0x002000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Positioning.
 * @else
 * @brief 蓝牙定位设备。
 * @endif
 */
#define SERVICE_POSITIONING 0x010000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Networking.
 * @else
 * @brief 蓝牙网络设备。
 * @endif
 */
#define SERVICE_NETWORKING 0x020000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Render.
 * @else
 * @brief 蓝牙渲染类型设备。
 * @endif
 */
#define SERVICE_RENDER 0x040000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Capture.
 * @else
 * @brief 蓝牙捕获类型设备。
 * @endif
 */
#define SERVICE_CAPTURE 0x080000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that object transfer.
 * @else
 * @brief 蓝牙对象传输类型设备。
 * @endif
 */
#define SERVICE_OBJECT_TRANSFER 0x100000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Audio.
 * @else
 * @brief 蓝牙音频类型设备。
 * @endif
 */
#define SERVICE_AUDIO 0x200000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Telephony.
 * @else
 * @brief 蓝牙电话类型设备。
 * @endif
 */
#define SERVICE_TELEPHONY 0x400000

/**
 * @if Eng
 * @brief The Class of Bluetooth Device/Service that Information.
 * @else
 * @brief 蓝牙信息类型设备。
 * @endif
 */
#define SERVICE_INFORMATION 0x800000

/**
 * @if Eng
 * @brief Enum of Bluetooth stack state.
 * @else
 * @brief 蓝牙协议栈状态。
 * @endif
 */
typedef enum {
    BT_STACK_STATE_TURNING_ON = 0x0, /*!< @if Eng Bluetooth stack state of turning on.
                                          @else   蓝牙协议栈正在初始化中。 @endif */
    BT_STACK_STATE_TURN_ON,          /*!< @if Eng Bluetooth stack state of turn on.
                                          @else   蓝牙协议栈正在运行中。 @endif */
    BT_STACK_STATE_TURNING_OFF,      /*!< @if Eng Bluetooth stack state of turning off.
                                          @else   蓝牙协议栈正在去初始化中。 @endif */
    BT_STACK_STATE_TURN_OFF          /*!< @if Eng Bluetooth stack state of turn off.
                                          @else   蓝牙协议栈已关闭。 @endif */
} bt_stack_state_t;

/**
 * @if Eng
 * @brief  Enum of bt connection strategy type.
 * @else
 * @brief  定义蓝牙的连接类型。
 * @endif
 */
typedef enum {
    BT_CONNECTION_UNKNOWN = 0x00, /*!< @if Eng status of unknown connection. @else 未知连接。 @endif */
    BT_CONNECTION_ALLOWED,        /*!< @if Eng status of allow connection. @else 允许连接。 @endif */
    BT_CONNECTION_FORBIDDEN       /*!< @if Eng status of forbidden connection. @else 禁止连接。 @endif */
} bt_connect_strategy_type_t;

/**
 * @if Eng
 * @brief  Enum of a2dp playing status.
 * @else
 * @brief  定义蓝牙a2dp的播放状态。
 * @endif
 */
typedef enum {
    A2DP_NOT_PLAYING = 0x00, /*!< @if Eng status of a2dp not playing. @else a2dp未播放。 @endif */
    A2DP_IS_PLAYING          /*!< @if Eng status of a2dp is playing. @else a2dp正在播放。 @endif */
} a2dp_playing_state_t;

/**
 * @if Eng
 * @brief  Enum of hfp sco connection status.
 * @else
 * @brief  定义蓝牙hfp的连接状态。
 * @endif
 */
typedef enum {
    HFP_SCO_STATE_CONNECTING = 0x01,    /*!< @if Eng status of sco is connecting. @else sco正在连接。 @endif */
    HFP_SCO_STATE_CONNECTED = 0x02,     /*!< @if Eng status of sco is connected. @else sco已连接。 @endif */
    HFP_SCO_STATE_DISCONNECTING = 0x03, /*!< @if Eng status of sco is disconnecting. @else sco已连接。 @endif */
    HFP_SCO_STATE_DISCONNECTED = 0x04   /*!< @if Eng status of sco is connected. @else sco已连接。 @endif */
} hfp_sco_connect_state_t;

/**
 * @if Eng
 * @brief  Enum of hfp volume type.
 * @else
 * @brief  定义蓝牙hfp的音量调整类型。
 * @endif
 */
typedef enum {
    HFP_VOLUME_MIC = 0x00, /*!< @if Eng volume type of microphone. @else 调整microphone增益。 @endif */
    HFP_VOLUME_SPEAKER     /*!< @if Eng volume type of speaker. @else 调整speaker增益。 @endif */
} hfp_volume_type_t;

/**
 * @if Eng
 * @brief  Enum of bt profile connection status.
 * @else
 * @brief  定义蓝牙协议的连接状态。
 * @endif
 */
typedef enum {
    PROFILE_STATE_CONNECTING = 0x01,    /*!< @if Eng status of connecting. @else 正在连接。 @endif */
    PROFILE_STATE_CONNECTED = 0x02,     /*!< @if Eng status of connected. @else 已连接。 @endif */
    PROFILE_STATE_DISCONNECTING = 0x03, /*!< @if Eng status of disconnecting. @else 正在断连。 @endif */
    PROFILE_STATE_DISCONNECTED = 0x04   /*!< @if Eng status of disconnected. @else 已断连。 @endif */
} profile_connect_state_t;

/**
 * @if Eng
 * @brief  Enum of transport type.
 * @else
 * @brief  定义蓝牙传输的类型。
 * @endif
 */
typedef enum {
    BT_TRANSPORT_INVALID = 0x00, /*!< @if Eng invalid transport type. @else 未知传输类型。 @endif */
    BT_TRANSPORT_BR_EDR = 0x01,  /*!< @if Eng BR/EDR transport type. @else 传输类型为BR/EDR。 @endif */
    BT_TRANSPORT_LE = 0x02       /*!< @if Eng BLE transport type. @else 传输类型为BLE。 @endif */
} bt_transport_type_t;


/**
 * @if Eng
 * @brief Enum of Bluetooth ACL connection state.
 * @else
 * @brief 蓝牙ACL连接状态。
 * @endif
 */
typedef enum {
    GAP_ACL_STATE_CONNECTED,      /*!< @if Eng GAP ACL state of connected.
                                       @else GAP ACL 已连接。 @endif */
    GAP_ACL_STATE_DISCONNECTED,   /*!< @if Eng GAP ACL state of disconnected.
                                       @else GAP ACL 已断接。 @endif */
    GAP_ACL_STATE_LE_CONNECTED,   /*!< @if Eng BLE GAP ACL state of connected.
                                       @else BLE GAP ACL 已连接。 @endif */
    GAP_ACL_STATE_LE_DISCONNECTED /*!< @if Eng BLE GAP ACL state of disconnected.
                                       @else BLE GAP ACL 已断接。 @endif */
} gap_acl_state_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth discovery state.
 * @else
 * @brief 蓝牙扫描状态。
 * @endif
 */
typedef enum {
    GAP_DISCOVERY_STARTED = 0x01, /*!< @if Eng Bluetooth discovery state of started.
                                       @else 已开始扫描。 @endif */
    GAP_DISCOVERYING = 0x02,      /*!< @if Eng Bluetooth discovery state of discovering.
                                       @else 正在扫描。 @endif */
    GAP_DISCOVERY_STOPED = 0x03,  /*!< @if Eng Bluetooth discovery state of stoped.
                                       @else 扫描已停止。 @endif */
} gap_discovery_state_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth bondable mode.
 * @else
 * @brief 蓝牙绑定状态。
 * @endif
 */
typedef enum {
    GAP_BONDABLE_MODE_OFF = 0x00, /*!< @if Eng Bluetooth bondable mode of off.
                                       @else 关闭蓝牙绑定模式。 @endif */
    GAP_BONDABLE_MODE_ON = 0x01   /*!< @if Eng Bluetooth bondable mode of on.
                                       @else 打开蓝牙绑定模式。 @endif */
} gap_bondable_mode_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth access permission.
 * @else
 * @brief 蓝牙访问权限。
 * @endif
 */
typedef enum {
    GAP_ACCESS_PERMISSION_UNKNOWN,   /*!< @if Eng Access type of unknown.
                                          @else 未知。 @endif */
    GAP_ACCESS_PERMISSION_ALLOWED,   /*!< @if Eng Access type of allow.
                                          @else 允许访问。 @endif */
    GAP_ACCESS_PERMISSION_FORBIDDEN, /*!< @if Eng Access type of forbidden.
                                          @else 禁止访问。 @endif */
} gap_access_permission_type_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth scan mode.
 * @else
 * @brief 蓝牙扫描模式。
 * @endif
 */
typedef enum {
    GAP_SCAN_MODE_NONE = 0x00,                      /*!< @if Eng Scan type of none.
                                                         @else 未知扫描类型。 @endif */
    GAP_SCAN_MODE_CONNECTABLE,                      /*!< @if Eng Scan type of connection.
                                                         @else 可连接扫描类型。 @endif */
    GAP_SCAN_MODE_GENERAL_DISCOVERABLE,             /*!< @if Eng Scan type of general discovery.
                                                         @else 通用广播扫描类型。 @endif */
    GAP_SCAN_MODE_LIMITED_DISCOVERABLE,             /*!< @if Eng Scan type of limited discovery.
                                                         @else 受限广播扫描类型。 @endif */
    GAP_SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE, /*!< @if Eng Scan type of connection and general discovery.
                                                         @else 可连接的通用广播扫描类型。 @endif */
    GAP_SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE  /*!< @if Eng Scan type of connection and limited discovery.
                                                         @else 可连接的受限广播扫描类型。 @endif */
} gap_scan_mode_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth profile.
 * @else
 * @brief 蓝牙Profile类型。
 * @endif
 */
typedef enum {
    BT_PROFILE_HEADSET = 0x0, /*!< @if Eng Profile type of Headset.
                                   @else Headset服务类型。 @endif */
    BT_PROFILE_A2DP,          /*!< @if Eng Profile type of A2DP.
                                   @else A2DP服务类型。 @endif */
    BT_PROFILE_OPP,           /*!< @if Eng Profile type of OPP.
                                   @else OPP服务类型。 @endif */
    BT_PROFILE_HID,           /*!< @if Eng Profile type of HID.
                                   @else HID服务类型。 @endif */
    BT_PROFILE_PANU,          /*!< @if Eng Profile type of PANU.
                                   @else PANU服务类型。 @endif */
    BT_PROFILE_NAP,           /*!< @if Eng Profile type of NAP.
                                   @else NAP服务类型。 @endif */
    BT_PROFILE_HFP            /*!< @if Eng Profile type of HFP.
                                   @else HFP服务类型。 @endif */
} bt_profile_type_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth pairing state.
 * @else
 * @brief 蓝牙配对状态。
 * @endif
 */
typedef enum {
    GAP_PAIR_NONE = 0x01,    /*!< @if Eng Pair state of none.
                                  @else 未配对状态。 @endif */
    GAP_PAIR_PAIRING = 0x02, /*!< @if Eng Pair state of pairing.
                                  @else 正在配对。 @endif */
    GAP_PAIR_PAIRED = 0x03   /*!< @if Eng Pair state of paired.
                                  @else 已完成配对。 @endif */
} gap_pair_state_t;

/**
 * @if Eng
 * @brief Length of bluetooth device name.
 * @else
 * @brief 蓝牙设备名称长度。
 * @endif
 */
#define BD_NAME_LEN 32

/**
 * @if Eng
 * @brief Struct of device's pairing information.
 * @else
 * @brief 配对设备信息数据。
 * @endif
 */
typedef struct {
    bd_addr_t addr;                         /*!< @if Eng Bluetooth device address.
                                                 @else   蓝牙地址。 @endif */
    unsigned char device_name[BD_NAME_LEN]; /*!< @if Eng Name of the device record, must be in UTF-8.
                                                 @else   蓝牙设备名称。 @endif */
} gap_paired_device_info_t;

/**
 * @if Eng
 * @brief Enum of Bluetooth pairing type.
 * @else
 * @brief 蓝牙配对类型。
 * @endif
 */
typedef enum {
    GAP_PAIR_CONFIRM_TYPE_PIN_CODE = 0x01,        /*!< @if Eng Pair type of pin code.
                                                       @else 通过pin code 配对。 @endif */
    GAP_PAIR_CONFIRM_TYPE_PASSKEY_DISPLAY = 0x02, /*!< @if Eng Pair type of dispaly passkey.
                                                       @else 展示PASSKEY。 @endif */
    GAP_PAIR_CONFIRM_TYPE_PASSKEY_INPUT = 0x03,   /*!< @if Eng Pair type of input passkey.
                                                       @else 输入PASSKEY。 @endif */
    GAP_PAIR_CONFIRM_TYPE_NUMERIC = 0x04,         /*!< @if Eng Pair type of numeric.
                                                       @else 配对数值。 @endif */
    GAP_PAIR_CONFIRM_TYPE_CONSENT = 0x05,         /*!< @if Eng Pair type of consent.
                                                       @else 请求用户同意。 @endif */
} gap_pair_confirm_type_t;
























/**
 * @if Eng
 * @brief Callback function for bluetooth state change.
 * @par   When registered, this callback reports any change of GAP state to upper layer application.
 * @attention NULL
 * @param  [in] transport #const int, transport type, refer to bt_transport_type_t.
 * @param  [in] status  #const int, GAP state, refer to bt_stack_state_t.
 * @par Dependency:
 * @li  bts_gap.h
* @see  gap_call_backs_t
 * @else
 * @brief  蓝牙开关回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将蓝牙开关状态传递给上层应用。
 * @attention 无
 * @param  [in] 数据传输类型， @ref bt_transport_type_t。
 * @param  [in] GAP状态，  @ref bt_stack_state_t。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_state_changed_callback)(const int transport, const int status);

/**
 * @if Eng
 * @brief Callback function for bluetooth ACL connecction state change.
 * @par   When registered, this callback reports any change of ACL connection state to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] state #gap_acl_state_t, ACL connection state.
 * @param  [in] reason #unsigned int, reason code for state change, refer to HCI error code.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙ACL连接状态变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将ACL连接状态传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] ACL连接状态。
 * @param  [in] 状态变化原因，参考标准HCI错误码。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_acl_state_changed_callback)(const bd_addr_t *bd_addr, gap_acl_state_t state, unsigned int reason);

/**
 * @if Eng
 * @brief Callback function for bluetooth discovery state change.
 * @par   When registered, this callback reports any change of discovery state to upper layer application.
 * @attention NULL
 * @param  [in] status #int, discovery state, refer to gap_discovery_state_t.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙发现状态变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将发现状态传递给上层应用。
 * @attention 无
 * @param  [in] 发现状态，@ref gap_discovery_state_t。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_discovery_state_changed_callback)(int status);

/**
 * @if Eng
 * @brief Callback function for bluetooth discovery result.
 * @par   When registered, this callback reports discovery result to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙发现结果回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将发现结果传递给上层应用。
 * @attention 无
 * @param  [in] 所发现的远端设备地址。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_discovery_result_callback)(const bd_addr_t *bd_addr);

/**
 * @if Eng
 * @brief Callback function for bluetooth pair request.
 * @par   When registered, this callback reports pair requests to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙配对请求回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将配对请求传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_pair_requested_callback)(const bd_addr_t *bd_addr);

/**
 * @if Eng
 * @brief Callback function for bluetooth pair confirmation.
 * @par   When registered, this callback reports pair confirmations to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] req_type #int, the pair request type, refer to gap_pair_confirm_type_t.
 * @param  [in] number #int, paired passkey.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙配对确认回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将配对确认传递给上层应用。
 * @attention 无
 * @param  [in]  远端设备地址。
 * @param  [in]  配对类型，@ref gap_pair_confirm_type_t。
 * @param  [in]  #int, 配对密码。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_pair_confirmed_callback)(const bd_addr_t *bd_addr, int req_type, int number);

/**
 * @if Eng
 * @brief Callback function for bluetooth scan mode change.
 * @par   When registered, this callback reports scan mode change to upper layer application.
 * @attention NULL
 * @param  [in] mode #int, scan mode.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙广播模式变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将广播模式变化传递给上层应用。
 * @attention 无
 * @param  [in]  广播模式。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_scan_mode_changed_callback)(int mode);

/**
 * @if Eng
 * @brief Callback function for bluetooth local device name change.
 * @par   When registered, this callback reports local device name change to upper layer application.
 * @attention NULL
 * @param  [in] device_name #const unsigned char *, local device name.
 * @param  [in] length #unsigned char, the length of device name.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙本地设备名称变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将本地设备名称变化传递给上层应用。
 * @attention 无
 * @param  [in] 本地设备名称。
 * @param  [in] 名称长度。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_local_device_name_changed_callback)(const unsigned char *device_name, unsigned char length);

/**
 * @if Eng
 * @brief Callback function for bluetooth local device address change.
 * @par   When registered, this callback reports local device address change to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the local device.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙本地设备地址变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将本地设备地址变化传递给上层应用。
 * @attention 无
 * @param  [in] 本地设备地址。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_local_device_addr_changed_callback)(const bd_addr_t *bd_addr);

/**
 * @if Eng
 * @brief Callback function for bluetooth pairing state change.
 * @par   When registered, this callback reports pairing state change to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the local device.
 * @param  [in] status #int, pairing status, @ref gap_pair_state_t.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  蓝牙配对状态变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将配对状态变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 配对状态，@ref gap_pair_state_t。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_pair_status_changed_callback)(const bd_addr_t *bd_addr, int status);

/**
 * @if Eng
 * @brief Callback function for uuid change of remote devices.
 * @par   When registered, this callback reports uuid change of remote devices to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the local device.
 * @param  [in] uuid #int, uuid of remote device.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  远端设备UUID变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将远端设备UUID变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 远端设备UUID。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_remote_uuid_changed_callback)(const bd_addr_t *bd_addr, bt_uuid_t uuid);

/**
 * @if Eng
 * @brief Callback function for name change of remote devices.
 * @par   When registered, this callback reports name change of remote devices to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] device_name #const unsigned char *, remote device name.
 * @param  [in] length #unsigned char, the length of device name.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  远端设备名称变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将远端设备名称变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 远端设备名称。
 * @param  [in] 名称长度。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_remote_name_changed_callback)(
    const bd_addr_t *bd_addr, const unsigned char *device_name, unsigned char length);

/**
 * @if Eng
 * @brief Callback function for alias change of remote devices.
 * @par   When registered, this callback reports alias change of remote devices to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] alias #const unsigned char *, remote device alias.
 * @param  [in] length #unsigned char, the length of device alias.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  远端设备昵称变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将远端设备昵称变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 远端设备昵称。
 * @param  [in] 昵称长度。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_remote_alias_changed_callback)(const bd_addr_t *bd_addr, const unsigned char *alias,
    unsigned char length);

/**
 * @if Eng
 * @brief Callback function for Class of Device(CoD) change of remote devices.
 * @par   When registered, this callback reports CoD change of remote devices to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] cod #const unsigned char *, Class of Device.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  远端设备类型变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将远端设备类型变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 设备类型。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_remote_cod_changed_callback)(const bd_addr_t *bd_addr, int cod);

/**
 * @if Eng
 * @brief Callback function for battery level change of remote devices.
 * @par   When registered, this callback reports battery level change of remote device to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] battery_level #const unsigned char *, remote device battery level.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  远端设备电量变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将远端设备电量变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 电量。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_remote_battery_level_changed_callback)(const bd_addr_t *bd_addr, int battery_level);

/**
 * @if Eng
 * @brief Callback function for RSSI change of remote devices.
 * @par   When registered, this callback reports RSSI change of remote device to upper layer application.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param  [in] rssi #const unsigned char *, rssi value.
 * @param  [in] status #const unsigned char *, bt status, @ref bt_status_t.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  远端设备信号强度变化回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数将远端设备信号强度变化传递给上层应用。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 信号强度。
 * @param  [in]BT状态。 @ref bt_status_t。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_read_remote_rssi_event_callback)(const bd_addr_t *bd_addr, int rssi, int status);

/**
 * @if Eng
 * @brief Callback function for check whether to accept connection request.
 * @par   When registered, this callback asks application whether to accept connection request.
 * @attention NULL
 * @param  [in] bd_addr #const bd_addr_t *, the address of the remote device.
 * @param   IN] res #bool *, decision made by application whether to accept.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @else
 * @brief  询问上层应用是否接受连接回调函数。
 * @par    注册该回调函数之后，bluetooth调用该回调函数询问上层应用是否接受连接。
 * @attention 无
 * @param  [in] 远端设备地址。
 * @param  [in] 是否接受连接。
 * @retval void  无返回值。
 * @par 依赖:
 * @li  bts_gap.h
 * @see  gap_call_backs_t
 * @endif
 */
typedef void (*gap_is_accept_conn_on_safe_mode_callback)(const bd_addr_t *bd_addr, bool *res);

/**
 * @if Eng
 * @brief Struct of GAP callback functions.
 * @else
 * @brief AP回调函数接口定义。
 * @endif
 */
typedef struct {
    gap_state_changed_callback state_change_callback;
    gap_acl_state_changed_callback acl_state_changed_callbak;
    gap_scan_mode_changed_callback scan_mode_changed_callback;
    gap_pair_status_changed_callback pair_status_changed_callback;
    gap_local_device_name_changed_callback device_name_changed_callback;
    gap_local_device_addr_changed_callback device_addr_changed_callback;
    gap_pair_requested_callback pair_requested_callback;
    gap_pair_confirmed_callback pair_confiremed_callback;
    gap_discovery_state_changed_callback discovery_state_changed_callback;
    gap_discovery_result_callback discovery_result_callback;
    gap_remote_uuid_changed_callback remote_uuid_changed_callback;
    gap_remote_name_changed_callback remote_name_changed_callback;
    gap_remote_alias_changed_callback remote_alias_changed_callback;
    gap_remote_cod_changed_callback remote_cod_changed_callback;
    gap_remote_battery_level_changed_callback remote_battery_level_changed_callback;
    gap_read_remote_rssi_event_callback read_remote_rssi_event_callback;
    gap_is_accept_conn_on_safe_mode_callback is_accept_conn_on_safe_mode_callback;
} gap_call_backs_t;


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
