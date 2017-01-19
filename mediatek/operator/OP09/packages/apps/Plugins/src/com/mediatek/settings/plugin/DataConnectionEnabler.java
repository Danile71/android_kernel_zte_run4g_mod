package com.mediatek.settings.plugin;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.widget.CompoundButton;
import android.widget.Switch;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;

/**
 * DataConnectionEnabler is a helper to manage the Data connection on/off checkbox
 * preference. It turns on/off Data connection and ensures the summary of the
 * preference reflects the current state.
 */
public final class DataConnectionEnabler implements CompoundButton.OnCheckedChangeListener {

    private static final String TAG = "DataConnectionEnabler";

    public static final String TRANSACTION_START = "com.android.mms.transaction.START";
    public static final String TRANSACTION_STOP = "com.android.mms.transaction.STOP";

    static final int PIN1_REQUEST_CODE = 302;

    // time out message event
    static final int DETACH_DATA_CONN_TIME_OUT = 10000;// in ms
    static final int ATTACH_DATA_CONN_TIME_OUT = 30000;// in ms

    static final int DATA_SWITCH_TIME_OUT_MSG = 1000;

    private static final int ALL_RADIO_OFF = 0;

    private final Context mContext;
    private ConnectivityManager mConnectivityManager;
    private Switch mSwitch;
    private boolean mIsDataConnectActing;
    private IntentFilter mIntentFilter;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                String reason = intent.getStringExtra(PhoneConstants.STATE_CHANGE_REASON_KEY);
                String apnTypeList = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
                PhoneConstants.DataState state = DataConnectionEnabler.getMobileDataState(intent);
                int slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
                Xlog.d(TAG, "onReceive action=" + action + ", ****the slot " + slotId + ", state=" + state +
                        ", reason =" + reason + ", apnTypeList=" + apnTypeList);
                if (PhoneConstants.APN_TYPE_DEFAULT.equals(apnTypeList)
                        && state != null
                        && (state.equals(PhoneConstants.DataState.CONNECTED) || state
                                .equals(PhoneConstants.DataState.DISCONNECTED))) {
                    if (mIsDataConnectActing) {
                        mHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
                        mIsDataConnectActing = false;
                    }
                    updateSwitcherState();
                }
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                updateSwitcherState();
            } else if (action.equals(Intent.ACTION_DUAL_SIM_MODE_CHANGED)) {
                updateSwitcherState();
            }
        }
    };

    private ContentObserver mDataConnectionObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Xlog.d(TAG, "onChange selfChange=" + selfChange);
            if (!selfChange) {
                updateSwitcherState();
            }
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (DATA_SWITCH_TIME_OUT_MSG == msg.what) {
                Xlog.i(TAG, "reveive time out msg...");
                mIsDataConnectActing = false;
                updateSwitcherState();
            }
        }
    };

    public DataConnectionEnabler(Context context, Switch switchs) {
        mContext = context;
        mConnectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        mSwitch = switchs;
        mIntentFilter = new IntentFilter(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        mIntentFilter.addAction(TRANSACTION_START);
        mIntentFilter.addAction(TRANSACTION_STOP);
        mIsDataConnectActing = false;
    }

    public void resume() {
        mContext.registerReceiver(mReceiver, mIntentFilter);
        mContext.getContentResolver().registerContentObserver(Settings.Global.getUriFor(Settings.Global.MOBILE_DATA),
                true, mDataConnectionObserver);
        mSwitch.setOnCheckedChangeListener(this);
        mSwitch.setChecked(mConnectivityManager.getMobileDataEnabled());
        updateSwitcherState();
        Xlog.d(TAG, "resume mConnectivityManager.getMobileDataEnabled()=" + mConnectivityManager.getMobileDataEnabled());
    }

    public void pause() {
        mContext.unregisterReceiver(mReceiver);
        mContext.getContentResolver().unregisterContentObserver(mDataConnectionObserver);
        mSwitch.setOnCheckedChangeListener(null);
    }

    public void setSwitch(Switch switchs) {
        if (mSwitch == switchs) {
            return;
        }
        mSwitch.setOnCheckedChangeListener(null);
        mSwitch = switchs;
        mSwitch.setOnCheckedChangeListener(this);
        mSwitch.setChecked(mConnectivityManager.getMobileDataEnabled());
        updateSwitcherState();
    }

    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (isChecked != mConnectivityManager.getMobileDataEnabled()) {
            mConnectivityManager.setMobileDataEnabled(isChecked);
            mSwitch.setEnabled(false);
            mIsDataConnectActing = true;
            mHandler.sendEmptyMessageDelayed(DATA_SWITCH_TIME_OUT_MSG, isChecked ? ATTACH_DATA_CONN_TIME_OUT
                    : DETACH_DATA_CONN_TIME_OUT);
            Xlog.d(TAG, "onCheckChanged to " + isChecked + ", and do switch");
        } else {
            Xlog.d(TAG, "onCheckChanged to " + isChecked + ", state is not changed, do nothing");
        }
    }

    private void updateSwitcherState() {
        boolean enabled = !mIsDataConnectActing && isGPRSEnable(mContext);
        Xlog.d(TAG, "updateSwitcherState enalbed=" + enabled);
        mSwitch.setEnabled(enabled);
        ///M: when receive the broadcast ,update the switch on/off
        mSwitch.setChecked(mConnectivityManager.getMobileDataEnabled());
    }
    /**
     * Returns whether is in airplance or mms is under transaction
     * 
     * @return is airplane or mms is in transaction
     * 
     */
    static boolean isGPRSEnable(Context context) {
        boolean isMMSProcess = false;
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            NetworkInfo networkInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_MOBILE_MMS);
            if (networkInfo != null) {
                NetworkInfo.State state = networkInfo.getState();
                Xlog.d(TAG,"mms state = " + state);
                isMMSProcess = (state == NetworkInfo.State.CONNECTING
                    || state == NetworkInfo.State.CONNECTED);
            }
        }
        boolean isRadioOff = isRadioOff(context);
        boolean hasSimUnLocked = !isRadioOff && hasSlotRadioOnNotLocked(context);
        Xlog.d(TAG, "isMMSProcess=" + isMMSProcess + " isRadioOff=" + isRadioOff + ", hasSimUnLocked=" + hasSimUnLocked);
        return !isMMSProcess && !isRadioOff && hasSimUnLocked;
    }

    private static boolean hasSlotRadioOnNotLocked(Context context) {
        for (int slot = PhoneConstants.GEMINI_SIM_1; slot < PhoneConstants.GEMINI_SIM_NUM; slot ++) {
            if (isTargetSlotRadioOn(slot)
                    && (getSimIndicator(context, slot) != PhoneConstants.SIM_INDICATOR_LOCKED)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @return is airplane mode or all sim card is set on radio off
     * 
     */
    static boolean isRadioOff(Context context) {
        boolean isAllRadioOff = (Settings.System.getInt(context.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, -1) == 1)
                || (Settings.System.getInt(context.getContentResolver(),
                        Settings.System.DUAL_SIM_MODE_SETTING, -1) == ALL_RADIO_OFF)
                || SimInfoManager.getInsertedSimInfoList(context).size() == 0;
        Xlog.d(TAG, "isAllRadioOff=" + isAllRadioOff);
        return isAllRadioOff;
    }

    static PhoneConstants.DataState getMobileDataState(Intent intent) {
        String str = intent.getStringExtra(PhoneConstants.STATE_KEY);
        if (str != null) {
            return Enum.valueOf(PhoneConstants.DataState.class, str);
        } else {
            return PhoneConstants.DataState.DISCONNECTED;
        }
    }

    static int getSimIndicator(Context context,  int slotId) {
        Xlog.d(TAG,"getSimIndicator---slotId=" + slotId);
        if (!isTargetSlotRadioOn(slotId)) {
            Xlog.d(TAG,"Force the state to be radio off as airplane mode or dual sim mode");
            return PhoneConstants.SIM_INDICATOR_RADIOOFF;    
        } else {
            return getSimIndicator(context.getContentResolver(), slotId);
        }
    }

    static boolean isTargetSlotRadioOn(int slotId) {
        try {
            ITelephonyEx iTelephonyEx = 
                ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
            if (iTelephonyEx != null) {
                return iTelephonyEx.isRadioOn(slotId);
            }
        } catch  (RemoteException e) {
            Xlog.d(TAG, "isTargetSlotRadioOn query error, return false");
        }
        return false;
    }

    /**
     * Get the sim inidicators 
     * @param iTelephony Itelephony interface
     * @param slotId sim slot id
     * @return sim indicator
     */
    public static int getSimIndicator(ContentResolver resolver, int slotId) {
        Xlog.d(TAG,"getSimIndicator---slotId=" + slotId);
        boolean isAirplaneOn = Settings.System.getInt(resolver,
                Settings.System.AIRPLANE_MODE_ON, -1) == 1;
        if (isAirplaneOn) {
            Xlog.d(TAG,"isAirplaneOn = " + isAirplaneOn);
            return PhoneConstants.SIM_INDICATOR_RADIOOFF;
        }
        int indicator = PhoneConstants.SIM_INDICATOR_UNKNOWN;
        try {
            ITelephonyEx iTelephonyEx = 
                ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
            if (iTelephonyEx != null) {
                indicator = iTelephonyEx.getSimIndicatorState(slotId);
            }
        } catch (RemoteException e) {
            Xlog.e(TAG, "RemoteException");
        } catch (NullPointerException ex) {
            Xlog.e(TAG, "NullPointerException");
        }
        return indicator;
    }
}
