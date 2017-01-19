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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ENGPARAM_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ENGPARAM_H_


namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/

struct EngParam: public ShotParam
{
    MUINT32                         u4SensorWidth;
    MUINT32                         u4SensorHeight;
    MUINT32                         u4Bitdepth;
    MUINT32                         u4RawPixelID;
    MUINT32                         u4VHDState;
    EImageFormat                    eImgFmt;
    int32_t                         mi4EngRawSaveEn;
    int32_t                         mi4EngSensorMode;
    int32_t                         mi4EngIspMode;
    MINT32                          u4EngCaptureISO;
    MINT32                          mi4CamShotType;
    MINT32                          mi4ManualMultiNREn;
    MINT32                          mi4ManualMultiNRType;
    MINT32                          mi4MFLLnum;
    MINT32                          u4rawStride;
    MUINT32                        u4Exposuretime; 
    MUINT32                        u4SensorGain; 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    EngParam(
                                        MUINT32 const                   SensorWidth     = 0,
                                        MUINT32 const                   SensorHeight    = 0,
                                        MUINT32 const                   Bitdepth        = 0,
                                        MUINT32 const                   RawPixelID      = 0,
                                        EImageFormat const              ImgFmt          = eImgFmt_UNKNOWN,
                                        int32_t const                   i4EngRawSaveEn  = 0, 
                                        int32_t const                   i4EngSensorMode = 0,
                                        int32_t const                   i4EngIspMode    = 0,
                                        MINT32  const                   RawStride       = 0, 
                                        MUINT32 const                  _u4Exposuretime = 0, 
                                        MUINT32 const                  _u4SensorGain = 0
                                    )
                                        : u4SensorWidth(SensorWidth)
                                        , u4SensorHeight(SensorHeight)
                                        , u4Bitdepth(Bitdepth)
                                        , u4RawPixelID(RawPixelID)
                                        , eImgFmt(ImgFmt)
                                        , mi4EngRawSaveEn(i4EngRawSaveEn)
                                        , mi4EngSensorMode(i4EngSensorMode)
                                        , mi4EngIspMode(i4EngIspMode)
                                        , u4rawStride(RawStride)
                                        , u4Exposuretime(_u4Exposuretime)
                                        , u4SensorGain(_u4SensorGain)
                                    {
                                    }

                                    enum EngSensorMode { // Duplicate from IHalSensor.h (alps\mediatek\hardware\include\mtkcam\hal)
                                        ENG_SENSOR_MODE_NORMAL_PREVIEW = 0,     // SENSOR_SCENARIO_ID_NORMAL_PREVIEW = 0,
                                        ENG_SENSOR_MODE_NORMAL_CAPTURE = 1,     // SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
                                        ENG_SENSOR_MODE_NORMAL_VIDEO = 2,       // SENSOR_SCENARIO_ID_NORMAL_VIDEO,
                                        ENG_SENSOR_MODE_SLIM_VIDEO1 = 3,        // SENSOR_SCENARIO_ID_SLIM_VIDEO1,
                                        ENG_SENSOR_MODE_SLIM_VIDEO2 = 4,        // SENSOR_SCENARIO_ID_SLIM_VIDEO2,
                                        ENG_SENSOR_MODE_CUSTOM1 = 5,    // SENSOR_SCENARIO_ID_CUSTOM1,
                                        ENG_SENSOR_MODE_CUSTOM2 = 6,    // SENSOR_SCENARIO_ID_CUSTOM2
                                        ENG_SENSOR_MODE_CUSTOM3 = 7 ,   // SENSOR_SCENARIO_ID_CUSTOM3
                                        ENG_SENSOR_MODE_CUSTOM4 = 8,    // SENSOR_SCENARIO_ID_CUSTOM4
                                        ENG_SENSOR_MODE_CUSTOM5 = 9,    // SENSOR_SCENARIO_ID_CUSTOM5                                    
                                        ENG_SENSOR_MODE_ENUM,
                                    };

                                    enum EngIspMode { // for Engineer Mode App (written in Java)
                                        ENG_ISP_MODE_PROCESSED_RAW = '0',
                                        ENG_ISP_MODE_PURE_RAW = '1',
                                        ENG_ISP_MODE_ENUM,
                                    };

                                    enum EngRawType { // for Cam IO
                                        ENG_RAW_TYPE_PURE_RAW = 0, // 0: pure raw
                                        ENG_RAW_TYPE_PROCESSED_RAW = 1, // 1: pre-process raw
                                        ENG_RAW_TYPE_FORCE_PROCESSED_RAW = 3,    // 3: force pre-processed raw, depend on singleShot
                                        ENG_RAW_TYPE_ENUM,
                                    };
                                    
                                    enum EngCamShotType { // CCT MFLL
                                        ENG_CAMSHOT_TYPE_SINGLE = 0,    // 0: single shot, default value
                                        ENG_CAMSHOT_TYPE_MFLL = 1,      // 1: MFLL shot
                                        ENG_CAMSHOT_TYPE_ENUM,
                                    };
};

} // namespace android
} // namespace NSShot
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ENGPARAM_H_

