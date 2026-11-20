/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026-2026. All rights reserved.
 *
 * Description: BLE ota Sample Source. \n
 *
 * History: \n
 * 2026-07-04, Create file. \n
 */
#include "securec.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "bts_gatt_server.h"
#include "ble_ota_server.h"

#define BLE_OTA_BT_STACK_POWER_MS      10000
#define SLE_OTA_TASK_PRIO                  28
#define SLE_OTA_TASK_STACK_SIZE            0x1200

typedef struct {
    uint8_t *value;
    uint16_t value_len;
} msg_data_t;

unsigned int msg_rev_size = sizeof(msg_data_t);


static void *ble_ota_server_task(const char *arg)
{
    unused(arg);
    ble_ota_server_init();
    while (1) {
        osal_msleep(1);
    }
    return NULL;
}


static void ble_ota_entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)ble_ota_server_task, 0, "BLEOtaServerTask",
                                      SLE_OTA_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, SLE_OTA_TASK_PRIO);
    }
    osal_kthread_unlock();
}

/* Run the ble_ota_entry. */
app_run(ble_ota_entry);