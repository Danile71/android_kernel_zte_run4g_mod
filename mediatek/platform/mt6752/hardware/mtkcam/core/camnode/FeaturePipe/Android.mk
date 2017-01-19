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
LOCAL_SRC_FILES += \
    FeaturePipe.Nodes.cpp \
    FeaturePipe.cpp \
    FeaturePipe.vFB.cpp \
    FeaturePipe.EIS.cpp \
    FeaturePipe.BufHandler.cpp \
    FeaturePipe.DataHandler.cpp \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(call include-path-for, camera)
#
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/iopipe/PostProc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include/mtkcam/v1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/camnode
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/iopipe/PostProc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/algorithm/libwarp
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/algorithm/libcore
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/algorithm/libutility
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc


#
# because IPipe.h has <vector>
LOCAL_C_INCLUDES += $(TOP)/bionic
LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#

#-----------------------------------------------------------
LOCAL_CFLAGS += 

ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#HALPOSTPROC_USING_TEST := '1'
#LOCAL_CFLAGS += -DHALPOSTPROC_USING_TEST
#-----------------------------------------------------------

#-----------------------------------------------------------
#LOCAL_WHOLE_STATIC_LIBRARIES += libcam.temp.camera1.hwscenario
#
LOCAL_STATIC_LIBRARIES += 

#-----------------------------------------------------------
LOCAL_MODULE := libcam.camnode.featurepipe

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

