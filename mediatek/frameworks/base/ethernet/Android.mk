ifeq ($(strip $(MTK_ALPS_BOX_SUPPORT)), yes) 

LOCAL_PATH := $(call my-dir)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
