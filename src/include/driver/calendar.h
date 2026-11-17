/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: Provides Calendar driver api \n
 *
 * History: \n
 * 2022-07-20， Create file. \n
 */
#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup drivers_driver_calendar Calendar
 * @ingroup  drivers_driver
 * @{
 */

/**
 * @if Eng
 * @brief  Definition of Calendar data structure.
 * @else
 * @brief  日历数据结构定义。
 * @endif
 */
typedef struct calendar {
    uint32_t sec;           /*!< @if Eng Second, 0 ~ 59.
                                 @else   秒  0 ~ 59 @endif */
    uint32_t min;           /*!< @if Eng Minute, 0 ~ 59.
                                 @else   分钟  0 ~ 59 @endif */
    uint32_t hour;          /*!< @if Eng Hour, 0 ~ 23.
                                 @else   小时，0 ~ 23。 @endif */
    uint32_t day;           /*!< @if Eng Day, Max day of month.
                                 @else   日，月的最大天数。 @endif */
    uint32_t mon;           /*!< @if Eng Month, 1 ~ 12.
                                 @else   月，1 ~ 12。 @endif */
    uint32_t year;          /*!< @if Eng Year, 1970 ~ 2099.
                                 @else   年，1970 ~ 2099。 @endif */
} calendar_t;







/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
