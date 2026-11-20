/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026-2026. All rights reserved.
 *
 * Description: Air Mouse I2C Header. \n
 *
 * History: \n
 * 2026-04-21, Create file. \n
 */
#ifndef AIR_MOUSE_I2C_H
#define AIR_MOUSE_I2C_H

#include <stdint.h>
#include "slp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef union {
    uint8_t data;
    struct {
        uint8_t term_tmr      : 1;
        uint8_t chg_tmr       : 2;
        uint8_t en_timer      : 1;
        uint8_t en_term       : 1;
        uint8_t watchdog      : 2;
        uint8_t en_wd_dischg  : 1;
    } bits;
} sgm_reg05_t;

void i2c_disable_watchdog(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
