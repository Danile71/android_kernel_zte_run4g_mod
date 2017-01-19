LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    RefBaseDumpTunnel.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libui

LOCAL_MODULE:= test-RefBaseDumpTunnel

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
