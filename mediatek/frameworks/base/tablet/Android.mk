# limitations under the License.
#

# This makefile shows how to build your own shared library that can be
# shipped on the system of a phone, and included additional examples of
# including JNI code with the library and writing client applications against it.

ifneq ($(strip $(MTK_TABLET_PLUGIN_BUILD)),yes)

    LOCAL_PATH := $(call my-dir)
    
    # MediaTek tablet library.
    # ============================================================
    include $(CLEAR_VARS)
    
    LOCAL_MODULE := mediatek-tablet
    
    LOCAL_SRC_FILES := DummyTB.java
    
    include $(BUILD_JAVA_LIBRARY)

ifeq ($(strip $(BUILD_MTK_API_DEP)), yes)

# CustomProperties API table.
# ============================================================
LOCAL_MODULE := mediatek-tablet-api

LOCAL_JAVA_LIBRARIES += $(LOCAL_STATIC_JAVA_LIBRARIES)
LOCAL_MODULE_CLASS := JAVA_LIBRARIES

LOCAL_DROIDDOC_OPTIONS:= \
		-api $(TARGET_OUT_COMMON_INTERMEDIATES)/PACKAGING/mediatek-tablet-api.txt \
		-nodocs \
		-hidden

include $(BUILD_DROIDDOC)
endif
endif
