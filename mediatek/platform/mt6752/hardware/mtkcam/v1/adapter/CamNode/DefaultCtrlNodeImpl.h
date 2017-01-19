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
#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_DEFAULT_CTRL_NODES_IMPL_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_DEFAULT_CTRL_NODES_IMPL_H_
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/camnode/IspSyncControl.h>
using namespace NSCamNode;
//
#include <Callback/ICallBackZoom.h>
//
#include <cutils/atomic.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <mtkcam/featureio/vhdr_hal.h>
#include "mtkcam/featureio/eis_hal.h"
//
#include <CamNode/DefaultCtrlNode.h>
#include <CamNode/RawDumpCmdQueThread.h>
//
#define EIS_TEST    (1)
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s:%s]"fmt, getSensorIdx(), getName(), __func__, ##arg)
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
                                    printf("[%s/%s] "fmt"\n", LOG_TAG,__func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt,  __func__, ##arg); \
                                    printf("[%s/%s] "fmt"\n", LOG_TAG,__func__, ##arg)
#endif
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)
//
#define enableFlag(val, flag)   do{ val |= (flag); }while(0)
#define disableFlag(val, flag)  do{ val &= ~(flag); }while(0)
#define getFlag(val, flag)      (val&(flag))
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class DefaultCtrlNodeImpl : public DefaultCtrlNode
                                 , public I3ACallBack
{
    public:
        #define ALIGN_UP_SIZE(in,align)     ((in+align-1) & ~(align-1))
        #define ALIGN_DOWN_SIZE(in,align)   (in & ~(align-1))
        #define EIS_MIN_WIDTH               ((1280*EIS_FACTOR)/100)
        #define EIS_MAX_FPS                 (30)
        #define AAA_UPDATE_BASE             (30)

        enum
        {
            FLAG_STARTED        = 0x1,
            FLAG_DO_3A_UPDATE   = 0x2,
            FLAG_3A_PREVIEW_OK  = 0x4,
            FLAG_BUF_PREVIEW_OK = 0x8,
            FLAG_PRECAPTURE     = 0x10,
            FLAG_ALL            = 0x1F,
        };

    public: // ctor & dtor
        DefaultCtrlNodeImpl(const char* userName);
        ~DefaultCtrlNodeImpl();
        //
        DECLARE_ICAMNODE_INTERFACES();
        virtual MBOOL setConfig(CONFIG_INFO configInfo);
        virtual MBOOL updateDelay(EQueryType_T const type);
        virtual MBOOL setParameters();
        virtual MBOOL startSmoothZoom(MINT32 arg1);
        virtual MBOOL stopSmoothZoom();
        virtual MBOOL precap();
        virtual MBOOL setForceRotation(
                        MBOOL   bIsForceRotation,
                        MUINT32 rotationAnagle);
        virtual MBOOL setMode(MODE_TYPE mode);
        virtual MBOOL updateShotMode();
        virtual MUINT32 getShotMode() const;
        virtual MBOOL getVideoSnapshotSize(
                            MINT32& width,
                            MINT32& height);
        virtual MBOOL isRecBufSnapshot();
        virtual MBOOL waitPreviewReady();
        //I3ACallBack
        virtual void doNotifyCb(
                        MINT32 _msgType,
                        MINT32 _ext1,
                        MINT32 _ext2,
                        MINT32 _ext3);
        
        virtual void doDataCb(
                        MINT32 _msgType,
                        void*   _data,
                        MUINT32 _size);
        //
        static MBOOL cbZoomFunc(
                            MVOID* pObj,
                            MINT32 ext1, 
                            MINT32 ext2, 
                            MINT32 ext3);
        virtual void doNotifyCbZoom(
                        MINT32 _msgType,
                        MINT32 _ext1,
                        MINT32 _ext2,
                        MINT32 _ext3);

    //
    public: // operations
        const char* getName() const {return msName;}
    //
    private:
        MBOOL         updateReadyMagic(MUINT32 const magic);
    protected:
        MBOOL         updateReadyBuffer(MUINT32 const dequeMagic);
    private:
        MBOOL         updatePrecap() const;
    //
    protected:
        MUINT32             muStateFlag;
        MINT32              mDropFrameCnt; //TODO: to be removed
        MUINT32             m3aReadyMagic;
        MSize               mPreviewSize;
        MSize               mVideoSize;
        MSize               mSensorSize;
        IHal3A*             mpHal3a;
        ICallBackZoom*      mpCallbackZoom;
        sp<IParamsManager>  mspParamsMgr;
        sp<CamMsgCbInfo>    mspCamMsgCbInfo;
        CapBufMgr*          mpCapBufMgr;
        IspSyncControl*     mpIspSyncCtrl;
        VHdrHal*            mpVHdrHal;
        EisHal*             mpEisHal;
        EIS_SCENARIO_ENUM   mEisScenario;
        const char* const   msName;
        MBOOL               mbIsForceRotation;
        MUINT32             mRotationAnagle;
        mutable Mutex       mLock;
        mutable Condition   mCond3A;
        MBOOL               mbSkipPrecapture;
        MBOOL               mRecordingHint;
        MBOOL               mbUpdateEis;
        MBOOL               mbUpdateVHdr;
        MUINT32             mSensorType;
        MUINT32             mSensorScenario;
        MUINT32             mReplaceBufNumResize;
        MUINT32             mReplaceBufNumFullsize;
        MUINT32             mRollbackBufNum;
        MUINT32             mRollbackBufPeriod;
        MUINT32             mRollbackBufCnt;
        MUINT32             mPreviewMaxFps;
        MUINT32             mShotMode;
        MODE_TYPE           mMode;
        EIspProfile_T       mIspProfile;
        //
        MUINT32             mCurMagicNum;
};


class DefaultCtrlNodeEngImpl : public DefaultCtrlNodeImpl
{
    public:
        DefaultCtrlNodeEngImpl(const char* userName);
        ~DefaultCtrlNodeEngImpl();
        virtual MBOOL onInit();
        virtual MBOOL onUninit();
        virtual MBOOL onNotify(
                MUINT32 const datamsg,
                MUINT32 const ext1,
                MUINT32 const ext2
                );
        virtual MBOOL onPostBuffer(
                MUINT32 const data,
                MUINT32 const buf,
                MUINT32 const ext
                );
    //
    public:
    //
    protected:
    //
    private:
        MUINT32 mu4MaxRawDumpCnt;     
        sp<IRawDumpCmdQueThread>        mpRawDumpCmdQueThread; 
    //
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif

