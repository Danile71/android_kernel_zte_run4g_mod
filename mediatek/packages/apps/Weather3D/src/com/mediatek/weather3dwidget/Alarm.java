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

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import java.util.Calendar;

public final class Alarm {
    private static final String TAG = "W3D/Alarm";
    private static final long FIVE_MIN_IN_MS = 5 * 60 * 1000;

    private Alarm() {}

    public static void setAlarm(Context context, String timezone) {
        LogUtil.v(TAG, "timezone = " + timezone);
        Calendar c = Calendar.getInstance();
        LogUtil.v(TAG, "now = " + c.getTimeInMillis());
        Calendar worldCalendar = Util.getTime(timezone);
        Calendar tempCalendar = Util.getTime(timezone);
        long deltaTime;

        int worldHour = worldCalendar.get(Calendar.HOUR_OF_DAY);
        if (worldHour >= 6 && worldHour < 18) {
            tempCalendar.set(Calendar.HOUR_OF_DAY, 18);
            tempCalendar.set(Calendar.MINUTE, 0);
            tempCalendar.set(Calendar.SECOND, 0);
            tempCalendar.set(Calendar.MILLISECOND, 0);
            deltaTime = tempCalendar.getTimeInMillis() - worldCalendar.getTimeInMillis();
        } else if (worldHour >= 18) {
            tempCalendar.set(Calendar.HOUR_OF_DAY, 23);
            tempCalendar.set(Calendar.MINUTE, 59);
            tempCalendar.set(Calendar.SECOND, 59);
            tempCalendar.set(Calendar.MILLISECOND, 0);
            deltaTime = tempCalendar.getTimeInMillis() - worldCalendar.getTimeInMillis() + 1000 + 6 * 60 * 60 * 1000;
        } else {
            tempCalendar.set(Calendar.HOUR_OF_DAY, 6);
            tempCalendar.set(Calendar.MINUTE, 0);
            tempCalendar.set(Calendar.SECOND, 0);
            tempCalendar.set(Calendar.MILLISECOND, 0);
            deltaTime = tempCalendar.getTimeInMillis() - worldCalendar.getTimeInMillis();
        }
        LogUtil.v(TAG, "deltaTime = " + deltaTime);

        Intent intent = new Intent(context, WeatherWidget.class);
        intent.setAction(WeatherWidgetAction.ACTION_ALARM_TIME_UP);
        PendingIntent sender = PendingIntent.getBroadcast(context, 1, intent, 0);

        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        am.setWindow(AlarmManager.RTC_WAKEUP, c.getTimeInMillis() + deltaTime, FIVE_MIN_IN_MS, sender);
    }

    public static void stopAlarm(Context context) {
        LogUtil.v(TAG);
        Intent intent = new Intent(context, WeatherWidget.class);
        PendingIntent sender = PendingIntent.getBroadcast(context, 1, intent, 0);

        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        am.cancel(sender);
    }
}
