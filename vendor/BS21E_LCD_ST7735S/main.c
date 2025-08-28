#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "tft.h"
#include "chinese_font.h"
#include "ui.h"
#include "pm_clock.h"
#define MAIN_TASK_PRIO 24
#define MAIN_TASK_STACK_SIZE 0x1000
#define LED_COUNT 9

static int main_task(const char *arg)
{
    unused(arg);
    uapi_clock_control(CLOCK_CONTROL_FREQ_LEVEL_CONFIG, CLOCK_FREQ_LEVEL_HIGH);
    LCD_setup_init();


    while(1)
    {
        screen_home();
        osal_msleep(5000);
        screen_work();
        osal_msleep(5000);
        screen_play();
        osal_msleep(5000);
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

/* Run the blinky_entry. */
app_run(main_entry);