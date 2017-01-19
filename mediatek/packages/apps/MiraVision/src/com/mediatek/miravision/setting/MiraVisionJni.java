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

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.miravision.setting;

import com.mediatek.miravision.ui.AalSettingsFragment;
import com.mediatek.miravision.ui.OverDriveFragment;
import com.mediatek.miravision.utils.Utils;

import android.content.ContentResolver;
import android.content.Context;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;

/**
 * Collection of utility functions used in this package.
 */
public class MiraVisionJni {

    public static final String TAG = "MiraVisionJni";
    public static final int PIC_MODE_STANDARD = 0;
    public static final int PIC_MODE_VIVID = 1;
    public static final int PIC_MODE_USER_DEF = 2;
    static boolean sLibStatus = true;

    public static class Range {
        public int min;
        public int max;
        public int defaultValue;

        public Range() {
            set(0, 0, 0);
        }

        public void set(int min, int max, int defaultValue) {
            this.min = min;
            this.max = max;
            this.defaultValue = defaultValue;
        }
    }

    private MiraVisionJni() {
    }

    static {
        try {
            Log.v("MiraVisionJni", "loadLibrary");
            System.loadLibrary("MiraVision_jni");
        } catch (UnsatisfiedLinkError e) {
            Log.e("MiraVisionJni", "UnsatisfiedLinkError");
            sLibStatus = false;
        }
    }

    public static boolean getLibStatus() {
        return sLibStatus;
    }

    boolean[] getFeaturesStatus(int isEnable) {
        boolean[] featuresStatus = { true, true, true, true, true, true, true, true, true };
        try {
            nativeEnablePQColor(isEnable);
        } catch (Exception e) {
            featuresStatus[0] = false;
        }
        return featuresStatus;
    }

    // enable PQ COLOR: 0 is disable, 1 is enable
    public static native boolean nativeEnablePQColor(int isEnable);

    // Picture Mode: STANDARD / VIVID / USER_DEF
    public static native int nativeGetPictureMode();

    // Picture Mode: STANDARD / VIVID / USER_DEF
    public static native boolean nativeSetPictureMode(int mode);

    // PQ COLOR ROI.
    public static native boolean nativeSetPQColorRegion(int isEnable, int startX, int startY,
            int endX, int endY);

    // Contrast
    private static native void nativeGetContrastIndexRange(Range r);
    public static Range getContrastIndexRange() {
        Range r = new Range();
        nativeGetContrastIndexRange(r);
        return r;
    }

	private static native int nativeGetContrastIndex();
    public static int getContrastIndex() {
		return nativeGetContrastIndex();
    }

    private static native void nativeSetContrastIndex(int index);
	public static void setContrastIndex(int index) {
		nativeSetContrastIndex(index);
    }

    // Saturation
    private static native void nativeGetSaturationIndexRange(Range r);
    public static Range getSaturationIndexRange() {
        Range r = new Range();
        nativeGetSaturationIndexRange(r);
        return r;
    }

    private static native int nativeGetSaturationIndex();
    public static int getSaturationIndex() {
		return nativeGetSaturationIndex();
    }

    private static native void nativeSetSaturationIndex(int index);
	public static void setSaturationIndex(int index) {
		nativeSetSaturationIndex(index);
    }

    // PicBrightness
    private static native void nativeGetPicBrightnessIndexRange(Range r);
	public static Range getPicBrightnessIndexRange() {
        Range r = new Range();
        nativeGetPicBrightnessIndexRange(r);
        return r;
    }

    private static native int nativeGetPicBrightnessIndex();
    public static int getPicBrightnessIndex() {
		return nativeGetPicBrightnessIndex();
    }

    private static native void nativeSetPicBrightnessIndex(int index);
	public static void setPicBrightnessIndex(int index) {
		nativeSetPicBrightnessIndex(index);
    }

    // Sharpness
	public static native boolean nativeSetTuningMode(int mode);

    private static native void nativeGetSharpnessIndexRange(Range r);
    public static Range getSharpnessIndexRange() {
        Range r = new Range();
        nativeGetSharpnessIndexRange(r);
        return r;
    }

    private static native int nativeGetSharpnessIndex();
    public static int getSharpnessIndex() {
		return nativeGetSharpnessIndex();
    }

    private static native void nativeSetSharpnessIndex(int index);
	public static void setSharpnessIndex(int index) {
		nativeSetSharpnessIndex(index);
    }

    // Dynamic Contrast: 0 is disable, 1 is enable
    private static native void nativeGetDynamicContrastIndexRange(Range r);
    public static Range getDynamicContrastIndexRange() {
        Range r = new Range();
        nativeGetDynamicContrastIndexRange(r);
        return r;
    }

    private static native int nativeGetDynamicContrastIndex();
    public static int getDynamicContrastIndex() {
		return nativeGetDynamicContrastIndex();
    }

