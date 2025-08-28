#ifndef __TFT_H
#define __TFT_H

#include "common_def.h"
#include "soc_osal.h"
#include "hal_gpio.h"
#include "gpio.h"

// LCD 尺寸定义
#define LCD_WIDTH 160
#define LCD_HEIGHT 128

// SPI 引脚定义
#define SPI_CLK_MASTER_PIN 14
#define SPI_SDA_MASTER_PIN 11
#define SPI_CS_PIN 13
#define SPI_DC_MASTER_PIN 15
#define SPI_RST_MASTER_PIN 12

// SPI总线配置
#define SPI_MASTER_BUS_ID 0
#define SPI_BUS_0 0

// LCD控制宏
#define LCD_CS_LOW uapi_gpio_set_val(SPI_CS_PIN, GPIO_LEVEL_LOW)
#define LCD_CS_HIGH uapi_gpio_set_val(SPI_CS_PIN, GPIO_LEVEL_HIGH)
#define LCD_DC_LOW uapi_gpio_set_val(SPI_DC_MASTER_PIN, GPIO_LEVEL_LOW)
#define LCD_DC_HIGH uapi_gpio_set_val(SPI_DC_MASTER_PIN, GPIO_LEVEL_HIGH)
#define LCD_RESET_LOW uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_LOW)
#define LCD_RESET_HIGH uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_HIGH)

// 颜色定义 (RGB565格式)
#define WHITE 0xFFFF
#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define YELLOW 0xFFE0
#define CYAN 0x07FF
#define MAGENTA 0xF81F

// SPI初始化函数
void app_spi_init_pin(void);
void app_spi_master_init_config(void);

// LCD硬件控制函数
void lcd_reset(void);
void st7735s_write_cmd(uint8_t cmd);
void st7735s_write_data8(uint8_t dat);
void st7735s_write_data16(uint16_t color);

// LCD初始化和控制函数
void lcd_init(void);
void lcd_set_address(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color);
void lcd_display_full(uint16_t color);
void lcd_display(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, 
                 uint16_t color);

// 应用层接口
void lcd_setup_init(void);
void lcd_setup_deinit(void);

// 电源管理函数
void lcd_deinit(void);
void lcd_enter_ultra_low_power_black(void);
void lcd_sleep(void);
void lcd_restore(void);

#endif // __TFT_H