#include "boards.h"
#include "pinctrl.h"
#include "gpio.h"
#include "soc_osal.h"
#include "app_init.h"
#include <string.h>

// 旋转编码器引脚定义
#define ROTARY_ENCODER_SW_PIN 5   // 按键引脚
#define ROTARY_ENCODER_DT_PIN 6   // DT 数据引脚
#define ROTARY_ENCODER_CLK_PIN 7 // CLK 时钟引脚

// 任务参数
#define ROTARY_ENCODER_TASK_PRIO 24
#define ROTARY_ENCODER_TASK_STACK_SIZE 0x2000

// 调试开关
#define ROTARY_ENCODER_DEBUG CONFIG_SAMPLE_ROTARY_ENCODER_DEBUG

// 时间常量
#define DEBOUNCE_THRESHOLD_MS 5
#define BUTTON_DEBOUNCE_THRESHOLD_MS 50
#define NO_CHANGE_REPORT_INTERVAL_MS 5000

// 旋转编码器状态结构
typedef struct {
    int32_t position;       // 当前位置
    gpio_level_t last_clk;  // 上次CLK状态
    gpio_level_t last_dt;   // 上次DT状态
    gpio_level_t last_sw;   // 上次SW状态
    uint32_t debounce_time; // 防抖时间
} rotary_encoder_state_t;

static rotary_encoder_state_t g_encoder_state = {0};

/**
 * @brief 初始化旋转编码器GPIO
 */