    private static native void nativeSetDynamicContrastIndex(int index);
	public static void setDynamicContrastIndex(int index) {
		nativeSetDynamicContrastIndex(index);
    }

    // enable OD Demo: 0 is disable, 1 is enable
    public static native boolean nativeEnableODDemo(int isEnable);



    private static final String GAMMA_INDEX_PROPERTY_NAME = "persist.sys.gamma.index";

    private static native void nativeGetGammaIndexRange(Range r);

    /**
     * Get index range of gamma.
     * The valid gamma index is in [Range.min, Range.max].
     *
     * @see getGammaIndex, setGammaIndex
     */
    public static Range getGammaIndexRange() {
        Range r = new Range();
        nativeGetGammaIndexRange(r);
        return r;
    }

    /**
     * Set gamma index.
     * The index value should be in [Range.min, Range.max].
     *
     * @see getGammaIndexRange, getGammaIndex
     */
    public static void setGammaIndex(int index) {
        SystemProperties.set(GAMMA_INDEX_PROPERTY_NAME, Integer.toString(index));
        nativeSetGammaIndex(index);
    }

    /**
     * Get current gamma index setting.
     *
     * @see setGammaIndex
     */
    public static int getGammaIndex() {
        return SystemProperties
                .getInt(GAMMA_INDEX_PROPERTY_NAME, getGammaIndexRange().defaultValue);
    }

    private static native void nativeSetGammaIndex(int index);

    private static native void nativeSetUserBrightness(int level);

    /**
     * Set AAL function by bit-wise control code. Value can be one of
     * combination of Settings.System.AAL_FUNC_LABC, Settings.AAL_FUNC_CABC &
     * Settings.AAL_FUNC_DRE. For example:
     * MiraVisionJni.setAALFunction(Settings.System.AAL_FUNC_LABC |
     * Settings.System.AAL_FUNC_DRE) to enable LABC & DRE but disable CABC.
     *
     * Please get current setting from
     * Settings.System.SCREEN_BRIGHTNESS_AAL_FUNCTION. Before calling this
     * function to modify AAL parameter, also write the value into
     * Settings.System.SCREEN_BRIGHTNESS_AAL_FUNCTION by caller.
     *
     * @see Settings.System.setAALFunction()
     */
    public static void setAALFunction(int func) {
        /*
         * PowerManagerService observes the content of
         * Settings.System.SCREEN_BRIGHTNESS_AAL_FUNCTION, so we don't need to
         * pass the setting to AALService here. Once the setting changed, PMS
         * will know and execute the power state update process, hence the
         * DisplayPowerController will get the update of AAL function and write
         * to AALService.
         */
    }

    private static native void nativeGetUserBrightnessRange(Range r);

    public static Range getUserBrightnessRange() {
        Range r = new Range();
        nativeGetUserBrightnessRange(r);
        return r;
    }

    /**
     * Screen brightness of auto mode. Level should be in [USER_BRIGHTNESS_MIN,
     * USER_BRIGHTNESS_MAX]. Please get current setting from
     * Settings.System.SCREEN_BRIGHTNESS_OF_AUTO_MODE. Before calling this
     * function to modify AAL parameter, also write the value into
     * Settings.System.SCREEN_BRIGHTNESS_OF_AUTO_MODE by caller.
     */
    public static void setUserBrightness(int level) {
        nativeSetUserBrightness(level);
    }

    public static void resetPQ(Context context) {
        Log.d(TAG, "resetPQ");
        setContrastIndex(getContrastIndexRange().defaultValue);
        setSaturationIndex(getSaturationIndexRange().defaultValue);
        setSharpnessIndex(getSharpnessIndexRange().defaultValue);
        setPicBrightnessIndex(getPicBrightnessIndexRange().defaultValue);
        setGammaIndex(getGammaIndexRange().defaultValue);
        setDynamicContrastIndex(getDynamicContrastIndexRange().defaultValue);
        // Reset OD
        Utils utils = new Utils(context);
        utils.setSharePrefBoolenValue(OverDriveFragment.SHARED_PREFERENCES_OD_STATUS, false);
        nativeEnableODDemo(0);
        // Reset AAL
        ContentResolver contentResolver = context.getContentResolver();
        AalSettingsFragment.setAalFunction(contentResolver, Settings.System.AAL_FUNC_LABC, false);
        AalSettingsFragment.setAalFunction(contentResolver, Settings.System.AAL_FUNC_CABC, true);
        AalSettingsFragment.setAalFunction(contentResolver, Settings.System.AAL_FUNC_DRE, false);
        setUserBrightness(getUserBrightnessRange().defaultValue);
        Settings.System.putIntForUser(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_OF_AUTO_MODE,
                getUserBrightnessRange().defaultValue, UserHandle.USER_CURRENT);

        nativeSetPictureMode(PIC_MODE_STANDARD);
    }
}
