# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# #############################################################################
# NOTE ON BUILDING JNI LIBRARY FOR ANDROID VERSIONS 4.2 AND 4.3/4.4						
#
# IN JB43, the pread api is build with different mangled name than 4.2					
#
# therefore to build the jni lib for 4.2.2 you need to use 4.2.2 android source code
# 1. export an environment variable 'TREE_PATH' with absolute path to the top of the 4.2.2 android tree
# 2. cd <android-branch>/DrmAssist-Recommended/jni
# 3. mm
# 4. the libDrmAssistJNI would be created in <android-branch>/DrmAssist-Recommended/jni/libs/armeabi/libDrmAssistJNI.so
#										
#										
# To build the jni lib for 4.3/4.4 :
# 1. copy the entire DrmAssist-Recommended to <android-tree>/vendor/Discretix/DrmAssist-Recommended
# 2. cd to <android-tree>/vendor/Discretix/DrmAssist-Recommended/jni
# 3. mm
# 4. the libDrmAssistJNI would be created in <android-tree>/out/target/product/generic/system/lib/
#													
# #############################################################################

LOCAL_PATH := $(call my-dir)

# #############################################################################
# if TREE_PATH was not defined, then assuming we are compiling under the android tree directly
# DrmAssist path should be: <android-tree>/vendor/Discretix/DrmAssist-Recommended
# Recommended plugin path should be: <android-tree>/frameworks/av/drm/libdrmframework/plugins/playready_Recommended/
$(warning "TREE_PATH=$(TREE_PATH)")
ifeq ($(strip $(TREE_PATH)),)
$(warning "in if")
#TREE_PATH := $(LOCAL_PATH)/../../../..
TREE_PATH := $(TOP)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    frameworks/include/drm \
	  frameworks/av/include/drm/ \
	  system/core/include/android/ \
	  mediatek/frameworks/opt/playready/include
	
$(info LOCAL_C_INCLUDES=$(LOCAL_C_INCLUDES))
LOCAL_SHARED_LIBRARIES := \
        libutils          \
        libcutils         \
        libbinder         \
        libdrmframework   \
	      liblog

# #############################################################################
else
$(warning "in else")
# TREE_PATH was defined. I.e. we compile under the android branch with a reference to the tree.
# #############################################################################

# this is an example for the android tree path 
# TREE_PATH := /scratch/ohadn/dev/android_factory/JB422_mtk

# this is the android branch top
#BRANCH_TOP := $(LOCAL_PATH)/../..
BRANCH_TOP := $(TOP)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    frameworks/native/include \
    frameworks/av/drm/ \
    frameworks/av/include/drm/ \
    system/core/include \
    $(BRANCH_TOP)/dxdrmframework/playready_Recommended/include/ \
    mediatek/frameworks/opt/playready/include

endif
# #############################################################################

#LIB_PATH := out/target/product/generic/system/lib/
TREE_VER = $(call substr,$(PLATFORM_VERSION),1,3)
$(warning "TREE_VER = $(TREE_VER)")
ifeq ($(strip $(TREE_VER)), 4.4)
  $(warning "4.4 platform")
  TREE_VER=44
else ifeq ($(strip $(TREE_VER)), 4.3)
  $(warning "4.3 platform")
  TREE_VER=43
else ifeq ($(strip $(TREE_VER)), 4.2)
  $(warning "4.2 platform")
  TREE_VER=42
else
  $(warning "other platform set TREE_VER to 42")
  TREE_VER=42
endif

$(warning "PLATFORM_VERSION = $(PLATFORM_VERSION)")
LOCAL_MODULE    := libDrmAssistJNI$(TREE_VER)
LOCAL_SRC_FILES := DecryptSession.cpp

LOCAL_MODULE_TAGS:= debug eng
LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_LDLIBS := -L$(LIB_PATH) -llog $(LIB_PATH)/libcutils.so  $(LIB_PATH)/libutils.so $(LIB_PATH)/libbinder.so $(LIB_PATH)/libdrmframework.so 
include $(BUILD_SHARED_LIBRARY)
