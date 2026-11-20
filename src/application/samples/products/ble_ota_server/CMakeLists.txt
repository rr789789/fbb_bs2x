#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
#===============================================================================
set(SOURCES_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/ble_ota_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ble_ota_server_adv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ble_ota.c
)

set(SOURCES "${SOURCES}" ${SOURCES_LIST} PARENT_SCOPE)