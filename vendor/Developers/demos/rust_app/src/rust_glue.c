#include "app_init.h"

// 声明 Rust 导出的函数
extern void rust_app_init(void);

// 使用 app_run 宏注册 Rust 应用
app_run(rust_app_init);