LOCAL_PATH:= $(call my-dir)
LOCAL_INCLUDES := $(LOCAL_PATH)/libwifi2agps

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= libwifi2agps/nl80211.c libwifi2agps/event_loop.c \
interface/agps2wifi_interface.c interface/data_coder.c interface/wifi2agps_interface.c
LOCAL_C_INCLUDES := external/libnl-headers $(LOCAL_INCLUDES)
LOCAL_MODULE := libwifi2agps
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := main.c
LOCAL_C_INCLUDES := $(LOCAL_INCLUDES)
LOCAL_SHARED_LIBRARIES := libc liblog libcutils
LOCAL_STATIC_LIBRARIES := libwifi2agps libnl_2
LOCAL_MODULE := wifi2agps
include $(BUILD_EXECUTABLE)
