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

################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
MY_CLIENT_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/include

#-----------------------------------------------------------
#
# Preview Feature Clients
MTKCAM_HAVE_PREVIEWFEATURE_CLIENT ?= '1'
#
# Preview Callback Client
MTKCAM_HAVE_PREVIEWCALLBACK_CLIENT ?= '1'
#
# Display Client
MTKCAM_HAVE_DISPLAY_CLIENT ?= '1'
#
# Record Client
MTKCAM_HAVE_RECORD_CLIENT ?= '1'
#
# FD Client
MTKCAM_HAVE_FD_CLIENT ?= '1'
#
# OT CLient
ifeq ($(MTK_CAM_OT_SUPPORT),yes)
MTKCAM_HAVE_OT_CLIENT ?= '1'
else
MTKCAM_HAVE_OT_CLIENT ?= '0'
endif
#

#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES := 
#
# Generic
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.generic
#
# Preview Features Clients
ifeq "'1'" "$(strip $(MTKCAM_HAVE_PREVIEWFEATURE_CLIENT))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.previewfeature
endif
#
# Preview Callback Client
ifeq "'1'" "$(strip $(MTKCAM_HAVE_PREVIEWCALLBACK_CLIENT))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.previewcallback
endif
#
# Display Client
ifeq "'1'" "$(strip $(MTKCAM_HAVE_DISPLAY_CLIENT))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.displayclient
endif
#
# Record Client
ifeq "'1'" "$(strip $(MTKCAM_HAVE_RECORD_CLIENT))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.record.common
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.record.platform
endif
#
# FD Client
ifeq "'1'" "$(strip $(MTKCAM_HAVE_FD_CLIENT))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.fd
endif
#
# OT Client
ifeq "'1'" "$(strip $(MTKCAM_HAVE_OT_CLIENT))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.client.camclient.ot
endif
#
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libhardware
#
LOCAL_SHARED_LIBRARIES += libcamera_client libcamera_client_mtk
#
LOCAL_SHARED_LIBRARIES += libcam_mmp
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam.utils
LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
#
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libfeatureio
#
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libJpgEncPipe
LOCAL_SHARED_LIBRARIES += libmpo
LOCAL_SHARED_LIBRARIES += libmpoencoder
LOCAL_SHARED_LIBRARIES += libcam.exif
LOCAL_SHARED_LIBRARIES += libcameracustom
#
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.client

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

