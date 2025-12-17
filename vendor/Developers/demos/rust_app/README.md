# Rust 应用示例

## 功能说明

本示例演示如何在 WS63 平台上使用 Rust 语言开发嵌入式应用。通过 Rust 实现一个简单的 LED 闪烁程序，展示 Rust 与 C 代码的互操作。

## 主要特性

- ✅ Rust 嵌入式开发
- ✅ `#![no_std]` 裸机环境
- ✅ C/Rust 互操作
- ✅ 自动生成 C 绑定
- ✅ GPIO 控制
- ✅ 任务创建和调度

## 技术栈

- **语言**: Rust (no_std)
- **目标平台**: riscv32imfc-unknown-none-elf
- **构建工具**: Cargo + CMake
- **绑定生成**: bindgen

## 代码结构

```
rust_app/
├── src/
│   ├── lib.rs          # Rust 主程序
│   ├── bindings.rs     # C 绑定（自动生成）
│   └── rust_glue.c     # C 胶水代码
├── adapter/            # 适配器代码
├── .cargo/
│   └── config.toml     # Cargo 配置
├── build.rs            # 构建脚本
├── Cargo.toml          # Rust 项目配置
├── Cargo.lock          # 依赖锁定
├── CMakeLists.txt      # CMake 配置
├── wrapper.h           # C 头文件包装
├── rust-toolchain.toml # Rust 工具链配置
├── riscv32imfc-unknown-none-elf.json  # 目标配置
└── README.md           # 本文件
```

## 工作原理

### 1. Rust 代码编译

Rust 代码被编译为静态库 `librust_app.a`：

```rust
#![no_std]
#![no_main]

// Blinky 任务
unsafe extern "C" fn blinky_task(_arg: *const cty::c_char) -> cty::c_int {
    // 设置 GPIO
    uapi_pin_set_mode(CONFIG_SAMPLE_BLINKY_PIN, HAL_PIO_FUNC_GPIO as u32);
    uapi_gpio_set_dir(CONFIG_SAMPLE_BLINKY_PIN, GPIO_DIRECTION_OUTPUT as u32);
    
    loop {
        osal_msleep(500);
        uapi_gpio_toggle(CONFIG_SAMPLE_BLINKY_PIN);
    }
}
```

### 2. C 绑定生成

使用 `bindgen` 自动生成 C API 的 Rust 绑定：

```rust
// build.rs
bindgen::Builder::default()
    .header("wrapper.h")
    .generate()
    .unwrap()
    .write_to_file(out_path.join("bindings.rs"))
    .unwrap();
```

### 3. C/Rust 互操作

通过 C 胶水代码调用 Rust 函数：

```c
// rust_glue.c
extern void rust_app_init(void);
app_run(rust_app_init);
```

### 4. 链接过程

CMake 将 Rust 静态库链接到最终固件：

```cmake
set(LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/target/riscv32imfc-unknown-none-elf/debug/librust_app.a
)
```

## 环境配置

### 1. 安装 Rust

```bash
# 安装 Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# 添加 RISC-V 目标
rustup target add riscv32imfc-unknown-none-elf
```

### 2. 安装 bindgen 依赖

**Windows**:
```bash
# 安装 LLVM
choco install llvm

# 设置环境变量
set LIBCLANG_PATH=C:\Program Files\LLVM\bin
```

**Linux**:
```bash
sudo apt-get install llvm-dev libclang-dev clang
```

**macOS**:
```bash
brew install llvm
export LLVM_CONFIG_PATH=/usr/local/opt/llvm/bin/llvm-config
```

### 3. 配置工具链

项目已包含 `rust-toolchain.toml`，会自动使用正确的工具链版本。

## 编译步骤

### 1. 编译 Rust 代码

```bash
cd src/application/samples/peripheral/rust_app
cargo build --target riscv32imfc-unknown-none-elf
```

### 2. 编译整个项目

```bash
cd src
python build.py ws63-liteos-app
```

### 3. 烧录运行

使用 HiSparkStudio 或命令行工具烧录固件。

## 配置参数

### Cargo.toml

```toml
[package]
name = "rust_app"
version = "0.1.0"
edition = "2021"

[dependencies]
cty = "0.2"

[profile.dev]
opt-level = "z"        # 优化大小
lto = true             # 链接时优化
panic = "abort"        # Panic 时中止
```

### rust-toolchain.toml

```toml
[toolchain]
channel = "nightly"
targets = ["riscv32imfc-unknown-none-elf"]
```

### 目标配置

`riscv32imfc-unknown-none-elf.json` 定义了 RISC-V 目标的详细配置。

