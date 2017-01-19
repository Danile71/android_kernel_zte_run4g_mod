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
#define LOG_TAG "MtkCam/SShotCtrl"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] "fmt, getSensorIdx(), __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
//
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <mtkcam/camnode/IspSyncControl.h>
#include "../inc/StereoShotCtrlNode.h"
using namespace NSCamNode;
//
#include <semaphore.h>
using namespace std;
//
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/exif/IDbgInfoContainer.h>
//
#include <utils/Mutex.h>
using namespace android;

#define USE_3A

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
//namespace NSCamStereo {
////////////////////////////////////////////////////////////////////////////////

#define MODULE_NAME        "SShotCtrl"

class SCtrlNodeImpl : public StereoShotCtrlNode
{
    public:     /*createInstance()*/
               SCtrlNodeImpl(ShotParam const & rShotParam);
               ~SCtrlNodeImpl();

        virtual MBOOL   setIspProfile(NS3A::EIspProfile_T const profile)
        {
            return MFALSE;
        }

    protected:

        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onStart();
        virtual MBOOL onStop();
        virtual MBOOL onNotify(
                        MUINT32 const msg,
                        MUINT32 const ext1,
                        MUINT32 const ext2
                        ) = 0;
        virtual MBOOL onPostBuffer(
                        MUINT32 const data,
                        MUINT32 const buf,
                        MUINT32 const ext
                        ) = 0;
        virtual MBOOL onReturnBuffer(
                        MUINT32 const data,
                        MUINT32 const buf,
                        MUINT32 const ext
                        );
        virtual MVOID onDumpBuffer(
                        const char*   usr,
                        MUINT32 const data,
                        MUINT32 const buf,
                        MUINT32 const ext
                        );

    protected:

        virtual MBOOL _init() { return MTRUE; }
        virtual MBOOL _uninit() { return MTRUE; }

    protected:

        IHal3A*               mpHal3A;
        IspSyncControl*       mpIspSyncCtrl;

        EIspProfile_T         mIspProfile;
};


class NormalCtrl : public SCtrlNodeImpl, public I3ACallBack
{
    public:

        NormalCtrl(ShotParam const & rShotParam)
            : SCtrlNodeImpl(rShotParam)
            , mCurMagicNum(-1)
            , mbDoCap(MFALSE)
        {};
        ~NormalCtrl() {};

        //I3ACallBack
        virtual void doNotifyCb ( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3);

        virtual void doDataCb ( int32_t _msgType, void*   _data, uint32_t _size); 

        virtual MBOOL setIspProfile(NS3A::EIspProfile_T const profile);

    protected:

        virtual MBOOL _init();
        virtual MBOOL _uninit();
        virtual MBOOL onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);
        virtual MBOOL onPostBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext);
        
    protected:
        // 3A related
        MINT32                mCurMagicNum;     //lastest updated magic num
        MBOOL                 mbDoCap;
};


class ZsdVssCtrl : public SCtrlNodeImpl
{
    public:

        ZsdVssCtrl(ShotParam const & rShotParam, MUINT32 const style)
            : SCtrlNodeImpl(rShotParam)
            , muStyle(style)
        {};
        ~ZsdVssCtrl() {};

    protected:

        virtual MBOOL onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);
        virtual MBOOL onPostBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext);

    private:
        
        const MUINT32       muStyle;
};


/*******************************************************************************
*
********************************************************************************/
StereoShotCtrlNode*
StereoShotCtrlNode::
createInstance(ShotParam const & rShotParam, MUINT32 const style, MBOOL const withP1)
{
    if( !withP1 )
        return new ZsdVssCtrl(rShotParam, style);

    return new NormalCtrl(rShotParam);
}


/*******************************************************************************
*
********************************************************************************/
void
StereoShotCtrlNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
StereoShotCtrlNode::
StereoShotCtrlNode(ShotParam const & rShotParam)
    : ICamNode(MODULE_NAME)
    , mShotParam(rShotParam)
{
}


/*******************************************************************************
*
********************************************************************************/
StereoShotCtrlNode::
~StereoShotCtrlNode()
{
}


