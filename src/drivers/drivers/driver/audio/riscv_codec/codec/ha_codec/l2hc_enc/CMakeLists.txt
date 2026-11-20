#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
#===============================================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ROOT_DIR}/drivers/drivers/driver/audio/riscv_codec/codec/ha_codec/l2hc_enc/${CHIP}/)
file(GLOB OSAL_SOURCES_INC "${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos/*.c")
file(GLOB L2HC_ENC_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/encoder/*.c")
file(GLOB L2HC_COMMON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.c")
file(GLOB L2HC_SYS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/sys/*.c")
#component l2hc_enc_16k

set(COMPONENT_NAME "l2hc_enc_16k")

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xal2hc_enc_comp.c
   ${SOURCES}
   ${OSAL_SOURCES_INC}
   ${L2HC_ENC_SOURCE}
   ${L2HC_COMMON_SOURCE}
   ${L2HC_SYS_SOURCE}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/src/encoder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos
)

#set(LIBS ${LIB_PATH_16K})

set(PRIVATE_DEFINES
)

set(PUBLIC_DEFINES
)

# use this when you want to add ccflags like -include xxx
set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(WHOLE_LINK
    false
)

set(MAIN_COMPONENT
    false
)

set(DEFINES)

set(CCFLAGS -O2 -DLINX_CODEC -DLINX_L2HC_ENC -DL2HC_10MS_ENABLE -DL2HC_16K_SAMPLERATE_ENABLE
    -ffunction-sections
    -femit-clz -femit-muliadd -fimm-compare -fmerge-immshf -fldm-stm-optimize -femit-lli)

set(LIB_OUT_PATH ${ROOT_DIR}/interim_binary/${CHIP}/libs/audio/codec)

build_component()

#component l2hc_enc_48k_10ms

set(COMPONENT_NAME "l2hc_enc_48k_10ms")

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xal2hc_enc_comp.c
   ${SOURCES}
   ${OSAL_SOURCES_INC}
   ${L2HC_ENC_SOURCE}
   ${L2HC_COMMON_SOURCE}
   ${L2HC_SYS_SOURCE}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/src/encoder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos
)

#set(LIBS ${LIB_PATH_48K_10MS})

set(PRIVATE_DEFINES
)

set(PUBLIC_DEFINES
)

# use this when you want to add ccflags like -include xxx
set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(WHOLE_LINK
    false
)

set(MAIN_COMPONENT
    false
)

set(DEFINES)

set(CCFLAGS -O2 -DLINX_CODEC -DLINX_L2HC_ENC -DL2HC_10MS_ENABLE -DL2HC_48K_SAMPLERATE_ENABLE
    -ffunction-sections
    -femit-clz -femit-muliadd -fimm-compare -fmerge-immshf -fldm-stm-optimize -femit-lli)

set(LIB_OUT_PATH ${ROOT_DIR}/interim_binary/${CHIP}/libs/audio/codec)

build_component()

#component l2hc_enc_48k_5ms

set(COMPONENT_NAME "l2hc_enc_48k_5ms")

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xal2hc_enc_comp.c
   ${SOURCES}
   ${OSAL_SOURCES_INC}
   ${L2HC_ENC_SOURCE}
   ${L2HC_COMMON_SOURCE}
   ${L2HC_SYS_SOURCE}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/src/encoder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos
)

#set(LIBS ${LIB_PATH_48K_5MS})

set(PRIVATE_DEFINES
)

set(PUBLIC_DEFINES
)

# use this when you want to add ccflags like -include xxx
set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(WHOLE_LINK
    false
)

set(MAIN_COMPONENT
    false
)

set(DEFINES)

set(CCFLAGS -O2 -DLINX_CODEC -DLINX_L2HC_ENC -DL2HC_5MS_ENABLE -DL2HC_48K_SAMPLERATE_ENABLE
    -ffunction-sections
    -femit-clz -femit-muliadd -fimm-compare -fmerge-immshf -fldm-stm-optimize -femit-lli)

set(LIB_OUT_PATH ${ROOT_DIR}/interim_binary/${CHIP}/libs/audio/codec)

build_component()
