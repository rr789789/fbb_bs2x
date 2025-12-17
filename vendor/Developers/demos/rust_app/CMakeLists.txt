#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
#===============================================================================
set(COMPONENT_NAME "rust_app")

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rust_glue.c
)

set(PRIVATE_HEADER
)

set(PUBLIC_HEADER
)

set(PRIVATE_DEFINES
)

set(PUBLIC_DEFINES
)

set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/target/riscv32imfc-unknown-none-elf/debug/librust_app.a
)

set(WHOLE_LINK
true
)

set(BUILD_AS_OBJ
false
)

set(MAIN_COMPONENT
false
)

build_component()