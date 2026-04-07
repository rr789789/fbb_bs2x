/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE-BLE Gateway entry point -- creates the gateway task. \n
 */
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "pm_clock.h"
#include "gateway_service.h"
#include "gw_sle_adapter.h"
#include "gw_ble_adapter.h"
#include "gateway_demo.h"

#define GW_ENTRY_LOG        "[gw_entry] "
#define GW_TASK_STACK_SIZE  0x1800
#define GW_TASK_PRIO        24

static void *gateway_task(const char *arg)
{
    unused(arg);

    /* Initialize service layer with demo callbacks */
    if (gw_service_init(gateway_demo_on_connect, gateway_demo_on_disconnect) != ERRCODE_SUCC) {
        osal_printk("%s service init fail\r\n", GW_ENTRY_LOG);
        return NULL;
    }

    /* Initialize SLE adapter (register callbacks + start advertising) */
    if (gw_sle_adapter_init() != ERRCODE_SUCC) {
        osal_printk("%s sle adapter init fail\r\n", GW_ENTRY_LOG);
        return NULL;
    }

    /* Initialize BLE adapter (enable BLE + register GATT server + start advertising) */
    if (gw_ble_adapter_init() != ERRCODE_SUCC) {
        osal_printk("%s ble adapter init fail\r\n", GW_ENTRY_LOG);
        return NULL;
    }

    osal_printk("%s gateway running\r\n", GW_ENTRY_LOG);

    /* Main event loop -- blocks on message queue */
    while (1) {
        gw_service_process_events();
    }
    return NULL;
}

static void gateway_entry(void)
{
    if (uapi_clock_control(CLOCK_CONTROL_FREQ_LEVEL_CONFIG, CLOCK_FREQ_LEVEL_HIGH) == ERRCODE_SUCC) {
        osal_printk("%s clock config ok\r\n", GW_ENTRY_LOG);
    }

    osal_kthread_lock();
    /* Register SLE device-manager callbacks (power-on / enable) before task creation */
    gw_sle_dev_register_cbks();

    osal_task *task_handle = osal_kthread_create((osal_kthread_handler)gateway_task,
                                                  NULL, "GatewayTask", GW_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, GW_TASK_PRIO);
    } else {
        osal_printk("%s create task fail\r\n", GW_ENTRY_LOG);
    }
    osal_kthread_unlock();
}

app_run(gateway_entry);
