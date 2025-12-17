
#ifndef RUST_WRAPPER_H
#define RUST_WRAPPER_H

// 标准头文件
#include <stdint.h>
#include <stddef.h>

// 尝试包含需要的原始 SDK 头文件
// 这些路径会通过 -I 参数在编译时解析
#ifdef __has_include
  #if __has_include("gpio.h")
    #include "gpio.h"
  #endif
  #if __has_include("pinctrl.h")
    #include "pinctrl.h"
  #endif
  #if __has_include("soc_osal.h")
    #include "soc_osal.h"
  #endif
  #if __has_include("app_init.h")
    #include "app_init.h"
  #endif
  #if __has_include("boards.h")
    #include "boards.h"
  #endif
#else
  // 如果编译器不支持 __has_include，则尝试直接包含
  // 这可能会产生警告，但会继续编译
  #include "gpio.h"
  #include "pinctrl.h"
#endif

// 手动定义一些基本常量和类型（作为后备）
#ifndef CONFIG_SAMPLE_BLINKY_PIN
#define CONFIG_SAMPLE_BLINKY_PIN 5
#endif

// GPIO 相关常量
#ifndef HAL_PIO_FUNC_GPIO
#define HAL_PIO_FUNC_GPIO 0
#endif

#ifndef GPIO_DIRECTION_OUTPUT
#define GPIO_DIRECTION_OUTPUT 1
#endif

#ifndef GPIO_LEVEL_LOW
#define GPIO_LEVEL_LOW 0
#endif

// OSAL 相关的基本声明（如果找不到头文件）
#ifndef _OSAL_TASK_DEFINED
typedef struct osal_task osal_task;
#define _OSAL_TASK_DEFINED
#endif

// 函数声明（如果头文件中没有）
#ifndef UAPI_FUNCTIONS_DECLARED
extern int uapi_pin_set_mode(unsigned int pin, unsigned int mode);
extern int uapi_gpio_set_dir(unsigned int pin, unsigned int direction);
extern int uapi_gpio_set_val(unsigned int pin, unsigned int level);
extern int uapi_gpio_toggle(unsigned int pin);
#define UAPI_FUNCTIONS_DECLARED
#endif

#ifndef OSAL_FUNCTIONS_DECLARED
extern void osal_msleep(unsigned int ms);
extern void osal_printk(const char *fmt, ...);
extern void osal_kthread_lock(void);
extern void osal_kthread_unlock(void);
extern osal_task* osal_kthread_create(void* handler, int arg, const char *name, unsigned int stack_size);
extern int osal_kthread_set_priority(osal_task* task, unsigned int priority);
#define OSAL_FUNCTIONS_DECLARED
#endif

#endif // RUST_WRAPPER_H
