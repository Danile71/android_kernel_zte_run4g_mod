
ifeq ($(strip $(MTK_MPEG_DASH_SUPPORT)),yes)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DPACKAGE_NAME=\"uriparser\" \
-DPACKAGE_TARNAME=\"uriparser\" \
-DPACKAGE_VERSION=\"0.7.7\" \
-DPACKAGE_STRING=\"uriparser\ 0.7.7\" \
-DPACKAGE_BUGREPORT=\"\" \
-DPACKAGE_URL=\"\" \
-DPACKAGE=\"uriparser\" \
-DVERSION=\"0.7.7\" \
-DSTDC_HEADERS=1 \
-DHAVE_SYS_TYPES_H=1 \
-DHAVE_SYS_STAT_H=1 \
-DHAVE_STDLIB_H=1 \
-DHAVE_STRING_H=1 \
-DHAVE_MEMORY_H=1 \
-DHAVE_STRINGS_H=1 \
-DHAVE_INTTYPES_H=1 \
-DHAVE_STDINT_H=1 \
-DHAVE_UNISTD_H=1 \
-DHAVE_DLFCN_H=1 \
-DLT_OBJDIR=\".libs/\" \
-DSTDC_HEADERS=1 \
-DHAVE_WPRINTF=/\*\*/ \
-O0 -g 


LOCAL_SRC_FILES := src/UriCommon.c \
src/UriCompare.c \
src/UriEscape.c \
src/UriFile.c \
src/UriIp4.c \
src/UriIp4Base.c \
src/UriNormalize.c \
src/UriNormalizeBase.c \
src/UriParse.c \
src/UriParseBase.c \
src/UriQuery.c \
src/UriRecompose.c \
src/UriResolve.c \
src/UriShorten.c


LOCAL_SRC_FILES += uriparser_external/UriGlue.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
$(LOCAL_PATH)/uriparser_external

LOCAL_MODULE:= liburiparser

include $(BUILD_SHARED_LIBRARY)

endif

