include $(RTOSLITETOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS_y := los_obj_alloc.c
LOCAL_SRCS    = $(LOCAL_SRCS_y)
#LOCAL_INCLUDE := \
	#-I $(LITEOSTOPDIR)/kernel_lite/extended/include

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS) $(LITEOS_NON_SECURE_LOCAL_OPTS)

include $(MODULE)
