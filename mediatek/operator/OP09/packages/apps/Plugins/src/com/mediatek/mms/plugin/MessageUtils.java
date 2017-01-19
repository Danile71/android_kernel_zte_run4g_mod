/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.provider.Telephony.SIMInfo;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.text.format.Time;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.TelephonyManagerEx;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Formatter;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * @author mtk81191
 *
 */
public class MessageUtils {

    private static final int SINGLE_SIM_CARD = 1;
    private static final int DOUBLE_SIM_CARD = 2;

    private static final String TIMEZONE_ID_BEIJING = "Asia/Shanghai";

    private static final String TAG = "OP09MessageUtils";

    public static final int[] SEND_BUTTON_DRAWABLE_RESOURCE_ID_C_BIG = new int[] {
        R.drawable.ct_send_1_big_orange,
        R.drawable.ct_send_1_big_orange,
        R.drawable.ct_send_1_big_orange,
        R.drawable.ct_send_1_big_orange
    };

    public static final int[] SEND_BUTTON_DRAWABLE_RESOURCE_ID_C_SMALL = new int[] {
        R.drawable.ct_send_1_small_orange,
        R.drawable.ct_send_1_small_orange,
        R.drawable.ct_send_1_small_orange,
        R.drawable.ct_send_1_small_orange
    };

    public static final int[] SEND_BUTTON_DRAWABLE_RESOURCE_ID_G_BIG = new int[] {
        R.drawable.ct_send_2_big_blue,
        R.drawable.ct_send_2_big_blue,
        R.drawable.ct_send_2_big_blue,
        R.drawable.ct_send_2_big_blue
    };

    public static final int[] SEND_BUTTON_DRAWABLE_RESOURCE_ID_G_SMALL = new int[] {
        R.drawable.ct_send_2_small_blue,
        R.drawable.ct_send_2_small_blue,
        R.drawable.ct_send_2_small_blue,
        R.drawable.ct_send_2_small_blue
    };

    public static final int[] SEND_BUTTON_ACTIVIATE_RESOURCE_ID_C_SMALL = new int[]{
        R.drawable.ct_send_1_small_orange_activated,
        R.drawable.ct_send_1_small_orange_activated,
        R.drawable.ct_send_1_small_orange_activated,
        R.drawable.ct_send_1_small_orange_activated
    };
    public static final int[] SEND_BUTTON_ACTIVIATE_RESOURCE_ID_C_BIG = new int[]{
        R.drawable.ct_send_1_big_orange_activated,
        R.drawable.ct_send_1_big_orange_activated,
        R.drawable.ct_send_1_big_orange_activated,
        R.drawable.ct_send_1_big_orange_activated
    };
    public static final int[] SEND_BUTTON_ACTIVIATE_RESOURCE_ID_G_SMALL = new int[]{
        R.drawable.ct_send_2_small_blue_activated,
        R.drawable.ct_send_2_small_blue_activated,
        R.drawable.ct_send_2_small_blue_activated,
        R.drawable.ct_send_2_small_blue_activated
    };
    public static final int[] SEND_BUTTON_ACTIVIATE_RESOURCE_ID_G_BIG = new int[]{
        R.drawable.ct_send_2_big_blue_activated,
        R.drawable.ct_send_2_big_blue_activated,
        R.drawable.ct_send_2_big_blue_activated,
        R.drawable.ct_send_2_big_blue_activated
    };

    public static String getShortTimeString(Context context, long time) {
        int formatFlags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
            | DateUtils.FORMAT_CAP_AMPM;
        formatFlags |= DateUtils.FORMAT_SHOW_TIME;
        return formatDateTime(context, time, formatFlags);
    }

    private static List<SIMInfo> mListSimInfo;

    public static String formatDateTime(Context context, long time, int formatFlags) {
        if (!isInternationalRoamingStatus(context)) {
            MmsLog.d(TAG, "formatDateTime Default");
            return DateUtils.formatDateTime(context, time, formatFlags);
        }
        int localNum = Settings.System.getInt(context.getContentResolver(), Settings.System.CT_TIME_DISPLAY_MODE, 0);
        if (localNum == 0) {
            Formatter f = new Formatter(new StringBuilder(50), Locale.CHINA);
            String str = DateUtils.formatDateRange(context, f, time, time, formatFlags, TIMEZONE_ID_BEIJING).toString();
            MmsLog.d(TAG, "FormateDateTime  Time:" + time + "\t formatFlags:" + formatFlags
                    + "\tTimeZone:" + TIMEZONE_ID_BEIJING);
            return str;
        } else {
            MmsLog.d(TAG, "FormateDateTime; time display mode: LOCAL");
            return DateUtils.formatDateTime(context, time, formatFlags);
        }
    }

