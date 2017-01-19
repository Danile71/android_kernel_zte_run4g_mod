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

package com.mediatek.csm;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;
import android.view.WindowManager;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.CellConnService.CellConnMgr;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;

public class CustomerServiceFragment extends PreferenceFragment {
    static final String LOG_TAG = "CustomerService";
    static final boolean DEBUG = true;

    /*
     * CAUTION: Must be consistent with Phone app Constants definitions
     */
    public static final String EXTRA_SLOT_ID = "com.android.phone.extra.slot";
    public static final String SCHEME_TEL = "tel://";
    public static final int INVALID_SIM_ID = -1;

    // Broadcast for sending sms
    public static final String ACTION_SENDTO_SEND = "com.mediatek.ct.csm.action.ACTION_SEND";
    public static final String ACTION_SENDTO_DELIVERED = "com.mediatek.ct.csm.action.ACTION_DELIVERED";

    // Must be consistent with the "key-value" in res/xml/preferences.xml
    private static final String HOTLINE = "pref_hotline";
    private static final String SELF_HOTLINE = "pref_self_hotline";
    private static final String EHALL_MESSAGE = "pref_message";
    private static final String EHALL_WAP = "pref_wap";
    private static final String SUBSCRIBE = "pref_subscribe";
    private static final String UNSUBSCRIBE = "pref_unsubscribe";
    private static final String PRL_UPDATE = "pref_prl";

    // Message handling
    private static final int MSG_INVALID = -1;
    private static final int MSG_HOTLINE = 0x0001;
    private static final int MSG_SELF_HOTLINE = 0x0002;
    private static final int MSG_EHALL_MESSAGE = 0x0003;
    private static final int MSG_EHALL_WAP = 0x0004;
    private static final int MSG_SUBSCRIBE = 0x0005;
    private static final int MSG_UNSUBSCRIBE = 0x0006;
    private static final int MSG_PRL_UPDATE = 0x0007;

    private static HashMap<String, Integer> sMap = new HashMap<String, Integer>();

    /*
     * CAUTION: Must be consistent with MSG_XXX above
     */
    static {
        sMap.put(HOTLINE, MSG_HOTLINE);
        sMap.put(SELF_HOTLINE, MSG_SELF_HOTLINE);
        sMap.put(EHALL_MESSAGE, MSG_EHALL_MESSAGE);
        // sMap.put(EHALL_WAP, MSG_EHALL_WAP);
        sMap.put(SUBSCRIBE, MSG_SUBSCRIBE);
        sMap.put(UNSUBSCRIBE, MSG_UNSUBSCRIBE);
        sMap.put(PRL_UPDATE, MSG_PRL_UPDATE);
    }

    private Context mContext;
    private ProgressDialog mProgress;
    private CellConnMgr mConnMgr;
    private TelephonyManagerEx mTelephonyManager;

