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

package com.mediatek.csm.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.SystemClock;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.android.internal.telephony.IPhoneStateListener;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.csm.CustomerServiceFragment;
import com.mediatek.csm.CustomerServiceMenu;
import com.mediatek.csm.PhoneManager;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

public class CustomerServiceTest extends ActivityInstrumentationTestCase2<CustomerServiceMenu> {
    private static final String LOG_TAG = "CustomerServiceTests";
    private static final boolean DEBUG = true;

    private static final int WAIT_TIME_MIN = 100;
    private static final int WAIT_TIME = 500;
    private static final int WAIT_TIME_X2 = 1000;
    private static final int WAIT_TIME_X4 = 2000;

    // Must be consistent with the "key-value" in res/xml/preferences.xml
    private static final String MAIN_SCR = "main_panel";
    private static final String HOTLINE_SCR = "customer_hotline";
    private static final String EHALL_SCR = "ehall";
    private static final String SUBSCRIBE_SCR = "subscription";
    private static final String PRL_SCR = "pref_prl";
    private static final String GLOBAL_HOTLINE_SCR = "global_hotline";
    private static final String GUIDE_SCR = "guide";

    private static final String HOTLINE_PREF = "pref_hotline";
    private static final String SELF_HOTLINE_PREF = "pref_self_hotline";
    private static final String EHALL_MESSAGE_PREF = "pref_message";
    private static final String EHALL_WAP_PREF = "pref_wap";
    private static final String SUBSCRIBE_PREF = "pref_subscribe";
    private static final String UNSUBSCRIBE_PREF = "pref_unsubscribe";
    private static final String PRL_UPDATE_PREF = "pref_prl";
    
    private static final int INDEX_EHALL = 0;
    private static final int INDEX_SUBSCRIPTION = 1;
    private static final int INDEX_PREF_PRL = 2;
    private static final int INDEX_CUSTOMER_HOTLINE = 3;
    private static final int INDEX_GLOBAL_HOTLINE = 4;
    private static final int INDEX_GUIDE = 5;

    private CustomerServiceMenu mActivity;
    private Context mContext;
    private Instrumentation mInst;
    private PreferenceFragment mFragment;
    private volatile boolean mSuccess;
    private volatile boolean mWaitingForConnection;
    private volatile boolean mWaitingForStateChange;
    private volatile int mState;

    class Item {
        public Preference screen;
        public View view;
        public ListView parent;

        public Item(ListView prt, Preference scr, View v) {
            parent = prt;
            screen = scr;
            view = v;
        }
    }

