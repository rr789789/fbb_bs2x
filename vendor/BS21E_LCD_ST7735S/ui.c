#include "chinese_font.h"
#include "tft.h"
#include "ui.h"
#include "font.h"



void draw_background(void)
{
    showimage(font, 0, 0, 160, 128);
    LCD_display(0, 0, 39, 127, 0x064F);

    // 绘制圆角矩形按钮
    LCD_draw_rounded_rect_button(50, 10, 45, 45,
                                 8, 0x064F, 0xFFFF, 2);

    LCD_draw_rounded_rect_button(105, 10, 45, 45,
                                 8, 0x064F, 0xFFFF, 2);

    LCD_draw_rounded_rect_button(50, 65, 45, 45,
                                 8, 0x064F, 0xFFFF, 2);

    LCD_draw_rounded_rect_button(105, 65, 45, 45,
                                 8, 0x064F, 0xFFFF, 2);

    LCD_display(38, 0, 39, 127, 0xFFFF); // 右边界线
}

void screen_work(void)
{

    draw_vertical_english_word("WORK", 8);
    // 显示两个放大的汉字
    LCD_show_chinese_char_scaled(4, 60, "办", 0xFFFF, 0x064F, 2);
    LCD_show_chinese_char_scaled(4, 94, "公", 0xFFFF, 0x064F, 2);

    LCD_show_chinese_string(50 + 7, 10 + 15, "解锁", 0xFFFF, 0x064F);
    LCD_show_chinese_string(105 + 7, 10 + 15, "截屏", 0xFFFF, 0x064F);
    LCD_show_chinese_string(50 + 7, 65 + 15, "复贴", 0xFFFF, 0x064F);
    LCD_show_chinese_string(105 + 7, 65 + 15, "切屏", 0xFFFF, 0x064F);
}

void screen_home(void)
{

    draw_vertical_english_word("HOME", 8);
    // 显示两个放大的汉字
    LCD_show_chinese_char_scaled(4, 60, "家", 0xFFFF, 0x064F, 2);
    LCD_show_chinese_char_scaled(4, 94, "居", 0xFFFF, 0x064F, 2);

    LCD_show_chinese_string(50 + 7, 10 + 15, "灯具", 0xFFFF, 0x064F);
    LCD_show_chinese_string(105 + 7, 10 + 15, "门禁", 0xFFFF, 0x064F);
    LCD_show_chinese_string(50 + 7, 65 + 15, "复贴", 0xFFFF, 0x064F);
    LCD_show_chinese_string(105 + 7, 65 + 15, "切屏", 0xFFFF, 0x064F);
}

void screen_play(void)
{

    draw_vertical_english_word("PLAY", 8);
    // 显示两个放大的汉字
    LCD_show_chinese_char_scaled(4, 60, "娱", 0xFFFF, 0x064F, 2);
    LCD_show_chinese_char_scaled(4, 94, "乐", 0xFFFF, 0x064F, 2);

    LCD_show_chinese_string(50 + 7, 10 + 15, "上键", 0xFFFF, 0x064F);
    LCD_show_chinese_string(105 + 7, 10 + 15, "下键", 0xFFFF, 0x064F);
    LCD_show_chinese_string(50 + 7, 65 + 15, "左键", 0xFFFF, 0x064F);
    LCD_show_chinese_string(105 + 7, 65 + 15, "右键", 0xFFFF, 0x064F);
}



/**
 * 圆角边框绘制
 * @param button_id 按钮ID (1-4对应四个按钮)
 * @param border_color 边框颜色
 * @param border_width 边框宽度
 */
void draw_button_rounded_border(button_position_t button_id, uint16_t border_color, uint8_t border_width)
{
    if (button_id < BUTTON_TOP_LEFT || button_id > BUTTON_BOTTOM_RIGHT)
    {
        osal_printk("Invalid button_id: %d\n", button_id);
        return;
    }

    // 修正数组索引：button_id从1开始，数组从0开始
    const button_info_t *btn = &button_positions[button_id - 1];
    uint8_t radius = btn->radius;
    uint8_t x = btn->x;
    uint8_t y = btn->y;
    uint8_t width = btn->width;
    uint8_t height = btn->height;

    // 参考LCD_draw_rounded_rect_button的完整逻辑
    uint8_t i, j;
    int16_t dx, dy, distance_sq, radius_sq;
    int16_t inner_radius_sq = (radius - border_width) * (radius - border_width);

    // 计算半径的平方，避免开方运算
    radius_sq = radius * radius;

    // 遍历按钮区域内的每个像素
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            uint8_t pixel_x = x + j;
            uint8_t pixel_y = y + i;
            bool is_border_pixel = false;
            bool is_inside = true;

            // 判断当前像素是否在圆角区域
            if ((i < radius && j < radius) ||           // 左上角
                (i < radius && j >= width - radius) ||  // 右上角
                (i >= height - radius && j < radius) || // 左下角
                (i >= height - radius && j >= width - radius))
            { // 右下角

                // 计算到对应圆心的距离
                if (i < radius && j < radius)
                {
                    // 左上角圆心
                    dx = j - radius;
                    dy = i - radius;
                }
                else if (i < radius && j >= width - radius)
                {
                    // 右上角圆心
                    dx = j - (width - radius - 1);
                    dy = i - radius;
                }
                else if (i >= height - radius && j < radius)
                {
                    // 左下角圆心
                    dx = j - radius;
                    dy = i - (height - radius - 1);
                }
                else
                {
                    // 右下角圆心
                    dx = j - (width - radius - 1);
                    dy = i - (height - radius - 1);
                }

                distance_sq = dx * dx + dy * dy;

                // 如果距离大于圆角半径，则不是有效像素
                if (distance_sq > radius_sq)
                {
                    is_inside = false;
                }
                else if (distance_sq > inner_radius_sq)
                {
                    // 如果在边框范围内（圆角区域）
                    is_border_pixel = true;
                }
            }
            else
            {
                // 非圆角区域，判断是否为边框
                if (i < border_width || i >= height - border_width ||
                    j < border_width || j >= width - border_width)
                {
                    is_border_pixel = true;
                }
            }

            // 只绘制边框像素
            if (is_inside && is_border_pixel)
            {
                LCD_draw_pixel(pixel_x, pixel_y, border_color);
            }
        }
    }
}

/**
 * 快速按钮按下反馈效果 - 只改变边框
 * @param button_id 按钮ID (1-4对应四个按钮)
 */
void button_press_feedback(button_position_t button_id)
{
    // 1. 立即改变边框为红色并加粗
    draw_button_rounded_border(button_id, BUTTON_PRESSED_BORDER, 2);
    // 3. 恢复正常边框
    draw_button_rounded_border(button_id, BUTTON_BORDER_COLOR, 2);

}