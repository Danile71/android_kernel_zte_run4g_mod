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

package com.mediatek.weather3dwidget;

public final class SnowTextureList {
    static final int SNOW_PILE_000 =  0;
    static final int SNOW_PILE_005 =  1;
    /*  the following is intentionally left for mapping.
        static final int SNOW_PILE_010 =  2;
        static final int SNOW_PILE_015 =  3;
        static final int SNOW_PILE_020 =  4;
        static final int SNOW_PILE_025 =  5;
        static final int SNOW_PILE_030 =  6;
        static final int SNOW_PILE_035 =  7;
        static final int SNOW_PILE_040 =  8;
        static final int SNOW_PILE_045 =  9;
        static final int SNOW_PILE_050 = 10;
        static final int SNOW_PILE_055 = 11;
        static final int SNOW_PILE_060 = 12;
        static final int SNOW_PILE_065 = 13;
        static final int SNOW_PILE_070 = 14;
        static final int SNOW_PILE_075 = 15;
        static final int SNOW_PILE_080 = 16;
        static final int SNOW_PILE_085 = 17;
        static final int SNOW_PILE_090 = 18;
        static final int SNOW_PILE_095 = 19;
        static final int SNOW_PILE_100 = 20;
        static final int SNOW_PILE_105 = 21;
        static final int SNOW_PILE_110 = 22;
        static final int SNOW_PILE_115 = 23;
        static final int SNOW_PILE_120 = 24; */

    static final int SNOW_ANI_TOTAL_DURATION = 12000;

    static final int[] TEXTURE_TIME = {
            0,   500,  1000,  1500,  2000,    //  0
         2500,  3000,  3500,  4000,  4500,    //  5
         5000,  5500,  6000,  6500,  7000,    // 10
         7500,  8000,  8500,  9000,  9500,    // 15
        10000, 10500, 11000, 11500, 11800     // 20
    };

    static final String[] TEXTURE_MAP = {
        "snow_dh_00000.jpg", "snow_dh_00005.jpg", "snow_dh_00010.jpg", "snow_dh_00015.jpg", "snow_dh_00020.jpg",    //  0
        "snow_dh_00025.jpg", "snow_dh_00030.jpg", "snow_dh_00035.jpg", "snow_dh_00040.jpg", "snow_dh_00045.jpg",    //  5
        "snow_dh_00050.jpg", "snow_dh_00055.jpg", "snow_dh_00060.jpg", "snow_dh_00065.jpg", "snow_dh_00070.jpg",    // 10
        "snow_dh_00075.jpg", "snow_dh_00080.jpg", "snow_dh_00085.jpg", "snow_dh_00090.jpg", "snow_dh_00095.jpg",    // 15
        "snow_dh_00100.jpg", "snow_dh_00105.jpg", "snow_dh_00110.jpg", "snow_dh_00115.jpg", "snow_dh_00120.jpg"     // 20
    };

    private SnowTextureList() {}
}
