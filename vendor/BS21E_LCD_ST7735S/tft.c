#include "tft.h"
#include "pinctrl.h"
#include "spi.h"
#include "hal_gpio.h"
#include "chinese_font.h"

// ST7735S 160x128横屏显示配置
#define LCD_WIDTH 160
#define LCD_HEIGHT 128

void app_spi_init_pin(void)
{
    uapi_pin_set_mode(SPI_CS_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(SPI_CS_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(SPI_CS_PIN, GPIO_LEVEL_HIGH);

    uapi_pin_set_mode(SPI_RST_MASTER_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(SPI_RST_MASTER_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_HIGH);

    uapi_pin_set_mode(SPI_DC_MASTER_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(SPI_DC_MASTER_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(SPI_DC_MASTER_PIN, GPIO_LEVEL_HIGH);

    uapi_pin_set_mode(SPI_SDA_MASTER_PIN, 2);
    uapi_pin_set_mode(SPI_CLK_MASTER_PIN, 3);
    uapi_pin_set_ds(SPI_CLK_MASTER_PIN, 3);
}

void app_spi_master_init_config(void)
{
    spi_attr_t config = {0};
    spi_extra_attr_t ext_config = {0};

    config.is_slave = false;
    config.slave_num = SPI_SLAVE_NUM;
    config.bus_clk = 32000000; // 32MHz
    config.freq_mhz = 8;
    config.clk_polarity = SPI_CFG_CLK_CPOL_0;
    config.clk_phase = SPI_CFG_CLK_CPHA_0;
    config.frame_format = SPI_CFG_FRAME_FORMAT_MOTOROLA_SPI;
    config.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD;
    config.frame_size = HAL_SPI_FRAME_SIZE_8;
    config.tmod = HAL_SPI_TRANS_MODE_TX;
    config.sste = 0;
    ext_config.qspi_param.wait_cycles = SPI_WAIT_CYCLES;

    int ret = uapi_spi_init(SPI_MASTER_BUS_ID, &config, &ext_config);
    if (ret != 0)
    {
        osal_printk("SPI init failed: 0x%x\n", ret);
    }
}

// ST7735S复位函数
void lcd_reset(void)
{
    LCD_RESET_LOW;
    osal_msleep(50);
    LCD_RESET_HIGH;
    osal_msleep(120);
}

// 写8位数据
void ST7735S_write_data8(uint8_t dat)
{
    LCD_CS_LOW;
    LCD_DC_HIGH; // 数据模式

    spi_xfer_data_t data;
    data.tx_buff = &dat;
    data.tx_bytes = 1;
    data.rx_buff = NULL;
    data.rx_bytes = 0;

    uapi_spi_master_write(SPI_BUS_0, &data, 100);
    LCD_CS_HIGH;
}

// 写命令
void ST7735S_write_cmd(uint8_t cmd)
{
    LCD_CS_LOW;
    LCD_DC_LOW; // 命令模式

    spi_xfer_data_t data;
    data.tx_buff = &cmd;
    data.tx_bytes = 1;
    data.rx_buff = NULL;
    data.rx_bytes = 0;

    uapi_spi_master_write(SPI_BUS_0, &data, 100);
    LCD_CS_HIGH;
}

// 写16位颜色数据
void ST7735S_write_data16(uint16_t color)
{
    LCD_CS_LOW;
    LCD_DC_HIGH;

    uint8_t data_bytes[2];
    data_bytes[0] = (uint8_t)(color >> 8);   // 高8位
    data_bytes[1] = (uint8_t)(color & 0xFF); // 低8位

    spi_xfer_data_t data = {
        .tx_buff = data_bytes,
        .tx_bytes = 2,
        .rx_buff = NULL,
        .rx_bytes = 0};

    uapi_spi_master_write(SPI_BUS_0, &data, 100);
    LCD_CS_HIGH;
}

// ST7735S初始化
void LCD_init(void)
{
    lcd_reset();

    ST7735S_write_cmd(0x01); // 软件复位
    osal_mdelay(150);

    ST7735S_write_cmd(0x11); // 退出睡眠模式
    osal_mdelay(255);

    ST7735S_write_cmd(0xB1); // 帧率控制 - 正常模式
    ST7735S_write_data8(0x01);
    ST7735S_write_data8(0x2C);
    ST7735S_write_data8(0x2D);

    ST7735S_write_cmd(0xB2); // 帧率控制 - 空闲模式
    ST7735S_write_data8(0x01);
    ST7735S_write_data8(0x2C);
    ST7735S_write_data8(0x2D);

    ST7735S_write_cmd(0xB3); // 帧率控制 - 部分模式
    ST7735S_write_data8(0x01);
    ST7735S_write_data8(0x2C);
    ST7735S_write_data8(0x2D);
    ST7735S_write_data8(0x01);
    ST7735S_write_data8(0x2C);
    ST7735S_write_data8(0x2D);

    ST7735S_write_cmd(0xB4); // 显示反转控制
    ST7735S_write_data8(0x07);

    ST7735S_write_cmd(0xC0); // LCM控制
    ST7735S_write_data8(0xA2);
    ST7735S_write_data8(0x02);
    ST7735S_write_data8(0x84);

    ST7735S_write_cmd(0xC1); // LCM控制
    ST7735S_write_data8(0xC5);

    ST7735S_write_cmd(0xC2); // LCM控制
    ST7735S_write_data8(0x0A);
    ST7735S_write_data8(0x00);

    ST7735S_write_cmd(0xC3); // LCM控制
    ST7735S_write_data8(0x8A);
    ST7735S_write_data8(0x2A);

    ST7735S_write_cmd(0xC4); // LCM控制
    ST7735S_write_data8(0x8A);
    ST7735S_write_data8(0xEE);

    ST7735S_write_cmd(0xC5); // VCOM控制
    ST7735S_write_data8(0x0E);

    ST7735S_write_cmd(0x36);   // 内存访问控制 - 横屏显示
    ST7735S_write_data8(0xA0); // MY=1, MX=0, MV=1 for landscape

    ST7735S_write_cmd(0x3A);   // 像素格式设置
    ST7735S_write_data8(0x05); // 16位/像素

    // Gamma校正
    ST7735S_write_cmd(0xE0); // 正极性Gamma校正
    ST7735S_write_data8(0x02);
    ST7735S_write_data8(0x1C);
    ST7735S_write_data8(0x07);
    ST7735S_write_data8(0x12);
    ST7735S_write_data8(0x37);
    ST7735S_write_data8(0x32);
    ST7735S_write_data8(0x29);
    ST7735S_write_data8(0x2D);
    ST7735S_write_data8(0x29);
    ST7735S_write_data8(0x25);
    ST7735S_write_data8(0x2B);
    ST7735S_write_data8(0x39);
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(0x01);
    ST7735S_write_data8(0x03);
    ST7735S_write_data8(0x10);

    ST7735S_write_cmd(0xE1); // 负极性Gamma校正
    ST7735S_write_data8(0x03);
    ST7735S_write_data8(0x1D);
    ST7735S_write_data8(0x07);
    ST7735S_write_data8(0x06);
    ST7735S_write_data8(0x2E);
    ST7735S_write_data8(0x2C);
    ST7735S_write_data8(0x29);
    ST7735S_write_data8(0x2D);
    ST7735S_write_data8(0x2E);
    ST7735S_write_data8(0x2E);
    ST7735S_write_data8(0x37);
    ST7735S_write_data8(0x3F);
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(0x02);
    ST7735S_write_data8(0x10);

    ST7735S_write_cmd(0x13); // 正常显示模式
    osal_mdelay(10);

    ST7735S_write_cmd(0x29); // 显示开启
    osal_mdelay(100);

    osal_printk("ST7735S LCD init done - 160x128 landscape mode\n");
}

// 设置显示窗口
void LCD_set_address(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    // 边界检查
    if (x1 >= LCD_WIDTH)
        x1 = LCD_WIDTH - 1;
    if (y1 >= LCD_HEIGHT)
        y1 = LCD_HEIGHT - 1;
    if (x2 >= LCD_WIDTH)
        x2 = LCD_WIDTH - 1;
    if (y2 >= LCD_HEIGHT)
        y2 = LCD_HEIGHT - 1;

    ST7735S_write_cmd(0x2A); // 列地址设置
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(x1);
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(x2);

    ST7735S_write_cmd(0x2B); // 行地址设置
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(y1);
    ST7735S_write_data8(0x00);
    ST7735S_write_data8(y2);

    ST7735S_write_cmd(0x2C); // 内存写入
}

// 画单个像素点
void LCD_draw_pixel(uint8_t x, uint8_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
        return;

    LCD_set_address(x, y, x, y);
    ST7735S_write_data16(color);
}

// 显示全屏颜色
void LCD_display_full(uint16_t color)
{
    LCD_set_address(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
    {
        ST7735S_write_data16(color);
    }
}

// 显示矩形区域
void LCD_display(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    if (x1 > x2 || y1 > y2)
        return;
    if (x2 >= LCD_WIDTH || y2 >= LCD_HEIGHT)
        return;

    LCD_set_address(x1, y1, x2, y2);

    int pixel_count = (x2 - x1 + 1) * (y2 - y1 + 1);
    for (int i = 0; i < pixel_count; i++)
    {
        ST7735S_write_data16(color);
    }
}

// 查找字符索引的辅助函数
int find_ascii_index(char ch)
{
    for (int i = 0; i < ascii_font_8x16_count; i++)
    {
        if (ascii_char_map[i] == ch)
        {
            return i;
        }
    }
    return -1; // 未找到
}




/*
*****************************************************
**************************应用层*********************
*****************************************************
*/

void LCD_setup_init(void)
{
    app_spi_init_pin(); 
    app_spi_master_init_config(); 
    LCD_init(); 
}
// 显示图片
void showimage(const unsigned char *p, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if (x + width > LCD_WIDTH || y + height > LCD_HEIGHT)
    {
        osal_printk("Image out of bounds\n");
        return;
    }

    LCD_set_address(x, y, x + width - 1, y + height - 1);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int index = (i * width + j) * 2;
            uint8_t picL = *(p + index);
            uint8_t picH = *(p + index + 1);
            uint16_t color = (picH << 8) | picL;
            ST7735S_write_data16(color);
        }
    }
    osal_printk("Image displayed\n");
}

// 显示单个汉字
void LCD_show_chinese_char(uint8_t x, uint8_t y, const char *chinese_char, uint16_t color, uint16_t bg_color)
{
    const uint8_t *font_data = NULL;

    // 在字库中查找汉字
    for (int i = 0; i < chinese_font_16x16_count; i++)
    {
        if (strcmp(chinese_font_16x16[i].utf8_code, chinese_char) == 0)
        {
            font_data = chinese_font_16x16[i].font_data;
            break;
        }
    }

    if (font_data == NULL)
    {
        osal_printk("Chinese character '%s' not found\n", chinese_char);
        return;
    }

    // 边界检查
    if (x + 16 > LCD_WIDTH || y + 16 > LCD_HEIGHT)
    {
        osal_printk("Chinese char out of bounds\n");
        return;
    }

    LCD_set_address(x, y, x + 15, y + 15);

    // 逐像素显示
    for (int row = 0; row < 16; row++)
    {
        uint8_t byte1 = font_data[row * 2];     // 第一个字节
        uint8_t byte2 = font_data[row * 2 + 1]; // 第二个字节

        // 显示第一个字节的8个像素
        for (int col = 0; col < 8; col++)
        {
            if (byte1 & (0x80 >> col))
            {
                ST7735S_write_data16(color);
            }
            else
            {
                ST7735S_write_data16(bg_color);
            }
        }

        // 显示第二个字节的8个像素
        for (int col = 0; col < 8; col++)
        {
            if (byte2 & (0x80 >> col))
            {
                ST7735S_write_data16(color);
            }
            else
            {
                ST7735S_write_data16(bg_color);
            }
        }
    }

    osal_printk("Chinese char '%s' displayed at (%d,%d)\n", chinese_char, x, y);
}

// 显示汉字字符串
void LCD_show_chinese_string(uint8_t x, uint8_t y, const char *chinese_string, uint16_t color, uint16_t bg_color)
{
    uint8_t current_x = x;
    uint8_t current_y = y;
    int len = strlen(chinese_string);
    int i = 0;

    while (i < len)
    {
        // UTF-8汉字通常占3个字节
        if ((chinese_string[i] & 0xE0) == 0xE0)
        {
            char single_char[4] = {0};
            single_char[0] = chinese_string[i];
            single_char[1] = chinese_string[i + 1];
            single_char[2] = chinese_string[i + 2];

            LCD_show_chinese_char(current_x, current_y, single_char, color, bg_color);

            current_x += 16;
            // 检查是否需要换行
            if (current_x + 16 > LCD_WIDTH)
            {
                current_x = x;
                current_y += 16;
            }

            i += 3;
        }
        else
        {
            // ASCII字符处理
            LCD_show_ascii_char(current_x, current_y, chinese_string[i], color, bg_color);
            current_x += 8;
            if (current_x + 8 > LCD_WIDTH)
            {
                current_x = x;
                current_y += 16;
            }
            i++;
        }
    }
}



/**
 * 显示ASCII字符
 * @param x X坐标
 * @param y Y坐标
 * @param ch 要显示的字符
 * @param color 前景色
 * @param bg_color 背景色
 */
void LCD_show_ascii_char(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color)
{
    int index = find_ascii_index(ch);
    if (index == -1)
    {
        osal_printk("ASCII character '%c' not found\n", ch);
        return;
    }

    const uint8_t *font_data = ascii_font_8x16[index];

        // 显示字符
    for (int row = 0; row < 16; row++)
    {
        uint8_t data = font_data[row];
        LCD_set_address(x, y + row, x + 7, y + row);

        for (int col = 0; col < 8; col++)
        {
            uint16_t pixel_color = (data & (0x80 >> col)) ? color : bg_color;
            ST7735S_write_data16(pixel_color);
        }
    }
}

/**
 * 绘制圆角矩形按钮
 * @param x 起始x坐标
 * @param y 起始y坐标
 * @param width 矩形宽度
 * @param height 矩形高度
 * @param radius 圆角半径
 * @param color 填充颜色
 * @param border_color 边框颜色
 * @param border_width 边框宽度
    */
void LCD_draw_rounded_rect_button(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                                  uint8_t radius, uint16_t color, uint16_t border_color,
                                  uint8_t border_width)
{
    // 参数校验
    if (x + width > LCD_WIDTH || y + height > LCD_HEIGHT)
        return;
    if (radius > width / 2 || radius > height / 2)
    {
        radius = (width < height ? width : height) / 2;
    }

    uint8_t i, j;
    int16_t dx, dy, distance_sq, radius_sq;

    // 计算半径的平方，避免开方运算
    radius_sq = radius * radius;

    // 遍历矩形区域内的每个像素
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            uint8_t pixel_x = x + j;
            uint8_t pixel_y = y + i;
            uint16_t pixel_color = color;
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

                // 如果距离大于圆角半径，则不绘制该像素
                if (distance_sq > radius_sq)
                {
                    is_inside = false;
                }
                else if (border_width > 0 && distance_sq > (radius - border_width) * (radius - border_width))
                {
                    // 如果在边框范围内
                    pixel_color = border_color;
                }
            }
            else
            {
                // 非圆角区域，判断是否为边框
                if (border_width > 0)
                {
                    if (i < border_width || i >= height - border_width ||
                        j < border_width || j >= width - border_width)
                    {
                        pixel_color = border_color;
                    }
                }
            }

            // 绘制像素
            if (is_inside)
            {
                LCD_draw_pixel(pixel_x, pixel_y, pixel_color);
            }
        }
    }

    osal_printk("Rounded rect button drawn at (%d,%d) size %dx%d\n", x, y, width, height);
}

// 通用字符串显示函数
void LCD_show_string(uint8_t x, uint8_t y, const char *string, uint16_t color, uint16_t bg_color)
{
    LCD_show_chinese_string(x, y, string, color, bg_color);
}

/**
 * 显示放大的汉字（2倍放大）
 * @param x 起始x坐标
 * @param y 起始y坐标
 * @param chinese_char 汉字字符串
 * @param color 前景色
 * @param bg_color 背景色
 * @param scale 放大倍数
 */
void LCD_show_chinese_char_scaled(uint8_t x, uint8_t y, const char *chinese_char,
                                  uint16_t color, uint16_t bg_color, uint8_t scale)
{
    const uint8_t *font_data = NULL;

    // 在字库中查找汉字
    for (int i = 0; i < chinese_font_16x16_count; i++)
    {
        if (strcmp(chinese_font_16x16[i].utf8_code, chinese_char) == 0)
        {
            font_data = chinese_font_16x16[i].font_data;
            break;
        }
    }

    if (font_data == NULL)
    {
        osal_printk("Chinese character '%s' not found\n", chinese_char);
        return;
    }

    // 边界检查
    if (x + 16 * scale > LCD_WIDTH || y + 16 * scale > LCD_HEIGHT)
    {
        osal_printk("Scaled Chinese char out of bounds\n");
        return;
    }

    // 逐像素显示，按比例放大
    for (int row = 0; row < 16; row++)
    {
        uint8_t byte1 = font_data[row * 2];     // 第一个字节
        uint8_t byte2 = font_data[row * 2 + 1]; // 第二个字节

        // 每行重复scale次
        for (int row_repeat = 0; row_repeat < scale; row_repeat++)
        {
            LCD_set_address(x, y + row * scale + row_repeat,
                            x + 16 * scale - 1, y + row * scale + row_repeat);

            // 显示第一个字节的8个像素
            for (int col = 0; col < 8; col++)
            {
                uint16_t pixel_color = (byte1 & (0x80 >> col)) ? color : bg_color;
                // 每个像素重复scale次
                for (int col_repeat = 0; col_repeat < scale; col_repeat++)
                {
                    ST7735S_write_data16(pixel_color);
                }
            }

            // 显示第二个字节的8个像素
            for (int col = 0; col < 8; col++)
            {
                uint16_t pixel_color = (byte2 & (0x80 >> col)) ? color : bg_color;
                // 每个像素重复scale次
                for (int col_repeat = 0; col_repeat < scale; col_repeat++)
                {
                    ST7735S_write_data16(pixel_color);
                }
            }
        }
    }

    osal_printk("Scaled Chinese char '%s' displayed at (%d,%d) scale=%d\n", chinese_char, x, y, scale);
}

/**
 * 在左侧面板竖向显示英文单词
 * @param word 要显示的英文单词
 * @param start_y 起始Y坐标
 */
void draw_vertical_english_word(const char *word, uint8_t start_y)
{
    int len = strlen(word);
    uint8_t char_x = 16;       // 水平居中位置 (40-8)/2 = 16
    uint8_t char_spacing = 12; // 字符间距

    // 逐个字符竖向显示
    for (int i = 0; i < len; i++)
    {
        uint8_t char_y = start_y + i * char_spacing;

        // 边界检查
        if (char_y + 16 > 127)
            break;

        // ASCII字符显示函数
        LCD_show_ascii_char(char_x, char_y, word[i], 0xFFFF, 0x064F);
    }
}


void LCD_deinit(void)
{
    // 1. 关闭显示
    ST7735S_write_cmd(0x01);
    osal_msleep(120);

    ST7735S_write_cmd(0x28); // Display OFF
    osal_msleep(10);

    // 2. 进入睡眠模式
    ST7735S_write_cmd(0x10); // Sleep IN
    osal_msleep(120);        // 等待进入睡眠模式

    // 3. 硬件复位屏幕到低功耗状态
    LCD_RESET_LOW;
    osal_msleep(10);

    // 4. 释放SPI资源
    uapi_spi_deinit(SPI_MASTER_BUS_ID);

    // 5. 设置所有控制引脚为输入模式以降低功耗
    // CS引脚设为高阻态
    uapi_gpio_set_dir(SPI_CS_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_mode(SPI_CS_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);

    // DC引脚设为高阻态
    uapi_gpio_set_dir(SPI_DC_MASTER_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_mode(SPI_DC_MASTER_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);

    // RST引脚保持低电平（复位状态）
    uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_LOW);
    uapi_gpio_set_dir(SPI_RST_MASTER_PIN, GPIO_DIRECTION_OUTPUT);

    // SPI数据和时钟引脚设为高阻态
    uapi_pin_set_mode(SPI_SDA_MASTER_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);
    uapi_pin_set_mode(SPI_CLK_MASTER_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);

    osal_printk("TFT LCD deinit done - entered low power mode\n");
}

void LCD_enter_ultra_low_power_black(void)
{
    osal_printk("[LCD] Progressive low power black mode...\n");
    
    // 第一步：设置黑屏
    LCD_display_full(0x0000);
    osal_msleep(100);
    osal_printk("[LCD] Step 1: Black screen set\n");
    
    // 第二步：降低帧率
    ST7735S_write_cmd(0xB1);  // 正常模式帧率控制
    ST7735S_write_data8(0x05); // 降低帧率
    ST7735S_write_data8(0x3C);
    ST7735S_write_data8(0x3C);
    osal_msleep(50);
    osal_printk("[LCD] Step 2: Frame rate reduced\n");
    
    // 第三步：进入空闲模式
    ST7735S_write_cmd(0x39);  // IDMON
    osal_msleep(30);
    
    ST7735S_write_cmd(0xB2);  // 空闲模式帧率控制
    ST7735S_write_data8(0x08);
    ST7735S_write_data8(0x08);
    ST7735S_write_data8(0x08);
    osal_msleep(30);
    osal_printk("[LCD] Step 3: Idle mode activated\n");
    
    // 第四步：轻微降低电压
    ST7735S_write_cmd(0xC1);  // 电源控制2
    ST7735S_write_data8(0x95); // 轻微降低
    osal_msleep(30);
    
    ST7735S_write_cmd(0xC5);  // VCOM控制
    ST7735S_write_data8(0x08);
    osal_msleep(30);
    osal_printk("[LCD] Step 4: Power slightly reduced\n");
    
    osal_printk("[LCD] Progressive low power mode completed\n");
}