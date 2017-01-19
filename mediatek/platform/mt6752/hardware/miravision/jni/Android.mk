LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	miravision_jni.cpp
	 
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
        $(MTK_PATH_PLATFORM)/kernel/drivers/dispsys \
        $(MTK_PATH_PLATFORM)/../../hardware/dpframework/inc \
        $(MTK_PATH_PLATFORM)/hardware/pq \
        $(MTK_PATH_PLATFORM)/hardware/aal/inc
	
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils

ifeq ($(MTK_AAL_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += \
    libaal
endif

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libMiraVision_jni

LOCAL_MODULE_CLASS := SHARED_LIBRARIES
INTERMEDIATES := $(call local-intermediates-dir)
LOCAL_GENERATED_SOURCES += \
     $(INTERMEDIATES)/cust_gamma.cpp

$(INTERMEDIATES)/cust_gamma.cpp:$(MTK_ROOT_CUSTOM_OUT)/hal/gamma/cust_gamma.cpp
	@mkdir -p $(dir $@)
	@cp -f $< $@

	
LOCAL_GENERATED_SOURCES += \
	$(INTERMEDIATES)/cust_color.cpp

$(INTERMEDIATES)/cust_color.cpp:$(MTK_ROOT_CUSTOM_OUT)/hal/pq/cust_color.cpp
	@mkdir -p $(dir $@)
	@cp -f $< $@


LOCAL_GENERATED_SOURCES += \
	$(INTERMEDIATES)/cust_tdshp.cpp

$(INTERMEDIATES)/cust_tdshp.cpp:$(MTK_ROOT_CUSTOM_OUT)/hal/pq/cust_tdshp.cpp
	@mkdir -p $(dir $@)
	@cp -f $< $@	

include $(BUILD_SHARED_LIBRARY)
