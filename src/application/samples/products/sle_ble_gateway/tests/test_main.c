/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Test runner entry point. \n
 *
 * Build and run on a Linux/macOS/Windows host:
 *
 *   gcc -std=c99 -Wall                           \
 *       -DCONFIG_GW_MAX_CONNECTIONS=8            \
 *       -DCONFIG_GW_DATA_MAX_LEN=256             \
 *       -I.. -Itests                             \
 *       tests/mock_impl.c                        \
 *       tests/test_gateway_service.c             \
 *       tests/test_main.c                        \
 *       gateway_service.c                        \
 *       -o gateway_test
 *
 *   ./gateway_test
 *
 * The '-I..' flag makes the compiler find the real gateway_service.h from
 * the sle_ble_gateway directory, while '-Itests' provides mock headers that
 * shadow the embedded SDK headers (soc_osal.h, securec.h, errcode.h, ...).
 */
#include <stdio.h>
#include "test_utils.h"

int g_test_passed = 0;
int g_test_failed = 0;

extern void run_gateway_service_tests(void);

int main(void)
{
    run_gateway_service_tests();

    printf("\n===========================\n");
    printf("Total: %d  Passed: %d  Failed: %d\n",
           g_test_passed + g_test_failed, g_test_passed, g_test_failed);
    return g_test_failed == 0 ? 0 : 1;
}
