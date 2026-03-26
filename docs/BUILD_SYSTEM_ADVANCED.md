# fbb_bs2x 编译系统高阶使用指南

本文档深入介绍 fbb_bs2x SDK 编译系统的内部机制和高阶用法，适用于需要进行深度定制、问题排查和性能优化的开发者。

---

## 目录

1. [编译系统架构概览](#1-编译系统架构概览)
2. [目标配置系统详解](#2-目标配置系统详解)
3. [组件模型深度解析](#3-组件模型深度解析)
4. [ROM/RAM 分离机制](#4-romram-分离机制)
5. [链接脚本自动生成](#5-链接脚本自动生成)
6. [SDK 生成与闭源组件](#6-sdk-生成与闭源组件)
7. [签名与打包系统](#7-签名与打包系统)
8. [NV 配置与分区表](#8-nv-配置与分区表)
9. [HSO 数据库系统](#9-hso-数据库系统)
10. [高级调试技巧](#10-高级调试技巧)

---

## 1. 编译系统架构概览

### 1.1 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        build.py (入口)                           │
│                   参数解析、目标分发                              │
└──────────────────────┬──────────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────────┐
│              cmake_builder.py (Python 构建层)                   │
│        目标环境初始化、CMake 命令组装、构建流程控制               │
└──────────────────────┬──────────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────────┐
│                     CMake 构建系统                               │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌───────────┐ │
│  │ build_component│ build_linker│ build_sign  │ build_nv_bin│ │
│  └─────────────┘ └─────────────┘ └─────────────┘ └───────────┘ │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌───────────┐ │
│  │ build_rom_  │ build_hso_db  │ build_partition│ build_sdk  │ │
│  │ _callback   │               │               │            │ │
│  └─────────────┘ └─────────────┘ └─────────────┘ └───────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 关键构建阶段

```python
# 编译流程（cmake_builder.py）
def compile_target(self, target_name, env):
    1. deal_symbol_link(env)        # 处理 ROM 符号链接
    2. add_build_param(env)         # 添加构建参数
    3. pre_sdk(output_path, env)    # SDK 预处理
    4. rom_callback(env, ...)       # ROM 回调生成（两阶段编译）
    5. start(env, ...)              # 实际编译执行
    6. rom_check(env, ...)          # ROM 一致性检查
```

---

## 2. 目标配置系统详解

### 2.1 配置层次结构

目标配置采用三层继承结构：

```python
# 第一层：target_template（模板定义）
# 文件：src/build/config/target_config/<chip>/target_config.py
target_template = {
    'target_bs21e_application_template': {
        'chip': 'bs21e',
        'core': 'acore',
        'tool_chain': 'riscv32_musl_b010_fp',
        'defines': [...],
        'ram_component': [...],
        'rom_component': [...],
    }
}

# 第二层：target（具体目标）
# 文件：src/build/config/target_config/<chip>/config.py
target = {
    'standard-bs21e-1100e': {
        'base_target_name': 'target_bs21e_application_template',
        'defines': [...],           # 继承并扩展
        'ram_component': [...],     # 继承并扩展
        'rom_component': [...],
    }
}
```

### 2.2 关键配置项说明

| 配置项 | 类型 | 说明 |
|--------|------|------|
| `ram_component` | list | RAM 中编译的组件列表，支持 `-:component` 排除语法 |
| `rom_component` | list | ROM 中编译的组件列表 |
| `ram_component_set` | list | 组件集合别名，通过 `common_config.py` 展开 |
| `defines` | list | 编译宏定义，支持 `-:MACRO` 删除语法 |
| `ccflags` | list | C 编译器选项，支持 `-:-option` 删除语法 |
| `linkflags` | list | 链接器选项 |
| `build_rom_callback` | bool | 是否启用 ROM 回调生成机制 |
| `fixed_rom` | bool | 是否使用固定 ROM 二进制 |
| `rom_sym_path` | str | ROM 符号表路径，用于符号链接 |

### 2.3 配置继承与覆盖规则

```python
# 示例：standard-bs21e-1100e 配置继承链
target_bs21e_application_template (基模板)
    ↓ 继承
standard-bs21e-1100e (具体目标)
    ↓ 继承
bs21e-1100e-slp (变体目标，继承自 standard-bs21e-1100e)

# 合并规则：
# 1. list 类型：合并去重，`-:item` 表示删除
# 2. str/bool：直接覆盖
```

---

## 3. 组件模型深度解析

### 3.1 build_component() 完整流程

```cmake
macro(build_component)
    1. component_on_rom()           # 判断组件在 RAM 还是 ROM
    2. need_whole_archiv()          # 判断是否需要整库链接
    3. 自动添加头文件目录：
       - private/ → PRIVATE_HEADER
       - public/ → PUBLIC_HEADER
       - include/ → PUBLIC_HEADER
    4. 根据 ROM/RAM 设置不同编译选项
    5. 创建 interface 库（供依赖使用）
    6. build_library()              # 构建静态库
    7. reg_rom_callback()           # 注册 ROM 回调（如需要）
    8. define_this_module_and_file_id()  # 自动定义模块 ID
    9. sdk_export_component()       # SDK 导出信息
endmacro()
```

### 3.2 组件变量全览

```cmake
# 必需变量
set(COMPONENT_NAME "hal_uart")          # 组件唯一标识
set(SOURCES "file.c")                    # 源文件列表

# 可选变量
set(PUBLIC_HEADER "include")             # 公共头文件目录（暴露给依赖者）
set(PRIVATE_HEADER "private")            # 私有头文件目录
set(PUBLIC_DEFINES "CONFIG_UART=1")      # 公共宏定义
set(PRIVATE_DEFINES "INTERNAL=1")        # 私有宏定义
set(COMPONENT_CCFLAGS "-O3")             # 组件专属编译选项
set(WHOLE_LINK true)                     # 强制整库链接
set(BUILD_AS_OBJ true)                   # 编译为对象文件而非静态库
set(MAIN_COMPONENT true)                 # 标记为主组件（SDK 用）
set(LIBS "libxxx.a")                     # 依赖的第三方库
set(LIB_DIR "lib")                       # 第三方库搜索路径

# 条件编译（基于 Kconfig）
if(DEFINED CONFIG_UART_USING_V120)
    list(APPEND SOURCES "v120/hal_uart_v120.c")
endif()

build_component()
```

### 3.3 组件集合（Component Set）

```python
# 文件：src/build/config/target_config/common_config.py
component_set = {
    'uart': ['hal_uart', 'uart', 'uart_port'],
    'time_set': ['systick', 'tcxo', 'drv_timer', ...],
    'security': ['hal_hash', 'hash', 'trng', ...],
}

# 使用方式（在 target 配置中）
'ram_component_set': ['uart', 'time_set']
```

### 3.4 闭源组件机制

```python
# SDK 生成时的闭源组件处理
sdk_close_components = ['secure_lib']  # 标记为闭源

# 在组件 CMakeLists.txt 中
if(CLOSED_COMPONENTS)
    # 闭源组件只发布 .a 库，不发布源代码
    install_sdk(${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_COMMAND}/lib${COMPONENT_NAME}.a "*.a")
else()
    # 开源组件发布源代码
    install_sdk(${src} "*")
endif()
```

---

## 4. ROM/RAM 分离机制

### 4.1 ROM 回调机制原理

ROM 回调用于解决 ROM 代码调用 RAM 函数的问题（长跳转）：

```
ROM 代码 (.romtext 段)
    ↓ 调用
RAM 函数 (通过 ROM Callback 中转)
    ↓ 长跳转
实际 RAM 函数实现
```

### 4.2 两阶段编译流程

```python
# cmake_builder.py::rom_callback()
def rom_callback(self, env, target_name, output_path):
    if not env.get('build_rom_callback'):
        return False

    # 第一阶段：预编译 ROM 组件
    self.start(env, target_name, output_path, clean=True)

    # 生成 ROM 回调相关信息
    gen_rom_ram_callback(target, rom_output)

    # 第二阶段：重新编译，使用生成的回调
    env.append('defines', '_PRE_FEATURE_VENEER_ROM')
    env.append('ram_component', 'rom_callback')
    return True
```

### 4.3 ROM 回调自动生成文件

| 文件 | 说明 |
|------|------|
| `rom_callback.S` | ARM/RISC-V 汇编跳板代码 |
| `rom_callback_wrap.cmake` | 符号重定义宏（`__wrap_xxx` → `__real_xxx`） |
| `function.lds` | 函数段链接脚本片段 |
| `data.lds` | 数据段链接脚本片段 |
| `rom_ram_func.list` | ROM-RAM 函数调用列表 |
| `rom_ram_call_data.list` | ROM-RAM 数据访问列表 |

### 4.4 手动控制 ROM 回调

```cmake
# 在组件 CMakeLists.txt 中强制启用/禁用 ROM 回调
set(FORCE_ROM_CALLBACK true)   # 强制为 ROM 组件生成回调
set(SKIP_ROM_CALLBACK true)    # 跳过 ROM 回调生成
```

### 4.5 符号链接（Symbol Link）

```python
# 使用 ROM 符号表进行链接
deal_symbol_link(env):
    if env.config["rom_sym_path"]:
        # 添加符号链接标志
        env.config["linkflags"].append("-Wl,--just-symbols=" + rom_sym_path)
        env.config["defines"].append("ROM_SYMBOL_LINK")
```

---

## 5. 链接脚本自动生成

### 5.1 链接脚本处理流程

```cmake
# build_linker.cmake

# 1. 自动生成库函数宏
auto_gen_lds("LIB_RAM_FUNC" RAM_COMPONENT)
auto_gen_lds("LIB_ROM_FUNC" ROM_COMPONENT)

# 2. 预处理链接脚本（使用 C 预处理器）
add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
    COMMAND ${CMAKE_C_COMPILER} -P -xc -E -o linker.lds
            ${LDS_DEFINES} ${LDS_HEADER} ${LINK_SCRIPT}
)

# 3. 生成的 linker.lds 包含：
#    - 自动生成的库函数段定义
#    - 条件编译处理后的内存布局
#    - ROM/RAM 段分离定义
```

### 5.2 链接脚本模板语法

```ld
/* 链接脚本模板 (.prelds) */

/* 使用 C 预处理器条件 */
#ifdef CONFIG_SFC_SUPPORT_RWE_INDEPENDENT
    .fls_loader_ram : { KEEP(*(.fls_loader_ram)) }
#endif

/* 使用自动生成的库函数宏 */
LIB_RAM_FUNC(KEEP)
LIB_ROM_FUNC(KEEP)

/* 标准段定义 */
SECTIONS {
    .text : {
        *(.text*)
        *(.romtext*)    /* ROM 代码段 */
    } > ROM

    .data : {
        *(.data*)
    } > RAM AT> ROM
}
```

### 5.3 内存布局配置

```python
# 分区配置（flash_sector_config）
SECTOR_CFG = 'bs21e-standard'  # 引用 bs21e-standard.json

# JSON 格式分区表
{
    "partitions": [
        {"name": "boot", "offset": "0x0", "size": "0x10000"},
        {"name": "app", "offset": "0x10000", "size": "0xF0000"},
    ]
}
```

---

## 6. SDK 生成与闭源组件

### 6.1 SDK 生成命令

```bash
# 生成 SDK 包（包含源代码和工具链）
python src/build.py sdk-bs21e-standard

# SDK 类型在 target 配置中指定
'sdk_type': ['src', 'project']  # src=源代码, project=项目文件
```

### 6.2 SDK 生成流程

```python
class SdkGenerator:
    def __init__(self, env, sdk_root_path):
        1. 初始化复制模块（处理路径替换）
        2. 设置闭源组件掩码
        3. 解析 CMake 追踪文件（获取依赖关系）

    def parse_depend_cmake_files(self, cmake_trace_file):
        # 解析 cmake --trace 输出，获取完整依赖树
        # 生成 SDK 需要的 CMakeLists.txt 子集

    def sdk_build(self, build_time, no_hso, build_level):
        1. 复制公共工具文件
        2. 根据 trace 复制 CMake 文件
        3. 处理闭源组件（替换源码为 .a 库）
        4. 生成项目配置文件
        5. 生成工具链配置文件
```

### 6.3 闭源组件配置

```python
# target_config.py 中配置
sdk_close_components = [
    'secure_crypto',   # 这些组件只发布 .a 库
    'proprietary_algo',
]

# 或使用白名单模式
open_components = ['public_component']  # 只开源这些组件，其他都闭源
```

### 6.4 SDK 库生成（Lib Gen）

```python
# 将多个组件打包为单个库
'lib_gen': {
    'libmy_sdk.a': {
        'base_target_name': 'standard-bs21e-1100e',
        'components': ['app_init', 'my_component'],
        'defines': ['SDK_EXPORT'],
    }
}
```

---

## 7. 签名与打包系统

### 7.1 安全启动签名流程

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  loaderboot │────▶│  flashboot  │────▶│  application│
│  (二级加载)  │     │  (主启动)    │     │  (应用)      │
└─────────────┘     └─────────────┘     └─────────────┘
       │                   │                   │
       ▼                   ▼                   ▼
  loaderboot_sign     flashboot_sign      application_sign
       │                   │                   │
       └───────────────────┴───────────────────┘
                           │
                           ▼
                    使用 sign_tool 签名
```

### 7.2 签名配置

```bash
# 签名配置文件位置
src/build/config/target_config/<chip>/sign_config/<target>.cfg

# 签名命令（内部调用）
sign_tool 0 <config_file>
```

### 7.3 固件包生成（fwpkg）

```bash
# 打包命令
python src/build.py pack-bs21e-standard

# 或构建时自动打包
python src/build.py standard-bs21e-1100e  # packet: true 时

# 输出位置
tools/pkg/fwpkg/bs21e/bs21e_all.fwpkg
```

### 7.4 分区配置选择

```python
# 不同产品使用不同分区表
'sector_cfg': 'bs21e-standard'           # 标准分区
'sector_cfg': 'bs21e-slp-central'        # SLP 主设备分区
'sector_cfg': 'bs21e-slp-peripheral'     # SLP 从设备分区
```

---

## 8. NV 配置与分区表

### 8.1 NV 系统架构

```
NV 配置文件层次：
├─ nv_target.json          # NV 条目定义（结构体描述）
├─ <nv_cfg>/
│  ├─ cfg/                 # NV 默认值配置
│  │  ├─ nv_0000.cfg
│  │  └─ nv_0001.cfg
│  └─ include/             # NV 头文件
└─ generated/              # 生成的 NV 二进制
```

### 8.2 NV 配置目标选择

```python
# target 配置中指定 NV 配置集
'nv_cfg': 'bs21e_nv_default'     # 默认 NV 配置
'nv_cfg': 'bs21e_nv_slp'         # SLP 模式 NV 配置
```

### 8.3 NV 编译流程

```cmake
# build_nv_bin.cmake

1. 预处理 NV 头文件（生成 .etypes）
   COMMAND ${CMAKE_C_COMPILER} -E ${GEN_TARGET_SRC} -o ${PRECOMPILE_TARGET}

2. 生成 NV 二进制
   COMMAND python ${BUILD_NV_TOOL} ${NV_CFG_JSON} ${CORE} ${CRC_FLAGS}

3. 输出
   - interim_binary/<chip>/bin/nv/<target>/<chip>_all_nv.bin
   - output/<chip>/<core>/<target>/<chip>_all_nv.bin
```

### 8.4 分区表配置

```json
// flash_sector_config/bs21e-standard.json
{
    "flash_size": "0x100000",
    "partitions": [
        {"name": "loaderboot", "offset": "0x000000", "size": "0x010000"},
        {"name": "flashboot",  "offset": "0x010000", "size": "0x010000"},
        {"name": "nv",         "offset": "0x020000", "size": "0x010000"},
        {"name": "app",        "offset": "0x030000", "size": "0x0D0000"}
    ]
}
```

---

## 9. HSO 数据库系统

### 9.1 HSO（HiSilicon Operation）系统

HSO 是海思维测系统，用于日志解析和调试：

```
源代码中的日志宏
       │
       ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  LOG_DEF    │────▶│  XML 生成   │────▶│  HSO 数据库 │
│  定义文件   │     │  (mk_hso_)  │     │  (.hdb)    │
└─────────────┘     └─────────────┘     └─────────────┘
                                               │
                                               ▼
                                        HSO 工具解析日志
```

### 9.2 HSO 配置

```python
# target 配置
'hso_enable': True,           # 启用 HSO 数据库生成
'hso_enable_bt': True,        # 包含蓝牙状态 HSO
'gen_parse_tool': True,       # 生成日志解析工具
```

### 9.3 日志定义文件

```cmake
# 在组件 CMakeLists.txt 中
set(LOG_DEF "${CMAKE_CURRENT_SOURCE_DIR}/my_log_def.xml")
```

### 9.4 跳过 HSO 构建

```bash
# 快速构建（跳过 HSO 数据库更新）
python src/build.py -nhso standard-bs21e-1100e
```

---

## 10. 高级调试技巧

### 10.1 构建问题诊断

```bash
# 1. 查看详细构建日志
python src/build.py -dump standard-bs21e-1100e

# 2. 查看 CMake 命令
cat output/bs21e/acore/standard-bs21e-1100e/cmake_command.txt

# 3. 查看编译数据库（用于 IDE）
cat output/bs21e/acore/standard-bs21e-1100e/compile_commands.json

# 4. 组件依赖信息
cat output/bs21e/acore/standard-bs21e-1100e/component_info/info.json
```

### 10.2 内存分析

```bash
# 1. 查看 MAP 文件
vim output/bs21e/acore/standard-bs21e-1100e/application.map

# 2. 内存统计脚本
python src/build/script/utils/mem_stats.py <map_file>

# 3. 代码大小统计
python src/build/script/codesize_statistic.py
```

### 10.3 ROM 一致性检查

```python
# 自动检查（build_rom_callback 后）
if compare_bin(rom_bin, expected_rom_bin):
    print("ROM 一致性检查通过")
else:
    print("ROM 不一致！")  # 会终止构建
```

### 10.4 增量构建优化

```bash
# 仅构建指定组件（不改变链接）
python src/build.py -component=hal_uart,drivers standard-bs21e-1100e

# 使用 Ninja 加速（比 Make 快 20-30%）
python src/build.py -ninja standard-bs21e-1100e

# 条件重新生成（自动检测 CMake 命令变化）
# cmake_builder.py::is_command_refresh()
```

### 10.5 自定义构建步骤

```python
# 在 target_config/<chip>/script/custom_cmd.py 中
def run_custom_cmd(env, target, stage):
    if stage == 'build_pre':
        # 构建前执行
        pass
    elif stage == 'build_post':
        # 构建后执行
        pass
```

### 10.6 Kconfig 交互配置

```bash
# 打开 menuconfig（仅单个目标时有效）
python src/build.py menuconfig-bs21e-standard

# 保存配置后自动同步到 .config
# 然后正常构建
python src/build.py standard-bs21e-1100e
```

---

## 附录：常用命令速查表

| 命令 | 说明 |
|------|------|
| `python src/build.py <target>` | 构建指定目标 |
| `python src/build.py -c <target>` | 清理后构建 |
| `python src/build.py -j8 <target>` | 8 线程并行构建 |
| `python src/build.py -ninja <target>` | 使用 Ninja |
| `python src/build.py -nhso <target>` | 跳过 HSO |
| `python src/build.py -def=XXX=1 <target>` | 添加宏定义 |
| `python src/build.py -component=a,b <target>` | 仅构建组件 |
| `python src/build.py -dump <target>` | 详细输出 |
| `python src/build.py sdk-<target>` | 生成 SDK |
| `python src/build.py pack-<chip>` | 打包固件 |

---

*文档版本：2024.03*
*适用 SDK 版本：fbb_bs2x BS2X-1.0.17 及以上*
