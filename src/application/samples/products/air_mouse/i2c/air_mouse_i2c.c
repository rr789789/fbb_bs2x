/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026-2026. All rights reserved.
 *
 * Description: Air Mouse I2C Source. \n
 *
 * History: \n
 * 2026-04-21, Create file. \n
 */

#include "air_mouse_i2c.h"
#include "i2c.h"
#include "pinctrl.h"
#include "soc_osal.h"

#define WDT_I2C_SCL_PIN S_MGPIO7
#define WDT_I2C_SDA_PIN S_MGPIO8
#define SGM_DEVICE_REG05 0x05
#define SGM41562S_DEV_ADDR 0x03
#define WDT_I2C_REG_NUM_MAX 64

static errcode_t i2c_read_memory(uint8_t dev_addr, uint8_t regaddr, uint8_t *regval, uint8_t reg_num)
{
    i2c_data_t i2c_data;
    uint8_t tx_buffer[1] = {0};

    if (regval == NULL) {
        return ERRCODE_FAIL;
    }

    tx_buffer[0] = regaddr;
    i2c_data.send_buf = tx_buffer;
    i2c_data.send_len = 1;
    i2c_data.receive_buf = regval;
    i2c_data.receive_len = reg_num;

    errcode_t ret = uapi_i2c_master_writeread(I2C_BUS_0, dev_addr, &i2c_data);

    return ret;
}

static errcode_t i2c_write_memory(uint8_t dev_addr, uint8_t regaddr, const uint8_t *regval, uint8_t reg_num)
{
    i2c_data_t i2c_data;
    uint8_t tx_buffer[WDT_I2C_REG_NUM_MAX] = {0};
    uint8_t rd_buffer;

    if (regval == NULL || reg_num > WDT_I2C_REG_NUM_MAX - 1) {
        return ERRCODE_FAIL;
    }

    tx_buffer[0] = regaddr;
    (void)memcpy_s(&tx_buffer[1], reg_num, regval, reg_num);
    i2c_data.send_buf = tx_buffer;
    i2c_data.send_len = 1 + reg_num;
    i2c_data.receive_buf = &rd_buffer;
    i2c_data.receive_len = 0;

    errcode_t ret = uapi_i2c_master_write(I2C_BUS_0, dev_addr, &i2c_data);

    return ret;
}

static errcode_t i2c_init_watchdog(void)
{
    uapi_pin_set_mode(WDT_I2C_SCL_PIN, HAL_PIO_I2C0_CLK);
    uapi_pin_set_mode(WDT_I2C_SDA_PIN, HAL_PIO_I2C0_DATA);

    errcode_t ret = uapi_i2c_master_init(I2C_BUS_0, 400000, 5);

    return ret;
}

void i2c_disable_watchdog(void)
{
    sgm_reg05_t reg05 = {0};

    errcode_t ret = i2c_init_watchdog();
    if (ret != ERRCODE_SUCC) {
        osal_printk("i2c init fail:%x\r\n", ret);
        return; // 初始化失败不执行后续操作
    }
    osal_printk("i2c init success\r\n");

    ret = i2c_read_memory(SGM41562S_DEV_ADDR, SGM_DEVICE_REG05, &reg05.data, 1);
    if (ret != ERRCODE_SUCC) {
        osal_printk("i2c read fail:%x\r\n", ret);
        return;
    }

    reg05.bits.watchdog = 0;
    reg05.bits.en_wd_dischg = false;
    ret = i2c_write_memory(SGM41562S_DEV_ADDR, SGM_DEVICE_REG05, &reg05.data, 1);
    osal_printk("sgm wd cfg:%x\r\n", reg05.data);
}