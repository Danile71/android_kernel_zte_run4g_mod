/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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

package com.mediatek.dm.conn;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

import com.mediatek.dm.DmApplication;
import com.mediatek.dm.DmCommonFun;
import com.mediatek.dm.DmConst;
import com.mediatek.dm.DmConst.TAG;
import com.mediatek.dm.DmService;
import com.mediatek.dm.data.IDmPersistentValues;
import com.mediatek.dm.ext.MTKConnectivity;
import com.mediatek.dm.ext.MTKOptions;
import com.mediatek.dm.ext.MTKPhone;
import com.mediatek.dm.fumo.DmClient;
import com.mediatek.dm.option.Options;
import com.mediatek.dm.scomo.DmScomoDownloadDetailActivity;
import com.mediatek.dm.session.SessionEventQueue;
import com.mediatek.dm.util.DmThreadPool;
import com.mediatek.dm.util.ScreenLock;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

import junit.framework.Assert;

public final class DmDataConnection {

    private ConnectivityReceiver mConnectivityReceiver;
    private ConnectivityManager mConnMgr;
    private Context mContext;
    private DmDatabase mDmDatabase;

    private int mSimId = -1;

    private static Handler sClientHandler;
    private static Handler sScomoHandler;
    private static Handler sServiceHandler;

    private static DmDataConnection sInstance;

    private NetworkWatcherThread mNetworkThread;
    public static final int CONN_INTERVAL = 5000;

    // extended message handler
    private Handler mUserMsgHandler;

    public void setUserHandler(Handler hd) {
        mUserMsgHandler = hd;
    }

    private DmDataConnection(Context context) {
        mContext = context;
        mConnMgr = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        // register the CONNECTIVITY_ACTION receiver
        mConnectivityReceiver = new ConnectivityReceiver();

        if (!Options.USE_DIRECT_INTERNET) {
            // init DmDatabase
            mDmDatabase = new DmDatabase(context);
        }
        IntentFilter intent = new IntentFilter();
        intent.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        intent.addAction(DmConst.IntentAction.NET_DETECT_TIMEOUT);
        mContext.registerReceiver(mConnectivityReceiver, intent);

    }

