use std::env;
use std::path::PathBuf;

fn main() {
    // 告诉 cargo 如果这些文件改变了就重新构建
    println!("cargo:rerun-if-changed=wrapper.h");
    
    // 获取项目根目录
    let project_root = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .to_path_buf();
    
    // SDK 头文件路径 - 使用相对路径，并过滤存在的路径
    let potential_include_paths = vec![
        project_root.join("include"),
        project_root.join("include/driver"), 
        project_root.join("include/middleware"),
        project_root.join("drivers/chips/bs21e/board/board_config"),
        project_root.join("drivers/chips/bs2x"),
        project_root.join("kernel/osal/include"),
        project_root.join("kernel/liteos"),
        project_root.join("middleware/utils/app_init/public"),
        project_root.join("middleware/utils"),
    ];
    
    // 只使用存在的路径
    let sdk_include_paths: Vec<PathBuf> = potential_include_paths
        .into_iter()
        .filter(|path| path.exists())
        .collect();
    
    println!("Found {} include paths:", sdk_include_paths.len());
    for path in &sdk_include_paths {
        println!("  - {}", path.display());
    }

    // 尝试使用 bindgen 自动生成绑定
    if let Ok(bindings) = try_bindgen(&sdk_include_paths) {
        let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
        bindings
            .write_to_file(out_path.join("bindings.rs"))
            .expect("Couldn't write bindings!");
        println!("Generated bindings with bindgen");
    } else {
        // 如果 bindgen 失败，则使用手动绑定作为后备
        create_manual_bindings();
        println!("Using manual bindings as fallback");
    }
}

fn try_bindgen(include_paths: &[PathBuf]) -> Result<bindgen::Bindings, Box<dyn std::error::Error>> {
    // 创建 wrapper.h 文件
    create_wrapper_header();
    
    // 获取当前目录（Cargo.toml 所在目录）
    let cargo_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let project_root = cargo_dir.parent().unwrap().parent().unwrap();
    
    // 获取工具链路径（相对于项目根目录）
    let toolchain_root = project_root.join("tools/bin/compiler/riscv/cc_riscv32_musl_b010/cc_riscv32_musl_fp");
    
    let mut builder = bindgen::Builder::default()
        .header("wrapper.h")
        // 配置 clang 参数
        .clang_arg("-target")
        .clang_arg("riscv32-unknown-none-elf")
        .clang_arg("-march=rv32imfc")
        .clang_arg("-mabi=ilp32f");
    
    // 只有在工具链路径存在时才添加 sysroot
    if toolchain_root.exists() {
        builder = builder.clang_arg(format!("--sysroot={}", toolchain_root.join("riscv32-linux-musl").display()));
    }
    
    // 添加所有包含路径
    for path in include_paths {
        builder = builder.clang_arg(format!("-I{}", path.display()));
    }
    
    let bindings = builder
        // 只生成我们需要的函数和类型
        .allowlist_function("uapi_.*")
        .allowlist_function("osal_.*")
        .allowlist_function("app_.*")
        .allowlist_type("gpio_.*")
        .allowlist_type("osal_.*")
        .allowlist_var("CONFIG_.*")
        .allowlist_var("GPIO_.*")
        .allowlist_var("HAL_.*")
        // 告诉 bindgen 这些是不透明类型
        .opaque_type("osal_task")
        // 使用 no_std
        .use_core()
        .ctypes_prefix("cty")
        // 配置生成选项
        .derive_debug(false)
        .derive_default(false)
        .generate_comments(false)
        .generate()?;
    
    Ok(bindings)
}

