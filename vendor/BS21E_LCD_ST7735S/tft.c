#include "tft.h"
#include "pinctrl.h"
#include "spi.h"
#include "hal_gpio.h"
#include "chinese_font.h"

// ST7735S 160x128横屏显示配置
#define LCD_WIDTH 160
#define LCD_HEIGHT 128

// SPI引脚配置常量
#define SDA_MODE 2
#define CLK_MODE 3

// LCD时钟和频率配置
#define LCD_BUS_CLK 32000000
#define LCD_FREQ_MHZ 8

// 延时时间常量
#define SLEEP_10MS 10
#define SLEEP_30MS 30
#define SLEEP_50MS 50
#define SLEEP_100MS 100
#define SLEEP_120MS 120
#define LCD_INIT_DELAY1 150
#define LCD_INIT_DELAY2 255

// SPI缓冲区和传输配置
#define SPI_BUF_SIZE 100
#define SPI_TX_BYTES_SINGLE 1
#define SPI_RX_BYTES_ZERO 0

// LCD ST7735S 命令定义
#define CMD_SWRESET 0x01    // 软件复位
#define CMD_SLPOUT 0x11     // 退出睡眠模式
#define CMD_FRMCTR1 0xB1    // 帧率控制 - 正常模式
#define CMD_FRMCTR2 0xB2    // 帧率控制 - 空闲模式
#define CMD_FRMCTR3 0xB3    // 帧率控制 - 部分模式
#define CMD_INVCTR 0xB4     // 显示反转控制
#define CMD_PWCTR1 0xC0     // 电源控制1
#define CMD_PWCTR2 0xC1     // 电源控制2
#define CMD_PWCTR3 0xC2     // 电源控制3
#define CMD_PWCTR4 0xC3     // 电源控制4
#define CMD_PWCTR5 0xC4     // 电源控制5
#define CMD_VMCTR1 0xC5     // VCOM控制1
#define CMD_MADCTL 0x36     // 内存访问控制
#define CMD_COLMOD 0x3A     // 像素格式设置
#define CMD_GMCTRP1 0xE0    // 正极性Gamma校正
#define CMD_GMCTRN1 0xE1    // 负极性Gamma校正
#define CMD_NORON 0x13      // 正常显示模式
#define CMD_DISPON 0x29     // 显示开启
#define CMD_DISPOFF 0x28    // 显示关闭
#define CMD_SLPIN 0x10      // 进入睡眠模式
#define CMD_CASET 0x2A      // 列地址设置
#define CMD_RASET 0x2B      // 行地址设置
#define CMD_RAMWR 0x2C      // 内存写入
#define CMD_IDMON 0x39      // 空闲模式开启

// 帧率控制参数
#define FRMCTR_PARAM1 0x01
#define FRMCTR_PARAM2 0x2C
#define FRMCTR_PARAM3 0x2D

// 显示反转控制参数
#define INVCTR_PARAM 0x07

// 电源控制参数
#define PWCTR1_PARAM1 0xA2
#define PWCTR1_PARAM2 0x02
#define PWCTR1_PARAM3 0x84
#define PWCTR2_PARAM1 0xC5
#define PWCTR3_PARAM1 0x0A
#define PWCTR3_PARAM2 0x00
#define PWCTR4_PARAM1 0x8A
#define PWCTR4_PARAM2 0x2A
#define PWCTR5_PARAM1 0x8A
#define PWCTR5_PARAM2 0xEE
#define VMCTR1_PARAM 0x0E

// 内存访问控制参数（横屏显示）
#define MADCTL_LANDSCAPE 0xA0  // MY=1, MX=0, MV=1 for landscape

// 像素格式参数
#define COLMOD_16BIT 0x05  // 16位/像素

