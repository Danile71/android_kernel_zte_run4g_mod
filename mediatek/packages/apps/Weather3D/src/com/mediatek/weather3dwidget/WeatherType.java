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

public final class WeatherType {
    static class Type {
        // sunny
        public static final int SUNNY = 1;
        // windy
        public static final int WINDY = 2;
        // strong wind
        public static final int BLUSTERY = 3;
        public static final int TORNADO = 4;
        // cloudy
        public static final int CLOUDY = 5;
        public static final int OVERCAST = 6;
        // rain
        public static final int SHOWER = 7;
        public static final int RAIN = 8;
        // heavy rain
        public static final int DOWNPOUR = 9;
        public static final int HAIL = 10;
        // thunder shower
        public static final int THUNDER_SHOWER = 11;
        public static final int THUNDER_STORM = 12;
        public static final int THUNDER_HAIL = 13;
        // snow
        public static final int SNOW_SHOWER = 14;
        public static final int SNOW_LIGHT = 15;
        public static final int SNOW = 16;
        //heavy snow
        public static final int HEAVY_SNOW = 17;
        // snow rain
        public static final int SLEET = 18;
        // fog
        public static final int FOG = 19;
        // sand
        public static final int DUST = 20;
        public static final int SAND = 21;      
    }
    
    private static final int[] WEATHER_ICON = {
    //  0,                            1,                          2,                          3,
        R.drawable.ic_sunny,          R.drawable.ic_sunny,        R.drawable.ic_windy,        R.drawable.ic_blustery,
        R.drawable.ic_tornado,        R.drawable.ic_cloudy,       R.drawable.ic_overcast,     R.drawable.ic_showers,
        R.drawable.ic_rain,           R.drawable.ic_downpour,     R.drawable.ic_hail,         R.drawable.ic_thunder_shower,
        R.drawable.ic_thunder_storms, R.drawable.ic_thunder_hail, R.drawable.ic_snow_showers, R.drawable.ic_snow_light,
        R.drawable.ic_snow,           R.drawable.ic_snow_heavy,   R.drawable.ic_sleet,        R.drawable.ic_fog,
        R.drawable.ic_sandy,          R.drawable.ic_sandy};
    //  20,                           21

    static class ModelType {
        public static final int SUNNY = 1;
        public static final int WINDY = 2;
        public static final int BLUSTERY = 3;
        public static final int CLOUDY = 4;
        public static final int RAIN = 5;
        public static final int HEAVY_RAIN = 6;
        public static final int THUNDER = 7;
        public static final int SNOW = 8;
        public static final int HEAVY_SNOW = 9;
        public static final int SNOW_RAIN = 10;
        public static final int FOG = 11;
        public static final int SAND = 12;

        public static final int INDEX_MIN = SUNNY;
        public static final int INDEX_MAX = SAND;
    }

    private static final int[] MODE_TYPE = {
    //  0,                    1,                 2,                    3,                   4,
        ModelType.SUNNY,      ModelType.SUNNY,   ModelType.WINDY,      ModelType.BLUSTERY,  ModelType.BLUSTERY,
        ModelType.CLOUDY,     ModelType.CLOUDY,  ModelType.RAIN,       ModelType.RAIN,      ModelType.HEAVY_RAIN,
        ModelType.HEAVY_RAIN, ModelType.THUNDER, ModelType.THUNDER,    ModelType.THUNDER,   ModelType.SNOW,
        ModelType.SNOW,       ModelType.SNOW,    ModelType.HEAVY_SNOW, ModelType.SNOW_RAIN, ModelType.FOG,
        ModelType.SAND,       ModelType.SAND};
    //  20,                   21

    private WeatherType() {}

    public static int convertToModelType(int type) {
        return MODE_TYPE[type];
    }

    public static int getWeatherIcon(int type) {
        return WEATHER_ICON[type];
    }

    public static boolean isSunMoonNeededModelType(int modelType) {
        return !(modelType == ModelType.RAIN || modelType == ModelType.HEAVY_RAIN || modelType == ModelType.THUNDER ||
                modelType == ModelType.SNOW_RAIN);
    }

    public static boolean isSnowModelType(int modelType) {
        return (modelType == ModelType.SNOW || modelType == ModelType.HEAVY_SNOW || modelType == ModelType.SNOW_RAIN);
    }

    public static boolean isSandModelType(int modelType) {
        return (modelType == ModelType.SAND);
    }

    public static boolean isModelTypeInRange(int modelType) {
        return (modelType >= ModelType.INDEX_MIN && modelType <= ModelType.INDEX_MAX);
    }
}