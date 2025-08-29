#include "tft.h"
#include "pinctrl.h"
#include "spi.h"
#include "hal_gpio.h"

// SPI引脚配置常量
#define SDA_MODE (2)
#define CLK_MODE (3)

// LCD时钟和频率配置
#define LCD_BUS_CLK (32000000)
#define LCD_FREQ_MHZ (8)

// 延时时间常量
#define SLEEP_10MS (10)
#define SLEEP_30MS (30)
#define SLEEP_50MS (50)
#define SLEEP_100MS (100)
#define SLEEP_120MS (120)
#define LCD_INIT_DELAY1 (150)
#define LCD_INIT_DELAY2 (255)

// SPI缓冲区和传输配置
#define SPI_BUF_SIZE (100)
#define SPI_TX_BYTES_SINGLE (1)
#define SPI_RX_BYTES_ZERO (0)

// LCD ST7735S 命令定义
#define CMD_SWRESET (0x01)    // 软件复位
#define CMD_SLPOUT (0x11)     // 退出睡眠模式
#define CMD_FRMCTR1 (0xB1)    // 帧率控制 - 正常模式
#define CMD_FRMCTR2 (0xB2)    // 帧率控制 - 空闲模式
#define CMD_FRMCTR3 (0xB3)    // 帧率控制 - 部分模式
#define CMD_INVCTR (0xB4)     // 显示反转控制
#define CMD_PWCTR1 (0xC0)     // 电源控制1
#define CMD_PWCTR2 (0xC1)     // 电源控制2
#define CMD_PWCTR3 (0xC2)     // 电源控制3
#define CMD_PWCTR4 (0xC3)     // 电源控制4
#define CMD_PWCTR5 (0xC4)     // 电源控制5
#define CMD_VMCTR1 (0xC5)     // VCOM控制1
#define CMD_MADCTL (0x36)     // 内存访问控制
#define CMD_COLMOD (0x3A)     // 像素格式设置
#define CMD_GMCTRP1 (0xE0)    // 正极性Gamma校正
#define CMD_GMCTRN1 (0xE1)    // 负极性Gamma校正
#define CMD_NORON (0x13)      // 正常显示模式
#define CMD_DISPON (0x29)     // 显示开启
#define CMD_DISPOFF (0x28)    // 显示关闭
#define CMD_SLPIN (0x10)      // 进入睡眠模式
#define CMD_CASET (0x2A)      // 列地址设置
#define CMD_RASET (0x2B)      // 行地址设置
#define CMD_RAMWR (0x2C)      // 内存写入
#define CMD_IDMON (0x39)      // 空闲模式开启

// 帧率控制参数
#define FRMCTR_PARAM1 (0x01)
#define FRMCTR_PARAM2 (0x2C)
#define FRMCTR_PARAM3 (0x2D)

// 显示反转控制参数
#define INVCTR_PARAM (0x07)

// 电源控制参数
#define PWCTR1_PARAM1 (0xA2)
#define PWCTR1_PARAM2 (0x02)
#define PWCTR1_PARAM3 (0x84)
#define PWCTR2_PARAM1 (0xC5)
#define PWCTR3_PARAM1 (0x0A)
#define PWCTR3_PARAM2 (0x00)
#define PWCTR4_PARAM1 (0x8A)
#define PWCTR4_PARAM2 (0x2A)
#define PWCTR5_PARAM1 (0x8A)
#define PWCTR5_PARAM2 (0xEE)
#define VMCTR1_PARAM (0x0E)

// 内存访问控制参数（横屏显示）
#define MADCTL_LANDSCAPE (0xA0)  // MY=1, MX=0, MV=1 for landscape

// 像素格式参数
#define COLMOD_16BIT (0x05)  // 16位/像素

// Gamma校正参数 - 正极性
#define GMCTRP1_PARAM_COUNT (16)
#define GMCTRP1_PARAM1 (0x02)
#define GMCTRP1_PARAM2 (0x1C)
#define GMCTRP1_PARAM3 (0x07)
#define GMCTRP1_PARAM4 (0x12)
#define GMCTRP1_PARAM5 (0x37)
#define GMCTRP1_PARAM6 (0x32)
#define GMCTRP1_PARAM7 (0x29)
#define GMCTRP1_PARAM8 (0x2D)
#define GMCTRP1_PARAM9 (0x29)
#define GMCTRP1_PARAM10 (0x25)
#define GMCTRP1_PARAM11 (0x2B)
#define GMCTRP1_PARAM12 (0x39)
#define GMCTRP1_PARAM13 (0x00)
#define GMCTRP1_PARAM14 (0x01)
#define GMCTRP1_PARAM15 (0x03)
#define GMCTRP1_PARAM16 (0x10)

