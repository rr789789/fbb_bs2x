# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 提供在本代码库中工作的指导。

## 项目概述

**fbb_bs2x** 是海思（HiSilicon）星闪（NearLink）无线解决方案的 SDK，面向 BS20/BS21E/BS22 系列芯片。这是一个基于 FBB（Family Big Box）统一开发框架、搭载 Huawei LiteOS 实时操作系统的嵌入式 C/C++ SDK。

**关键特性：**
- 支持 SLE（星闪低功耗）、BLE（蓝牙低功耗）和 USB 连接
- 基于 CMake 的组件化架构，使用自定义 `build_component()` 宏
- 基于 Kconfig 的功能配置（类似 Linux 内核）
- RISC-V 32 位架构交叉编译

## 构建系统

构建系统使用 Python 脚本封装 CMake。所有构建命令都在仓库根目录执行。

### 常用构建命令

```bash
# 构建指定目标（示例：bs21e 标准目标）
python src/build.py standard-bs21e-1100e

# 清理后构建（先删除构建产物）
python src/build.py -c standard-bs21e-1100e

# 使用 8 线程并行构建
python src/build.py -j8 standard-bs21e-1100e

# 使用自定义宏构建
python src/build.py -def=CONFIG_UART_BUS_ID=2 standard-bs21e-1100e

# 仅构建指定组件
python src/build.py -component=hal_uart,drivers standard-bs21e-1100e

# 使用 Ninja 生成器替代 Make
python src/build.py -ninja standard-bs21e-1100e

# Debug 与 Release 构建
python src/build.py -debug standard-bs21e-1100e
python src/build.py -release standard-bs21e-1100e
```

### 构建参数说明

- `-c`：构建前先清理
- `-j<N>`：使用 N 线程编译（默认使用最大可用线程）
- `-def=XXX,YYY=x`：添加预处理器宏
- `-component=XXX`：仅构建指定组件
- `-ninja`：使用 Ninja 生成器（默认使用 Unix Makefiles）
- `-debug`/`-release`：控制反汇编信息的详细程度
- `-nhso`：跳过 HSO 数据库更新

### 构建输出

构建产物位于 `output/<芯片>/acore/<目标名>/`：
- `application.elf`：可调式的 ELF 二进制文件
- `application.bin`：用于烧录的原始二进制文件
- `application.fwpkg`：包含分区信息和签名的固件包

### 构建目标

可用目标定义在 `src/build/config/target_config/<芯片>/`。常用目标：
- `standard-bs21e-1100e`：BS21E 标准目标
- `standard-bs20-1100e`：BS20 标准目标
- `standard-bs22-1100e`：BS22 标准目标

## 代码架构

### 目录结构

```
src/
├── application/          # 应用层
│   ├── bs20/bs21e/bs22/  # 芯片特定应用代码
│   └── samples/          # 示例应用
│       ├── peripheral/   # 外设驱动示例（GPIO、UART、I2C 等）
│       ├── products/     # 产品级示例（sle_uart、ble_keyboard 等）
│       └── nfc/          # NFC 相关示例
├── drivers/              # 硬件驱动
│   ├── chips/            # 芯片特定驱动实现
│   └── drivers/          # HAL（硬件抽象层）驱动
│       └── hal/          # 各 HAL 模块（uart、gpio、i2c、spi 等）
├── kernel/               # 操作系统内核
│   ├── liteos/           # Huawei LiteOS 内核
│   └── osal_adapt/       # 操作系统抽象层（OSAL）
├── middleware/           # 中间件服务
│   ├── services/         # 系统服务（DFX、功耗管理、NV 存储、升级）
│   └── utils/            # 工具库
├── protocol/             # 协议栈
│   ├── bt/               # 蓝牙/BLE 协议栈
│   ├── slp/              # SLE（星闪）协议栈
│   └── nfc/              # NFC 协议栈
├── build/                # 构建系统
│   ├── cmake/            # CMake 宏和函数
│   ├── config/           # 目标配置
│   └── script/           # 构建脚本
├── include/              # 公共头文件
├── open_source/          # 第三方开源库
└── vendor/               # 厂商特定代码
```

### 组件模型

每个模块都使用 `build_component()` 宏构建为静态库。典型的组件 `CMakeLists.txt`：

```cmake
set(COMPONENT_NAME "hal_uart")

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/hal_uart.c
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 可选：基于 Kconfig 的条件编译
if(DEFINED CONFIG_UART_USING_V120)
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/v120/hal_uart_v120.c")
endif()

# 可选：整库链接
set(WHOLE_LINK true)

build_component()
```

