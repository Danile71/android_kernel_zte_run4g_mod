LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	mediatek/hardware/gralloc_extra/include

LOCAL_SRC_FILES:= \
	autoVDSaddVideo.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
    libui \
    libgui

LOCAL_MODULE:= autoVDSaddVideoUnitTest

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