    /*
     * To avoid sending duplicated messages
     */
    private int mCurrentMessage = MSG_INVALID;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            if (intent != null) {
                final String action = intent.getAction();
                final int resultCode = getResultCode();
                if (DEBUG) {
                    Xlog.d(LOG_TAG, "broadcast action=" + action + ", resultCode=" + resultCode);
                }
                if (action != null && ACTION_SENDTO_SEND.equals(action)) {
                    hideProgress();
                    // Now you can touch it again
                    mCurrentMessage = MSG_INVALID;
                    if (resultCode == Activity.RESULT_OK) {
                        showInfo(R.string.send_ok);
                    } else {
                        showInfo(R.string.send_fail);
                        Xlog.w(LOG_TAG, "Send sms failed! resultCode=" + resultCode);
                    }
                }
            }
        }
    };

    final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // get the right SIM according to CT spec
            final int simId = getSimId();

            switch (msg.what) {
                case MSG_HOTLINE:
                mCurrentMessage = MSG_INVALID;
                if (simId != INVALID_SIM_ID) {
                    call(simId, R.string.hotline_addr);
                } else {
                    showInfo(R.string.sim_not_available);
                }
                break;

            case MSG_SELF_HOTLINE:
                mCurrentMessage = MSG_INVALID;
                if (simId != INVALID_SIM_ID) {
                    call(simId, R.string.self_hotline_addr);
                } else {
                    showInfo(R.string.sim_not_available);
                }
                break;

            case MSG_EHALL_MESSAGE:
                if (simId != INVALID_SIM_ID) {
                    if (needToCheckConnectState(simId)) {
                        final int request = getConnectRequest(simId);
                        handleConnection(simId, request, mRunnable);
                    } else {
                        sendSms(simId, R.string.self_hotline_addr, R.string.ehall_message_data);
                    }
                } else {
                    showInfo(R.string.sim_not_available);
                    mCurrentMessage = MSG_INVALID;
                }
                break;

            case MSG_SUBSCRIBE:
                if (simId != INVALID_SIM_ID) {
                    if (needToCheckConnectState(simId)) {
                        final int request = getConnectRequest(simId);
                        handleConnection(simId, request, mRunnable);
                    } else {
                        sendSms(simId, R.string.self_hotline_addr, R.string.subscribe_data);
                    }
                } else {
                    showInfo(R.string.sim_not_available);
                    mCurrentMessage = MSG_INVALID;
                }
                break;

            case MSG_UNSUBSCRIBE:
                if (simId != INVALID_SIM_ID) {
                    if (needToCheckConnectState(simId)) {
                        final int request = getConnectRequest(simId);
                        handleConnection(simId, request, mRunnable);
                    } else {
                        sendSms(simId, R.string.self_hotline_addr, R.string.unsubscribe_data);
                    }
                } else {
                    showInfo(R.string.sim_not_available);
                    mCurrentMessage = MSG_INVALID;
                }
                break;

            case MSG_PRL_UPDATE:
                if (simId != INVALID_SIM_ID) {
                    if (needToCheckConnectState(simId)) {
                        final int request = getConnectRequest(simId);
                        handleConnection(simId, request, mRunnable);
                    } else {
                        sendSms(simId, R.string.prl_addr, R.string.prl_data);
                    }
                } else {
                    showInfo(R.string.sim_not_available);
                    mCurrentMessage = MSG_INVALID;
                }
                break;

            default:
                mCurrentMessage = MSG_INVALID;
                break;
        }
    }
    };

    private final Runnable mRunnable = new Runnable() {
        @Override
        public void run() {
            // hideProgress();
            final int result = mConnMgr.getResult();
            final int slot = mConnMgr.getPreferSlot();
            Xlog.d(LOG_TAG, "run, result = " + CellConnMgr.resultToString(result) + " slot = "
                    + slot);
            // Now you can handle another message
            mCurrentMessage = MSG_INVALID;
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onCreate()" + this);
        }
        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preferences);
        mContext = getActivity();
        mProgress = new ProgressDialog(mContext);

        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_SENDTO_SEND);
        mContext.registerReceiver(mReceiver, filter);

        mConnMgr = new CellConnMgr();
        mConnMgr.register(mContext);

        mTelephonyManager = new TelephonyManagerEx(mContext);
    }

    @Override
    public void onDestroy() {
        mContext.unregisterReceiver(mReceiver);
        mConnMgr.unregister();
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onDestroy() " + this);
        }
        super.onDestroy();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference != null && sMap.keySet().contains(preference.getKey())) {
            if (DEBUG) {
                Xlog.w(LOG_TAG, "handle click " + preference.getKey());
            }
            final int message = sMap.get(preference.getKey());

            if (mCurrentMessage != message) {
                mCurrentMessage = message;
                Message msg = mHandler.obtainMessage(message);
                mHandler.sendMessage(msg);
            } else {
                Xlog.w(LOG_TAG, "Ignore duplicated message: " + message);
            }
            return true;
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    public int getSimId() {
        // We expect that CT sim is plugged in slot0, but it also can be plugged
        // in slot1 when roaming
        int simId = INVALID_SIM_ID;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            Xlog.d(LOG_TAG, "Gemini SIM card!");
            final int sim01 = PhoneConstants.GEMINI_SIM_1;
            final int sim02 = PhoneConstants.GEMINI_SIM_2;
            final int type01 = PhoneManager.getPhoneType(sim01);
            final int type02 = PhoneManager.getPhoneType(sim02);
            if (PhoneManager.isSimInserted(sim01)) {
                Xlog.d(LOG_TAG, "GEMINI_SIM_1 is inserted and type is " + type01);
                if (type01 == TelephonyManager.PHONE_TYPE_CDMA
                        || (mTelephonyManager.isNetworkRoaming(sim01)
                                && (type01 == TelephonyManager.PHONE_TYPE_CDMA
                                || type01 == TelephonyManager.PHONE_TYPE_GSM))) {
                    return sim01;
                }
            } else if (PhoneManager.isSimInserted(sim02)
                    && mTelephonyManager.isNetworkRoaming(sim02)) {
                Xlog.d(LOG_TAG, "GEMINI_SIM_2 is inserted and type is " + type02);
                if (type02 == TelephonyManager.PHONE_TYPE_CDMA
                        || type02 == TelephonyManager.PHONE_TYPE_GSM) {
                    return sim02;
                }
            }
        } else {
            Xlog.d(LOG_TAG, "Single SIM card!");
            final int sim = PhoneConstants.GEMINI_SIM_1;
            final int type = PhoneManager.getPhoneType(sim);
            if (!PhoneManager.isSimInserted(sim)) {
                Xlog.w(LOG_TAG, "SIM card is not inserted!");
                simId = INVALID_SIM_ID;
            } else if (type != TelephonyManager.PHONE_TYPE_GSM
                    && type != TelephonyManager.PHONE_TYPE_CDMA) {
                Xlog.w(LOG_TAG, "Not CDMA! type=" + type);
            }
        }

        return simId;
    }

    public boolean needToCheckConnectState(int simId) {
        boolean ret = false;
        if (!PhoneManager.isRadioOn(simId)
                || PhoneManager.getSimState(simId) != TelephonyManager.SIM_STATE_READY) {
            Xlog.d(LOG_TAG, "needToCheckConnectState: The sim " + simId
                    + " is not raido on or not ready!");
            ret = true;
        }

        return ret;
    }

    public int getConnectRequest(int simId) {
        int ret = CellConnMgr.REQUEST_TYPE_UNKNOWN;
        final int state = PhoneManager.getSimState(simId);

        Xlog.d(LOG_TAG, "Get the sim " + simId + " state is " + state);

        switch (state) {
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED:
                ret = CellConnMgr.REQUEST_TYPE_FDN;
                break;

            case TelephonyManager.SIM_STATE_PIN_REQUIRED:
            case TelephonyManager.SIM_STATE_PUK_REQUIRED:
                ret = CellConnMgr.REQUEST_TYPE_SIMLOCK;
                break;

            case TelephonyManager.SIM_STATE_ABSENT:
            case TelephonyManager.SIM_STATE_UNKNOWN:
                ret = CellConnMgr.REQUEST_TYPE_ROAMING;
                break;
            // No need to handle CellConnection
            case TelephonyManager.SIM_STATE_READY:
            default:
                break;
        }
        return ret;
    }

    private void handleConnection(int simId, int request, Runnable r) {
        final int result = mConnMgr.handleCellConn(simId, request, r);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "handleConnection: result = " + CellConnMgr.resultToString(result)
                    + " slot = " + simId);
        }
        // Now you can handle another message
        mCurrentMessage = MSG_INVALID;
        if (result == CellConnMgr.RESULT_ABORT) {
            showInfo(R.string.waiting_networks);
        }
    }

    public boolean isSimAvailable(int simId) {
        boolean ret = true;
        final int state = PhoneManager.getSimState(simId);
        switch (state) {
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED:
                Xlog.d(LOG_TAG, "sim " + simId + " is SIM_STATE_NETWORK_LOCKED");
                break;

            case TelephonyManager.SIM_STATE_PIN_REQUIRED:
                Xlog.d(LOG_TAG, "sim " + simId + " is SIM_STATE_PIN_REQUIRED");
                break;

            case TelephonyManager.SIM_STATE_PUK_REQUIRED:
                Xlog.d(LOG_TAG, "sim " + simId + " is SIM_STATE_PUK_REQUIRED");
                break;

            case TelephonyManager.SIM_STATE_READY:
                Xlog.d(LOG_TAG, "sim " + simId + " is SIM_STATE_READY");
                break;

            case TelephonyManager.SIM_STATE_ABSENT:
                Xlog.d(LOG_TAG, "sim " + simId + " is SIM_STATE_ABSENT");
                ret = false;
                break;

            default:
                Xlog.w(LOG_TAG, "sim card " + simId + " is not available");
                ret = false;
                break;
        }
        return ret;
    }

    private void sendSms(int simId, int addressId, int msgId) {
        if (mContext == null) {
            Xlog.e(LOG_TAG, "sendSms: No context!");
            return;
        }
        Xlog.d(LOG_TAG, "sendSms by sim " + simId);

        final String address = getResources().getString(addressId);
        final String message = getResources().getString(msgId);
        PendingIntent sentIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(
                ACTION_SENDTO_SEND), 0);
        PendingIntent deliveryIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(
                ACTION_SENDTO_DELIVERED), 0);

        PhoneManager.sendTextMessage(address, message, simId, sentIntent, deliveryIntent);

        // Show the progress dialog just only a message has been sent
        showProgress(makeSmsDialogTitle(address, message));
    }

    private void call(int simId, int addressId) {
        if (mContext == null) {
            Xlog.e(LOG_TAG, "call: No context!");
            return;
        }

        Xlog.d(LOG_TAG, "call: call with sim " + simId);

        final String address = getResources().getString(addressId);
        Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse(SCHEME_TEL + address));
        // For gemini dual SIM if needed
        intent.putExtra(EXTRA_SLOT_ID, simId);
        startActivity(intent);
    }

    private void showInfo(int resId) {
        if (mContext != null) {
            Toast.makeText(mContext, resId, Toast.LENGTH_SHORT).show();
        }
    }

    private void showProgress(String title) {
        hideProgress();
        if (mProgress != null) {
            Xlog.d(LOG_TAG, "showProgress: create progress dialog");
            mProgress.setTitle(title);
            mProgress.setIndeterminate(true);
            mProgress.setCancelable(false);
            mProgress.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
            mProgress.show();
        }
    }

    private void hideProgress() {
        if (mProgress != null && mProgress.isShowing()) {
            Xlog.d(LOG_TAG, "hideProgress: dismiss progress dialog");
            mProgress.dismiss();
        }
    }

    private String makeSmsDialogTitle(String address, String message) {
        final String sending = getResources().getString(R.string.sending);
        final String to = getResources().getString(R.string.to);
        StringBuilder builder = new StringBuilder();
        builder.append(sending);
        builder.append(message);
        builder.append(to);
        builder.append(address);
        return builder.toString();
    }

    private String makeProgressDialogTitle(int resId) {
        return getResources().getString(resId);
    }
}
