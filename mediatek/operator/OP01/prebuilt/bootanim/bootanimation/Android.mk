# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


# barrier for searching Android.mk
# Android.mk in out will be enumrated in mediatek/build/android



#ifneq (,$(filter bootanim, $(OPTR_APK)))
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Specify LTE path 
ifeq ($(strip $(MTK_LTE_SUPPORT)), yes)
    LOCAL_PATH := $(LOCAL_PATH)/LTE
endif   

# your prebuilt file name
ifeq ($(LCM_HEIGHT), 800)
    LOCAL_PATH := $(LOCAL_PATH)/WVGA
else
    ifeq ($(LCM_HEIGHT), 480)
    	LOCAL_PATH := $(LOCAL_PATH)/HVGA
    endif

    ifeq ($(LCM_HEIGHT), 854)
        LOCAL_PATH := $(LOCAL_PATH)/FWVGA
    endif

    ifeq ($(LCM_HEIGHT), 960)
        LOCAL_PATH := $(LOCAL_PATH)/QHD
    endif

    ifeq ($(LCM_HEIGHT), 1280)
        LOCAL_PATH := $(LOCAL_PATH)/HD720
    endif

    ifeq ($(LCM_HEIGHT), 1920)
        LOCAL_PATH := $(LOCAL_PATH)/FHD
    endif

    ifeq ($(LCM_HEIGHT), 2560)
        LOCAL_PATH := $(LOCAL_PATH)/WQHD
    endif

endif
    	
LOCAL_MODULE := bootanimation.zip
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := media
# your prebuilt file (must be relative directory )
LOCAL_SRC_FILES := $(LOCAL_MODULE)
# the path your prebuilt file will be installed   $(TARGET_OUT) is the system directory
LOCAL_MODULE_PATH := $(TARGET_OUT)/media

# Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)), yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/media
endif

include $(BUILD_PREBUILT)

#endif

