/**
 * 
 *
 * 功能：控制TH10S Modbus温湿度读取器读取温湿度数据 ，发送 Modbus 命令，接收并解析 TH10S 温湿度数据。
 *
 * 修改记录：
 * 2023-06-29, 初版创建。
 * 2025-08-28, 使用星闪模组，优化接受
 */
#include "../../include/driver/pinctrl.h"
#include "../../include/driver/uart.h"
#include "../../include/driver/watchdog.h"
#include "soc_osal.h"
#include "app_init.h"
#include <string.h>

// 禁用 DMA 和中断模式，纯轮询更稳
#undef CONFIG_UART_SUPPORT_DMA
#undef CONFIG_UART_SUPPORT_INT_MODE

// UART 配置参数
#define UART_BAUDRATE                      9600           // 波特率 9600
#define UART_COMMAND_INTERVAL_MS           1000           // 每秒发一次，TH11S 推荐
#define UART_RESPONSE_TIMEOUT_MS           5000           // 接收超时 5 秒
#define UART_TASK_PRIO                     24             // 任务优先级
#define UART_TASK_STACK_SIZE               0x2000         // 栈大小 8KB
#define UART_RX_BUFFER_SIZE                256            // 接收缓冲区，够大才稳

// 引脚配置
#undef CONFIG_UART_H0_TX_PIN
#undef CONFIG_UART_L0_RX_PIN
#define CONFIG_UART_H0_TX_PIN              19             // TX 引脚
#define CONFIG_UART_L0_RX_PIN              20             // RX 引脚
#define CONFIG_UART_TXD_PIN_MODE           PIN_MODE_0     // 引脚模式
#define CONFIG_UART_RXD_PIN_MODE           PIN_MODE_0     // 引脚模式
#define CONFIG_UART_BUS_ID                 0              // UART0

// Modbus 协议相关定义
#define MODBUS_SLAVE_ADDR                  0x01           // 从机地址
#define MODBUS_FUNC_READ_INPUT_REG         0x04           // 功能码：读输入寄存器
#define MODBUS_START_ADDR_HIGH             0x00           // 起始地址高字节
#define MODBUS_START_ADDR_LOW              0x00           // 起始地址低字节
#define MODBUS_REG_COUNT_HIGH              0x00           // 寄存器数量高字节
#define MODBUS_REG_COUNT_LOW               0x02           // 寄存器数量低字节（2个寄存器）
#define MODBUS_CRC_LOW                     0x71           // CRC校验低字节
#define MODBUS_CRC_HIGH                    0xCB           // CRC校验高字节

// Modbus 命令：读取温湿度数据
static const uint8_t g_uart_command[] = {
    MODBUS_SLAVE_ADDR,
    MODBUS_FUNC_READ_INPUT_REG,
    MODBUS_START_ADDR_HIGH,
    MODBUS_START_ADDR_LOW,
    MODBUS_REG_COUNT_HIGH,
    MODBUS_REG_COUNT_LOW,
    MODBUS_CRC_LOW,
    MODBUS_CRC_HIGH
};
#define UART_COMMAND_SIZE (sizeof(g_uart_command) / sizeof(g_uart_command[0]))

// Modbus 响应格式定义
#define MODBUS_RESP_MIN_LEN                9              // 最小响应长度
#define MODBUS_RESP_SLAVE_ADDR_IDX         0              // 从机地址索引
#define MODBUS_RESP_FUNC_CODE_IDX          1              // 功能码索引
#define MODBUS_RESP_BYTE_COUNT_IDX         2              // 字节数索引
#define MODBUS_RESP_TEMP_HIGH_IDX          3              // 温度高字节索引
#define MODBUS_RESP_TEMP_LOW_IDX           4              // 温度低字节索引
#define MODBUS_RESP_HUM_HIGH_IDX           5              // 湿度高字节索引
#define MODBUS_RESP_HUM_LOW_IDX            6              // 湿度低字节索引
#define MODBUS_RESP_EXPECTED_BYTE_COUNT    0x04           // 预期字节数（4字节数据）

// 数据转换相关定义
#define TEMP_HUM_SCALE_FACTOR              10.0f          // 温湿度缩放因子（除以10）
#define BYTE_SHIFT_8                       8              // 字节移位数