// Gamma校正参数 - 正极性
#define GMCTRP1_PARAM_COUNT 16
#define GMCTRP1_PARAM1 0x02
#define GMCTRP1_PARAM2 0x1C
#define GMCTRP1_PARAM3 0x07
#define GMCTRP1_PARAM4 0x12
#define GMCTRP1_PARAM5 0x37
#define GMCTRP1_PARAM6 0x32
#define GMCTRP1_PARAM7 0x29
#define GMCTRP1_PARAM8 0x2D
#define GMCTRP1_PARAM9 0x29
#define GMCTRP1_PARAM10 0x25
#define GMCTRP1_PARAM11 0x2B
#define GMCTRP1_PARAM12 0x39
#define GMCTRP1_PARAM13 0x00
#define GMCTRP1_PARAM14 0x01
#define GMCTRP1_PARAM15 0x03
#define GMCTRP1_PARAM16 0x10

// Gamma校正参数 - 负极性
#define GMCTRN1_PARAM_COUNT 16
#define GMCTRN1_PARAM1 0x03
#define GMCTRN1_PARAM2 0x1D
#define GMCTRN1_PARAM3 0x07
#define GMCTRN1_PARAM4 0x06
#define GMCTRN1_PARAM5 0x2E
#define GMCTRN1_PARAM6 0x2C
#define GMCTRN1_PARAM7 0x29
#define GMCTRN1_PARAM8 0x2D
#define GMCTRN1_PARAM9 0x2E
#define GMCTRN1_PARAM10 0x2E
#define GMCTRN1_PARAM11 0x37
#define GMCTRN1_PARAM12 0x3F
#define GMCTRN1_PARAM13 0x00
#define GMCTRN1_PARAM14 0x00
#define GMCTRN1_PARAM15 0x02
#define GMCTRN1_PARAM16 0x10

// 颜色位操作常量
#define COLOR_HIGH_BYTE_SHIFT 8
#define COLOR_LOW_BYTE_MASK 0xFF
#define BYTE_HIGH_BIT 0x80
#define BYTE_BITS_COUNT 8
#define RGB565_DATA_SIZE 2

// 字体相关常量
#define ASCII_CHAR_WIDTH 8
#define ASCII_CHAR_HEIGHT 16
#define CHINESE_CHAR_WIDTH 16
#define CHINESE_CHAR_HEIGHT 16
#define CHINESE_CHAR_BYTES_PER_ROW 2
#define UTF8_CHINESE_BYTE_MASK 0xE0
#define UTF8_CHINESE_BYTE_PREFIX 0xE0
#define UTF8_CHINESE_CHAR_SIZE 3
#define STRING_TERM_NULL 4

// 圆角矩形相关常量
#define CORNER_RADIUS_MAX_RATIO 2

// 低功耗模式参数
#define LOW_POWER_FRMCTR_PARAM1 0x05
#define LOW_POWER_FRMCTR_PARAM2 0x3C
#define LOW_POWER_FRMCTR_PARAM3 0x3C
#define LOW_POWER_IDLE_FRMCTR_PARAM 0x08
#define LOW_POWER_PWCTR2_PARAM 0x95
#define LOW_POWER_VMCTR1_PARAM 0x08

// 数组索引和大小相关
#define ARRAY_INDEX_0 0
#define ARRAY_INDEX_1 1
#define ARRAY_INDEX_2 2
#define INVALID_INDEX -1
#define PIXEL_COUNT_MULTIPLIER 2

// 边界值常量
#define LCD_WIDTH_MINUS_ONE 159  // LCD_WIDTH - 1
#define LCD_HEIGHT_MINUS_ONE 127 // LCD_HEIGHT - 1

// 循环计数和字符串处理
#define CHAR_SPACING_VERTICAL 12
#define LEFT_PANEL_CHAR_X_POS 16
#define LCD_MAX_Y_BOUNDARY 127

// SPI 配置常量
#define SPI_SLAVE_NUM_ONE 1
#define SPI_WAIT_CYCLES_DEFAULT 0
#define SPI_EXTRA_CONFIG_DEFAULT 0

// GPIO 电平常量
#define GPIO_LOW_LEVEL 0
#define GPIO_HIGH_LEVEL 1

// 地址设置参数
#define ADDR_HIGH_BYTE 0x00

