#ifndef _BUTTON_H
#define _BUTTON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "pinctrl.h"
#include "common_def.h"
#include "soc_osal.h"
#include "gpio.h"
#include "hal_gpio.h"
#include "watchdog.h"
#include "app_init.h"

#define BUTTON_GPIO 14 // 按键
#define BUTTON_TASK_STACK_SIZE 0x0400
#define BUTTON_TASK_PRIO 25


#ifdef __cplusplus
}
#endif

#endif