    public static synchronized DmDataConnection getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new DmDataConnection(context);
        }
        return sInstance;
    }

    public int startDmDataConnectivity() throws IOException {
        Assert.assertFalse("startDmDataConnectivity MUST NOT be called in direct internet conn.",
                Options.USE_DIRECT_INTERNET);

        int result = -1;
        if (Options.USE_SMS_REGISTER) {
            mSimId = DmCommonFun.getSimRegisterID(mContext);
        } else {
            mSimId = DmCommonFun.getPreferredSimID(mContext);
            Log.d(TAG.CONNECTION, "Prefered SIM ID: " + mSimId);
        }

        // for gemini
        if (MTKOptions.MTK_GEMINI_SUPPORT) {
            if (mSimId == -1) {
                Log.e(TAG.CONNECTION, "Get Register SIM ID error in start data connection");
                return result;
            }
            // begin for connectity gemini
            if (!mDmDatabase.isDmApnReady(mSimId)) {
                Log.e(TAG.CONNECTION, "Dm apn table is not ready!");
                return result;
            }
            result = beginDmDataConnectivityGemini(mSimId);
        } else {
            if (!mDmDatabase.isDmApnReady(DmConst.GEMINI_SIM_1)) {
                Log.e(TAG.CONNECTION, "Dm apn table is not ready!");
                return result;
            }
            result = beginDmDataConnectivity();
        }

        if (result == MTKPhone.APN_TYPE_NOT_AVAILABLE || result == MTKPhone.APN_REQUEST_FAILED) {
            Log.e(TAG.CONNECTION, "start Dmdate Connectivity error");
        }

        // for test begin
        if (result == MTKPhone.APN_ALREADY_ACTIVE) {
            Log.i(TAG.CONNECTION,
                    "DataConnection is already exist and send MSG_WAP_CONNECTION_SUCCESS to client");

            notifyHandlers(IDmPersistentValues.MSG_WAP_CONNECTION_SUCCESS);
        } else {
            Log.i(TAG.CONNECTION,
                    "DataConn is not ready yet, start an thread to watch the connection");
            if (mNetworkThread == null) {
                mNetworkThread = new NetworkWatcherThread();
                DmThreadPool.getInstance().execute(mNetworkThread);
            }
        }

        // for test end
        return result;
    }

    public void stopDmDataConnectivity() {
        Assert.assertFalse("stopDmDataConnectivity MUST NOT be called in direct internet conn.",
                Options.USE_DIRECT_INTERNET);
        Log.v(TAG.CONNECTION, "stopDmDataConnectivity");
        try {
            mSimId = DmCommonFun.getSimRegisterID(mContext);
            if (mSimId == -1) {
                Log.e(TAG.CONNECTION, "Get Register SIM ID error in stop data connection");
                return;
            }
            if (MTKOptions.MTK_GEMINI_SUPPORT) {
                // begin for connectity gemini
                endDmConnectivityGemini(mSimId);
            } else {
                endDmDataConnectivity();
            }
            ScreenLock.releaseWakeLock(mContext);
            ScreenLock.enableKeyguard(mContext);
            // mContext.unregisterReceiver(mConnectivityReceiver);
            // mConnectivityReceiver = null;
            // mContext = null;
            // instance = null;
        } finally {
            Log.v(TAG.CONNECTION, "stopUsingNetworkFeature end");
        }
    }

    public class ConnectivityReceiver extends BroadcastReceiver {
        public boolean bConn = false;

        @Override
        public void onReceive(Context context, Intent intent) {
            if (context == null || intent == null) {
                return;
            }

            if (intent.getAction().equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                Log.d(TAG.CONNECTION,
                        "ConnectivityReceiver Receive android.net.conn.CONNECTIVITY_CHANGE");
                Bundle mBdl = intent.getExtras();
                if (mBdl != null) {
                    @SuppressWarnings("deprecation")
                    NetworkInfo info = (NetworkInfo) mBdl
                            .get(ConnectivityManager.EXTRA_NETWORK_INFO);
                    if (info == null) {
                        Log.e(TAG.CONNECTION, "[dm-conn]->Get NetworkInfo error");
                        return;
                    }

                    int networkType = info.getType();
                    if (!Options.USE_DIRECT_INTERNET) {
                        if (Options.USE_SMS_REGISTER) {
                            mSimId = DmCommonFun.getSimRegisterID(mContext);
                            if (mSimId == -1) {
                                Log.e(TAG.CONNECTION,
                                        "[dm-conn]->Get Register SIM ID error in connetivity receiver");
                                return;
                            }
                        } else {
                            mSimId = DmCommonFun.getPreferredSimID(mContext);
                            if (!MTKOptions.MTK_GEMINI_SUPPORT && mSimId == -1) {
                                mSimId = 0;
                            }
                        }

                        int networkSimId = MTKConnectivity.getSimId(info);
                        int intentSimId = intent.getIntExtra(MTKConnectivity.EXTRA_SIM_ID, 0);

                        Log.i(TAG.CONNECTION,
                                new StringBuilder("[dm-conn]->type == ").append(info.getTypeName())
                                        .append("(").append(networkType).append(")").append("\n")
                                        .append("[dm-conn]->intent_sim_Id == ").append(intentSimId)
                                        .append("\n").append("[dm-conn]->network_sim_Id == ")
                                        .append(networkSimId).append("\n")
                                        .append("[dm-conn]->registered_sim_Id == ").append(mSimId)
                                        .toString());

                        if (intentSimId != mSimId || networkType != MTKConnectivity.TYPE_MOBILE_DM) {
                            Log.e(TAG.CONNECTION, "[dm-conn]->SimId or connect type is incorrect");
                            return;
                        }

                    } else {

                        Log.i(TAG.CONNECTION,
                                new StringBuilder("[dm-conn]->type == ").append(info.getTypeName())
                                        .append("(").append(networkType).append(")").toString());
                        if (networkType != MTKConnectivity.TYPE_MOBILE
                                && networkType != MTKConnectivity.TYPE_WIFI) {
                            Log.e(TAG.CONNECTION, "connect type is incorrect");
                            return;
                        }
                    }

                    State state = info.getState();

                    if (state == State.CONNECTED) { // TODO: swap
                        Log.i(TAG.CONNECTION, "[dm-conn]->state == CONNECTED");
                        bConn = true;
                        try {
                            if (!Options.USE_DIRECT_INTERNET) {
                                ensureRouteToHost();
                            }
                            // store CONNECTED event.
                            DmApplication.getInstance().queueEvent(
                                    SessionEventQueue.EVENT_CONN_CONNECTED);
                            Log.i(TAG.CONNECTION, ">>sending msg WAP_CONN_SUCCESS");
                            notifyHandlers(IDmPersistentValues.MSG_WAP_CONNECTION_SUCCESS);

                        } catch (IOException ex) {
                            Log.e(TAG.CONNECTION, "[dm-conn]->ensureRouteToHost() failed:", ex);
                        }
                    } else if (state == State.CONNECTING) {
                        Log.i(TAG.CONNECTION, "[dm-conn]->state == CONNECTING");
                        return;
                    } else if (state == State.DISCONNECTED) {
                        Log.i(TAG.CONNECTION, "[dm-conn]->state == DISCONNECTED");

                        // store DISCONNECTED event.
                        DmApplication.getInstance().queueEvent(
                                SessionEventQueue.EVENT_CONN_DISCONNECTED);
                        return;
                    }
                }

            } else if (intent.getAction().equalsIgnoreCase(DmConst.IntentAction.NET_DETECT_TIMEOUT)) {
                Log.i(TAG.CONNECTION, "[dm-conn]->action == com.mediatek.dm.NETDETECTTIMEOUT");

                Log.i(TAG.CONNECTION, ">>>sending msg WAP_CONN_TIMEOUT");
                notifyHandlers(IDmPersistentValues.MSG_WAP_CONNECTION_TIMEOUT);
            }
        }// onReceive
    }// class ConnectivityReceiver

    private class NetworkWatcherThread extends Thread {
        public void run() {
        Log.i(TAG.CONNECTION, "start to run watcher thread");
            try {
                try {
                    // sleep 30s to wait for the first dm data connnection completed
                    Thread.sleep(CONN_INTERVAL * 6);
                    int times = 12;
                    if (mConnectivityReceiver.bConn) {
                        Log.i(TAG.CONNECTION, "[dm-conn] already successful, no need to start again");
                        return;
                    } else {
                        Log.i(TAG.CONNECTION, "[dm-conn] is not ready after 30s, need to retry");
                        Log.i(TAG.CONNECTION, "[dm-conn] stop the dm data connectivity before retry");
                        stopDmDataConnectivity();
                    }

                    int result = MTKPhone.APN_REQUEST_FAILED;
                    while (!mConnectivityReceiver.bConn && times > 0) {
                        Log.i(TAG.CONNECTION, "[dm-conn] begin the " + times + "retry ");
                        result = startDmDataConnectivity();
                        if (result == MTKPhone.APN_ALREADY_ACTIVE) {
                            break;
                        } else if (result == MTKPhone.APN_REQUEST_STARTED) {
                            Log.i(TAG.CONNECTION, "[dm-conn] is waiting for intent, sleep 10 seconds");
                            // sleep 10s to wait for the dm connection complete
                            Thread.sleep(CONN_INTERVAL * 2);
                            if (mConnectivityReceiver.bConn) {
                                break;
                            }
                        } else if (result == MTKPhone.APN_REQUEST_FAILED){
                            Log.i(TAG.CONNECTION, "[dm-conn] has connected failed");
                        }
                        Log.i(TAG.CONNECTION, "[dm-conn] should be stoped before another try");
                        stopDmDataConnectivity();
                        times--;
                    }
                    if (result != MTKPhone.APN_ALREADY_ACTIVE) {
                        Log.e(TAG.CONNECTION, "[dm-conn] has tried for 12 times in 2 minute and can not connect successully!");
                    } else {
                        Log.i(TAG.CONNECTION, "[dm-conn] has been connected successully!");
                    }
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    private int beginDmDataConnectivity() throws IOException {

        int result = mConnMgr.startUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE,
                MTKPhone.FEATURE_ENABLE_DM);

        Log.i(TAG.CONNECTION, "[dm-conn]->startUsingNetworkFeature: result=" + result);

        switch (result) {
        case MTKPhone.APN_ALREADY_ACTIVE:
            Log.i(TAG.CONNECTION, "[dm-conn]->APN_ALREADY_ACTIVE");
            ScreenLock.releaseWakeLock(mContext);
            ScreenLock.acquirePartialWakelock(mContext);
            ensureRouteToHost();
            break;
        case MTKPhone.APN_REQUEST_STARTED:
            Log.i(TAG.CONNECTION, "[dm-conn]->APN_REQUEST_STARTED, waiting for intent.");
            ScreenLock.releaseWakeLock(mContext);
            ScreenLock.acquirePartialWakelock(mContext);
            break;
        // mContext.registerReceiver(mConnectivityReceiver, new
        // IntentFilter(
        // ConnectivityManager.CONNECTIVITY_ACTION));
        // mTelephonyManager=(TelephonyManager)
        // mContext.getSystemService(Service.TELEPHONY_SERVICE);
        case MTKPhone.APN_REQUEST_FAILED:
            Log.e(TAG.CONNECTION, "[dm-conn]->APN_REQUEST_FAILED");
            break;
        default:
            throw new IOException("[dm-conn]:Cannot establish DM data connectivity");
        }
        return result;
    }

    private void endDmDataConnectivity() {
        try {
            Log.v(TAG.CONNECTION, "endDmDataConnectivity");

            if (mConnMgr != null) {
                mConnMgr.stopUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE,
                        MTKPhone.FEATURE_ENABLE_DM);
            }
        } finally {
            Log.v(TAG.CONNECTION, "stopUsingNetworkFeature end");
        }
    }

    private int beginDmDataConnectivityGemini(int simId) throws IOException {

        int result = MTKConnectivity.startUsingNetworkFeatureGemini(mConnMgr,
                ConnectivityManager.TYPE_MOBILE, MTKPhone.FEATURE_ENABLE_DM, simId);

        Log.i(TAG.CONNECTION, "startDmDataConnectivityGemini: simId = " + simId + "\t result="
                + result);

        switch (result) {
        case MTKPhone.APN_ALREADY_ACTIVE:
            Log.w(TAG.CONNECTION, "The data connection is already exist, go ahead");
            ScreenLock.releaseWakeLock(mContext);
            ScreenLock.acquirePartialWakelock(mContext);
            ensureRouteToHost();
            break;
        case MTKPhone.APN_REQUEST_STARTED:
            Log.w(TAG.CONNECTION,
                    "The new data connection is started register and waiting for the intent");
            ScreenLock.releaseWakeLock(mContext);
            ScreenLock.acquirePartialWakelock(mContext);
            break;
        // mContext.registerReceiver(mConnectivityReceiver, new
        // IntentFilter(
        // ConnectivityManager.CONNECTIVITY_ACTION));
        // mContext.registerReceiver(mConnectivityReceiver, new
        // IntentFilter(DmConst.intentAction.NET_DETECT_TIMEOUT));
        // mTelephonyManager=(TelephonyManager)
        // mContext.getSystemService(Service.TELEPHONY_SERVICE);
        case MTKPhone.APN_REQUEST_FAILED:
            Log.e(TAG.CONNECTION, "startUsingnetworkfeature failed");
            break;
        default:
            throw new IOException("Cannot establish Dm Data connectivity");
        }
        return result;
    }

    // add for gemini
    private void endDmConnectivityGemini(int simId) {
        try {
            Log.i(TAG.CONNECTION, "endDmDataConnectivityGemini: simId = " + simId);

            if (mConnMgr != null) {
                MTKConnectivity.stopUsingNetworkFeatureGemini(mConnMgr,
                        ConnectivityManager.TYPE_MOBILE, MTKPhone.FEATURE_ENABLE_DM, simId);
            }
        } finally {
            Log.v(TAG.CONNECTION, "stopUsingNetworkFeature end");
        }
    }

    private void ensureRouteToHost() throws IOException {
        Log.v(TAG.CONNECTION, "Begin ensureRouteToHost");
        // call getApnInfoFromSettings
        String proxyAddr = mDmDatabase.getApnProxyFromSettings();
        int inetAddr = lookupHost(proxyAddr);
        Log.i(TAG.CONNECTION, "inetAddr = " + inetAddr);

        // get the addr form setting
        if (!mConnMgr.requestRouteToHost(MTKConnectivity.TYPE_MOBILE_DM, inetAddr)) {
            throw new IOException("Cannot establish route to proxy " + inetAddr);
        }

    }

    public static int lookupHost(String hostname) {
        InetAddress inetAddress;
        try {
            inetAddress = InetAddress.getByName(hostname);
        } catch (UnknownHostException e) {
            return -1;
        }
        byte[] addrBytes;
        int addr;
        addrBytes = inetAddress.getAddress();
        addr = ((addrBytes[3] & 0xff) << 24) | ((addrBytes[2] & 0xff) << 16)
                | ((addrBytes[1] & 0xff) << 8) | (addrBytes[0] & 0xff);
        return addr;
    }

    private void notifyHandlers(int msgCode) {
        sClientHandler = null;
        sScomoHandler = null;
        if (DmClient.getVdmClientInstance() != null) {
            sClientHandler = DmClient.getVdmClientInstance().mApnConnHandler;
        }

        if (DmScomoDownloadDetailActivity.getInstance() != null) {
            sScomoHandler = DmScomoDownloadDetailActivity.getInstance().mApnConnHandler;
        }

        if (sServiceHandler == null) {
            if (DmService.getInstance() != null) {
                sServiceHandler = DmService.getInstance().mHandler;
            }
        }
        if (sClientHandler != null) {
            sClientHandler.sendMessage(sClientHandler.obtainMessage(msgCode));
        }
        if (sScomoHandler != null) {
            sScomoHandler.sendMessage(sScomoHandler.obtainMessage(msgCode));
        }
        if (sServiceHandler != null) {
            sServiceHandler.sendMessage(sServiceHandler.obtainMessage(msgCode));
        }

        // extended message handler
        if (mUserMsgHandler != null) {
            mUserMsgHandler.sendMessage(mUserMsgHandler.obtainMessage(msgCode));
        }
    }

    private void destroyDataConnection() {
        mContext.unregisterReceiver(mConnectivityReceiver);
        mContext = null;
    }

    public static void destroyInstance() {
        if (sInstance != null) {
            sInstance.destroyDataConnection();
            sInstance = null;
        }
    }

}
