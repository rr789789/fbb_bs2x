/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: Board UART port\n
 *
 * History: \n
 * 2023-03-25, Create file. \n
 */
#include "tiot_board_uart_port.h"
#include "tiot_board_log.h"
#include "uart.h"
#include "uart_porting.h"
#include "pinctrl.h"
#include "pinctrl_porting.h"
#include "tiot_board_uart_port_config.h"
#include "tiot_board_misc_port.h"
#include "gpio.h"
#ifdef CONFIG_XMIT_WITH_DMA
#include "tiot_osal.h"
#include "hal_dma.h"
#include "dma.h"
#include "dma_porting.h"
#endif
#define UART_RW_TIMEOUT    0x5000
#define UART_RX_BUFF_MAX   1024
#define UART_WAIT_PENDING_MAX_TIMES 2000

#ifdef CONFIG_XMIT_WITH_RX_DMA
#define UART_DATA_FIFO_OFFSET_ADDR         0x4

typedef struct {
    uart_bus_t bus;
    dma_channel_t rx_dma_channel;
    dma_ch_user_peripheral_config_t rx_dma_config;
} uart_dma_lli_ch_t;
 
uart_dma_lli_ch_t g_uart_lli_cfg[UART_BUS_MAX_NUMBER] = {
    {UART_BUS_MAX_NUMBER,  DMA_CHANNEL_NONE, { 0 }},
#if UART_BUS_MAX_NUM > 1
    {UART_BUS_MAX_NUMBER,  DMA_CHANNEL_NONE, { 0 }},
#endif
#if UART_BUS_MAX_NUM > 2
    {UART_BUS_MAX_NUMBER,  DMA_CHANNEL_NONE, { 0 }}
#endif
};
 
static int32_t uart_dma_read(uart_bus_t bus, uart_dma_lli_ch_t *dma_lli, uint8_t *buff, uint16_t len);
static void uart_dma_lli_config(uart_bus_t bus, uart_dma_lli_ch_t *dma_lli);
#endif

static void board_uart_txrx_pinmux_init(tiot_board_uart_port *port)
{
    /* 防止板级未初始化. */
    uapi_pin_set_mode(port->pin_config.tx_pin, (pin_mode_t)port->pinmux[UART_PIN_TYPE_TX]);
    uapi_pin_set_pull(port->pin_config.tx_pin, PIN_PULL_UP);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    /* bs2x ie默认关闭，uart rx使能时需要重新配置 */
    uapi_pin_set_ie(port->pin_config.rx_pin, PIN_IE_1);
#endif
    uapi_pin_set_mode(port->pin_config.rx_pin, (pin_mode_t)port->pinmux[UART_PIN_TYPE_RX]);
    /* 取消rx管脚下拉配置，否则漏电 */
    uapi_pin_set_pull(port->pin_config.rx_pin, PIN_PULL_UP);
}

static void board_uart_switch_flowctrl(tiot_board_uart_port *port)
{
    /* 板级uart初始化不会初始化ctsrts pinmux... */
    if (port->flow_ctrl_bk == TIOT_UART_ATTR_FLOW_CTRL_ENABLE) {
        port->pin_config.cts_pin = port->pin_config_orig.cts_pin;
        port->pin_config.rts_pin = port->pin_config_orig.rts_pin;
        /* 取消cts管脚下拉配置，否则漏电 */
        uapi_pin_set_pull(port->pin_config.cts_pin, PIN_PULL_NONE);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    /* bs2x ie默认关闭，uart ctx使能时需要重新配置 */
    uapi_pin_set_ie(port->pin_config.cts_pin, PIN_IE_1);
#endif
        uapi_pin_set_mode(port->pin_config.cts_pin, (pin_mode_t)port->pinmux[UART_PIN_TYPE_CTS]);
        uapi_pin_set_mode(port->pin_config.rts_pin, (pin_mode_t)port->pinmux[UART_PIN_TYPE_RTS]);
    } else {
        port->pin_config.cts_pin = PIN_NONE;
        port->pin_config.rts_pin = PIN_NONE;
    }
}

static int32_t board_uart_reopen(uart_bus_t bus, tiot_board_uart_port *port, const uart_attr_t *attr)
{
    uart_buffer_config_t buff_cfg = { (uint8_t *)port->rx_buff, UART_RX_BUFFER_SIZE };
    (void)uapi_uart_deinit(bus);
    board_uart_txrx_pinmux_init(port);
    board_uart_switch_flowctrl(port);
    errcode_t ret = uapi_uart_init(bus, &(port->pin_config), attr, port->extra_attr, &buff_cfg);
    if (ret != ERRCODE_SUCC) {
        return -1;
    }
#if !defined (CONFIG_XMIT_WITH_RX_DMA)
    uapi_uart_register_rx_callback(bus, UART_RX_CONDITION_FULL_OR_SUFFICIENT_DATA_OR_IDLE,
                                   UART_RX_BUFF_MAX, port->rx_callback);
#endif
    return 0;
}