// Gamma校正参数 - 负极性
#define GMCTRN1_PARAM_COUNT (16)
#define GMCTRN1_PARAM1 (0x03)
#define GMCTRN1_PARAM2 (0x1D)
#define GMCTRN1_PARAM3 (0x07)
#define GMCTRN1_PARAM4 (0x06)
#define GMCTRN1_PARAM5 (0x2E)
#define GMCTRN1_PARAM6 (0x2C)
#define GMCTRN1_PARAM7 (0x29)
#define GMCTRN1_PARAM8 (0x2D)
#define GMCTRN1_PARAM9 (0x2E)
#define GMCTRN1_PARAM10 (0x2E)
#define GMCTRN1_PARAM11 (0x37)
#define GMCTRN1_PARAM12 (0x3F)
#define GMCTRN1_PARAM13 (0x00)
#define GMCTRN1_PARAM14 (0x00)
#define GMCTRN1_PARAM15 (0x02)
#define GMCTRN1_PARAM16 (0x10)

// 颜色位操作常量
#define COLOR_HIGH_BYTE_SHIFT (8)
#define COLOR_LOW_BYTE_MASK (0xFF)
#define BYTE_HIGH_BIT (0x80)
#define BYTE_BITS_COUNT (8)
#define RGB565_DATA_SIZE (2)

// 低功耗模式参数
#define LOW_POWER_FRMCTR_PARAM1 (0x05)
#define LOW_POWER_FRMCTR_PARAM2 (0x3C)
#define LOW_POWER_FRMCTR_PARAM3 (0x3C)
#define LOW_POWER_IDLE_FRMCTR_PARAM (0x08)
#define LOW_POWER_PWCTR2_PARAM (0x95)
#define LOW_POWER_VMCTR1_PARAM (0x08)

// 数组索引和大小相关
#define ARRAY_INDEX_0 (0)
#define ARRAY_INDEX_1 (1)
#define ARRAY_INDEX_2 (2)
#define INVALID_INDEX (-1)
#define PIXEL_COUNT_MULTIPLIER (2)

// 边界值常量
#define LCD_WIDTH_MINUS_ONE (LCD_WIDTH - 1)
#define LCD_HEIGHT_MINUS_ONE (LCD_HEIGHT - 1)

// 循环计数和字符串处理
#define CORNER_RADIUS_MAX_RATIO (2)

// SPI 配置常量
#define SPI_SLAVE_NUM_ONE (1)
#define SPI_WAIT_CYCLES_DEFAULT (0)

// GPIO 电平常量
#define GPIO_LOW_LEVEL (0)
#define GPIO_HIGH_LEVEL (1)

// 地址设置参数
#define ADDR_HIGH_BYTE (0x00)

// 像素计算相关
#define PIXEL_CALCULATION_OFFSET (1)

// 前向声明
static void lcd_init_frame_control(void);
static void lcd_init_power_control(void);
static void lcd_init_gamma_correction(void);
static void lcd_shutdown_display(void);
static void lcd_set_pins_low_power(void);
static void lcd_reduce_power_consumption(void);

