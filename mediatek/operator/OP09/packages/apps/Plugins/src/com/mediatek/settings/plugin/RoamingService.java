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

package com.mediatek.settings.plugin;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.provider.Telephony.SIMInfo;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.Collections;
import java.util.List;
import java.util.Locale;

import libcore.icu.TimeZoneNames;

public class RoamingService extends Service {
    private static final String TAG = "RoamingService";

    private static final String TIME_ZONE_PACKAGE_NAME = "com.mediatek.op09.plugin";
    private static final String TIME_ZONE_ACTIVITY_NAME = "com.mediatek.settings.plugin.TimeZoneActivity";
    private TelephonyManagerEx mTelephonyManagerEx;
    private NotificationManager mNM = null;

    private int mServiceState;
    private int mEVDOModemSlot;
    private int mSimStat;
    private int mCardType;
    private Context mContext;
    private ITelephony mITelephony;
    private ITelephonyEx mITelephonyEx;
    private boolean mIsFirstBoot = true;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        Xlog.d(TAG, "onCreate");
        super.onCreate();
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        mITelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        mITelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));

        mContext = getApplicationContext();
        mEVDOModemSlot = getExternalModemSlot();
        mTelephonyManagerEx.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE, mEVDOModemSlot);
    }

    @Override
    public void onDestroy() {
        Xlog.d(TAG, "onDestroy");
        mTelephonyManagerEx.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE, mEVDOModemSlot);
        removeNoServiceDialog();
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            Xlog.d(TAG, "onServiceStateChanged serviceState=" + serviceState);
            mServiceState = serviceState.getState();
            /// M: LEGO refactory start @{
            mSimStat = mTelephonyManagerEx.getSimState(mEVDOModemSlot);
            /// @}
            Xlog.d(TAG, "onServiceStateChanged mSimStat=" + mSimStat);
            if (mSimStat == TelephonyManager.SIM_STATE_ABSENT) {
                Xlog.w(TAG, "Sim absent ..");
                return;
            }
            try {
                if (mITelephony != null) {
                    mCardType = mITelephonyEx.getInternationalCardType(mEVDOModemSlot);
                }
            } catch (android.os.RemoteException ex) {
                Xlog.e(TAG, "error to getInternationalCardType ");
                return;
            }

            // Only works for dual-mode SIM card
            Xlog.d(TAG, "onServiceStateChanged mCardType=" + mCardType);
            if (mCardType != InternationalRoamingConstants.CARD_TYPE_DUAL_MODE) {
                Xlog.w(TAG, "Only works for dual-mode SIM card..return");
                return;
            }

            // Registered on network
            if (serviceState.getRegState() == ServiceState.REGISTRATION_STATE_HOME_NETWORK
                    || serviceState.getRegState() == ServiceState.REGISTRATION_STATE_ROAMING) {
                Xlog.v(TAG, "Registered on network...");
                mRegisteredNetwork = true;
                removeNoServiceDialog();
                String mccMnc = serviceState.getOperatorNumeric();
                Xlog.v(TAG, "serviceState.getOperatorNumeric(): " + mccMnc);
                if (mccMnc != null && mccMnc.length() >= 5 && !mccMnc.equals("00000")) {
                    String mcc = mccMnc.substring(0, 3);
                    String mnc = mccMnc.substring(3, 5);
                    Xlog.v(TAG, "serviceState.getOperatorNumeric() mcc: " + mcc + "mnc: " + mnc);
                    String latestMCCMNC = PreferenceManager.getDefaultSharedPreferences(RoamingService.this).getString(
                            "ct_latest_mccmnc", null);
                    // store the latest MCCMNC
                    PreferenceManager.getDefaultSharedPreferences(RoamingService.this).edit().putString("ct_latest_mccmnc",
                            mccMnc).commit();
                    /*
                     * In international roaming status, if the device is registered to CDMA network after
                     * booting,"Time dispaly mode" dialogue should pop up automatically.
                     */
                    if (mIsFirstBoot && mCardType == PhoneConstants.PHONE_TYPE_CDMA && isInternationalRoamingStatus()) {
                        Xlog.v(TAG, "the device is registered to CDMA network...");
                        notifyTimeZoneRecommend(false);
                        mIsFirstBoot = false;
                        return;
                    }
                    if (latestMCCMNC != null && !isSupportNitz() && isGgStandBy()) {
                        Xlog.v(TAG, "Register g network and don't support Nitz...");
                        String latestMcc = latestMCCMNC.substring(0, 3);
                        String latestMnc = latestMCCMNC.substring(3, 5);
                        if (mccMnc.startsWith(latestMcc)) {
                            Xlog.v(TAG, "MCC isn't changed mcc: " + mcc);
                            // mcc isn't changed, and mnc changed, show time zone recommend notification
                            if (!mccMnc.endsWith(latestMnc)) {
                                showTimeZoneRecommendInNotificationBar();
                            }
                        } else {
                            Xlog.v(TAG, "MCC is changed, last mccmnc: " + latestMCCMNC + ", new mccmnc: " + mccMnc);
                            // Handle several time zones in one county,and pop up dialog
                            if (isMultipltTimeZone(Integer.valueOf(mcc))) {
                                // if several time zones in this county
                                // Enter time zone recommend(show notification)
                                // showTimeZoneRecommendInNotificationBar();
                                notifyTimeZoneRecommend(false);
                            } else {
                                // 1. update current time zone and time
                                // 2. pop up time choose dialog --> CT comment needn't pop up dialog here
                                MccTable.updateMccMncConfiguration(RoamingService.this, mccMnc);
                            }
                        }
                    }
                    Xlog.v(TAG, "Latest MCCMNC: " + latestMCCMNC + ", new mccmnc: " + mccMnc);
                }
            }
        }
    };

    private int getExternalModemSlot() {
        int slot = 0;
        slot = SystemProperties.getInt("ril.external.md", 0);
        Xlog.d(TAG, "getExternalModemSlot slot=" + slot);
        return slot - 1;
    }

    private boolean isGgStandBy() {
        List<SIMInfo> insertedSimList = SIMInfo.getInsertedSIMList(mContext);
        if (insertedSimList.size() >= 2) {
            if (insertedSimList.get(0).mSlot > insertedSimList.get(1).mSlot) {
                Collections.swap(insertedSimList, 0, 1);
            }
            int slot1PhoneType = mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_1);
            int slot2PhoneType = mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_2);
            // GG dual stand by
            Xlog.d(TAG, "GG dual stand by," + ",slot1PhoneType :" + slot1PhoneType);
            Xlog.d(TAG, "GG dual stand by," + ",slot2PhoneType :" + slot2PhoneType);
            if ((slot1PhoneType == PhoneConstants.PHONE_TYPE_GSM && slot2PhoneType == PhoneConstants.PHONE_TYPE_GSM)) {
                return true;
            }
        } else if (insertedSimList.size() == 1) {
            if (insertedSimList.get(0).mSlot == PhoneConstants.GEMINI_SIM_1) {
                int slot1PhoneType = mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_1);
                Xlog.d(TAG, "slot1PhoneType :" + slot1PhoneType);
                if (slot1PhoneType == PhoneConstants.PHONE_TYPE_GSM) { // G net and not support NITZ
                    return true;
                }
            } else {
                int slot2PhoneType = mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_2);
                Xlog.d(TAG, "slot2PhoneType :" + slot2PhoneType);
                if (slot2PhoneType == PhoneConstants.PHONE_TYPE_GSM) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isSupportNitz() {
        List<SIMInfo> insertedSimList = SIMInfo.getInsertedSIMList(mContext);
        if (insertedSimList.size() >= 2) {
            if (insertedSimList.get(0).mSlot > insertedSimList.get(1).mSlot) {
                Collections.swap(insertedSimList, 0, 1);
            }
            String sim1Nitz = SystemProperties.get("gsm.nitz.time", ""); // Add Slot1 as parameter
            String sim2Nitz = SystemProperties.get("gsm.nitz.time.2", ""); // Add Slot2 as parameter
            Xlog.d(TAG, "GG dual stand by, sim1Nitz is :" + sim1Nitz);
            Xlog.d(TAG, "GG dual stand by, sim2Nitz is :" + sim2Nitz);
            if ((("".equals(sim1Nitz)) || ("".equals(sim1Nitz)))) { // Slot1 or Slot2 not support NITZ
                return false;
            }
        } else if (insertedSimList.size() == 1) {
            if (insertedSimList.get(0).mSlot == PhoneConstants.GEMINI_SIM_1) {
                String sim1Nitz = SystemProperties.get("gsm.nitz.time", ""); // Add Slot1 as parameter
                Xlog.d(TAG, "sim1Nitz is :" + sim1Nitz);
                if ("".equals(sim1Nitz)) { // G net and not support NITZ
                    return false;
                }
            } else {
                String sim2Nitz = SystemProperties.get("gsm.nitz.time.2", ""); // Add Slot2 as parameter
                Xlog.d(TAG, "sim2Nitz is :" + sim2Nitz);
                if ("".equals(sim2Nitz)) {
                    return false;
                }
            }
        } else {
            return false;
        }
        return true;
    }

    /**
     * Show time zone recommend in notification bar
     */
    private void showTimeZoneRecommendInNotificationBar() {
        String notificationTitle = getString(R.string.time_zone_recommend_title);
        String notificationSummary = getString(R.string.time_zone_recommend_notify_summary);
        if (mNM == null) {
            mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        }
        Notification notification = new Notification(R.drawable.ic_notification_time_zone_recommendation, notificationTitle,
                System.currentTimeMillis());
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        /** The PendingIntent to launch our activity if the user selects this notification */
        PendingIntent contentIntent = null;
        Intent backIntent = new Intent();
        backIntent.setComponent(new ComponentName(TIME_ZONE_PACKAGE_NAME, TIME_ZONE_ACTIVITY_NAME));
        backIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);

        if (getPackageManager().resolveActivity(backIntent, 0) != null) {
            contentIntent = PendingIntent.getActivity(this, 0, backIntent, 0);
        } else {
            Xlog.e(TAG, "Could not find OP09 plugin.TimeZoneActivity.");
        }
        notification.setLatestEventInfo(this, notificationTitle, notificationSummary, contentIntent);
        if (mNM != null) {
            mNM.notify(R.drawable.ic_notification_time_zone_recommendation, notification);
        }
    }

    private boolean isMultipltTimeZone(int mcc) {
        String iso = MccTable.countryCodeForMcc(mcc);
        String language = MccTable.defaultLanguageForMcc(mcc);
        if (iso == null) {
            return false;
        } else {
            Locale locale;
            if (language == null) {
                locale = new Locale(iso);
            } else {
                locale = new Locale(language, iso);
            }
            String[] tz = TimeZoneNames.forLocale(locale);
            for (int i = 0; i < tz.length; i++) {
                Xlog.d(TAG, "tz[" + i + "]=" + tz[i]);
            }
            if (tz.length < 2) {
                Xlog.d(TAG, "isMultipltTimeZone false");
                return false;
            }
            Xlog.d(TAG, "isMultipltTimeZone true");
            return true;
        }
    }

    private boolean isInternationalRoamingStatus() {
        boolean isRoaming = false;
        List<SIMInfo> insertedSimList = SIMInfo.getInsertedSIMList(mContext);
        if (insertedSimList.size() == 2) {
            if (insertedSimList.get(0).mSlot > insertedSimList.get(1).mSlot) {
                Collections.swap(insertedSimList, 0, 1);
            }
        }
        if (insertedSimList != null && insertedSimList.size() == 2) { // Two SIMs inserted
            isRoaming = mTelephonyManagerEx.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1);
        } else if (insertedSimList != null && insertedSimList.size() == 1) { // One SIM inserted
            SIMInfo simInfo = insertedSimList.get(0);
            isRoaming = mTelephonyManagerEx.isNetworkRoaming(simInfo.mSlot);
        }
        Xlog.d(TAG, "is International Roaming Status: " + isRoaming);
        return isRoaming;
    }

    private AlertDialog mNoServiceDialog;
    private boolean mRegisteredNetwork = true;

    private void removeNoServiceDialog() {
        if (mNoServiceDialog != null && mNoServiceDialog.isShowing()) {
            Xlog.d(TAG, "Remove No Service Dialog.");
            mNoServiceDialog.dismiss();
        }
    }

    private void notifyTimeZoneRecommend(boolean isMccChange) {
        if (isMccChange) {
            RoamingNotify.getInstance(this).showTimeZoneRecommendInNotificationBar();
        } else {
            RoamingNotify.getInstance(this).showTimeZoneChooseDialog();
        }
    }
}