int32_t tiot_board_uart_open(tiot_xmit *xmit, tiot_xmit_callbacks *cbs)
{
    enum tiot_uart_port port_type = board_uart_bus_convert_port((uart_bus_t)xmit->id);
    if (port_type == TIOT_UART_NONE) {
        return -1;
    }

#if defined SUPPORT_UART_TYPE_CHOOSE && SUPPORT_UART_TYPE_CHOOSE == 1
    tiot_board_uart_port *port;
    if (HAL_GetUartNum() == HAL_UART_0) {
        port = &g_tiot_board_uart_port[port_type];
    } else {
        port = &g_tiot_board_uart_port_flowctrl[port_type];
    }
#else
    tiot_board_uart_port *port = &g_tiot_board_uart_port[port_type];
#endif
    const uart_attr_t init_attr = { 115200, UART_DATA_BIT_8, UART_STOP_BIT_1, UART_PARITY_NONE,
                                    ((port->flow_ctrl_bk == TIOT_UART_ATTR_FLOW_CTRL_ENABLE) ?
                                    UART_FLOW_CTRL_RTS_CTS : UART_FLOW_CTRL_NONE) };
    port->pin_config.tx_pin = port->pin_config_orig.tx_pin;
    port->pin_config.rx_pin = port->pin_config_orig.rx_pin;
    if (board_uart_reopen((uart_bus_t)xmit->id, port, &init_attr) != 0) {
        return -1;
    }
    g_tiot_xmit_cbs = *cbs;
    port->xmit = xmit;
    xmit->handle = port;
    return 0;
}

#ifdef GPIO_SET_DIR_INPUT
static void board_set_uart_pin_dir(uart_bus_t bus, gpio_direction_t dir)
{
    enum tiot_uart_port port_type = board_uart_bus_convert_port(bus);
#if defined SUPPORT_UART_TYPE_CHOOSE && SUPPORT_UART_TYPE_CHOOSE == 1
    tiot_board_uart_port *port;
    if (HAL_GetUartNum() == HAL_UART_0) {
        port = &g_tiot_board_uart_port[port_type];
    } else {
        port = &g_tiot_board_uart_port_flowctrl[port_type];
    }
#else
    tiot_board_uart_port *port = &g_tiot_board_uart_port[port_type];
#endif

    (void)uapi_pin_set_mode((pin_t)port->pin_config_orig.tx_pin, (pin_mode_t)HAL_PIO_FUNC_GPIO);
    (void)uapi_gpio_set_dir((pin_t)port->pin_config_orig.tx_pin, dir);
    (void)uapi_pin_set_pull((pin_t)port->pin_config_orig.tx_pin, PIN_PULL_NONE);

    (void)uapi_pin_set_mode((pin_t)port->pin_config_orig.rx_pin, (pin_mode_t)HAL_PIO_FUNC_GPIO);
    (void)uapi_gpio_set_dir((pin_t)port->pin_config_orig.rx_pin, dir);
    (void)uapi_pin_set_pull((pin_t)port->pin_config_orig.rx_pin, PIN_PULL_NONE);

#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    uapi_pin_set_ie(port->pin_config.rx_pin, PIN_IE_0);
#endif
    if (port->flow_ctrl_bk == TIOT_UART_ATTR_FLOW_CTRL_ENABLE) {
        (void)uapi_pin_set_mode((pin_t)port->pin_config_orig.rts_pin, (pin_mode_t)HAL_PIO_FUNC_GPIO);
        (void)uapi_gpio_set_dir((pin_t)port->pin_config_orig.rts_pin, dir);

        (void)uapi_pin_set_mode((pin_t)port->pin_config_orig.cts_pin, (pin_mode_t)HAL_PIO_FUNC_GPIO);
        (void)uapi_gpio_set_dir((pin_t)port->pin_config_orig.cts_pin, dir);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
        uapi_pin_set_ie(port->pin_config.cts_pin, PIN_IE_0);
#endif
    }
}
#endif

