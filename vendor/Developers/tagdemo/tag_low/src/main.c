#include "sle_client.h"
#include "lowp.h"
#include "RGB.h"

#define SLE_CLIENT_TASK_PRIO           26
#define SLE_CLIENT_TASK_STACK_SIZE     0x2000
#define RGB_TASK_PRIO                  24
#define RGB_TASK_STACK_SIZE            0x1000 

static void sle_client_entry(void)
{
    osal_task *sle_progress_task_handle = NULL;
    osal_task *rgb_task_handle = NULL;
    sle_client_init();
    sle_start_scan(); 
    rgb_init();  
    pm_low_power_entry();


    osal_kthread_lock();
    sle_progress_task_handle = osal_kthread_create((osal_kthread_handler)sle_progress_task, 0, "sle_progress_task", SLE_CLIENT_TASK_STACK_SIZE);
    if (sle_progress_task_handle != NULL) 
    {
        osal_kthread_set_priority(sle_progress_task_handle, SLE_CLIENT_TASK_PRIO);
        osal_kfree(sle_progress_task_handle);
    }

    rgb_task_handle = osal_kthread_create((osal_kthread_handler)rgb_blinky_task, 0, "RGBBlinkyTask", RGB_TASK_STACK_SIZE);
    if (rgb_task_handle != NULL) 
    {
        osal_kthread_set_priority(rgb_task_handle, RGB_TASK_PRIO);
        osal_printk("RGB blinky task created successfully\r\n");
        osal_kfree(rgb_task_handle);
    }
    osal_kthread_unlock();
}

/* Run the blinky_entry. */
app_run(sle_client_entry);