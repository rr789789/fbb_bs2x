#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "tft.h"

#include "pm_clock.h"

// 任务配置常量
#define MAIN_TASK_PRIO 24
#define MAIN_TASK_STACK_SIZE 0x1000

// 颜色定义
#define RED_COLOR 0xF800

// 延时常量
#define DELAY_5S 5000

static int main_task(const char *arg)
{
    unused(arg);
    
    // 设置高频时钟
    uapi_clock_control(CLOCK_CONTROL_FREQ_LEVEL_CONFIG, CLOCK_FREQ_LEVEL_HIGH);
    
    // 初始化LCD
     lcd_setup_init();
    
    // 显示红色
    lcd_display_full(RED_COLOR);
    
    // 主循环 - 保持显示
    while(1)
    {
        osal_msleep(DELAY_5S);
    }
    
    return 0;
}

static void main_entry(void)
{
    osal_task *task_handle = NULL;
    
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)main_task, 0, "MainTask", MAIN_TASK_STACK_SIZE);
    if (task_handle != NULL)
    {
        osal_kthread_set_priority(task_handle, MAIN_TASK_PRIO);
    }
    osal_kthread_unlock();
}

/* Run the main_entry. */
app_run(main_entry);