// 像素计算相关
#define PIXEL_CALCULATION_OFFSET 1

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

    uapi_pin_set_mode(SPI_SDA_MASTER_PIN, SDA_MODE);
    uapi_pin_set_mode(SPI_CLK_MASTER_PIN, CLK_MODE);
    uapi_pin_set_ds(SPI_CLK_MASTER_PIN, CLK_MODE);
}

void app_spi_master_init_config(void)
{
    spi_attr_t config = {ARRAY_INDEX_0};
    spi_extra_attr_t ext_config = {ARRAY_INDEX_0};

    config.is_slave = false;
    config.slave_num = SPI_SLAVE_NUM_ONE;
    config.bus_clk = LCD_BUS_CLK; // 32MHz
    config.freq_mhz = LCD_FREQ_MHZ;
    config.clk_polarity = SPI_CFG_CLK_CPOL_0;
    config.clk_phase = SPI_CFG_CLK_CPHA_0;
    config.frame_format = SPI_CFG_FRAME_FORMAT_MOTOROLA_SPI;
    config.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD;
    config.frame_size = HAL_SPI_FRAME_SIZE_8;
    config.tmod = HAL_SPI_TRANS_MODE_TX;
    config.sste = ARRAY_INDEX_0;
    ext_config.qspi_param.wait_cycles = SPI_WAIT_CYCLES_DEFAULT;

    int ret = uapi_spi_init(SPI_MASTER_BUS_ID, &config, &ext_config);
    if (ret != ARRAY_INDEX_0)
    {
        osal_printk("SPI init failed: 0x%x\n", ret);
    }
}

// ST7735S复位函数
void lcd_reset(void)
{
    LCD_RESET_LOW;
    osal_msleep(SLEEP_50MS);
    LCD_RESET_HIGH;
    osal_msleep(SLEEP_120MS);
}

// 写8位数据
void ST7735S_write_data8(uint8_t dat)
{
    LCD_CS_LOW;
    LCD_DC_HIGH; // 数据模式

    spi_xfer_data_t data;
    data.tx_buff = &dat;
    data.tx_bytes = SPI_TX_BYTES_SINGLE;
    data.rx_buff = NULL;
    data.rx_bytes = SPI_RX_BYTES_ZERO;

    uapi_spi_master_write(SPI_BUS_0, &data, SPI_BUF_SIZE);
    LCD_CS_HIGH;
}

// 写命令
void ST7735S_write_cmd(uint8_t cmd)
{
    LCD_CS_LOW;
    LCD_DC_LOW; // 命令模式

    spi_xfer_data_t data;
    data.tx_buff = &cmd;
    data.tx_bytes = SPI_TX_BYTES_SINGLE;
    data.rx_buff = NULL;
    data.rx_bytes = SPI_RX_BYTES_ZERO;

    uapi_spi_master_write(SPI_BUS_0, &data, SPI_BUF_SIZE);
    LCD_CS_HIGH;
}

// 写16位颜色数据
void ST7735S_write_data16(uint16_t color)
{
    LCD_CS_LOW;
    LCD_DC_HIGH;

    uint8_t data_bytes[RGB565_DATA_SIZE];
    data_bytes[ARRAY_INDEX_0] = (uint8_t)(color >> COLOR_HIGH_BYTE_SHIFT);   // 高8位
    data_bytes[ARRAY_INDEX_1] = (uint8_t)(color & COLOR_LOW_BYTE_MASK);      // 低8位

    spi_xfer_data_t data = {
        .tx_buff = data_bytes,
        .tx_bytes = RGB565_DATA_SIZE,
        .rx_buff = NULL,
        .rx_bytes = SPI_RX_BYTES_ZERO};

    uapi_spi_master_write(SPI_BUS_0, &data, SPI_BUF_SIZE);
    LCD_CS_HIGH;
}

