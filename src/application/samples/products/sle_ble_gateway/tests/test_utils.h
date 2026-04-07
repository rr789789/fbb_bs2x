/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Minimal unit-test utility macros. \n
 *
 * Compile gateway_service.c for PC with:
 *   gcc -DCONFIG_GW_MAX_CONNECTIONS=8 -DCONFIG_GW_DATA_MAX_LEN=256 \
 *       -I. -Itests                                                   \
 *       tests/mock_impl.c tests/test_gateway_service.c tests/test_main.c \
 *       gateway_service.c                                              \
 *       -o gateway_test && ./gateway_test
 */
#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>

extern int g_test_passed;
extern int g_test_failed;

#define TEST_ASSERT(cond) do {                                               \
    if (!(cond)) {                                                            \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond);            \
        g_test_failed++;                                                      \
    } else {                                                                  \
        g_test_passed++;                                                      \
    }                                                                         \
} while (0)

#define TEST_RUN(fn) do {                                                     \
    printf("%-50s ", #fn);                                                    \
    int _before_fail = g_test_failed;                                         \
    fn();                                                                     \
    if (g_test_failed == _before_fail) {                                      \
        printf("ok\n");                                                       \
    } else {                                                                  \
        printf("FAILED\n");                                                   \
    }                                                                         \
} while (0)

#endif /* TEST_UTILS_H */
