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

package com.mediatek.weather;

import android.os.Parcel;
import android.os.Parcelable;

public class WeatherUpdateResult implements Parcelable {
    /** init state */
    public static final int INIT_VALUE = -1;
    /** success */ 
    public static final int SUCCESS = 0;
    /** system time not correct */
    public static final int ERROR_SYSTEM_TIME_NOT_CORRECT = 1;
    /**network not available */
    public static final int ERROR_NETWORK_NOT_AVAILABLE = 2;
    /**update weather failed */
    public static final int ERROR_UPDATE_WEATHER_FAILED = 3;
    /** city id not correct */
    public static final int ERROR_CITY_ID_NOT_CORRECT = 4; 

    public int mResult = SUCCESS;
    public int mErrMsgResId = -1;

    public void setResult(int rst) {
        mResult = rst;
    }

    public int getResult() {
        return mResult;
    }

    public void setErrMsgResId(int resId) {
        mErrMsgResId = resId;
    }

    public int getErrMsgResId() {
        return mErrMsgResId;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flag) {
        out.writeInt(mResult);
        out.writeInt(mErrMsgResId);
    }

    public void readFromParcel(Parcel in) {
        mResult = in.readInt();
        mErrMsgResId = in.readInt();
    }

    public static final Parcelable.Creator<WeatherUpdateResult> CREATOR = new Parcelable.Creator<WeatherUpdateResult>() {
        public WeatherUpdateResult createFromParcel(Parcel in) {
            WeatherUpdateResult r = new WeatherUpdateResult();
            r.readFromParcel(in);
            return r;
        }

        public WeatherUpdateResult[] newArray(int size) {
            return new WeatherUpdateResult[size];
        }
    };

    public String toString() {
        if (mResult == SUCCESS) {
            return "update success";
        } else if (mResult == ERROR_CITY_ID_NOT_CORRECT) {
            return "city id not correct";
        } else if (mResult == ERROR_NETWORK_NOT_AVAILABLE) {
            return "network not available";
        } else if (mResult == ERROR_SYSTEM_TIME_NOT_CORRECT) {
            return "system time not correct";
        } else if (mResult == ERROR_UPDATE_WEATHER_FAILED) {
            return "general update weather fail";
        } else {
            return "check impl";
        }
    }
}