static uint8_t g_app_uart_rx_buff[UART_RX_BUFFER_SIZE] = { 0 };
static uart_buffer_config_t g_app_uart_buffer_config = {
    .rx_buffer = g_app_uart_rx_buff,
    .rx_buffer_size = UART_RX_BUFFER_SIZE
};

// 配置 UART 引脚
static void app_uart_init_pin(void)
{
    osal_printk("配置 UART 引脚：TX=%d, RX=%d\n", CONFIG_UART_H0_TX_PIN, CONFIG_UART_L0_RX_PIN);
#if defined(CONFIG_PINCTRL_SUPPORT_IE)
    if (uapi_pin_set_ie(CONFIG_UART_L0_RX_PIN, PIN_IE_1) != ERRCODE_SUCC) {
        osal_printk("RX 引脚 %d 输入使能设置失败！\n", CONFIG_UART_L0_RX_PIN);
        return;
    }
#endif
    if (uapi_pin_set_mode(CONFIG_UART_H0_TX_PIN, CONFIG_UART_TXD_PIN_MODE) != ERRCODE_SUCC) {
        osal_printk("TX 引脚 %d 模式设置失败！\n", CONFIG_UART_H0_TX_PIN);
        return;
    }
    if (uapi_pin_set_mode(CONFIG_UART_L0_RX_PIN, CONFIG_UART_RXD_PIN_MODE) != ERRCODE_SUCC) {
        osal_printk("RX 引脚 %d 模式设置失败！\n", CONFIG_UART_L0_RX_PIN);
        return;
    }
    osal_printk("UART 引脚配置成功！\n");
}

// 初始化 UART 参数
static void app_uart_init_config(void)
{
    uart_attr_t attr = {
        .baud_rate = UART_BAUDRATE,
        .data_bits = UART_DATA_BIT_8,
        .stop_bits = UART_STOP_BIT_1,
        .parity = UART_PARITY_NONE
    };

    uart_pin_config_t pin_config = {
        .tx_pin = CONFIG_UART_H0_TX_PIN,
        .rx_pin = CONFIG_UART_L0_RX_PIN,
        .cts_pin = PIN_NONE,
        .rts_pin = PIN_NONE
    };

    osal_printk("开始初始化 UART%d\n", CONFIG_UART_BUS_ID);
    uapi_uart_deinit(CONFIG_UART_BUS_ID);
    if (uapi_uart_init(CONFIG_UART_BUS_ID, &pin_config, &attr, NULL, &g_app_uart_buffer_config) != ERRCODE_SUCC) {
        osal_printk("UART%d 初始化失败！\n", CONFIG_UART_BUS_ID);
        return;
    }
    osal_printk("UART%d 初始化成功！波特率=%d, TX=%d, RX=%d\n",
                CONFIG_UART_BUS_ID, UART_BAUDRATE, CONFIG_UART_H0_TX_PIN, CONFIG_UART_L0_RX_PIN);
}

// 解析 Modbus 响应，提取温湿度
/**
 * 
 * 注意：每一个数据用两个字节整数表示，高位在前，低位在后
如： 带符号整数 范围 -32768---32767
	上传数据需除十，正数的范围为16进制0X0000-0X7FFF，负数采用正数的补码方式传输,其范围为16进制0X8000-0XFFFF，
如:
湿度上传16进制 0X0311，对应十进制785，表示78.5%
温度上传16进制 0X00FF，对应十进制255，表示25.5℃
温度上传16进制 0XFF9B，对应十进制101(取反加1后0X65，实际解析不用转换，直接赋值即可), 表示-10.1℃
 */
