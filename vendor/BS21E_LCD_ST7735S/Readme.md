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
├── inc/                    # 头文件目录
│   ├── tft.h              # LCD 硬件驱动接口
│   ├── font.h             # ASCII 字体定义
│   ├── chinese_font.h     # 中文字体定义
│   └── ui.h               # 用户界面相关
├── src/                   # 源文件目录
│   ├── main.c            # 主程序入口
│   ├── tft.c             # LCD 驱动实现
│   ├── font.c            # ASCII 字体数据
│   ├── chinese_font.c    # 中文字体数据
│   └── ui.c              # 界面显示逻辑
├── CMakeLists.txt        # 构建配置
└── Readme.md            # 说明文档
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
void LCD_display_full(uint16_t color);
void LCD_display(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
```

### 4. 文本显示
```c
// ASCII 字符显示
void LCD_show_ascii_char(uint8_t x, uint8_t y, char ch, 
                        uint16_t color, uint16_t bg_color);

// 中文字符显示
void LCD_show_chinese_char(uint8_t x, uint8_t y, const char *chinese_char,
                          uint16_t color, uint16_t bg_color);
void LCD_show_chinese_string(uint8_t x, uint8_t y, const char *chinese_string,
                            uint16_t color, uint16_t bg_color);

// 通用字符串显示(支持中英文混合)
void LCD_show_string(uint8_t x, uint8_t y, const char *string,
                    uint16_t color, uint16_t bg_color);

// 缩放显示
void LCD_show_chinese_char_scaled(uint8_t x, uint8_t y, const char *chinese_char,
                                 uint16_t color, uint16_t bg_color, uint8_t scale);
```

### 5. 图像显示
```c
// 位图显示
void showimage(const unsigned char *p, uint8_t x, uint8_t y,
              uint8_t width, uint8_t height);
```

### 6. 电源管理
```c
void LCD_sleep(void);               // 进入睡眠模式
void LCD_restore(void);             // 从睡眠中恢复
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

### 基本初始化和显示
```c
#include "tft.h"

void demo_basic_display(void)
{
    // 1. 初始化 LCD
    LCD_setup_init();
    
    // 2. 全屏填充蓝色背景
    LCD_display_full(BLUE);
    
    // 3. 显示文本
    LCD_show_string(10, 10, "Hello World!", WHITE, BLACK);
    LCD_show_chinese_string(10, 30, "你好世界", RED, BLACK);
    
    // 4. 绘制矩形
    LCD_display(50, 60, 110, 100, GREEN);
}
```

### 图像显示示例
```c
void demo_image_display(void)
{
    // 显示预定义的图像数据
    extern const unsigned char logo_data[];
    showimage(logo_data, 0, 0, 64, 64);
}
```

### 动态显示更新
```c
void demo_dynamic_update(void)
{
    static int counter = 0;
    char buffer[32];
    
    while(1) {
        // 清除特定区域
        LCD_display(10, 80, 150, 100, BLACK);
        
        // 显示动态内容
        sprintf(buffer, "Count: %d", counter++);
        LCD_show_string(10, 80, buffer, YELLOW, BLACK);
        
        osal_msleep(1000);
    }
}
```

## 字体支持

### ASCII 字体
- 字体大小：8x16 像素
- 字符集：标准 ASCII (0x20-0x7F)
- 存储位置：[font.c](src/font.c)

### 中文字体
- 字体大小：16x16 像素  
- 编码：UTF-8
- 支持常用汉字，可在 [chinese_font.c](src/chinese_font.c) 中添加更多字符

## 配置说明

### SPI 配置参数
在 [tft.h](inc/tft.h) 中可调整：
```c
#define SPI_MASTER_BUS_ID    0          // SPI 总线号
#define SPI_SLAVE_NUM        1          // 从设备数量
#define SPI_CLK_POLARITY     0          // 时钟极性
#define SPI_CLK_PHASE        0          // 时钟相位
#define SPI_FRAME_FORMAT     0          // 帧格式
```

### 显示参数
```c
#define LCD_WIDTH           160         // 屏幕宽度
#define LCD_HEIGHT          128         // 屏幕高度
```

## 性能优化建议

### 1. 批量数据传输
```c
// 优化前：逐像素绘制
for(int i = 0; i < 100; i++) {
    LCD_draw_pixel(x+i, y, color);
}

// 优化后：设置窗口后批量写入
LCD_set_address(x, y, x+99, y);
for(int i = 0; i < 100; i++) {
    ST7735S_write_data16(color);
}
```

### 2. 减少不必要的更新
```c
// 只在内容变化时才更新显示
if(new_value != old_value) {
    LCD_show_string(x, y, new_text, color, bg_color);
    old_value = new_value;
}
```

### 3. 使用局部刷新
```c
// 只刷新变化的区域，而不是整个屏幕
LCD_display(x1, y1, x2, y2, BLACK);  // 清除区域
LCD_show_string(x1, y1, new_text, WHITE, BLACK);  // 显示新内容
```

## 故障排除

### 1. 屏幕不显示
- 检查电源连接(3.3V)
- 确认 SPI 引脚连接正确
- 测量复位信号是否正常

### 2. 显示异常/花屏
- 检查 SPI 时序配置
- 确认数据线连接稳定
- 验证 DC 引脚控制正确

### 3. 编译错误
- 确保包含了所有必要的头文件
- 检查 CMakeLists.txt 配置
- 验证字体数据完整性

### 4. 性能问题
- 避免频繁的全屏刷新
- 使用合适的 SPI 时钟频率
- 优化显示内容更新策略

## 扩展开发

### 添加新字体
1. 准备字体数据（可使用字模生成工具）
2. 在相应的 .c 文件中添加字符数据
3. 更新字符映射表
4. 实现对应的显示函数

### 添加图形绘制功能
```c
// 示例：绘制圆形
void LCD_draw_circle(uint8_t cx, uint8_t cy, uint8_t r, uint16_t color);

// 示例：绘制直线
void LCD_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
```

## 版本信息

- 当前版本：1.0.0
- 兼容芯片：BS21E 系列
- 支持屏幕：ST7735S (160x128)
- 开发环境：支持标准 C99

## 技术支持

如遇到问题，请检查：
1. 硬件连接是否正确
2. 电源电压是否稳定
3. SPI 通信是否正常
4. 驱动配置是否匹配硬件

更多技术细节请参考源代码注释和相关技术文档。