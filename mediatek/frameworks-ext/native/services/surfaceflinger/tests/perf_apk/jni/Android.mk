LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libsurfaceut
LOCAL_CFLAGS    := -Werror -DHAVE_PTHREADS

LOCAL_SRC_FILES := \
	surface.cpp \
	utProc.cpp \
	FpsCounter.cpp

LOCAL_LDLIBS    := -llog -lcutils -lutils -lbinder -lui -lgui -landroid

#LOCAL_SHARED_LIBRARIES := \
#	libcutils \
#	libutils \
#	libbinder \
#	libui \
#	libgui

include $(BUILD_SHARED_LIBRARY)
