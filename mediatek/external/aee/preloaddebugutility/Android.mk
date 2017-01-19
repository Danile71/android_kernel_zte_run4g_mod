LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
		  debug.c

LOCAL_SHARED_LIBRARIES := libcutils

LOCAL_MODULE := libpldbgutil
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
