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

import static java.util.Calendar.DAY_OF_MONTH;
import static java.util.Calendar.MONTH;
import static java.util.Calendar.YEAR;

import java.util.Calendar;
import java.util.GregorianCalendar;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.provider.BaseColumns;
import android.util.Log;

public final class Utils {
    public static final int TEMPERATURE_CELSIUS = 0;
    public static final int TEMPERATURE_FAHRENHEIT = 1;
    private static final String WEATHER_PROVIDER_PACKAGE_ID = "com.mediatek.weather";

    public static boolean isNetworkAvailable(Context context) {
        ConnectivityManager connectivity = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivity != null) {
            NetworkInfo info = connectivity.getActiveNetworkInfo();
            if (info != null) {
                if (info.getState() == NetworkInfo.State.CONNECTED) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * compare day1 to day2
     * @param day1 one day
     * @param day2 other day
     * @return true :  equal ,otherwise false
     */
    public static boolean isSameDay(long day1, long day2) {
        Calendar c1 = Calendar.getInstance();
        Calendar c2 = Calendar.getInstance();
        c1.setTimeInMillis(day1);
        c2.setTimeInMillis(day2);
        return c1.get(YEAR) == c2.get(YEAR) && c1.get(MONTH) == c2.get(MONTH)
                && c1.get(DAY_OF_MONTH) == c2.get(DAY_OF_MONTH);
    }

    public static long getDayStart(long day) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(day);
        GregorianCalendar greCalendar = new GregorianCalendar(
                calendar.get(Calendar.YEAR),
                calendar.get(Calendar.MONTH),
                calendar.get(Calendar.DAY_OF_MONTH));
        long timeDayMillis = greCalendar.getTimeInMillis();
        return timeDayMillis;
    }

    /**
     * get weather description for resId
     *
     * @param desId
     * @param context
     * @return
     * @throws NameNotFoundException
     */
    public static String getWeatherDescription(int desId, Context context)
            throws NameNotFoundException {
        Resources resources = context.getPackageManager()
                .getResourcesForApplication(WEATHER_PROVIDER_PACKAGE_ID);
        return resources.getString(desId);
    }

    /**
     * get the error description for the error code note: 1 only
     * ERROR_SYSTEM_TIME_NOT_CORRECT,
     * ERROR_NETWORK_NOT_AVALIABLE,ERROR_UPDATE_WEATHER_FAILED are i18n
     *
     * @param errorCodeId
     * @param context
     * @return
     * @throws NameNotFoundException
     */
    public static String getErrorDescritiption(WeatherUpdateResult result,
            String cityName, Context context) {
        int errorCodeId = result.getResult();
        if (errorCodeId == WeatherUpdateResult.SUCCESS) {
            return "success";
        }

        Resources resources;
        try {
            resources = context.getPackageManager()
                    .getResourcesForApplication(WEATHER_PROVIDER_PACKAGE_ID);
            int resId = result.getErrMsgResId();
            Log.i("MTKWeatherWidget", "getErrorDescritiption: resId = " + resId);
            if (errorCodeId == WeatherUpdateResult.ERROR_UPDATE_WEATHER_FAILED) {
                return resources.getString(resId, cityName);
            } else {
                return resources.getString(resId);
            }
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }

        return "general update weather fail";
    }
}
