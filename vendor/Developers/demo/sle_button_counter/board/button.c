/**
 * Copyright (c) EndyTsang 2025. All rights reserved. \n
 *
 * Description: Button Task. \n
 * 
 */

#include "button.h"

bool buttonPressed = false;

static void gpio_callback_func(pin_t pin, uintptr_t param)
{
    UNUSED(pin);
    UNUSED(param);
    buttonPressed = true;
    printf("Button pressed.\r\n");
}

static void *button_task(const char *arg)
{
    unused(arg);
    uapi_pin_set_mode(BUTTON_GPIO, HAL_PIO_FUNC_GPIO);
    uapi_pin_set_pull(BUTTON_GPIO, (pin_pull_t)HAL_PIO_PULL_UP);
    uapi_gpio_set_dir(BUTTON_GPIO, GPIO_DIRECTION_INPUT);
    errcode_t ret = uapi_gpio_register_isr_func(BUTTON_GPIO, GPIO_INTERRUPT_RISING_EDGE, gpio_callback_func);
    if (ret != 0) {
        printf("uapi_gpio_unregister_isr_func fail\r\n");
        uapi_gpio_unregister_isr_func(BUTTON_GPIO);
    }
    while (1) {
        osal_msleep(500); // 延时500毫秒
    }
    return NULL;
}

static void button_entry(void)
{
    uint32_t ret;
    osal_task *taskid;
    // 创建任务调度
    osal_kthread_lock();
    // 创建任务1
    taskid = osal_kthread_create((osal_kthread_handler)button_task, NULL, "button_task", BUTTON_TASK_STACK_SIZE);
    ret = osal_kthread_set_priority(taskid, BUTTON_TASK_PRIO);
    if (ret != OSAL_SUCCESS) {
        printf("create task1 failed .\n");
    }
    osal_kthread_unlock();
}

/* Run the blinky_entry. */
app_run(button_entry);