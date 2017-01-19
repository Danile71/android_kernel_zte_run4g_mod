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

package com.mediatek.imsframework;

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
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.common.ims.IImsManagerService;
import com.mediatek.common.ims.mo.ImsIcsi;
import com.mediatek.common.ims.mo.ImsAuthInfo;
import com.mediatek.common.ims.mo.ImsLboPcscf;
import com.mediatek.common.ims.mo.ImsPhoneCtx;
import com.mediatek.common.ims.mo.ImsXcapInfo;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.ims.ImsConstants;


import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

/**
 * Example service to handle IMS setup after boot completed event.
 *
 * Setting {@code android:persistent="true"} in the manifest will cause
 * {@link ImsFrameworkApp#onCreate()} to be called at system startup,
 * before {@link Intent#ACTION_BOOT_COMPLETED} is broadcast, so early
 * initialization can be performed there, such as registering to receive
 * telephony state change broadcasts that can't be declared in the manifest.
 */
public class ImsFrameworkService extends Service {
    private static final String TAG = "ImsFrameworkService";

    private static final int MAX_MO_COUNT = 4;

    private static final int EVENT_AT_CMD_READ_DONE                 = 1001;
    private static final int EVENT_AT_CMD_WRITE_DONE                 = 1002;

    private static final long TIMEOUT_INTERACTION_MILLIS = 5000;

    private boolean mDone = false;
    private String  mAtCmdResult = "0";

    private Handler mHandler;
    private HandlerThread mThread;
    private Context mContext;
    private static final Object mAtCmdSyncObj = new Object();
    private PersistentDataStore mPersistentDataStore;
    private Phone mGsmPhone = null;
    private String mImsi = "";


    private final boolean IsTestMode = false;
    private static final int MAX_ENTRIES = 64;
    private final HashMap<String, String> mModemCache = new HashMap<String, String>(MAX_ENTRIES);

    @Override
    public void onCreate() {
        super.onCreate();
        ServiceManager.addService(Context.IMS_SERVICE, mBinder);
        mPersistentDataStore = new PersistentDataStore();

        mThread = new HandlerThread(TAG);
        mThread.start();
        mHandler = new MyHandler(mThread.getLooper());

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);

        BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();

