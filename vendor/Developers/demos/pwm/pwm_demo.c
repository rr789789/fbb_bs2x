/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: PWM Sample Source. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-06-27, Create file. \n
 * 2025-05-28, Modified for beep - on - off effect. \n
 */
#include "pinctrl.h"
#include "pwm.h"
#include "tcxo.h"
#include "soc_osal.h"
#include "app_init.h"

#define TEST_TCXO_DELAY_1000MS     1000 // 延迟时间为1000毫秒
#define BEEP_ON_TIME_MS            500  // 蜂鸣器响的时间(毫秒)
#define BEEP_OFF_TIME_MS           500  // 蜂鸣器停的时间(毫秒)
#define BEEP_CYCLE_COUNT           5    // 蜂鸣器响停循环的次数

#define PWM_TASK_PRIO              24
#define PWM_TASK_STACK_SIZE        0x1000


static errcode_t pwm_sample_callback(uint8_t channel)//当PWM周期完成时被调用
{
    osal_printk("PWM %d, cycle done. \r\n", channel);
    return ERRCODE_SUCC;
}

// 全局变量，用于控制PWM状态
static bool pwm_initialized = false;
static bool pwm_running = false;

// static void pwm_init(void)
// {
void pwm_init()
{
    pwm_config_t cfg_no_repeat = {
        20000,
        20000,
        0,
        0xFF,
        true
    };

    uapi_pin_set_mode(CONFIG_PWM_PIN, CONFIG_PWM_PIN_MODE); // 设置PWM引脚的模式
    uapi_pwm_deinit(); // 去初始化pwm
    uapi_pwm_init(); // 初始化pwm
    uapi_pwm_open(CONFIG_PWM_CHANNEL, &cfg_no_repeat); // 打开 PWM 通道，并应用配置

    uapi_pwm_unregister_interrupt(CONFIG_PWM_CHANNEL); // 去 PWM 注册中断回调
    uapi_pwm_register_interrupt(CONFIG_PWM_CHANNEL, pwm_sample_callback); // 为PWM注册中断回调

    pwm_initialized = true;
}

// 开始响
void pwm_beep_start(void)
{
    if (!pwm_initialized)
    {
        pwm_init("");
        osal_printk("PWM not initialized!\r\n");
        return;
    }

    if (!pwm_running)
    {
#ifdef CONFIG_PWM_USING_V151
        uint8_t channel_id = CONFIG_PWM_CHANNEL;
        uapi_pwm_set_group(CONFIG_PWM_GROUP_ID, &channel_id, 1);
        uapi_pwm_start_group(CONFIG_PWM_GROUP_ID); // 启动PWM
#else
        uapi_pwm_start(CONFIG_PWM_CHANNEL); // 启动PWM
#endif
        pwm_running = true;
        osal_printk("Beep ON\r\n");
    }
}

// 停止响
void pwm_beep_stop(void)
{
    if (pwm_running)
    {
#ifdef CONFIG_PWM_USING_V151
        uapi_pwm_stop_group(CONFIG_PWM_GROUP_ID); // 停止PWM
#else
        uapi_pwm_stop(CONFIG_PWM_CHANNEL); // 停止PWM
#endif
        pwm_running = false;
        osal_printk("Beep OFF\r\n");
    }
    osal_printk("PWM stopped successfully\r\n");
}


void set_pwm_beep_frequency(int time)
{ 
    
    while (time--)
    {
        pwm_beep_start();
        osal_msleep(TEST_TCXO_DELAY_1000MS);
        pwm_beep_stop();
        osal_msleep(TEST_TCXO_DELAY_1000MS);
    }
}

static void pwm_entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)set_pwm_beep_frequency, 0, "PwmTask", PWM_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, PWM_TASK_PRIO);
    }
    osal_kthread_unlock();
}

/* Run the pwm_entry. */
app_run(pwm_entry);