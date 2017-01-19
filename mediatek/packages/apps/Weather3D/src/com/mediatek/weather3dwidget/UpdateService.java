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

import android.app.PendingIntent;
import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.IBinder;
import android.view.View;
import android.widget.RemoteViews;
import com.mediatek.weather.WeatherTable;
import com.mediatek.weather.WeatherUpdateResult;

public class UpdateService extends Service {
    private static final String TAG = "W3D/UpdateService";
    private WeatherBureau mWeatherBureau;
    private static final String METHOD_SETID = "setWidgetId";

    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        LogUtil.e(TAG, "onDestroy");
        if (mWeatherBureau != null) {
            mWeatherBureau.cancelAlarm(UpdateService.this);
            mWeatherBureau.deinit();
            mWeatherBureau = null;
        }
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtil.i(TAG, "onStartCommand, intent = " + intent);

        if (intent == null) {
            return START_REDELIVER_INTENT;
        }

        if (mWeatherBureau == null) {
            mWeatherBureau = new WeatherBureau();
        }

        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(getApplicationContext());
        RemoteViews views = new RemoteViews(this.getPackageName(), R.layout.weather);

        int widgetId = AppWidgetManager.INVALID_APPWIDGET_ID;
        if (intent.getDataString() != null) {
            widgetId = Integer.parseInt(intent.getDataString());
        }

        registerScreenActionReceiver();

        LogUtil.v(TAG, "widgetId = " + widgetId);
        String intentAction = intent.getAction();
        LogUtil.v(TAG, "intentAction = " + intentAction);

