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

public final class WeatherWidgetManager {
    private static final String TAG = "W3D/WeatherWidgetManager";

    private static WeatherWidgetManager sInstance = null;
    private final Context mContext;

    public static final String SEPARATOR = ", ";

    public static final String INSEPARATOR = "=";

    public static final String WIDGET_ID = "widget_id_";

    public static final String DEMO_MODE = "demo_mode";

    public static final String LOCATION_ID = "location_id";

    public static final String CITY_ID = "city_id";

    public static final String CITY_NAME = "city_name";

    public static final String TIME_ZONE = "time_zone";

    public static final String TEMPERATURE_TYPE = "temp_type";

    public static final String LAST_UPDATED = "last_updated";

    private static Object sWidgetLock = new Object();

    private WeatherWidgetManager(Context c) {
        mContext = c;
    }

    public static WeatherWidgetManager getInstance(Context c) {
        synchronized (sWidgetLock) {
            if (sInstance == null) {
                sInstance = new WeatherWidgetManager(c);
            }
            return sInstance;
        }
    }

    // separate widget information
    public void saveWidgetStatus(int appWidgetId, boolean demoMode, int locationId, int cityId,
            String cityName, String timeZone, int tempType, long lastUpdate) {
        synchronized (sWidgetLock) {
            SharedPreferences pref = Util.getSharedPreferences(mContext);

            String key = WIDGET_ID + appWidgetId;

            StringBuilder sb = new StringBuilder();
            sb.append(DEMO_MODE).append(INSEPARATOR).append(demoMode).append(SEPARATOR);
            sb.append(LOCATION_ID).append(INSEPARATOR).append(locationId).append(SEPARATOR);
            sb.append(CITY_ID).append(INSEPARATOR).append(cityId).append(SEPARATOR);
            sb.append(CITY_NAME).append(INSEPARATOR).append(cityName).append(SEPARATOR);
            sb.append(TIME_ZONE).append(INSEPARATOR).append(timeZone).append(SEPARATOR);
            sb.append(TEMPERATURE_TYPE).append(INSEPARATOR).append(tempType).append(SEPARATOR);
            sb.append(LAST_UPDATED).append(INSEPARATOR).append(lastUpdate);

            pref.edit().putString(key, sb.toString()).commit();
            String content = pref.getString(key, "");
            LogUtil.v(TAG, "saveWidgetStatus - " + key + ", " + content);
        }
    }

    public void updateWidgetStatus(int appWidgetId, WidgetInfo wInfo) {
        LogUtil.v(TAG, "updateWidgetStatus");
        synchronized (sWidgetLock) {
            WidgetInfo oldInfo = getWidgetStatus(appWidgetId);
            if (oldInfo == null) {
                LogUtil.i(TAG, "oldInfo == " + null);
                return;
            }

            oldInfo.mDemoMode = wInfo.mDemoMode;
            oldInfo.mLocationId = wInfo.mLocationId;
            oldInfo.mCityId = wInfo.mCityId;
            oldInfo.mCityName = wInfo.mCityName;
            oldInfo.mTimeZone = wInfo.mTimeZone;
            oldInfo.mTempType = wInfo.mTempType;
            oldInfo.mLastUpdate = wInfo.mLastUpdate;

            saveWidgetStatus(appWidgetId, oldInfo.mDemoMode, oldInfo.mLocationId, oldInfo.mCityId, oldInfo.mCityName,
                    oldInfo.mTimeZone, oldInfo.mTempType, oldInfo.mLastUpdate);
        }
    }

    private WidgetInfo parseWidgetInfo(String widgetContent, int appWidgetId) {
        WidgetInfo widgetInfo = null;
        if (widgetContent != null && !"".equals(widgetContent)) {
            widgetInfo = new WidgetInfo();

            String[] keyValues = widgetContent.split(SEPARATOR);
            for (String keyValue : keyValues) {
                String[] entry = keyValue.split(INSEPARATOR);
                if (entry.length != 2) {
                    continue;
                }
                if (DEMO_MODE.equals(entry[0])) {
                    widgetInfo.mDemoMode = Boolean.valueOf(entry[1]);
                } else if (LOCATION_ID.equals(entry[0])) {
                    widgetInfo.mLocationId = Integer.valueOf(entry[1]);
                } else if (CITY_ID.equals(entry[0])) {
                    widgetInfo.mCityId = Integer.valueOf(entry[1]);
                } else if (CITY_NAME.equals(entry[0]) && !(entry[1].equals("null"))) {
                    widgetInfo.mCityName = entry[1];
                } else if (TIME_ZONE.equals(entry[0])) {
                    widgetInfo.mTimeZone = entry[1];
                } else if (TEMPERATURE_TYPE.equals(entry[0])) {
                    widgetInfo.mTempType = Integer.valueOf(entry[1]);
                } else if (LAST_UPDATED.equals(entry[0])) {
                    widgetInfo.mLastUpdate = Long.valueOf(entry[1]);
                }
            }
        }
        return widgetInfo;
    }

    public WidgetInfo getWidgetStatus(int appWidgetId) {
        LogUtil.v(TAG, "getWidgetStatus");
        synchronized (sWidgetLock) {
            WidgetInfo widgetInfo = null;
            SharedPreferences pref = Util.getSharedPreferences(mContext);
            String key = WIDGET_ID + appWidgetId;
            if (pref.contains(key)) {
                LogUtil.v(TAG, "getWidgetStatus = yes");
                String widgetContent = pref.getString(key,"");
                LogUtil.v(TAG, "getWidgetStatus - " + key + ", " + widgetContent);
                widgetInfo = parseWidgetInfo(widgetContent, appWidgetId);
            } else {
                LogUtil.v(TAG, "getWidgetStatus = no");
            }
            return widgetInfo;
        }
    }

    public void deleteWidgetStatus(int appWidgetId) {
        LogUtil.v(TAG, "deleteWidgetStatus");
        synchronized (sWidgetLock) {
            SharedPreferences pref = Util.getSharedPreferences(mContext);
            String key = WIDGET_ID + appWidgetId;
            if (pref.contains(key)) {
                LogUtil.v(TAG, "deleteWidgetStatus = yes - " + key);
                pref.edit().remove(key).commit();
            } else {
                LogUtil.v(TAG, "deleteWidgetStatus = no");
            }
        }
    }
}
