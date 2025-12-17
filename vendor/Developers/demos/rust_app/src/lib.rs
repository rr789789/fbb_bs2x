#![no_std]
#![no_main]

// 包含生成的绑定（在编译时由 build.rs 生成）
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

// 如果 build.rs 生成失败，则使用本地绑定文件作为后备
#[cfg(not(any()))]
mod bindings;
#[cfg(not(any()))]
use bindings::*;

// 常量定义
const BLINKY_DURATION_MS: u32 = 500;
const BLINKY_TASK_PRIO: u32 = 24;
const BLINKY_TASK_STACK_SIZE: u32 = 0x1000;

// 任务函数
unsafe extern "C" fn blinky_task(_arg: *const cty::c_char) -> cty::c_int {
    // 设置引脚模式
    uapi_pin_set_mode(CONFIG_SAMPLE_BLINKY_PIN, HAL_PIO_FUNC_GPIO as u32);
    
    // 设置 GPIO 方向和初始值
    uapi_gpio_set_dir(CONFIG_SAMPLE_BLINKY_PIN, GPIO_DIRECTION_OUTPUT as u32);
    uapi_gpio_set_val(CONFIG_SAMPLE_BLINKY_PIN, GPIO_LEVEL_LOW as u32);
    
    loop {
        osal_msleep(BLINKY_DURATION_MS);
        uapi_gpio_toggle(CONFIG_SAMPLE_BLINKY_PIN);
        
        // 使用简单的字符串输出
        let msg = b"Blinky working.\r\n\0";
        osal_printk(msg.as_ptr() as *const cty::c_char);
    }
}

// 入口函数
#[no_mangle]
pub unsafe extern "C" fn blinky_entry() {
    let mut task_handle: *mut osal_task = core::ptr::null_mut();
    
    osal_kthread_lock();
    
    task_handle = osal_kthread_create(
        Some(blinky_task),
        0 as *mut cty::c_void,
        b"BlinkyTask\0".as_ptr() as *const cty::c_char,
        BLINKY_TASK_STACK_SIZE,
    );
    
    if !task_handle.is_null() {
        osal_kthread_set_priority(task_handle, BLINKY_TASK_PRIO);
    }
    
    osal_kthread_unlock();
}

// 模拟 app_run 宏的效果
#[no_mangle]
pub extern "C" fn rust_app_init() {
    unsafe {
        blinky_entry();
    }
}

// Panic 处理
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}