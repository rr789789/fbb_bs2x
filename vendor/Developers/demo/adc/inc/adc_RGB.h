#ifndef ADC_RGB_H
#define ADC_RGB_H


#include "pinctrl.h"
#include "gpio.h"
#include "adc.h"
#include "adc_porting.h"
#include "osal_debug.h"
#include "cmsis_os2.h"
#include "app_init.h"
#include "pm_pmu.h"
#include "boards.h"
#include "soc_osal.h"
#include "watchdog.h" 
#define ADC_TASK_STACK_SIZE               0x1000
#define ADC_TASK_PRIO                     (osPriority_t)(17)
#define ADC_AUTO_SAMPLE_TEST_TIMES        1000      //自动采样次数

#define GAFE_SAMPLE_VALUE_SIGN_BIT        17        //ADC 采样值的符号位（17 位）
#define ADC_REFERENCE_VOLTAGE_MV          1500      //ADC 的参考电压mV
#define ADC_REF_VOL_DIFFERENCE_MULT       2         //参考电压的倍数
#define ADC_TICK2VOL_REF_VOLTAGE_MV       (ADC_REFERENCE_VOLTAGE_MV * ADC_REF_VOL_DIFFERENCE_MULT)//计算了用于将 ADC 采样值转换为电压的参考电压（3000 毫伏）。

#define RGB_DURATION_MS      500 // RGB灯闪烁间隔时间
#define RGB_TASK_PRIO        24  // 闪烁任务优先级
#define RGB_TASK_STACK_SIZE  0X1000 // 闪烁任务堆栈大小

// 三色灯引脚
#define RED_LED_PIN               16
#define GREEN_LED_PIN             15
#define BLUE_LED_PIN              18
//extern volatile int led_state = 4;
void init_rgb_led_pins(pin_t pin);
void control_led(int red_state, int green_state, int blue_state);
void set_led_state(int state);

#endif // ADC_RGB_H
