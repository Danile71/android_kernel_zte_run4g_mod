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

import android.content.Context;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

public final class Util {
    private static final String TAG = "W3D/Util";
    private static final String PREF = "WEATHERWIDGET_PREF";

    private Util() {}

    public static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(PREF, Context.MODE_PRIVATE);
    }

    public static boolean isSameDay(long day1, long day2) {
        Calendar c1 = Calendar.getInstance();
        Calendar c2 = Calendar.getInstance();
        c1.setTimeInMillis(day1);
        c2.setTimeInMillis(day2);
        return c1.get(Calendar.YEAR) == c2.get(Calendar.YEAR) && c1.get(Calendar.MONTH) == c2.get(Calendar.MONTH)
                && c1.get(Calendar.DAY_OF_MONTH) == c2.get(Calendar.DAY_OF_MONTH);
    }

    public static String getDateString(long day) {
        return new SimpleDateFormat("MM-dd", Locale.getDefault()).format(new Date(day));
    }

    public static String getTimeString(Context context, long day) {
        DateFormat dateFormat;
        String timeFormat = android.provider.Settings.System.getString(context.getContentResolver(),
                android.provider.Settings.System.TIME_12_24);

        Locale locale = Locale.getDefault();
        LogUtil.v(TAG, "locale = " + locale);
        if ("24".equals(timeFormat)) {
            dateFormat = new SimpleDateFormat("HH:mm", locale);
        } else {
            dateFormat = DateFormat.getTimeInstance(DateFormat.SHORT);
        }

        return dateFormat.format(new Date(day));
    }

    public static int getDayNight(String timeZoneString) {
        final TimeZone timeZone = TimeZone.getTimeZone(timeZoneString);
        final Calendar now = Calendar.getInstance(timeZone);
        final int hourNow = now.get(Calendar.HOUR_OF_DAY);

        LogUtil.v(TAG, "getDayNight = (hour, timeZone) = (" + hourNow + ", " + timeZone + ")");

        if (hourNow >= 6 && hourNow < 18) {
            return DayNight.DAY;
        } else {
            return DayNight.NIGHT;
        }
    }

    public static Calendar getTime(String timeZoneString) {
        final TimeZone timeZone = TimeZone.getTimeZone(timeZoneString);
        return Calendar.getInstance(timeZone);
    }

    public static Bundle getWeatherBundle(int cityIndex, int totalCity, LocationWeather weather, int order) {
        Bundle bundle = new Bundle();
        bundle.putInt("totalCity", totalCity);

        if (totalCity > 0) {
            bundle.putInt("order", order);
            bundle.putInt("cityIndex", cityIndex);
            bundle.putInt("tempType", weather.getTempType());
            bundle.putDouble("temp", weather.getCurrentTemp());
            bundle.putDouble("lowTemp", weather.getTempLow());
            bundle.putDouble("highTemp", weather.getTempHigh());
            bundle.putString("cityName", weather.getLocationName());
            bundle.putLong("lastUpdated", weather.getLastUpdated());
            bundle.putInt("condition", weather.getWeather());
            bundle.putString("timeZone", weather.getTimezone());
            bundle.putInt("result", weather.getResult());
            ForecastData[] data = weather.getForecastData();
            bundle.putInt("firstDayOfWeek", data[0].getDayOfWeek());
            bundle.putDouble("firstHighTemp", data[0].getHighTemp());
            bundle.putDouble("firstLowTemp", data[0].getLowTemp());
            bundle.putInt("firstForecast", data[0].getWeatherCondition());
            bundle.putInt("secondDayOfWeek", data[1].getDayOfWeek());
            bundle.putDouble("secondHighTemp", data[1].getHighTemp());
            bundle.putDouble("secondLowTemp", data[1].getLowTemp());
            bundle.putInt("secondForecast", data[1].getWeatherCondition());
            bundle.putInt("thirdDayOfWeek", data[2].getDayOfWeek());
            bundle.putDouble("thirdHighTemp", data[2].getHighTemp());
            bundle.putDouble("thirdLowTemp", data[2].getLowTemp());
            bundle.putInt("thirdForecast", data[2].getWeatherCondition());
        }
        return bundle;
    }

    public static boolean isNetworkAvailable(Context context) {
        boolean isNetworkAvailable = false;
        ConnectivityManager connManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo info = connManager.getActiveNetworkInfo();
        if (info != null && info.isConnected() && info.isAvailable()) {
            isNetworkAvailable = true;
        }
        LogUtil.v(TAG, "isNetworkAvailable = " + isNetworkAvailable);
        return isNetworkAvailable;
    }
}
