LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := BluetoothLe
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS

#Path to aspect root folder
ifeq ($(strip $(MTK_AUTO_TEST)), yes)

LOCAL_ASPECTS_DIR := aspect/BluetoothLe/src

endif



LOCAL_CERTIFICATE := platform


LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += telephony-common mms-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common

LOCAL_STATIC_JAVA_LIBRARIES := joda-time-2.3 aspectj-runtime
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.bluetoothle.ext

# LOCAL_REQUIRED_MODULES := Add the required module

# Add for Proguard
ifneq ($(strip $(MTK_AUTO_TEST)), yes)
LOCAL_PROGUARD_ENABLED := custom
LOCAL_PROGUARD_FLAG_FILES := proguard.flags
endif
# Add for Proguard
# Added for JPE begin
LOCAL_JAVASSIST_ENABLED := true
LOCAL_JAVASSIST_OPTIONS := $(LOCAL_PATH)/jpe.config
# Added For JPE end

include $(BUILD_PREBUILT)

##################################################
include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := joda-time-2.3:lib/joda-time-2.3.jar


