#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>

// 按钮颜色定义 - 增加边框反馈颜色
#define BUTTON_NORMAL_COLOR 0x064F     // 按钮正常颜色
#define BUTTON_PRESSED_COLOR 0xF800    // 按钮按下颜色(红色) - 现在主要用于边框
#define BUTTON_BORDER_COLOR 0xFFFF     // 按钮正常边框颜色(白色)
#define BUTTON_PRESSED_BORDER 0xF800   // 按钮按下边框颜色(红色)


// 按钮信息结构体
typedef struct
{
    uint8_t x;      // 按钮x坐标
    uint8_t y;      // 按钮y坐标
    uint8_t width;  // 按钮宽度
    uint8_t height; // 按钮高度
    uint8_t radius; // 按钮圆角半径
} button_info_t;

// 按钮位置信息表
static const button_info_t button_positions[4] = {
    {50, 10, 45, 45, 8},  // 左上按钮
    {105, 10, 45, 45, 8}, // 右上按钮
    {50, 65, 45, 45, 8},  // 左下按钮
    {105, 65, 45, 45, 8}  // 右下按钮
};

// 按钮位置枚举
typedef enum
{
    BUTTON_TOP_LEFT = 1, // 左上按钮
    BUTTON_TOP_RIGHT,    // 右上按钮
    BUTTON_BOTTOM_LEFT,  // 左下按钮
    BUTTON_BOTTOM_RIGHT, // 右下按钮
} button_position_t;

// 界面绘制函数
void draw_background(void);
void screen_work(void);
void screen_home(void); 
void screen_play(void);

void draw_button_rounded_border(button_position_t button_id, uint16_t border_color, uint8_t border_width);
void button_press_feedback(button_position_t button_id);

#endif // UI_H