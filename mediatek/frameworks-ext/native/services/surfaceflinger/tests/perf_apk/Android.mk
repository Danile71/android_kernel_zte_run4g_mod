#########################################################################
# This makefile builds both an activity and a shared library.
#########################################################################


LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := SurfaceUT

LOCAL_JNI_SHARED_LIBRARIES := libsurfaceut

LOCAL_REQUIRED_MODULES := libsurfaceut

include $(BUILD_PACKAGE)



#########################################################################
# Build JNI Shared Library
#########################################################################

LOCAL_PATH:= $(LOCAL_PATH)/jni

include $(CLEAR_VARS)

LOCAL_MODULE    := libsurfaceut
LOCAL_CFLAGS    := -Werror

LOCAL_SRC_FILES := \
	surface.cpp \
	utProc.cpp \
	FpsCounter.cpp

LOCAL_LDLIBS    := -llog -lcutils -lutils -lbinder -lui -lgui

LOCAL_SHARED_LIBRARIES += \
    libutils \
    libcutils \
    libbinder \
    libskia \
    libgui \
    libui \
    libnativehelper \
    libandroid_runtime \
    libandroid

LOCAL_C_INCLUDES += \
	$(TOP)/$(MTK_ROOT)/hardware/gralloc_extra/include

LOCAL_STATIC_LIBRARIES := \
	libgralloc_extra

#LOCAL_SHARED_LIBRARIES := \
#	libcutils \
#	libutils \
#	libbinder \
#	libui \
#	libgui

include $(BUILD_SHARED_LIBRARY)