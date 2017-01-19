LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_pq.cpp 

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
	$(MTK_PATH_PLATFORM)/kernel/drivers/dispsys \
	$(MTK_PATH_PLATFORM)/../../hardware/dpframework/inc

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libPQjni \
    libPQDCjni \
    libpq_prot \
    libpq_cust

LOCAL_MODULE:= pq

LOCAL_MODULE_CLASS := EXECUTABLES

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


include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))
