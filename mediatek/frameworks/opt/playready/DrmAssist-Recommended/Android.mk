LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := DrmAssist-Recommended
LOCAL_CERTIFICATE := media

LOCAL_JNI_SHARED_LIBRARIES += libDrmAssistJNI42 \
                              libDrmAssistJNI43 \
                              libDrmAssistJNI44

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))