void app_spi_init_pin(void) {
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

void app_spi_master_init_config(void) {
    spi_attr_t config = {ARRAY_INDEX_0};
    spi_extra_attr_t ext_config = {ARRAY_INDEX_0};

    config.is_slave = false;
    config.slave_num = SPI_SLAVE_NUM_ONE;
    config.bus_clk = LCD_BUS_CLK;
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
    if (ret != ARRAY_INDEX_0) {
        osal_printk("SPI init failed: 0x%x\n", ret);
    }
}

void lcd_reset(void) {
    LCD_RESET_LOW;
    osal_msleep(SLEEP_50MS);
    LCD_RESET_HIGH;
    osal_msleep(SLEEP_120MS);
}

void st7735s_write_data8(uint8_t dat) {
    LCD_CS_LOW;
    LCD_DC_HIGH;

    spi_xfer_data_t data;
    data.tx_buff = &dat;
    data.tx_bytes = SPI_TX_BYTES_SINGLE;
    data.rx_buff = NULL;
    data.rx_bytes = SPI_RX_BYTES_ZERO;

    uapi_spi_master_write(SPI_BUS_0, &data, SPI_BUF_SIZE);
    LCD_CS_HIGH;
}

void st7735s_write_cmd(uint8_t cmd) {
    LCD_CS_LOW;
    LCD_DC_LOW;

    spi_xfer_data_t data;
    data.tx_buff = &cmd;
    data.tx_bytes = SPI_TX_BYTES_SINGLE;
    data.rx_buff = NULL;
    data.rx_bytes = SPI_RX_BYTES_ZERO;

    uapi_spi_master_write(SPI_BUS_0, &data, SPI_BUF_SIZE);
    LCD_CS_HIGH;
}

void st7735s_write_data16(uint16_t color) {
    LCD_CS_LOW;
    LCD_DC_HIGH;

    uint8_t data_bytes[RGB565_DATA_SIZE];
    data_bytes[ARRAY_INDEX_0] = (uint8_t)(color >> COLOR_HIGH_BYTE_SHIFT);
    data_bytes[ARRAY_INDEX_1] = (uint8_t)(color & COLOR_LOW_BYTE_MASK);

    spi_xfer_data_t data = {
        .tx_buff = data_bytes,
        .tx_bytes = RGB565_DATA_SIZE,
        .rx_buff = NULL,
        .rx_bytes = SPI_RX_BYTES_ZERO
    };

    uapi_spi_master_write(SPI_BUS_0, &data, SPI_BUF_SIZE);
    LCD_CS_HIGH;
}

static void lcd_init_frame_control(void) {
    st7735s_write_cmd(CMD_FRMCTR1);
    st7735s_write_data8(FRMCTR_PARAM1);
    st7735s_write_data8(FRMCTR_PARAM2);
    st7735s_write_data8(FRMCTR_PARAM3);

    st7735s_write_cmd(CMD_FRMCTR2);
    st7735s_write_data8(FRMCTR_PARAM1);
    st7735s_write_data8(FRMCTR_PARAM2);
    st7735s_write_data8(FRMCTR_PARAM3);

    st7735s_write_cmd(CMD_FRMCTR3);
    st7735s_write_data8(FRMCTR_PARAM1);
    st7735s_write_data8(FRMCTR_PARAM2);
    st7735s_write_data8(FRMCTR_PARAM3);
    st7735s_write_data8(FRMCTR_PARAM1);
    st7735s_write_data8(FRMCTR_PARAM2);
    st7735s_write_data8(FRMCTR_PARAM3);

    st7735s_write_cmd(CMD_INVCTR);
    st7735s_write_data8(INVCTR_PARAM);
}

static void lcd_init_power_control(void) {
    st7735s_write_cmd(CMD_PWCTR1);
    st7735s_write_data8(PWCTR1_PARAM1);
    st7735s_write_data8(PWCTR1_PARAM2);
    st7735s_write_data8(PWCTR1_PARAM3);

    st7735s_write_cmd(CMD_PWCTR2);
    st7735s_write_data8(PWCTR2_PARAM1);

    st7735s_write_cmd(CMD_PWCTR3);
    st7735s_write_data8(PWCTR3_PARAM1);
    st7735s_write_data8(PWCTR3_PARAM2);

    st7735s_write_cmd(CMD_PWCTR4);
    st7735s_write_data8(PWCTR4_PARAM1);
    st7735s_write_data8(PWCTR4_PARAM2);

    st7735s_write_cmd(CMD_PWCTR5);
    st7735s_write_data8(PWCTR5_PARAM1);
    st7735s_write_data8(PWCTR5_PARAM2);

    st7735s_write_cmd(CMD_VMCTR1);
    st7735s_write_data8(VMCTR1_PARAM);
}

static void lcd_init_gamma_correction(void) {
    st7735s_write_cmd(CMD_GMCTRP1);
    st7735s_write_data8(GMCTRP1_PARAM1);
    st7735s_write_data8(GMCTRP1_PARAM2);
    st7735s_write_data8(GMCTRP1_PARAM3);
    st7735s_write_data8(GMCTRP1_PARAM4);
    st7735s_write_data8(GMCTRP1_PARAM5);
    st7735s_write_data8(GMCTRP1_PARAM6);
    st7735s_write_data8(GMCTRP1_PARAM7);
    st7735s_write_data8(GMCTRP1_PARAM8);
    st7735s_write_data8(GMCTRP1_PARAM9);
    st7735s_write_data8(GMCTRP1_PARAM10);
    st7735s_write_data8(GMCTRP1_PARAM11);
    st7735s_write_data8(GMCTRP1_PARAM12);
    st7735s_write_data8(GMCTRP1_PARAM13);
    st7735s_write_data8(GMCTRP1_PARAM14);
    st7735s_write_data8(GMCTRP1_PARAM15);
    st7735s_write_data8(GMCTRP1_PARAM16);

    st7735s_write_cmd(CMD_GMCTRN1);
    st7735s_write_data8(GMCTRN1_PARAM1);
    st7735s_write_data8(GMCTRN1_PARAM2);
    st7735s_write_data8(GMCTRN1_PARAM3);
    st7735s_write_data8(GMCTRN1_PARAM4);
    st7735s_write_data8(GMCTRN1_PARAM5);
    st7735s_write_data8(GMCTRN1_PARAM6);
    st7735s_write_data8(GMCTRN1_PARAM7);
    st7735s_write_data8(GMCTRN1_PARAM8);
    st7735s_write_data8(GMCTRN1_PARAM9);
    st7735s_write_data8(GMCTRN1_PARAM10);
    st7735s_write_data8(GMCTRN1_PARAM11);
    st7735s_write_data8(GMCTRN1_PARAM12);
    st7735s_write_data8(GMCTRN1_PARAM13);
    st7735s_write_data8(GMCTRN1_PARAM14);
    st7735s_write_data8(GMCTRN1_PARAM15);
    st7735s_write_data8(GMCTRN1_PARAM16);
}

void lcd_init(void) {
    lcd_reset();

    st7735s_write_cmd(CMD_SWRESET);
    osal_mdelay(LCD_INIT_DELAY1);

    st7735s_write_cmd(CMD_SLPOUT);
    osal_mdelay(LCD_INIT_DELAY2);

    lcd_init_frame_control();
    lcd_init_power_control();

    st7735s_write_cmd(CMD_MADCTL);
    st7735s_write_data8(MADCTL_LANDSCAPE);

    st7735s_write_cmd(CMD_COLMOD);
    st7735s_write_data8(COLMOD_16BIT);

    lcd_init_gamma_correction();

    st7735s_write_cmd(CMD_NORON);
    osal_mdelay(SLEEP_10MS);

    st7735s_write_cmd(CMD_DISPON);
    osal_mdelay(SLEEP_100MS);

    osal_printk("ST7735S LCD init done - 160x128 landscape mode\n");
}

void lcd_set_address(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    if (x1 >= LCD_WIDTH) {
        x1 = LCD_WIDTH_MINUS_ONE;
    }
    if (y1 >= LCD_HEIGHT) {
        y1 = LCD_HEIGHT_MINUS_ONE;
    }
    if (x2 >= LCD_WIDTH) {
        x2 = LCD_WIDTH_MINUS_ONE;
    }
    if (y2 >= LCD_HEIGHT) {
        y2 = LCD_HEIGHT_MINUS_ONE;
    }

    st7735s_write_cmd(CMD_CASET);
    st7735s_write_data8(ADDR_HIGH_BYTE);
    st7735s_write_data8(x1);
    st7735s_write_data8(ADDR_HIGH_BYTE);
    st7735s_write_data8(x2);

    st7735s_write_cmd(CMD_RASET);
    st7735s_write_data8(ADDR_HIGH_BYTE);
    st7735s_write_data8(y1);
    st7735s_write_data8(ADDR_HIGH_BYTE);
    st7735s_write_data8(y2);

    st7735s_write_cmd(CMD_RAMWR);
}

void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }
    lcd_set_address(x, y, x, y);
    st7735s_write_data16(color);
}

