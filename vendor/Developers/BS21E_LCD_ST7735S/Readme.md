# BS21E LCD ST7735S 显示驱动

基于 BS21E 芯片的 ST7735S TFT LCD 显示屏驱动库，支持 160x128 像素彩色显示。

## 硬件连接

### SPI 引脚配置
```
LCD引脚    BS21E引脚    功能说明
VCC       3.3V        电源正极
GND       GND         电源负极  
SCL       GPIO14      SPI 时钟线
SDA       GPIO11      SPI 数据线
RES       GPIO12      复位引脚
DC        GPIO15      数据/命令选择
CS        GPIO13      片选信号
BLK       3.3V        背光控制(可接PWM控制)
```

### 电气特性
- 工作电压：3.3V
- 显示尺寸：160x128 像素
- 颜色深度：16位 RGB565
- SPI 时钟频率：8MHz
- 显示方向：横屏显示

## 项目结构

```
BS21E_LCD_ST7735S/
├── main.c            # 主程序入口
├── tft.c             # LCD 驱动实现
├── tft.h             # LCD 驱动头文件
├── CMakeLists.txt    # 构建配置
└── Readme.md         # 说明文档
```

## 核心 API 接口

### 1. 系统初始化
```c
void LCD_setup_init(void);          // LCD 完整初始化
void LCD_setup_deinit(void);        // LCD 去初始化
```

### 2. 硬件层接口
```c
// SPI 通信
void app_spi_init_pin(void);                    // 初始化 SPI 引脚
void app_spi_master_init_config(void);          // 配置 SPI 主机模式

// 底层控制
void lcd_reset(void);                           // 硬件复位
void ST7735S_write_cmd(uint8_t cmd);           // 发送命令
void ST7735S_write_data8(uint8_t dat);         // 发送8位数据
void ST7735S_write_data16(uint16_t color);     // 发送16位颜色数据
void LCD_init(void);                           // LCD 初始化序列
```

### 3. 显示控制
```c
// 窗口设置
void LCD_set_address(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

// 基础绘制
void LCD_draw_pixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_display_full(uint16_t color);                        // 全屏填充颜色
void LCD_display(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
```

### 4. 电源管理
```c
void LCD_sleep(void);                           // 进入睡眠模式
void LCD_restore(void);                         // 从睡眠中恢复
void LCD_enter_ultra_low_power_black(void);    // 进入超低功耗黑屏模式
```

## 颜色定义

使用 RGB565 格式，常用颜色宏定义：
```c
#define WHITE       0xFFFF      // 白色
#define BLACK       0x0000      // 黑色
#define RED         0xF800      // 红色
#define GREEN       0x07E0      // 绿色
#define BLUE        0x001F      // 蓝色
#define YELLOW      0xFFE0      // 黄色
#define CYAN        0x07FF      // 青色
#define MAGENTA     0xF81F      // 品红
```

## 使用示例

### 基本红色显示（当前main.c实现）
```c
#include "tft.h"

#define RED_COLOR 0xF800

int main_task(const char *arg)
{
    // 设置高频时钟
    uapi_clock_control(CLOCK_CONTROL_FREQ_LEVEL_CONFIG, CLOCK_FREQ_LEVEL_HIGH);
    
    // 初始化LCD
    LCD_setup_init();
    
    // 显示红色
    LCD_display_full(RED_COLOR);
    
    // 保持显示
    while(1)
    {
        osal_msleep(5000);
    }
    
    return 0;
}
```

### 其他颜色显示示例
```c
void demo_color_display(void)
{
    // 初始化 LCD
    LCD_setup_init();
    
    // 显示不同颜色
    LCD_display_full(RED);      // 红色
    osal_msleep(2000);
    
    LCD_display_full(GREEN);    // 绿色
    osal_msleep(2000);
    
    LCD_display_full(BLUE);     // 蓝色
    osal_msleep(2000);
    
    LCD_display_full(WHITE);    // 白色
    osal_msleep(2000);
}
```