static void parse_modbus_response(uint8_t *buffer, uint32_t length)
{
    // 预期响应：01 04 04 [温度H] [温度L] [湿度H] [湿度L] [CRCH] [CRCL]
    if (length < MODBUS_RESP_MIN_LEN || 
        buffer[MODBUS_RESP_SLAVE_ADDR_IDX] != MODBUS_SLAVE_ADDR || 
        buffer[MODBUS_RESP_FUNC_CODE_IDX] != MODBUS_FUNC_READ_INPUT_REG || 
        buffer[MODBUS_RESP_BYTE_COUNT_IDX] != MODBUS_RESP_EXPECTED_BYTE_COUNT) {
        osal_printk("UART%d 数据格式不对！长度=%d, [0]=0x%02X, [1]=0x%02X, [2]=0x%02X\n",
                    CONFIG_UART_BUS_ID, length, 
                    buffer[MODBUS_RESP_SLAVE_ADDR_IDX], 
                    buffer[MODBUS_RESP_FUNC_CODE_IDX], 
                    buffer[MODBUS_RESP_BYTE_COUNT_IDX]);
        return;
    }

    // 提取温度和湿度，2 字节有符号整数，高位在前
    int16_t temp_raw = (buffer[MODBUS_RESP_TEMP_HIGH_IDX] << BYTE_SHIFT_8) | buffer[MODBUS_RESP_TEMP_LOW_IDX];
    int16_t hum_raw = (buffer[MODBUS_RESP_HUM_HIGH_IDX] << BYTE_SHIFT_8) | buffer[MODBUS_RESP_HUM_LOW_IDX];

    // 除以缩放因子得到实际值
    float temperature = (float)temp_raw / TEMP_HUM_SCALE_FACTOR; // 单位：℃
    float humidity = (float)hum_raw / TEMP_HUM_SCALE_FACTOR;     // 单位：%

    // 打印原始和解析结果
    osal_printk("UART%d 原始数据：温度=0x%04X (%d), 湿度=0x%04X (%d)\n",
                CONFIG_UART_BUS_ID, temp_raw & 0xFFFF, temp_raw, hum_raw & 0xFFFF, hum_raw);
    osal_printk("UART%d 解析结果：温度=%.1f℃, 湿度=%.1f%%\n",
                CONFIG_UART_BUS_ID, temperature, humidity);
}

// 打印接收到的数据
static void print_response_data(uint8_t *buffer, uint32_t length)
{
    osal_printk("UART%d 收到数据：", CONFIG_UART_BUS_ID);
    for (uint32_t i = 0; i < length; i++) {
        osal_printk("0x%02X ", buffer[i]);
    }
    osal_printk("\n");

    // 解析温湿度
    parse_modbus_response(buffer, length);
}

// UART 任务
static void *uart_task(const char *arg)
{
    unused(arg);
    osal_printk("UART 任务启动！\n");

    // 配置引脚
    app_uart_init_pin();

    // 初始化 UART
    app_uart_init_config();

    while (1) {
        uapi_watchdog_kick(); // 喂狗，防止复位
        osal_printk("UART 任务运行中\n");

        // 发送命令
        osal_printk("UART%d 发送命令：", CONFIG_UART_BUS_ID);
        for (uint32_t i = 0; i < UART_COMMAND_SIZE; i++) {
            osal_printk("0x%02X ", g_uart_command[i]);
        }
        osal_printk("\n");

        int32_t bytes_written = uapi_uart_write(CONFIG_UART_BUS_ID, (uint8_t *)g_uart_command, UART_COMMAND_SIZE, 0);
        if (bytes_written == UART_COMMAND_SIZE) {
            osal_printk("UART%d 命令发送成功！\n", CONFIG_UART_BUS_ID);
        } else {
            osal_printk("UART%d 命令发送失败！发送字节=%d\n", CONFIG_UART_BUS_ID, bytes_written);
            osal_msleep(UART_COMMAND_INTERVAL_MS);
            continue;
        }

        // 接收数据
        osal_printk("UART%d 准备接收数据\n", CONFIG_UART_BUS_ID);
        int32_t bytes_read = uapi_uart_read(CONFIG_UART_BUS_ID, g_app_uart_rx_buff, UART_RX_BUFFER_SIZE, UART_RESPONSE_TIMEOUT_MS);
        if (bytes_read > 0) {
            osal_printk("UART%d 收到 %d 字节！\n", CONFIG_UART_BUS_ID, bytes_read);
            print_response_data(g_app_uart_rx_buff, bytes_read);
        } else {
            osal_printk("UART%d 接收失败或超时！字节数=%d\n", CONFIG_UART_BUS_ID, bytes_read);
        }

        // 等待下一次发送
        osal_msleep(UART_COMMAND_INTERVAL_MS);
    }

    return NULL;
}

// UART 入口函数
static void uart_entry(void)
{
    osal_printk("进入 UART 入口\n");
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)uart_task, 0, "UartTask", UART_TASK_STACK_SIZE);
    if (task_handle == NULL) {
        osal_printk("创建 UART 任务失败！\n");
        return;
    }
    osal_kthread_set_priority(task_handle, UART_TASK_PRIO);
    osal_printk("UART 任务创建成功\n");
    osal_kthread_unlock();
}

/* 启动入口 */
app_run(uart_entry);