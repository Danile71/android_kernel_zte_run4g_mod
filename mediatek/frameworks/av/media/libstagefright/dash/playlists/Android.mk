LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := src/playlist/src/MPDPeriod.cpp   \
src/playlist/src/MPDBaseUrl.cpp   \
src/playlist/src/MPDPlaylist.cpp   \
src/playlist/src/MPDPlaylists.cpp   \
src/playlist/src/MPDAdaptationset.cpp   \
src/playlist/src/MPDContentProtection.cpp   \
src/playlist/src/MPDRepresentation.cpp   \
src/playlist/src/MPDSegment.cpp   \
src/playlist/src/MPDSubrepresentation.cpp   \
src/parser/AltovaException.cpp   \
src/parser/CMPDSchema.cpp   \
src/parser/MPDParser.cpp   \
src/parser/MPDSchema-typeinfo.cpp   \
src/parser/MPDSchema.cpp   \
src/parser/MPDSchema_ex.cpp   \
src/parser/Node.cpp   \
src/parser/SchemaTypes.cpp   \
src/parser/StdAfx.cpp   \
src/parser/StructInfo.cpp   \
src/parser/XmlException.cpp   \
src/parser/dash_static.cpp   \
src/parser/xs-datetime.cpp   \
src/parser/xs-types.cpp



LOCAL_CFLAGS := -g -D__STDC_LIMIT_MACROS -O0 -fexceptions -fpermissive -frtti
 
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
$(LOCAL_PATH)/src/playlist/include \
$(LOCAL_PATH)/src/parser \
bionic \
external/stlport/stlport \
$(LOCAL_PATH)/../../../../../../external/xerces/src \
$(LOCAL_PATH)/../../../../../../external/uriparser/uriparser_external

 
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/../../../../../../../prebuilts/ndk/current/sources/cxx-stl/gnu-libstdc++/libs/armeabi \
-lgnustl_static -lsupc++
 
LOCAL_SHARED_LIBRARIES := libxerces liburiparser libdl libstlport libcutils libutils liblog
LOCAL_MODULE:= libmpdplaylists

include $(BUILD_SHARED_LIBRARY)
