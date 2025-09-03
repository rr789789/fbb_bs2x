// ec11_encoder.c
#include "encoder.h"
#include "errcode.h"
#include "hal_gpio.h"
#include "string.h"
#include "gpio.h"
#include "soc_osal.h"
#include "systick.h"
#include "surface_dial_hid.h"



// 编码器状态存储
static struct
{
    ec11_config_t config;
    ec11_state_t state;
    uint8_t last_encoded;
    bool initialized;
    uint64_t last_change_time;
} ec11_dev = {0};

// 前向声明
static void ec11_gpio_isr(pin_t pin);
static void ec11_button_isr(pin_t pin);

// 初始化EC11编码器
errcode_t ec11_init(const ec11_config_t *config)
{
    if (config == NULL)
    {
        return ERRCODE_FAIL;
    }

    if (ec11_dev.initialized)
    {
        ec11_deinit();
    }

    // 保存配置
    memcpy(&ec11_dev.config, config, sizeof(ec11_config_t));
    ec11_dev.state = EC11_ROTATE_NONE;
    ec11_dev.last_encoded = 0;
    ec11_dev.last_change_time = 0;

    // 初始化GPIO
    uapi_gpio_init();

    
    // 配置AB引脚输入
    uapi_gpio_set_dir(config->pin_a, GPIO_DIRECTION_INPUT);
    uapi_gpio_set_dir(config->pin_b, GPIO_DIRECTION_INPUT);

    // 注册中断
    uapi_gpio_register_isr_func(config->pin_a, 2 | 1, (gpio_callback_t)ec11_gpio_isr);
    uapi_gpio_register_isr_func(config->pin_b, 2 | 1, (gpio_callback_t)ec11_gpio_isr);

    // 使能中断
    uapi_gpio_enable_interrupt(config->pin_a);
    uapi_gpio_enable_interrupt(config->pin_b);

    // 如果启用按键
    if (config->has_button)
    {
        uapi_gpio_set_dir(config->pin_button, GPIO_DIRECTION_INPUT);
        uapi_gpio_register_isr_func(config->pin_button, 2 | 1, (gpio_callback_t)ec11_button_isr);
        uapi_gpio_enable_interrupt(config->pin_button);
    }

    ec11_dev.initialized = true;
    return ERRCODE_SUCC;
}

// 反初始化EC11编码器
void ec11_deinit(void)
{
    if (!ec11_dev.initialized)
    {
        return;
    }

    // 禁用中断
    uapi_gpio_disable_interrupt(ec11_dev.config.pin_a);
    uapi_gpio_disable_interrupt(ec11_dev.config.pin_b);

    // 取消注册中断处理函数
    uapi_gpio_unregister_isr_func(ec11_dev.config.pin_a);
    uapi_gpio_unregister_isr_func(ec11_dev.config.pin_b);

    // 如果启用了按键功能
    if (ec11_dev.config.has_button)
    {
        uapi_gpio_disable_interrupt(ec11_dev.config.pin_button);
        uapi_gpio_unregister_isr_func(ec11_dev.config.pin_button);
    }

    ec11_dev.initialized = false;
}

// 获取当前状态
ec11_state_t ec11_get_state(void)
{
    ec11_state_t current_state = ec11_dev.state;
    // 读取后清除状态(单次触发)
    ec11_dev.state = EC11_ROTATE_NONE;
    return current_state;
}

// 处理旋转中断
static void ec11_gpio_isr(pin_t pin)
{
    unused(pin);

    uint64_t current_time = uapi_systick_get_ms();

    // 消抖
    if ((current_time - ec11_dev.last_change_time) < ec11_dev.config.debounce_ms)
    {
        return ;
    }

    ec11_dev.last_change_time = current_time;

    // 读取AB引脚电平
    gpio_level_t pin_a_val = uapi_gpio_get_val(ec11_dev.config.pin_a);
    gpio_level_t pin_b_val = uapi_gpio_get_val(ec11_dev.config.pin_b);

    // 当前编码状态
    uint8_t encoded = (pin_a_val << 1) | pin_b_val;
    uint8_t last = ec11_dev.last_encoded;

    // 计算四步状态转换(格雷码)
    uint8_t sum = (last << 2) | encoded;

    // 确定旋转方向 - 根据格雷码状态判断方向
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    {
        ec11_dev.state = EC11_ROTATE_CW;
    }
    else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    {
        ec11_dev.state = EC11_ROTATE_CCW;
    }

    // 存储当前编码状态用于下次比较
    ec11_dev.last_encoded = encoded;

    // 调用回调函数
    if (ec11_dev.config.callback && ec11_dev.state != EC11_ROTATE_NONE)
    {
        ec11_dev.config.callback(ec11_dev.state, ec11_dev.config.user_data);
    }
}

// 处理按键中断
static void ec11_button_isr(pin_t pin)
{
    unused(pin);

    uint32_t current_time = uapi_systick_get_ms();

    // 消抖
    if ((current_time - ec11_dev.last_change_time) < ec11_dev.config.debounce_ms)
    {
        return ;
    }

    ec11_dev.last_change_time = current_time;

    // 读取按键状态
    gpio_level_t button_val = uapi_gpio_get_val(ec11_dev.config.pin_button);

    if (button_val == GPIO_LEVEL_LOW)
    {
        // 按键按下(低电平有效)
        ec11_dev.state = EC11_BUTTON_PRESS;
    }
    else
    {
        // 按键释放
        ec11_dev.state = EC11_BUTTON_RELEASE;
    }

    // 调用回调函数
    if (ec11_dev.config.callback)
    {
        ec11_dev.config.callback(ec11_dev.state, ec11_dev.config.user_data);
    }
}