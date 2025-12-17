// 手动创建的绑定文件
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