static void rotary_encoder_gpio_init(void)
{
    // 配置SW引脚为输入，启用上拉
    uapi_pin_set_mode(ROTARY_ENCODER_SW_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(ROTARY_ENCODER_SW_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_pull(ROTARY_ENCODER_SW_PIN, PIN_PULL_UP);

    // 配置DT引脚为输入，启用上拉
    uapi_pin_set_mode(ROTARY_ENCODER_DT_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(ROTARY_ENCODER_DT_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_pull(ROTARY_ENCODER_DT_PIN, PIN_PULL_UP);

    // 配置CLK引脚为输入，启用上拉
    uapi_pin_set_mode(ROTARY_ENCODER_CLK_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(ROTARY_ENCODER_CLK_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_pull(ROTARY_ENCODER_CLK_PIN, PIN_PULL_UP);

    // 初始化状态
    g_encoder_state.last_clk = uapi_gpio_get_val(ROTARY_ENCODER_CLK_PIN);
    g_encoder_state.last_dt = uapi_gpio_get_val(ROTARY_ENCODER_DT_PIN);
    g_encoder_state.last_sw = uapi_gpio_get_val(ROTARY_ENCODER_SW_PIN);
    g_encoder_state.position = 0;
    g_encoder_state.debounce_time = 0;

#if ROTARY_ENCODER_DEBUG
    osal_printk("旋转编码器GPIO初始化完成\n");
    osal_printk("SW引脚: %d, DT引脚: %d, CLK引脚: %d\n", ROTARY_ENCODER_SW_PIN, ROTARY_ENCODER_DT_PIN,
                ROTARY_ENCODER_CLK_PIN);
    osal_printk("初始状态 - CLK: %d, DT: %d, SW: %d\n", g_encoder_state.last_clk, g_encoder_state.last_dt,
                g_encoder_state.last_sw);
#endif
}

/**
 * @brief 读取旋转编码器状态并更新位置
 */
static void rotary_encoder_update(void)
{
    gpio_level_t current_clk = uapi_gpio_get_val(ROTARY_ENCODER_CLK_PIN);
    gpio_level_t current_dt = uapi_gpio_get_val(ROTARY_ENCODER_DT_PIN);
    gpio_level_t current_sw = uapi_gpio_get_val(ROTARY_ENCODER_SW_PIN);

    uint32_t current_time = osal_jiffies_to_msecs((uint32_t)osal_get_jiffies());

    // 检测CLK信号的下降沿
    if (g_encoder_state.last_clk == GPIO_LEVEL_HIGH && current_clk == GPIO_LEVEL_LOW) {
        // 防抖处理：如果距离上次变化时间太短，则忽略
        if (current_time - g_encoder_state.debounce_time > DEBOUNCE_THRESHOLD_MS) {
            if (current_dt == GPIO_LEVEL_LOW) {
                // CLK下降沿时DT为低电平，顺时针旋转
                g_encoder_state.position++;
#if ROTARY_ENCODER_DEBUG
                osal_printk("顺时针旋转，位置: %d\n", g_encoder_state.position);
#endif
            } else {
                // CLK下降沿时DT为高电平，逆时针旋转
                g_encoder_state.position--;
#if ROTARY_ENCODER_DEBUG
                osal_printk("逆时针旋转，位置: %d\n", g_encoder_state.position);
#endif
            }
            g_encoder_state.debounce_time = current_time;
        }
    }

    // 检测按键状态变化
    if (g_encoder_state.last_sw == GPIO_LEVEL_HIGH && current_sw == GPIO_LEVEL_LOW) {
        // 按键被按下
        if (current_time - g_encoder_state.debounce_time > BUTTON_DEBOUNCE_THRESHOLD_MS) { // 按键防抖时间更长
#if ROTARY_ENCODER_DEBUG
            osal_printk("按键按下, 位置重置为0\n");
#endif
            g_encoder_state.position = 0; // 按键按下时重置位置
            g_encoder_state.debounce_time = current_time;
        }
    }

    // 更新状态
    g_encoder_state.last_clk = current_clk;
    g_encoder_state.last_dt = current_dt;
    g_encoder_state.last_sw = current_sw;
}

/**
 * @brief 旋转编码器任务函数
 */
static void *rotary_encoder_task(const char *arg)
{
    unused(arg);

    osal_printk("=== 旋转编码器演示程序启动 ===\n");
    osal_printk("使用引脚配置:\n");
    osal_printk("  SW (按键): GPIO %d\n", ROTARY_ENCODER_SW_PIN);
    osal_printk("  DT (数据): GPIO %d\n", ROTARY_ENCODER_DT_PIN);
    osal_printk("  CLK (时钟): GPIO %d\n", ROTARY_ENCODER_CLK_PIN);
    osal_printk("操作说明:\n");
    osal_printk("  - 顺时针旋转: 位置递增\n");
    osal_printk("  - 逆时针旋转: 位置递减\n");
    osal_printk("  - 按下按键: 位置重置为0\n");
    osal_printk("==============================\n\n");

    // 初始化GPIO
    rotary_encoder_gpio_init();

    int32_t last_position = g_encoder_state.position;
    uint32_t last_output_time = 0;

    while (1) {
        // 更新编码器状态
        rotary_encoder_update();

        uint32_t current_time = osal_jiffies_to_msecs((uint32_t)osal_get_jiffies());

        // 如果位置发生变化，立即输出
        if (g_encoder_state.position != last_position) {
            osal_printk("当前位置: %d (变化: %+d)\n", g_encoder_state.position,
                        g_encoder_state.position - last_position);
            last_position = g_encoder_state.position;
            last_output_time = current_time;

            // 每5秒输出一次当前位置（即使没有变化）
        } else if (current_time - last_output_time >= NO_CHANGE_REPORT_INTERVAL_MS) {
            osal_printk("当前位置: %d (无变化)\n", g_encoder_state.position);
            last_output_time = current_time;
        }

        // 短暂延时，避免过度占用CPU
        osal_msleep(1);
    }

    return NULL;
}

/**
 * @brief 旋转编码器演示初始化函数
 */
static void rotary_encoder_demo_init(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();

    task_handle = osal_kthread_create((osal_kthread_handler)rotary_encoder_task, 0, "RotaryEncoderTask",
                                      ROTARY_ENCODER_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, ROTARY_ENCODER_TASK_PRIO);
        osal_kfree(task_handle);
    }

    osal_kthread_unlock();
}

app_run(rotary_encoder_demo_init);