        if (widgetId == AppWidgetManager.INVALID_APPWIDGET_ID) {
            int[] appWidgetIds = appWidgetManager.getAppWidgetIds(
                    new ComponentName(getApplicationContext(), WeatherWidget.class));
            LogUtil.v(TAG, "appWidgetIds len = " + appWidgetIds.length);

            // not related to widget
            if (WeatherWidgetAction.ACTION_CITY_LIST_CHANGED.equals(intentAction)) {
                LogUtil.v(TAG, "city list changed intent");
                // if the updateService is killed, then there will be no WeatherBureau, it will cause update fail
                // (nullPointException), so check if need init first
                if (isWeatherBureauInited()) {
                    mWeatherBureau.refreshWeatherForCityListChange();
                } else {
                    reinitWeatherBureauIfNeeded();
                }
                // related to multi widgets
            } else if (WeatherWidgetAction.ACTION_ALARM_TIME_UP.equals(intentAction)) {
                LogUtil.v(TAG, "alarm time up intent");
                if (isWeatherBureauInited()) {
                    String timeUpTimeZone = mWeatherBureau.getUpdateTimeZone();
                    LogUtil.v(TAG, "timeZone = " + timeUpTimeZone);
                    if (timeUpTimeZone != null) {
                        updateDayNight(appWidgetManager, views, appWidgetIds);
                        mWeatherBureau.cancelAlarm(UpdateService.this);
                        mWeatherBureau.setNextAlarm(UpdateService.this);
                    }
                }
            } else if (Intent.ACTION_TIMEZONE_CHANGED.equals(intentAction) ||
                    Intent.ACTION_TIME_CHANGED.equals(intentAction)) {
                LogUtil.v(TAG, "timezone changed or time changed");
                onTimeChanged(appWidgetManager, views, appWidgetIds);
            } else if (WeatherWidgetAction.ACTION_WEATHER_BUREAU_NOTIFY.equals(intentAction)) {
                int notifyType = intent.getIntExtra(WeatherWidgetAction.NOTIFY_TYPE, 0);
                LogUtil.v(TAG, "weather bureau notify, type = " + notifyType);

                if (notifyType == WeatherBureau.NOTIFY_INITED) {
                    onWeatherBureauInited(appWidgetManager, views, appWidgetIds);
                } else if (notifyType == WeatherBureau.NOTIFY_REFRESH_FINISH) {
                    onRefreshFinish(intent, appWidgetManager, views, appWidgetIds);
                } else if (notifyType == WeatherBureau.NOTIFY_ON_WEATHER_UPDATE_FINISH) {
                    onUpdateWeatherFinish(intent, appWidgetManager, views, appWidgetIds);
                } else if (notifyType == WeatherBureau.NOTIFY_ON_CITY_LIST_CHANGE_FINISH) {
                    onCityListChangeFinish(appWidgetManager, views, appWidgetIds);
                } else if (notifyType == WeatherBureau.NOTIFY_INIT_FAIL) {
                    onWeatherBureauInitFail(appWidgetManager, views, appWidgetIds);
                }
            } else if (WeatherWidgetAction.ACTION_NETWORK_CONNECTED.equals(intentAction)) {
                LogUtil.v(TAG, "network connected intent");
                onNetworkConnected(appWidgetManager, views, appWidgetIds);
            } else if (WeatherWidgetAction.ACTION_WEATHER_CHANGED.equals(intentAction)) {
                // if the updateService is killed, then there will be no WeatherBureau, it will cause update fail
                // (nullPointException), so check if need init first
                boolean isTempUnitChanged = intent.getBooleanExtra("tempTypeChanged", false);
                int cityList[] = null;
                LogUtil.v(TAG, "weather changed intent, tempUnitChanged = " + isTempUnitChanged);
                if (!isTempUnitChanged) {
                    cityList = intent.getIntArrayExtra("citylist");
                    LogUtil.v(TAG, "weather changed intent, cityList = " + cityList);
                }
                if (isWeatherBureauInited()) {
                    mWeatherBureau.refreshWeatherByCityId(cityList);
                } else {
                    reinitWeatherBureauIfNeeded();
                }
                // related to multi widgets
            } else if (WeatherWidgetAction.ACTION_DATA_CLEARED.equals(intentAction)) {
                String packageName = intent.getStringExtra("packageName");
                LogUtil.v(TAG, "data cleared intent - " + packageName);
                Context context = getApplicationContext();

                if (packageName.equals(context.getPackageName())) {
                    // com.mediatek.weather3dwidget is cleared.
                    // restore widget status
                    WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
                    for (int i = 0; i < appWidgetIds.length; i++) {
                        manager.saveWidgetStatus(appWidgetIds[i], false, 0, -1, null, null, WeatherTable.TEMPERATURE_CELSIUS,
                                System.currentTimeMillis());
                    }
                } else if (packageName.equals(WeatherTable.PROVIDER_PACKAGE_NAME)) {
                    // com.mediatek.weather is cleared.
                    // WeatherService data is cleared, all cities information is cleared.
                    // WeatherService is also cleared. So mWeatherService in mWeatherBureau cannot be used anymore.
                    // so deinit and then init
                    mWeatherBureau.deinit();
                    mWeatherBureau.init(UpdateService.this);
                }
            } else if (WeatherWidgetAction.ACTION_SCREEN_OFF.equals(intentAction)) {
                LogUtil.v(TAG, "screen off intent");
                onScreenOff(appWidgetManager, views, appWidgetIds);
            } else if (WeatherWidgetAction.ACTION_UPDATE.equals(intentAction)) {
                LogUtil.v(TAG, "update intent");
                onUpdateContent(appWidgetManager, views, appWidgetIds);
            }
        } else {
            // update the assigned widget id widget
            Context context = getApplicationContext();

            WeatherWidgetManager wManager = WeatherWidgetManager.getInstance(context);
            WidgetInfo wInfo = wManager.getWidgetStatus(widgetId);

            if (WeatherWidgetAction.ACTION_INIT.equals(intentAction)) {
                LogUtil.v(TAG, "init intent - WeatherBureau isNeedInit = " + mWeatherBureau.isNeedInit());
                if (isWeatherBureauInited()) {
                    updateWeatherViewContent(appWidgetManager, views, widgetId, 0, ScrollType.NO_SCROLL);
                } else {
                    if (mWeatherBureau.isNeedInit()) {
                        showProgressBar(views);
                        appWidgetManager.updateAppWidget(widgetId, views);

                        mWeatherBureau.init(UpdateService.this);
                    }
                }
            } else if (WeatherWidgetAction.ACTION_SCROLL.equals(intentAction)) {
                LogUtil.v(TAG, "scroll intent");
                if (isWeatherBureauInited()) {
                    int total = mWeatherBureau.getLocationCount();
                    LogUtil.v(TAG, "total = " + total);
                    CityManager.setTotal(total);

                    if (total > 0) {
                        // could not send out city change when there is not city set.
                        int locationId = wInfo.mLocationId;
                        CityManager.setCurrentIndex(locationId);
                        LogUtil.v(TAG, "locationId = " + locationId);

                        int order;
                        if (intent.getStringExtra(
                                WeatherWidgetAction.DIRECTION).equals(WeatherWidgetAction.DIRECTION_NEXT)) {
                            LogUtil.v(TAG, "scroll down - widgetId = " + widgetId);
                            locationId = CityManager.getNextCity();
                            order = ScrollType.SCROLL_DOWN;
                        } else {
                            LogUtil.v(TAG, "scroll up - widgetId = " + widgetId);
                            locationId = CityManager.getPreviousCity();
                            order = ScrollType.SCROLL_UP;
                        }

                        LocationWeather weather = mWeatherBureau.getLocationByIndex(locationId);
                        int result = weather.getResult();
                        int newCityId = weather.getCityId();
                        int newLocationId = weather.getLocationIndex();
                        LogUtil.v(TAG, "result = " + result + ", cityId = " + newCityId + ", locationId = " + newLocationId);

                        if (weather.getResult() == WeatherUpdateResult.SUCCESS) {
                            updateWeatherViewContent(appWidgetManager, views, widgetId, locationId, order);
                        } else {
                            boolean isNetworkAvailable = Util.isNetworkAvailable(context);
                            if (isNetworkAvailable) {
                                // the original result is not SUCCESS, and now, the network is available again,
                                // so try to refresh by light_refresh to get weather again.

                                // here update the location and city id to be the new one,
                                // so that when refresh finish notify,
                                // onRefreshFinish will if the city and location is the same
                                wInfo.mLocationId = newLocationId;
                                wInfo.mCityId = newCityId;
                                wManager.updateWidgetStatus(widgetId, wInfo);

                                mWeatherBureau.refreshWeatherByLocationId(locationId, true);
                                showProgressBar(views);
                                appWidgetManager.updateAppWidget(widgetId, views);
                            } else {
                                // the original result is not SUCCESS, but the network is still not available,
                                // so show not_available string.
                                updateWeatherViewContent(appWidgetManager, views, widgetId, locationId, order);
                            }
                        }
                    }
                } else {
                    reinitWeatherBureauIfNeeded();
                    showProgressBar(views);
                    appWidgetManager.updateAppWidget(widgetId, views);
                }
            } else if (WeatherWidgetAction.ACTION_REFRESH.equals(intentAction)) {
                LogUtil.v(TAG, "refresh intent");
                if (isWeatherBureauInited()) {
                    if (!wInfo.mDemoMode) {
                        int locationId = wInfo.mLocationId;
                        LogUtil.v(TAG, "refresh intent, widgetId = " + widgetId + ", locationId = " + locationId);
                        if (mWeatherBureau.isLocationIdExist(locationId)) {
                            mWeatherBureau.refreshWeatherByLocationId(locationId, false);
                        } else {
                            LogUtil.i(TAG, "refresh intent, fail");
                            return super.onStartCommand(intent, flags, startId);
                        }
                    } else {
                        return super.onStartCommand(intent, flags, startId);
                    }
                } else {
                    reinitWeatherBureauIfNeeded();
                }
                showProgressBar(views);
                appWidgetManager.updateAppWidget(widgetId, views);
            } else if (WeatherWidgetAction.ACTION_CHECK_WIDGET.equals(intentAction)) {
                LogUtil.v(TAG, "check widget intent");
                if (!isWeatherBureauInited()) {
                    reinitWeatherBureauIfNeeded();
                    showProgressBar(views);
                    appWidgetManager.updateAppWidget(widgetId, views);
                }
            }
        }

