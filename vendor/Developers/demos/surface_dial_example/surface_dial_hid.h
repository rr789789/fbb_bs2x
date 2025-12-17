#ifndef SURFACE_DIAL_HID_H
#define SURFACE_DIAL_HID_H

#include "gadget/f_hid.h"
#include "implementation/usb_init.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Surface Dial HID报告描述符
    static const uint8_t g_surface_dial_report_desc[] = {
        0x05, 0x01,       //	USAGE_PAGE(Generic Desktop Controls)
        0x09, 0x0E,       //	LOCAL_USAGE(Reserved)
        0xA1, 0x01,       //	COLLECTION(Applicatior)
        0x85, 0x05,       //		REPORT_ID(5)
        0x05, 0x0D,       //		USAGE_PAGE(Digitizer)
        0x09, 0x21,       //		LOCAL_USAGE(Puck)
        0xA1, 0x00,       //		MAIN_COLLECTION(Physical)
        0x05, 0x09,       //		USAGE_PAGE(Button)
        0x09, 0x01,       //		LOCAL_USAGE(Button 1 (primary/trigger))
        0x95, 0x01,       //		REPORT_COUNT(1)
        0x75, 0x01,       //		REPORT_SIZE(1)
        0x15, 0x00,       //		LOGICAL_MINIMUM(0)
        0x25, 0x01,       //		LOCAL_MAXIMUM(1)
        0x81, 0x02,       //		INPUT(data var absolute NoWrap linear) Input 0.1
        0x05, 0x01,       //		USAGE_PAGE(Generic Desktop Controls)
        0x09, 0x37,       //		LOCAL_USAGE(Dial)
        0x95, 0x01,       //		REPORT_COUNT(1)
        0x75, 0x0F,       //		REPORT_SIZE(15)
        0x55, 0x0F,       //		UNIT_EXPONENT(15)
        0x65, 0x14,       //		REPORT_UNIT(20)
        0x36, 0xF0, 0xF1, //		PHYSICAL_MINIMUM(61936)
        0x46, 0x10, 0x0E, //		PHYSICAL_MAXIMUM(3600)
        0x16, 0xF0, 0xF1, //		LOGICAL_MINIMUM(-3600)
        0x26, 0x10, 0x0E, //		LOCAL_MAXIMUM(3600)
        0x81, 0x06,       //		INPUT(data var relative NoWrap linear) Input 2.0
        

        0xC0,       //		END_COLLECTION
        0xC0,       //	END_COLLECTION
    };

    // // Surface Dial报告结构
    // typedef struct
    // {
    //     uint8_t button : 1; // 按钮状态
    //     int16_t dial : 15;  // 旋转值（15位有符号）
    // } __attribute__((packed)) surface_dial_report_t;

    // Surface Dial报告结构 (修正版)
    typedef struct __attribute__((packed))
    {
        int8_t rotation;         // 8位旋转值 [-127,127]
        uint8_t button_play : 1; // 播放/暂停按钮
        uint8_t button_mute : 1; // 静音按钮
        uint8_t reserved : 6;    // 保留位
    } surface_dial_report_t;


    // 函数声明
    int surface_dial_init(void);
    void surface_dial_deinit(void);
    int surface_dial_send_rotation(int16_t rotation);
    int surface_dial_send_button(uint8_t pressed);

#ifdef __cplusplus
}
#endif

#endif // SURFACE_DIAL_HID_H