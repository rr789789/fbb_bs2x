#include "encoder.h"
#include "soc_osal.h"
#include "stdbool.h"
#include "errcode.h"
#include "surface_dial_hid.h"


// 旋钮状态变化回调函数
static void ec11_event_handler(ec11_state_t state, void *arg)
{
    unused(arg);

    switch (state)
    {
    case EC11_ROTATE_CW:
        osal_printk("Surface Dial CW rotation\n");
        // 发送相对增量值 +1
        surface_dial_send_rotation(120);
        break;

    case EC11_ROTATE_CCW:
        osal_printk("Surface Dial CCW rotation\n");

        // 发送相对增量值 -1
        surface_dial_send_rotation(-120);

        break;

    case EC11_BUTTON_PRESS:
        osal_printk("Surface Dial button press\n");
        surface_dial_send_button(1);
        break;

    case EC11_BUTTON_RELEASE:
        osal_printk("Surface Dial button release\n");
        surface_dial_send_button(0);
        break;

    default:
        break;
    }
}

// 主函数
void encoder_init(void)
{
    

    // 配置EC11编码器
    ec11_config_t config = {
        .pin_a = 2,                    // A相连接到GPIO 13
        .pin_b = 3,                    // B相连接到GPIO 15
        .pin_button = 4,               // 按键连接到GPIO 16
        .has_button = true,             // 启用按键功能
        .debounce_ms = 5,               // 5ms去抖
        .callback = ec11_event_handler, // 设置回调函数
        .user_data = NULL               // 不使用用户数据
    };

    // 初始化EC11编码器
    errcode_t ret = ec11_init(&config);
    if (ret != ERRCODE_SUCC)
    {
        osal_printk("EC11 init failed! %d\r\n",ret);
        return ;
    }

    osal_printk("EC11 init succ!\r\n");


    // 初始化Surface Dial HID设备
    int hid_index = surface_dial_init();
    if (hid_index < 0)
    {
        osal_printk("Surface Dial init failed!\n");
        return;
    }

    
    return ;
}