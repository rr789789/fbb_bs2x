// ec11_encoder.h
#ifndef EC11_ENCODER_H
#define EC11_ENCODER_H

#include "gpio.h"
#include "soc_osal.h"
#include "stdbool.h"

// 自定义GPIO中断类型枚举
typedef enum
{
    GPIO_IRQ_TRIGGER_RISING,  // 上升沿触发
    GPIO_IRQ_TRIGGER_FALLING, // 下降沿触发
    GPIO_IRQ_TRIGGER_BOTH     // 双边沿触发
} gpio_irq_type_t;

// 定义EC11状态
typedef enum
{
    EC11_ROTATE_NONE = 0,
    EC11_ROTATE_CW,     // 顺时针
    EC11_ROTATE_CCW,    // 逆时针
    EC11_BUTTON_PRESS,  // 按键按下
    EC11_BUTTON_RELEASE // 按键释放
} ec11_state_t;

// 定义EC11回调函数类型，作用于创建下方结构体
typedef void (*ec11_callback_t)(ec11_state_t state, void *arg);

// EC11编码器配置结构体
typedef struct
{
    pin_t pin_a;              // A相引脚
    pin_t pin_b;              // B相引脚
    pin_t pin_button;         // 按键引脚
    bool has_button;          // 是否使用按键功能
    uint32_t debounce_ms;     // 去抖延时(毫秒)
    ec11_callback_t callback; // 状态变化回调
    void *user_data;          // 用户数据指针
} ec11_config_t;

// 初始化EC11编码器
errcode_t ec11_init(const ec11_config_t *config);

// 反初始化EC11编码器
void ec11_deinit(void);

// 获取当前状态(非阻塞)
ec11_state_t ec11_get_state(void);

#endif // EC11_ENCODER_H