/*******************************************************************************
*
********************************************************************************/
SCtrlNodeImpl::
SCtrlNodeImpl(ShotParam const & rShotParam)
    : StereoShotCtrlNode(rShotParam)
    , mpHal3A(NULL)
    , mpIspSyncCtrl(NULL)
    , mIspProfile(EIspProfile_Capture)
{
    addDataSupport( ENDPOINT_SRC, CONTROL_FULLRAW );
    addDataSupport( ENDPOINT_SRC, CONTROL_RESIZEDRAW );

    addDataSupport( ENDPOINT_DST, CONTROL_PRV_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_CAP_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_DBGINFO );

    addNotifySupport( CONTROL_STOP_PASS1 | CONTROL_SHUTTER );
}


/*******************************************************************************
*
********************************************************************************/
SCtrlNodeImpl::
~SCtrlNodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onInit()
{
    MBOOL ret = MFALSE;
    //
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    if( mpIspSyncCtrl == NULL )
    {
        MY_LOGE("IspSyncCtrl:createInstance fail");
        goto lbExit;
    }
    //
    mpIspSyncCtrl->setPreviewSize(mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight);
    mpIspSyncCtrl->setCurZoomRatio(mShotParam.u4ZoomRatio);
    //
#ifdef USE_3A
    mpHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getSensorIdx(), getName() );
    if(mpHal3A == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }
