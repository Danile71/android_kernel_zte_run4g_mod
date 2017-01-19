/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/CBMgr"
//
#include <list>
#include <utils/threads.h>
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <vector>
using namespace std;
//
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format; 
//
#include <mtkcam/camshot/CapBufMgr.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s]"fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s]"fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s]"fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s]"fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s]"fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s]"fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s]"fmt, __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG, __func__, ##arg)
#endif
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot
{
//
class CapBufMgrImp : public CapBufMgr
{
    public:
        CapBufMgrImp();
        ~CapBufMgrImp();
        //
        virtual MVOID   destroyInstance(void);
        virtual MBOOL   setMaxDequeNum(MUINT32 maxNum);
        virtual MBOOL   dequeBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   enqueBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   popBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   pushBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize = MTRUE);
        //
    private:
        mutable Mutex               mLock;
        mutable Condition           mCond;
        mutable Condition           mCondDequeBuf;
        list<IImageBuffer*>         mlpImgBuf;      // full-size
        list<IImageBuffer*>         mlpImgBuf_Prv;  // preview-size
        MBOOL                       mbUse;
        MUINT32                     mDequeBufCnt;
        MUINT32                     mDequeBufMaxNum;
};
/*******************************************************************************
*
********************************************************************************/
static Mutex                gCapBufMgrImpLock;
static list<CapBufMgrImp*>  glpCapBufMgrImpList;
//-----------------------------------------------------------------------------
CapBufMgr*
CapBufMgr::
createInstance(void)
{
    CapBufMgrImp* pCapBufMgrImp = new CapBufMgrImp();
    return pCapBufMgrImp;
}
//-----------------------------------------------------------------------------
MVOID
CapBufMgrImp::
destroyInstance(void)
{
    delete this;
}
//------------------------------------------------------------------------------
CapBufMgrImp::
CapBufMgrImp()
{
    FUNC_NAME;
    mlpImgBuf.clear();
    mlpImgBuf_Prv.clear();
    mbUse = MFALSE;
    mDequeBufCnt = 0;
    mDequeBufMaxNum = 0;
}
//-----------------------------------------------------------------------------
CapBufMgrImp::
~CapBufMgrImp()
{
    FUNC_NAME;
    mlpImgBuf.clear();
    mlpImgBuf_Prv.clear();
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
setMaxDequeNum(MUINT32 maxNum)
{
    mDequeBufMaxNum = maxNum;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
dequeBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize)
{
    Mutex::Autolock _l(mLock);
    //
    list<IImageBuffer*>::iterator it;
    while(1)
    {
        if(mDequeBufCnt >= mDequeBufMaxNum)
        {
            MY_LOGW("DBC(%d) < DBMN(%d), Wait E",
                    mDequeBufCnt,
                    mDequeBufMaxNum);
            mCondDequeBuf.wait(mLock);
            MY_LOGW("DBC(%d), Wait X",mDequeBufCnt);
        }
        else
        {
            for(it = mlpImgBuf.begin(); it != mlpImgBuf.end(); it++)
            {
                if((*it)->getTimestamp() != 0)
                {
                    break;
                }
            }
            //
            if(it == mlpImgBuf.end())
            {
                MY_LOGW("No buf is ready, Wait E");
                mCond.wait(mLock);
                MY_LOGW("Wait X");
            }
            else
            {
                break;
            }
        }
    }
    //
    mDequeBufCnt++;
    MY_LOGD("(%d),Buf(0x%08X),TS(%d.%06d),DBC(%d)",
            isFullSize,
            (MUINT32)*it,
            (MUINT32)(((*it)->getTimestamp()/1000)/1000000), 
            (MUINT32)(((*it)->getTimestamp()/1000)%1000000),
            mDequeBufCnt);
    //
    pImageBuffer = *it;
    mlpImgBuf.erase(it);
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
enqueBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize)
{
    Mutex::Autolock _l(mLock);
    //
    pImageBuffer->setTimestamp(0);
    //
    mlpImgBuf.push_back(pImageBuffer);
    if(mDequeBufCnt == 0)
    {
        MY_LOGE("mDequeBufCnt is 0");
    }
    else
    {
        mDequeBufCnt--;
    }
    //
    mCondDequeBuf.signal();
    //
    MY_LOGD("(%d),Buf(0x%08X),TS(%d.%06d),DBC(%d)",
            isFullSize,
            (MUINT32)pImageBuffer,
            (MUINT32)((pImageBuffer->getTimestamp()/1000)/1000000), 
            (MUINT32)((pImageBuffer->getTimestamp()/1000)%1000000),
            mDequeBufCnt);
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
popBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize)
{
    Mutex::Autolock _l(mLock);
    //
    if(mlpImgBuf.empty())
    {
        if(mbUse)
        {
            MY_LOGW("buf is empty");
        }
        return MFALSE;
    }
    //
    list<IImageBuffer*>::iterator it = mlpImgBuf.begin();
    //
    MY_LOGD("(%d),Buf(0x%08X),TS(%d.%06d)",
            isFullSize,
            (MUINT32)*it,
            (MUINT32)(((*it)->getTimestamp()/1000)/1000000), 
            (MUINT32)(((*it)->getTimestamp()/1000)%1000000));
    //
    pImageBuffer = *it;
    mlpImgBuf.erase(it);
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
pushBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("(%d),Buf(0x%08X),TS(%d.%06d)",
            isFullSize,
            (MUINT32)pImageBuffer,
            (MUINT32)((pImageBuffer->getTimestamp()/1000)/1000000), 
            (MUINT32)((pImageBuffer->getTimestamp()/1000)%1000000));
    //
    mlpImgBuf.push_back(pImageBuffer);
    if(pImageBuffer->getTimestamp() > 0)
    {
        mCond.signal();
    }
    //
    if(!mbUse)
    {
        mbUse = MTRUE;
    }
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
};

