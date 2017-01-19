LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES := telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += com.mediatek.settings.ext

LOCAL_PACKAGE_NAME := UsbChecker
LOCAL_MULTILIB := 32
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

::LOCAL_PROGUARD_FLAG_FILES := proguard.flags


::LOCAL_AAPT_FLAGS := --auto-add-overlay --extra-packages com.android.keyguard
::LOCAL_AAPT_FLAGS += --extra-packages com.mediatek.keyguard.ext

include $(BUILD_PACKAGE)

