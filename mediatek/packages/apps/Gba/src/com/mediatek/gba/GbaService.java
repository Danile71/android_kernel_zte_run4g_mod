/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.gba;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.IBinder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.IccCardConstants;

import com.mediatek.common.gba.IGbaService;
import com.mediatek.common.gba.NafSessionKey;
import com.mediatek.gba.auth.AkaDigestAuth;
import com.mediatek.gba.cache.GbaKeysCache;
import com.mediatek.gba.element.NafId;
import com.mediatek.gba.telephony.TelephonyUtils;


import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

public class GbaService extends Service {
    public static final String TAG = "GbaService";

    private Context mContext;
    private GbaKeysCache mGbaKeysCache = null;
    private int mGbaType = GbaConstant.GBA_NONE;
    private int mCardType = GbaConstant.GBA_CARD_UNKNOWN;

    private static final int EVENT_SIM_STATE_CHANGED = 0;

    @Override
    public void onCreate() {
        super.onCreate();

        ServiceManager.addService(TAG, mBinder);

        mContext = this.getBaseContext();
        if(mGbaKeysCache == null){
            mGbaKeysCache = new GbaKeysCache();
        }

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);

        BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();

                if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                    String iccState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                    Log.d(TAG, "iccState:" + iccState);
                    if (!iccState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                        return;
                    }
                    mHandler.sendEmptyMessage(EVENT_SIM_STATE_CHANGED);
                }
            }
        };
        mContext.registerReceiver(receiver, intentFilter);

        Log.d(TAG, "Add service for GbaService");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if(intent == null) return Service.START_STICKY;

        String action = intent.getAction();
        Log.d(TAG, "Service starting for intent " + action);

        return Service.START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;    // clients can't bind to this service
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "Service is onDestroy");
    }


    private IBinder mBinder = new IGbaService.Stub() {

        public int getGbaSupported(){
            return TelephonyUtils.getGbaSupported();
        }

        public boolean isGbaKeyExpired(String nafFqdn, byte[] nafSecurProtocolId){
            boolean bIsKeyExpired = true;

            NafId nafId = NafId.createFromNafFqdnAndProtocolId(nafFqdn, nafSecurProtocolId);
            bIsKeyExpired = mGbaKeysCache.isExpiredKey(nafId, mGbaType);

            return bIsKeyExpired;
        }

        public NafSessionKey runGbaAuthentication(String nafFqdn, byte[] nafSecurProtocolId, boolean forceRun) {
            NafSessionKey nafSessionKey = null;

            ///M: Error handling for non-standard nafFqdn; @{
            if (nafFqdn.indexOf(":") != -1) {
                nafFqdn = nafFqdn.substring(0, nafFqdn.indexOf(":"));
            }
            ///@}

            NafId nafId = NafId.createFromNafFqdnAndProtocolId(nafFqdn, nafSecurProtocolId);
            boolean isExpiredKey = mGbaKeysCache.isExpiredKey(nafId, mGbaType);

            boolean needForce = SystemProperties.getBoolean("gba.run", false);
            forceRun = true;
            if (needForce){
               Log.i(TAG, "Need force:" + needForce);
               forceRun = true;
            }

            if (!isExpiredKey && !forceRun) {
                nafSessionKey = mGbaKeysCache.getKeys(nafId, mGbaType);
            } else {
                mCardType = TelephonyUtils.getIccCardType();
                mGbaType  = TelephonyUtils.getGbaSupported();
                GbaBsfProcedure gbaProcedure = new GbaBsfProcedure(mGbaType, mCardType, mContext);
                try{
                    nafSessionKey = gbaProcedure.perform(nafId);
                    if(nafSessionKey != null){
                        Log.i(TAG, "nafSessionKey:" + nafSessionKey);
                        mGbaKeysCache.putKeys(nafId, mGbaType, nafSessionKey);
                    }
                }catch(Exception e){
                    e.printStackTrace();
                }
            }

            return nafSessionKey;
        }
    };


    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch(msg.what){
                case EVENT_SIM_STATE_CHANGED:
                    mGbaType = TelephonyUtils.getGbaSupported();
                    mCardType = TelephonyUtils.getIccCardType();
                    Log.i(TAG, "Get GBA Type:" + mGbaType);
                break;
            }
        }
    };
}
