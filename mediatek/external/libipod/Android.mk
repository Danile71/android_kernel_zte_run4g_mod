#
# libipod
#
ifeq (yes, $(strip $(MTK_IPO_SUPPORT)))

LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libipod.so

include $(BUILD_MULTI_PREBUILT)

endif