#endif
    //
    ret = _init();
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onUninit()
{
    if( !_uninit() )
    {
        MY_LOGE("_uninit failed");
    }
    //
    if(mpHal3A)
    {
        mpHal3A->destroyInstance(getName());
        mpHal3A = NULL;
    }
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onStart()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onStop()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
onPostBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MUINT32       dstdata = 0;
    //
    if( !mbDoCap ) {
        handleReturnBuffer(data, buf);
        goto lbExit;
    }
    //
    switch(data)
    {
        case CONTROL_FULLRAW:
            dstdata    = CONTROL_CAP_SRC;
            break;
        case CONTROL_RESIZEDRAW:
            dstdata = CONTROL_PRV_SRC;
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    if( dstdata != 0 ) {
        handlePostBuffer(dstdata, buf);
    }
    //
lbExit:
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SCtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    MY_LOGV("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    switch(data)
    {
        case CONTROL_PRV_SRC:
            handleReturnBuffer(CONTROL_RESIZEDRAW, buf);
            break;
        case CONTROL_CAP_SRC:
            handleReturnBuffer(CONTROL_FULLRAW, buf);
            break;
        case CONTROL_DBGINFO:
            if( buf != 0 )
            {
                IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(buf);
                pDbgInfo->destroyInstance();
            }
            return MTRUE;
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SCtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
void
NormalCtrl::
doNotifyCb( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3)
{
    //
    if(_msgType == I3ACallBack::eID_NOTIFY_3APROC_FINISH)
    {
        MY_LOGD("3APROC_FINISH:0x%08X,0x%08X",
                _ext1,
                _ext2);

        if(mCurMagicNum == _ext1)
        {
            if(_ext2 & (1 << I3ACallBack::e3AProcOK))
            {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            else
            {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_FAIL);
            }
        }
        else
        {
            MY_LOGE("MagicNum:Cur(0x%08X) != Notify(0x%08X)",
                    mCurMagicNum,
                    _ext1);
        }
    }
}


/*******************************************************************************
*
********************************************************************************/
void
NormalCtrl::
doDataCb( int32_t _msgType, void*   _data, uint32_t _size)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
NormalCtrl::
setIspProfile(NS3A::EIspProfile_T const profile)
{
    mIspProfile = profile;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
_init()
{
    if( mpHal3A )
        mpHal3A->addCallbacks(this);
    //
    if( mIspProfile == EIspProfile_IHDR_Preview || mIspProfile == EIspProfile_IHDR_Video )
        mpIspSyncCtrl->setHdrState(MTRUE);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
_uninit()
{
    //
    if( mpHal3A )
        mpHal3A->removeCallbacks(this);
    //
    if( mIspProfile == EIspProfile_IHDR_Preview || mIspProfile == EIspProfile_IHDR_Video )
        mpIspSyncCtrl->setHdrState(MFALSE);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalCtrl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    switch(msg)
    {
        case PASS1_START_ISP:
            {
                MUINT32     magicNum = ext1;
                MUINT32     sensorScenario;
                MUINT32     sensorWidth;
                MUINT32     sensorHeight;
                MUINT32     sensorType;
                //
                mpIspSyncCtrl->getSensorInfo( 
                        sensorScenario, 
                        sensorWidth,
                        sensorHeight, 
                        sensorType);
                //
                mpHal3A->enterCaptureProcess();
                mpHal3A->setSensorMode(sensorScenario);
                mpHal3A->sendCommand(ECmd_CaptureStart);

                MY_LOGD("3A setIspProfile %d, # 0x%x",
                        EIspProfile_Preview,
                        magicNum );
                ParamIspProfile_T _3A_profile(
                        EIspProfile_Preview,
                        magicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3A->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            break;

        case PASS1_EOF:
            if(mpHal3A && ext1 != MAGIC_NUM_INVALID ) //check if 3A udpate ready
            {
                if( ext2 != MAGIC_NUM_INVALID ) // wait for sensor dealy
                {
                    // stop pass1
                    handleNotify( CONTROL_STOP_PASS1, 0, 0 );

                    if(mpHal3A)
                    {
                        const MUINT32 magicDeque = ext2;

                        mpHal3A->sendCommand(ECmd_CaptureEnd);

                        if( isDataConnected(CONTROL_CAP_SRC) ) {
                            IDbgInfoContainer* pDbgInfo = NULL;
                            if( isDataConnected(CONTROL_DBGINFO) )
                            {
                                pDbgInfo = IDbgInfoContainer::createInstance();
                            }
                            MY_LOGD("3A setIspProfile %d, # 0x%x",
                                    mIspProfile,
                                    SetCap(magicDeque) );
                            ParamIspProfile_T _3A_profile(
                                    mIspProfile,
                                    SetCap(magicDeque),
                                    MFALSE, //always pgn
                                    ParamIspProfile_T::EParamValidate_P2Only);
                            mpHal3A->setIspProfile(_3A_profile, pDbgInfo);
                            if( pDbgInfo != NULL )
                                handlePostBuffer(CONTROL_DBGINFO, (MUINT32)pDbgInfo, 0);
                        }

                        if( isDataConnected(CONTROL_PRV_SRC) ) {
                            MY_LOGD("3A setIspProfile %d, # 0x%x",
                                    EIspProfile_Preview,
                                    magicDeque);
                            ParamIspProfile_T _3A_profile(
                                    EIspProfile_Preview,
                                    magicDeque,
                                    MTRUE, //always rpg
                                    ParamIspProfile_T::EParamValidate_P2Only);
                            mpHal3A->setIspProfile(_3A_profile);
                        }
                    }
                    handleNotify(CONTROL_SHUTTER);
                    mbDoCap = MTRUE;
                    break;
                }
                else
                {
                    //do 3A update
                    mCurMagicNum = ext1;
                    ParamIspProfile_T _3A_profile(
                            EIspProfile_Preview,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINT32>(&_3A_profile));
                }
            }
            break;

        case PASS1_STOP_ISP:
            if( mpHal3A)
                mpHal3A->exitCaptureProcess();
            break;

        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdVssCtrl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    switch(msg)
    {
        case PASS1_START_ISP:
            // do nothing
            break;

        case PASS1_EOF:
            if( ext2 != MAGIC_NUM_INVALID ) // wait for sensor dealy
            {
                // stop pass1
                handleNotify( CONTROL_STOP_PASS1, 0, 0 );
                if(mpHal3A)
                {
                    const MUINT32 magicDeque = ext2;

                    if( isDataConnected(CONTROL_CAP_SRC) ) {
                        IDbgInfoContainer* pDbgInfo = NULL;
                        if( isDataConnected(CONTROL_DBGINFO) )
                        {
                            pDbgInfo = IDbgInfoContainer::createInstance();
                        }
                        MY_LOGD("3A setIspProfile %d, # 0x%x",
                                mIspProfile,
                                SetCap(magicDeque) );
                        ParamIspProfile_T _3A_profile(
                                mIspProfile,
                                SetCap(magicDeque),
                                MFALSE,
                                ParamIspProfile_T::EParamValidate_P2Only);
                        mpHal3A->setIspProfile(_3A_profile, pDbgInfo);
                        if( pDbgInfo != NULL )
                            handlePostBuffer(CONTROL_DBGINFO, (MUINT32)pDbgInfo, 0);
                    }
                }
                handleNotify(CONTROL_SHUTTER);
                break;
            }
            break;

        case PASS1_STOP_ISP:
            // do nothing
            break;

        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdVssCtrl::
onPostBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MUINT32       dstdata = 0;
    //
    switch(data)
    {
        case CONTROL_FULLRAW:
            dstdata    = CONTROL_CAP_SRC;
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    if( dstdata != 0 ) {
        handlePostBuffer(dstdata, buf);
    }
    //
    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
//};	//namespace NSCamStereo
};  //namespace NSCamShot

