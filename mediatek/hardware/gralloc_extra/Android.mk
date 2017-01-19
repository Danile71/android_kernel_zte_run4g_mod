LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	GraphicBufferExtra.cpp \
	GraphicBufferExtra_hal.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	external/zlib \
	external/libpng \
	external/skia/src/images \
	external/skia/include/core

LOCAL_SHARED_LIBRARIES := \
	libui \
	libpng \
	libcutils \
	libhardware \
	libutils \
	libdl \
	libpng \
	libz

LOCAL_EXPORT_C_INCLUDE_DIRS := \
	$(LOCAL_PATH)/include

LOCAL_MODULE := libgralloc_extra

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	GraphicBufferExtra.cpp \
	GraphicBufferExtra_hal.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	external/zlib \
	external/libpng \
	external/skia/src/images \
	external/skia/include/core

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libpng \
	libz

#LOCAL_SHARED_LIBRARIES := \
#	libpng \
#	libcutils \
#	libhardware \
#	libutils \
#	libdl \

LOCAL_MODULE:= libgralloc_extra

include $(BUILD_STATIC_LIBRARY)
