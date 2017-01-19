LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	gralloc_test.cpp

LOCAL_C_INCLUDES := 

LOCAL_CFLAGS := \
	-DLOG_TAG=\"test-gralloc\" \
	-DGRALLOC_EXTRA_SUPPORT

LOCAL_SHARED_LIBRARIES := \
	libui \
	libutils \
	libgralloc_extra

LOCAL_MODULE := test-gralloc

include $(BUILD_EXECUTABLE)
