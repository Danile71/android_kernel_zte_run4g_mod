package com.mediatek.settings.plugin;

import android.app.ActionBar;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.widget.Switch;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.CellConnService.CellConnMgr;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;


public class DataConnectionSettings extends PreferenceActivity {

    private static final String TAG = "DataConnectionSettings";

    private static final String SIM_SLOT_1_KEY = "data_connection_sim_slot_1";
    private static final String SIM_SLOT_2_KEY = "data_connection_sim_slot_2";

    private static final int PROGRESS_DIALOG = 1000;

    private ConnectivityManager mConnectivityManager;
    private CellConnMgr mCellConnMgr;
    private boolean mIsDataConnectActing;
    private IntentFilter mIntentFilter;
    private RadioPreference mSlot1Preference;
    private RadioPreference mSlot2Preference;
    private int mProDlgMsgId = -1;
    private DataConnectionEnabler mDataConnectionEnabler;

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
                        && state != null && state.equals(PhoneConstants.DataState.CONNECTED)) {
                    if (mIsDataConnectActing) {
                        mHandler.removeMessages(DataConnectionEnabler.DATA_SWITCH_TIME_OUT_MSG);
                        mIsDataConnectActing = false;
                        hideProgressDlg();
                    }
                    updateScreen();
                }
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                updateScreen();
            } else if (action.equals(Intent.ACTION_DUAL_SIM_MODE_CHANGED)) {
                updateScreen();
            }
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (DataConnectionEnabler.DATA_SWITCH_TIME_OUT_MSG == msg.what) {
                Xlog.i(TAG, "reveive time out msg...");
                mIsDataConnectActing = false;
                hideProgressDlg();
                updateScreen();
            }
        }
    };

    private ContentObserver mDataConnectionSlotObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Xlog.d(TAG, "mDataConnectionSlotObserver onChange selfChange=" + selfChange);
            if (!selfChange) {
                updateScreen();
            }
        }
    };

    private ContentObserver mDataConnectionObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Xlog.d(TAG, "mDataConnectionObserver onChange selfChange=" + selfChange);
            if (!selfChange) {
                if (mIsDataConnectActing) {
                    mHandler.removeMessages(DataConnectionEnabler.DATA_SWITCH_TIME_OUT_MSG);
                    mIsDataConnectActing = false;
                    hideProgressDlg();
                }
                updateScreen();
            }
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.data_connection_settings);
        mSlot1Preference = (RadioPreference) findPreference(SIM_SLOT_1_KEY);
        mSlot2Preference = (RadioPreference) findPreference(SIM_SLOT_2_KEY);
        mIntentFilter = new IntentFilter(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        mIntentFilter.addAction(DataConnectionEnabler.TRANSACTION_START);
        mIntentFilter.addAction(DataConnectionEnabler.TRANSACTION_STOP);
        mConnectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        mCellConnMgr = new CellConnMgr();
        mCellConnMgr.register(this);

        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        Switch actionBarSwitch = (Switch) inflater.inflate(com.mediatek.internal.R.layout.imageswitch_layout, null);
        final int padding = getResources().getDimensionPixelSize(R.dimen.action_bar_switch_padding);
        actionBarSwitch.setPadding(0, 0, padding, 0);
        getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar().setCustomView(
                actionBarSwitch,
                new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT,
                        Gravity.CENTER_VERTICAL | Gravity.END));
        mDataConnectionEnabler = new DataConnectionEnabler(this, actionBarSwitch);

        mIsDataConnectActing = false;
    }

    @Override
    public void onResume() {
        super.onResume();
        registerReceiver(mReceiver, mIntentFilter);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(Settings.Global.MOBILE_DATA),
                true, mDataConnectionObserver);
        getContentResolver().registerContentObserver(Settings.System.getUriFor(Settings.System.GPRS_CONNECTION_SETTING),
                true, mDataConnectionSlotObserver);
        updateScreen();
        if (mDataConnectionEnabler != null) {
            mDataConnectionEnabler.resume();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mReceiver);
        getContentResolver().unregisterContentObserver(mDataConnectionObserver);
        getContentResolver().unregisterContentObserver(mDataConnectionSlotObserver);
        if (mDataConnectionEnabler != null) {
            mDataConnectionEnabler.pause();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mCellConnMgr.unregister();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference.equals(mSlot1Preference)) {
            handleDataConnectionChange(PhoneConstants.GEMINI_SIM_1);
            return true;
        } else if (preference.equals(mSlot2Preference)) {
            handleDataConnectionChange(PhoneConstants.GEMINI_SIM_2);
            return true;
        } else {
            return super.onPreferenceTreeClick(preferenceScreen, preference);
        }
    }

    @Override
    public Dialog onCreateDialog(int id) {
        switch (id) {
        case PROGRESS_DIALOG:
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getResources().getString(mProDlgMsgId));
            dialog.setIndeterminate(true);
            dialog.setCancelable(false);
            return dialog;
        default:
            return null;
        }
    }

    private void updateScreen() {
        boolean enabled = mConnectivityManager.getMobileDataEnabled() && !mIsDataConnectActing
                && DataConnectionEnabler.isGPRSEnable(this);
        boolean slot1RadioOn = DataConnectionEnabler.isTargetSlotRadioOn(PhoneConstants.GEMINI_SIM_1);
        boolean slot2RadioOn = DataConnectionEnabler.isTargetSlotRadioOn(PhoneConstants.GEMINI_SIM_2);
        boolean slot1NotLocked = slot1RadioOn
                && DataConnectionEnabler.getSimIndicator(this, PhoneConstants.GEMINI_SIM_1)
                        != PhoneConstants.SIM_INDICATOR_LOCKED;
        boolean slot2NotLocked = slot2RadioOn
                && DataConnectionEnabler.getSimIndicator(this, PhoneConstants.GEMINI_SIM_2)
                        != PhoneConstants.SIM_INDICATOR_LOCKED;
        int dataConnectionId = getDataConnectionSlotId(this);
        Xlog.d(TAG, "updateSwitcherState enalbed=" + enabled + ", mIsDataConnectActing=" + mIsDataConnectActing
                + ", slot1RadioOn=" + slot1RadioOn + ", slot2RadioOn=" + slot2RadioOn
                + ", slot1NotLocked=" + slot1NotLocked + ", slot2NotLocked=" + slot2NotLocked
                + ", dataConnectionId=" + dataConnectionId);

        mSlot1Preference.setEnabled(enabled && slot1RadioOn && slot1NotLocked);
        mSlot2Preference.setEnabled(enabled && slot2RadioOn && slot2NotLocked);
        if (dataConnectionId == PhoneConstants.GEMINI_SIM_1) {
            mSlot1Preference.setChecked(true);
            mSlot2Preference.setChecked(false);
        } else if (dataConnectionId == PhoneConstants.GEMINI_SIM_2) {
            mSlot1Preference.setChecked(false);
            mSlot2Preference.setChecked(true);
        } else {
            mSlot1Preference.setChecked(false);
            mSlot2Preference.setChecked(false);
        }
    }

    private void handleDataConnectionChange(int newSlot) {
        Xlog.d(TAG, "handleDataConnectionChange newSlot=" + newSlot);
        if (newSlot >= PhoneConstants.GEMINI_SIM_1
                && DataConnectionEnabler.getSimIndicator(this, newSlot) == PhoneConstants.SIM_INDICATOR_LOCKED) {
            mCellConnMgr.handleCellConn(newSlot, DataConnectionEnabler.PIN1_REQUEST_CODE);
            return;
        }
        // The data connectionId index starts from 1, need to minus 1 to match Gemini SIM slot id
        if (getDataConnectionSlotId(this) != newSlot) {
            Intent intent = new Intent(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
            intent.putExtra("simid", SimInfoManager.getSimInfoBySlot(this, newSlot).mSimInfoId);
            sendBroadcast(intent);
            showDataConnDialog(newSlot >= PhoneConstants.GEMINI_SIM_1);
        }
    }

    /**
     *show attach gprs dialog and revent time out to send a delay msg
     * 
     */
    private void showDataConnDialog(boolean isConnect) {
        long delaytime = 0;
        if (isConnect) {
            delaytime = DataConnectionEnabler.ATTACH_DATA_CONN_TIME_OUT;
        } else {
            delaytime = DataConnectionEnabler.DETACH_DATA_CONN_TIME_OUT;
        }
        mHandler.sendEmptyMessageDelayed(DataConnectionEnabler.DATA_SWITCH_TIME_OUT_MSG, delaytime);
        showProgressDlg(R.string.gemini_data_connection_progress_message);
        mIsDataConnectActing = true;
    }

    private void showProgressDlg(int dialogMsg) {
        Xlog.d(TAG,"showProgressDlg() with dialogMsg = " + dialogMsg);
        mProDlgMsgId = dialogMsg;
        showDialog(PROGRESS_DIALOG);
    }

    private void hideProgressDlg() {
        Xlog.d(TAG, "hideProgressDlg()");
        dismissDialog(PROGRESS_DIALOG);
    }

    private static int getDataConnectionSlotId(Context context) {
        // Data framework defined that this id is actual (slotId + 1), we need to minus 1 before return
        return (int) Settings.System.getLong(context.getContentResolver(), Settings.System.GPRS_CONNECTION_SETTING,
                Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
    }

    private static PhoneConstants.DataState getMobileDataState(Intent intent) {
        String str = intent.getStringExtra(PhoneConstants.STATE_KEY);
        if (str != null) {
            return Enum.valueOf(PhoneConstants.DataState.class, str);
        } else {
            return PhoneConstants.DataState.DISCONNECTED;
        }
    }
}
