ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

#libft
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES := src/PortHandle.cpp\
                   src/SerPort.cpp\
                   src/Device.cpp

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/inc \
									 mediatek/external/dfo/featured \
									 $(TARGET_OUT_HEADERS)/dfo	\			
LOCAL_MODULE:= libft
include $(BUILD_STATIC_LIBRARY)



#meta_tst
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
CORE_SRC_FILES := src/tst_main.cpp

LOCAL_SRC_FILES := \
	$(CORE_SRC_FILES)\
	src/CmdTarget.cpp\
	src/Context.cpp\
	src/Device.cpp\
	src/Frame.cpp\
	src/FtModule.cpp\
	src/MdRxWatcher.cpp\
	src/Modem.cpp\
	src/SerPort.cpp\
	src/UsbRxWatcher.cpp\
	src/PortHandle.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc 

MTK_META_AUDIO_SUPPORT := yes
MTK_META_CCAP_SUPPORT := yes
MTK_META_GSENSOR_SUPPORT := yes
MTK_META_MSENSOR_SUPPORT := yes
MTK_META_ALSPS_SUPPORT := yes
MTK_META_GYROSCOPE_SUPPORT := yes
MTK_META_DFO_SUPPORT := yes
MTK_META_TOUCH_SUPPORT := yes
MTK_META_LCDBK_SUPPORT := yes
MTK_META_KEYPADBK_SUPPORT := yes
MTK_META_LCD_SUPPORT := yes
MTK_META_VIBRATOR_SUPPORT := yes
MTK_META_CPU_SUPPORT := yes
MTK_META_SDCARD_SUPPORT := yes
MTK_META_ADC_SUPPORT := yes
MTK_META_NVRAM_SUPPORT := yes
MTK_META_GPIO_SUPPORT := yes

#CCCI interface
LOCAL_C_INCLUDES += $(TOPDIR)/hardware/libhardware_legacy/include\
	            $(TOPDIR)/hardware/libhardware/include

#DFO
LOCAL_C_INCLUDES += mediatek/external/dfo/featured \
	                  $(TARGET_OUT_HEADERS)/dfo
ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif
	
LOCAL_SHARED_LIBRARIES += libdl libhwm libhardware_legacy libmedia libcutils libacdk liblog libcutils



# DriverInterface Begin

ifeq ($(MTK_META_DFO_SUPPORT),yes)
LOCAL_C_INCLUDES +=$(PLATFORM_PATH)/dfo \
                    $(MTK_PATH_SOURCE)/external/dfo/boot
LOCAL_STATIC_LIBRARIES +=  libmeta_dfo
LOCAL_CFLAGS += \
    -DFT_DFO_FEATURE
endif