fn create_wrapper_header() {
    let wrapper_content = r#"
#ifndef RUST_WRAPPER_H
#define RUST_WRAPPER_H

// 标准头文件
#include <stdint.h>
#include <stddef.h>

// 尝试包含需要的原始 SDK 头文件
// 这些路径会通过 -I 参数在编译时解析
#ifdef __has_include
  #if __has_include("gpio.h")
    #include "gpio.h"
  #endif
  #if __has_include("pinctrl.h")
    #include "pinctrl.h"
  #endif
  #if __has_include("soc_osal.h")
    #include "soc_osal.h"
  #endif
  #if __has_include("app_init.h")
    #include "app_init.h"
  #endif
  #if __has_include("boards.h")
    #include "boards.h"
  #endif
#else
  // 如果编译器不支持 __has_include，则尝试直接包含
  // 这可能会产生警告，但会继续编译
  #include "gpio.h"
  #include "pinctrl.h"
#endif

// 手动定义一些基本常量和类型（作为后备）
#ifndef CONFIG_SAMPLE_BLINKY_PIN
#define CONFIG_SAMPLE_BLINKY_PIN 5
#endif

// GPIO 相关常量
#ifndef HAL_PIO_FUNC_GPIO
#define HAL_PIO_FUNC_GPIO 0
#endif

#ifndef GPIO_DIRECTION_OUTPUT
#define GPIO_DIRECTION_OUTPUT 1
#endif

#ifndef GPIO_LEVEL_LOW
#define GPIO_LEVEL_LOW 0
#endif

// OSAL 相关的基本声明（如果找不到头文件）
#ifndef _OSAL_TASK_DEFINED
typedef struct osal_task osal_task;
#define _OSAL_TASK_DEFINED
#endif

// 函数声明（如果头文件中没有）
#ifndef UAPI_FUNCTIONS_DECLARED
extern int uapi_pin_set_mode(unsigned int pin, unsigned int mode);
extern int uapi_gpio_set_dir(unsigned int pin, unsigned int direction);
extern int uapi_gpio_set_val(unsigned int pin, unsigned int level);
extern int uapi_gpio_toggle(unsigned int pin);
#define UAPI_FUNCTIONS_DECLARED
#endif

#ifndef OSAL_FUNCTIONS_DECLARED
extern void osal_msleep(unsigned int ms);
extern void osal_printk(const char *fmt, ...);
extern void osal_kthread_lock(void);
extern void osal_kthread_unlock(void);
extern osal_task* osal_kthread_create(void* handler, int arg, const char *name, unsigned int stack_size);
extern int osal_kthread_set_priority(osal_task* task, unsigned int priority);
#define OSAL_FUNCTIONS_DECLARED
#endif

#endif // RUST_WRAPPER_H
"#;

    std::fs::write("wrapper.h", wrapper_content)
        .expect("Couldn't write wrapper.h");
}

fn create_manual_bindings() {
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let bindings_content = r#"
// 手动创建的绑定文件 - bindgen 后备方案
use cty::*;

// 常量定义
pub const CONFIG_SAMPLE_BLINKY_PIN: u32 = 5;
pub const HAL_PIO_FUNC_GPIO: u32 = 0;
pub const GPIO_DIRECTION_OUTPUT: u32 = 1;
pub const GPIO_LEVEL_LOW: u32 = 0;

// 不透明类型
#[repr(C)]
pub struct osal_task {
    _unused: [u8; 0],
}

// 函数声明
extern "C" {
    pub fn uapi_pin_set_mode(pin: c_uint, mode: c_uint) -> c_int;
    pub fn uapi_gpio_set_dir(pin: c_uint, direction: c_uint) -> c_int;
    pub fn uapi_gpio_set_val(pin: c_uint, level: c_uint) -> c_int;
    pub fn uapi_gpio_toggle(pin: c_uint) -> c_int;
    pub fn osal_msleep(ms: c_uint);
    pub fn osal_printk(fmt: *const c_char, ...);
    pub fn osal_kthread_lock();
    pub fn osal_kthread_unlock();
    pub fn osal_kthread_create(
        handler: Option<unsafe extern "C" fn(*const c_char) -> c_int>,
        arg: *mut c_void,
        name: *const c_char,
        stack_size: c_uint,
    ) -> *mut osal_task;
    pub fn osal_kthread_set_priority(task: *mut osal_task, priority: c_uint) -> c_int;
}
"#;

    std::fs::write(out_path.join("bindings.rs"), bindings_content)
        .expect("Couldn't write manual bindings!");
}
