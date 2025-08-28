#include "RGB.h"
// 模拟接收到的指令
int led_state = 0; // 灯的状态

void init_rgb_led_pins(pin_t pin) // 初始化灯的状态
{
    uapi_pin_set_mode(pin, HAL_PIO_FUNC_GPIO); // 初始化RGB灯的引脚为GPIO模式，并设置为输出
    uapi_gpio_set_dir(pin, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(pin, GPIO_LEVEL_LOW); // 初始状态为关闭所有灯（低电平）
}

void control_led(int red_state, int green_state, int blue_state) 
{
    uapi_gpio_set_val(RED_LED_PIN, red_state);
    uapi_gpio_set_val(GREEN_LED_PIN, green_state);
    uapi_gpio_set_val(BLUE_LED_PIN, blue_state);
    osal_msleep(RGB_DURATION_MS);
    uapi_gpio_set_val(RED_LED_PIN, GPIO_LEVEL_LOW);
    uapi_gpio_set_val(GREEN_LED_PIN, GPIO_LEVEL_LOW);
    uapi_gpio_set_val(BLUE_LED_PIN, GPIO_LEVEL_LOW);
}

void rgb_init(void)
{
    init_rgb_led_pins(RED_LED_PIN);
    init_rgb_led_pins(GREEN_LED_PIN);
    init_rgb_led_pins(BLUE_LED_PIN);
    osal_printk("rgb init okk \n");
}

void rgb_blinky_task(void)
{
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

// 虚拟接口，用于设置灯的状态
void set_led_state(int state) 
{
    if (state >= 0 && state <= 4) 
    { // 状态值范围从0到4
        led_state = state;
    }
}