// ST7735S初始化
void LCD_init(void)
{
    lcd_reset();

    ST7735S_write_cmd(CMD_SWRESET); // 软件复位
    osal_mdelay(LCD_INIT_DELAY1);

    ST7735S_write_cmd(CMD_SLPOUT); // 退出睡眠模式
    osal_mdelay(LCD_INIT_DELAY2);

    ST7735S_write_cmd(CMD_FRMCTR1); // 帧率控制 - 正常模式
    ST7735S_write_data8(FRMCTR_PARAM1);
    ST7735S_write_data8(FRMCTR_PARAM2);
    ST7735S_write_data8(FRMCTR_PARAM3);

    ST7735S_write_cmd(CMD_FRMCTR2); // 帧率控制 - 空闲模式
    ST7735S_write_data8(FRMCTR_PARAM1);
    ST7735S_write_data8(FRMCTR_PARAM2);
    ST7735S_write_data8(FRMCTR_PARAM3);

    ST7735S_write_cmd(CMD_FRMCTR3); // 帧率控制 - 部分模式
    ST7735S_write_data8(FRMCTR_PARAM1);
    ST7735S_write_data8(FRMCTR_PARAM2);
    ST7735S_write_data8(FRMCTR_PARAM3);
    ST7735S_write_data8(FRMCTR_PARAM1);
    ST7735S_write_data8(FRMCTR_PARAM2);
    ST7735S_write_data8(FRMCTR_PARAM3);

    ST7735S_write_cmd(CMD_INVCTR); // 显示反转控制
    ST7735S_write_data8(INVCTR_PARAM);

    ST7735S_write_cmd(CMD_PWCTR1); // LCM控制
    ST7735S_write_data8(PWCTR1_PARAM1);
    ST7735S_write_data8(PWCTR1_PARAM2);
    ST7735S_write_data8(PWCTR1_PARAM3);

    ST7735S_write_cmd(CMD_PWCTR2); // LCM控制
    ST7735S_write_data8(PWCTR2_PARAM1);

    ST7735S_write_cmd(CMD_PWCTR3); // LCM控制
    ST7735S_write_data8(PWCTR3_PARAM1);
    ST7735S_write_data8(PWCTR3_PARAM2);

    ST7735S_write_cmd(CMD_PWCTR4); // LCM控制
    ST7735S_write_data8(PWCTR4_PARAM1);
    ST7735S_write_data8(PWCTR4_PARAM2);

    ST7735S_write_cmd(CMD_PWCTR5); // LCM控制
    ST7735S_write_data8(PWCTR5_PARAM1);
    ST7735S_write_data8(PWCTR5_PARAM2);

    ST7735S_write_cmd(CMD_VMCTR1); // VCOM控制
    ST7735S_write_data8(VMCTR1_PARAM);

    ST7735S_write_cmd(CMD_MADCTL);   // 内存访问控制 - 横屏显示
    ST7735S_write_data8(MADCTL_LANDSCAPE); // MY=1, MX=0, MV=1 for landscape

    ST7735S_write_cmd(CMD_COLMOD);   // 像素格式设置
    ST7735S_write_data8(COLMOD_16BIT); // 16位/像素

    // Gamma校正
    ST7735S_write_cmd(CMD_GMCTRP1); // 正极性Gamma校正
    ST7735S_write_data8(GMCTRP1_PARAM1);
    ST7735S_write_data8(GMCTRP1_PARAM2);
    ST7735S_write_data8(GMCTRP1_PARAM3);
    ST7735S_write_data8(GMCTRP1_PARAM4);
    ST7735S_write_data8(GMCTRP1_PARAM5);
    ST7735S_write_data8(GMCTRP1_PARAM6);
    ST7735S_write_data8(GMCTRP1_PARAM7);
    ST7735S_write_data8(GMCTRP1_PARAM8);
    ST7735S_write_data8(GMCTRP1_PARAM9);
    ST7735S_write_data8(GMCTRP1_PARAM10);
    ST7735S_write_data8(GMCTRP1_PARAM11);
    ST7735S_write_data8(GMCTRP1_PARAM12);
    ST7735S_write_data8(GMCTRP1_PARAM13);
    ST7735S_write_data8(GMCTRP1_PARAM14);
    ST7735S_write_data8(GMCTRP1_PARAM15);
    ST7735S_write_data8(GMCTRP1_PARAM16);

    ST7735S_write_cmd(CMD_GMCTRN1); // 负极性Gamma校正
    ST7735S_write_data8(GMCTRN1_PARAM1);
    ST7735S_write_data8(GMCTRN1_PARAM2);
    ST7735S_write_data8(GMCTRN1_PARAM3);
    ST7735S_write_data8(GMCTRN1_PARAM4);
    ST7735S_write_data8(GMCTRN1_PARAM5);
    ST7735S_write_data8(GMCTRN1_PARAM6);
    ST7735S_write_data8(GMCTRN1_PARAM7);
    ST7735S_write_data8(GMCTRN1_PARAM8);
    ST7735S_write_data8(GMCTRN1_PARAM9);
    ST7735S_write_data8(GMCTRN1_PARAM10);
    ST7735S_write_data8(GMCTRN1_PARAM11);
    ST7735S_write_data8(GMCTRN1_PARAM12);
    ST7735S_write_data8(GMCTRN1_PARAM13);
    ST7735S_write_data8(GMCTRN1_PARAM14);
    ST7735S_write_data8(GMCTRN1_PARAM15);
    ST7735S_write_data8(GMCTRN1_PARAM16);

    ST7735S_write_cmd(CMD_NORON); // 正常显示模式
    osal_mdelay(SLEEP_10MS);

    ST7735S_write_cmd(CMD_DISPON); // 显示开启
    osal_mdelay(SLEEP_100MS);

    osal_printk("ST7735S LCD init done - 160x128 landscape mode\n");
}

