/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides EDGE driver api. \n
 *
 * History: \n
 * 2022-12-02, Create file. \n
 */

#ifndef EDGE_H
#define EDGE_H

#include <stdint.h>
#include "errcode.h"
#include "hal_edge.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup drivers_driver_edge EDGE
 * @ingroup  drivers_driver
 * @{
 */

/**
 * @if Eng
 * @brief  EDGE configuration parameters, see @ref hal_edge_param_t.
 * @else
 * @brief  EDGE配置参数，参考 @ref hal_edge_param_t 。
 * @endif
 */
typedef hal_edge_param_t edge_param_t;





/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
