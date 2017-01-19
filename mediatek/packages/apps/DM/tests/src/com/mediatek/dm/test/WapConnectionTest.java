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

package com.mediatek.dm.test;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.test.AndroidTestCase;
import android.util.Log;

//import com.android.internal.telephony.Phone;
import com.mediatek.dm.DmCommonFun;
import com.mediatek.dm.DmConst;
import com.mediatek.dm.conn.DmDataConnection;
import com.mediatek.dm.conn.DmDatabase;
import com.mediatek.dm.data.IDmPersistentValues;

import java.io.IOException;

import junit.framework.Assert;

public class WapConnectionTest extends AndroidTestCase {
    private static final String TAG = "[WapConnectionTest]";

    private static final int TIME_OUT_VALUE = 30;

    private static final int ONESEVOND = 1000;

    private DmDatabase apnDB;
    private Object lockObj;

    private AlarmManager alarmMgr;
    private PendingIntent networkTimeoutOp;

    private Handler msgHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case IDmPersistentValues.MSG_WAP_CONNECTION_SUCCESS:
                Log.i(TAG, "==== handled WAP_CONN_SUCCESS ====");
                cancleNetworkTimeoutAlarm();
                break;
            case IDmPersistentValues.MSG_WAP_CONNECTION_TIMEOUT:
                Log.i(TAG, "==== handled WAP_CONN_TIMEOUT ====");
                break;
            }
            synchronized (lockObj) {
                lockObj.notify();
            }
        }
    };

    protected void setUp() throws Exception {
        super.setUp();
        apnDB = new DmDatabase(mContext);
        lockObj = new Object();
    }

    public void testDmApnDatabase() {
        int simId = DmCommonFun.getSimRegisterID(mContext);
        boolean isApnReady = apnDB.isDmApnReady(simId);
        Assert.assertTrue("Dm APN table is not ready.", isApnReady);

        String proxyUrl = apnDB.getApnProxyFromSettings();
        int proxyPort = apnDB.getApnProxyPortFromSettings();

        Log.d(TAG, "APN Proxy: " + proxyUrl + ":" + proxyPort);
        Assert.assertEquals("getApnProxyFromSettings", "10.0.0.172", proxyUrl);
        Assert.assertEquals("getApnProxyPortFromSettings", 80, proxyPort);

        String dmAddr = apnDB.getDmAddressFromSettings();
        Log.d(TAG, "DM addr: " + dmAddr);
        Assert.assertNotNull("getDmAddressFromSettings", dmAddr);
    }

    /*public void testTriggerWapConn() {
        DmDataConnection.getInstance(mContext).setUserHandler(msgHandler);

        int result = -1;
        try {
            result = DmDataConnection.getInstance(mContext)
                    .startDmDataConnectivity();
            Log.d(TAG, "startDmDataConn result=" + result);
        } catch (IOException e) {
            Log.e(TAG, "startDmDataConnectivity failed.", e);
            e.printStackTrace();
        }

        if (result == Phone.APN_ALREADY_ACTIVE) {
            Log.i(TAG, "WAP Conn was already active!");
        } else {
            Log.i(TAG, "---waiting for WAP Conn set up...");
            setNetworkTimeoutAlarm();
            synchronized (lockObj) {
                try {
                    lockObj.wait();
                } catch (InterruptedException ex) {
                    Log.w(TAG, "waiting was interruped.");
                }
            }
            Log.i(TAG, "+++waiting finished.");
        }
        msgHandler = null;
        DmDataConnection.getInstance(mContext).setUserHandler(null);
    }*/
    private void setNetworkTimeoutAlarm() {
        Log.d(TAG, "setAlarm alarm");
        Intent intent = new Intent();
        intent.setAction(DmConst.IntentAction.NET_DETECT_TIMEOUT);
        alarmMgr = (AlarmManager) getContext().getSystemService(
                Context.ALARM_SERVICE);
        if (alarmMgr == null) {
            Log.w(TAG, "setAlarm alarmMgr is null");
            return;
        }
        if (alarmMgr != null) {
            networkTimeoutOp = PendingIntent.getBroadcast(
                    getContext(), 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
            alarmMgr.cancel(networkTimeoutOp);
            alarmMgr.setExact(AlarmManager.RTC_WAKEUP,
                    (System.currentTimeMillis() + TIME_OUT_VALUE * ONESEVOND),
                    networkTimeoutOp);
        }
    }

    private void cancleNetworkTimeoutAlarm() {
        if (alarmMgr != null && networkTimeoutOp != null) {
            alarmMgr.cancel(networkTimeoutOp);
            alarmMgr = null;
            networkTimeoutOp = null;
        }
    }
}