// 设置显示窗口
void LCD_set_address(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    // 边界检查
    if (x1 >= LCD_WIDTH)
        x1 = LCD_WIDTH_MINUS_ONE;
    if (y1 >= LCD_HEIGHT)
        y1 = LCD_HEIGHT_MINUS_ONE;
    if (x2 >= LCD_WIDTH)
        x2 = LCD_WIDTH_MINUS_ONE;
    if (y2 >= LCD_HEIGHT)
        y2 = LCD_HEIGHT_MINUS_ONE;

    ST7735S_write_cmd(CMD_CASET); // 列地址设置
    ST7735S_write_data8(ADDR_HIGH_BYTE);
    ST7735S_write_data8(x1);
    ST7735S_write_data8(ADDR_HIGH_BYTE);
    ST7735S_write_data8(x2);

    ST7735S_write_cmd(CMD_RASET); // 行地址设置
    ST7735S_write_data8(ADDR_HIGH_BYTE);
    ST7735S_write_data8(y1);
    ST7735S_write_data8(ADDR_HIGH_BYTE);
    ST7735S_write_data8(y2);

    ST7735S_write_cmd(CMD_RAMWR); // 内存写入
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
    LCD_set_address(ARRAY_INDEX_0, ARRAY_INDEX_0, LCD_WIDTH_MINUS_ONE, LCD_HEIGHT_MINUS_ONE);

    int total_pixels = LCD_WIDTH * LCD_HEIGHT;
    for (int i = ARRAY_INDEX_0; i < total_pixels; i++)
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

    int pixel_count = (x2 - x1 + PIXEL_CALCULATION_OFFSET) * (y2 - y1 + PIXEL_CALCULATION_OFFSET);
    for (int i = ARRAY_INDEX_0; i < pixel_count; i++)
    {
        ST7735S_write_data16(color);
    }
}