void lcd_display_full(uint16_t color) {
    lcd_set_address(ARRAY_INDEX_0, ARRAY_INDEX_0,
                    LCD_WIDTH_MINUS_ONE, LCD_HEIGHT_MINUS_ONE);

    int total_pixels = LCD_WIDTH * LCD_HEIGHT;
    for (int i = ARRAY_INDEX_0; i < total_pixels; i++) {
        st7735s_write_data16(color);
    }
}

void lcd_display(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                 uint16_t color) {
    if (x1 > x2 || y1 > y2) {
        return;
    }
    if (x2 >= LCD_WIDTH || y2 >= LCD_HEIGHT) {
        return;
    }

    lcd_set_address(x1, y1, x2, y2);

    int pixel_count = (x2 - x1 + PIXEL_CALCULATION_OFFSET) *
                      (y2 - y1 + PIXEL_CALCULATION_OFFSET);
    for (int i = ARRAY_INDEX_0; i < pixel_count; i++) {
        st7735s_write_data16(color);
    }
}

void lcd_setup_init(void) {
    app_spi_init_pin();
    app_spi_master_init_config();
    lcd_init();
}

static void lcd_shutdown_display(void) {
    st7735s_write_cmd(CMD_SWRESET);
    osal_msleep(SLEEP_120MS);

    st7735s_write_cmd(CMD_DISPOFF);
    osal_msleep(SLEEP_10MS);

    st7735s_write_cmd(CMD_SLPIN);
    osal_msleep(SLEEP_120MS);
}

