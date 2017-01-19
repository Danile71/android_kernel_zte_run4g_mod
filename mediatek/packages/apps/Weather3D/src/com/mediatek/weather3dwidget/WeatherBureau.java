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

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.weather.IWeatherService;
import com.mediatek.weather.Weather;
import com.mediatek.weather.WeatherTable;
import com.mediatek.weather.WeatherUpdateResult;

import java.util.Arrays;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.TreeMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class WeatherBureau {

    private static final String TAG = "W3D/WeatherBureau";
    private ContentResolver mContentResolver;
    private IWeatherService mWeatherService;
    private Context mContext;
    private int mState = STATE_NOT_INIT;

    public LinkedList<LocationWeather> mLocations = new LinkedList<LocationWeather>();
    private final HashMap<Integer, Integer> mCityIdIndexMap = new HashMap<Integer, Integer>();

    private static final LocationWeather INVALID_LOCATION = new LocationWeather(-1);
    private boolean mIsAlarmSet;

    private int mNeedInitCount = 0;
    private int mInitCount = 0;
    private int mInitedCount = 0;

    private static final int REFRESH_TYPE_INIT = 0;

    // press refresh button, refreshWeatherByLocationId
    private static final int REFRESH_TYPE_REFRESH = 1;

    // happen under these two conditions:
    // (1) when the original result is not SUCCESS, and then the network available, try to get weather
    // (2) scroll, the weather result of next city is not SUCCESS, and the network is available, tyr to get weather
    private static final int REFRESH_TYPE_REFRESH_LIGHT = 2;

    private static final int REFRESH_TYPE_ADD_CITY = 3;

    // server update, onWeatherUpdate, refresh_single in UpdateService, refreshWeatherByCityId
    private static final int REFRESH_TYPE_ON_UPDATE = 4;

    public static final int NOTIFY_INITED = 1;
    public static final int NOTIFY_REFRESH_FINISH = 2;
    public static final int NOTIFY_ON_CITY_LIST_CHANGE_FINISH = 3;
    public static final int NOTIFY_ON_WEATHER_UPDATE_FINISH = 4;
    public static final int NOTIFY_INIT_FAIL = 5;

    private static final int STATE_NOT_INIT = 0;
    private static final int STATE_INITING = 1;
    private static final int STATE_INTIED = 2;

    public void init(final Context context) {
        LogUtil.v(TAG, "init - mState = " + mState);
        if (mState == STATE_NOT_INIT) {
            mContext = context;
            if (Weather3D.isDemoMode()) {
                LogUtil.v(TAG, "init - isDemoMode - true");
                initDemoData();
            } else {
                LogUtil.v(TAG, "init - isDemoMode - false");
                bindToWeatherService();
                // initWeatherData will do in onServiceConnected
            }
        }
    }

    private void initDemoData() {
        LogUtil.v(TAG, "initDemoData");
        // Sunny
        ForecastData[] forecastData1 = {new ForecastData(1, 29, 25,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny)),
                new ForecastData(2, 30, 24, LocationWeather.getWeather(WeatherTable.WeatherCondition.Windy)),
                new ForecastData(3, 28, 25, LocationWeather.getWeather(WeatherTable.WeatherCondition.Cloudy))};
        mLocations.add(new LocationWeather(0, "Shenzhen", "GMT+8", WeatherTable.WeatherCondition.Sunny,
                28, 24, 32, forecastData1));

        ForecastData[] forecastData2 = {new ForecastData(1, 31, 25,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Hurricane)),
                new ForecastData(2, 30, 25, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny)),
                new ForecastData(3, 30, 25, LocationWeather.getWeather(WeatherTable.WeatherCondition.Windy))};
        mLocations.add(new LocationWeather(1, "Japanese Village One", "GMT-4", WeatherTable.WeatherCondition.Sunny,
                26, 25, 31, forecastData2));

        // Windy
        ForecastData[] forecastData3 = {new ForecastData(1, 17, 4,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny)),
                new ForecastData(2, 18, 4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Windy)),
                new ForecastData(3, 21, 6, LocationWeather.getWeather(WeatherTable.WeatherCondition.Hurricane))};
        mLocations.add(new LocationWeather(2, "San Francisco", "GMT-8", WeatherTable.WeatherCondition.Windy,
                6, 7, 17, forecastData3));

        ForecastData[] forecastData4 = {new ForecastData(1, 25, 14,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Hurricane)),
                new ForecastData(2, 25, 14, LocationWeather.getWeather(WeatherTable.WeatherCondition.Windy)),
                new ForecastData(3, 24, 14, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny))};
        mLocations.add(new LocationWeather(3, "Dubai", "GMT+4", WeatherTable.WeatherCondition.Windy,
                23, 14, 27, forecastData4));

        // Blustery (Strong Wind)
        ForecastData[] forecastData5 = {new ForecastData(1, 17, 4,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Tornado)),
                new ForecastData(2, 18, 4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Hurricane)),
                new ForecastData(3, 21, 6, LocationWeather.getWeather(WeatherTable.WeatherCondition.Cloudy))};
        mLocations.add(new LocationWeather(4, "Los Angeles", "GMT-8", WeatherTable.WeatherCondition.Hurricane,
                6, 7, 17, forecastData5));

        ForecastData[] forecastData6 = {new ForecastData(1, 25, 14,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Cloudy)),
                new ForecastData(2, 25, 14, LocationWeather.getWeather(WeatherTable.WeatherCondition.Hurricane)),
                new ForecastData(3, 24, 14, LocationWeather.getWeather(WeatherTable.WeatherCondition.Tornado))};
        mLocations.add(new LocationWeather(5, "Moscow", "GMT+4", WeatherTable.WeatherCondition.Hurricane,
                23, 14, 27, forecastData6));

        // Cloudy
        ForecastData[] forecastData7 = {new ForecastData(1, 1, -5,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Overcast)),
                new ForecastData(2, 2, -3, LocationWeather.getWeather(WeatherTable.WeatherCondition.Cloudy)),
                new ForecastData(3, 1, -10, LocationWeather.getWeather(WeatherTable.WeatherCondition.Drizzle))};
        mLocations.add(new LocationWeather(6, "Dhaka", "GMT+6", WeatherTable.WeatherCondition.Cloudy,
                0, -5, 2, forecastData7));

        ForecastData[] forecastData8 = {new ForecastData(1, 3, -3,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Drizzle)),
                new ForecastData(2, 2, -4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Cloudy)),
                new ForecastData(3, 0, -5, LocationWeather.getWeather(WeatherTable.WeatherCondition.Overcast))};
        mLocations.add(new LocationWeather(7, "Houston", "GMT-6", WeatherTable.WeatherCondition.Cloudy,
                1, -10, 3, forecastData8));

        // Rainy
        ForecastData[] forecastData9 = {new ForecastData(1, 29, 14,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Drizzle)),
                new ForecastData(2, 29, 16, LocationWeather.getWeather(WeatherTable.WeatherCondition.Shower)),
                new ForecastData(3, 30, 16, LocationWeather.getWeather(WeatherTable.WeatherCondition.Rain))};
        mLocations.add(new LocationWeather(8, "Santiago", "GMT-4", WeatherTable.WeatherCondition.Shower,
            22, 14, 27, forecastData9));

        ForecastData[] forecastData10 = {new ForecastData(1, 15, 12,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Rain)),
                new ForecastData(2, 15, 12, LocationWeather.getWeather(WeatherTable.WeatherCondition.Shower)),
                new ForecastData(3, 14, 11, LocationWeather.getWeather(WeatherTable.WeatherCondition.Drizzle))};
        mLocations.add(new LocationWeather(9, "Taipei", "GMT+8", WeatherTable.WeatherCondition.Shower,
            18, 14, 22, forecastData10));

        // Heavy Rain
        ForecastData[] forecastData11 = {new ForecastData(1, 15, 12,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Downpour)),
                new ForecastData(2, 15, 12, LocationWeather.getWeather(WeatherTable.WeatherCondition.FreezingRain)),
                new ForecastData(3, 14, 11, LocationWeather.getWeather(WeatherTable.WeatherCondition.Hail))};
        mLocations.add(new LocationWeather(10, "Barcelona", "GMT+1", WeatherTable.WeatherCondition.Downpour,
                18, 14, 22, forecastData11));

        ForecastData[] forecastData12 = {new ForecastData(1, 18, 12,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Hail)),
                new ForecastData(2, 20, 13, LocationWeather.getWeather(WeatherTable.WeatherCondition.FreezingRain)),
                new ForecastData(3, 20, 15, LocationWeather.getWeather(WeatherTable.WeatherCondition.Downpour))};
        mLocations.add(new LocationWeather(11, "Wellington", "GMT+13", WeatherTable.WeatherCondition.Downpour,
                11, 11, 19, forecastData12));

        // Thunder
        ForecastData[] forecastData13 = {new ForecastData(1, 12, 9,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.ThunderstormHail)),
                new ForecastData(2, 13, 3, LocationWeather.getWeather(WeatherTable.WeatherCondition.ThunderyShower)),
                new ForecastData(3, 9, 5, LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow))};
        mLocations.add(new LocationWeather(12, "Lisbon", "GMT", WeatherTable.WeatherCondition.ThunderyShower,
                8, 7, 14, forecastData13));

        ForecastData[] forecastData14 = {new ForecastData(1, 18, 12,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow)),
                new ForecastData(2, 20, 13, LocationWeather.getWeather(WeatherTable.WeatherCondition.ThunderyShower)),
                new ForecastData(3, 20, 15, LocationWeather.getWeather(WeatherTable.WeatherCondition.ThunderstormHail))};
        mLocations.add(new LocationWeather(13, "Suva", "GMT+12", WeatherTable.WeatherCondition.ThunderyShower,
                11, 11, 19, forecastData14));

        // Snow
        ForecastData[] forecastData15 = {new ForecastData(1, 1, -5,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.SnowShowers)),
                new ForecastData(2, 2, -3, LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow)),
                new ForecastData(3, 1, -10, LocationWeather.getWeather(WeatherTable.WeatherCondition.Flurries))};
        mLocations.add(new LocationWeather(14, "St. Petersburg", "GMT+6", WeatherTable.WeatherCondition.Snow,
                0, -5, 2, forecastData15));

        ForecastData[] forecastData16 = {new ForecastData(1, 3, -3,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Flurries)),
                new ForecastData(2, 2, -4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow)),
                new ForecastData(3, 0, -5, LocationWeather.getWeather(WeatherTable.WeatherCondition.SnowShowers))};
        mLocations.add(new LocationWeather(15, "Chicago", "GMT-6", WeatherTable.WeatherCondition.Snow,
                1, -10, 3, forecastData16));

        // Heavy Snow
        ForecastData[] forecastData17 = {new ForecastData(1, 1, -5,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow)),
                new ForecastData(2, 2, -3, LocationWeather.getWeather(WeatherTable.WeatherCondition.HeavySnow)),
                new ForecastData(3, 1, -10, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sleet))};
        mLocations.add(new LocationWeather(16, "Kiev", "GMT+6", WeatherTable.WeatherCondition.HeavySnow,
                0, -5, 2, forecastData17));

        ForecastData[] forecastData18 = {new ForecastData(1, 3, -3,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Sleet)),
                new ForecastData(2, 2, -4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Blizzard)),
                new ForecastData(3, 0, -5, LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow))};
        mLocations.add(new LocationWeather(17, "Winnipeg", "GMT-6", WeatherTable.WeatherCondition.HeavySnow,
                1, -10, 3, forecastData18));

        // Snow Rain
        ForecastData[] forecastData19 = {new ForecastData(1, 1, -5,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.HeavySnow)),
                new ForecastData(2, 2, -3, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sleet)),
                new ForecastData(3, 1, -10, LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow))};
        mLocations.add(new LocationWeather(18, "Tokyo", "GMT+9", WeatherTable.WeatherCondition.Sleet,
                0, -5, 2, forecastData19));

        ForecastData[] forecastData20 = {new ForecastData(1, 3, -3,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow)),
                new ForecastData(2, 2, -4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sleet)),
                new ForecastData(3, 0, -5, LocationWeather.getWeather(WeatherTable.WeatherCondition.HeavySnow))};
        mLocations.add(new LocationWeather(19, "Greenland", "GMT-3", WeatherTable.WeatherCondition.Sleet,
                1, -10, 3, forecastData20));

        // Fog
        ForecastData[] forecastData21 = {new ForecastData(1, 1, -5,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny)),
                new ForecastData(2, 2, -3, LocationWeather.getWeather(WeatherTable.WeatherCondition.Fog)),
                new ForecastData(3, 1, -10, LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow))};
        mLocations.add(new LocationWeather(20, "London", "GMT", WeatherTable.WeatherCondition.Fog,
                0, -5, 2, forecastData21));

        ForecastData[] forecastData22 = {new ForecastData(1, 3, -3,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Snow)),
                new ForecastData(2, 2, -4, LocationWeather.getWeather(WeatherTable.WeatherCondition.Fog)),
                new ForecastData(3, 0, -5, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny))};
        mLocations.add(new LocationWeather(21, "Sydney", "GMT+12", WeatherTable.WeatherCondition.Fog,
                1, -10, 3, forecastData22));

        // Sand
        ForecastData[] forecastData23 = {new ForecastData(1, 1, -5,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Dust)),
                new ForecastData(2, 2, -3, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sand)),
                new ForecastData(3, 1, -10, LocationWeather.getWeather(WeatherTable.WeatherCondition.SandStorm))};
        mLocations.add(new LocationWeather(22, "Roseau", "GMT-4", WeatherTable.WeatherCondition.Sand,
                0, -5, 2, forecastData23));

        ForecastData[] forecastData24 = {new ForecastData(1, 30, 19,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.SandStorm)),
                new ForecastData(2, 29, 15, LocationWeather.getWeather(WeatherTable.WeatherCondition.Sand)),
                new ForecastData(3, 25, 14, LocationWeather.getWeather(WeatherTable.WeatherCondition.Dust))};
        mLocations.add(new LocationWeather(23, "Ulan Bator", "GMT+8", WeatherTable.WeatherCondition.Sand,
                16, 29, 14, forecastData24));

        mState = STATE_INTIED;
        sendOutNotifyIntent(NOTIFY_INITED);
    }

    private void initWeatherData() {
        LogUtil.v(TAG, "initWeatherData - mState = " + mState);
        if (mState == STATE_NOT_INIT) {
            mState = STATE_INITING;

            mContentResolver = mContext.getContentResolver();
            queryLocations(mContext);
        }
    }

    private void queryLocations(Context context) {
        LogUtil.v(TAG, "queryLocations");
        if (mWeatherService == null) {
            LogUtil.v(TAG, "queryLocations - mService - null");
            return;
        }
        final Cursor cc = context.getContentResolver().query(WeatherTable.CityTable.CONTENT_URI, null, null, null, null);
        if (cc == null) {
            return;
        }

        mLocations.clear();
        mNeedInitCount = cc.getCount();
        mInitCount = 0;
        mInitedCount = 0;
        if (cc.getCount() != 0 && cc.moveToFirst()) {
            int locationIndex = 0;
            do {
                int cityId = cc.getInt(cc.getColumnIndex(WeatherTable.CityTable.CITY_ID));
                LogUtil.v(TAG, "weather_in_db, index = " + locationIndex + ", cityId = " + cityId);
                LocationWeather lw = new LocationWeather(locationIndex, cityId);
                // get CityName first
                lw.queryCityName(cc);
                mCityIdIndexMap.put(cityId, locationIndex);
                mLocations.add(lw);
                // get Weather information from non-UI thread
                mInitCount++;
                refreshWeather(REFRESH_TYPE_INIT, cityId, locationIndex, lw);
                locationIndex++;
            } while (cc.moveToNext());
        } else {
            mState = STATE_INTIED;
            sendOutNotifyIntent(NOTIFY_INITED);
        }
        cc.close();
    }

    public void deinit() {
        LogUtil.v(TAG, "deinit");

        if (!Weather3D.isDemoMode()) {
            LogUtil.i(TAG, "onDestroy call unbindService");
            if (mContext != null) {
                mContext.unbindService(mConnection);
            }
            // break cyclic garbage - WeatherBureau -> ContentResolver -> Main
            mContentResolver = null;
            mWeatherService = null;
        }
        if (mLocations != null) {
            mLocations.clear();
        }
        if (mCityIdIndexMap != null) {
            mCityIdIndexMap.clear();
        }
        mState = STATE_NOT_INIT;
    }

    private LinkedList<LocationWeather> getCurrentLocations() {
        return mLocations;
    }

    public int getLocationCount() {
        return getCurrentLocations().size();
    }

    public LocationWeather getLocationByIndex(int index) {
        LinkedList<LocationWeather> locations = getCurrentLocations();

        if (index < 0 || index >= locations.size()) {
            return INVALID_LOCATION;
        }
        return locations.get(index);
    }

    public LocationWeather getLocationByCityId(int cityId) {
        LinkedList<LocationWeather> locations = getCurrentLocations();

        for (int i = 0; i < locations.size(); i++) {
            if (locations.get(i).mCityId == cityId) {
                return locations.get(i);
            }
        }
        return INVALID_LOCATION;
    }

    public void refresh(Context context) {
        // refresh all weather info
        LogUtil.v(TAG, "refresh");
        queryLocations(context);
    }

    public void refreshWeatherByCityId(int[] cityList) {
        if (cityList == null) {
            // cityList == null, means Weather Temperature changed
            LogUtil.v(TAG, "refreshWeatherByCityId, cityList null, tempUnitChanged = true");
            for (LocationWeather lw : mLocations) {
                refreshWeather(REFRESH_TYPE_ON_UPDATE, lw.getCityId(), lw.getLocationIndex(), lw);
                LogUtil.v(TAG, "refreshWeatherByCityId, cityID = " + lw.getCityId() +
                        ", locationIndex = " + lw.getLocationIndex());
            }
        } else {
            LogUtil.v(TAG, "refreshWeatherByCityId, cityList not null, tempUnitChanged = false");
            for (int cityId : cityList) {
                int locationIndex = mCityIdIndexMap.get(cityId);
                LogUtil.v(TAG, "refreshWeatherByCityId, cityID = " + cityId + ", locationIndex = " + locationIndex);
                LocationWeather lw = mLocations.get(locationIndex);
                if (lw != null) {
                    refreshWeather(REFRESH_TYPE_ON_UPDATE, cityId, locationIndex, lw);
                }
            }
        }
    }

    public boolean isLocationIdExist(int locationIndex) {
        int size = mLocations.size();
        LogUtil.v(TAG, "LocationWeather size = " + size + ", locationIndex = " + locationIndex);
        if (locationIndex < size) {
            return true;
        } else {
            return false;
        }
    }

    public void refreshWeatherByLocationId(int locationIndex, boolean isLightRefresh) {
        LocationWeather lw = mLocations.get(locationIndex);
        if (lw == null) {
            LogUtil.v(TAG, "refreshWeatherByLocationId - no such location index");
        } else {
            int cityId = lw.getCityId();
            LogUtil.v(TAG, "refreshWeatherByLocationId, cityID = " + cityId + ", locationIndex = " + locationIndex);

            int refreshType = REFRESH_TYPE_REFRESH;
            if (isLightRefresh) {
                refreshType = REFRESH_TYPE_REFRESH_LIGHT;
            }
            refreshWeather(refreshType, cityId, locationIndex, lw);
        }
    }

    public void refreshWeatherForCityListChange() {
        LogUtil.v(TAG, "refreshWeatherForCityListChange");
        final Cursor cc = mContentResolver.query(WeatherTable.CityTable.CONTENT_URI, null, null, null, null);
        if (cc == null) {
            return;
        }

        int newCityCount = cc.getCount();
        int oldCityCount = getLocationCount();

        HashMap<Integer, Integer> cityHashMap = new HashMap<Integer, Integer>();
        int[] cityArray = new int[newCityCount];

        if (newCityCount != 0 && cc.moveToFirst()) {
            int index = 0;
            do {
                int cityId = cc.getInt(cc.getColumnIndex(WeatherTable.CityTable.CITY_ID));
                cityHashMap.put(cityId, index);
                cityArray[index] = cityId;
                index++;
            } while (cc.moveToNext());
        }

        cc.close();

        LogUtil.v(TAG, "refreshWeatherForCityListChange - (new, old) = (" + newCityCount + ", " + oldCityCount + ")");

        int deltaCityCount = newCityCount - oldCityCount;
        int workingCityId = 0;
        int workingLocationId = 0;
        LocationWeather workingLW;

        if (deltaCityCount < 0) {
            // used to save locationId of deleted cities
            int[] deletedCityArray = new int[Math.abs(deltaCityCount)];
            int count = 0;
            // delete city, maybe more than one
            for (int cityId : mCityIdIndexMap.keySet()) {
                if (!cityHashMap.containsKey(cityId)) {
                    deletedCityArray[count++] = mCityIdIndexMap.get(cityId);
                }
                if (count == Math.abs(deltaCityCount)) {
                    break;
                }
            }
            Arrays.sort(deletedCityArray);

            int i;
            for (i = deletedCityArray.length - 1; i >= 0; i --) {
                mLocations.remove(deletedCityArray[i]);
            }

            mCityIdIndexMap.clear();
            for (i = 0; i < mLocations.size(); i++) {
                workingLW = mLocations.get(i);
                LogUtil.v(TAG, "locationID (new, old) = (" + i + ", " + workingLW.getLocationIndex() + ")");
                LogUtil.v(TAG, "cityName = " + workingLW.getLocationName());
                workingLW.mLocationIndex = i;
                mCityIdIndexMap.put(workingLW.getCityId(), workingLW.getLocationIndex());
            }

            LogUtil.v(TAG, "final city count = " + mLocations.size());
            callbackForCityListChanged(-1);
            LogUtil.v(TAG, "refreshWeatherForCityListChange - delete " + deltaCityCount + " cities done");
        } else if (deltaCityCount == 0) {
            // change city order
            LocationWeather[] mTempLocations = new LocationWeather[newCityCount];
            int i;

            for (i = 0; i < newCityCount; i++) {
                mTempLocations[i] = mLocations.get(mCityIdIndexMap.get(cityArray[i]));
                mTempLocations[i].mLocationIndex = i;
            }
            mLocations.clear();
            mCityIdIndexMap.clear();
            for (i = 0; i < newCityCount; i++) {
                mLocations.add(mTempLocations[i]);
                mCityIdIndexMap.put(mTempLocations[i].getCityId(), mTempLocations[i].getLocationIndex());
            }
            callbackForCityListChanged(0);
            LogUtil.v(TAG, "refreshWeatherForCityListChange - change city order done");
        } else if (deltaCityCount == 1) {
            // add one new city
            for (int cityID : cityHashMap.keySet()) {
                if (!mCityIdIndexMap.containsKey(cityID)) {
                    workingCityId = cityID;
                    break;
                }
            }

            // because new city will be added to the last position, so id = original size
            workingLocationId = mLocations.size();
            workingLW = new LocationWeather(workingLocationId, workingCityId);
            mLocations.add(workingLW);

            mCityIdIndexMap.put(workingCityId, workingLocationId);
            refreshWeather(REFRESH_TYPE_ADD_CITY, workingCityId, workingLocationId, workingLW);
            LogUtil.v(TAG, "refreshWeatherForCityListChange - lw = " + workingLW);
        }
    }

    private void callbackForCityListChanged(int type) {
        // type = -1: delete a city
        // type = 0: city order change
        // type = 1: add a city
        Intent intent = new Intent(mContext, UpdateService.class);
        intent.setAction(WeatherWidgetAction.ACTION_WEATHER_BUREAU_NOTIFY);
        intent.putExtra(WeatherWidgetAction.NOTIFY_TYPE, NOTIFY_ON_CITY_LIST_CHANGE_FINISH);
        mContext.startService(intent);
    }

    public boolean isCityIdExist(int cityId) {
        return mCityIdIndexMap.containsKey(cityId);
    }

    public int getLocationIdByCityId(int cityId) {
        return mCityIdIndexMap.get(cityId);
    }

    public boolean isInited() {
        LogUtil.v(TAG, "mState = " + mState);
        return mState == STATE_INTIED ? true : false;
    }

    public void setNextAlarm(Context context) {
        int count = getLocationCount();
        if (count != 0) {
            TreeMap<Integer, String> timeUp = new TreeMap<Integer, String>();
            Calendar c;
            LocationWeather lw;
            int hour; // the hour of city
            int min;  // the min of city
            int diffMin; // min difference of next alarm

            for (int i = 0; i < count; i++) {
                lw = mLocations.get(i);
                LogUtil.v(TAG, "ith = " + i + "; " + lw);
                if (lw.getResult() == WeatherUpdateResult.SUCCESS) {
                    // get correct timezone, when WeatherUpdateResult = SUCCESS and ERROR_NETWORK_NOT_AVAILABLE;
                    // and get null timezone when WeatherUpdateResult = ERROR_UPDATE_WEATHER_FAILED;
                    // and we just need to care about day night update when the result = SUCCESS.
                    c = Util.getTime(lw.getTimezone());
                    hour = c.get(Calendar.HOUR_OF_DAY);
                    min = c.get(Calendar.MINUTE);

                    if (hour >= 6 && hour < 18) {
                        // calculate how many minutes from now to 18:00
                        // current time is 6:00 - 17:59
                        diffMin = (60 - min) + (17 - hour) * 60;
                    } else if (hour >= 18) {
                        // calculate how many minutes from now to 6:00
                        // current time >= 18:00, but not yet pass 24:00
                        diffMin = (60 - min) + (24 + 5 - hour) * 60;
                    } else {
                        // calculate how many minutes from now to 6:00
                        // current time pass 24:00, but not yet 06:00
                        diffMin = (60 - min) + (5 - hour) * 60;
                    }
                    LogUtil.v(TAG, "ith = " + i + "; diffMin = " + diffMin);
                    timeUp.put(diffMin, lw.getTimezone());
                }
            }
            LogUtil.v(TAG, "setNextAlarm - sortMap = " + timeUp);

            if (!timeUp.isEmpty()) {
                Alarm.setAlarm(context, timeUp.get(timeUp.firstKey()));
                mIsAlarmSet = true;
            }
        }
    }

    public void cancelAlarm(Context context) {
        LogUtil.v(TAG, "cancelAlarm");
        if (mIsAlarmSet) {
            Alarm.stopAlarm(context);
            mIsAlarmSet = false;
        }
    }

    public String getUpdateTimeZone() {
        int count = getLocationCount();
        if (count == 0) {
            return null;
        }

        int hour;
        int min;
        int diffMin;
        Calendar c;
        LocationWeather lw;

        for (int i = 0; i < count; i++) {
            lw = mLocations.get(i);
            LogUtil.v(TAG, "ith = " + i + "; " + lw);
            if (lw.getResult() == WeatherUpdateResult.SUCCESS) {
                c = Util.getTime(lw.getTimezone());
                hour = c.get(Calendar.HOUR_OF_DAY);
                min = c.get(Calendar.MINUTE);

                if (hour >= 6 && hour < 18) {
                    // calculate how many minutes passed 06:00
                    // current time is 6:00 - 17:59
                    diffMin = (hour - 6) * 60 + min;
                } else if (hour >= 18) {
                    // calculate how many minutes passed 18:00
                    // current time >= 18:00, but not yet pass 24:00
                    diffMin = (hour - 18) * 60 + min;
                } else {
                    // calculate how many minutes passed 18:00
                    // current time >= 18:00, and passed 24:00
                    diffMin = (hour + 24 - 18) * 60 + min;
                }

                if (diffMin < 1) {
                    return lw.getTimezone();
                }
            }
        }
        return null;
    }

    private ExecutorService mExecutorService;
    private class WeatherLoader implements Runnable {
        private final int mLocationIndex;
        private final int mCityId;
        private final int mRefreshType;
        private final LocationWeather mLocationWeather;

        public WeatherLoader(int refreshType, int cityId, int locationIndex, LocationWeather lw) {
            mRefreshType = refreshType;
            mCityId = cityId;
            mLocationIndex = locationIndex;
            mLocationWeather = lw;
        }

        public void run() {
            int result = WeatherUpdateResult.INIT_VALUE;
            long currentMillis = System.currentTimeMillis();

            Calendar calendar = Calendar.getInstance();
            calendar.setTimeInMillis(currentMillis);
            GregorianCalendar greCalendar = new GregorianCalendar(
                    calendar.get(Calendar.YEAR),
                    calendar.get(Calendar.MONTH),
                    calendar.get(Calendar.DAY_OF_MONTH));

            long timeTodayMillis = greCalendar.getTimeInMillis();

            try {
                if (mRefreshType == REFRESH_TYPE_REFRESH) {
                    // force refresh, pass -1 as argument
                    result = mWeatherService.updateWeather(mCityId, -1).mResult;
                } else {
                    result = mWeatherService.updateWeather(mCityId, timeTodayMillis).mResult;
                }
            } catch (RemoteException e) {
                LogUtil.v(TAG, "got exception, " + e.getMessage());
            }

            LogUtil.v(TAG, "run result = " + result);

            mLocationWeather.mResult = result;
            mLocationWeather.queryGeoInformation(mContentResolver);
            // no matter result is 0 or not, should set Time zone information here.
            // when weather bureau inited, will set alarm, it will need timezone to set.
            // if not set, then null pointer exception happens

            if (result == WeatherUpdateResult.SUCCESS) {
                if (!mLocationWeather.queryWeather(mContentResolver)) {
                    mLocationWeather.mResult = WeatherUpdateResult.ERROR_UPDATE_WEATHER_FAILED;
                }
                LogUtil.v(TAG, "WeatherLoader done = " + mLocationWeather);
            } else {
                mLocationWeather.mLastUpdated = currentMillis;
                LogUtil.v(TAG, "get weather fail");
            }

            if (mRefreshType == REFRESH_TYPE_INIT) {
                LogUtil.v(TAG, "WeatherLoader - REFRESH_TYPE_INIT");
                if (!isInited()) {
                    mInitedCount++;
                    setInitFlag();
                }
                if (isInited()) {
                    sendOutNotifyIntent(NOTIFY_INITED);
                }
            } else if (mRefreshType == REFRESH_TYPE_REFRESH || mRefreshType == REFRESH_TYPE_REFRESH_LIGHT) {
                LogUtil.v(TAG, "WeatherLoader - REFRESH_TYPE_REFRESH");
                Intent intent = new Intent(mContext, UpdateService.class);
                intent.setAction(WeatherWidgetAction.ACTION_WEATHER_BUREAU_NOTIFY);
                intent.putExtra(WeatherWidgetAction.NOTIFY_TYPE, NOTIFY_REFRESH_FINISH);
                intent.putExtra(WeatherWidgetAction.LOCATION_ID, mLocationWeather.getLocationIndex());
                intent.putExtra(WeatherWidgetAction.CITY_ID, mLocationWeather.getCityId());
                mContext.startService(intent);
            } else if (mRefreshType == REFRESH_TYPE_ON_UPDATE) {
                LogUtil.v(TAG, "WeatherLoader - REFRESH_TYPE_ON_UPDATE - id = " + mLocationWeather.getLocationIndex());
                Intent intent = new Intent(mContext, UpdateService.class);
                intent.setAction(WeatherWidgetAction.ACTION_WEATHER_BUREAU_NOTIFY);
                intent.putExtra(WeatherWidgetAction.NOTIFY_TYPE, NOTIFY_ON_WEATHER_UPDATE_FINISH);
                intent.putExtra(WeatherWidgetAction.LOCATION_ID, mLocationWeather.getLocationIndex());
                intent.putExtra(WeatherWidgetAction.CITY_ID, mLocationWeather.getCityId());
                mContext.startService(intent);
            } else if (mRefreshType == REFRESH_TYPE_ADD_CITY) {
                LogUtil.v(TAG, "WeatherLoader - REFRESH_TYPE_ADD_CITY");
                callbackForCityListChanged(1);
            }
        }
    }

    private void refreshWeather(int refreshType, int cityId, int locationIndex, LocationWeather lw) {
        WeatherLoader wl = new WeatherLoader(refreshType, cityId, locationIndex, lw);
        if (mExecutorService == null) {
            // Weather Provider allows one thread query
            mExecutorService = Executors.newSingleThreadExecutor();
        }
        mExecutorService.submit(wl);
    }

    private void setInitFlag() {
        LogUtil.v(TAG, "(NeedInitCount, InitCount, InitedCount) = (" +
                mNeedInitCount + ", " + mInitCount + ", " + mInitedCount + ")");
        if (mNeedInitCount == mInitCount && mNeedInitCount == mInitedCount) {
            mNeedInitCount = 0;
            mInitCount = 0;
            mInitedCount = 0;
            mState = STATE_INTIED;
            LogUtil.v(TAG, "WeatherBureau Inited");
        }
    }

    // below is WeatherService related code
    private boolean bindToWeatherService() {
        boolean result = mContext.bindService(
                new Intent(IWeatherService.class.getName()), mConnection, Context.BIND_AUTO_CREATE);
        if (result) {
            LogUtil.i(TAG, "bindToWeatherService - success");
        } else {
            LogUtil.i(TAG, "bindToWeatherService - fail");
        }
        return result;
    }

    /**
     * Class for interacting with the main interface of the service.
     */
    private final ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            LogUtil.i(TAG, "weather service connected");
            mWeatherService = IWeatherService.Stub.asInterface(service);
            initWeatherData();
        }

        public void onServiceDisconnected(ComponentName className) {
            LogUtil.i(TAG,"weather service disconnected");
            mWeatherService = null;

            // rebind weather service again
            bindToWeatherService();
        }
    };

    private void sendOutNotifyIntent(int notifyType) {
        Intent intent = new Intent(mContext, UpdateService.class);
        intent.setAction(WeatherWidgetAction.ACTION_WEATHER_BUREAU_NOTIFY);
        intent.putExtra(WeatherWidgetAction.NOTIFY_TYPE, notifyType);
        mContext.startService(intent);
    }

    public boolean isNeedInit() {
        if (mState == STATE_NOT_INIT) {
            LogUtil.v(TAG, "isNeedInit, mState = " + mState);
            return true;
        } else {
            LogUtil.v(TAG, "isNeedInit, mState = " + mState);
            return false;
        }
    }
}