## 关键概念

### 1. `#![no_std]`

不使用标准库，适用于嵌入式环境：

```rust
#![no_std]

// 不能使用：
// - std::vec::Vec
// - std::string::String
// - std::collections::HashMap

// 可以使用：
// - core::* (核心库)
// - alloc::* (需要分配器)
```

### 2. `#![no_main]`

不使用标准的 `main` 函数：

```rust
#![no_main]

// 自定义入口
#[no_mangle]
pub extern "C" fn rust_app_init() {
    // 初始化代码
}
```

### 3. Panic 处理

必须提供 panic 处理函数：

```rust
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
```

### 4. FFI (Foreign Function Interface)

调用 C 函数：

```rust
extern "C" {
    fn uapi_gpio_toggle(pin: u32);
}

unsafe {
    uapi_gpio_toggle(pin);
}
```

## 常见问题

### Q1: 编译失败，找不到 bindgen？

**A:** 安装 bindgen：
```bash
cargo install bindgen-cli
```

### Q2: 链接错误？

**A:** 确保：
1. Rust 代码已编译
2. 静态库路径正确
3. 目标架构匹配

### Q3: 运行时崩溃？

**A:** 检查：
1. 栈大小是否足够
2. 是否有未处理的 panic
3. FFI 调用是否正确

### Q4: 如何调试 Rust 代码？

**A:** 
1. 使用 `osal_printk` 打印日志
2. 使用 GDB 调试
3. 检查生成的汇编代码

## 扩展功能

### 1. 使用 Rust 标准数据结构

需要实现全局分配器：

```rust
use core::alloc::{GlobalAlloc, Layout};

struct MyAllocator;

unsafe impl GlobalAlloc for MyAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        // 实现分配逻辑
    }
    
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        // 实现释放逻辑
    }
}

#[global_allocator]
static ALLOCATOR: MyAllocator = MyAllocator;
```

### 2. 添加更多 C 绑定

在 `wrapper.h` 中添加需要的头文件：

```c
#include "uart.h"
#include "i2c.h"
#include "spi.h"
```

### 3. 使用 Rust 宏

```rust
macro_rules! gpio_set {
    ($pin:expr, $val:expr) => {
        unsafe {
            uapi_gpio_set_val($pin, $val as u32);
        }
    };
}

// 使用
gpio_set!(CONFIG_SAMPLE_BLINKY_PIN, GPIO_LEVEL_HIGH);
```

### 4. 错误处理

```rust
type Result<T> = core::result::Result<T, ErrorCode>;

enum ErrorCode {
    InvalidPin,
    InitFailed,
    Timeout,
}

fn init_gpio(pin: u32) -> Result<()> {
    if pin > MAX_PIN {
        return Err(ErrorCode::InvalidPin);
    }
    // ...
    Ok(())
}
```

## 性能对比

| 指标 | C 代码 | Rust 代码 | 说明 |
|------|--------|-----------|------|
| 代码大小 | 基准 | +5~10% | Rust 略大 |
| 运行速度 | 基准 | 相当 | 优化后相当 |
| 内存安全 | 手动 | 自动 | Rust 优势 |
| 开发效率 | 中等 | 较高 | 类型系统帮助 |

## 优势与劣势

### 优势

- ✅ 内存安全（无空指针、无缓冲区溢出）
- ✅ 类型安全（编译时检查）
- ✅ 零成本抽象
- ✅ 现代语言特性（模式匹配、迭代器等）
- ✅ 优秀的包管理（Cargo）

### 劣势

- ❌ 学习曲线陡峭
- ❌ 编译时间较长
- ❌ 生态系统相对较小
- ❌ 调试工具支持有限
- ❌ 代码体积略大

## 注意事项

1. **工具链版本**：使用 nightly 版本以支持嵌入式特性
2. **优化级别**：使用 `opt-level = "z"` 优化代码大小
3. **链接时优化**：启用 LTO 减小代码体积
4. **Panic 处理**：必须实现 panic_handler
5. **FFI 安全**：所有 C 函数调用都是 unsafe 的

## 参考资料

- [The Embedded Rust Book](https://rust-embedded.github.io/book/)
- [Rust FFI Omnibus](http://jakegoulding.com/rust-ffi-omnibus/)
- [bindgen User Guide](https://rust-lang.github.io/rust-bindgen/)
- [RISC-V Rust](https://github.com/rust-embedded/riscv)

## 版本历史

- v1.0：初始版本，实现基本的 Blinky 功能
- v1.1：添加 bindgen 自动生成绑定
- v1.2：代码规范化，添加详细注释和文档
