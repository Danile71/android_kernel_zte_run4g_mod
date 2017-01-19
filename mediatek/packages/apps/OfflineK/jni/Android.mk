	LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lm
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_MODULE    := libSdioETT
LOCAL_SRC_FILES := SdioETT.cpp ett_core.cpp

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)