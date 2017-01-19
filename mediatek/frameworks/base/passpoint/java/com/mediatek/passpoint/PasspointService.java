/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.passpoint;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.NotificationManager;
import android.app.Notification;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;

import android.net.wifi.WifiManager;

import android.util.Log;
import android.util.LogPrinter;
import android.util.Slog;

import com.mediatek.common.passpoint.*;
import com.mediatek.internal.R;

import java.util.List;

public class PasspointService extends IPasspointManager.Stub {
    private static final String TAG = "PasspointService";
    private static final String TAG_SPINFO = "passpoint.spinfo";
    private static final String TAG_CONNECTED = "passpoint.connected";
    private static final String TAG_PFAIL = "passpoint.pfail";

    private static PasspointService mServiceInstance;
    private static PasspointStateMachine mPsm;
    private AlarmManager mAlarmManager;
    private NotificationManager mNoficationMgr;
    private Handler mHandler;
    private Object mPublicSync;
    private WifiManager mWifimgr;
    private Context mContext;
    private PendingIntent mPolicyPollIntent;
    private PendingIntent mRemediationPollIntent;

    public static final String ACTION_NETWORK_POLICY_POLL =
        "mediatek.intent.action.PASSPOINT_POLICY_POLL";

    public static final String ACTION_NETWORK_REMEDIATION_POLL =
        "mediatek.intent.action.PASSPOINT_REMEDIATION_POLL";

