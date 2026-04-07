# Gemini CLI 项目上下文：fbb_bs2x 星闪 SDK

## 项目概述
`fbb_bs2x` 是海思（HiSilicon）星闪（NearLink）解决方案的软件开发工具包（SDK），涵盖了 **BS20**、**BS21E** 和 **BS22** 系列芯片。该 SDK 基于 **FBB (Family Big Box)** 统一开发框架构建，搭载 **Huawei LiteOS** 操作系统，专为鼠标、键盘、测距、电子消费类等应用场景设计，支持 SLE（星闪低功耗）、BLE（蓝牙低功耗）和 USB 等多种连接方式。

## 目录结构深度解析
- **`src/`**: SDK 核心源码
  - **`application/`**: 应用层代码，包含 `samples`（示例程序，如 SLE/BLE 透传、HID 设备等）。
  - **`build/`**: 构建系统脚本和配置。
    - `cmake/`: 核心 CMake 构建逻辑（组件注册、编译选项、链接脚本生成等）。
    - `config/target_config/`: 针对不同芯片（bs20/bs21e/bs22）的详细编译配置。
  - **`drivers/`**: 驱动层，包含芯片外设（GPIO, UART, I2C, SPI, PWM, ADC 等）的 HAL 和驱动实现。
  - **`kernel/`**: 操作系统内核，主要是 `liteos` 的适配和 OSAL（操作系统抽象层）。
  - **`middleware/`**: 中间件，包含 AT 命令、DFX（维测）、功耗管理（PM）、NV 存储、升级（Update）等服务。
  - **`protocol/`**: 协议栈实现，包括星闪（SLE）和蓝牙（BT/BLE）。
- **`docs/`**: 详尽的中英文在线文档和本地手册，包含 IO 复用表和硬件指南。
- **`tools/`**: 开发环境搭建工具及指南（如 WSL 环境搭建、烧录工具说明）。
- **`vendor/`**: 合作厂商（如润和 HH-D03、小熊派 BearPi-Pico）的开发板资料、硬件原理图及特定案例代码。

## 构建系统与编译指南
项目采用 Python 脚本 (`src/build.py`) 封装 CMake 的构建流程。

### 常用编译命令
- **编译特定目标 (以 BS21E 为例):**
  ```powershell
  python src/build.py standard-bs21e-1100e
  ```
- **完整清理并编译:**
  ```powershell
  python src/build.py -c standard-bs21e-1100e
  ```
- **多线程编译:**
  ```powershell
  python src/build.py -j8 standard-bs21e-1100e
  ```
- **编译参数说明:**
  - `-def=macro`: 传入预编译宏（例如 `-def=CONFIG_UART_BUS_ID=2`）。
  - `-ninja`: 使用 Ninja 生成器（默认使用 Unix Makefile）。
  - `-release/debug`: 控制编译版本，debug 模式包含更多维测信息。

### 产物位置
编译完成后，产物位于根目录下的 `output/` 文件夹中，通常包含：
- `.elf`: 调试用的二进制文件。
- `.bin`: 烧录用的原始二进制镜像。
- `.fwpkg`: 固件升级包，包含分区信息和签名。

## 开发规范与最佳实践

### 代码风格
- **C/C++**: 严格遵循根目录下的 `.clang-format` 配置（4 空格缩进，120 字符行宽，基于 LLVM 风格）。
- **组件化管理**: 使用 `build_component()` 宏注册组件，通过 `Kconfig` 管理功能开关。

### 示例开发流程
1. **环境准备**: 参考 `tools/WSL子系统开发环境搭建.md` 搭建 Linux 交叉编译环境（推荐）。
2. **选择案例**: 在 `src/application/samples/` 中寻找相似的参考代码。
3. **配置功能**: 修改对应目标的 `menuconfig`（位于 `src/build/config/target_config/<chip>/menuconfig/`）。
4. **编译与验证**: 执行编译脚本，并通过烧录工具将 `output` 目录下的固件刷入开发板。

### 关键配置文件
- **`src/build/config/target_config/bs21e/bs21e.json`**: 定义了工具链路径、烧录协议、串口波特率等关键信息。
- **`src/include/errcode.h`**: 全局错误码定义，开发时需严格遵循返回码规范。

## 维测与调试 (DFX)
- **日志系统**: 使用 `middleware/utils/dfx` 提供的日志接口，支持通过 HSO 数据库进行解析。
- **AT 命令**: 可通过串口发送 AT 命令进行硬件测试和状态查询。
- **GDB 调试**: 配合 J-Link 等工具，根据生成的 `.elf` 文件进行在线调试。

## 硬件支持列表
- **润和 HH-D03/HH-D121**: 通用开发板，资料丰富。
- **小熊派 BearPi-Pico_H2821E**: 紧凑型开发板，支持多种星闪案例。
- **利尔达 EB21/2825**: 针对模组和外设的开发方案。
