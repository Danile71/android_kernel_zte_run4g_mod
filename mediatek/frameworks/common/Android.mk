# limitations under the License.
#

# This makefile shows how to build your own shared library that can be
# shipped on the system of a phone, and included additional examples of
# including JNI code with the library and writing client applications against it.

LOCAL_PATH := $(call my-dir)

# MediaTek common library.
# ============================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := mediatek-common

LOCAL_MODULE_CLASS := JAVA_LIBRARIES

LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_AIDL_INCLUDES += $(FRAMEWORKS_BASE_JAVA_SRC_DIRS)

LOCAL_SRC_FILES += src/com/mediatek/common/audioprofile/IAudioProfileService.aidl \
                   src/com/mediatek/common/audioprofile/IAudioProfileListener.aidl \
                   src/com/mediatek/common/dm/DmAgent.aidl \
                   src/com/mediatek/common/ppl/IPplAgent.aidl \
                   src/com/mediatek/common/ppl/IPplManager.aidl \
                   src/com/mediatek/common/ppl/IPplSmsFilter.java \
                   src/com/mediatek/common/voicecommand/IVoiceCommandListener.aidl \
                   src/com/mediatek/common/voicecommand/IVoiceCommandManagerService.aidl\
                   src/com/mediatek/common/voiceextension/IVoiceExtCommandListener.aidl \
                   src/com/mediatek/common/voiceextension/IVoiceExtCommandManager.aidl\
                   src/com/mediatek/common/telephony/ITelephonyEx.aidl\
                   src/com/mediatek/common/msgmonitorservice/IMessageLogger.aidl \
                   src/com/mediatek/common/msgmonitorservice/IMessageLoggerWrapper.aidl \
                   src/com/mediatek/common/search/ISearchEngineManagerService.aidl \
                   src/com/mediatek/common/perfservice/IPerfService.aidl \
                   src/com/mediatek/common/mom/IMobileManagerService.aidl \
                   src/com/mediatek/common/mom/IPermissionListener.aidl \
                   src/com/mediatek/common/mom/IRequestedPermissionCallback.aidl \
                   src/com/mediatek/common/mom/IPackageInstallCallback.aidl \
                   src/com/mediatek/common/mom/INotificationListener.aidl \
                   src/com/mediatek/common/mom/IMobileConnectionCallback.aidl \
                   src/com/mediatek/common/mom/ICallInterceptionListener.aidl \
                   src/com/mediatek/common/mom/IMessageInterceptListener.aidl \
                   src/com/mediatek/common/ims/IImsManagerService.aidl \
                   src/com/mediatek/common/gba/IGbaService.aidl \
                   src/com/mediatek/common/hdmi/IMtkHdmiManager.aidl \
                   src/com/mediatek/common/recovery/IRecoveryManagerService.aidl \
                   src/com/mediatek/common/passpoint/IPasspointManager.aidl \
                   src/com/mediatek/common/operamax/ILoaderStateListener.aidl \
                   src/com/mediatek/common/operamax/ILoaderService.aidl \
                   src/com/mediatek/sensorhub/ISensorHubService.aidl \

#LOCAL_SRC_FILES += src/com/mediatek/common/IMyModuleCallback.aidl \
                   src/com/mediatek/common/dm/DmAgent.aidl \
                   src/com/mediatek/common/audioprofile/IAudioProfileService.aidl \
                   src/com/mediatek/common/audioprofile/IAudioProfileListener.aidl \

JAVAOPTFILEPATH := $(call local-intermediates-dir)
JAVAOPTFILE := $(JAVAOPTFILEPATH)/FeatureOption.java
JAVAIMEOPTFILE := $(JAVAOPTFILEPATH)/IMEFeatureOption.java

include $(MTK_ROOT_BUILD)/tools/javaoptgen.mk
include $(MTK_ROOT_BUILD)/tools/IMEFeatureOption.mk

LOCAL_GENERATED_SOURCES += $(JAVAOPTFILE) 
LOCAL_GENERATED_SOURCES += $(JAVAIMEOPTFILE)

# Always use the latest prebuilt Android library.
LOCAL_SDK_VERSION := 19

include $(BUILD_JAVA_LIBRARY)

ifeq ($(strip $(BUILD_MTK_API_DEP)), yes)
# mediatek-common API table.
# ============================================================
LOCAL_MODULE := mediatek-common-api

LOCAL_STATIC_JAVA_LIBRARIES := 
LOCAL_MODULE_CLASS := JAVA_LIBRARIES

LOCAL_DROIDDOC_OPTIONS:= \
		-stubs $(TARGET_OUT_COMMON_INTERMEDIATES)/JAVA_LIBRARIES/mediatek-common-api_intermediates/src \
		-api $(TARGET_OUT_COMMON_INTERMEDIATES)/PACKAGING/mediatek-common-api.txt \
		-nodocs \
        -hidden

include $(BUILD_DROIDDOC)
endif
