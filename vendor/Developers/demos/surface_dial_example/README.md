# Surface Dial 旋钮示例

## 功能说明

本示例演示如何使用 WS63 开发板模拟 Microsoft Surface Dial 设备，通过 USB HID 协议实现旋钮和按键功能。

## 主要特性

- ✅ USB HID 设备模拟
- ✅ EC11 旋转编码器支持
- ✅ 旋转检测（顺时针/逆时针）
- ✅ 按键检测
- ✅ 格雷码解码
- ✅ 消抖处理
- ✅ Windows 系统识别为 Surface Dial

## 硬件要求

### EC11 旋转编码器

| EC11 引脚 | WS63 引脚 | 说明 |
|-----------|-----------|------|
| A | GPIO_X | 编码器 A 相 |
| B | GPIO_Y | 编码器 B 相 |
| KEY | GPIO_Z | 按键 |
| GND | GND | 地 |
| VCC | 3.3V | 电源 |

### USB 连接

- USB D+ / D- 连接到 WS63 的 USB 引脚
- 确保 USB 供电正常

## 工作原理

### 1. USB HID 设备

本示例将 WS63 配置为 USB HID 设备，模拟 Microsoft Surface Dial：

- **Vendor ID**: 0x1D50 (OpenMoko)
- **Product ID**: 0x6D1A
- **设备名称**: Microsoft Surface Dial
- **HID 报告 ID**: 0x05

### 2. EC11 编码器

EC11 是一个增量式旋转编码器，输出两路正交信号（A 相和 B 相）：

```
顺时针旋转：
A: ┌─┐   ┌─┐
   │ └───┘ └───
B:   ┌─┐   ┌─┐
   ──┘ └───┘ └─

逆时针旋转：
A:   ┌─┐   ┌─┐
   ──┘ └───┘ └─
B: ┌─┐   ┌─┐
   │ └───┘ └───
```

### 3. 格雷码解码

编码器使用格雷码（Gray Code）编码，通过状态机判断旋转方向：

**顺时针状态序列**：
- 0b1101 (13)
- 0b0100 (4)
- 0b0010 (2)
- 0b1011 (11)

**逆时针状态序列**：
- 0b1110 (14)
- 0b0111 (7)
- 0b0001 (1)
- 0b1000 (8)

### 4. HID 报告格式

```
Byte 0: Report ID (0x05)
Byte 1: Button (bit 0) + Rotation Low 7 bits
Byte 2: Rotation High 8 bits
```

**旋转值**：
- 15 位有符号整数
- 范围：-3600 ~ 3600
- 正值：顺时针
- 负值：逆时针

## 代码结构

```
surface_dial_example/
├── surface_dial_hid.c      # USB HID 设备实现
├── surface_dial_hid.h      # HID 接口定义
├── encoder.c               # EC11 编码器驱动
├── encoder.h               # 编码器接口定义
├── encoder_setting.c       # 编码器配置和主程序
├── encoder_setting.h       # 配置定义
├── CMakeLists.txt          # 编译配置
└── README.md               # 本文件
```

## 关键函数说明

### surface_dial_init()
初始化 USB HID 设备，注册为 Surface Dial。

### surface_dial_send_rotation()
发送旋转数据到主机。

**参数**：
- `rotation`: 旋转值（-3600 ~ 3600）

### surface_dial_send_button()
发送按键状态到主机。

**参数**：
- `pressed`: 按键状态（0=释放，1=按下）

### ec11_init()
初始化 EC11 编码器，配置 GPIO 和中断。

**参数**：
- `config`: 编码器配置结构体

### ec11_get_state()
获取编码器当前状态。

**返回值**：
- `EC11_ROTATE_CW`: 顺时针旋转
- `EC11_ROTATE_CCW`: 逆时针旋转
- `EC11_BUTTON_PRESS`: 按键按下
- `EC11_BUTTON_RELEASE`: 按键释放
- `EC11_ROTATE_NONE`: 无动作

## 配置参数

### 编码器配置

```c
ec11_config_t config = {
    .pin_a = GPIO_PIN_A,           // A 相引脚
    .pin_b = GPIO_PIN_B,           // B 相引脚
    .pin_button = GPIO_PIN_KEY,    // 按键引脚
    .has_button = true,            // 启用按键
    .debounce_ms = 10,             // 消抖时间（毫秒）
    .callback = encoder_callback,  // 回调函数
    .user_data = NULL              // 用户数据
};
```

