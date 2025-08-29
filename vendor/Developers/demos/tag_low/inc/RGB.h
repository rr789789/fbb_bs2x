
#ifndef RGB_H
#define RGB_H

#include "boards.h"
#include "pinctrl.h"
#include "gpio.h"
#include "soc_osal.h"
#include "app_init.h"
#include "watchdog.h"

#define RGB_DURATION_MS      500 // RGB灯闪烁间隔时间
#define RGB_TASK_PRIO        24  // 闪烁任务优先级
#define RGB_TASK_STACK_SIZE  0x1000 // 闪烁任务堆栈大小

// 三色灯引脚
#define RED_LED_PIN               16
#define GREEN_LED_PIN             15
#define BLUE_LED_PIN              18
#define RST_PIN                   6
#define ALL_LEDS_OFF                       0
#define RED_LED_ON                         1
#define GREEN_LED_ON                       2
#define BLUE_LED_ON                        3
#define ALL_LEDS_ON                        4
#define XZERO                              0
#define XFOUR                              4
extern int led_state; // 灯的状态

// 初始化RGB灯引脚
void init_rgb_led_pins(pin_t pin);
void sle_client_init(void);
void rgb_init(void);
void rgb_blinky_task(void);
// 控制RGB灯
void control_led(int red_state, int green_state, int blue_state);
#endif 
