#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
#===============================================================================
set(SOURCES "${SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/chinese_font.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tft.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/font.c"
    PARENT_SCOPE)
