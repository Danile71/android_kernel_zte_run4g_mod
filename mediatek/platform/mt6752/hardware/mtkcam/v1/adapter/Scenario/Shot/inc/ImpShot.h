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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_INC_IMPSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_INC_IMPSHOT_H_

#include <mtkcam/camshot/CapBufMgr.h>

class IDbgInfoContainer;

namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *  Implement Shot Class
 ******************************************************************************/
class ICamBufHandler;
class ImpShot : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Attributes.

    String8 const                   ms8ShotName;        //  shot name
    uint32_t const                  mu4ShotMode;        //  shot mode (defined in EShotMode)
    int32_t const                   mi4OpenId;          //  open id: 0/1/2

    sp<IShotCallback>               mpShotCallback;     //  pointer to IShotCallback.

    ShotParam                       mShotParam;         //  Shot parameters.
    JpegParam                       mJpegParam;         //  Jpeg parameters.
    ICamBufHandler*                 mpPrvBufHandler;
    NSCamShot::CapBufMgr*           mpCapBufMgr;
    NSCamShot::CapBufMgr*           mpCapBufMgr_Main2;  //  for Stereo Main2
    int32_t                         mi4DataMsgSet;

public:     ////                    Attributes.
    inline char const*              getShotName() const     { return ms8ShotName.string(); }
    inline uint32_t                 getShotMode() const     { return mu4ShotMode; }
    inline int32_t                  getOpenId() const       { return mi4OpenId; }

    //  Set Shot parameters.
    virtual bool                    setShotParam(void const* pParam, size_t const size);

    //  Set Jpeg parameters.
    virtual bool                    setJpegParam(void const* pParam, size_t const size);

    //  Set Preview buf handler
    virtual bool                    setPrvBufHandler(void const* pParam, size_t const size);

    //  Set CapBufMgr
    virtual bool                    setCapBufMgr(void const* pParam_0, void const* pParam_1, size_t const size);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~ImpShot();
                                    ImpShot(
                                        char const*const pszShotName, 
                                        uint32_t const u4ShotMode, 
                                        int32_t const i4OpenId
                                    );

public:     ////                    Operations.

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void                    onDestroy()                             = 0;

    //  Set callbacks.
    virtual bool                    setCallback(sp<IShotCallback>& rpShotCallback);

    virtual bool                    sendCommand(
                                        uint32_t const  cmd, 
                                        uint32_t const  arg1, 
                                        uint32_t const  arg2,
                                        uint32_t const  arg3 = 0
                                    );

    virtual bool   isDataMsgEnabled(MINT32 const i4MsgTypes);
   
    virtual void   enableDataMsg(MINT32 const i4MsgTypes);
   
    virtual void   disableDataMsg(MINT32 const i4MsgTypes);   

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    
    virtual bool      makeExifHeader(MUINT32 const u4CamMode, 
                                        MUINT8* const puThumbBuf, 
                                        MUINT32 const u4ThumbSize, 
                                        MUINT8* puExifBuf, 
                                        MUINT32 &u4FinalExifSize, 
                                        IDbgInfoContainer* pDbg = NULL,
                                        MUINT32 u4ImgIndex = 0, 
                                        MUINT32 u4GroupId = 0,
                                        MUINT32 u4FocusValH = 0,
                                        MUINT32 u4FocusValL = 0 
                                        ); 

    // for Stereo EXIF <pDbg_0 for 1st camera, pDbg_1 for 2nd(minor) camera>
    virtual bool      makeExifHeader(MUINT32 const u4CamMode, 
                                        MUINT8* const puThumbBuf, 
                                        MUINT32 const u4ThumbSize, 
                                        MUINT8* puExifBuf, 
                                        MUINT32 &u4FinalExifSize, 
                                        IDbgInfoContainer* pDbg_0,
                                        IDbgInfoContainer* pDbg_1
                                        ); 

    // NR type <0:none, 1:hwnr, 2:swnr>
    virtual MUINT32 queryCapNRType(
                        MUINT32 currentIso,
                        MBOOL usemfll = MFALSE
                        ) const;

    virtual MUINT32 getPreviewIso() const;
    virtual MUINT32 getCaptureIso() const;

};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_INC_IMPSHOT_H_