### HID 设备配置

```c
#define SURFACE_DIAL_VENDOR_ID   0x1D50    // 厂商 ID
#define SURFACE_DIAL_PRODUCT_ID  0x6D1A    // 产品 ID
#define SURFACE_DIAL_RELEASE_NUM 0x0110    // 版本号
```

## 使用方法

### 1. 硬件连接

按照硬件要求表连接 EC11 编码器和 USB。

### 2. 配置引脚

在 `encoder_setting.c` 中修改引脚定义：

```c
#define ENCODER_PIN_A      GPIO_XX
#define ENCODER_PIN_B      GPIO_YY
#define ENCODER_PIN_BUTTON GPIO_ZZ
```

### 3. 编译烧录

```bash
cd src
python build.py ws63-liteos-app
```

### 4. 连接测试

1. 将 WS63 通过 USB 连接到 Windows 电脑
2. 系统会识别为 "Microsoft Surface Dial"
3. 旋转编码器，观察系统响应
4. 按下按键，触发按键事件

### 5. 应用场景

- **图像编辑**：Photoshop、Lightroom 等
- **视频编辑**：Premiere Pro、DaVinci Resolve 等
- **3D 建模**：Blender、Maya 等
- **音频编辑**：音量控制、参数调节
- **浏览器**：页面缩放、滚动

## 常见问题

### Q1: 系统无法识别设备？

**A:** 检查以下几点：
1. USB 连接是否正常
2. USB D+/D- 引脚是否正确
3. 设备描述符是否正确
4. 驱动是否安装

### Q2: 旋转方向相反？

**A:** 交换 A 相和 B 相引脚，或修改代码中的方向判断逻辑。

### Q3: 旋转不灵敏？

**A:** 可能原因：
1. 消抖时间过长，减小 `debounce_ms`
2. 编码器质量问题
3. 接线接触不良

### Q4: 按键无响应？

**A:** 检查：
1. 按键引脚配置是否正确
2. 上拉电阻是否启用
3. 中断是否正确注册

## 技术细节

### 格雷码状态机

```
状态转换表（顺时针）：
00 -> 01 -> 11 -> 10 -> 00

状态转换表（逆时针）：
00 -> 10 -> 11 -> 01 -> 00
```

### 消抖算法

使用时间戳方式消抖：

```c
if ((current_time - last_change_time) < debounce_ms) {
    return;  // 忽略抖动
}
```

### 中断处理

- 使用双边沿触发中断
- 在中断中快速读取状态
- 通过回调函数通知应用层

## 扩展功能

### 1. 添加 LED 指示

```c
void encoder_callback(ec11_state_t state, void *user_data)
{
    if (state == EC11_ROTATE_CW) {
        // 点亮 LED
        uapi_gpio_set_val(LED_PIN, GPIO_LEVEL_HIGH);
    }
}
```

### 2. 多级旋转速度

```c
int16_t get_rotation_value(uint32_t time_diff)
{
    if (time_diff < 50) {
        return 100;  // 快速旋转
    } else if (time_diff < 200) {
        return 10;   // 中速旋转
    } else {
        return 1;    // 慢速旋转
    }
}
```

### 3. 长按检测

```c
#define LONG_PRESS_TIME_MS  1000

void button_callback(void)
{
    static uint64_t press_time = 0;
    
    if (button_pressed) {
        press_time = uapi_systick_get_ms();
    } else {
        uint64_t duration = uapi_systick_get_ms() - press_time;
        if (duration > LONG_PRESS_TIME_MS) {
            // 长按事件
        }
    }
}
```

## 注意事项

1. **USB 供电**：确保 USB 供电稳定，电流充足
2. **引脚配置**：根据实际硬件修改引脚定义
3. **消抖时间**：根据编码器质量调整消抖参数
4. **中断优先级**：确保中断优先级设置合理
5. **格雷码**：不同编码器可能使用不同的格雷码序列

## 参考资料

- Microsoft Surface Dial 规范
- USB HID 规范
- EC11 编码器数据手册
- 格雷码编码原理

## 版本历史

- v1.0：初始版本，实现基本功能
- v1.1：添加消抖和格雷码解码
- v1.2：代码规范化，添加详细注释和文档