    public PasspointService(Context context) {

        mContext = context;
        HandlerThread handlerThread = new HandlerThread("PasspointServiceThread");
        handlerThread.start();
        mAlarmManager = (AlarmManager)mContext.getSystemService(Context.ALARM_SERVICE);
        mNoficationMgr = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        mHandler = new MyHandler(handlerThread.getLooper());
        mPublicSync = new Object();
        mWifimgr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        mPsm = new PasspointStateMachine("PasspointSM", mHandler, context);
        mPsm.start();
        //Set default to 2.5 minutes
        //registerPollAlarm( 150 );

        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_NETWORK_POLICY_POLL);
        intentFilter.addAction(ACTION_NETWORK_REMEDIATION_POLL);
        mContext.registerReceiver(mReceiver, intentFilter);
    }

    public void systemReady() {
        //mPpm.readPolicy();
        //registerPollAlarm( 150 );
    }

    private void registerPolicyUpdatePollAlarm(long second) {
        Log.d(TAG, "registerPolicyUpdatePollAlarm:" + Long.toString(second));
        //cancel previous poll
        cancelPolicyPollAlarm();

        mPolicyPollIntent = PendingIntent.getBroadcast(
                                mContext, 0, new Intent(ACTION_NETWORK_POLICY_POLL), 0);

        long intervalMillis = second * 1000;
        //long triggerAtMillis = SystemClock.elapsedRealtime() + intervalMillis; // first go off
        // for plugfest we alarm immediately, remove this when production
        long triggerAtMillis = SystemClock.elapsedRealtime(); // first go off
        /*
         *type:             One of ELAPSED_REALTIME, ELAPSED_REALTIME_WAKEUP}, RTC or RTC_WAKEUP.
         *triggerAtMillis:  time in milliseconds that the alarm should first go off,
         *                  using the appropriate clock (depending on the alarm type).
         *intervalMillis:   interval in milliseconds between subsequent repeats of the alarm.
         *operation:        Action to perform when the alarm goes off; typically comes from IntentSender.getBroadcast().
         */
        mAlarmManager.setRepeating(AlarmManager.ELAPSED_REALTIME, triggerAtMillis ,
                                   intervalMillis, mPolicyPollIntent);
    }

    private void registerRemediationPollAlarm(long second) {
        Log.d(TAG, "registerRemediationPollAlarm:" + Long.toString(second));
        //cancel previous poll
        cancelRemediationPollAlarm();

        mRemediationPollIntent = PendingIntent.getBroadcast(
                                     mContext, 0, new Intent(ACTION_NETWORK_REMEDIATION_POLL), 0);

        long intervalMillis = second * 1000;
        //long triggerAtMillis = SystemClock.elapsedRealtime() + intervalMillis; // first go off
        // for plugfest we alarm immediately, remove this when production
        long triggerAtMillis = SystemClock.elapsedRealtime(); // first go off
        /*
         *type:             One of ELAPSED_REALTIME, ELAPSED_REALTIME_WAKEUP}, RTC or RTC_WAKEUP.
         *triggerAtMillis:  time in milliseconds that the alarm should first go off,
         *                  using the appropriate clock (depending on the alarm type).
         *intervalMillis:   interval in milliseconds between subsequent repeats of the alarm.
         *operation:        Action to perform when the alarm goes off; typically comes from IntentSender.getBroadcast().
         */
        mAlarmManager.setRepeating(AlarmManager.ELAPSED_REALTIME, triggerAtMillis ,
                                   intervalMillis, mRemediationPollIntent);
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive:" + action);

            if (action.equals(ACTION_NETWORK_POLICY_POLL)) {
                mPsm.startNetworkPolicyPoll();
            } else if (action.equals(ACTION_NETWORK_REMEDIATION_POLL)) {
                mPsm.startRemediationPoll();
            }
        }
    };

    public void sendregisterPollAlarm(long second) {
        Long iSecond = new Long(second);
        Message msg = mHandler.obtainMessage(PasspointManager.EVENT_SET_POLICY_POLL_TIMER, iSecond);
        msg.sendToTarget();
    }

    public void cancelPolicyPollAlarm() {
        if (mPolicyPollIntent != null) {
            mAlarmManager.cancel(mPolicyPollIntent);
        }
    }

    public void cancelRemediationPollAlarm() {
        if (mRemediationPollIntent != null) {
            mAlarmManager.cancel(mRemediationPollIntent);
        }
    }

    public void setOsuServiceProvider(String bssid) {
        mPsm.setOsuServiceProvider(bssid);
    }


    public void setSpExLis(String exSplist) {
        mPsm.setSpExLis(exSplist);
    }
    public void addCredential(PasspointCredential pc) {
        mPsm.addCredential(pc);
    }

    public void delCredential(PasspointCredential pc) {
        mPsm.delCredential(pc);
    }
    public void delCredentialbyindex(int index) {
        mPsm.delCredentialbyindex(index);
    }
    public void delAllCredential() {
        mPsm.delAllCredential();
    }
    public boolean addCredentialbyValue(String type, String username, String passwd, String imsi, String mnc, String mcc, String root_ca, String realm, int preferred, String fqdn, String client_ca) {
        return mPsm.addCredentialbyValue(type, username, passwd, imsi,
                                         mnc,mcc,root_ca,realm,preferred,fqdn,client_ca);
    }
    public String dumpCredential() {
        return mPsm.dumpCredential();
    }
    public void installPpsMo(int index) {
        mPsm.installPpsMo(index);
    }

    public void installRootCA() {
        Log.d(TAG, "remove me");
    }

    public void installServerCA() {
        Log.d(TAG, "remove me");
    }

    public void Set8211W(int index) {
        mPsm.Set8211W(index);
    }

    public void clearPolicies() {
        mPsm.clearPolicies();
    }

    public void setPasspointUserPolicies(PasspointPolicy[] policies) {
        synchronized (mPublicSync) {
            mPsm.setPasspointUserPolicies(policies);
        }
    }

    public PasspointPolicy[] getPasspointUserPolicies() {
        synchronized (mPublicSync) {
            return mPsm.getPasspointUserPolicies();
        }
    }

    public List<PasspointServiceProviderInfo> getServiceProviderInfoList() {
        synchronized (mPublicSync) {
            return mPsm.getServiceProviderInfoList();
        }
    }

    public void setPreferCredential(String credname, String wifispfqdn) {
        mPsm.setPreferCredential(credname, wifispfqdn);
    }

    public List<PasspointCredentialInfo> getCredentialInfoList() {
        synchronized (mPublicSync) {
            return mPsm.getCredentialInfoList();
        }
    }

    public void notifySubscriptionProvisionStatus(boolean isReady) {
        mPsm.notifySubscriptionProvisionStatus(isReady);
    }

    private void onHandleProvisionDone(int result, WifiTree tree) {
        Log.d(TAG, "onHandleProvisionDone");
        mPsm.sendProvisionDone(result, tree);
    }

    private void onHandlePreProvisionDone(int result, WifiTree tree) {
        Log.d(TAG, "onHandlePreProvisionDone");
        mPsm.sendPreProvisionDone(result, tree);
    }

    private void onHandleSimProvisionDone(int result, WifiTree tree) {
        Log.d(TAG, "onHandleSimProvisionDone");
        mPsm.sendSimProvisionDone(result, tree);
    }

    private void onHandleProvisionFromFileDone() {
        Log.d(TAG, "onHandleProvisionFromFileDone");
        mPsm.sendProvisionFromFileDone();
    }

    private void onHandleProvisionUpdateResponseDone(boolean success) {
        Log.d(TAG, "onHandleProvisionUpdateResponseDone");
        mPsm.sendProvisionUpdateResponseDone(success);
    }

    private void onHandleSimProvisionUpdateResponseDone(boolean success) {
        Log.d(TAG, "onHandleSimProvisionUpdateResponseDone");
        mPsm.sendSimProvisionUpdateResponseDone(success);
    }

    private void onHandleRemediationUpdateResponseDone(boolean success) {
        Log.d(TAG, "onHandleRemediationUpdateResponseDone");
        mPsm.sendRemediationUpdateResponseDone(success);
    }

    private void onHandlePolicyProvisionUpdateResponseDone(boolean success) {
        Log.d(TAG, "onHandleSoapPolicyProvisionUpdateResponseDone");
        mPsm.sendPolicyProvisionUpdateResponseDone(success);
    }

    private void onHandleRegisterCertEnrollmentDone(boolean success) {
        Log.d(TAG, "onHandleRegisterCertEnrollmentDone, success = " + success);
        mPsm.sendRegisterCertEnrollmentDone(success);
    }

    private void onHandlePolicyProvisionDone(int result, WifiTree tree) {
        Log.d(TAG, "onHandlePolicyProvisionDone:" + result);
        mPsm.sendPolicyProvisionDone(result, tree);
    }

    private void onHandleRemediationDone(int result, WifiTree tree) {
        Log.d(TAG, "onHandleSoapRemediationDone:" + result);
        mPsm.sendRemediationDone(result, tree);
    }

    private void onHandleUpdateNotificationServiceProviderInfo(boolean isAvaiable, int numOfSp) {
        Log.d(TAG, "onHandleUpdateNotificationServiceProviderInfo:" + numOfSp);
        if (isAvaiable) {
            Intent intent = new Intent("android.net.wifi.PICK_PASSPOINT_NETWORK");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

            PendingIntent pIntent = PendingIntent.getActivity(mContext, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

            Notification.Builder builder  = new Notification.Builder(mContext);
            builder.setSmallIcon(R.drawable.ic_passpoint_osu);
            builder.setTicker(mContext.getResources().getString(R.string.passpoint_sp_notification_ticker));
            builder.setContentTitle(mContext.getResources().getString(R.string.passpoint_sp_notification_title));
            builder.setContentText(mContext.getResources().getString(R.string.passpoint_sp_notification_content));
            builder.setAutoCancel(false);
            builder.setPriority(Notification.PRIORITY_HIGH);
            builder.addAction(R.drawable.ic_passpoint_osu,
                              mContext.getResources().getString(R.string.passpoint_sp_notification_action),
                              pIntent);

            Notification notification = builder.build();
            notification.flags |= Notification.FLAG_NO_CLEAR;

            mNoficationMgr.notify(TAG_SPINFO, 0, notification);
        } else {
            mNoficationMgr.cancel(TAG_SPINFO, 0);
        }

    }

    void onHandleUpdateNotificationConnected(boolean isConnected, String info) {
        if (isConnected) {
            Notification.Builder builder  = new Notification.Builder(mContext);
            builder.setSmallIcon(R.drawable.ic_passpoint_connected);
            builder.setTicker(mContext.getResources().getString(R.string.passpoint_is_connected));
            builder.setContentTitle(mContext.getResources().getString(R.string.passpoint_is_connected));
            builder.setContentText(info);
            //builder.setContentIntent(pIntent);
            builder.setAutoCancel(false);

            Notification notification = builder.build();
            notification.flags |= Notification.FLAG_NO_CLEAR;

            mNoficationMgr.notify(TAG_CONNECTED, 0, notification);
        } else {
            mNoficationMgr.cancel(TAG_CONNECTED, 0);
        }
    }

    void onHandleUpdateNotificationProvisionFail() {
        Intent intent = new Intent("android.net.wifi.PICK_PASSPOINT_NETWORK");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        PendingIntent pIntent = PendingIntent.getActivity(mContext, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        Notification.Builder builder  = new Notification.Builder(mContext);
        builder.setSmallIcon(com.android.internal.R.drawable.stat_notify_error);
        builder.setTicker(mContext.getResources().getString(R.string.passpoint_provision_fail_title));
        builder.setContentTitle(mContext.getResources().getString(R.string.passpoint_provision_fail_title));
        builder.setContentText(mContext.getResources().getString(R.string.passpoint_provision_fail_content));
        builder.setAutoCancel(true);
        builder.setContentIntent(pIntent);
        Notification notification = builder.build();

        mNoficationMgr.notify(TAG_PFAIL, 0, notification);

    }

    private class MyHandler extends Handler {
        public MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "processMessage what=" + msg.what);
            boolean success;
            WifiTree tree = null;
            int result = -1;
            switch (msg.what) {
            case PasspointManager.EVENT_POLICY_PROVISION_DONE:
                result = msg.arg1;
                tree = (WifiTree)msg.obj;
                onHandlePolicyProvisionDone(result, tree);
                break;

            case PasspointManager.EVENT_REMEDIATION_DONE:
                result = msg.arg1;
                tree = (WifiTree)msg.obj;
                onHandleRemediationDone(result, tree);
                break;

            case PasspointManager.EVENT_PROVISION_DONE:
                result = msg.arg1;
                tree = (WifiTree)msg.obj;
                onHandleProvisionDone(result, tree);
                break;

            case PasspointManager.EVENT_PRE_PROVISION_DONE:
                result = msg.arg1;
                tree = (WifiTree)msg.obj;
                onHandlePreProvisionDone(result, tree);
                break;

            case PasspointManager.EVENT_SIM_PROVISION_DONE:
                result = msg.arg1;
                tree = (WifiTree)msg.obj;
                onHandleSimProvisionDone(result, tree);
                break;

            case PasspointManager.EVENT_PROVISION_FROM_FILE_DONE:
                onHandleProvisionFromFileDone();
                break;

            case PasspointManager.EVENT_SET_POLICY_POLL_TIMER:
                Long polsec = (Long)msg.obj;
                registerPolicyUpdatePollAlarm(polsec);
                break;

            case PasspointManager.EVENT_CANCEL_POLICY_POLL_TIMER:
                cancelPolicyPollAlarm();
                break;

            case PasspointManager.EVENT_PROVISION_UPDATE_RESPONSE_DONE:
                success = (Boolean)msg.obj;
                onHandleProvisionUpdateResponseDone(success);
                break;

            case PasspointManager.EVENT_SET_REMEDIATION_POLL_TIMER:
                Long remsec = (Long)msg.obj;
                registerRemediationPollAlarm(remsec);
                break;

            case PasspointManager.EVENT_CANCEL_REMEDIATION_POLL_TIMER:
                cancelRemediationPollAlarm();
                break;

            case PasspointManager.EVENT_SIM_PROVISION_UPDATE_RESPONSE_DONE:
                success = (Boolean)msg.obj;
                onHandleSimProvisionUpdateResponseDone(success);
                break;

            case PasspointManager.EVENT_REMEDIATION_UPDATE_RESPONSE_DONE:
                success = (Boolean)msg.obj;
                onHandleRemediationUpdateResponseDone(success);
                break;

            case PasspointManager.EVENT_POLICY_PROVISION_UPDATE_RESPONSE_DONE:
                success = (Boolean)msg.obj;
                onHandlePolicyProvisionUpdateResponseDone(success);
                break;

            case PasspointManager.EVENT_REGISTER_CERT_ENROLLMENT_DONE:
                success = (Boolean)msg.obj;
                onHandleRegisterCertEnrollmentDone(success);
                break;

            case PasspointManager.EVENT_UPDATE_NOTIFICATION_SPINFO:
                boolean available = msg.arg1 == 1? true: false;
                int numOfSp = msg.arg2;
                onHandleUpdateNotificationServiceProviderInfo(available, numOfSp);
                break;

            case PasspointManager.EVENT_UPDATE_NOTIFICATION_CONNEDTED:
                boolean connected = msg.arg1 == 1? true: false;
                String info = (String)msg.obj;
                onHandleUpdateNotificationConnected(connected, info);
                break;

            case PasspointManager.EVENT_UPDATE_NOTIFICATION_PROVISION_FAIL:
                onHandleUpdateNotificationProvisionFail();
                break;
            default:
                break;
            }
        }
    }
}
