
/**
 * Copyright (c) @CompanyNameMagicTag 2023-2024. All rights reserved. \n
 *
 * Description: Application core main function for pm demo \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-28, Create file. \n
 */

#include "lowp.h"

static uint8_t g_pm_state = PM_STATE_WORK; // 记录当前的电源管理状态

// GPIO中断处理函数 //
// 清除GPIO中断标志，并调用uapi_pm_work_state_reset()函数重置工作状态，使系统从低功耗状态恢复到工作状态 
static void pm_gpio_irq_func(pin_t pin)
{
    osal_printk("[pm_sys]gpio%d irq.\n", pin);
    uapi_gpio_clear_interrupt(pin);
    uapi_pm_work_state_reset();
}

// ULP GPIO唤醒处理函数 //
// 当ULP GPIO唤醒唤醒时，调用uapi_pm_wkup_process()函数处理唤醒事件，并打印唤醒信息
static void pm_ulpgpio_wkup_handler(uint8_t ulp_gpio)
{
    uapi_pm_wkup_process(0);
    osal_printk("ulp_gpio%d wakeup\n", ulp_gpio);
}

// GPIO配置函数 //
// 定义了一个ULP GPIO中断唤醒配置数组，配置了GPIO引脚PM_SAMPLE_GPIO_NUM，当检测到下降沿中断时，调用pm_ulpgpio_wkup_handler函数
static ulp_gpio_int_wkup_cfg_t g_pm_wk_cfg[] = {
    { 0, PM_SAMPLE_GPIO_NUM, true, ULP_GPIO_INTERRUPT_RISING_EDGE, pm_ulpgpio_wkup_handler }, 
    //{ 0, PM_SAMPLE_GPIO_NUM, true, ULP_GPIO_INTERRUPT_FALLING_EDGE, pm_ulpgpio_wkup_handler },    // ulpgpio唤醒
};

// GPIO睡眠配置函数 //
// 在进入睡眠状态时调用此函数：关闭普通GPIO功能，初始化ULP GPIO功能，配置ULP GPIO中断唤醒功能
static void pm_gpio_slp_cfg(void)
{
    uapi_gpio_deinit();
    ulp_gpio_init();
    ulp_gpio_int_wkup_config(g_pm_wk_cfg, sizeof(g_pm_wk_cfg) / sizeof(ulp_gpio_int_wkup_cfg_t));
}

// GPIO唤醒配置函数 //
// 恢复到工作状态时调用此函数：关闭ULP GPIO功能，初始化普通GPIO功能，配置GPIO引脚P为输入模式，上拉电阻，并注册中断处理函数
void pm_gpio_wkup_cfg(void)
{
    ulp_gpio_deinit();
    uapi_gpio_init();
    // 睡眠时GPIO不下电，支持GPIO中断唤醒
    uapi_pin_set_mode(PM_SAMPLE_GPIO_NUM, 0);
    uapi_pin_set_pull(PM_SAMPLE_GPIO_NUM, PIN_PULL_UP);
    uapi_gpio_set_dir(PM_SAMPLE_GPIO_NUM, GPIO_DIRECTION_INPUT);
   // uapi_gpio_register_isr_func(PM_SAMPLE_GPIO_NUM, GPIO_INTERRUPT_RISING_EDGE, (gpio_callback_t)pm_gpio_irq_func);
    uapi_gpio_register_isr_func(PM_SAMPLE_GPIO_NUM, GPIO_INTERRUPT_FALLING_EDGE, (gpio_callback_t)pm_gpio_irq_func);
    osal_printk("GPIO14 level after gpio init: %d\n", uapi_gpio_get_val(RED_LED_PIN));
}
// 状态转换函数 //
// 每个函数都通过osal_irq_lock和osal_irq_restore保护临界区，以防止中断干扰
// 从工作状态切换到待机状态 
static int32_t pm_state_work_to_standby(uintptr_t arg)
{
    unused(arg);
 
    uint32_t irq_status = osal_irq_lock();
    // 连接态：增大连接间隔；广播态：暂不操作。
    g_pm_state = PM_STATE_STANDBY;
    pm_gpio_slp_cfg();
    osal_irq_restore(irq_status);
    osal_printk("work to standby\r\n");
 
    return 0;
}
// 从待机状态切换到睡眠状态
static int32_t pm_state_standby_to_sleep(uintptr_t arg)
{
    unused(arg);

    uint32_t irq_status = osal_irq_lock();
    sle_stop_seek();
    sle_seek_param(40000, 300);
    g_pm_state = PM_STATE_SLEEP;
    osal_irq_restore(irq_status);
    osal_printk("standby to sleep\r\n");
    
    return 0;
}
// 从待机状态切换到工作状态
static int32_t pm_state_standby_to_work(uintptr_t arg)
{
    unused(arg);
    uint32_t irq_status = osal_irq_lock();
    pm_gpio_wkup_cfg();
    rgb_init(); 
    g_pm_state = PM_STATE_WORK;
    osal_irq_restore(irq_status);
    osal_printk("standby to work\r\n");
    return 0;
}
// 从睡眠状态切换到工作状态
static int32_t pm_state_sleep_to_work(uintptr_t arg)
{
    unused(arg);
    uint32_t irq_status = osal_irq_lock();
    pm_gpio_wkup_cfg();
    // 打开广播、蓝牙回连
    rgb_init(); 
    //sle_client_param(100, 100);
    g_pm_state = PM_STATE_WORK;
    osal_irq_restore(irq_status);
    osal_printk("sleep to work\r\n");
    return 0;
}

// 低功耗入口函数 //
// 注册状态转换处理函数，重置工作状态，并设置状态转换持续时间
void pm_low_power_entry(void)
{
    pm_state_trans_handler_t handler = {
        .work_to_standby = pm_state_work_to_standby,
        .standby_to_sleep = pm_state_standby_to_sleep,
        .standby_to_work = pm_state_standby_to_work,
        .sleep_to_work = pm_state_sleep_to_work,
    };
    uapi_pm_state_trans_handler_register(&handler);

    uapi_pm_work_state_reset();
    uapi_pm_set_state_trans_duration(10000, 10000);//系统在工作状态持续10s后自动进入待机,待机状态持续10s后自动进入睡眠
}



