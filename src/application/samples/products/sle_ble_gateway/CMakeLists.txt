#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
#===============================================================================

if(DEFINED CONFIG_SAMPLE_SUPPORT_SLE_BLE_GATEWAY)
    set(SOURCES_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/gateway_service.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gw_sle_adapter.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gw_ble_adapter.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gateway_entry.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gateway_demo.c
    )
    set(SOURCES "${SOURCES}" ${SOURCES_LIST} PARENT_SCOPE)
    set(PUBLIC_HEADER "${PUBLIC_HEADER}" ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()
