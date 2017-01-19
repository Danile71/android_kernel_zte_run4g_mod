ifeq ($(strip $(MTK_MPEG_DASH_SUPPORT)), yes)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=                 \
        DashDataProcessor.cpp   \
        DashFmp4Parser.cpp	\
        DashDrmHandler.cpp  \
        DashSession.cpp  \
        DashFileFetcher.cpp

LOCAL_C_INCLUDES:= \
	$(TOP)/frameworks/av/media/libstagefright \
	$(TOP)/frameworks/av/media/libstagefright/mpeg2ts  \
	$(TOP)/frameworks/av/media/libstagefright/include \
	$(TOP)/frameworks/av/include \
	$(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/libstagefright/dash \
    $(TOP)/external/openssl/include \
	$(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/libstagefright/dash/playlists/include \
	$(TOP)/bionic  \
	$(TOP)/external/stlport/stlport

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        liblog                          \
        libstagefright_foundation       \
        libstagefright                  \
        libutils                        \
        libdl                           \
        libdrmframework					\
		libstlport                      \
        libmpdplaylists

LOCAL_MODULE:= libstagefright_dash


include $(BUILD_SHARED_LIBRARY)

#ifeq ($(strip $(DASHTEST)),yes)

################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        processTest.cpp

LOCAL_C_INCLUDES:= \
	$(TOP)/frameworks/av/media/libstagefright \
	$(TOP)/frameworks/av/media/libstagefright/mpeg2ts  \
	$(TOP)/frameworks/av/media/libstagefright/include \
	$(TOP)/frameworks/av/include \
	$(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/libstagefright/dash \
	$(TOP)/external/openssl/include \
	$(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/libstagefright/dash/playlists/include \
	$(TOP)/bionic  \
	$(TOP)/external/stlport/stlport

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        liblog                          \
        libstagefright_foundation       \
        libstagefright                  \
        libutils                        \
        libdl                           \
        libdrmframework					\
		libstlport                      \
        libmpdplaylists					\
        libstagefright_dash
        

LOCAL_MODULE:= pTest

LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)


##########################################################
#endif

## build libmpdplaylists
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