    public CustomerServiceTest() {
        super(CustomerServiceMenu.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        if (DEBUG) {
            Xlog.d(LOG_TAG, "setUp()");
        }
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mFragment = (PreferenceFragment) Reflection.getDeclaredProperty(
                mActivity, "mFragment");
        IntentFilter filter = new IntentFilter();
        filter.addAction(CustomerServiceFragment.ACTION_SENDTO_SEND);
        mContext.registerReceiver(mReceiver, filter);
    }

    @Override
    protected void tearDown() throws Exception {
        if (mActivity != null) {
            mActivity.finish();
            mActivity = null;
        }
        mContext.unregisterReceiver(mReceiver);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "tearDown()");
        }
        super.tearDown();
    }

    public void test01HotLine() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testHotLine()");
        }
        Item item = goToScreen(INDEX_CUSTOMER_HOTLINE);
        assertEquals(HOTLINE_SCR, item.screen.getKey());
        final int simId = checkSimState();
        // Waiting for call state changed
        registerListener(PhoneStateListener.LISTEN_CALL_STATE, simId);

        int callState = PhoneManager.getCallState(simId);
        if (callState != TelephonyManager.CALL_STATE_IDLE) {
            Xlog.w(LOG_TAG, "has calling!");
            PhoneManager.endCall(simId);
            waitForMs(WAIT_TIME);
            waitForStateChanged(TelephonyManager.CALL_STATE_IDLE);
        }
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);

        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 1, 0);
        waitForStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "calling!");
        }
        callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, callState);

        PhoneManager.endCall(simId);
        waitForStateChanged(TelephonyManager.CALL_STATE_IDLE);

        unregisterListener(simId);
        callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "calling end!");
        }
    }

    public void test02SelfHotLine() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testSelfHotLine()");
        }
        Item item = goToScreen(INDEX_CUSTOMER_HOTLINE);
        assertEquals(HOTLINE_SCR, item.screen.getKey());
        final int simId = checkSimState();
        // Waiting for call state changed
        registerListener(PhoneStateListener.LISTEN_CALL_STATE, simId);

        int callState = PhoneManager.getCallState(simId);
        if (callState != TelephonyManager.CALL_STATE_IDLE) {
            Xlog.w(LOG_TAG, "has calling!");
            PhoneManager.endCall(simId);
            waitForMs(WAIT_TIME);
            waitForStateChanged(TelephonyManager.CALL_STATE_IDLE);
        }
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);

        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 2, 0);
        waitForStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "calling!");
        }
        callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, callState);

        PhoneManager.endCall(simId);
        waitForStateChanged(TelephonyManager.CALL_STATE_IDLE);

        unregisterListener(simId);
        callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "calling end!");
        }
    }

    public void test03EhallMessage() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testEhallMessage()");
        }
        Item item = goToScreen(INDEX_EHALL);
        assertEquals(EHALL_SCR, item.screen.getKey());
        final int simId = checkSimState();
        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 0, 0);
        // Need to waiting for broadcast ACTION_SENDTO_SEND
        waitForConnection();
        if (TelephonyManagerEx.getDefault().getPhoneType(simId) == TelephonyManager.PHONE_TYPE_CDMA) {
            assertTrue(mSuccess);
        }
    }

    public void test04EhallWap() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testEhallWap()");
        }
        Item item = goToScreen(INDEX_EHALL);
        assertEquals(EHALL_SCR, item.screen.getKey());
        final int simId = checkSimState();

        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 1, 0);
        waitForMs(WAIT_TIME_X4);

    }

    public void test05Subscribe() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testSubscribe()");
        }
        Item item = goToScreen(INDEX_SUBSCRIPTION);
        assertEquals(SUBSCRIBE_SCR, item.screen.getKey());
        final int simId = checkSimState();
        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 0, 0);
        // Need to waiting for broadcast ACTION_SENDTO_SEND
        waitForConnection();
        if (TelephonyManagerEx.getDefault().getPhoneType(simId) == TelephonyManager.PHONE_TYPE_CDMA) {
            assertTrue(mSuccess);
        }
    }

    public void test06Unsubscribe() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testUnsubscribe()");
        }
        Item item = goToScreen(INDEX_SUBSCRIPTION);
        assertEquals(SUBSCRIBE_SCR, item.screen.getKey());
        final int simId = checkSimState();

        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 1, 0);
        // Need to waiting for broadcast ACTION_SENDTO_SEND
        waitForConnection();

        if (TelephonyManagerEx.getDefault().getPhoneType(simId) == TelephonyManager.PHONE_TYPE_CDMA) {
            assertTrue(mSuccess);
        }
    }

    public void test07Prl() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testPrl()");
        }
        // This will trigger the action
        Item item = goToScreen(INDEX_PREF_PRL);
        assertEquals(PRL_SCR, item.screen.getKey());
        final int simId = checkSimState();
        // Need to waiting for broadcast ACTION_SENDTO_SEND
        waitForConnection();
        if (TelephonyManagerEx.getDefault().getPhoneType(simId) == TelephonyManager.PHONE_TYPE_CDMA) {
            assertTrue(mSuccess);
        }
    }

    public void test08GlobalHotline() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testGlobalHotline()");
        }
        Item item = goToScreen(INDEX_GLOBAL_HOTLINE);
        assertEquals(GLOBAL_HOTLINE_SCR, item.screen.getKey());
        final int simId = checkSimState();
        registerListener(PhoneStateListener.LISTEN_CALL_STATE, simId);

        int callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);

        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 0, 0);
        waitForStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "calling!");
        }
        callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, callState);

        PhoneManager.endCall(simId);
        waitForStateChanged(TelephonyManager.CALL_STATE_IDLE);

        unregisterListener(simId);
        callState = PhoneManager.getCallState(simId);
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "calling end!");
        }
    }

    public void test09Guide() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "testGuide()");
        }
        Item item = goToScreen(INDEX_GUIDE);
        assertEquals(GUIDE_SCR, item.screen.getKey());
        final int simId = checkSimState();
        ((PreferenceScreen) item.screen).onItemClick(item.parent, null, 0, 0);
        // TODO: click the phone-number to show phone dialer
        // click the http link to show browser
        waitForMs(WAIT_TIME);

    }

    private void registerListener(int events, int simId) {
        PhoneManager.listen(mContext.getPackageName(), mStateListener,
                events, true, simId);
        // Ignore the first response callback when register
        waitForMs(WAIT_TIME_MIN);
    }

    private void unregisterListener(int simId) {
        PhoneManager.listen(mContext.getPackageName(), mStateListener,
                PhoneStateListener.LISTEN_NONE, true, simId);
    }

    private void waitForMs(int ms) {
        SystemClock.sleep(ms);
    }

    private void waitForWindowFocusLost(Activity activity) {
        while (activity.hasWindowFocus()) {
            SystemClock.sleep(WAIT_TIME_MIN);
        }
    }

    private void waitForStateChanged(int state) {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "waitForStateChanged " + state);
        }
        mWaitingForStateChange = true;
        mState = state;
        while (mWaitingForStateChange) {
            waitForMs(WAIT_TIME_MIN);
        }
        waitForMs(WAIT_TIME_MIN);
    }

    private void waitForConnection() {
        if (DEBUG) {
            Xlog.d(LOG_TAG, "waitForConnection");
        }
        mWaitingForConnection = true;
        while (mWaitingForConnection) {
            waitForMs(WAIT_TIME_MIN);
        }
    }

    private Item goToScreen(int index) {
        ListView listView = mFragment.getListView();
        final Preference screen = (Preference)
                listView.getItemAtPosition(index);
        final ViewGroup layout = (ViewGroup) listView.getChildAt(index);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "totlal screen: " + listView.getChildCount() + " current: "
                    + screen + " view: " + layout + " child count: " + layout.getChildCount());
        }
        TouchUtils.clickView(this, listView.getChildAt(index));
        return new Item(listView, screen, layout);
    }

    private int checkSimState() {
        int simId = PhoneConstants.GEMINI_SIM_1;
        if (!PhoneManager.isSimInserted(PhoneConstants.GEMINI_SIM_1) &&
                !PhoneManager.isSimInserted(PhoneConstants.GEMINI_SIM_2)) {
            throw new IllegalStateException("No SIM card inserted");
        }
        if (PhoneManager.isSimInserted(PhoneConstants.GEMINI_SIM_1)) {
            TelephonyManagerEx tpm = TelephonyManagerEx.getDefault();
            final int type01 = tpm.getPhoneType(PhoneConstants.GEMINI_SIM_1);
            final int type02 = tpm.getPhoneType(PhoneConstants.GEMINI_SIM_2);
            if (type01 != TelephonyManager.PHONE_TYPE_GSM &&
                    type01 != TelephonyManager.PHONE_TYPE_CDMA &&
                    PhoneManager.isSimInserted(PhoneConstants.GEMINI_SIM_2)) {
                simId = PhoneConstants.GEMINI_SIM_2;
            }
        }

        return simId;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                final String action = intent.getAction();
                final int resultCode = getResultCode();
                if (DEBUG) {
                    Xlog.d(LOG_TAG, "broadcast action=" + action
                            + ", resultCode=" + resultCode);
                }
                if (action != null && CustomerServiceFragment.ACTION_SENDTO_SEND.equals(action)) {
                    // Now you can touch it again
                    mWaitingForConnection = false;
                    if (resultCode == Activity.RESULT_OK) {
                        mSuccess = true;
                    } else {
                        mSuccess = false;
                    }
                }
            }
        }
    };

    private IPhoneStateListener mStateListener = new IPhoneStateListener.Stub() {
        public void onServiceStateChanged(ServiceState serviceState) {
            if (DEBUG) {
                Xlog.d(LOG_TAG, "onServiceStateChanged() " + serviceState.getState());
            }
            if (mState == serviceState.getState()) {
                mWaitingForStateChange = false;
            }
        }

        public void onCallStateChanged(int state, String incomingNumber) {
            if (DEBUG) {
                Xlog.d(LOG_TAG, "onCallStateChanged() " + state + ", " + incomingNumber);
            }
            if (mState == state) {
                mWaitingForStateChange = false;
            }
        }

        public void onDataConnectionStateChanged(int state, int networkType) {
            if (DEBUG) {
                Xlog.d(LOG_TAG, "onDataConnectionStateChanged() " + state + ", " + networkType);
            }
            if (mState == state) {
                mWaitingForStateChange = false;
            }
        }

        // Don't care of these
        public void onSignalStrengthChanged(int asu) {
        }

        public void onMessageWaitingIndicatorChanged(boolean mwi) {
        }

        public void onCallForwardingIndicatorChanged(boolean cfi) {
        }

        public void onCellLocationChanged(Bundle location) {
        }

        public void onDataActivity(int direction) {
        }

        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
        }

        public void onOtaspChanged(int otaspMode) {
        }

        public void onCellInfoChanged(java.util.List<android.telephony.CellInfo> cellInfo) {
        }
    };

}
