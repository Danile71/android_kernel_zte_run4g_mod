LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

$(shell rm -f $(LOCAL_PATH)/custom)

LOCAL_SRC_FILES := \
    cust_pq_main.cpp 

LOCAL_SHARED_LIBRARIES := \
    libcutils \

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc,$(MTK_PLATFORM))/hardware/pq \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc,$(MTK_PLATFORM))/kernel/drivers/dispsys \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc

LOCAL_MODULE:= libpq_cust

LOCAL_MODULE_CLASS := SHARED_LIBRARIES
INTERMEDIATES := $(call local-intermediates-dir)

LOCAL_GENERATED_SOURCES += \
     $(INTERMEDIATES)/cust_pqdc.cpp

$(INTERMEDIATES)/cust_pqdc.cpp:$(MTK_ROOT_CUSTOM_OUT)/hal/pq/cust_pqdc.cpp
	@mkdir -p $(dir $@)
	@cp -f $< $@

LOCAL_GENERATED_SOURCES += \
     $(INTERMEDIATES)/cust_pqds.cpp

$(INTERMEDIATES)/cust_pqds.cpp:$(MTK_ROOT_CUSTOM_OUT)/hal/pq/cust_pqds.cpp
	@mkdir -p $(dir $@)
	@cp -f $< $@

include $(BUILD_SHARED_LIBRARY)
