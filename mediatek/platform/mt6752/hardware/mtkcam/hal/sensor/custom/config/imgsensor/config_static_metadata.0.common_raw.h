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


STATIC_METADATA_BEGIN(0, COMMON_RAW)
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION)
        CONFIG_ENTRY_VALUE(MRect(MPoint(0, 0), MSize(3200, 2400)));
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT)
        CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_BGGR);
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PHYSICAL_SIZE)
        CONFIG_ENTRY_VALUE(3.20f);      // millimeters
        CONFIG_ENTRY_VALUE(2.40f);      // millimeters
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE)
        CONFIG_ENTRY_VALUE(MSize(3200, 2400));
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_WHITE_LEVEL)
        CONFIG_ENTRY_VALUE(4000);
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PACKAGE)
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_ZSD);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(15);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(0, 0), MSize(3200, 2400)));
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(3200, 2400));
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_PREVIEW);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(30);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(0, 0), MSize(3200, 2400)));
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(1600, 1200));
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_CAPTURE);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(15);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(0, 0), MSize(3200, 2400)));
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(3200, 2400));
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_VIDEO);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(30);
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(540, 405), MSize(2120, 1590)));
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(2120, 1590));
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
STATIC_METADATA_END()

