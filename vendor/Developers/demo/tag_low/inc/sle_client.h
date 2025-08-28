/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 *
 * Description: SLE private service register sample of client.
 */

/**
 * @defgroup SLE UUID CLIENT API
 * @ingroup
 * @{
 */

#ifndef SLE_CLIENT_H
#define SLE_CLIENT_H

#include "app_init.h"
#include "systick.h"
#include "tcxo.h"
#include "los_memory.h"
#include "securec.h"
#include "soc_osal.h"
#include "common_def.h"
#include "sle_device_discovery.h"
#include "sle_device_manager.h"
#include "sle_connection_manager.h"
#include "sle_ssap_client.h"
#include "watchdog.h" 
#include "pm_sys.h"

#define SLE_MTU_SIZE_DEFAULT        1500
#define SLE_SEEK_INTERVAL_DEFAULT   2000 //50*125ms
#define SLE_SEEK_WINDOW_DEFAULT     100
#define SLE_RX_BUF_LEN 256
#define XZERO        0
#define XONE         1
#define XSECOND      2
#define XTHREE       3
#define XSIX         6
#define OSL100       100
#define OSL50        50

void sle_client_init(void);
void sle_start_scan(void);
void sle_seek_param(uint16_t seek_interval, uint16_t seek_window);

#endif