void tiot_board_uart_close(tiot_xmit *xmit)
{
    uapi_uart_unregister_rx_callback(xmit->id);
    (void)uapi_uart_deinit(xmit->id);
#ifdef GPIO_SET_DIR_INPUT
    /* 将uart的相关管脚配置成输入高阻 */
    board_set_uart_pin_dir(xmit->id, GPIO_DIRECTION_INPUT);
#endif
}

int32_t tiot_board_uart_write(tiot_xmit *xmit, const uint8_t *buff, uint32_t len)
{
#ifdef CONFIG_XMIT_WITH_TX_DMA
    uart_write_dma_config_t dma_cfg = {
        .src_width = HAL_DMA_TRANSFER_WIDTH_8,
        .dest_width = HAL_DMA_TRANSFER_WIDTH_8,
        .burst_length = HAL_DMA_BURST_TRANSACTION_LENGTH_1,
        .priority = HAL_DMA_CH_PRIORITY_0
    };
    return uapi_uart_write_by_dma(xmit->id, buff, len, &dma_cfg);
#else
    return uapi_uart_write_nolock(xmit->id, buff, len, UART_RW_TIMEOUT);
#endif
}

int32_t tiot_board_uart_read(tiot_xmit *xmit, uint8_t *buff, uint32_t len)
{
#ifdef CONFIG_XMIT_WITH_RX_DMA
    enum tiot_uart_port port_type;
    /* 入参buff为空，dma目的地址默认选择uart rx buffer */
    if (buff == NULL) {
        port_type = board_uart_bus_convert_port((uart_bus_t)xmit->id);
        return uart_dma_read(xmit->id, g_uart_lli_cfg, g_tiot_board_uart_port[port_type].rx_buff, (uint16_t)len);
    }
    return uart_dma_read(xmit->id, g_uart_lli_cfg, buff, (uint16_t)len);
#else
    return uapi_uart_read(xmit->id, buff, len, UART_RW_TIMEOUT);
#endif
}

int32_t tiot_board_uart_set_config(tiot_xmit *xmit, tiot_uart_config *config)
{
    errcode_t ret;
    uint32_t loop = 0;
    tiot_board_uart_port *port = xmit->handle;
    if ((config == NULL) || (config->attr.data_bits >= TIOT_UART_ATTR_DATABIT_BUTT) ||
        (config->attr.stop_bits >= TIOT_UART_ATTR_STOPBIT_BUTT) ||
        (config->attr.parity >= TIOT_UART_ATTR_PARITY_BUTT)) {
        return -1;
    }
    if (port == NULL) {
        return -1;
    }
    /* 确保当前无数据正在FIFO内传输，才能修改配置. */
    while (uapi_uart_has_pending_transmissions(xmit->id) == true) {
        loop++;
        tiot_board_udelay(10); /* 10us */
        if (loop > UART_WAIT_PENDING_MAX_TIMES) { // 超时时间20ms (10us * UART_WAIT_PENDING_MAX_TIMES)
            tiot_print_err("tiot_board_uart_set_config wait timeout.\r\n");
            return -1;
        }
    }

    const uart_attr_t attr = { config->baudrate, UART_DATA_BIT_8, UART_STOP_BIT_1, UART_PARITY_NONE,
                               ((config->attr.flow_ctrl == TIOT_UART_ATTR_FLOW_CTRL_ENABLE) ?
                               UART_FLOW_CTRL_RTS_CTS : UART_FLOW_CTRL_NONE) };
    /* 调整流控需要重新init. */
    if (config->attr.flow_ctrl != port->flow_ctrl_bk) {
        port->flow_ctrl_bk = config->attr.flow_ctrl;
        if (board_uart_reopen((uart_bus_t)xmit->id, port, &attr) != 0) {
            return -1;
        }
    }
    ret = uapi_uart_set_attr(xmit->id, &attr);
#ifdef CONFIG_XMIT_WITH_RX_DMA
    /* dma config init */
    uart_dma_lli_config(xmit->id, g_uart_lli_cfg);
#endif
    return (ret == ERRCODE_SUCC) ? 0 : -1;
}

