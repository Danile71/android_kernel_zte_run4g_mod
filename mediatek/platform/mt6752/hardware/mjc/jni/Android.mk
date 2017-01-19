ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)

ifeq ($(MTK_PLATFORM), $(filter $(MTK_PLATFORM) , MT6752))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	mjc_jni.cpp
	 
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
	
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libMJCjni

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif

endif