        return super.onStartCommand(intent, flags, startId);
    }

    private void showProgressBar(RemoteViews views) {
        views.setInt(R.id.view_3d_weather, "showUpdating", 1);
        views.setViewVisibility(R.id.progress_bar, View.VISIBLE);
        views.setInt(R.id.view_3d_weather, "startTimer", 1);
        LogUtil.v(TAG, "progress_bar : VISIBLE");
    }

    private void hideProgressBar(RemoteViews views) {
        views.setViewVisibility(R.id.progress_bar, View.GONE);
        views.setInt(R.id.view_3d_weather, "stopTimer", 1);
        LogUtil.v(TAG, "progress_bar : GONE");
    }

    private void onTimeChanged(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onTimeChanged");
        if (isWeatherBureauInited()) {
            mWeatherBureau.cancelAlarm(UpdateService.this);
            mWeatherBureau.setNextAlarm(UpdateService.this);

            Context context = getApplicationContext();
            WeatherWidgetManager wManager = WeatherWidgetManager.getInstance(context);

            for (int widgetId: appWidgetIds) {
                WidgetInfo wInfo = wManager.getWidgetStatus(widgetId);
                if (wInfo == null) {
                    LogUtil.v(TAG, "noSuchWidgetId");
                    continue;
                }
                updateWeatherViewContent(appWidgetManager, views, widgetId, wInfo.mLocationId, ScrollType.NO_SCROLL);
            }
        }
    }

    private void onWeatherBureauInited(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onWeatherBureauInited");
        if (isWeatherBureauInited()) {
            mWeatherBureau.cancelAlarm(UpdateService.this);
            mWeatherBureau.setNextAlarm(UpdateService.this);
            for (int widgetId: appWidgetIds) {
                updateWeatherViewContent(appWidgetManager, views, widgetId, 0, ScrollType.NO_SCROLL);
            }
        }
    }

    private void onRefreshFinish(Intent intent, AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        int locationId = intent.getIntExtra(WeatherWidgetAction.LOCATION_ID, 0);
        int cityId = intent.getIntExtra(WeatherWidgetAction.CITY_ID, -1);
        LogUtil.v(TAG, "onRefreshFinish - locationId = " + locationId + ", cityId = " + cityId);

        Context context = getApplicationContext();
        WeatherWidgetManager wManager = WeatherWidgetManager.getInstance(context);

        for (int widgetId: appWidgetIds) {
            LogUtil.v(TAG, "onRefreshFinish - id = " + widgetId);
            WidgetInfo wInfo = wManager.getWidgetStatus(widgetId);
            if (wInfo == null) {
                LogUtil.v(TAG, "noSuchWidgetId");
                return;
            }

            LogUtil.v(TAG, "onRefreshFinish - current LId = " + wInfo.mLocationId + ", CId = " + wInfo.mCityId +
                    ", origin - LId = " + locationId + ", CId = " + cityId);
            updateWeatherViewContent(appWidgetManager, views, widgetId, wInfo.mLocationId, ScrollType.NO_SCROLL);
        }
    }

    private void onUpdateWeatherFinish(Intent intent, AppWidgetManager appWidgetManager, RemoteViews views,
            int[] appWidgetIds) {
        int locationId = intent.getIntExtra(WeatherWidgetAction.LOCATION_ID, 0);
        int cityId = intent.getIntExtra(WeatherWidgetAction.CITY_ID, -1);
        LogUtil.v(TAG, "onUpdateWeatherFinish - locationId = " + locationId + ", cityId = " + cityId);

        for (int widgetId: appWidgetIds) {
            LogUtil.v(TAG, "onUpdateWeatherFinish - id = " + widgetId);
            WidgetInfo wInfo = getWidgetInfo(widgetId);
            if (wInfo == null) {
                LogUtil.v(TAG, "onUpdateWeatherFinish - noSuchWidgetId");
                return;
            }

            LogUtil.v(TAG, "onUpdateWeatherFinish - currentLId = " + wInfo.mLocationId + ", currentCId = " + wInfo.mCityId);
            if (cityId == wInfo.mCityId) {
                updateWeatherViewContent(appWidgetManager, views, widgetId, locationId, ScrollType.NO_SCROLL);
            }
        }
    }

    private void onCityListChangeFinish(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onCityListChangeFinish");

        int total = mWeatherBureau.getLocationCount();
        mWeatherBureau.cancelAlarm(UpdateService.this);
        if (total > 0) {
            mWeatherBureau.setNextAlarm(UpdateService.this);
        }

        Context context = getApplicationContext();
        WeatherWidgetManager wManager = WeatherWidgetManager.getInstance(context);

        for (int widgetId: appWidgetIds) {
            LogUtil.v(TAG, "onCityListChangeFinish - id = " + widgetId);
            WidgetInfo wInfo = wManager.getWidgetStatus(widgetId);
            if (wInfo == null) {
                LogUtil.v(TAG, "noSuchWidgetId");
                return;
            }

            int locationId = 0;

            if (total > 0) {
                locationId = wInfo.mLocationId;
                int cityId = wInfo.mCityId;

                if (mWeatherBureau.isCityIdExist(cityId)) {
                    locationId = mWeatherBureau.getLocationIdByCityId(cityId);
                } else {
                    if (locationId >= total) {
                        // show last city according to location id order
                        locationId = total - 1;
                    }
                    // Others:
                    // remain to show the new city in original location id
                }
            }
            updateWeatherViewContent(appWidgetManager, views, widgetId, locationId, ScrollType.NO_SCROLL);
        }
    }

    private void onWeatherBureauInitFail(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onWeatherBureauInitFail");
        for (int widgetId: appWidgetIds) {
            updateInitFailWeatherView(appWidgetManager, views, widgetId);
        }
    }

    private void updateInitFailWeatherView(AppWidgetManager appWidgetManager, RemoteViews views, int widgetId) {
        Context context = getApplicationContext();
        WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
        WidgetInfo wInfo = manager.getWidgetStatus(widgetId);
        if (wInfo == null) {
            LogUtil.v(TAG, "noSuchWidgetId");
            return;
        }

        // total = -1, means WeatherBureau init fail
        int total = -1;
        LogUtil.v(TAG, "total = " + total);
        views.setBundle(R.id.view_3d_weather, "updateWeatherView",
                Util.getWeatherBundle(0, total, null, 0));
        // reset locationId & cityId
        wInfo.mLocationId = 0;
        wInfo.mCityId = -1;
        wInfo.mTimeZone = null;
        wInfo.mCityName = null;

        views.setInt(R.id.view_3d_weather, METHOD_SETID, widgetId);
        hideProgressBar(views);

        if (!wInfo.mDemoMode) {
            views.setViewVisibility(R.id.refresh, View.VISIBLE);
            views.setOnClickPendingIntent(R.id.refresh, getRefreshPendingIntent(context, widgetId));
            views.setViewVisibility(R.id.setting, View.VISIBLE);
            views.setOnClickPendingIntent(R.id.setting, getSettingPendingIntent(context));
        }
        LogUtil.v(TAG, "set button setting/refresh intent");
        appWidgetManager.updateAppWidget(widgetId, views);
        manager.updateWidgetStatus(widgetId, wInfo);
    }

    private void onNetworkConnected(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onNetworkConnected");

        if (!isWeatherBureauInited()) {
            return;
        }

        int total = mWeatherBureau.getLocationCount();

        Context context = getApplicationContext();
        WeatherWidgetManager wManager = WeatherWidgetManager.getInstance(context);

        for (int widgetId: appWidgetIds) {
            LogUtil.v(TAG, "onNetworkConnected - id = " + widgetId);
            WidgetInfo wInfo = wManager.getWidgetStatus(widgetId);
            if (wInfo == null || wInfo.mDemoMode) {
                LogUtil.v(TAG, "noSuchWidgetId or DemoMode");
                return;
            }

            if (total > 0) {
                int locationId = wInfo.mLocationId;

                if (mWeatherBureau.getLocationByIndex(locationId).getResult() != WeatherUpdateResult.SUCCESS) {
                    LogUtil.v(TAG, "Result not SUCCESS, refresh, widgetId = " + widgetId + ", locationId = " + locationId);
                    mWeatherBureau.refreshWeatherByLocationId(locationId, true);
                    showProgressBar(views);
                    appWidgetManager.updateAppWidget(widgetId, views);
                }
            }
        }
    }

    private void onScreenOff(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onScreenOff");
        views.setInt(R.id.view_3d_weather, "onScreenOff", 1);

        for (int widgetId: appWidgetIds) {
            appWidgetManager.updateAppWidget(widgetId, views);
        }
    }

    private void onUpdateContent(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        LogUtil.v(TAG, "onUpdateContent");

        if (isWeatherBureauInited()) {
            Context context = getApplicationContext();
            WeatherWidgetManager wManager = WeatherWidgetManager.getInstance(context);

            for (int widgetId: appWidgetIds) {
                LogUtil.v(TAG, "onUpdateContent - id = " + widgetId);
                WidgetInfo wInfo = wManager.getWidgetStatus(widgetId);
                if (wInfo == null) {
                    LogUtil.v(TAG, "noSuchWidgetId");
                    return;
                }

                LogUtil.v(TAG, "onUpdateContent - current LId = " + wInfo.mLocationId + ", CId = " + wInfo.mCityId);
                updateWeatherViewContent(appWidgetManager, views, widgetId, wInfo.mLocationId, ScrollType.NO_SCROLL);
            }
        } else {
            reinitWeatherBureauIfNeeded();
            showProgressBar(views);
            for (int widgetId: appWidgetIds) {
                appWidgetManager.updateAppWidget(widgetId, views);
            }
        }
    }

    private void updateWeatherViewContent(AppWidgetManager appWidgetManager, RemoteViews views,
            int widgetId, int locationId, int order) {
        LogUtil.v(TAG, "updateWeatherViewContent - widgetId = " + widgetId + ", locationId = " + locationId);

        Context context = getApplicationContext();
        WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
        WidgetInfo wInfo = manager.getWidgetStatus(widgetId);
        if (wInfo == null) {
            LogUtil.v(TAG, "noSuchWidgetId");
            return;
        }

        int total = mWeatherBureau.getLocationCount();
        LogUtil.v(TAG, "total = " + total);

        if (total > 0) {
            LocationWeather weather = mWeatherBureau.getLocationByIndex(locationId);
            LogUtil.v(TAG, "weather = " + weather);
            views.setBundle(R.id.view_3d_weather, "updateWeatherView",
                    Util.getWeatherBundle(locationId, total, weather, order));

            wInfo.mLocationId = locationId;
            wInfo.mCityId = weather.getCityId();
            wInfo.mTimeZone = weather.getTimezone();
            wInfo.mCityName = weather.getLocationName();
        } else {
            views.setBundle(R.id.view_3d_weather, "updateWeatherView",
                    Util.getWeatherBundle(locationId, total, null, order));
            // reset locationId & cityId
            wInfo.mLocationId = 0;
            wInfo.mCityId = -1;
            wInfo.mTimeZone = null;
            wInfo.mCityName = null;
        }

        views.setInt(R.id.view_3d_weather, METHOD_SETID, widgetId);
        hideProgressBar(views);

        if (!wInfo.mDemoMode) {
            if (total > 0) {
                views.setViewVisibility(R.id.refresh, View.VISIBLE);
                views.setOnClickPendingIntent(R.id.refresh, getRefreshPendingIntent(context, widgetId));
            } else {
                views.setViewVisibility(R.id.refresh, View.GONE);
            }
            views.setViewVisibility(R.id.setting, View.VISIBLE);
            views.setOnClickPendingIntent(R.id.setting, getSettingPendingIntent(context));
        }
        LogUtil.v(TAG, "set button setting/refresh intent");
        appWidgetManager.updateAppWidget(widgetId, views);
        manager.updateWidgetStatus(widgetId, wInfo);
    }

    private void updateDayNight(AppWidgetManager appWidgetManager, RemoteViews views, int[] appWidgetIds) {
        for (int widgetId: appWidgetIds) {
            LogUtil.v(TAG, "updateDayNight - id = " + widgetId);
            views.setInt(R.id.view_3d_weather, "switchDayNight", 1);
            appWidgetManager.updateAppWidget(widgetId, views);
        }
    }

    private boolean isWeatherBureauInited() {
        return mWeatherBureau.isInited();
    }

    private void reinitWeatherBureauIfNeeded() {
        if (mWeatherBureau.isNeedInit()) {
            // if in INITING and INITED case, then will not init again.
            // if in NOT_INIT case, then init again.
            LogUtil.v(TAG, "re-initialize WeatherBureau");
            mWeatherBureau.init(UpdateService.this);
        }
    }

    private WidgetInfo getWidgetInfo(int widgetId) {
        return WeatherWidgetManager.getInstance(getApplicationContext()).getWidgetStatus(widgetId);
    }

    private PendingIntent getRefreshPendingIntent(Context context, final int appWidgetId) {
        Intent intent = new Intent(context, UpdateService.class).
                setAction(WeatherWidgetAction.ACTION_REFRESH).
                setData(Uri.parse(String.valueOf(appWidgetId)));
        return (PendingIntent.getService(context, 0, intent, 0));
    }

    private PendingIntent getSettingPendingIntent(Context context) {
        Intent intent = new Intent(WeatherTable.Intents.ACTION_SETTING);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        return (PendingIntent.getActivity(context, 0, intent, 0));
    }

    private void registerScreenActionReceiver() {
        final IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        registerReceiver(mReceiver, filter);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(final Context context, final Intent intent) {
            LogUtil.i(TAG, "onReceive - " + intent.getAction());
            Intent offIntent = new Intent(WeatherWidgetAction.ACTION_SCREEN_OFF);
            getApplicationContext().sendBroadcast(offIntent);
        }
    };
}
