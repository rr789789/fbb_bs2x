/**
 * Copyright (c) @CompanyNameMagicTag 2023-2024. All rights reserved. \n
 *
 * Description: Application core main function for pm demo \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-28, Create file. \n
 */

#ifndef LOWP_H
#define LOWP_H

#include "app_init.h"
#include "hal_reboot.h"
#include "pinctrl.h"
#include "gpio.h"
#include "preserve.h"
#include "cmsis_os2.h"
#include "watchdog.h"
#include "soc_osal.h"

#include "ulp_gpio.h"
#include "pm_sys.h"
#include "RGB.h"
#include "sle_client.h"
// 低功耗的三个状态
#define PM_STATE_WORK      0
#define PM_STATE_STANDBY   1
#define PM_STATE_SLEEP     2
extern int led_state; // 灯的状态
#define PM_SAMPLE_GPIO_NUM  S_MGPIO2       // 用于低功耗唤醒的GPIO引脚编号


void pm_low_power_entry(void);
#endif