# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

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


# Copyright 2005 The Android Open Source Project

###############################################################################
# Configuration
###############################################################################
#MTK_GPS_CHIP =  MTK_GPS_MT6628 MTK_GPS_MT6572 MTK_GPS_MT6582 MTK_GPS_MT3332 MTK_GPS_MT6620 MTK_GPS_MT6592
###############################################################################
# build start
###############################################################################
ifeq ($(MTK_GPS_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)
MY_LOCAL_PATH := $(LOCAL_PATH)
include $(MY_LOCAL_PATH)/libs/Android.mk
#define my_libmnlp
$(warning feature_option=$(MTK_GPS_CHIP))
##################6628#####################
ifeq ($(findstring MTK_GPS_MT6628,$(MTK_GPS_CHIP)),MTK_GPS_MT6628)
include $(MY_LOCAL_PATH)/mnl6628/lib/Android.mk          
MNL6628_SRC_FILES := \
          mnl6628/mnl_process_6620.c \
          mnl6628/mtk_gps_6620.c \
          mnl6628/mnl_common_6620.c
include $(CLEAR_VARS)
LOCAL_PATH := $(MY_LOCAL_PATH)
LOCAL_SRC_FILES := \
         $(MNL6628_SRC_FILES) 
LOCAL_MODULE := libmnlp_mt6628
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/mnl6628/inc \
				   $(LOCAL_PATH)/../inc \
                   $(MTK_PATH_SOURCE)/external/nvram/libnvram \
                   $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
                   $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
                   $(MTK_PATH_CUSTOM)/cgen/inc  
LOCAL_SHARED_LIBRARIES += libcutils libc libm libnvram
LOCAL_STATIC_LIBRARIES +=  libsupl libagent_6628 libhotstill libmnl_6628
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endif
##################6620##########################
ifeq ($(findstring MTK_GPS_MT6620,$(MTK_GPS_CHIP)),MTK_GPS_MT6620)
include $(MY_LOCAL_PATH)/mnl6620/lib/Android.mk          
MNL6620_SRC_FILES := \
          mnl6620/mnl_process_6620.c \
          mnl6620/mtk_gps_6620.c \
          mnl6620/mnl_common_6620.c
include $(CLEAR_VARS)
LOCAL_PATH := $(MY_LOCAL_PATH)
LOCAL_SRC_FILES := \
         $(MNL6620_SRC_FILES) 
LOCAL_MODULE := libmnlp_mt6620
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/mnl6620/inc \
				   $(LOCAL_PATH)/../inc \
                   $(MTK_PATH_SOURCE)/external/nvram/libnvram \
                   $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
                   $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
                   $(MTK_PATH_CUSTOM)/cgen/inc  
LOCAL_SHARED_LIBRARIES += libcutils libc libm libnvram
LOCAL_STATIC_LIBRARIES += libsupl libagent_6620 libhotstill libmnl_6620
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endif
endif
