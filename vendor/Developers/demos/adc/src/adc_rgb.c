/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: ADC Sample Source. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-07-06, Create file. \n
 */
#include "adc_rgb.h"
#define ostime       1000
#define XONE         1
#define WOLTMV       900
// 模拟接收到的指令
int led_state = 0; // 灯的状态
void rgb_blinky_task(void);

void init_rgb_led_pins(pin_t pin)
{
    uapi_pin_set_mode(pin, HAL_PIO_FUNC_GPIO); // 初始化RGB灯的引脚为GPIO模式，并设置为输出
    uapi_gpio_set_dir(pin, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(pin, GPIO_LEVEL_LOW); // 初始状态为关闭所有灯（低电平）
    osal_printk("Pin %d initialized as GPIO output\n", pin);
}

void control_led(int red_state, int green_state, int blue_state) 
{
    uapi_gpio_set_val(RED_LED_PIN, red_state);
    uapi_gpio_set_val(GREEN_LED_PIN, green_state);
    uapi_gpio_set_val(BLUE_LED_PIN, blue_state);
}

void rgb_init(void)
{
    init_rgb_led_pins(RED_LED_PIN);
    init_rgb_led_pins(GREEN_LED_PIN);
    init_rgb_led_pins(BLUE_LED_PIN);
    osal_printk("rgb init okk \n");
}

void set_led_state(int state) 
{
    if (state >= XZERO && state <= XFOUR)
    {  // 状态值范围从0到4
        led_state = state;
        osal_printk("set state ok");
    }
}

void rgb_blinky_task(void)
{
    rgb_init();
    while (1) 
    {
        switch (led_state) 
        {
            case ALL_LEDS_OFF:
                // 所有灯熄灭
                control_led(GPIO_LEVEL_LOW, GPIO_LEVEL_LOW, GPIO_LEVEL_LOW);
                break;
            case RED_LED_ON:
                // 红灯亮
                control_led(GPIO_LEVEL_HIGH, GPIO_LEVEL_LOW, GPIO_LEVEL_LOW);
                break;
            case GREEN_LED_ON:
                // 绿灯亮
                control_led(GPIO_LEVEL_LOW, GPIO_LEVEL_HIGH, GPIO_LEVEL_LOW);
                break;
            case BLUE_LED_ON:
                // 蓝灯亮
                control_led(GPIO_LEVEL_LOW, GPIO_LEVEL_LOW, GPIO_LEVEL_HIGH);
                break;
            case ALL_LEDS_ON:
                // 三个灯同时亮
                control_led(GPIO_LEVEL_HIGH, GPIO_LEVEL_HIGH, GPIO_LEVEL_HIGH);
                break;
            default:
                // 错误状态处理
                break;
        }
        osal_msleep(RGB_DURATION_MS);
        uapi_watchdog_kick();
    }
}

static void adc_set_io(uint8_t channel) //ADC引脚配置
{
    pin_t adc_pin[] = {S_MGPIO2, S_MGPIO3, S_MGPIO6};
    uapi_pin_set_mode(adc_pin[channel], PIN_MODE_0); //将引脚模式设置为 PIN_MODE_0
    uapi_gpio_set_dir(adc_pin[channel], GPIO_DIRECTION_INPUT); //将引脚方向设置为输入
    uapi_pin_set_pull(adc_pin[channel], PIN_PULL_NONE); //禁用引脚的上拉或下拉电阻
}


static void *adc_task(const char *arg)
{
    UNUSED(arg);
    uapi_pmu_control(PMU_CONTROL_MICLDO_POWER, PMU_CONTROL_POWER_ON); //控制电源管理单元（PMU），确保 ADC 有稳定的电源
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_1V5);  //在MLDO脚产生一个1.5V的电压便以测试
    osal_printk("start adc sample test");
    uapi_adc_init(ADC_CLOCK_NONE); //初始化 ADC 模块
    adc_set_io(GADC_CHANNEL_2); //配置 ADC 引脚
    uapi_adc_open_channel(GADC_CHANNEL_2);  //GPIO02,打开指定的 ADC 通道
    uapi_adc_power_en(AFE_GADC_MODE, true);  //启用 ADC 电源。
    adc_calibration(AFE_GADC_MODE, true, true, true);   //进行 ADC 校准。

    while (1)
    {
        int gadc_value = 0;
        gadc_value =  uapi_adc_auto_sample(GADC_CHANNEL_2);    //从指定通道采集 ADC 值。
        osal_printk("adcc:  %d\n",gadc_value);
        uint32_t voltage_mv = (gadc_value * ADC_TICK2VOL_REF_VOLTAGE_MV) >> GAFE_SAMPLE_VALUE_SIGN_BIT; // 将采样值转换为电压值
        osal_printk("gadc: %dmv\n", voltage_mv);
        // 如果电压小于900毫伏，亮红灯
        if (voltage_mv < WOLTMV) 
        {
            led_state = XONE; // 设置红灯亮
        }
        osDelay(ostime);  //1000ms
    }
    return NULL;
}

static void task_entry(void)
{

    osal_kthread_lock();
    osal_task *adc_task_handle = NULL;
    adc_task_handle = osal_kthread_create((osal_kthread_handler)adc_task, 0, "adcTask", adc_TASK_STACK_SIZE);
    if (adc_task_handle != NULL) 
    {
        osal_kthread_set_priority(adc_task_handle, adc_TASK_PRIO);
        osal_printk("adc task created successfully\r\n");
        osal_kfree(adc_task_handle);
    }

    osal_task *rgb_task_handle = NULL;
    rgb_task_handle = osal_kthread_create((osal_kthread_handler)rgb_blinky_task, 0, "RGBBlinkyTask", RGB_TASK_STACK_SIZE);
    if (rgb_task_handle != NULL) 
    {
        osal_kthread_set_priority(rgb_task_handle, RGB_TASK_PRIO);
        osal_printk("RGB blinky task created successfully\r\n");
        osal_kfree(rgb_task_handle);
    }
   osal_kthread_unlock();
}
app_run(task_entry);





