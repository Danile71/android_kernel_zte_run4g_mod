/*
**
** Copyright (C) 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "mmsdk/MMSdkService"

#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>

#include <binder/AppOpsManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>

#include <mtkcam/common.h>

using namespace android; 
//
#if '1' == MTKCAM_HAVE_SDK_CLIENT
using namespace android; 
#include "MyUtils.h"
using namespace MtkCamUtils;
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamClient.h>
#include <mtkcam/v1/sdkClient/IGestureClient.h>
using namespace android::NSSdkClient;
#endif 

// scaler client
#include <mmsdk/IImageTransformUser.h>
#include "./client/include/ImageTransformUser.h"

// Effect client
#include <mmsdk/IEffectUser.h>
#include "./client/include/EffectFaceBeautyUser.h"

// Gesture client
#include <mmsdk/IGestureUser.h>
#include "./client/include/GestureUser.h"
//
#include <mtkcam/device/Cam1Device.h>


//
#include "MMSdkService.h"

using namespace android; 
using namespace NSMMSdk; 

namespace android {


// ----------------------------------------------------------------------------
// Logging support -- this is for debugging only
// Use "adb shell dumpsys media.mmsdk -v 1" to change it.
volatile int32_t gLogLevel = 1;

#include <cutils/xlog.h>
#define MY_LOGV(fmt, arg...)       XLOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       XLOGD(fmt"\r\n", ##arg)
#define MY_LOGI(fmt, arg...)       XLOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       XLOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       XLOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)

#define LOG1(...) ALOGD_IF(gLogLevel >= 1, __VA_ARGS__);
#define LOG2(...) ALOGD_IF(gLogLevel >= 2, __VA_ARGS__);

static void setLogLevel(int level) {
    android_atomic_write(level, &gLogLevel);
}

// ----------------------------------------------------------------------------

/******************************************************************************
 *
 ******************************************************************************/
static int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

/******************************************************************************
 *
 ******************************************************************************/
static int getCallingUid() {
    return IPCThreadState::self()->getCallingUid();
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkService::MMSdkService()    
    :mpCam1Device(0)
#if '1' == MTKCAM_HAVE_SDK_CLIENT       
    ,mpGestureClient(0)
#endif    
{
    ALOGI("MMSdkService started (pid=%d)", getpid());
}

/******************************************************************************
 *
 ******************************************************************************/
void MMSdkService::onFirstRef()
{
    LOG1("MMSdkService::onFirstRef");

    BnMMSdkService::onFirstRef();
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkService::~MMSdkService() 
{
    LOG1("MMSdkService::~MMSdkService()"); 
#if '1' == MTKCAM_HAVE_SDK_CLIENT      
    mpGestureClient = NULL; 
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectImageTransformUser (
    sp<NSImageTransform::IImageTransformUser>& client
) 
{
    
    LOG1("MMSdkService::connectSImager E"); 
    using namespace android::NSMMSdk::NSImageTransform; 
    sp <ImageTransformUser> user = new ImageTransformUser(); 
    
    client = user; 
    LOG1("MMSdkService::connectSImager X"); 
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectEffect (
    String8 const &clientName, 
    sp<NSEffect::IEffectUser>& client
) 
{     
    
    LOG1("MMSdkService::connectEffect[%s] E", clientName.string()); 

    using namespace android::NSMMSdk::NSEffect; 
    if (clientName == "faceBeauty")
    {
        sp<NSEffect::EffectFaceBeautyUser> user = new EffectFaceBeautyUser(); 
        client = user;
        return OK;
    }
    else 
    {
        LOG1("unknown client name"); 
    }
    
    LOG1("MMSdkService::connectEffect X"); 
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectGesture (
    sp<NSGesture::IGestureUser>& client
) 
{
    
    LOG1("MMSdkService::connectGesture E"); 
    using namespace android::NSMMSdk::NSGesture; 
#if '1' == MTKCAM_HAVE_SDK_CLIENT        
    if (mpGestureClient == NULL)
    {
        MY_LOGE("no native gesture client"); 
        return UNKNOWN_ERROR; 
    }
    sp <GestureUser> user = new GestureUser(mpGestureClient); 
#else 
    sp <GestureUser> user = new GestureUser(); 
#endif 
    
    client = user; 
    LOG1("MMSdkService::connectGesture X"); 
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::
registerCamera1Device(
    NSCam::ICamDevice *device
)
{
    mpCam1Device = reinterpret_cast<NSCam::Cam1Device*>(device);
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    using namespace NSSdkClient;
    mpGestureClient = android::NSSdkClient::IGestureClient::createInstance();
    mpCam1Device->addCamClient(mpGestureClient, "GestureClient");
#endif 
    LOG1("MMSdkService::registerCamera1Device:%p", device);
    
    return OK; 
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::
unRegisterCamera1Device(
    NSCam::ICamDevice *device
)
{
    if (mpCam1Device == device)
    {
        LOG1("MMSdkService::unregisterCamera1Device:%p", device); 
        mpCam1Device = 0; 
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
#if 0 
status_t MMSdkService::connectSample(
) 
{
    LOG1("MMSdkService::connectSample E"); 

    //LOG1("MMSdkService::connectSImager E (pid %d \"%s\")", callingPid,
    //        clientName8.string());
    return OK;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::onTransact(
    uint32_t code, 
    const Parcel& data, 
    Parcel* reply, 
    uint32_t flags) 
{
    // Permission checks
    switch (code) 
    {
        case BnMMSdkService::CONNECT_IMAGETRANSFORMUSER:
            break; 
        case BnMMSdkService::CONNECT_EFFECT: 
            break; 
        case BnMMSdkService::CONNECT_GESTURE:
            break; 
    }

    return BnMMSdkService::onTransact(code, data, reply, flags);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::dump(
    int fd, 
    const Vector<String16>& args
) 
{
    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
/*virtual*/void MMSdkService::binderDied
(
    const wp<IBinder> &who
) 
{
    /**
      * While tempting to promote the wp<IBinder> into a sp,
      * it's actually not supported by the binder driver
      */
    ALOGD("java clients' binder died");
}


}; // namespace android
