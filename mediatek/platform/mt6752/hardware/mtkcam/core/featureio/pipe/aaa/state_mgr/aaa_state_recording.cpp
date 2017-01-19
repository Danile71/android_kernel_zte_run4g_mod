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
#define LOG_TAG "aaa_state_recording"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <mtkcam/core/featureio/pipe/aaa/aaa_hal.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr/aaa_state.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr/aaa_state_mgr.h>

#include <awb_mgr_if.h>
#include <aao_buf_mgr.h>


using namespace NS3A;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateRecording
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateRecording::
StateRecording(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateRecording", sensorDevId, pStateMgr)
{
}


MRESULT StateRecording::exitPreview()
{
	MY_LOG("StateRecording::exitPreview line=%d",__LINE__);

    // AAO DMA uninit
    if (!IAAOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;    
    }

    // AWB uninit
    if (!IAwbMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAwbMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }

    m_pStateMgr->transitState(eState_Recording, eState_Init);

    return  S_3A_OK;
}


MRESULT
StateRecording::
sendIntent(intent2type<eIntent_CamcorderPreviewEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_CamcorderPreviewEnd> line=%d", __LINE__);
	exitPreview();
	/*NeedUpdate*///FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId, 0);
    return  S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_RecordingStart>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_RecordingStart>");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_RecordingEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_RecordingEndd>");

    // State transition: eState_Recording --> eState_CamcorderPreview
    m_pStateMgr->transitState(eState_Recording, eState_CamcorderPreview);

    //if(FlashMgr::getInstance()->getFlashMode()==LIB3A_FLASH_MODE_AUTO)
    //if(FlashMgr::getInstance()->getFlashMode()!=LIB3A_FLASH_MODE_FORCE_TORCH)
    //	FlashMgr::getInstance()->setAFLampOnOff(0);
	/*NeedUpdate*///FlashMgr::getInstance().videoRecordingEnd(m_SensorDevId);
	/*NeedUpdate*///FlickerHalBase::getInstance().recordingEnd(m_SensorDevId);


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_VsyncUpdate>");

    // Update frame count
    m_pStateMgr->updateFrameCount();

    // Dequeue AAO DMA buffer
    BufInfo_T rBufInfo;
    IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);

    // AWB
    MINT32 i4SceneLv = 80; // FIXME: IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
    MBOOL bAEStable = MTRUE;//FIXME: IAeMgr::getInstance().IsAEStable(m_SensorDevId)
    IAwbMgr::getInstance().doPvAWB(m_SensorDevId, m_pStateMgr->getFrameCount(), bAEStable, i4SceneLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

    // Enqueue AAO DMA buffer
    IAAOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);

    // Update AAO DMA base address for next frame
    IAAOBufMgr::getInstance().updateDMABaseAddr(m_SensorDevId);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFUpdate>");
#if ENABLE_3A_CODE_BODY /*NeedUpdate*/
    BufInfo_T rBufInfo;

    // Dequeue AFO DMA buffer
    IAFOBufMgr::getInstance().dequeueHwBuf(1/*sensordev*/, rBufInfo);

    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_SensorDevId, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);     // Profiling End.

    // Enqueue AFO DMA buffer
    IAFOBufMgr::getInstance().enqueueHwBuf(1/*sensordev*/, rBufInfo);
#endif /*NeedUpdate*/
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFStart>");
#if ENABLE_3A_CODE_BODY /*NeedUpdate*/
    // Init
    if(IAeMgr::getInstance().IsDoAEInPreAF(m_SensorDevId) == MTRUE)   {
        MY_LOG("Enter PreAF state");
        transitAFState(eAFState_PreAF);
    }
    else   {
        MY_LOG("Enter AF state");
        IAfMgr::getInstance().triggerAF(m_SensorDevId);
        transitAFState(eAFState_AF);
    }

    // State transition: eState_CameraPreview --> eState_AF
    //m_pStateMgr->transitState(eState_CameraPreview, eState_AF);
#endif /*NeedUpdate*/
    m_pStateMgr->transitState(eState_Recording, eState_AF);



    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFEnd>");

    return  S_3A_OK;
}