    public static boolean isInternationalRoamingStatus(Context context) {
        TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
        boolean isRoaming = false;
        if (mSimInfoMap == null) {
            initSiminfoList(context);
        }
        if (mSimInfoMap != null && mSimInfoMap.size() == DOUBLE_SIM_CARD) { // Two SIMs inserted
            // isRoaming = isCdmaRoaming();
            isRoaming = telephonyManagerEx.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1);
        } else if (mSimInfoMap != null && mSimInfoMap.size() == SINGLE_SIM_CARD) { // One SIM inserted
            SimInfoManager.SimInfoRecord sir = mSimInfoMap.values().iterator().next();
            isRoaming = telephonyManagerEx.isNetworkRoaming(sir.mSimSlotId);
        } else { // Error: no SIM inserted
            MmsLog.e(TAG, "There is no SIM inserted!");
        }
        MmsLog.d(TAG, "isInternationalRoamingStatus:" + isRoaming);
        return isRoaming;
    }

    public static boolean isInternationalRoamingStatus(Context context, int slotId) {
        TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
        boolean isRoaming = false;
        if (mSimInfoMap == null) {
            initSiminfoList(context);
        }
        if (mSimInfoMap.size() <= 0
                || (mSimInfoMap.size() == SINGLE_SIM_CARD && slotId > 0)
                || (mSimInfoMap.size() == DOUBLE_SIM_CARD && slotId > 1)) {
            MmsLog.e(TAG, "MessageUtils.isInternationalRoamingStatus(): Wrong slotId!");
            return false;
        }

        isRoaming = telephonyManagerEx.isNetworkRoaming(slotId);
        MmsLog.d(TAG, "isInternationalRoamingStatus() isRoaming: " + isRoaming);
        return isRoaming;
    }

    public static boolean isInternationalRoamingStatusBySimid(Context context, int simId) {
        if (mSimInfoMap == null) {
            /// This step will try to init mSimInfoMap.
            initSiminfoList(context);
        }
        /// check the mSimInfoMap value again.
        if (mSimInfoMap == null) {
            return false;
        }
        SimInfoManager.SimInfoRecord sir = getSimInfoBySimId(context, simId);
        if (sir == null) {
            return false;
        }
        TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
        boolean isRoaming = false;
        isRoaming = telephonyManagerEx.isNetworkRoaming(sir.mSimSlotId);
        MmsLog.d(TAG, "isInternationalRoamingStatus() isRoaming: " + isRoaming);
        return isRoaming;
    }

    private static String removeYearFromFormat(String allFormatStr) {
        if (TextUtils.isEmpty(allFormatStr)) {
            return allFormatStr;
        }
        String finalStr = "";
        int yearIndex = allFormatStr.indexOf("y");
        int monthIndex = allFormatStr.indexOf("M");
        int dayIndex = allFormatStr.indexOf("d");
        if (yearIndex >= 0) {
            if (yearIndex > monthIndex) {
                finalStr = allFormatStr.substring(0, yearIndex).trim();
            } else if (monthIndex > dayIndex) {
                finalStr = allFormatStr.substring(dayIndex, allFormatStr.length()).trim();
            } else {
                finalStr = allFormatStr.substring(monthIndex, allFormatStr.length()).trim();
            }
            if (finalStr.endsWith(",") || finalStr.endsWith("/") || finalStr.endsWith(".") || finalStr.endsWith("-")) {
                finalStr = finalStr.substring(0, finalStr.length() - 1);
            }
            return finalStr;
        } else {
            return allFormatStr;
        }
    }

    public static String formatDateOrTimeStampStringWithSystemSetting(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int formatFlags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
            | DateUtils.FORMAT_CAP_AMPM;
        SimpleDateFormat sdf = (SimpleDateFormat) (DateFormat.getDateFormat(context));
        String allDateFormat = sdf.toPattern();

        if (fullFormat) {
            String timeStr = getShortTimeString(context, when);
            String dateStr = DateFormat.format(allDateFormat, when).toString();
            formatFlags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
            String defaultDateStr = formatDateTime(context, when, formatFlags);
            if (defaultDateStr.indexOf(":") > 5) {
                return dateStr + " " + timeStr;
            }
            return timeStr + " " + dateStr;
        }
        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            formatFlags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
            return DateFormat.format(allDateFormat, when).toString();
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            formatFlags |= DateUtils.FORMAT_SHOW_DATE;
            if ((now.yearDay - then.yearDay) == 1) {
                return context.getString(R.string.str_yesterday);
            } else {
            String dayMonthFormatStr = removeYearFromFormat(allDateFormat);
            return DateFormat.format(dayMonthFormatStr, when).toString();
            }
        } else if (0 <= (now.toMillis(false) - then.toMillis(false))
            && (now.toMillis(false) - then.toMillis(false)) < 60000) {
            return context.getString(R.string.time_now);
        } else {
            // Otherwise, if the message is from today, show the time.
            formatFlags |= DateUtils.FORMAT_SHOW_TIME;
        }
        return formatDateTime(context, when, formatFlags);
    }

    public static String handleMissedParts(SmsMessage[] parts) {
        if (parts == null || parts.length <= 0) {
            MmsLog.e(TAG, "[fake invalid message array");
            return null;
        }

        int totalPartsNum = parts[0].getUserDataHeader().concatRef.msgCount;

        String[] fakeContents = new String[totalPartsNum];
        for (SmsMessage msg : parts) {
            int seq = msg.getUserDataHeader().concatRef.seqNumber;
            MmsLog.d(TAG, "[fake add segment " + seq);
            fakeContents[seq - 1] = msg.getDisplayMessageBody();
        }
        for (int i = 0; i < fakeContents.length; ++i) {
            if (fakeContents[i] == null) {
                MmsLog.d(TAG, "[fake replace segment " + (i + 1));
                fakeContents[i] = "(...)";
            }
        }

        StringBuilder body = new StringBuilder();
        for (String s : fakeContents) {
            body.append(s);
        }
        return body.toString();
    }

    public static boolean isCDMAType(Context context, int slotId) {
        TelephonyManagerEx tme = new TelephonyManagerEx(context);
        int phoneType = tme.getPhoneType(slotId);
        MmsLog.d(TAG, "isCDMAType() phoneType:" + phoneType);

        return phoneType == PhoneConstants.PHONE_TYPE_CDMA;
    }

    /// M: the key is simId;
    private static Map<Integer, SimInfoManager.SimInfoRecord> mSimInfoMap = null;
    /// M: the key is slotId;
    private static Map<Integer, SimInfoManager.SimInfoRecord> mSimInfoMapBySlotId = null;

    private static BroadcastReceiver mSimReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SIM_INFO_UPDATE)) {
                getSimInfoList(context);
            }
        }
    };

    public static void initSiminfoList(Context context) {
        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        context.registerReceiver(mSimReceiver, intentFilter);
        getSimInfoList(context);
    }

    public static SimInfoManager.SimInfoRecord getSimInfoBySimId(Context ctx, int simId) {
        if (mSimInfoMap == null) {
            initSiminfoList(ctx);
        }
        if (mSimInfoMap != null) {
            return mSimInfoMap.get(simId);
        }
        return null;
    }

    /**
     * Get Sim Info by slot id.
     * @param ctx
     * @param slotId
     * @return
     */
    public static SimInfoManager.SimInfoRecord getSimInfoBySlotId(Context ctx, int slotId) {
        if (slotId < 0) {
            return null;
        }
        if (mSimInfoMapBySlotId == null) {
            initSiminfoList(ctx);
        }
        if (mSimInfoMapBySlotId != null) {
            return mSimInfoMapBySlotId.get(slotId);
        }
        return null;
    }

    public static Map<Integer, SimInfoManager.SimInfoRecord> getSimInfoFromCache(Context ctx) {
        if (mSimInfoMap == null) {
            initSiminfoList(ctx);
        }
        return mSimInfoMap;
    }

    private static void getSimInfoList(Context ctx) {
        if (ctx == null) {
            return;
        }
        List<SimInfoManager.SimInfoRecord> oldSimList = SimInfoManager.getInsertedSimInfoList(ctx);
        if (oldSimList == null) {
            MmsLog.d(TAG, "getSimInfoList: null");
            return;
        }
        mSimInfoMap = new HashMap<Integer, SimInfoManager.SimInfoRecord>();
        mSimInfoMapBySlotId = new HashMap<Integer, SimInfoManager.SimInfoRecord>();
        for (SimInfoManager.SimInfoRecord msir : oldSimList) {
            mSimInfoMap.put((int) msir.mSimInfoId, msir);
            mSimInfoMapBySlotId.put((int) msir.mSimSlotId, msir);
        }
    }

}