ifeq ($(MTK_WLAN_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/wifi
LOCAL_STATIC_LIBRARIES += libmeta_wifi
LOCAL_SHARED_LIBRARIES += libnetutils
LOCAL_CFLAGS += \
    -DFT_WIFI_FEATURE
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/gps
LOCAL_STATIC_LIBRARIES += libmeta_gps 
LOCAL_CFLAGS += \
    -DFT_GPS_FEATURE
endif

ifeq ($(MTK_NFC_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/nfc
LOCAL_SHARED_LIBRARIES += libmtknfc_dynamic_load_jni
LOCAL_STATIC_LIBRARIES += libmeta_nfc
LOCAL_CFLAGS += \
    -DFT_NFC_FEATURE
endif

ifeq ($(MTK_BT_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/bluetooth
LOCAL_STATIC_LIBRARIES += libmeta_bluetooth 
LOCAL_CFLAGS += \
    -DFT_BT_FEATURE
endif  

ifeq ($(MTK_FM_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/fm
LOCAL_STATIC_LIBRARIES += libmeta_fm   
LOCAL_CFLAGS += \
    -DFT_FM_FEATURE
endif  

ifeq ($(MTK_META_AUDIO_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/Audio 
LOCAL_SHARED_LIBRARIES += libaudio.primary.default libutils
LOCAL_STATIC_LIBRARIES += libmeta_audio
LOCAL_CFLAGS += \
    -DFT_AUDIO_FEATURE
endif

ifeq ($(MTK_META_CCAP_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/cameratool/CCAP\
                    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/acdk/inc/cct\
                    $(TOP)/$(MTK_PATH_SOURCE)/hardware/include\
                    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include\
                    $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
                    #$(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/acdk\
                    #$(TOP)/$(MTK_PATH_PLATFORM)/hardware/include\
                    #$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc  
LOCAL_STATIC_LIBRARIES += libccap
LOCAL_CFLAGS += \
    -DFT_CCAP_FEATURE
endif

ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/matv
LOCAL_SHARED_LIBRARIES += libmatv_cust
LOCAL_STATIC_LIBRARIES += libmeta_matv
LOCAL_CFLAGS += \
    -DFT_MATV_FEATURE
endif 

ifeq ($(MTK_META_GSENSOR_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/gsensor 
LOCAL_STATIC_LIBRARIES += libmeta_gsensor 
LOCAL_CFLAGS += \
    -DFT_GSENSOR_FEATURE  
endif 

ifeq ($(MTK_META_MSENSOR_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/msensor 
LOCAL_STATIC_LIBRARIES += libmeta_msensor
LOCAL_CFLAGS += \
    -DFT_MSENSOR_FEATURE  
endif 

ifeq ($(MTK_META_ALSPS_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/alsps 
LOCAL_STATIC_LIBRARIES += libmeta_alsps
LOCAL_CFLAGS += \
    -DFT_ALSPS_FEATURE     
endif 

ifeq ($(MTK_META_GYROSCOPE_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/gyroscope 
LOCAL_STATIC_LIBRARIES += libmeta_gyroscope
LOCAL_CFLAGS += \
    -DFT_GYROSCOPE_FEATURE   
endif 

ifeq ($(MTK_META_TOUCH_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/touch 
LOCAL_STATIC_LIBRARIES += libmeta_touch 
LOCAL_CFLAGS += \
    -DFT_TOUCH_FEATURE  
endif 

ifeq ($(MTK_META_LCDBK_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/LCDBK 
LOCAL_STATIC_LIBRARIES += libmeta_lcdbk 
LOCAL_CFLAGS += \
    -DFT_LCDBK_FEATURE  
endif 

ifeq ($(MTK_META_KEYPADBK_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/keypadbk 
LOCAL_STATIC_LIBRARIES += libmeta_keypadbk 
LOCAL_CFLAGS += \
    -DFT_KEYPADBK_FEATURE  
endif 

ifeq ($(MTK_META_LCD_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/lcd 
LOCAL_STATIC_LIBRARIES += libmeta_lcd 
LOCAL_CFLAGS += \
    -DFT_LCD_FEATURE  
endif 

ifeq ($(MTK_META_VIBRATOR_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/vibrator 
LOCAL_STATIC_LIBRARIES += libmeta_vibrator 
LOCAL_CFLAGS += \
    -DFT_VIBRATOR_FEATURE  
endif 

ifeq ($(MTK_META_CPU_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/cpu 
LOCAL_STATIC_LIBRARIES += libmeta_cpu 
LOCAL_CFLAGS += \
    -DFT_CPU_FEATURE  
endif 

ifeq ($(MTK_META_SDCARD_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/sdcard 
LOCAL_STATIC_LIBRARIES += libmeta_sdcard 
LOCAL_CFLAGS += \
    -DFT_SDCARD_FEATURE  
endif 

ifeq ($(MTK_EMMC_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/emmc\
										$(PLATFORM_PATH)/cryptfs
LOCAL_STATIC_LIBRARIES += libmeta_clr_emmc \
                          libext4_utils_static \
                          libz \
                          libmeta_cryptfs\
                          libmtdutil
LOCAL_CFLAGS += \
    -DFT_EMMC_FEATURE
LOCAL_CFLAGS += \
    -DFT_CRYPTFS_FEATURE
endif 

ifeq ($(MTK_META_ADC_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/ADC 
LOCAL_STATIC_LIBRARIES += libmeta_adc_old 
LOCAL_CFLAGS += \
    -DFT_ADC_FEATURE  
endif 

ifeq ($(MTK_DX_HDCP_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/hdcp 
LOCAL_SHARED_LIBRARIES += libDxHdcp
LOCAL_STATIC_LIBRARIES += libmeta_hdcp 
LOCAL_CFLAGS += \
    -DFT_HDCP_FEATURE  
endif 


ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifeq ($(MTK_DRM_KEY_MNG_SUPPORT),yes)
LOCAL_CFLAGS += -DFT_DRM_KEY_MNG_FEATURE
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/drmkey
LOCAL_STATIC_LIBRARIES += liburee_meta_drmkeyinstall 
LOCAL_SHARED_LIBRARIES += libtz_uree
endif
endif


ifeq ($(strip $(MTK_META_NVRAM_SUPPORT)),yes)
LOCAL_STATIC_LIBRARIES += libfft
LOCAL_SHARED_LIBRARIES += libnvram 
LOCAL_SHARED_LIBRARIES += libfile_op
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libfile_op
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/Meta_APEditor
LOCAL_STATIC_LIBRARIES += libmeta_apeditor
LOCAL_CFLAGS += \
    -DFT_NVRAM_FEATURE 
endif 

ifeq ($(strip $(MTK_META_GPIO_SUPPORT)),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/gpio 
LOCAL_STATIC_LIBRARIES += libmeta_gpio 
LOCAL_CFLAGS += \
    -DFT_GPIO_FEATURE  
endif 


ifeq ($(GEMINI),yes)
LOCAL_CFLAGS += \
    -DGEMINI
endif    

ifeq ($(MTK_GEMINI_3SIM_SUPPORT),yes)
LOCAL_CFLAGS += \
    -DMTK_GEMINI_3SIM_SUPPORT
endif    

ifeq ($(MTK_GEMINI_4SIM_SUPPORT),yes)
LOCAL_CFLAGS += \
    -DMTK_GEMINI_4SIM_SUPPORT
endif

ifeq ($(MTK_SPEAKER_MONITOR_SUPPORT),yes)
LOCAL_CFLAGS += \
    -DMTK_SPEAKER_MONITOR_SUPPORT
endif  

ifeq ($(MODEM_1_DHL_SUPPORT),TRUE)
LOCAL_CFLAGS +=-DMTK_MD1_DHL_SUPPORT
endif

ifeq ($(MODEM_2_DHL_SUPPORT),TRUE)
LOCAL_CFLAGS +=-DMTK_MD2_DHL_SUPPORT
endif

ifeq ($(MODEM_5_DHL_SUPPORT),TRUE)
LOCAL_CFLAGS +=-DMTK_MD5_DHL_SUPPORT
endif

ifeq ($(MTK_DT_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif

ifeq ($(MTK_EXTMD_NATIVE_DOWNLOAD_SUPPORT),yes)
LOCAL_CFLAGS += -MTK_EXTMD_NATIVE_DOWNLOAD_SUPPORT
endif

ifeq ($(EVDO_DT_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_EVDO_DT_SUPPORT
endif

ifneq ($(MTK_EXTERNAL_MODEM_SLOT),0)
LOCAL_CFLAGS += -DMTK_EXTERNAL_MODEM
endif

# DriverInterface End

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_MODULE:=meta_tst

LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)

include $(BUILD_EXECUTABLE)

endif   # TARGET_ARCH == arm
endif	# !TARGET_SIMULATOR