                if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                    String iccState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                    Log.d(TAG, "iccState:" + iccState);
                    getGsmPhone();
                    if (!iccState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                        return;
                    }
                    updateImsi();
                }
            }
        };
        mContext = this.getBaseContext();
        mContext.registerReceiver(receiver, intentFilter);

        Log.d(TAG, "Add service for " + Context.IMS_SERVICE);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if(intent == null) return Service.START_STICKY;

        String action = intent.getAction();
        Log.d(TAG, "Service starting for intent " + action);
        if (action != null && action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            Log.d(TAG, "Received ACTION_BOOT_COMPLETED");
            handleBootCompleted();
        }

        return Service.START_STICKY;
    }

    private void SetCacheMo(String keyStr, String keyValue) {
        if(mModemCache.containsKey(keyStr)) {
            mModemCache.remove(keyStr);
        }
        Log.d(TAG, "SetCacheMo:" + keyStr + ":" + keyValue);
        mModemCache.put(keyStr, keyValue);
    }

    private String getCacheMo(String keyStr) {
        String retStr = mModemCache.get(keyStr);

        if(retStr == null) {
            retStr = "1"; //Provide default value
        } else if(retStr == "true") {
            retStr = "1";
        } else if(retStr == "false") {
            retStr = "0";
        }

        Log.d(TAG, "getCacheMo:" + keyStr + ":" + retStr);

        return retStr;
    }

    private void updateImsi() {
        mImsi = mGsmPhone.getSubscriberId();
    }

    private void handleBootCompleted() {
        // Code to execute after boot completes, e.g. connecting to the IMS PDN
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;    // clients can't bind to this service
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "Service is onDestroy");
    }

    private void getGsmPhone() {
        if(mGsmPhone == null) {
            Log.e(TAG, "mGsmPhone is null");
            try {
                if(FeatureOption.MTK_GEMINI_SUPPORT) {
                    ITelephony iTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
                    if(iTelephony == null) {
                        Log.e(TAG, "iTelephony is null");
                        return;
                    }
                    Log.d(TAG, "Configure default phone");
                    iTelephony.setDefaultPhone(PhoneConstants.GEMINI_SIM_1);
                }
            } catch(Exception e) {
                e.printStackTrace();
            } finally {
                mGsmPhone = PhoneFactory.getDefaultPhone();
            }
        }
    }

    private boolean handleImsMoBool(int nodeId, boolean isRead, boolean nodeValue) {
        boolean ret = false;
        String cfgName = "";

        switch(nodeId) {
        case ImsConstants.IMS_MO_RESOURCE:
            cfgName = "resource_allocation_mode";
            break;
        case ImsConstants.IMS_MO_MOBILITY:
            /*
            Defined Values
            <setting>: integer type; indicates the mobility management IMS voice termination preference of the UE. The
            default value is manufacturer specific.
            1 Mobility Management for IMS Voice Termination disabled
            2 Mobility Management for IMS Voice Termination enabled
            */
            if(isRead) {
                try {
                    Log.d(TAG, "[GsmPhone]getUeMobilityManagementImsVoiceTermination");
                    String atCmdLine = "AT+CMMIVT?";
                    mAtCmdResult = "";
                    mDone = false;
                    mGsmPhone.invokeOemRilRequestRaw(getBytes(atCmdLine), mHandler.obtainMessage(EVENT_AT_CMD_READ_DONE));

                    String response = handleCommandResponse("+CMMIVT");

                    try {
                        if(response.equalsIgnoreCase("2")) {
                            ret = true;
                        }
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                    return ret;
                    //int cfgValue = mGsmPhone.getUeMobilityManagementImsVoiceTermination();
                    //return ((cfgValue ==1) ? true : false);
                } catch(Exception e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    Log.d(TAG, "[GsmPhone]setUeMobilityManagementImsVoiceTermination");
                    mGsmPhone.setUeMobilityManagementImsVoiceTermination(((nodeValue) ? 2 : 1));
                    ret = true;
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
            break;
        case ImsConstants.IMS_MO_SMS:
            cfgName = "sms_over_ip";
            break;
        case ImsConstants.IMS_MO_KEEPALIVE:
            cfgName = "UA_keep_alive";
            break;
        default:
            Log.e(TAG, "[handleImsMoBool] No match ID:" + nodeId);
            break;
        }

        if(cfgName.length() > 0) {
            if(isRead) {
                try {
                    String cfgValue = getGenernalImsMoConfiguration(cfgName);
                    Log.i(TAG, "Get Cfg info:" + cfgName + ":" + cfgValue);
                    return (cfgValue.equals("1") ? true : false);
                } catch(Exception e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    Log.i(TAG, "Set Cfg info:" + cfgName + ":" + nodeValue);
                    if(nodeValue) {
                        setGenernalImsMoConfiguration(cfgName, "1");
                    } else {
                        setGenernalImsMoConfiguration(cfgName, "0");
                    }
                    ret = true;
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
        }

        return ret;
    }

    private String[] handleImsMoStringArray(int nodeId, boolean isRead, String[] nodesValue) {
        String[] isimImpu = new String[MAX_MO_COUNT];
        final TelephonyManager telephony = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);

        isimImpu[0] = "";

        if(nodeId == ImsConstants.IMS_MO_IMPU) {
            if(isRead) {
                String [] tmpImpu = telephony.getIsimImpu();
                if(tmpImpu != null) {
                    return tmpImpu;
                }
            } else {
                Log.e(TAG, "Write: Non-supported for IMS_MO_IMPU");
            }
        }

        return isimImpu;
    }

    private String handleImsMoString(int nodeId, boolean isRead, String nodeValue, String impi) {
        String nodeReadValue = "";
        final TelephonyManager telephony = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
        if(telephony == null) {
            Log.e(TAG, "Write: Non-supported for IMS MO IMPI");
            return null;
        }

        switch(nodeId) {
        case ImsConstants.IMS_MO_IMPI:
            if(isRead) {
                nodeReadValue = telephony.getIsimImpi();
            } else {
                Log.e(TAG, "Write: Non-supported for IMS_MO_IMPI");
            }
            break;
        case ImsConstants.IMS_MO_DOMAIN:
            if(isRead) {
                nodeReadValue = telephony.getIsimDomain();
            } else {
                Log.e(TAG, "Write: Non-supported for IMS_MO_DOMAIN");
            }
            break;
        case ImsConstants.IMS_MO_PCSCF:
            if(isRead) {
                ImsMoInfo info = mPersistentDataStore.getImsMoInfo(impi);
                if(info == null) {
                    Log.e(TAG, "No ImsMoInfo");
                    return "";
                }
                return info.getPcscf();
            } else {
                ImsMoInfo info = mPersistentDataStore.getImsMoInfo(impi);
                if(info == null) {
                    info = new ImsMoInfo(impi);
                }
                info.setPcscf(nodeValue);
                mPersistentDataStore.addImsMoInfo(info);
                mPersistentDataStore.saveIfNeeded();
            }
            break;
        default:
            Log.e(TAG, "[handleImsMoString]No match id:" + nodeId);
            break;
        }

        Log.d(TAG, "handleImsMoString:" + nodeReadValue);
        if(nodeReadValue == null) nodeReadValue = "";

        return nodeReadValue;
    }

    synchronized private void setCacheImsMoConfiguration(String cfgName, String cfgValue) {
        SetCacheMo(cfgName, cfgValue);        
    }

    synchronized private String getCacheImsMoConfiguration(String cfgName) {
        return getCacheMo(cfgName);
    }

    synchronized private void setGenernalImsMoConfiguration(String cfgName, String cfgValue) {
        String atCmdLine = "AT+ECFGSET=\"" + cfgName + "\",\"" + cfgValue + "\"";
        Log.i(TAG, "setGenernalImsMoConfiguration:" + atCmdLine);
        getGsmPhone();

        if(IsTestMode) {
            SetCacheMo(cfgName, cfgValue);
            return;
        }
        mDone = false;
        mGsmPhone.invokeOemRilRequestRaw(getBytes(atCmdLine), mHandler.obtainMessage(EVENT_AT_CMD_WRITE_DONE));

        long waitTimeMillis = TIMEOUT_INTERACTION_MILLIS;
        synchronized(mAtCmdSyncObj) {
            while (!mDone) {
                try {
                    Log.i(TAG, "Wait Result");
                    mAtCmdSyncObj.wait(waitTimeMillis);
                    Log.i(TAG, "Result:" + mAtCmdResult);
                } catch(InterruptedException e) {
                    Log.e(TAG, "Interrupted exception");
                }
            }
        }
    }

    private String handleCommandResponse(String cmd) {
        String retStr = "";
        
        long waitTimeMillis = TIMEOUT_INTERACTION_MILLIS;
        synchronized(mAtCmdSyncObj) {
            while (!mDone) {
                try {
                    Log.i(TAG, "Wait Result");
                    mAtCmdSyncObj.wait(TIMEOUT_INTERACTION_MILLIS);
                } catch(InterruptedException e) {
                    Log.e(TAG, "Interrupted exception");
                }
            }
            Log.i(TAG, "handleCommandResponse:" + mAtCmdResult);

            if(mAtCmdResult.indexOf(cmd) != -1) {
                int pos = mAtCmdResult.indexOf(":");
                int pos2 =  mAtCmdResult.lastIndexOf("\"");
                if(pos2 != -1) { //handle string
                    if(mAtCmdResult.indexOf("ECFGGET") != -1) {
                        pos = mAtCmdResult.indexOf(",") + 1;
                    } else {
                        pos = mAtCmdResult.indexOf("\"");
                }
                Log.i(TAG, "pos:" + pos + "/" + pos2);
                    retStr = mAtCmdResult.substring(pos+1, pos2).trim();
                } else {     //handle int
                    pos2 =  mAtCmdResult.indexOf("\n", pos);
                    retStr = mAtCmdResult.substring(pos+1, pos2).trim();
                }
            } else {
                retStr = "";
            }
        }
        return retStr;
    }

    synchronized private String getGenernalImsMoConfiguration(String cfgName) {
        String atCmdLine = "AT+ECFGGET=\"" + cfgName + "\"";

        Log.i(TAG, "getGenernalImsMoConfiguration:" + atCmdLine);
        getGsmPhone();

        if(IsTestMode) {
            return getCacheMo(cfgName);
        }
        
        mAtCmdResult = "";
        mDone = false;
        mGsmPhone.invokeOemRilRequestRaw(getBytes(atCmdLine), mHandler.obtainMessage(EVENT_AT_CMD_READ_DONE));

        return handleCommandResponse("+ECFGGET");
    }

    private byte[] getBytes(String data) {
        int size = data.length();
        byte[] arr = new byte[size + 1];

        for(int i = 0; i < size; i++) {
            arr[i] = (byte) data.charAt(i);
        }
        arr[size] = 0;
        return arr;
    }

    /** Handler to do the network accesses on */
    private class MyHandler extends Handler {

        public MyHandler(Looper l) {
            super(l);
        }

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;

            Log.i(TAG, "msg.what:" + msg.what);

            switch (msg.what) {
            case EVENT_AT_CMD_WRITE_DONE:
                synchronized(mAtCmdSyncObj) {
                    mDone = true;
                    if (ar.exception != null) {
                        Log.e(TAG, "Exception");
                        mAtCmdResult = "0";
                    } else {
                        int i = 0;
                        byte[] result = (byte[]) ar.result;
                        mAtCmdResult = new String(result);
                    }
                    mAtCmdSyncObj.notifyAll();
                }
                break;
            case EVENT_AT_CMD_READ_DONE:
                synchronized(mAtCmdSyncObj) {
                    mDone = true;
                    if (ar.exception != null) {
                        Log.e(TAG, "Exception");
                        mAtCmdResult = "0";
                    } else {
                        int i = 0;
                        mAtCmdResult = new String((byte[]) ar.result);
                        if(mAtCmdResult.startsWith("+CME ERROR")) {
                            mAtCmdResult = "";
                        }
                    }
                    mAtCmdSyncObj.notifyAll();
                }

                break;
            }
        }
    }

    private int handleImsMoInt(int nodeId, boolean isRead, int nodeValue) {
        int ret = -1;
        String cfgName = "";

        switch(nodeId) {
        case ImsConstants.IMS_MO_TIMER_1:
            cfgName = "UA_timer_T1";
            break;
        case ImsConstants.IMS_MO_TIMER_2:
            cfgName = "UA_timer_T2";
            break;
        case ImsConstants.IMS_MO_TIMER_4:
            cfgName = "UA_timer_T4";
            break;
        case ImsConstants.IMS_MO_VOICE_E:
            if(isRead) {
                try {
                    Log.d(TAG, "[GsmPhone]getUeEUtranVoiceDomainPreference");
                    String atCmdLine = "AT+CEVDP?";
                    mAtCmdResult = "";
                    mDone = false;
                    mGsmPhone.invokeOemRilRequestRaw(getBytes(atCmdLine), mHandler.obtainMessage(EVENT_AT_CMD_READ_DONE));

                    String response = handleCommandResponse("+CEVDP");

                    try {
                        ret = Integer.parseInt(response);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                    //ret = mGsmPhone.getUeEUtranVoiceDomainPreference();
                    return ret;
                } catch(Exception e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    Log.d(TAG, "[GsmPhone]setUeEUtranVoiceDomainPreference");
                    mGsmPhone.setUeEUtranVoiceDomainPreference(nodeValue);
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
            break;
        case ImsConstants.IMS_MO_VOICE_U:
            if(isRead) {
                try {
                    cfgName = "IMS_MO_VOICE_U";
                    Log.d(TAG, "[GsmPhone]getUeUtranVoiceDomainPreference");
                    ret = 1;
                    try {
                        ret = Integer.parseInt(getCacheImsMoConfiguration(cfgName));
                    }catch(Exception e) {
                        e.printStackTrace();
                    }
                    return ret;
                    /*
                    String atCmdLine = "AT+CVDP?";
                    mAtCmdResult = "";
                    mGsmPhone.invokeOemRilRequestRaw(getBytes(atCmdLine), mHandler.obtainMessage(EVENT_AT_CMD_READ_DONE));
                    String response = handleCommandResponse("+CVDP");
                    try {
                        ret = Integer.parseInt(response);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                    //ret = mGsmPhone.getUeUtranVoiceDomainPreference();
                    return ret;
                    */
                } catch(Exception e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    cfgName = "IMS_MO_VOICE_U";
                    Log.d(TAG, "[GsmPhone]setUeUtranVoiceDomainPreference");
                    /*
                    Defined Values
                    <setting>: integer type; indicates the voice domain preference of the UE. The default value is manufacturer specific.
                    1 CS Voice only
                    2 CS Voice preferred, IMS PS Voice as secondary
                    3 IMS PS Voice preferred, CS Voice as secondary
                    */
                    //mGsmPhone.setUeUtranVoiceDomainPreference(nodeValue);
                    setCacheImsMoConfiguration(cfgName, String.valueOf(nodeValue));
                    return ret;
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
            break;
        case ImsConstants.IMS_MO_REG_BASE:
            cfgName = "UA_reg_retry_base_time";
            break;
        case ImsConstants.IMS_MO_REG_MAX:
            cfgName = "UA_reg_retry_max_time";
            break;
        default:
            Log.e(TAG, "[handleImsMoInt]No match id:" + nodeId);
            break;
        }

        if(cfgName.length() > 0) {
            if(isRead) {
                try {
                    String cfgValue = getGenernalImsMoConfiguration(cfgName);
                    Log.i(TAG, "Get Cfg info:" + cfgName + ":" + cfgValue);
                    ret = Integer.parseInt(cfgValue);
                } catch(Exception e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    Log.i(TAG, "Set Cfg info:" + cfgName + ":" + nodeValue);
                    setGenernalImsMoConfiguration(cfgName, String.valueOf(nodeValue));
                    ret = 0;
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
        }

        return ret;
    }

    private IBinder mBinder = new IImsManagerService.Stub() {

        public void setImsMoImpi(String imsi) {
            Log.d(TAG, "Configure:" + imsi);
            mImsi = imsi;
        }

        public void writeImsMoBool(int nodeId, boolean nodeValue) throws RemoteException {
            Log.d(TAG, "writeImsMoBool");
            try {
                handleImsMoBool(nodeId, false, nodeValue);
            } catch(Exception e) {
                e.printStackTrace();
            }
        }

        public boolean readImsMoBool(int nodeId) throws RemoteException {
            return handleImsMoBool(nodeId, true, false);
        }

        public void writeImsMoInt(int nodeId, int nodeValue) throws RemoteException {
            handleImsMoInt(nodeId, false, nodeValue);
        }

        public int readImsMoInt(int nodeId)throws RemoteException {
            return handleImsMoInt(nodeId, true, -1);
        }

        public void writeImsMoString(int nodeId, String nodeValue)throws RemoteException {
            handleImsMoString(nodeId, false, nodeValue, mImsi);
        }

        public String readImsMoString(int nodeId)throws RemoteException {
            return handleImsMoString(nodeId, true, null, mImsi);
        }

        public void writeImsMoStringArray(int nodeId, String[] nodesValue) throws RemoteException {
            Log.e(TAG, "N/A in writeImsMoStringArray:" + nodeId);
        }

        public String[] readImsMoStringArray(int nodeId)throws RemoteException {
            return handleImsMoStringArray(nodeId, true, null);
        }

        public void writeImsIcsiMo(ImsIcsi[] nodesValue) throws RemoteException {
            int i = 0;

            for(i = 0; i < MAX_MO_COUNT && i < nodesValue.length; i++) {
                String cfgIcsiName = "icsi_" + (i+1);
                String cfgIcsiResouceName = "icsi_resource_allocation_mode_" + (i+1);
                Log.i(TAG, "set cfgIcsiName:" + cfgIcsiName + ":" + nodesValue[i].getIcsi());
                setGenernalImsMoConfiguration(cfgIcsiName, nodesValue[i].getIcsi());
                Log.i(TAG, "set cfgIcsiName:" + cfgIcsiResouceName + ":" + nodesValue[i].getIsAllocated());
                setGenernalImsMoConfiguration(cfgIcsiResouceName, (nodesValue[i].getIsAllocated()) ? "1" : "0");
            }
        }

        public ImsIcsi[] readImsIcsiMo()throws RemoteException {
            List<ImsIcsi> icsiList = new ArrayList<ImsIcsi>();
            int i = 0;

            for(i = 0; i < MAX_MO_COUNT; i++) {
                icsiList.add(new ImsIcsi("", false));
            }

            for(i = 0; i < MAX_MO_COUNT; i++) {
                String cfgIcsiName = "icsi_" + (i+1);
                String cfgIcsiResouceName = "icsi_resource_allocation_mode_" + (i+1);

                try {
                    String icsi = getGenernalImsMoConfiguration(cfgIcsiName);
                    String isAllocated = getGenernalImsMoConfiguration(cfgIcsiResouceName);
                    if(icsi.length() > 0) {
                        icsiList.set(i, new ImsIcsi(icsi, isAllocated.equals("1") ? true : false));
                    }
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }

            return icsiList.toArray(new ImsIcsi[icsiList.size()]);
        }

        public void writeImsAuthInfoMo(ImsAuthInfo nodeValue) throws RemoteException {
            Log.i(TAG, "writeImsAuthInfoMo:" + nodeValue);
            ImsMoInfo info = mPersistentDataStore.getImsMoInfo(mImsi);
            if(info == null) {
                info = new ImsMoInfo(mImsi);
            }
            info.setImsAuthInfo(nodeValue);
            mPersistentDataStore.addImsMoInfo(info);
            mPersistentDataStore.saveIfNeeded();
        }

        public ImsAuthInfo readImsAuthInfoMo()throws RemoteException {
            ImsMoInfo info = mPersistentDataStore.getImsMoInfo(mImsi);

            if(info == null) {
                Log.e(TAG, "No ImsMoInfo in readImsAuthInfoMo");
                return new ImsAuthInfo();
            } else if(info.getImsAuthInfo() == null) {
                Log.e(TAG, "No ImsAuthInfo in getImsAuthInfo");
                return new ImsAuthInfo();
            }

            return info.getImsAuthInfo();
        }

        public void writeImsLboPcscfMo(ImsLboPcscf[] nodesValue) throws RemoteException {
            int i = 0;

            ImsMoInfo info = mPersistentDataStore.getImsMoInfo(mImsi);
            if(info == null) {
                info = new ImsMoInfo(mImsi);
            }

            //Remove old PCSCF list
            info.removeLboPcscfList();

            for(i = 0; i < nodesValue.length && i < MAX_MO_COUNT; i++) {
                info.addLboPcscfList(nodesValue[i]);
            }
            mPersistentDataStore.addImsMoInfo(info);
            mPersistentDataStore.saveIfNeeded();
        }

        public ImsLboPcscf[] readImsLboPcscfMo()throws RemoteException {
            Collection<ImsLboPcscf> result = new ArrayList<ImsLboPcscf>();

            ImsMoInfo info = mPersistentDataStore.getImsMoInfo(mImsi);
            int i = 0;

            result.clear();
            for(i = 0; i < MAX_MO_COUNT; i++) {
                result.add(new ImsLboPcscf());
            }

            if(info == null) {
                Log.e(TAG, "No ImsMoInfo in readImsLboPcscfMo");
            } else {
                result.clear();
                result = info.getLboPcscfList();
                Log.i(TAG, "getLboPcscfList:" + result.size());
            }

            return result.toArray(new ImsLboPcscf[result.size()]);
        }

        public void writeImsImsPhoneCtxMo(ImsPhoneCtx[] nodesValue) throws RemoteException {
            int i = 0, j = 0;

            for(i = 0; i < MAX_MO_COUNT && i < nodesValue.length; i++) {
                String cfgCtxName = "UA_phone_context_" + (i+1);

                Log.i(TAG, "set cfgCtxName:" + cfgCtxName + ":" + nodesValue[i].getPhoneCtx());
                if(i == 0){
                    cfgCtxName = "UA_phone_context";
                    setGenernalImsMoConfiguration(cfgCtxName, nodesValue[i].getPhoneCtx());
                }else{                    
                    setCacheImsMoConfiguration(cfgCtxName, nodesValue[i].getPhoneCtx());
                }

                String[] CtxIpuis = nodesValue[i].getPhoneCtxIpuis();
                for(j = 0; j < CtxIpuis.length && j  < MAX_MO_COUNT; j++) {
                    String cfgCtxIpuiName = "UA_phone_context_associated_impu_" + (i+1) + "_" + (j+1);
                    Log.i(TAG, "set cfgCtxIpuiName:" + cfgCtxIpuiName + ":" + CtxIpuis[j]);
                    if(i == 0 && j == 0){
                        cfgCtxIpuiName = "UA_phone_context_associated_impu";
                        setGenernalImsMoConfiguration(cfgCtxIpuiName, CtxIpuis[j]);
                    }else{
                        setCacheImsMoConfiguration(cfgCtxIpuiName, CtxIpuis[j]);
                    }
                }
            }
        }

        public ImsPhoneCtx[] readImsPhoneCtxMo()throws RemoteException {
            List<ImsPhoneCtx> phoneCtxList = new ArrayList<ImsPhoneCtx>();
            List<String> phoneCtxImpi = new ArrayList<String>();
            String ctx = "";
            int i = 0, j = 0;

            phoneCtxList.clear();
            for(i = 0; i < MAX_MO_COUNT; i++) {
                ctx = "";
                phoneCtxImpi.clear();
                for( j = 0; j < MAX_MO_COUNT; j++) {
                    phoneCtxImpi.add("");
                }
                phoneCtxList.add(new ImsPhoneCtx(ctx, phoneCtxImpi.toArray(new String[phoneCtxImpi.size()])));
            }

            for(i = 0; i < MAX_MO_COUNT; i++) {
                String cfgCtxName = "UA_phone_context_" + (i+1);

                try {
                    if(i == 0){
                        cfgCtxName = "UA_phone_context";
                    ctx = getGenernalImsMoConfiguration(cfgCtxName);
                    }else{
                        ctx = getCacheImsMoConfiguration(cfgCtxName);
                    }
                    Log.i(TAG, "readImsPhoneCtxMo:" + ctx);
                    if(ctx.length() == 0) {
                        continue;
                    }

                    for( j = 0; j < MAX_MO_COUNT; j++) {
                        String cfgCtxIpuiName = "UA_phone_context_associated_impu_" + (i+1) + "_" + (j+1);                        
                        String ctxImpu = getCacheImsMoConfiguration(cfgCtxIpuiName);
                        if(i == 0 && j == 0){
                            cfgCtxIpuiName = "UA_phone_context_associated_impu";
                            ctxImpu = getGenernalImsMoConfiguration(cfgCtxIpuiName);
                        }
                        Log.d(TAG, i + " ctxImpu:" + ctxImpu);
                        if(ctxImpu.length() > 0) {
                            phoneCtxImpi.set(j,ctxImpu);
                        }
                    }
                    phoneCtxList.set(i, new ImsPhoneCtx(ctx, phoneCtxImpi.toArray(new String[MAX_MO_COUNT])));
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }

            return phoneCtxList.toArray(new ImsPhoneCtx[MAX_MO_COUNT]);
        }

        public void writeImsXcapInfoMo(ImsXcapInfo nodeValue) throws RemoteException {
            int i = 0;
            Log.i(TAG, "writeImsXcapInfoMo:" + nodeValue);

            ImsMoInfo info = mPersistentDataStore.getImsMoInfo(mImsi);
            if(info == null) {
                info = new ImsMoInfo(mImsi);
            }
            info.setImsXcapInfo(nodeValue);
            mPersistentDataStore.addImsMoInfo(info);
            mPersistentDataStore.saveIfNeeded();
        }

        public ImsXcapInfo readImsXcapInfoMo()throws RemoteException {
            ImsMoInfo info = mPersistentDataStore.getImsMoInfo(mImsi);
            if(info == null) {
                Log.e(TAG, "No ImsMoInfo in readImsXcapInfoMo");
                return new ImsXcapInfo();
            } else if(info.getImsXcapInfo() == null) {
                Log.e(TAG, "No ImsXcapInfo in getImsXcapInfo");
                return new ImsXcapInfo();
            }

            return info.getImsXcapInfo();
        }

        private void dumpMo(Object[] nodesValue) {
            int i = 0;
            for(i = 0; i < nodesValue.length; i++) {
                Log.i(TAG, (String) nodesValue[i].toString());
            }
        }
    };
}
