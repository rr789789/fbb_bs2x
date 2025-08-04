#include "boards.h"
#include "pinctrl.h"
#include "gpio.h"
#include "soc_osal.h"
#include "app_init.h"
#include <string.h>

// 旋转编码器引脚定义
#define ROTARY_ENCODER_SW_PIN   CONFIG_SAMPLE_ROTARY_ENCODER_SW_PIN   // 按键引脚
#define ROTARY_ENCODER_DT_PIN   CONFIG_SAMPLE_ROTARY_ENCODER_DT_PIN   // DT 数据引脚
#define ROTARY_ENCODER_CLK_PIN  CONFIG_SAMPLE_ROTARY_ENCODER_CLK_PIN  // CLK 时钟引脚

// 任务参数
#define ROTARY_ENCODER_TASK_PRIO        24
#define ROTARY_ENCODER_TASK_STACK_SIZE  0x2000

// 调试开关
#define ROTARY_ENCODER_DEBUG CONFIG_SAMPLE_ROTARY_ENCODER_DEBUG

// 旋转编码器状态结构
typedef struct {
    int32_t position;           // 当前位置
    gpio_level_t last_clk;      // 上次CLK状态
    gpio_level_t last_dt;       // 上次DT状态
    gpio_level_t last_sw;       // 上次SW状态
    uint32_t debounce_time;     // 防抖时间
} rotary_encoder_state_t;

static rotary_encoder_state_t encoder_state = {0};

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
    encoder_state.last_clk = uapi_gpio_get_val(ROTARY_ENCODER_CLK_PIN);
    encoder_state.last_dt = uapi_gpio_get_val(ROTARY_ENCODER_DT_PIN);
    encoder_state.last_sw = uapi_gpio_get_val(ROTARY_ENCODER_SW_PIN);
    encoder_state.position = 0;
    encoder_state.debounce_time = 0;

#if ROTARY_ENCODER_DEBUG
    osal_printk("旋转编码器GPIO初始化完成\n");
    osal_printk("SW引脚: %d, DT引脚: %d, CLK引脚: %d\n", 
                ROTARY_ENCODER_SW_PIN, ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_CLK_PIN);
    osal_printk("初始状态 - CLK: %d, DT: %d, SW: %d\n", 
                encoder_state.last_clk, encoder_state.last_dt, encoder_state.last_sw);
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
    if (encoder_state.last_clk == GPIO_LEVEL_HIGH && current_clk == GPIO_LEVEL_LOW) {
        // 防抖处理：如果距离上次变化时间太短，则忽略
        if (current_time - encoder_state.debounce_time > 5) {
            if (current_dt == GPIO_LEVEL_LOW) {
                // CLK下降沿时DT为低电平，顺时针旋转
                encoder_state.position++;
#if ROTARY_ENCODER_DEBUG
                osal_printk("顺时针旋转，位置: %d\n", encoder_state.position);
#endif
            } else {
                // CLK下降沿时DT为高电平，逆时针旋转
                encoder_state.position--;
#if ROTARY_ENCODER_DEBUG
                osal_printk("逆时针旋转，位置: %d\n", encoder_state.position);
#endif
            }
            encoder_state.debounce_time = current_time;
        }
    }
    
    // 检测按键状态变化
    if (encoder_state.last_sw == GPIO_LEVEL_HIGH && current_sw == GPIO_LEVEL_LOW) {
        // 按键被按下
        if (current_time - encoder_state.debounce_time > 50) {  // 按键防抖时间更长
#if ROTARY_ENCODER_DEBUG
            osal_printk("按键按下, 位置重置为0\n");
#endif
            encoder_state.position = 0;  // 按键按下时重置位置
            encoder_state.debounce_time = current_time;
        }
    }
    
    // 更新状态
    encoder_state.last_clk = current_clk;
    encoder_state.last_dt = current_dt;
    encoder_state.last_sw = current_sw;
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
    
    int32_t last_position = encoder_state.position;
    uint32_t last_output_time = 0;
    
    while (1) {
        // 更新编码器状态
        rotary_encoder_update();
        
        uint32_t current_time = osal_jiffies_to_msecs((uint32_t)osal_get_jiffies());
        
        // 如果位置发生变化，立即输出
        if (encoder_state.position != last_position) {
            osal_printk("当前位置: %d (变化: %+d)\n", 
                       encoder_state.position, 
                       encoder_state.position - last_position);
            last_position = encoder_state.position;
            last_output_time = current_time;
        }
        // 每5秒输出一次当前位置（即使没有变化）
        else if (current_time - last_output_time >= 5000) {
            osal_printk("当前位置: %d (无变化)\n", encoder_state.position);
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
    
    task_handle = osal_kthread_create((osal_kthread_handler)rotary_encoder_task, 
                                      0, "RotaryEncoderTask", 
                                      ROTARY_ENCODER_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, ROTARY_ENCODER_TASK_PRIO);
        osal_kfree(task_handle);
    }
    
    osal_kthread_unlock();
}

app_run(rotary_encoder_demo_init);