#ifdef CONFIG_XMIT_WITH_RX_DMA
static void uart_dma_lli_recv_cb(uint8_t intr, uint8_t channel, uintptr_t arg)
{
    unused(intr & arg);
    /* 获取当前channel对应的UART bus */
    uint8_t cur_bus = 0;
    enum tiot_uart_port port_type = 0;
    while (cur_bus < UART_BUS_MAX_NUMBER) {
        if (g_uart_lli_cfg[cur_bus].rx_dma_channel == channel) { break; }
        cur_bus++;
    }
    if (cur_bus >= UART_BUS_MAX_NUMBER) {
        tiot_print_err("rx callback error : invalid uart bus\r\n");
        uapi_dma_end_transfer(channel);
        return;
    }
    port_type = board_uart_bus_convert_port(cur_bus);
    uapi_dma_end_transfer(channel);
    g_uart_lli_cfg[cur_bus].rx_dma_channel = DMA_CHANNEL_NONE;
    g_tiot_board_uart_port[port_type].rx_callback(g_tiot_board_uart_port[port_type].rx_buff,
                                                  g_uart_lli_cfg[cur_bus].rx_dma_config.transfer_num, 0);
}
 
static void uart_dma_lli_config(uart_bus_t bus, uart_dma_lli_ch_t *dma_lli)
{
    dma_lli[bus].bus = bus;
    /* rx dma */
    dma_lli[bus].rx_dma_config.src = uart_porting_base_addr_get(bus) + UART_DATA_FIFO_OFFSET_ADDR;
    dma_lli[bus].rx_dma_config.src_handshaking = uart_port_get_dma_trans_src_handshaking(bus);
    dma_lli[bus].rx_dma_config.trans_type = HAL_DMA_TRANS_PERIPHERAL_TO_MEMORY_DMA;
    dma_lli[bus].rx_dma_config.trans_dir = HAL_DMA_TRANSFER_DIR_PERIPHERAL_TO_MEM;
    dma_lli[bus].rx_dma_config.src_increment = HAL_DMA_ADDRESS_INC_NO_CHANGE;
    dma_lli[bus].rx_dma_config.dest_increment = HAL_DMA_ADDRESS_INC_INCREMENT;
    dma_lli[bus].rx_dma_config.src_width = HAL_DMA_TRANSFER_WIDTH_8;
    dma_lli[bus].rx_dma_config.dest_width = HAL_DMA_TRANSFER_WIDTH_8;
    dma_lli[bus].rx_dma_config.priority = HAL_DMA_CH_PRIORITY_1;
    dma_lli[bus].rx_dma_config.burst_length = HAL_DMA_BURST_TRANSACTION_LENGTH_1;
    dma_lli[bus].rx_dma_config.protection = HAL_DMA_PROTECTION_CONTROL_NONE;
    dma_lli[bus].rx_dma_config.dest_handshaking = 0;
}

static void uart_dma_lli_rx_config(uart_dma_lli_ch_t *dma_lli, const void *buff, uint16_t len)
{
    uint16_t rev_len = len > UART_RX_BUFFER_SIZE ? UART_RX_BUFFER_SIZE :len;
    dma_lli->rx_dma_config.dest = (uint32_t)(uintptr_t)buff;
    dma_lli->rx_dma_channel = (dma_channel_t)uapi_dma_get_lli_channel(HAL_DMA_BURST_TRANSACTION_LENGTH_1,
        uart_port_get_dma_trans_src_handshaking(dma_lli->bus));
    dma_lli->rx_dma_config.transfer_num = (uint16_t)(rev_len >> dma_lli->rx_dma_config.src_width);
}

static int32_t uart_enable_dma_lli(uint8_t channel, dma_transfer_cb_t callback)
{
    errcode_t ret = ERRCODE_FAIL;
    uint32_t lock = osal_irq_lock();
    ret = uapi_dma_enable_lli(channel, callback, (uintptr_t)NULL);
    if (ret != ERRCODE_SUCC) {
        tiot_print_err("dma enable faile %x, channel %d\n", ret, channel);
    }
    osal_irq_restore(lock);
    return ret;
}

static int32_t uart_dma_read(uart_bus_t bus, uart_dma_lli_ch_t *dma_lli, uint8_t *buff, uint16_t len)
{
    errcode_t ret = ERRCODE_FAIL;
    uint32_t lock = osal_irq_lock();
    uapi_dma_end_transfer(dma_lli[bus].rx_dma_channel);
    uart_dma_lli_rx_config(&dma_lli[bus], buff, len);
    ret = uapi_dma_configure_peripheral_transfer_lli(dma_lli[bus].rx_dma_channel,
                                                     &(dma_lli[bus].rx_dma_config), NULL);
    if (ret != ERRCODE_SUCC) {
        tiot_print_err("uart rx dma config peripheral transfer lli fail, ret:%x!\r\n", ret);
    }
    /* rx dma使能 */
    ret = uart_enable_dma_lli(dma_lli[bus].rx_dma_channel, uart_dma_lli_recv_cb);
    osal_irq_restore(lock);
    return ret;
}
#endif