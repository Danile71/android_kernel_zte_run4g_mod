/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>
#define  LOG_TAG    "SFT"
#include <android/log.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <cutils/memory.h>
#include <cutils/xlog.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
//#include <binder/IServiceManager.h>

//#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
//#include <cutils/properties.h>

#include <ui/DisplayInfo.h>
#include <utils/KeyedVector.h>

#include "jni.h"
//#include "JNIHelp.h"
//#include "JniInvocation.h"
//#include <android_runtime/AndroidRuntime.h>
//#include <android_runtime/android_view_Surface.h>
//#include <android_runtime/android_graphics_SurfaceTexture.h>
#include <android/native_window_jni.h>

#include "utProc.h"

using namespace android;


#define LOGV(x, ...) XLOGV("[SFT_jni] "x, ##__VA_ARGS__)
#define LOGD(x, ...) XLOGD("[SFT_jni] "x, ##__VA_ARGS__)
#define LOGI(x, ...) XLOGI("[SFT_jni] "x, ##__VA_ARGS__)
#define LOGW(x, ...) XLOGW("[SFT_jni] "x, ##__VA_ARGS__)
#define LOGE(x, ...) XLOGE("[SFT_jni] "x, ##__VA_ARGS__)

sp<SfUtProc> utProc = NULL;

extern "C" {
    JNIEXPORT int JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_connect(JNIEnv *_env, jobject _this,
        jobject native_window, jboolean useDefaultSize, jint w, jint h, jboolean updateScreen, jint api, jint format, jint colorIndex);
    JNIEXPORT void JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_disconnect(JNIEnv *_env, jobject _this, jint id);
    JNIEXPORT void JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_setGlobalConfig(JNIEnv *_env, jobject _this,
        jlong color);
    JNIEXPORT float JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_getFps(JNIEnv *_env, jobject _this);
    JNIEXPORT void JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_destroy(JNIEnv *_env, jobject _this);
};

JNIEXPORT int JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_connect(JNIEnv *_env, jobject _this,
    jobject native_window, jboolean useDefaultSize, jint w, jint h, jboolean updateScreen, jint api, jint format, jint colorIndex) {
    if (utProc == NULL)
        utProc = new SfUtProc();

    sp<ANativeWindow> window;
    utProc->mId++;
    LOGD("connect, native_window=%p, useDefaultSize=%d, w=%d, h=%d, updateScreen=%d, api=%d, format=%x, colorIndex=%d",
        native_window, useDefaultSize, w, h, updateScreen, api, format, colorIndex);

    if (native_window == NULL) {
        LOGW("native_window is null");
//not_valid_surface:
//        jniThrowException(_env, "java/lang/IllegalArgumentException",
//                "[SFT_jni] Make sure the SurfaceView or associated SurfaceHolder has a valid Surface");
        return -1;
    }

//    window = android_view_Surface_getNativeWindow(_env, native_window);
    window = ANativeWindow_fromSurface(_env, native_window);
    if (window == NULL) {
        LOGW("getNative window is null");
//        goto not_valid_surface;
        return -1;
    }

    utProc->connect(utProc->mId, window, useDefaultSize, w, h, updateScreen, api, format, colorIndex);

    return utProc->mId;
}

JNIEXPORT void JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_disconnect(JNIEnv *_env, jobject _this, jint id) {
    LOGD("disconnect");
    utProc->disconnect(id);
    LOGI("disconnect, id=%d, utProc=%p", id, utProc.get());
}

JNIEXPORT void JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_setGlobalConfig(JNIEnv *_env, jobject _this,
    jlong color) {
    if (utProc == NULL)
        utProc = new SfUtProc();

    LOGI("setGlobalConfig, color=%x", color);
    utProc->setGlobalConfig(color);
}

JNIEXPORT float JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_getFps(JNIEnv *_env, jobject _this) {
    if (utProc != NULL)
        return utProc->getFps();
    else return 0.0f;
}

JNIEXPORT void JNICALL Java_com_mediatek_ut_SurfaceFlingerTest_destroy(JNIEnv *_env, jobject _this) {
    LOGI("destroy");
    utProc = NULL;
}
