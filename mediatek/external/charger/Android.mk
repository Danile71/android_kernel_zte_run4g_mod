
ifneq (,$(filter yes, $(MTK_KERNEL_POWER_OFF_CHARGING)))

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
                  common.cpp \
                  lights.cpp \
                  bootlogo.cpp \
				  main.cpp \
                  key_control.cpp \
                  charging_control.cpp \
                  alarm_control.cpp

LOCAL_CFLAGS += $(MTK_CDEFS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libshowlogo/                   \
 $(MTK_PATH_CUSTOM)/lk/include/target \
 $(MTK_PATH_PLATFORM)/lk/include/target \
 $(LOCAL_PATH)/include \
 $(TOP)/external/zlib/

LOCAL_C_INCLUDES += bootable/bootloader/lk/target/$(LK_PROJECT)/inc
LOCAL_C_INCLUDES += bootable/bootloader/lk/target/$(LK_PROJECT)/include/target

LOCAL_MODULE:= kpoc_charger

LOCAL_SHARED_LIBRARIES := libcutils libutils libc libstdc++ libz libdl liblog libgui libui libshowlogo \
	libhardware_legacy libsuspend


include $(BUILD_EXECUTABLE)

endif
