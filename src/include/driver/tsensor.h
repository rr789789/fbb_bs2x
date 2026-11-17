/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides Tsensor driver api \n
 *
 * History: \n
 * 2022-09-16, Create file. \n
 */

#ifndef TSENSOR_H
#define TSENSOR_H

#include <stdint.h>
#include "errcode.h"
#include "tsensor_porting.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup drivers_driver_tsensor Tsensor
 * @ingroup  drivers_driver
 * @{
 */

#if defined(CONFIG_TSENSOR_TEMP_COMPENSATION)
/**
 * @if Eng
 * @brief  Tsensor calibration configuration.
 * @else
 * @brief  Tsensor温度补偿点定义。
 * @endif
 */
typedef struct tsensor_calibration_point {
    int8_t tsensor_temp;            /*!< @if Eng tsensor temp.
                                          @else   tsensor传感器温度。  @endif */
    int8_t environment_temp;        /*!< @if Eng environment temp.
                                          @else   真实环境温度。  @endif */
} tsensor_calibration_point_t;
#endif /* CONFIG_TSENSOR_TEMP_COMPENSATION */

/**
 * @if Eng
 * @brief  Definition of the samping mode of Tsensor.
 * @else
 * @brief  Tsensor设置采样模式定义。
 * @endif
 */
typedef enum tsensor_samp_mode {
    TSENSOR_SAMP_MODE_AVERAGE_ONCE,        /*!< @if Eng 16-point average single report mode.
                                                @else   16点平均单次上报模式。  @endif */
    TSENSOR_SAMP_MODE_AVERAGE_CYCLE,       /*!< @if Eng 16-point average cyclic reporting mode.
                                                @else   16点平均循环上报模式。  @endif */
    TSENSOR_SAMP_MODE_SINGLE_POINT_CYCLE,  /*!< @if Eng single point cyclic reporting mode.
                                                @else   单点循环上报模式。  @endif */
    TSENSOR_SAMP_MODE_MAX_NUM,
    TSENSOR_SAMP_MODE_NONE = TSENSOR_SAMP_MODE_MAX_NUM
} tsensor_samp_mode_t;

/**
 * @if Eng
 * @brief  Tsensor callback functions.
 * @param  [in]  temp Tsensor temperature.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t .
 * @else
 * @brief  Tsensor 回调类型定义。
 * @param  [in]  temp Tsensor温度。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败，参考 @ref errcode_t 。
 * @endif
 */
typedef errcode_t (*uapi_tsensor_callback_t)(int8_t temp);








#if defined(CONFIG_TSENSOR_TEMP_COMPENSATION)

#endif /* CONFIG_TSENSOR_TEMP_COMPENSATION */

#if defined(CONFIG_TSENSOR_MULTILEVEL)

#endif /* CONFIG_TSENSOR_MULTILEVEL */

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
