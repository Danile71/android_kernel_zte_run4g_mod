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
#define LOG_TAG "MtkCam/DCNode"
//
#include <cutils/properties.h>      // [debug] should be remove
//
#include <CamUtils.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <vector>
using namespace std;
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <DefaultCtrlNodeImpl.h>
using namespace NSCamNode;
//
#include <mtkcam/core/camshot/inc/CamShotUtils.h>
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNode*
DefaultCtrlNode::
createInstance(
    const char*     userName,
    CTRL_NODE_TYPE  ctrlNodeType)
{
    switch(ctrlNodeType)
    {
        case CTRL_NODE_DEFAULT:
        {
            return new DefaultCtrlNodeImpl(userName);
        }
        case CTRL_NODE_ENG:
        {
            return new DefaultCtrlNodeEngImpl(userName);
        }
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
DefaultCtrlNode::
destroyInstance()
{
  delete this;
}


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNode::
DefaultCtrlNode()
    : ICamNode(LOG_TAG)
{
}


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNode::
~DefaultCtrlNode()
{
}


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNodeImpl::
DefaultCtrlNodeImpl(const char* userName)
    : muStateFlag(FLAG_DO_3A_UPDATE)
    , mDropFrameCnt(0)
    , m3aReadyMagic(MAGIC_NUM_INVALID)
    , mpHal3a(NULL)
    , mpCallbackZoom(NULL)
    , mspParamsMgr(NULL)
    , mspCamMsgCbInfo(NULL)
    , mpCapBufMgr(NULL)
    , mpIspSyncCtrl(NULL)
    , mpVHdrHal(NULL)
    , mpEisHal(NULL)
    , mEisScenario(EIS_SCE_EIS)
    , msName(userName)
    , mbIsForceRotation(MFALSE)
    , mRotationAnagle(0)
    , mbSkipPrecapture(MFALSE)
    , mRecordingHint(MFALSE)
    , mbUpdateEis(MFALSE)
    , mbUpdateVHdr(MFALSE)
    , mSensorType(0)
    , mSensorScenario(0)
    , mReplaceBufNumResize(0)
    , mReplaceBufNumFullsize(0)
    , mRollbackBufNum(0)
    , mRollbackBufPeriod(1)
    , mRollbackBufCnt(0)
    , mPreviewMaxFps(30)
    , mShotMode(0)
    , mMode(MODE_IDLE)
    , mCurMagicNum(0)
{
    FUNC_NAME;

    //DATA
    addDataSupport( ENDPOINT_SRC, CONTROL_FULLRAW );
    addDataSupport( ENDPOINT_SRC, CONTROL_RESIZEDRAW );
    addDataSupport( ENDPOINT_DST, CONTROL_PRV_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_CAP_SRC );
    //notify
    addNotifySupport( CONTROL_STOP_PASS1 );
    //
    mPreviewSize.w = 0;
    mPreviewSize.h = 0;
    mVideoSize.w = 0;
    mVideoSize.h = 0;
    mSensorSize.w = 0;
    mSensorSize.h = 0;
}


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNodeImpl::
~DefaultCtrlNodeImpl()
{
    FUNC_NAME;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onInit()
{
    FUNC_START;
    //
    mpHal3a = IHal3A::createInstance(
                        IHal3A::E_Camera_1,
                        getSensorIdx(),
                        LOG_TAG);
    if(mpHal3a == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        return MFALSE;
    }
    //
    mpHal3a->addCallbacks(this);
    //
    updateDelay(EQueryType_Init);
    //
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    if(mpIspSyncCtrl)
    {
        MINT32 i;
        vector<MUINT32> zoomRatios;
        for(i=0; i<=mspParamsMgr->getInt(CameraParameters::KEY_MAX_ZOOM); i++)
        {
            zoomRatios.push_back(mspParamsMgr->getZoomRatioByIndex(i));
        }
        mpIspSyncCtrl->setZoomRatioList(zoomRatios);
        //
        if(CamManager::getInstance()->isMultiDevice())
        {
            mpIspSyncCtrl->setHdrState(SENSOR_VHDR_MODE_NONE);
        }
        else
        {
            mpIspSyncCtrl->setHdrState(mspParamsMgr->getVHdr());
        }
        mpIspSyncCtrl->addPass1ReplaceBuffer(mReplaceBufNumFullsize, mReplaceBufNumResize);
        mpIspSyncCtrl->setPreviewSize(mPreviewSize.w, mPreviewSize.h);
        if (mspParamsMgr != NULL)
        {
            mpIspSyncCtrl->setCurZoomRatio(mspParamsMgr->getZoomRatioByIndex(mspParamsMgr->getInt(CameraParameters::KEY_ZOOM)));
        }
        //
        MUINT32 sensorW, sensorH;
        mpIspSyncCtrl->getSensorInfo(
                            mSensorScenario,
                            sensorW,
                            sensorH,
                            mSensorType);
        mSensorSize.w = sensorW;
        mSensorSize.h = sensorH;
        //
        MSize videoSize, rrzoMinSize, initRrzoSize;
        //
        if( mSensorSize.w*mSensorSize.h >= IMG_2160P_SIZE &&
            mVideoSize.w*mVideoSize.h > IMG_1080P_SIZE)
        {
            videoSize.w = IMG_2160P_W;
            videoSize.h = IMG_2160P_H;
        }
        else
        {
            videoSize.w = IMG_1080P_W;
            videoSize.h = IMG_1080P_H;
        }
        //
        rrzoMinSize.w = EIS_MIN_WIDTH;
        rrzoMinSize.h = (MUINT32)((EIS_MIN_WIDTH*mPreviewSize.h)/mPreviewSize.w);
        //
        MSize maxSize, sizeList[3] = {mPreviewSize, videoSize, rrzoMinSize};
        //4K2K video record does not support full size output.
        MY_LOGD("RH(%d)",mRecordingHint);
        if (mMode != MODE_STEREO_PREVIEW || mMode != MODE_STEREO_ZSD_PREVIEW)
        {
            if( mRecordingHint == MTRUE &&
                mMode == MODE_VIDEO_PREVIEW)
            {
                maxSize.w = 0;
                maxSize.h = 0;
                for(i=0; i<3; i++)
                {
                    MY_LOGD("sizeList[%d]=%dx%d",
                            i,
                            sizeList[i].w,
                            sizeList[i].h);
                    if(maxSize.w < sizeList[i].w)
                    {
                       maxSize.w = sizeList[i].w;
                    }
                    if(maxSize.h < sizeList[i].h)
                    {
                       maxSize.h = sizeList[i].h;
                    }
                }
                //
                MY_LOGD("maxSize(%dx%d)",
                        maxSize.w,
                        maxSize.h);
                initRrzoSize.w = (maxSize.w*EIS_FACTOR)/100;
                initRrzoSize.h = (maxSize.h*EIS_FACTOR)/100;
                MY_LOGD("initRrzoSize(%dx%d)",
                        initRrzoSize.w,
                        initRrzoSize.h);
                //
                MUINT32 aaaUpdatePeriod;
                if(mPreviewMaxFps < AAA_UPDATE_BASE)
                {
                    aaaUpdatePeriod = 1;
                }
                else
                {
                    aaaUpdatePeriod = mPreviewMaxFps/AAA_UPDATE_BASE;
                }
                if(mspParamsMgr->getVideoStabilization())
                {
                    mpIspSyncCtrl->setVideoSize(
                                        (MUINT32)((mPreviewSize.w*EIS_FACTOR)/100),
                                        (MUINT32)((mPreviewSize.h*EIS_FACTOR)/100));
                }
                else
                {
                    mpIspSyncCtrl->setVideoSize(0, 0);
                }
                //
                mpIspSyncCtrl->set3AUpdatePeriod(aaaUpdatePeriod);
                mpIspSyncCtrl->setRrzoMinSize(
                                    rrzoMinSize.w,
                                    rrzoMinSize.h);
            }
            else
            {
                initRrzoSize.w = mPreviewSize.w;
                initRrzoSize.h = mPreviewSize.h;
                //
                mpIspSyncCtrl->setVideoSize(0, 0);
                mpIspSyncCtrl->set3AUpdatePeriod(1);
                mpIspSyncCtrl->setRrzoMinSize(0, 0);
            }
            //
            if( initRrzoSize.w > mSensorSize.w)
            {
                initRrzoSize.w = mSensorSize.w;
            }
            if( initRrzoSize.h > mSensorSize.h)
            {
                initRrzoSize.h = mSensorSize.h;
            }
            MY_LOGD("initRrzoSize(%dx%d)",
                    initRrzoSize.w,
                    initRrzoSize.h);
            //
            mpIspSyncCtrl->setPass1InitRrzoSize(
                                initRrzoSize.w,
                                initRrzoSize.h);
            mpIspSyncCtrl->calRrzoMaxZoomRatio();
        }
        else
        {
            mpIspSyncCtrl->set3AUpdatePeriod(1);
        }
    }
    //
    #ifdef MTK_CAM_VHDR_SUPPORT
    if(mpIspSyncCtrl->getHdrState() != SENSOR_VHDR_MODE_NONE)
    {
        mpVHdrHal = VHdrHal::CreateInstance(LOG_TAG,getSensorIdx());
        if(mpVHdrHal)
        {
            mpVHdrHal->Init(mspParamsMgr->getVHdr());
        }
        else
        {
            MY_LOGE("mpVHdrHal is NULL");
        }
    }
    #endif
    //
#if 0    //this chip not support EIS2.0
    MBOOL isVFB = (::strcmp(mspParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0) ? MTRUE : MFALSE;
    if(mVideoSize.w*mVideoSize.h > IMG_1080P_SIZE || isVFB)
    {
        mEisScenario = EIS_SCE_EIS;
    }
    else
    {
        mEisScenario = EIS_SCE_EIS_PLUS;
    }
#else
    mEisScenario = EIS_SCE_EIS;
#endif
    //
    mRollbackBufCnt = 0;
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onUninit()        
{
    FUNC_START;
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    #if EIS_TEST
    if(mpEisHal)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance(LOG_TAG);
        mpEisHal = NULL;
    }
    #endif
    //
    #ifdef MTK_CAM_VHDR_SUPPORT
    if(mpVHdrHal)
    {
        mpVHdrHal->Uninit();
        mpVHdrHal->DestroyInstance(LOG_TAG);
        mpVHdrHal = NULL;
    }
    #endif
    //
    if(mpHal3a)
    {
        mpHal3a->removeCallbacks(this);
        mpHal3a->destroyInstance(LOG_TAG);
        mpHal3a = NULL;
    }
    //
    if(mpCallbackZoom)
    {
        mpCallbackZoom->destroyUser();
        mpCallbackZoom->destroyInstance();
        mpCallbackZoom = NULL;
    }
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onStart()        
{
    FUNC_START;
    // reset variables
    muStateFlag = FLAG_DO_3A_UPDATE|FLAG_STARTED;
    m3aReadyMagic = MAGIC_NUM_INVALID;
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onStop()
{
    FUNC_START;
    //
    IImageBuffer* pImgBuf = NULL;
    while(mpCapBufMgr->popBuf(pImgBuf))
    {
        handleReturnBuffer(CONTROL_FULLRAW, (MUINT32)pImgBuf);
    }
    disableFlag(muStateFlag, FLAG_ALL);
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    //FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    if(msg != PASS1_CONFIG_FRAME)
    {
        MY_LOGD("(0x%08X,0x%08X,0x%08X)",
                msg,
                ext1,
                ext2);
    }
    //
    switch(msg)
    {
        case PASS1_START_ISP:
        {
            if(mpHal3a)
            {
                ECmd_T cmd;
                E3APreviewMode_T pvMode;
                //
                mCurMagicNum = ext1;
                if( mMode == MODE_NORMAL_PREVIEW ||
                    mMode == MODE_ZSD_PREVIEW ||
                    mMode == MODE_STEREO_PREVIEW ||
                    mMode == MODE_STEREO_ZSD_PREVIEW )
                {
                    pvMode = EPv_Normal;
                    cmd = ECmd_CameraPreviewStart;
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_IVHDR)
                    {
                        mIspProfile = EIspProfile_IHDR_Preview;
                    }
                    else
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_MVHDR)
                    {
                        mIspProfile = EIspProfile_MHDR_Preview;
                    }
                    else
                    {
                        mIspProfile = EIspProfile_Preview;
                    }
                }
                else
                if( mMode == MODE_VIDEO_PREVIEW ||
                    mMode == MODE_VIDEO_RECORD)
                {
                    pvMode = EPv_Video;
                    cmd = ECmd_CameraPreviewStart;
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_IVHDR)
                    {
                        mIspProfile = EIspProfile_IHDR_Video;
                    }
                    else
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_MVHDR)
                    {
                        mIspProfile = EIspProfile_MHDR_Video;
                    }
                    else
                    {
                        mIspProfile = EIspProfile_Video;
                    }
                }
                else
                {
                    MY_LOGE("un-support mMode(%d)",mMode);
                }
                mpHal3a->setSensorMode(mSensorScenario);
                //mpHal3a->set3APreviewMode(pvMode);
                #ifdef MTK_CAM_VHDR_SUPPORT
                if(mpVHdrHal)
                {
                    VHDR_HAL_CONFIG_DATA vhdrConfigData;
                    vhdrConfigData.cameraVer = VHDR_CAMERA_VER_1;
                    mpVHdrHal->ConfigVHdr(vhdrConfigData);
                }
                #endif
                // update crop information to 3A
                mpIspSyncCtrl->updatePass2ZoomRatio(100);
                MUINT32 zoomRatio = 0, cropX = 0, cropY = 0, cropW = 0, cropH = 0;
                if(mpIspSyncCtrl->getCurPass2ZoomInfo(zoomRatio,cropX,cropY,cropW,cropH))
                {
                    MY_LOGD("PASS1_START_ISP: setZoom:ZR(%d),Crop(%d,%d,%dx%d)",zoomRatio,cropX,cropY,cropW,cropH);
                    mpHal3a->setZoom(
                                zoomRatio,
                                cropX,
                                cropY,
                                cropW,
                                cropH);
                }
                //
                mpHal3a->sendCommand(cmd);
                ParamIspProfile_T _3A_profile(
                        mIspProfile,
                        mCurMagicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3a->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            //
            #if EIS_TEST
            MBOOL bCreateEis = MFALSE;
            if( mPreviewMaxFps <= EIS_MAX_FPS &&
                (   mMode == MODE_VIDEO_PREVIEW ||
                    mMode == MODE_VIDEO_RECORD ||
                    mMode == MODE_NORMAL_PREVIEW ||
                    mMode == MODE_ZSD_PREVIEW ||
                    mMode == MODE_STEREO_PREVIEW ||
                    mMode == MODE_STEREO_ZSD_PREVIEW))
            {
                bCreateEis = MTRUE;
            }
            //
            if(bCreateEis)
            {
                mpEisHal = EisHal::CreateInstance(LOG_TAG,getSensorIdx());
                if(mpEisHal)
                {
                    mpEisHal->Init();
                    if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                    {
                        EIS_PASS_ENUM eisPass = EIS_PASS_1;
                        EIS_HAL_CONFIG_DATA  eisConfig;
                        MINT32 eisErr;
                        eisConfig.sensorType    = mSensorType;
                        eisConfig.configSce     = mEisScenario;
                        eisConfig.memAlignment  = 2;
                        eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                        if(eisErr != EIS_RETURN_NO_ERROR)
                        {
                            MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                        }
                    }
                }
                else
                {
                    MY_LOGE("mpEisHal is NULL");
                }
            }
            #endif
            break;
        }
        case PASS1_STOP_ISP:
        {
            if(mpEisHal)
            {
                mpEisHal->SendCommand(EIS_CMD_SET_STATE,EIS_SW_STATE_UNINIT);
            }
            //
            if(mpHal3a)
            {
                mpHal3a->sendCommand(ECmd_CameraPreviewEnd);
            }
            else
            {
                MY_LOGW("mpHal3a is NULL");
            }
            //
            if(mpEisHal)
            {
                mpEisHal->SendCommand(EIS_CMD_CHECK_STATE,EIS_SW_STATE_UNINIT_CHECK);
            }
            break;
        }
        case PASS1_EOF:
        {
            {
                Mutex::Autolock _l(mLock);
                //
                if( mpHal3a &&
                    getFlag(muStateFlag, FLAG_DO_3A_UPDATE) && // to avoid send update after precaptrue-end
                    ext1 != MAGIC_NUM_INVALID )
                {
                    MUINT32 zoomRatio = 0, cropX = 0, cropY = 0, cropW = 0, cropH = 0;
                    if(mpIspSyncCtrl->getCurPass2ZoomInfo(zoomRatio,cropX,cropY,cropW,cropH))
                    {
                        MY_LOGD("setZoom:ZR(%d),Crop(%d,%d,%dx%d)",zoomRatio,cropX,cropY,cropW,cropH);
                        mpHal3a->setZoom(
                                    zoomRatio,
                                    cropX,
                                    cropY,
                                    cropW,
                                    cropH);
                    }
                    //do 3A update
                    mCurMagicNum = ext1;
                    ParamIspProfile_T _3A_profile(
                            mIspProfile,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3a->sendCommand(ECmd_Update, reinterpret_cast<MINT32>(&_3A_profile));
                }
                else
                {
                    MY_LOGD("skip update");
                }
            }
            //
            if(updateReadyBuffer(ext2))
            {
                mbUpdateEis = MTRUE;
                mbUpdateVHdr = MTRUE;
            }
            else
            {
                mbUpdateVHdr = MFALSE;

                #ifdef MTK_CAM_VHDR_SUPPORT
                if(mpVHdrHal)
                {
                    //set timestamp = 0 for VHDR driver.
                    mpVHdrHal->DoVHdr(0);
                }
                #endif

                mbUpdateEis = MFALSE;
                if(mpEisHal)
                {
                    //set timestamp = 0 for EIS driver.
                    mpEisHal->DoEis(EIS_PASS_1,NULL,0);
                }
            }
            break;
        }
        case PASS1_CONFIG_FRAME:
        {
            #if EIS_TEST
            if( mpEisHal &&
                mpIspSyncCtrl)
            {
                MUINT32 magicNum = ext1;
                MUINT32 currentZoomRatio;
                MUINT32 targetZoomRatio;
                MRect inCrop;
                MSize outSize;
                //
                if(mpIspSyncCtrl->queryPass1CropInfo(
                                    magicNum,
                                    currentZoomRatio,
                                    targetZoomRatio,
                                    inCrop,
                                    outSize))
                {
                    // not use at this chip
                }
                //
                if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                {
                    EIS_PASS_ENUM eisPass = EIS_PASS_1;
                    EIS_HAL_CONFIG_DATA  eisConfig;
                    MINT32 eisErr;
                    eisConfig.sensorType    = mSensorType;
                    eisConfig.configSce     = mEisScenario;
                    eisConfig.memAlignment  = 2;
                    eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                    if(eisErr != EIS_RETURN_NO_ERROR)
                    {
                        MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                    }
                }
            }
            #endif
            break;
        }
        default:
        {
            ret = MTRUE;
        }
    }
    //
    //FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onPostBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    //FUNC_START;
    MBOOL         doPost  = MFALSE;
    MUINT32       dstdata = 0;
    MY_LOGV("(%d,0x%08X,0x%08X)", data, buf, ext);

    {
        //drop frame or not
        Mutex::Autolock _l(mLock);
        if( getFlag(muStateFlag, FLAG_BUF_PREVIEW_OK) )
            doPost = MTRUE;
    }

    if( !doPost )
    {
        // drop frame
        handleReturnBuffer(data, buf);
        return MTRUE;
    }

    //post buffer to Pass2
    switch(data)
    {
        case CONTROL_FULLRAW:
        {
            dstdata = CONTROL_CAP_SRC;
            break;
        }
        case CONTROL_RESIZEDRAW:
        {
            dstdata = CONTROL_PRV_SRC;
            break;
        }
        default:
        {
            MY_LOGE("not support yet: %d", data);
            break;
        }
    }
    //
    #if EIS_TEST
    if(mbUpdateEis)
    {
        if(mpEisHal)
        {
            IspSyncControl::EIS_CROP_INFO eisCrop;
            eisCrop.enable = MFALSE;
            IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
            EIS_HAL_CONFIG_DATA eisHalCfgData;

            eisHalCfgData.p1ImgW = (MUINT32)pImageBuffer->getImgSize().w;
            eisHalCfgData.p1ImgH = (MUINT32)pImageBuffer->getImgSize().h;

            mpEisHal->DoEis(
                        EIS_PASS_1,
                        &eisHalCfgData,
                        pImageBuffer->getTimestamp());
            //
            if( mpEisHal->GetEisSupportInfo(getSensorIdx()) &&
                (mMode == MODE_VIDEO_PREVIEW || mMode == MODE_VIDEO_RECORD))
            {
                if( mspParamsMgr->getVideoStabilization() &&
                    mEisScenario == EIS_SCE_EIS)
                {
                    MUINT32 tarWidth = 0, tarHeight = 0;
                    mpEisHal->GetEisResult(
                                eisCrop.xInt,
                                eisCrop.xFlt,
                                eisCrop.yInt,
                                eisCrop.yFlt,
                                tarWidth,
                                tarHeight);
                    //
                    eisCrop.enable = MTRUE;
                    eisCrop.scaleFactor = 100.0/EIS_FACTOR;
                }
            }
            mpIspSyncCtrl->setEisResult(
                            pImageBuffer,
                            eisCrop);
        }
        //
        mbUpdateEis = MFALSE;
    }
    #endif
    //
    if(mbUpdateVHdr)
    {
        #ifdef MTK_CAM_VHDR_SUPPORT
        if(mpVHdrHal)
        {
            IImageBuffer* pImageBuffer = (IImageBuffer*)buf;            
            mpVHdrHal->DoVHdr(pImageBuffer->getTimestamp());
        }
        #endif
        //
        mbUpdateVHdr = MFALSE;
    }
    //
    handlePostBuffer(dstdata, buf);
    //
    if( data == CONTROL_RESIZEDRAW )
    {
    //  wait here to let pass1node possible to receive CONTROL_STOP_PASS1 notify
    //  before deque next buffer
        MBOOL wait = MFALSE;
        {
            Mutex::Autolock _l(mLock);
            wait = getFlag(muStateFlag, FLAG_PRECAPTURE);
        }

        if( wait )
        {
#define TIMEOUT_WAIT_READYTOCAP     (10000) //us
            NG_TRACE_NAME("waitReadyToCap");
            MY_LOGD("wait for %d", TIMEOUT_WAIT_READYTOCAP);
            usleep(TIMEOUT_WAIT_READYTOCAP);
#undef TIMEOUT_WAIT_READYTOCAP
        }
    }
    //FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL 
DefaultCtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    //FUNC_START;
    MY_LOGV("(%d,0x%08X,0x%08X)", data, buf, ext);
    switch(data)
    {
        case CONTROL_CAP_SRC:
        {
            MBOOL saveBuf = MTRUE;
            /*
            MY_LOGD("Cnt(%d),Num(%d),Period(%d)",
                    mRollbackBufCnt,
                    mRollbackBufNum,
                    mRollbackBufPeriod);
            */
            if(mRollbackBufCnt > mRollbackBufNum)
            {
                if((mRollbackBufCnt-mRollbackBufNum) % mRollbackBufPeriod != 0)
                {
                    saveBuf = MFALSE;
                    mRollbackBufCnt++;
                }
            }
            //
            if( mRollbackBufNum > 0 &&
                saveBuf)
            {
                
                IImageBuffer* pImgBuf = NULL;
                pImgBuf = (IImageBuffer*)buf;
                mpCapBufMgr->pushBuf(pImgBuf);
                //
                if(mRollbackBufCnt >= mRollbackBufNum)
                {
                    pImgBuf = NULL;
                    mpCapBufMgr->popBuf(pImgBuf);
                    handleReturnBuffer(CONTROL_FULLRAW, (MUINT32)pImgBuf);
                }
                mRollbackBufCnt++;
            }
            else
            {
                handleReturnBuffer(CONTROL_FULLRAW, buf);
            }
            break;
        }
        case CONTROL_PRV_SRC:
        {
            handleReturnBuffer(CONTROL_RESIZEDRAW, buf);
            break;
        }
        default:
        {
            MY_LOGE("not support yet: %d", data);
            break;
        }
    }
    //FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
DefaultCtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
    //TBD
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
setConfig(CONFIG_INFO configInfo)
{
    MY_LOGD("RPBuf:Num RRZO(%d) IMGO(%d),RBBuf:Num(%d) Period(%d),PrvMaxFps(%d)",
            configInfo.replaceBufNumResize,
            configInfo.replaceBufNumFullsize,
            configInfo.rollbackBufNum,
            configInfo.rollbackBufPeriod,
            configInfo.previewMaxFps);
    //
    if(configInfo.spParamsMgr != NULL)
    {
        mspParamsMgr = configInfo.spParamsMgr;
    }
    //
    if(configInfo.spCamMsgCbInfo != NULL)
    {
        mspCamMsgCbInfo = configInfo.spCamMsgCbInfo;
    }
    //
    if(configInfo.pCapBufMgr != NULL)
    {
        mpCapBufMgr = configInfo.pCapBufMgr;
    }
    //
    mReplaceBufNumResize = configInfo.replaceBufNumResize;
    mReplaceBufNumFullsize = configInfo.replaceBufNumFullsize;
    mRollbackBufNum = configInfo.rollbackBufNum;
    mRollbackBufPeriod = configInfo.rollbackBufPeriod;
    mPreviewMaxFps = configInfo.previewMaxFps;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
updateDelay(EQueryType_T const type)
{
    MBOOL ret = MTRUE;
    MINT32 delayCnt = 0;
    //(1)
    switch (type)
    {
        case EQueryType_Init:
        {
            MINT32 delay3A = 0;
            //
            if(mpHal3a)
            {
                delay3A = mpHal3a->getDelayFrame(type);
            }
            else
            {
                MY_LOGW("mpHal3a is NULL");
            }
            delayCnt = delay3A;
            //
            MY_LOGD("3A(%d)", delay3A);
            break;
        }
        case EQueryType_Effect:
        case EQueryType_AWB:
        {
            if(mpHal3a)
            {
                delayCnt = mpHal3a->getDelayFrame(type);
            }
            else
            {
                MY_LOGW("mpHal3a is NULL");
            }
            break;
        }
        default:
        {
            MY_LOGW("not support type(%d)",type);
            break;
        }
    }
    //
    //
    MY_LOGD("type(%d),delay(%d)",
            type,
            delayCnt);
    if(delayCnt > 0)
    {
        // [debug] should be remove
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        MINT32 debug = true;
        property_get("debug.aaa.ctrl.enable", value, "767");
        debug = atoi(value);
        if (debug != 0)
        {
            Mutex::Autolock _l(mLock);
            mDropFrameCnt = delayCnt;
        }
    }
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
setParameters()
{
    FUNC_NAME;
    //
    if(mspParamsMgr != NULL)
    {
        if(mMode == MODE_IDLE)
        {
            mspParamsMgr->getPreviewSize(&mPreviewSize.w, &mPreviewSize.h);
        }
        //
        if( mMode != MODE_VIDEO_RECORD &&
            mMode != MODE_VIDEO_SNAPSHOT)
        {
            mspParamsMgr->getVideoSize(&mVideoSize.w, &mVideoSize.h);
        }
        //
        if( mspParamsMgr->getRecordingHint() &&
            (   (   mspParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) >= 120 &&
                    (mVideoSize.w*mVideoSize.h) >= IMG_1080P_SIZE) ||
                mspParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) >= 180))
        {
            mspParamsMgr->set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, MtkCameraParameters::PIXEL_FORMAT_BITSTREAM);
        }
        else
        {
            mspParamsMgr->set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420P);
        }
        //
        if( mbIsForceRotation &&
            (   mRotationAnagle == 90 ||
                mRotationAnagle == 270))
        {
            MINT32 temp = mPreviewSize.w;
            mPreviewSize.w = mPreviewSize.h;
            mPreviewSize.h = temp;
        }
        //
        if( mpIspSyncCtrl != NULL)
        {
            mpIspSyncCtrl->setPreviewSize(mPreviewSize.w, mPreviewSize.h);
            mpIspSyncCtrl->setCurZoomRatio(mspParamsMgr->getZoomRatioByIndex(mspParamsMgr->getInt(CameraParameters::KEY_ZOOM)));
        }
        //
        MY_LOGD("FR(%d,%d),PrvSize(%dx%d),VdoSize(%dx%d)",
                mbIsForceRotation,
                mRotationAnagle,
                mPreviewSize.w,
                mPreviewSize.h,
                mVideoSize.w,
                mVideoSize.h);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
startSmoothZoom(MINT32 arg1)
{
    MY_LOGD("+ (%d)", arg1);
    //
    if(mspParamsMgr == NULL)
    {
        MY_LOGW("mspParamsMgr is nULL");
        return MFALSE;
    }
    //
    MINT32 i, curIdx = mspParamsMgr->getInt(CameraParameters::KEY_ZOOM);
    //
    if(arg1 < 0 || arg1 > mspParamsMgr->getInt(CameraParameters::KEY_MAX_ZOOM))
    {
        MY_LOGE("return fail: smooth zoom(%d)", arg1);
        return MFALSE;
    }
    //
    if(arg1 == curIdx)
    {
        MY_LOGD("smooth zoom(%d) equals to current", arg1);
        return MTRUE; 
    }
    //initial setting
    if(mpCallbackZoom == NULL)
    {
        mpCallbackZoom = ICallBackZoom::createInstance();
        if(mpCallbackZoom == NULL)
        {
            MY_LOGE("mpCallbackZoom == NULL");
            return INVALID_OPERATION;
        }
    }
    //
    mpCallbackZoom->setUser(
        mspCamMsgCbInfo,
        mspParamsMgr,
        mpIspSyncCtrl);
    //
    mpCallbackZoom->setZoomTarget(curIdx, arg1);
    //
    if(mpIspSyncCtrl)
    {
       mpIspSyncCtrl->setCallbackZoom(
                        this,
                        cbZoomFunc);
        //
        while(mpIspSyncCtrl->popEndZoomRatio() != -1);
        //
        if(curIdx < arg1)
        {
            for(i=curIdx+1; i<=arg1; i++)
            {
                mpIspSyncCtrl->addZoomRatio(mspParamsMgr->getZoomRatioByIndex(i));
            }
        }
        else
        if(curIdx > arg1)
        {
            for(i=curIdx-1; i>=arg1; i--)
            {
                mpIspSyncCtrl->addZoomRatio(mspParamsMgr->getZoomRatioByIndex(i));
            }
        }
    }
    //
    FUNC_END;
    //
    return MTRUE;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
stopSmoothZoom()
{
    FUNC_START;
    //
    if(mpCallbackZoom == NULL)
    {
        MY_LOGD("Just do nothing");
        return MTRUE;
    }
    //
    MINT32 popRatio = -1;
    Vector<MINT32> vPopRatio;
    while( -1 != (popRatio = mpIspSyncCtrl->popEndZoomRatio()) )
    {
        vPopRatio.push_back(popRatio);
    }
    // To guarentee:
    // (1) must send one callback if AP has ever sent startSmoothZoom;
    // (2) the last callback must indicate "stop";  
    while( ! vPopRatio.empty() && ! mpCallbackZoom->requestStop())
    {
        MY_LOGD("Push ratio(%d) for compensation", *(vPopRatio.end()-1));
        mpIspSyncCtrl->addZoomRatio(*(vPopRatio.end()-1));
        vPopRatio.erase(vPopRatio.end()-1);
    }
    //
    if(mpIspSyncCtrl)
    {
       mpIspSyncCtrl->setCallbackZoom(
                        (MVOID*)this,
                        cbZoomFunc);
    }
    //
    FUNC_END;    
    //
    return MTRUE;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
setForceRotation(
    MBOOL   bIsForceRotation,
    MUINT32 rotationAnagle)
{
    MY_LOGD("FR(%d,%d)",
            bIsForceRotation,
            rotationAnagle);
    mbIsForceRotation = bIsForceRotation;
    mRotationAnagle = rotationAnagle;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
setMode(MODE_TYPE mode)
{
    MY_LOGD("mode(%d->%d)",
            mMode,
            mode);
    //
    switch(mode)
    {
        case MODE_IDLE:
        {
            break;
        }
        case MODE_NORMAL_PREVIEW:
        case MODE_STEREO_PREVIEW:
        case MODE_ZSD_PREVIEW:
        case MODE_STEREO_ZSD_PREVIEW:
        {
            mRecordingHint = mspParamsMgr->getRecordingHint();
            break;
        }
        case MODE_NORMAL_CAPTURE:
        {
            break;
        }
        case MODE_VIDEO_PREVIEW:
        {
            if(mMode == MODE_IDLE)
            {
                mRecordingHint = mspParamsMgr->getRecordingHint();
            }
            else
            if(mMode == MODE_VIDEO_RECORD)
            {
                mpHal3a->sendCommand(ECmd_RecordingEnd);
                //
                if(mRecordingHint == MTRUE)
                {
                    MUINT32 vdoWidth = 0, vdoHeight = 0;
                    if(mspParamsMgr->getVideoStabilization())
                    {
                        vdoWidth = (MUINT32)((mPreviewSize.w*EIS_FACTOR)/100);
                        vdoHeight = (MUINT32)((mPreviewSize.h*EIS_FACTOR)/100);
                    }
                    else
                    {
                        vdoWidth = 0;
                        vdoHeight = 0;
                    }
                    mpIspSyncCtrl->setVideoSize(vdoWidth,vdoHeight);
                    mpIspSyncCtrl->calRrzoMaxZoomRatio();
                }
                else
                {
                    mode = MODE_NORMAL_PREVIEW;
                }
            }
            break;
        }
        case MODE_VIDEO_RECORD:
        {
            mpHal3a->sendCommand(ECmd_RecordingStart);
            //
            if(mRecordingHint == MTRUE)
            {
                MUINT32 vdoWidth = 0, vdoHeight = 0;
                if(mspParamsMgr->getVideoStabilization())
                {
                    if(mPreviewSize.w > mVideoSize.w)
                    {
                        vdoWidth = (MUINT32)((mPreviewSize.w*EIS_FACTOR)/100);
                        vdoHeight = (MUINT32)((mPreviewSize.h*EIS_FACTOR)/100);
                    }
                    else
                    {
                        vdoWidth = (MUINT32)((mVideoSize.w*EIS_FACTOR)/100);
                        vdoHeight = (MUINT32)((mVideoSize.h*EIS_FACTOR)/100);
                    }
                }
                else
                {
                    vdoWidth = mVideoSize.w;
                    vdoHeight = mVideoSize.h;
                }
                mpIspSyncCtrl->setVideoSize(
                                    vdoWidth,
                                    vdoHeight);
                mpIspSyncCtrl->calRrzoMaxZoomRatio();
            }
            //
            MUINT32 aaaUpdatePeriod;
            if(mPreviewMaxFps < AAA_UPDATE_BASE)
            {
                aaaUpdatePeriod = 1;
            }
            else
            {
                aaaUpdatePeriod = mPreviewMaxFps/AAA_UPDATE_BASE;
            }
            mpIspSyncCtrl->set3AUpdatePeriod(aaaUpdatePeriod);
            break;
        }
        case MODE_VIDEO_SNAPSHOT:
        {
            break;
        }
        case MODE_ZSD_CAPTRUE:
        {
            break;
        }
        default:
        {
            MY_LOGE("Un-support mode(%d)",mode);
            break;
        }
    }
    //
    mMode = mode;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
updateShotMode()
{
    Mutex::Autolock _l(mLock);
    //
    MUINT32 isFlashOn = 0, shotMode = mspParamsMgr->getShotMode();
    String8 const s8ShotMode = mspParamsMgr->getShotModeStr();
    MY_LOGD("<shot mode> %#x(%s)", shotMode, s8ShotMode.string());
    //
    mShotMode = shotMode;
    //
    if( !mpHal3a )
        return MTRUE;
    //
    isFlashOn = mpHal3a->isNeedFiringFlash(MTRUE);
    MY_LOGD("flash %d", isFlashOn);

    mbSkipPrecapture = MFALSE;
    switch(mMode)
    {
        case MODE_NORMAL_PREVIEW:
        case MODE_STEREO_PREVIEW:
            if( !isFlashOn && mShotMode == eShotMode_ContinuousShot)
            {
                // flash off, CShot: no need to perform precapture
                mbSkipPrecapture = MTRUE;
            }
            break;
        case MODE_ZSD_PREVIEW:
        case MODE_STEREO_ZSD_PREVIEW:
            if( !isFlashOn )
            {
                switch(mShotMode)
                {
                    case eShotMode_NormalShot:
                        {
                            MBOOL runMfb = IsSupportMfb(mspParamsMgr->getMultFrameBlending(), getSensorIdx());
                            if(!runMfb) {
                                // ZSD shot only works when normal shot with strobe off & MFLL/AIS off
                                mShotMode = eShotMode_ZsdShot;
                            }
                            break;
                        }
                    case eShotMode_ContinuousShot:
                        // ContinuousShot with strobe off, no need to stopPreview
                        mShotMode = eShotMode_ContinuousShotCc;
                        break;
                    default:
                        break;
                }
            }
            break;
        case MODE_VIDEO_RECORD:
            mShotMode = eShotMode_VideoSnapShot;
            break;
        default:
            break;
    };
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
DefaultCtrlNodeImpl::
getShotMode() const
{
    MY_LOGD("mShotMode(%d)",mShotMode);
    //
    return mShotMode;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
getVideoSnapshotSize(
    MINT32& width,
    MINT32& height)
{
    if( mSensorSize.w >= mVideoSize.w &&
        mSensorSize.h >= mVideoSize.h)
    {
        width = mSensorSize.w;
        height = (mSensorSize.w*mVideoSize.h)/mVideoSize.w;
    }
    else
    {
        width = mVideoSize.w;
        height = mVideoSize.h;
    }
    //
    width = ALIGN_UP_SIZE(width,2);
    height = ALIGN_UP_SIZE(height,2); 
    //
    MY_LOGD("Sensor(%dx%d),Video(%dx%d),VSS(%dx%d)",
            mSensorSize.w,
            mSensorSize.h,
            mVideoSize.w,
            mVideoSize.h,
            width,
            height);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
isRecBufSnapshot()
{
    MY_LOGD("Video(%dx%d)",
            mVideoSize.w,
            mVideoSize.h);
    //
    if( (mVideoSize.w*mVideoSize.h) > IMG_1080P_SIZE ||
        ::strcmp(mspParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0 ||
        ::strcmp(mspParamsMgr->getStr(MtkCameraParameters::KEY_3DNR_MODE), MtkCameraParameters::ON) == 0 ||
        mRecordingHint == MFALSE ||
        mpIspSyncCtrl->getHdrState() != SENSOR_VHDR_MODE_NONE)
    {
        if( ::strcmp(mspParamsMgr->getStr(MtkCameraParameters::KEY_VIDEO_FRAME_FORMAT), MtkCameraParameters::PIXEL_FORMAT_BITSTREAM) == 0 ||
            mPreviewMaxFps > 30)
        {
            return MFALSE;
        }
        else
        {
            return MTRUE;
        }
    }
    else
    {
        return MFALSE;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
waitPreviewReady()
{
    MBOOL ret;
#define START_PREVIEW_TIMEOUT (3000000000) //ns
    Mutex::Autolock _l(mLock);
    if( !getFlag(muStateFlag, FLAG_STARTED) )
    {
        MY_LOGW("not start yet");
        return MTRUE;
    }

    ret = getFlag(muStateFlag, FLAG_3A_PREVIEW_OK);
    if( !ret )
    {
        NG_TRACE_NAME("waitReadyToPreview");
        MY_LOGD("wait for start preview done +");
        mCond3A.waitRelative(mLock, START_PREVIEW_TIMEOUT);
        MY_LOGD("start preview done -");
    }
#undef START_PREVIEW_TIMEOUT

    ret = getFlag(muStateFlag, FLAG_3A_PREVIEW_OK);
    if( !ret )
    {
        MY_LOGE("wait preview ready timeout!");
    }

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
void
DefaultCtrlNodeImpl::
doNotifyCb(
    MINT32 _msgType,
    MINT32 _ext1,
    MINT32 _ext2,
    MINT32 _ext3)
{
#define is3aStatus(ext, bit) (ext & (1<<bit))
    //
    switch(_msgType)
    {
        case I3ACallBack::eID_NOTIFY_3APROC_FINISH:
            NG_TRACE_BEGIN("3Aproc");
            MY_LOGD("3APROC_FINISH:0x%08X,0x%08X",
                    _ext1,
                    _ext2);
            if(mCurMagicNum != (MUINT32)_ext1) {
                MY_LOGE("MagicNum:Cur(0x%08X) != Notify(0x%08X)",
                        mCurMagicNum,
                        _ext1);
                NG_TRACE_END();
                break;
            }

            if( is3aStatus(_ext2,I3ACallBack::e3AProcOK) ) {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            } else {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_FAIL);
            }

            if( is3aStatus(_ext2,I3ACallBack::e3APvInitReady) ) {
                updateReadyMagic(mCurMagicNum);
            }
            NG_TRACE_END();
            break;

        case I3ACallBack::eID_NOTIFY_READY2CAP:
            NG_TRACE_BEGIN("3AReadyToCap");
            MY_LOGD("ReadyToCap");
            if( !updatePrecap() ) {
                MY_LOGE("updatePrecap failed");
            }
            NG_TRACE_END();
            break;

        default:
            break;
    }
#undef is3aStatus
}


/*******************************************************************************
*
********************************************************************************/
void
DefaultCtrlNodeImpl::
doDataCb(
    MINT32  _msgType,
    void*   _data,
    MUINT32 _size)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
cbZoomFunc(
    MVOID* pObj,
    MINT32 ext1, 
    MINT32 ext2, 
    MINT32 ext3)
{
    DefaultCtrlNodeImpl* pDefaultCtrlNodeImpl = (DefaultCtrlNodeImpl*)pObj;
    pDefaultCtrlNodeImpl->doNotifyCbZoom(
                            ICallBackZoom::eID_NOTIFY_Zoom,
                            ext1,
                            ext2,
                            ext3);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
void
DefaultCtrlNodeImpl::
doNotifyCbZoom(
    MINT32 _msgType,
    MINT32 _ext1,
    MINT32 _ext2,
    MINT32 _ext3)
{
    //
    if(_msgType == ICallBackZoom::eID_NOTIFY_Zoom)
    {
        MY_LOGD("Index(%d)",_ext1);
        mpCallbackZoom->doNotifyCb(
                            _msgType,
                            _ext1,
                            _ext2,
                            _ext3);
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
updateReadyMagic(MUINT32 const magic)
{
    Mutex::Autolock _l(mLock);
    if( m3aReadyMagic == MAGIC_NUM_INVALID )
    {
        enableFlag(muStateFlag, FLAG_3A_PREVIEW_OK);
        m3aReadyMagic = magic;
        MY_LOGD("3A start preview done # 0x%X", m3aReadyMagic);
        // signal for 3A start preview done
        mCond3A.signal();
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
updateReadyBuffer(MUINT32 const dequeMagic)
{
    Mutex::Autolock _l(mLock);
    if( dequeMagic == MAGIC_NUM_INVALID )
        return MFALSE;
    
    if( !getFlag(muStateFlag, FLAG_BUF_PREVIEW_OK) )
    {
        if( m3aReadyMagic != MAGIC_NUM_INVALID && dequeMagic >= m3aReadyMagic )
        {
            enableFlag(muStateFlag, FLAG_BUF_PREVIEW_OK);
            return MTRUE;
        }
        return MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
precap()
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    if( mbSkipPrecapture )
    {
        MY_LOGD("skip precapture");
        goto lbExit;
    }

    {
        Mutex::Autolock _l(mLock);
        enableFlag(muStateFlag, FLAG_PRECAPTURE);
    }
    //
    MY_LOGD("3A precapture start");
    mpHal3a->sendCommand(ECmd_PrecaptureStart);
    {
        Mutex::Autolock _l(mLock);
        if( !mpHal3a->isReadyToCapture() ) {
            //wait for precap done
            mCond3A.wait(mLock);
        }
        disableFlag(muStateFlag, FLAG_DO_3A_UPDATE);
    }
    MY_LOGD("3A precapture end");
    mpHal3a->sendCommand(ECmd_PrecaptureEnd);
    //
    // stop pass1 immediately to reduce shutter delay
    handleNotify( CONTROL_STOP_PASS1, 0, 0 );
    //
    {
        Mutex::Autolock _l(mLock);
        disableFlag(muStateFlag, FLAG_PRECAPTURE);
    }
    //
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeImpl::
updatePrecap() const
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("singal precap");
    mCond3A.signal();
    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

