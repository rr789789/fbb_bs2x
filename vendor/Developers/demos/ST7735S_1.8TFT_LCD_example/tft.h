#ifndef TFT_H
#define TFT_H

#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "boards.h"
#include "pinctrl.h"
#include "gpio.h"
#include <stdbool.h>

// SPI引脚定义
#define SPI_CS_PIN 13
#define SPI_RST_MASTER_PIN 12
#define SPI_SDA_MASTER_PIN 11
#define SPI_CLK_MASTER_PIN 14
#define SPI_DC_MASTER_PIN 15
#define SPI_MASTER_BUS_ID 0

// ST7735S 160x128横屏显示配置
#define LCD_WIDTH 160
#define LCD_HEIGHT 128

// TFT屏幕SPI引脚宏定义
#define LCD_SCL_HIGH uapi_gpio_set_val(SPI_CLK_MASTER_PIN, GPIO_LEVEL_HIGH)
#define LCD_SCL_LOW uapi_gpio_set_val(SPI_CLK_MASTER_PIN, GPIO_LEVEL_LOW)
#define LCD_SDA_HIGH uapi_gpio_set_val(SPI_SDA_MASTER_PIN, GPIO_LEVEL_HIGH)
#define LCD_SDA_LOW uapi_gpio_set_val(SPI_SDA_MASTER_PIN, GPIO_LEVEL_LOW)
#define LCD_CS_HIGH uapi_gpio_set_val(SPI_CS_PIN, GPIO_LEVEL_HIGH)
#define LCD_CS_LOW uapi_gpio_set_val(SPI_CS_PIN, GPIO_LEVEL_LOW)
#define LCD_DC_HIGH uapi_gpio_set_val(SPI_DC_MASTER_PIN, GPIO_LEVEL_HIGH)
#define LCD_DC_LOW uapi_gpio_set_val(SPI_DC_MASTER_PIN, GPIO_LEVEL_LOW)
#define LCD_RESET_HIGH uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_HIGH)
#define LCD_RESET_LOW uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_LOW)

// SPI配置参数
#define SPI_SLAVE_NUM 1
#define SPI_CLK_POLARITY 0
#define SPI_CLK_PHASE 0
#define SPI_FRAME_FORMAT 0
#define SPI_FRAME_FORMAT_STANDARD 0
#define SPI_TMOD 0
#define SPI_WAIT_CYCLES 0x10
#define SPI_BUS_0 0

// 常用颜色定义 (RGB565格式)
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GRAY 0x7BEF

// 字体大小定义
#define FONT_SIZE_16 16 // 16x16像素汉字
#define FONT_SIZE_12 12 // 12x12像素汉字
#define FONT_SIZE_8 8   // 8x16像素ASCII字符

// SPI初始化和配置函数
void app_spi_init_pin(void);
void app_spi_master_init_config(void);

// ST7735S底层驱动函数
void lcd_reset(void);
void ST7735S_write_data8(uint8_t dat);
void ST7735S_write_data16(uint16_t color);
void ST7735S_write_cmd(uint8_t cmd);

// LCD初始化和基础控制函数
void LCD_init(void);
void LCD_set_address(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

// 基础绘图函数
void LCD_draw_pixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_display_full(uint16_t color);
void LCD_display(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);

// 图像显示函数
void showimage(const unsigned char *p, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

// 汉字和文本显示函数
void LCD_show_chinese_char(uint8_t x, uint8_t y, const char *chinese_char, uint16_t color, uint16_t bg_color);
void LCD_show_chinese_string(uint8_t x, uint8_t y, const char *chinese_string, uint16_t color, uint16_t bg_color);
void LCD_show_ascii_char(uint8_t x, uint8_t y, char ascii_char, uint16_t color, uint16_t bg_color);
void LCD_show_string(uint8_t x, uint8_t y, const char *string, uint16_t color, uint16_t bg_color);
int find_ascii_index(char ch);
// 高级绘图函数
void LCD_draw_rounded_rect_button(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                                  uint8_t radius, uint16_t color, uint16_t border_color,
                                  uint8_t border_width);
void LCD_show_chinese_char_scaled(uint8_t x, uint8_t y, const char *chinese_char,
                                  uint16_t color, uint16_t bg_color, uint8_t scale);
void draw_vertical_english_word(const char *word, uint8_t start_y);
void LCD_setup_init(void);

void LCD_deinit(void);
// 兼容性定义（保持向后兼容）
#define WHITE COLOR_WHITE

#endif // TFT_H