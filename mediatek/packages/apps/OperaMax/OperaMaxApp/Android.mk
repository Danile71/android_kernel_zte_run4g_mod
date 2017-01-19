###############################################################################
# 
# ANT Stack
# 
# Copyright 2010 Dynastream Innovations
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and  
# limitations under the License.
# 
###############################################################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := OuPengApp
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := liuliangbao-3.8.16.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/operator/app
include $(BUILD_PREBUILT)


###############################################################################

#LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := OperaMaxSystem
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := tunman
LOCAL_MODULE_CLASS := EXECUTABLES
#LOCAL_MODULE_SUFFIX := $(TARGET_EXECUTABLE_SUFFIX)
#LOCAL_CERTIFICATE := PRESIGNED
#LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
include $(BUILD_PREBUILT)

################################################################################

#LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := OperaMaxApp
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := OperaMax-0.9.670.8.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/operator/app
include $(BUILD_PREBUILT)
