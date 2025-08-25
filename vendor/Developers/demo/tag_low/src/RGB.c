#include "RGB.h"

// 模拟接收到的指令
int led_state = 0; // 灯的状态

void init_rgb_led_pins(pin_t pin) // 初始化灯的状态
{
    uapi_pin_set_mode(pin, HAL_PIO_FUNC_GPIO); // 初始化RGB灯的引脚为GPIO模式，并设置为输出
    uapi_gpio_set_dir(pin, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(pin, GPIO_LEVEL_LOW); // 初始状态为关闭所有灯（低电平）
}


void control_led(int red_state, int green_state, int blue_state) {
    uapi_gpio_set_val(RED_LED_PIN, red_state);
    uapi_gpio_set_val(GREEN_LED_PIN, green_state);
    uapi_gpio_set_val(BLUE_LED_PIN, blue_state);
    osal_msleep(RGB_DURATION_MS);
    uapi_gpio_set_val(RED_LED_PIN, GPIO_LEVEL_LOW);
    uapi_gpio_set_val(GREEN_LED_PIN, GPIO_LEVEL_LOW);
    uapi_gpio_set_val(BLUE_LED_PIN, GPIO_LEVEL_LOW);
}

void rgb_init(void){
    init_rgb_led_pins(RED_LED_PIN);
    init_rgb_led_pins(GREEN_LED_PIN);
    init_rgb_led_pins(BLUE_LED_PIN);
    osal_printk("rgb init okk \n");
}

void rgb_blinky_task(void)
{
    
    while (1) {
        switch (led_state) {
            case 0:
                // 所有灯熄灭
                control_led(GPIO_LEVEL_LOW, GPIO_LEVEL_LOW, GPIO_LEVEL_LOW);
                //osal_printk("RGB_State:0000\r\n");
                break;
            case 1:
                // 红灯亮
                control_led(GPIO_LEVEL_HIGH, GPIO_LEVEL_LOW, GPIO_LEVEL_LOW);
                //osal_printk("RGB_State: REDDD1111111111\r\n");
                break;
            case 2:
                // 绿灯亮
                control_led(GPIO_LEVEL_LOW, GPIO_LEVEL_HIGH, GPIO_LEVEL_LOW);
               // osal_printk("RGB_State:ggggggggggg)\r\n");
                break;
            case 3:
                // 蓝灯亮
                control_led(GPIO_LEVEL_LOW, GPIO_LEVEL_LOW, GPIO_LEVEL_HIGH);
                // osal_printk("RGB_State: bbbbbbbbbbbbbb(01)\r\n");
             
                break;
            case 4:
                // 三个灯同时亮
                control_led(GPIO_LEVEL_HIGH, GPIO_LEVEL_HIGH, GPIO_LEVEL_HIGH);
                //osal_printk("RGB_State: rgbbb\r\n");
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
void set_led_state(int state) {
    if (state >= 0 && state <= 4) { // 状态值范围从0到4
        led_state = state;
    }
}





