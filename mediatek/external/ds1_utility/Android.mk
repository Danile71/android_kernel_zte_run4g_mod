
ifneq (,$(filter yes, $(MTK_AUDIO_DDPLUS_SUPPORT) $(MTK_DOLBY_DAP_SUPPORT)))

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := ds1_utility.cpp

LOCAL_SHARED_LIBRARIES :=  \
     libcutils \
     libutils \
     libbinder

LOCAL_MODULE := ds1_utility

include $(BUILD_STATIC_LIBRARY)

endif