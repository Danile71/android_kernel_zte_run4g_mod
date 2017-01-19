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

//
#include <utils/Log.h>
#include <utils/Vector.h>
#include <binder/Parcel.h>
#include <cutils/memory.h>

//
#include "inc/log.h"
#include "inc/utils.h"
#include "inc/command.h"
#include "res/YV12_640x480.h"

#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

#include <binder/IServiceManager.h>
#include <mmsdk/IMMSdkService.h>
#include <mmsdk/IImageTransformUser.h>

#if defined(HAVE_COMMAND_client_scaler)
// 
using namespace android;
using namespace NSMMSdk; 
//
//
//
/******************************************************************************
 *  Command
 *      scaler <connect>  <-h>
 *
 *      -h:                     help
 *      <connect>   test connect
 *
 ******************************************************************************/
namespace NSCmd_client_scaler {
struct CmdImp : public CmdBase
{
    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

protected:  ////                Implementation.
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd);

    virtual bool                onConnectClient(); 
protected:  ////                Implementation.
    virtual sp<IMMSdkService>     getMMSdkService(); 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Data Members (Parameters)

};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_client_scaler, new CmdImp(HAVE_COMMAND_client_scaler));
};  // NSCmd_test_preview
using namespace NSCmd_client_scaler;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    return  onParseActionCommand(rvCmd);
}




/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd)
{
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            String8 text;
            text += "\n";
            text += "\n   scaler <connect> <-h>";
            text += "\n   <-h>:                   help";
            text += "\n   <connect>   connect test";
            text += "\n";
            MY_LOGD("%s", text.string());
            return  true;
        }
        //
        if  ( s8Cmd == "connect" ) {
            //return  setProperty(pArgument->ms8Key, pArgument->ms8Value);
            return onConnectClient(); 
        }
        //
    }
    return  false;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onConnectClient()
{
    // 
    MMSDK_LOGD("onConnectClientTest"); 
    sp<IMMSdkService> cs = getMMSdkService();
    using namespace NSImageTransform; 
    
    sp <NSImageTransform::IImageTransformUser> scaler; 
    cs->connectImageTransformUser(scaler); 
    // 
    if (scaler == 0)
    {
        MMSDK_LOGE("connect fail"); 
        return false; 
    }

    bool ret = false; 
    //
    //
    MMSDK_LOGD("applyTransform"); 
    int yv12Stride[3] = {640, 320, 320}; 
    int yuy2Stride[3] = {640*2, 0, 0}; 
    sp<MemoryHeapBase> pSrcHeap;    
    sp<MemoryHeapBase> pDestHeap;    
    sp<MemoryBase> pSrcBuffer; 
    sp<MemoryBase> pDestBuffer; 
    
    pSrcHeap =  new MemoryHeapBase(640*480*3/2);
    pDestHeap = new MemoryHeapBase(640*480*2); 

    pSrcBuffer = new MemoryBase(pSrcHeap, 0, 640*480*3/2 ); 
    pDestBuffer = new MemoryBase(pDestHeap,0, 640*480*2); 

    memcpy((void*)pSrcHeap->base(), (void*)TestImage_YV12_640x480_1, 640*480*3/2); 

    saveBufToFile("/sdcard/src.yuv", (uint8_t*)pSrcHeap->base(), 640*480*3/2); 

    ImageInfo rSrc(HAL_PIXEL_FORMAT_YV12,
                         640, 
                         480, 
                         3, 
                         yv12Stride
                         ); 

    ImageInfo rDst(HAL_PIXEL_FORMAT_YCbCr_422_I,
                   640,
                   480, 
                   1, 
                   yuy2Stride
                        ); 

    NSImageTransform::TrasformOptions rOptions; 
    rOptions.rect = Rect(640, 480);

    ret = scaler->applyTransform(rSrc, pSrcBuffer, rDst, pDestBuffer,  rOptions); 
    //
    saveBufToFile("/sdcard/dest.yuv", (uint8_t*)pDestHeap->base(), 640*480*2); 

    MMSDK_LOGD("disconnect"); 
    scaler->disconnect(); 
    pSrcBuffer = 0; 
    pDestBuffer = 0; 

    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IMMSdkService>
CmdImp::
getMMSdkService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0)
    {
        MMSDK_LOGE("Can not get the service mananger"); 
    }
    //
    sp<IBinder> binder = sm->getService(String16("media.mmsdk"));
    if (binder == 0)
    {
        MMSDK_LOGE("Can not get mmsdk service"); 
        return 0; 
    }
    //
    sp<IMMSdkService> service = interface_cast<IMMSdkService>(binder);
    if (service == 0)
    {
        MMSDK_LOGE("Null mmsdk service"); 
        return 0; 
    }
    return service;
}



/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

