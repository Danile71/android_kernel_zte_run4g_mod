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

public final class GloList {
    static final String[] ALLGLO = {
        "sheep_eat.glo", // 0
        "sheep_sleep.glo",
        "sheep_walk.glo",
        "tree_bend_gentle.glo",
        "tree_bend_moderate.glo",
        "tree_bend_gail.glo",  // 5
        "leaves_blow_gentle.glo",
        "leaves_blow_moderate.glo",
        "leaves_blow_heavy.glo",
        "leaves_blow_gail.glo",
        "leaves_blow_gentle_snow.glo", // 10
        "leaves_blow_gail_snow.glo",
        "leaves_sandy.glo",
        "cloudy.glo",
        "clouds_show_hide_light.glo",
        "clouds_show_hide_heavy_bright.glo", // 15
        "clouds_show_hide_heavy_dark.glo",
        "sandy_clouds.glo",
        "rain_fall.glo",
        "rain_light.glo",
        "drizzle.glo", // 20
        "drizzle_long.glo",
        "thunder.glo",
        "thunder_light.glo",
        "snow_small.glo",
        "snow_heavy.glo", // 25
        "snow_rain.glo",
        "snow_drizzle.glo",
        "fog1.glo",
        "sand1.glo" // 29
    };

    static final int SHEEP_EAT = 0;
    static final int SHEEP_SLEEP = 1;
    static final int SHEEP_WALK = 2;
    static final int TREE_BEND_GENTLE = 3;
    static final int TREE_BEND_MODERATE = 4;
    static final int TREE_BEND_GAIL = 5;
    static final int LEAVES_BLOW_GENTLE = 6;
    static final int LEAVES_BLOW_MODERATE = 7;
    static final int LEAVES_BLOW_HEAVY = 8;
    static final int LEAVES_BLOW_GAIL = 9;
    static final int LEAVES_BLOW_GENTLE_SNOW = 10;
    static final int LEAVES_BLOW_GAIL_SNOW = 11;
    static final int LEAVES_SANDY = 12;
    static final int CLOUDY_GLO = 13;
    static final int CLOUDS_SHOW_HIDE_LIGHT = 14;
    static final int CLOUDS_SHOW_HIDE_HEAVY_BRIGHT = 15;
    static final int CLOUDS_SHOW_HIDE_HEAVY_DARK = 16;
    static final int SANDY_CLOUDS = 17;
    static final int RAIN_FAIL = 18;
    static final int RAIN_LIGHT = 19;
    static final int DRIZZLE = 20;
    static final int DRIZZLE_LONG = 21;
    static final int THUNDER_GLO = 22;
    static final int THUNDER_LIGHT = 23;
    static final int SNOW_SMALL = 24;
    static final int SNOW_HEAVY = 25;
    static final int SNOW_RAIN_GLO = 26;
    static final int SNOW_DRIZZLE = 27;
    static final int FOG1 = 28;
    static final int SAND1 = 29;

    static final int[] SUNNY = {SHEEP_EAT, TREE_BEND_GENTLE, LEAVES_BLOW_GENTLE};
    static final int[] WINDY = {SHEEP_SLEEP, TREE_BEND_MODERATE, LEAVES_BLOW_MODERATE};
    static final int[] BLUSTERY = {SHEEP_WALK, TREE_BEND_GAIL, LEAVES_BLOW_HEAVY, CLOUDS_SHOW_HIDE_LIGHT};
    static final int[] CLOUDY = {SHEEP_EAT, TREE_BEND_GENTLE, LEAVES_BLOW_GENTLE, CLOUDY_GLO};
    static final int[] RAINY = {SHEEP_SLEEP, TREE_BEND_GAIL, LEAVES_BLOW_GAIL, CLOUDS_SHOW_HIDE_HEAVY_BRIGHT,
            RAIN_FAIL, RAIN_LIGHT};
    static final int[] HEAVY_RAIN = {SHEEP_WALK, TREE_BEND_GAIL, LEAVES_BLOW_GAIL, CLOUDS_SHOW_HIDE_HEAVY_BRIGHT,
            RAIN_FAIL, DRIZZLE, DRIZZLE_LONG, RAIN_LIGHT};
    static final int[] THUNDER = {SHEEP_EAT, TREE_BEND_GAIL, LEAVES_BLOW_GAIL, CLOUDS_SHOW_HIDE_HEAVY_DARK, RAIN_FAIL,
            DRIZZLE, THUNDER_GLO, THUNDER_LIGHT};
    static final int[] SNOW = {SHEEP_SLEEP, TREE_BEND_GENTLE, LEAVES_BLOW_GENTLE_SNOW, SNOW_SMALL};
    static final int[] HEAVY_SNOW = {SHEEP_WALK, TREE_BEND_GAIL, LEAVES_BLOW_GAIL_SNOW, SNOW_HEAVY};
    static final int[] SNOW_RAIN = {SHEEP_EAT, TREE_BEND_GAIL, LEAVES_BLOW_GAIL_SNOW, CLOUDS_SHOW_HIDE_HEAVY_BRIGHT,
            SNOW_RAIN_GLO, SNOW_DRIZZLE, SNOW_SMALL, RAIN_LIGHT};
    static final int[] FOG = {SHEEP_SLEEP, TREE_BEND_GENTLE, LEAVES_BLOW_GENTLE, FOG1};
    static final int[] SAND = {SHEEP_WALK, TREE_BEND_GENTLE, LEAVES_SANDY, SANDY_CLOUDS, SAND1};
}