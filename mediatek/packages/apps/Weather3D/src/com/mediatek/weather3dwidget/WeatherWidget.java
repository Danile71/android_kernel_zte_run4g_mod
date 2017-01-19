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

import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.RemoteViews;

import com.mediatek.weather.WeatherTable;

public class WeatherWidget extends AppWidgetProvider {
    private static final String TAG = "W3D/WeatherWidget";

    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        super.onUpdate(context, appWidgetManager, appWidgetIds);
        LogUtil.v(TAG, "onUpdate");

        int[] newAppWidgetIds = appWidgetIds;
        if (appWidgetIds == null) {
            LogUtil.v(TAG, "appWidgetIds = null");
            newAppWidgetIds = appWidgetManager.getAppWidgetIds(new ComponentName(
                    context, WeatherWidget.class));
        }

        LogUtil.v(TAG, "appWidgetIds_length = " + newAppWidgetIds.length);

        if (newAppWidgetIds.length == 1) {
            int appWidgetId = newAppWidgetIds[0];
            LogUtil.v(TAG, "appWidgetId_sub = " + appWidgetId);

            WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
            WidgetInfo info = manager.getWidgetStatus(appWidgetId);

            if (info == null) {
                // first added
                LogUtil.v(TAG, "onUpdate info = null");

                boolean isDemo = false;
                Weather3D.setIsDemoMode(isDemo);
                manager.saveWidgetStatus(appWidgetId, isDemo, 0, -1, null, null, WeatherTable.TEMPERATURE_CELSIUS,
                        System.currentTimeMillis());
                RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.weather);
                appWidgetManager.updateAppWidget(newAppWidgetIds, views);

                startUpdateService(context, WeatherWidgetAction.ACTION_INIT, appWidgetId);
            } else {
                LogUtil.v(TAG, "onUpdate info != null");
                Weather3D.setIsDemoMode(info.mDemoMode);
                startUpdateService(context, WeatherWidgetAction.ACTION_UPDATE);
            }

        } else {
            startUpdateService(context, WeatherWidgetAction.ACTION_UPDATE);
        }
    }

    @Override
    public void onEnabled(Context context) {
        super.onEnabled(context);
        LogUtil.v(TAG, "onEnabled");
    }

    @Override
    public void onDisabled(Context context) {
        super.onDisabled(context);
        LogUtil.v(TAG, "onDisabled");
        context.stopService(new Intent(context, UpdateService.class));
        LogUtil.v(TAG, "stopUpdateService");
    }

    @Override
    public void onDeleted(Context context, int[] appWidgetIds) {
        super.onDeleted(context, appWidgetIds);
        LogUtil.v(TAG, "onDeleted");
        WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
        for (int appWidgetId : appWidgetIds) {
            manager.deleteWidgetStatus(appWidgetId);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        super.onReceive(context, intent);
        LogUtil.i(TAG, "onReceive action = " + intent.getAction());
        int id = intent.getIntExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, AppWidgetManager.INVALID_APPWIDGET_ID);

        if (WeatherWidgetAction.ACTION_SCROLL.equals(intent.getAction())) {
            LogUtil.i(TAG, "onReceive action = scroll");
            String direction = intent.getStringExtra(WeatherWidgetAction.DIRECTION);
            if (id != AppWidgetManager.INVALID_APPWIDGET_ID && direction != null) {
                WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
                WidgetInfo wInfo = manager.getWidgetStatus(id);
                if (wInfo == null) {
                    LogUtil.i(TAG, "onReceive no widget info");
                    return;
                }
                startUpdateService(context, WeatherWidgetAction.ACTION_SCROLL, id, direction);
            }
        } else if (WeatherWidgetAction.ACTION_CHECK_WIDGET.equals(intent.getAction())) {
            LogUtil.i(TAG, "onReceive action = check widget, id = " + id);
            if (id != AppWidgetManager.INVALID_APPWIDGET_ID) {
                WeatherWidgetManager manager = WeatherWidgetManager.getInstance(context);
                WidgetInfo wInfo = manager.getWidgetStatus(id);
                if (wInfo == null) {
                    LogUtil.i(TAG, "onReceive no widget info");
                    return;
                }

                startUpdateService(context, WeatherWidgetAction.ACTION_CHECK_WIDGET, id);
            }
        } else {
            AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
            int[] appWidgetIds = appWidgetManager.getAppWidgetIds(new ComponentName(
                context, WeatherWidget.class));
            if (appWidgetIds.length == 0) {
                LogUtil.v(TAG, "onReceive - no widget instance, no handle intent");
                return;
            }

            boolean sendOutIntent = false;
            if (WeatherWidgetAction.ACTION_CITY_LIST_CHANGED.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = city_list_changed");
                sendOutIntent = true;
            } else if (WeatherWidgetAction.ACTION_ALARM_TIME_UP.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = alarm_time_up");
                sendOutIntent = true;
            } else if (Intent.ACTION_TIMEZONE_CHANGED.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = time_zone_changed");
                sendOutIntent = true;
            } else if (Intent.ACTION_TIME_CHANGED.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = time_changed");
                sendOutIntent = true;
            } else if (WeatherWidgetAction.ACTION_WEATHER_CHANGED.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = weather_changed");
                Intent outIntent = new Intent(context, UpdateService.class);
                outIntent.setAction(intent.getAction());
                boolean isTempUnitChanged = intent.getBooleanExtra("tempTypeChanged", false);
                if (isTempUnitChanged) {
                    outIntent.putExtra("tempTypeChanged", true);
                } else {
                    int cityList[] = intent.getIntArrayExtra("citylist");
                    outIntent.putExtra("citylist", cityList);
                }

                context.startService(outIntent);
                return;
            } else if (WeatherWidgetAction.ACTION_DATA_CLEARED.equals(intent.getAction())) {
                String packageName = intent.getStringExtra("packageName");
                if (packageName == null) {
                    return;
                }
                LogUtil.v(TAG, "onReceive action = data_cleared - " + packageName);

                Intent outIntent = new Intent(context, UpdateService.class);
                outIntent.setAction(intent.getAction());
                outIntent.putExtra("packageName", packageName);

                context.startService(outIntent);
                return;
            } else if (WeatherWidgetAction.ACTION_SCREEN_OFF.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = screen_off");
                sendOutIntent = true;
            } else if (WeatherWidgetAction.ACTION_UPDATE.equals(intent.getAction())) {
                LogUtil.v(TAG, "onReceive action = update");
                sendOutIntent = true;
            }

            if (sendOutIntent) {
                startUpdateService(context, intent.getAction());
            }
        }
    }

    private void startUpdateService(Context context, String action, int widgetId, String direction) {
        Intent intent = new Intent(context, UpdateService.class);
        intent.setAction(action);
        intent.setData(Uri.parse(String.valueOf(widgetId)));
        if (direction != null) {
            intent.putExtra(WeatherWidgetAction.DIRECTION, direction);
        }
        context.startService(intent);
    }

    private void startUpdateService(Context context, String action, int widgetId) {
        startUpdateService(context, action, widgetId, null);
    }

    private void startUpdateService(Context context, String action) {
        startUpdateService(context, action, AppWidgetManager.INVALID_APPWIDGET_ID);
    }
}

