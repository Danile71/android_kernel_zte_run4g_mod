ifneq ($(strip $(MTK_PLATFORM)),)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += src/com/mediatek/atci/service/IFMRadioService.aidl


LOCAL_PACKAGE_NAME := AtciService
LOCAL_CERTIFICATE := platform
LOCAL_REQUIRED_MODULES := libatciserv_jni

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-framework

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