组件关键变量：
- `COMPONENT_NAME`：组件唯一标识符
- `SOURCES`：要编译的源文件
- `PUBLIC_HEADER`：公共包含目录
- `PRIVATE_HEADER`：私有包含目录
- `PUBLIC_DEFINES`：暴露给依赖者的预处理器定义
- `PRIVATE_DEFINES`：仅内部使用的预处理器定义
- `WHOLE_LINK`：强制包含所有符号（用于 ROM 回调）

### 配置系统

Kconfig 文件定义可配置选项。配置文件位于：
`src/build/config/target_config/<芯片>/menuconfig/<核心>/<目标>.config`

可使用 menuconfig 交互式修改这些配置。

## 代码风格

项目使用 `.clang-format` 配置：
- 基于 LLVM 风格
- 4 空格缩进
- 120 字符行宽
- 提交前使用 `clang-format` 格式化代码

## 关键开发规范

### 错误处理

使用 `src/include/errcode.h` 中的错误码。常用模式：
```c
#include "errcode.h"

errcode_t result = some_operation();
if (result != ERRCODE_SUCC) {
    // 处理错误
}
```

### OS 抽象

使用 `soc_osal.h` 提供的 OSAL（操作系统抽象层）API：
```c
#include "soc_osal.h"

// 创建任务
osal_task_create(...);

// 互斥锁操作
osal_mutex_create(...);
osal_mutex_lock(...);

// 内存分配
osal_mem_alloc(...);
```

### 驱动使用

通过 HAL API 访问驱动：
```c
#include "uart.h"
#include "gpio.h"

// UART 操作
uapi_uart_write(CONFIG_SLE_UART_BUS, data, len, 0);
```

### 日志输出

使用 DFX 日志系统：
```c
osal_printk("调试信息，值为：%d\r\n", value);
```

## 烧录与调试

### 烧录

使用 HiSparkStudio 或命令行工具。固件包位置：
- `tools/pkg/fwpkg/bs21e/bs21e_all.fwpkg`

通过串口（默认波特率：750000）或 USB 烧录。

### 调试

- 调试 ELF：`output/<芯片>/acore/<目标>/application.elf`
- 支持 J-Link（SWD/JTAG）和 GDB
- 默认 J-Link 速度：5000 kHz
- 默认 GDB 端口：3333

## 重要文件位置

| 文件/目录 | 用途 |
|-----------|------|
| `src/build/config/target_config/<芯片>/<芯片>.json` | 芯片构建配置、工具链设置 |
| `src/build/config/target_config/<芯片>/menuconfig/` | Kconfig 配置文件 |
| `src/include/errcode.h` | 全局错误码定义 |
| `src/include/common_def.h` | 通用类型定义和宏 |
| `src/drivers/chips/<芯片>/` | 芯片特定驱动实现 |
| `src/application/samples/products/` | 产品级示例应用 |
| `tools/pkg/fwpkg/` | 预构建固件包 |
| `vendor/<开发板>/` | 厂商特定板级支持和演示 |

## 常见任务

### 添加新源文件

1. 将文件添加到组件 `CMakeLists.txt` 的 `SOURCES` 列表
2. 将新的包含目录添加到 `PUBLIC_HEADER` 或 `PRIVATE_HEADER`
3. 使用 `python src/build.py <目标>` 重新构建

### 添加新组件

1. 创建新目录并放入源文件
2. 创建 `CMakeLists.txt`，定义 `COMPONENT_NAME`、`SOURCES` 和 `PUBLIC_HEADER`
3. 在末尾调用 `build_component()`
4. 在父目录的 `CMakeLists.txt` 中使用 `add_subdirectory_if_exist()` 添加组件目录
5. 如需配置选项，在 Kconfig 中引用该组件

### 启用示例应用

通过 Kconfig 启用示例。编辑目标配置文件：
```
CONFIG_SAMPLE_SUPPORT_SLE_UART=y
CONFIG_SAMPLE_SUPPORT_SLE_UART_SERVER=y
```

或使用 menuconfig 交互式启用。

### 厂商开发板支持

厂商特定代码位于 `vendor/<厂商名>/`。各厂商通常提供：
- `doc/hardware/` 中的硬件原理图
- `demo/` 中的演示应用
- 板级特定配置

---

## 编译问题修复

**首次编译前请查看**: [`docs/BUILD_FIXES.md`](docs/BUILD_FIXES.md)

该文档记录了 BS21E SDK 首次编译时可能遇到的问题及修复方案，包括：
- WSL 环境设置
- 缺失的 HAL 驱动版本文件（符号链接修复）
- Kconfig 配置系统问题修复
- 完整修复脚本
