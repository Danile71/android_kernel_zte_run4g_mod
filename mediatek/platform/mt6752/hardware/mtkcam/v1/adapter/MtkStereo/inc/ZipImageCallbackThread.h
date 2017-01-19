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

#ifndef _MTK_HAL_CAMADAPTER_MTKSTEREO_INC_ZIPIMAGECALLBACKTHREAD_H_
#define _MTK_HAL_CAMADAPTER_MTKSTEREO_INC_ZIPIMAGECALLBACKTHREAD_H_
//
#include <queue>
using namespace std;
//
#include <utils/threads.h>
#include <utils/StrongPointer.h>
//
#include "inc/IState.h"

namespace android {
namespace NSMtkStereoCamAdapter {


/******************************************************************************
 *  ZIP (Compressed) Image Callback Thread
 ******************************************************************************/
class ZipImageCallbackThread : public Thread
{
    public:

        typedef enum
        {
            // notify callback
            callback_type_shutter,
            callback_type_shutter_nosound,
            callback_type_raw_notify,
            // data callback
            callback_type_raw,
            callback_type_jpeg,
            // exit type
            callback_type_exit,
        }callback_type;
        
        struct callback_data
        {
            callback_type           type;
            //image
            camera_memory*          pImage;
            uint32_t                u4CallbackIndex;
            bool                    isFinalImage;
        };

    protected:  ////                Data Members.
        sp<Thread>                  mpSelf;
        char const*const            mpszThreadName;
        IStateManager*              mpStateMgr;
        sp<CamMsgCbInfo>            mpCamMsgCbInfo;
        uint32_t                    mu4ShotMode;
        IState::ENState             mNextState;
        bool                        mIsExtCallback;

        Mutex                       mLock;
        Condition                   mCondCallback;

        queue<MUINT32>              mqTodoJobs;
        queue<callback_data>        mqTodoCallback;

    public:

        ZipImageCallbackThread(
                IStateManager* pStateMgr,
                sp<CamMsgCbInfo> pCamMsgCbInfo
                );
        ~ZipImageCallbackThread();

        // Good place to do one-time initializations
        status_t    readyToRun();
        void        setShotMode(uint32_t const u4ShotMode, const IState::ENState nextState);
        bool        isExtCallback() { return mIsExtCallback; }
        void        addCallbackData(callback_data* pData);
        void        doCallback(MUINT32 const type);

    protected:

        bool        getCallbackType(MUINT32* pType);
        bool        getCallbackData(MUINT32 const type, callback_data* pData);
        //
        bool        handleShutter(bool const bPlayShutterSound);
        bool        handleRawNotify();
        bool        handleRawCallback(callback_data const& data);
        bool        handleJpegCallback(callback_data const& data);

    private:

        bool        threadLoop();
};

}; // namespace NSMtkStereoCamAdapter
}; // namespace android
#endif  //_MTK_HAL_CAMADAPTER_MTKSTEREO_INC_ZIPIMAGECALLBACKTHREAD_H_