### 矩形区域绘制示例
```c
void demo_rectangle_display(void)
{
    LCD_setup_init();
    
    // 清屏（黑色背景）
    LCD_display_full(BLACK);
    
    // 绘制不同颜色的矩形
    LCD_display(10, 10, 50, 30, RED);      // 红色矩形
    LCD_display(60, 10, 100, 30, GREEN);   // 绿色矩形
    LCD_display(110, 10, 150, 30, BLUE);   // 蓝色矩形
    
    // 绘制大矩形边框
    LCD_display(5, 5, 155, 35, WHITE);     // 白色边框
}
```

### 像素级绘制示例
```c
void demo_pixel_drawing(void)
{
    LCD_setup_init();
    LCD_display_full(BLACK);   // 黑色背景
    
    // 绘制对角线
    for(int i = 0; i < 128; i++)
    {
        LCD_draw_pixel(i, i, WHITE);
        LCD_draw_pixel(159-i, i, RED);
    }
}
```

## 配置说明

### 在 tft.h 中的关键配置
```c
// SPI 引脚定义
#define SPI_CLK_MASTER_PIN      14      // SPI时钟引脚
#define SPI_SDA_MASTER_PIN      11      // SPI数据引脚  
#define SPI_CS_PIN              13      // 片选引脚
#define SPI_DC_MASTER_PIN       15      // 数据/命令选择引脚
#define SPI_RST_MASTER_PIN      12      // 复位引脚

// LCD 尺寸定义
#define LCD_WIDTH               160     // 屏幕宽度
#define LCD_HEIGHT              128     // 屏幕高度

// SPI 配置
#define SPI_MASTER_BUS_ID       0       // SPI总线ID
#define LCD_FREQ_MHZ            8       // SPI频率
```

## 代码特色

### 1. 统一的大括号风格
所有函数和控制结构都采用换行大括号风格：
```c
void function_name(void)
{
    if (condition)
    {
        // code here
    }
}
```

### 2. 完整的宏定义
所有数字常量都使用宏定义，增强代码可读性和可维护性：
```c
#define SLEEP_50MS 50
#define LCD_WIDTH_MINUS_ONE 159
#define RED_COLOR 0xF800
```

### 3. 严格的错误处理
包含完整的边界检查和错误处理机制。

## 性能特性

### 1. 高效的数据传输
- 使用SPI硬件传输，速度快
- 支持16位颜色数据直接传输
- 批量像素操作优化

### 2. 灵活的显示窗口
- 支持任意矩形区域设置
- 可实现局部刷新，提高效率

### 3. 低功耗模式
- 支持睡眠模式
- 超低功耗黑屏模式
- 完整的去初始化流程

## 故障排除

### 1. 屏幕不显示
- 检查电源连接(3.3V)
- 确认 SPI 引脚连接正确
- 测量复位信号是否正常
- 检查片选信号时序

### 2. 显示颜色异常
- 确认使用RGB565格式
- 检查颜色值计算是否正确
- 验证SPI数据传输时序

### 3. 编译错误
- 确保包含了所有必要的头文件
- 检查 CMakeLists.txt 配置
- 验证BS21E SDK路径配置

### 4. 性能问题
- 避免频繁的全屏刷新
- 使用LCD_display()进行区域更新
- 合理设置SPI时钟频率

## 扩展开发建议

### 1. 添加图形绘制功能
```c
// 可以扩展的功能
void LCD_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void LCD_draw_circle(uint8_t cx, uint8_t cy, uint8_t radius, uint16_t color);
void LCD_draw_rectangle_outline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
```

### 2. 添加文本显示功能
可以参考其他项目添加字体显示功能，支持ASCII和中文字符。

### 3. 添加图像显示功能
支持位图图像的显示和简单的图像处理。

## 构建说明

确保在BS21E开发环境中：
1. 将项目放在正确的samples目录下
2. 配置CMakeLists.txt包含必要的库
3. 使用标准的BS21E构建流程编译

## 版本信息

- 当前版本：1.0.0
- 兼容芯片：BS21E 系列
- 支持屏幕：ST7735S (160x128)
- 开发环境：BS21E SDK
- 代码风格：统一大括号换行 + 完整宏定义

## 技术支持

如遇到问题，请检查：
1. 硬件连接是否正确
2. 电源电压是否稳定(3.3V)
3. SPI通信时序是否正确
4. 引脚配置是否匹配硬件

更多技术细节请参考源代码注释和BS21E官方文档。