static void lcd_set_pins_low_power(void) {
    uapi_gpio_set_dir(SPI_CS_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_mode(SPI_CS_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);

    uapi_gpio_set_dir(SPI_DC_MASTER_PIN, GPIO_DIRECTION_INPUT);
    uapi_pin_set_mode(SPI_DC_MASTER_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);

    uapi_gpio_set_val(SPI_RST_MASTER_PIN, GPIO_LEVEL_LOW);
    uapi_gpio_set_dir(SPI_RST_MASTER_PIN, GPIO_DIRECTION_OUTPUT);

    uapi_pin_set_mode(SPI_SDA_MASTER_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);
    uapi_pin_set_mode(SPI_CLK_MASTER_PIN, HAL_PIO_FUNC_DEFAULT_HIGH_Z);
}

void lcd_deinit(void) {
    lcd_shutdown_display();

    LCD_RESET_LOW;
    osal_msleep(SLEEP_10MS);

    uapi_spi_deinit(SPI_MASTER_BUS_ID);
    lcd_set_pins_low_power();

    osal_printk("TFT LCD deinit done - entered low power mode\n");
}

void lcd_setup_deinit(void) {
    lcd_deinit();
}

static void lcd_reduce_power_consumption(void) {
    st7735s_write_cmd(CMD_FRMCTR1);
    st7735s_write_data8(LOW_POWER_FRMCTR_PARAM1);
    st7735s_write_data8(LOW_POWER_FRMCTR_PARAM2);
    st7735s_write_data8(LOW_POWER_FRMCTR_PARAM3);
    osal_msleep(SLEEP_50MS);

    st7735s_write_cmd(CMD_IDMON);
    osal_msleep(SLEEP_30MS);

    st7735s_write_cmd(CMD_FRMCTR2);
    st7735s_write_data8(LOW_POWER_IDLE_FRMCTR_PARAM);
    st7735s_write_data8(LOW_POWER_IDLE_FRMCTR_PARAM);
    st7735s_write_data8(LOW_POWER_IDLE_FRMCTR_PARAM);
    osal_msleep(SLEEP_30MS);

    st7735s_write_cmd(CMD_PWCTR2);
    st7735s_write_data8(LOW_POWER_PWCTR2_PARAM);
    osal_msleep(SLEEP_30MS);

    st7735s_write_cmd(CMD_VMCTR1);
    st7735s_write_data8(LOW_POWER_VMCTR1_PARAM);
    osal_msleep(SLEEP_30MS);
}

void lcd_enter_ultra_low_power_black(void) {
    osal_printk("[LCD] Progressive low power black mode...\n");

    lcd_display_full(0x0000);
    osal_msleep(SLEEP_100MS);
    osal_printk("[LCD] Step 1: Black screen set\n");

    lcd_reduce_power_consumption();
    osal_printk("[LCD] Progressive low power mode completed\n");
}

void lcd_sleep(void) {
    st7735s_write_cmd(CMD_DISPOFF);
    osal_msleep(SLEEP_10MS);
    st7735s_write_cmd(CMD_SLPIN);
    osal_msleep(SLEEP_120MS);
    osal_printk("LCD entered sleep mode\n");
}

void lcd_restore(void) {
    st7735s_write_cmd(CMD_SLPOUT);
    osal_msleep(SLEEP_120MS);
    st7735s_write_cmd(CMD_DISPON);
    osal_msleep(SLEEP_10MS);
    osal_printk("LCD restored from sleep\n");
}