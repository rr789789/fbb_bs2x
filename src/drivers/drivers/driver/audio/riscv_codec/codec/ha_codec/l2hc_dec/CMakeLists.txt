#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2022. All rights reserved.
#===============================================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ROOT_DIR}/drivers/drivers/driver/audio/riscv_codec/codec/ha_codec/l2hc_dec/${CHIP}/)
file(GLOB OSAL_SOURCES_INC "${CMAKE_CURRENT_SOURCE_DIR}/../../../osal/liteos/*.c")
file(GLOB SOURCES_DECODER "${CMAKE_CURRENT_SOURCE_DIR}/src/decoder/*.c")
file(GLOB L2HC_COMMON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.c")

# l2hc_dec_16k
set(COMPONENT_NAME "l2hc_dec_16k")

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xal2hc_dec_comp.c
   ${SOURCES_DECODER}
   ${L2HC_COMMON_SOURCE}
   ${SOURCES}
   ${OSAL_SOURCES_INC}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/decoder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos
    ${ROOT_DIR}/open_source/libboundscheck/include
)

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

set(CCFLAGS -O2 -DLINX_CODEC -DLINX_L2HC_DEC -DL2HC_10MS_ENABLE -DL2HC_16K_SAMPLERATE_ENABLE
    -femit-clz -femit-muliadd -fimm-compare -fmerge-immshf -femit-lli
    -ffunction-sections -fdata-sections --data-sections -fldm-stm-optimize
    -DENABLE_FIXED_POINT -DRAM_OPT -DLOW_COST)

set(LIB_OUT_PATH ${ROOT_DIR}/interim_binary/${CHIP}/libs/audio/codec)

build_component()

# l2hc_dec_48k_10ms
set(COMPONENT_NAME "l2hc_dec_48k_10ms")

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xal2hc_dec_comp.c
   ${SOURCES_DECODER}
   ${L2HC_COMMON_SOURCE}
   ${SOURCES}
   ${OSAL_SOURCES_INC}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/decoder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos
    ${ROOT_DIR}/open_source/libboundscheck/include
)

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

set(CCFLAGS -O2 -DLINX_CODEC -DLINX_L2HC_DEC -DL2HC_10MS_ENABLE -DL2HC_48K_SAMPLERATE_ENABLE
    -femit-clz -femit-muliadd -fimm-compare -fmerge-immshf -femit-lli
    -ffunction-sections -fdata-sections --data-sections -fldm-stm-optimize
    -DENABLE_FIXED_POINT -DRAM_OPT -DLOW_COST)

set(LIB_OUT_PATH ${ROOT_DIR}/interim_binary/${CHIP}/libs/audio/codec)

build_component()

# l2hc_dec_48k_5ms
set(COMPONENT_NAME "l2hc_dec_48k_5ms")

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xal2hc_dec_comp.c
   ${SOURCES_DECODER}
   ${L2HC_COMMON_SOURCE}
   ${SOURCES}
   ${OSAL_SOURCES_INC}
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/decoder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../framework/osal/liteos
    ${ROOT_DIR}/open_source/libboundscheck/include
)

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

set(CCFLAGS -O2 -DLINX_CODEC -DLINX_L2HC_DEC -DL2HC_5MS_ENABLE -DL2HC_48K_SAMPLERATE_ENABLE
    -femit-clz -femit-muliadd -fimm-compare -fmerge-immshf -femit-lli
    -ffunction-sections -fdata-sections --data-sections -fldm-stm-optimize
    -DENABLE_FIXED_POINT -DRAM_OPT -DLOW_COST)

set(LIB_OUT_PATH ${ROOT_DIR}/interim_binary/${CHIP}/libs/audio/codec)

build_component()
