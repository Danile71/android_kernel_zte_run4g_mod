LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := uree_test
LOCAL_SRC_FILES := test.c
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += \
    $(call include-path-for, trustzone) \
    $(call include-path-for, trustzone-uree)

LOCAL_CFLAGS += -Wall -Wno-unused-parameter -Werror

#LOCAL_CFLAGS += ${TZ_CFLAG}
#LOCAL_LDFLAGS += --gc-sections
#LOCAL_ASFLAGS += -DASSEMBLY
#LOCAL_STATIC_LIBRARIES += libc_tz libtest
LOCAL_SHARED_LIBRARIES += libtz_uree
include $(BUILD_EXECUTABLE)

