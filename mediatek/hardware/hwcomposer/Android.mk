LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_HWC_SUPPORT), yes)

LOCAL_SRC_FILES := \
	hwc.cpp

LOCAL_CFLAGS := \
	-DLOG_TAG=\"hwcomposer\"

ifeq ($(MTK_HDMI_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_EXTERNAL_SUPPORT
endif

ifeq ($(MTK_WFD_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_VIRTUAL_SUPPORT
endif

ifneq ($(MTK_PQ_SUPPORT), PQ_OFF)
LOCAL_CFLAGS += -DMTK_ENHAHCE_SUPPORT
endif

ifeq ($(TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS), true)
LOCAL_CFLAGS += -DMTK_FORCE_HWC_COPY_VDS
endif

MTK_HWC_CHIP = $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

ifneq ($(findstring 1.2, $(MTK_HWC_VERSION)),)
LOCAL_CFLAGS += -DMTK_HWC_VER_1_2
endif

ifneq ($(findstring 1.3, $(MTK_HWC_VERSION)),)
LOCAL_CFLAGS += -DMTK_HWC_VER_1_3
endif

LOCAL_STATIC_LIBRARIES += \
	hwcomposer.$(MTK_HWC_CHIP).$(MTK_HWC_VERSION) \

LOCAL_SHARED_LIBRARIES := \
	libEGL \
	libGLESv1_CM \
	libui \
	libutils \
	libcutils \
	libsync \
	libm4u \
	libion \
	libbwc \
	libdpframework \
	libhardware \
	libgui_ext \
	libui_ext \
	libgralloc_extra \
	libperfservicenative

ifeq ($(MTK_SEC_VIDEO_PATH_SUPPORT), yes)
ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += \
	libsec_mem
endif
endif # MTK_SEC_VIDEO_PATH_SUPPORT

# HAL module implemenation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
LOCAL_MODULE := hwcomposer.$(MTK_HWC_CHIP)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
include $(BUILD_SHARED_LIBRARY)

include $(MTK_ROOT)/tablet/symlink.mk

endif # MTK_HWC_SUPPORT