// 查找字符索引的辅助函数
int find_ascii_index(char ch)
{
    for (int i = ARRAY_INDEX_0; i < ascii_font_8x16_count; i++)
    {
        if (ascii_char_map[i] == ch)
        {
            return i;
        }
    }
    return INVALID_INDEX; // 未找到
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

    LCD_set_address(x, y, x + width - PIXEL_CALCULATION_OFFSET, y + height - PIXEL_CALCULATION_OFFSET);

    for (int i = ARRAY_INDEX_0; i < height; i++)
    {
        for (int j = ARRAY_INDEX_0; j < width; j++)
        {
            int index = (i * width + j) * PIXEL_COUNT_MULTIPLIER;
            uint8_t picL = *(p + index);
            uint8_t picH = *(p + index + PIXEL_CALCULATION_OFFSET);
            uint16_t color = (picH << COLOR_HIGH_BYTE_SHIFT) | picL;
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
    for (int i = ARRAY_INDEX_0; i < chinese_font_16x16_count; i++)
    {
        if (strcmp(chinese_font_16x16[i].utf8_code, chinese_char) == ARRAY_INDEX_0)
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
    if (x + CHINESE_CHAR_WIDTH > LCD_WIDTH || y + CHINESE_CHAR_HEIGHT > LCD_HEIGHT)
    {
        osal_printk("Chinese char out of bounds\n");
        return;
    }

    LCD_set_address(x, y, x + CHINESE_CHAR_WIDTH - PIXEL_CALCULATION_OFFSET, y + CHINESE_CHAR_HEIGHT - PIXEL_CALCULATION_OFFSET);

    // 逐像素显示
    for (int row = ARRAY_INDEX_0; row < CHINESE_CHAR_HEIGHT; row++)
    {
        uint8_t byte1 = font_data[row * CHINESE_CHAR_BYTES_PER_ROW];     // 第一个字节
        uint8_t byte2 = font_data[row * CHINESE_CHAR_BYTES_PER_ROW + PIXEL_CALCULATION_OFFSET]; // 第二个字节

        // 显示第一个字节的8个像素
        for (int col = ARRAY_INDEX_0; col < BYTE_BITS_COUNT; col++)
        {
            if (byte1 & (BYTE_HIGH_BIT >> col))
            {
                ST7735S_write_data16(color);
            }
            else
            {
                ST7735S_write_data16(bg_color);
            }
        }

        // 显示第二个字节的8个像素
        for (int col = ARRAY_INDEX_0; col < BYTE_BITS_COUNT; col++)
        {
            if (byte2 & (BYTE_HIGH_BIT >> col))
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
    int i = ARRAY_INDEX_0;

    while (i < len)
    {
        // UTF-8汉字通常占3个字节
        if ((chinese_string[i] & UTF8_CHINESE_BYTE_MASK) == UTF8_CHINESE_BYTE_PREFIX)
        {
            char single_char[UTF8_CHINESE_CHAR_SIZE + PIXEL_CALCULATION_OFFSET] = {ARRAY_INDEX_0};
            single_char[ARRAY_INDEX_0] = chinese_string[i];
            single_char[ARRAY_INDEX_1] = chinese_string[i + PIXEL_CALCULATION_OFFSET];
            single_char[ARRAY_INDEX_2] = chinese_string[i + PIXEL_COUNT_MULTIPLIER];

            LCD_show_chinese_char(current_x, current_y, single_char, color, bg_color);

            current_x += CHINESE_CHAR_WIDTH;
            // 检查是否需要换行
            if (current_x + CHINESE_CHAR_WIDTH > LCD_WIDTH)
            {
                current_x = x;
                current_y += CHINESE_CHAR_HEIGHT;
            }

            i += UTF8_CHINESE_CHAR_SIZE;
        }
        else
        {
            // ASCII字符处理
            LCD_show_ascii_char(current_x, current_y, chinese_string[i], color, bg_color);
            current_x += ASCII_CHAR_WIDTH;
            if (current_x + ASCII_CHAR_WIDTH > LCD_WIDTH)
            {
                current_x = x;
                current_y += ASCII_CHAR_HEIGHT;
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
    if (index == INVALID_INDEX)
    {
        osal_printk("ASCII character '%c' not found\n", ch);
        return;
    }

    const uint8_t *font_data = ascii_font_8x16[index];

    // 显示字符
    for (int row = ARRAY_INDEX_0; row < ASCII_CHAR_HEIGHT; row++)
    {
        uint8_t data = font_data[row];
        LCD_set_address(x, y + row, x + ASCII_CHAR_WIDTH - PIXEL_CALCULATION_OFFSET, y + row);

        for (int col = ARRAY_INDEX_0; col < ASCII_CHAR_WIDTH; col++)
        {
            uint16_t pixel_color = (data & (BYTE_HIGH_BIT >> col)) ? color : bg_color;
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
    if (radius > width / CORNER_RADIUS_MAX_RATIO || radius > height / CORNER_RADIUS_MAX_RATIO)
    {
        radius = (width < height ? width : height) / CORNER_RADIUS_MAX_RATIO;
    }

    uint8_t i, j;
    int16_t dx, dy, distance_sq, radius_sq;

    // 计算半径的平方，避免开方运算
    radius_sq = radius * radius;

    // 遍历矩形区域内的每个像素
    for (i = ARRAY_INDEX_0; i < height; i++)
    {
        for (j = ARRAY_INDEX_0; j < width; j++)
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
                    dx = j - (width - radius - PIXEL_CALCULATION_OFFSET);
                    dy = i - radius;
                }
                else if (i >= height - radius && j < radius)
                {
                    // 左下角圆心
                    dx = j - radius;
                    dy = i - (height - radius - PIXEL_CALCULATION_OFFSET);
                }
                else
                {
                    // 右下角圆心
                    dx = j - (width - radius - PIXEL_CALCULATION_OFFSET);
                    dy = i - (height - radius - PIXEL_CALCULATION_OFFSET);
                }

                distance_sq = dx * dx + dy * dy;

                // 如果距离大于圆角半径，则不绘制该像素
                if (distance_sq > radius_sq)
                {
                    is_inside = false;
                }
                else if (border_width > ARRAY_INDEX_0 && distance_sq > (radius - border_width) * (radius - border_width))
                {
                    // 如果在边框范围内
                    pixel_color = border_color;
                }
            }
            else
            {
                // 非圆角区域，判断是否为边框
                if (border_width > ARRAY_INDEX_0)
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
 * 显示放大的汉字（可指定放大倍数）
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
    for (int i = ARRAY_INDEX_0; i < chinese_font_16x16_count; i++)
    {
        if (strcmp(chinese_font_16x16[i].utf8_code, chinese_char) == ARRAY_INDEX_0)
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
    if (x + CHINESE_CHAR_WIDTH * scale > LCD_WIDTH || y + CHINESE_CHAR_HEIGHT * scale > LCD_HEIGHT)
    {
        osal_printk("Scaled Chinese char out of bounds\n");
        return;
    }

    // 逐像素显示，按比例放大
    for (int row = ARRAY_INDEX_0; row < CHINESE_CHAR_HEIGHT; row++)
    {
        uint8_t byte1 = font_data[row * CHINESE_CHAR_BYTES_PER_ROW];     // 第一个字节
        uint8_t byte2 = font_data[row * CHINESE_CHAR_BYTES_PER_ROW + PIXEL_CALCULATION_OFFSET]; // 第二个字节

        // 每行重复scale次
        for (int row_repeat = ARRAY_INDEX_0; row_repeat < scale; row_repeat++)
        {
            LCD_set_address(x, y + row * scale + row_repeat,
                            x + CHINESE_CHAR_WIDTH * scale - PIXEL_CALCULATION_OFFSET, y + row * scale + row_repeat);

            // 显示第一个字节的8个像素
            for (int col = ARRAY_INDEX_0; col < BYTE_BITS_COUNT; col++)
            {
                uint16_t pixel_color = (byte1 & (BYTE_HIGH_BIT >> col)) ? color : bg_color;
                // 每个像素重复scale次
                for (int col_repeat = ARRAY_INDEX_0; col_repeat < scale; col_repeat++)
                {
                    ST7735S_write_data16(pixel_color);
                }
            }

            // 显示第二个字节的8个像素
            for (int col = ARRAY_INDEX_0; col < BYTE_BITS_COUNT; col++)
            {
                uint16_t pixel_color = (byte2 & (BYTE_HIGH_BIT >> col)) ? color : bg_color;
                // 每个像素重复scale次
                for (int col_repeat = ARRAY_INDEX_0; col_repeat < scale; col_repeat++)
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
    uint8_t char_x = LEFT_PANEL_CHAR_X_POS;       // 水平居中位置 (40-8)/2 = 16
    uint8_t char_spacing = CHAR_SPACING_VERTICAL; // 字符间距

    // 逐个字符竖向显示
    for (int i = ARRAY_INDEX_0; i < len; i++)
    {
        uint8_t char_y = start_y + i * char_spacing;

        // 边界检查
        if (char_y + ASCII_CHAR_HEIGHT > LCD_MAX_Y_BOUNDARY)
            break;

        // ASCII字符显示函数
        LCD_show_ascii_char(char_x, char_y, word[i], 0xFFFF, 0x064F);
    }
}

void LCD_setup_deinit(void)
{
    LCD_deinit();
}

void LCD_deinit(void)
{
    // 1. 关闭显示
    ST7735S_write_cmd(CMD_SWRESET);
    osal_msleep(SLEEP_120MS);

    ST7735S_write_cmd(CMD_DISPOFF); // Display OFF
    osal_msleep(SLEEP_10MS);

    // 2. 进入睡眠模式
    ST7735S_write_cmd(CMD_SLPIN); // Sleep IN
    osal_msleep(SLEEP_120MS);        // 等待进入睡眠模式

    // 3. 硬件复位屏幕到低功耗状态
    LCD_RESET_LOW;
    osal_msleep(SLEEP_10MS);

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
    osal_msleep(SLEEP_100MS);
    osal_printk("[LCD] Step 1: Black screen set\n");
    
    // 第二步：降低帧率
    ST7735S_write_cmd(CMD_FRMCTR1);  // 正常模式帧率控制
    ST7735S_write_data8(LOW_POWER_FRMCTR_PARAM1); // 降低帧率
    ST7735S_write_data8(LOW_POWER_FRMCTR_PARAM2);
    ST7735S_write_data8(LOW_POWER_FRMCTR_PARAM3);
    osal_msleep(SLEEP_50MS);
    osal_printk("[LCD] Step 2: Frame rate reduced\n");
    
    // 第三步：进入空闲模式
    ST7735S_write_cmd(CMD_IDMON);  // IDMON
    osal_msleep(SLEEP_30MS);
    
    ST7735S_write_cmd(CMD_FRMCTR2);  // 空闲模式帧率控制
    ST7735S_write_data8(LOW_POWER_IDLE_FRMCTR_PARAM);
    ST7735S_write_data8(LOW_POWER_IDLE_FRMCTR_PARAM);
    ST7735S_write_data8(LOW_POWER_IDLE_FRMCTR_PARAM);
    osal_msleep(SLEEP_30MS);
    osal_printk("[LCD] Step 3: Idle mode activated\n");
    
    // 第四步：轻微降低电压
    ST7735S_write_cmd(CMD_PWCTR2);  // 电源控制2
    ST7735S_write_data8(LOW_POWER_PWCTR2_PARAM); // 轻微降低
    osal_msleep(SLEEP_30MS);
    
    ST7735S_write_cmd(CMD_VMCTR1);  // VCOM控制
    ST7735S_write_data8(LOW_POWER_VMCTR1_PARAM);
    osal_msleep(SLEEP_30MS);
    osal_printk("[LCD] Step 4: Power slightly reduced\n");
    
    osal_printk("[LCD] Progressive low power mode completed\n");
}

void LCD_sleep(void)
{
    ST7735S_write_cmd(CMD_DISPOFF); // 显示关闭
    osal_msleep(SLEEP_10MS);
    ST7735S_write_cmd(CMD_SLPIN);   // 进入睡眠
    osal_msleep(SLEEP_120MS);
    osal_printk("LCD entered sleep mode\n");
}

void LCD_restore(void)
{
    ST7735S_write_cmd(CMD_SLPOUT);  // 退出睡眠
    osal_msleep(SLEEP_120MS);
    ST7735S_write_cmd(CMD_DISPON);  // 显示开启
    osal_msleep(SLEEP_10MS);
    osal_printk("LCD restored from sleep\n");
}