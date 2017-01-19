/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.settings.plugin;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.provider.Telephony.SIMInfo;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

public class SlotSettingsFragment extends PreferenceFragment {
    private static final String TAG = "SlotSettingsFragment";

    private static final String INTENT_ACTION_START_SWITCH_PHONE = "com.mediatek.intent.action.START_RADIO_TECHNOLOGY";

    public static final String NUMERIC_CHINA_TELE = "46003";
    public static final String NUMERIC_CHINA_MACAO_TELE = "45502";
    public static final String NUMERIC_NO_NETWORK = "00000";
    public static final String NUMERIC_UNKNOWN = "-1";

    private static final int DLG_NETWORK_AUTO_SELECT = 1;

    private static final String PREFERENCES_NAME = "SlotSettingsFragment";
    private static final String GSM_AUTO_NETWORK_SELECTION = "gsm_auto_network_selection";

    // China numeric
    private static final String NATIVE_NUMERIC1 = "46003";
    private static final String NATIVE_NUMERIC2 = "45502";

    // constant for current sim mode
    private static final int ALL_RADIO_OFF = 0;
    private static final int SIM_SLOT_1_RADIO_ON = 1;
    private static final int SIM_SLOT_2_RADIO_ON = 2;
    private static final int ALL_RADIO_ON = 3;

    // when sim radio switch complete receive msg with this id
    private static final int EVENT_DUAL_SIM_MODE_CHANGED_COMPLETE = 1;

    // when finish gsm network auto select, receive msg with this id
    private static final int EVENT_AUTO_SELECT_DONE = 2;

    private static final String KEY_SIM_RADIO_STATE = "sim_radio_state";
    private static final String KEY_CURRENT_NETWORK_INFO = "current_network_info";
    private static final String KEY_MANUAL_NETWORK_CDMA_SELECTION = "manual_network_selection";
    private static final String KEY_MANUAL_NETWORK_GSM_SELECTION = "manual_network_selection_gsm";
    private static final String KEY_ROAMING_HOTLINE = "roaming_hotline";
    private static final String KEY_NOTES = "notes";

    private int mTargetSlot = 0;

    private TelephonyManager mTelephonyManager;
    private ITelephony mTelephony;
    private ITelephonyEx mTelephonyEx;
    private TelephonyManagerEx mTelephonyManagerEx;
    private ConnectivityManager mConnService;

    private Dialog mDialog;

    private IntentFilter mIntentFilter;
    private SIMInfo mSiminfo;
    private boolean mIsSim1Inserted;
    private boolean mIsSim2Inserted;
    private int mTotalSimNumer;

    private CheckBoxPreference mEnableSimRadioPref;
    private Preference mNetworkInfoPref;
    private Preference mManualNetworkPref;
    private CheckBoxPreference mManualNetworkGsmPref;
    private Preference mRoamingHotlinePref;
    private Preference mNotesPref;
    private boolean mIsSIMRadioSwitching = false;
    private Phone mPhone;
    private GeminiPhone mGeminiPhone;
    private boolean mIsForeground;
    private int mCardType;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Xlog.d(TAG, "handleMessage--mIsSIMRadioSwitching = " + mIsSIMRadioSwitching);
            switch (msg.what) {
            case EVENT_DUAL_SIM_MODE_CHANGED_COMPLETE:
                if (mIsSIMRadioSwitching && EVENT_DUAL_SIM_MODE_CHANGED_COMPLETE == msg.what) {
                    mIsSIMRadioSwitching = false;
                    if (mDialog != null) {
                        mDialog.dismiss();
                    }
                }
                break;
            case EVENT_AUTO_SELECT_DONE:
                if (mDialog != null) {
                    mDialog.dismiss();
                }
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Xlog.d(TAG, "EVENT_AUTO_SELECT_DONE network selection: failed!");
                    if (mIsForeground) {
                        displayNetworkSelectionFailed(ar.exception);
                    }
                } else {
                    Xlog.d(TAG, "EVENT_AUTO_SELECT_DONE network selection: succeed!");
                    if (mIsForeground) {
                        displayNetworkSelectionSucceeded();
                    }
                }
                break;

            default:
                break;
            }
        }
    };

    private Messenger mSwitchRadioStateMsg = new Messenger(mHandler);

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.d(TAG, "onReceive action = " + action);
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)
                    || action.equals(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED)
                    || action.equals(Intent.ACTION_DUAL_SIM_MODE_CHANGED)) {
                updateScreen();
            } else if (action.equals(INTENT_ACTION_START_SWITCH_PHONE)) {
                // Under switch not allowed to do these operation
                mNetworkInfoPref.setEnabled(false);
                mManualNetworkPref.setEnabled(false);
            } else if (action.equals(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED)) {
                updateScreen();
            } else if (action.equals(TelephonyIntents.ACTION_SIM_DETECTED)) {
                // Update SIM inserted status and update screen when SIM changes.
                updateSimInsertedState();
                updateScreen();
            }
        }
    };

    /**
     * Set the slot id of the fragment, this should be done before
     * Fragment.onCreate().
     * 
     * @param slot
     */
    public void setSlotId(int slot) {
        mTargetSlot = slot;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.d(TAG, "onCreate + mTargetSlot = " + mTargetSlot);
        if (mTargetSlot != PhoneConstants.GEMINI_SIM_1
                && mTargetSlot != PhoneConstants.GEMINI_SIM_2) {
            throw new IllegalStateException(
                    "Require sim slot is either slot1 or slo2");
        }

        addPreferencesFromResource(R.xml.slot_network_settings);
        mTotalSimNumer = SIMInfo.getInsertedSIMCount(getActivity());
        mTelephonyManager = (TelephonyManager) getActivity().getSystemService(
                Context.TELEPHONY_SERVICE);
        mTelephony = ITelephony.Stub.asInterface(ServiceManager
                .getService("phone"));
        mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
        try {
            if (mTelephonyEx != null) {
                mCardType = mTelephonyEx.getInternationalCardType(mTargetSlot);
            }
        } catch (android.os.RemoteException ex) {
            Xlog.v(TAG, "onCreate error to getInternationalCardType ");
        }

        mConnService = ConnectivityManager.from(getActivity());
        mEnableSimRadioPref = (CheckBoxPreference) findPreference(KEY_SIM_RADIO_STATE);
        mNetworkInfoPref = findPreference(KEY_CURRENT_NETWORK_INFO);
        mManualNetworkPref = findPreference(KEY_MANUAL_NETWORK_CDMA_SELECTION);
        mManualNetworkGsmPref = (CheckBoxPreference) findPreference(KEY_MANUAL_NETWORK_GSM_SELECTION);
        mRoamingHotlinePref = findPreference(KEY_ROAMING_HOTLINE);
        mNotesPref = findPreference(KEY_NOTES);

        if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
            getPreferenceScreen().removePreference(mRoamingHotlinePref);
            getPreferenceScreen().removePreference(mNotesPref);
            getPreferenceScreen().removePreference(mManualNetworkPref);
        } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
            getPreferenceScreen().removePreference(mManualNetworkGsmPref);
        }
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction("android.intent.action.START_RADIO_TECHNOLOGY");
        mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        mIntentFilter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_DETECTED);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mIntentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        }
        updateSimInsertedState();
    }

    private void updateSimInsertedState() {
        try {
            if (mTelephony != null && mTelephonyEx != null) {
                mIsSim1Inserted = mTelephonyEx.hasIccCard(PhoneConstants.GEMINI_SIM_1);
                mIsSim2Inserted = mTelephonyEx.hasIccCard(PhoneConstants.GEMINI_SIM_2);
                mTelephony.registerForSimModeChange(mSwitchRadioStateMsg.getBinder(),
                        EVENT_DUAL_SIM_MODE_CHANGED_COMPLETE);
                Xlog.d(TAG, "updateSimInsertedState mIsSim1Inserted=" + mIsSim1Inserted
                        + ", mIsSim2Inserted=" + mIsSim2Inserted);
            }
        } catch (RemoteException e) {
            Xlog.e(TAG, "mTelephony exception");
            return;
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        mIsForeground = true;
        updateSimInsertedState();
        mSiminfo = SIMInfo.getSIMInfoBySlot(getActivity(), mTargetSlot);
        Xlog.d(TAG, "onResume: mTargetSlot = " + mTargetSlot + ", mSiminfo = " + mSiminfo);

        updateScreen();
        if (mSiminfo != null) {
            initPreferenceState();
        }
        getActivity().registerReceiver(mReceiver, mIntentFilter);
    }

    @Override
    public void onPause() {
        super.onPause();
        Xlog.d(TAG, "onPause");
        mIsForeground = false;
        getActivity().unregisterReceiver(mReceiver);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Xlog.d(TAG, "onDestroy");
        if (mDialog != null) {
            mDialog.dismiss();
            mDialog = null;
        }
        mHandler.removeMessages(EVENT_DUAL_SIM_MODE_CHANGED_COMPLETE);
        try {
            if (mTelephony != null) {
                mTelephony.unregisterForSimModeChange(mSwitchRadioStateMsg.getBinder());
            }
        } catch (RemoteException e) {
            Xlog.e(TAG, "mTelephony exception");
        }
    }

    private void initPreferenceState() {
        mEnableSimRadioPref.setChecked(getRadioState());
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference == mEnableSimRadioPref) {
            if (!mIsSIMRadioSwitching) {
                mIsSIMRadioSwitching = true;
                switchSimRadioState(mTargetSlot,
                        mEnableSimRadioPref.isChecked());
            } else {
                Xlog.d(TAG, "Click too fast it is switching and set the switch to previous state");
                mEnableSimRadioPref.setChecked(!mEnableSimRadioPref.isChecked());
            }
        } else if (preference == mRoamingHotlinePref) {
            Xlog.d(TAG, "start FreeService");
            Intent intent = new Intent(getActivity(), FreeService.class);
            if (mIsSim1Inserted && mIsSim2Inserted) {
                intent.putExtra(FreeService.SIM_INFO, FreeService.TWO_SIM);
            } else if (mIsSim1Inserted) {
                intent.putExtra(FreeService.SIM_INFO, FreeService.ONE_CDMA);
            } else if (mIsSim2Inserted) {
                intent.putExtra(FreeService.SIM_INFO, FreeService.ONE_GSM);
            } else {
                intent.putExtra(FreeService.SIM_INFO, FreeService.NO_SIM_ERROR);
            }
            getActivity().startService(intent);
        } else if (preference == mNetworkInfoPref) {
            Intent intent = new Intent();
            intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mTargetSlot);
            intent.setClassName("com.mediatek.op09.plugin",
                    "com.mediatek.settings.plugin.CurrentNetworkInfoStatus");
            startActivity(intent);
        } else if (preference == mManualNetworkPref) {
            Intent manualNetworkSettingIntent = new Intent("com.mediatek.OP09.MANUAL_NETWORK_SELECTION");
            manualNetworkSettingIntent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mTargetSlot);
            startActivity(manualNetworkSettingIntent);
        } else if (preference == mManualNetworkGsmPref) {
            if (mManualNetworkGsmPref.isChecked()) {
                Intent manualNetworkSettingIntent = new Intent("com.mediatek.OP09.MANUAL_NETWORK_SELECTION");
                manualNetworkSettingIntent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mTargetSlot);
                startActivity(manualNetworkSettingIntent);
            } else {
                selectNetworkAutomatic();
            }
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private void selectNetworkAutomatic() {
        Xlog.d(TAG, "select network automatically...");
        SlotSettingsFragment.setGsmAutoNetowrkSelection(getActivity(), true);
        if (mIsForeground) {
            showDialog(DLG_NETWORK_AUTO_SELECT);
        }
        Message msg = mHandler.obtainMessage(EVENT_AUTO_SELECT_DONE);
        /// M: to avoid start two same activity @{
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            mPhone.setNetworkSelectionModeAutomatic(msg);
        } else {
            CurrentNetworkInfoStatus.getPhoneBySlotId(mGeminiPhone, mTargetSlot).setNetworkSelectionModeAutomatic(msg);
        }
        /// @}
    }

    private void switchSimRadioState(int slot, boolean isChecked) {
        int dualSimMode = Settings.System.getInt(getActivity()
                .getContentResolver(), Settings.System.DUAL_SIM_MODE_SETTING,
                -1);
        Xlog.i(TAG, "The current dual sim mode is " + dualSimMode);
        int dualState = 0;
        boolean isRadioOn = false;
        switch (dualSimMode) {
        case ALL_RADIO_OFF:
            if (slot == PhoneConstants.GEMINI_SIM_1) {
                dualState = SIM_SLOT_1_RADIO_ON;
            } else if (slot == PhoneConstants.GEMINI_SIM_2) {
                dualState = SIM_SLOT_2_RADIO_ON;
            }
            Xlog.d(TAG, "Turning on only sim " + slot);
            isRadioOn = true;
            break;
        case SIM_SLOT_1_RADIO_ON:
            if (slot == PhoneConstants.GEMINI_SIM_1) {
                if (isChecked) {
                    Xlog.d(TAG,
                            "try to turn on slot 1 again since it is already on");
                    dualState = dualSimMode;
                    isRadioOn = true;
                } else {
                    dualState = ALL_RADIO_OFF;
                    isRadioOn = false;
                }
                Xlog.d(TAG, "Turning off sim " + slot
                        + " and all sim radio is off");
            } else if (slot == PhoneConstants.GEMINI_SIM_2) {
                if (mIsSim1Inserted) {
                    dualState = ALL_RADIO_ON;
                    Xlog.d(TAG, "sim 0 was radio on and now turning on sim "
                            + slot);
                } else {
                    dualState = SIM_SLOT_2_RADIO_ON;
                    Xlog.d(TAG, "Turning on only sim " + slot);
                }
                isRadioOn = true;
            }
            break;
        case SIM_SLOT_2_RADIO_ON:
            if (slot == PhoneConstants.GEMINI_SIM_2) {
                if (isChecked) {
                    Xlog.d(TAG,
                            "try to turn on slot 2 again since it is already on");
                    dualState = dualSimMode;
                    isRadioOn = true;
                } else {
                    dualState = ALL_RADIO_OFF;
                    isRadioOn = false;
                }
                Xlog.d(TAG, "Turning off sim " + slot
                        + " and all sim radio is off");
            } else if (slot == PhoneConstants.GEMINI_SIM_1) {
                if (mIsSim2Inserted) {
                    dualState = ALL_RADIO_ON;
                    Xlog.d(TAG, "sim 1 was radio on and now turning on sim "
                            + slot);
                } else {
                    dualState = SIM_SLOT_1_RADIO_ON;
                    Xlog.d(TAG, "Turning on only sim " + slot);
                }
                isRadioOn = true;
            }
            break;
        case ALL_RADIO_ON:
            if (!isChecked) {
                if (slot == PhoneConstants.GEMINI_SIM_1) {
                    dualState = SIM_SLOT_2_RADIO_ON;

                } else if (slot == PhoneConstants.GEMINI_SIM_2) {
                    dualState = SIM_SLOT_1_RADIO_ON;
                }
                Xlog.d(TAG, "Turning off only sim " + slot);
                isRadioOn = false;
            } else {
                dualState = dualSimMode;
                isRadioOn = true;
                Xlog.d(TAG, "try to turn on but actually they are all on");
            }
            break;
        default:
            Xlog.d(TAG, "Error not correct values");
            return;
        }
        int msgId = 0;
        if (isRadioOn) {
            msgId = R.string.gemini_sim_mode_progress_activating_message;
        } else {
            msgId = R.string.gemini_sim_mode_progress_deactivating_message;
        }
        showProgressDialg(msgId);
        Xlog.d(TAG, "dualState=" + dualState + " isRadioOn=" + isRadioOn);
        Settings.System.putInt(getActivity().getContentResolver(),
                Settings.System.DUAL_SIM_MODE_SETTING, dualState);
        Intent intent = new Intent(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        intent.putExtra(Intent.EXTRA_DUAL_SIM_MODE, dualState);
        getActivity().sendBroadcast(intent);
    }

    private void showProgressDialg(int msgId) {
        ProgressDialog dialog = new ProgressDialog(getActivity());
        dialog.setMessage(getString(msgId));
        dialog.setIndeterminate(true);
        mDialog = dialog;
        mDialog.setCancelable(false);
        mDialog.show();
    }

    private void showDialog(int dialogId) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        switch (dialogId) {
        case DLG_NETWORK_AUTO_SELECT:
            if (mDialog != null) {
                mDialog.dismiss();
            }
            mDialog = new ProgressDialog(getActivity());
            ((ProgressDialog)mDialog).setMessage(getActivity().getResources().getString(R.string.register_automatically));
            ((ProgressDialog)mDialog).setCancelable(false);
            ((ProgressDialog)mDialog).setIndeterminate(true);
            mDialog.show();
            break;

        default:
            break;
        }
    }

    private boolean getRadioState() {
        Xlog.d(TAG, "getRadioState()");
        if (mTelephonyEx == null) {
            mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager
                    .getService("phoneEx"));
        }
        boolean isRadioStateOn = true;
        try {
            if (mTelephonyEx != null) {
                isRadioStateOn = mTelephonyEx.isRadioOn(mTargetSlot);
            }
        } catch (RemoteException e) {
            Xlog.d(TAG, "exception happend unable to get Itelephony state");
        }
        Xlog.d(TAG, "isRadioStateOn = " + isRadioStateOn);
        return isRadioStateOn;
    }

    private boolean targetSlotRadioOn() {
        boolean isRadioOn = false;
        try {
            if (mTelephonyEx != null) {
                isRadioOn = mTelephonyEx.isRadioOn(mTargetSlot);
                Xlog.d(TAG, "Slot " + mTargetSlot + " is in radion state " + isRadioOn);
            }
        } catch (RemoteException e) {
            Xlog.e(TAG, "mTelephony exception");
            return false;
        }
        return isRadioOn;
    }

    private void updateScreen() {
        final boolean isAirplaneOn = Settings.System.getInt(getActivity().getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, -1) > 0;

        final boolean targetSlotRadioOn = targetSlotRadioOn();
        Xlog.d(TAG, "updateScreen: isAirplaneOn = " + isAirplaneOn + ", mTargetSlot = "
                + mTargetSlot + ", mIsSim1Inserted = " + mIsSim1Inserted + ", mIsSim2Inserted = "
                + mIsSim2Inserted + ", targetSlotRadioOn = " + targetSlotRadioOn);
        getPreferenceScreen().setEnabled(!isAirplaneOn);

        if (!isAirplaneOn) {
            if (mTargetSlot == PhoneConstants.GEMINI_SIM_1) {
                mEnableSimRadioPref.setEnabled(mIsSim1Inserted);
                mEnableSimRadioPref.setChecked(mIsSim1Inserted && targetSlotRadioOn);
                mNetworkInfoPref.setEnabled(mIsSim1Inserted && targetSlotRadioOn);
                mRoamingHotlinePref.setEnabled(mIsSim1Inserted && targetSlotRadioOn);
                mNotesPref.setEnabled(mIsSim1Inserted && targetSlotRadioOn);
            } else if (mTargetSlot == PhoneConstants.GEMINI_SIM_2) {
                mEnableSimRadioPref.setEnabled(mIsSim2Inserted);
                mEnableSimRadioPref.setChecked(mIsSim2Inserted && targetSlotRadioOn);
                mNetworkInfoPref.setEnabled(mIsSim2Inserted && targetSlotRadioOn);
            }

            if (targetSlotRadioOn) {
                // The target slot is slot1.
                if (getPreferenceScreen().findPreference(KEY_MANUAL_NETWORK_CDMA_SELECTION) != null) {
                    mManualNetworkPref.setEnabled(externalSlotInRoamingService());
                }
                // The target slot is slot2.
                if (getPreferenceScreen().findPreference(KEY_MANUAL_NETWORK_GSM_SELECTION) != null) {
                    mManualNetworkGsmPref.setEnabled(mIsSim2Inserted);
                    mManualNetworkGsmPref.setChecked(!isGsmAutoNetowrkSelection(getActivity()));
                }
            } else {
                if (getPreferenceScreen().findPreference(KEY_MANUAL_NETWORK_CDMA_SELECTION) != null) {
                    mManualNetworkPref.setEnabled(false);
                }
                if (getPreferenceScreen().findPreference(KEY_MANUAL_NETWORK_GSM_SELECTION) != null) {
                    mManualNetworkGsmPref.setEnabled(false);
                    mManualNetworkGsmPref.setChecked(!isGsmAutoNetowrkSelection(getActivity()));
                }
            }
        } else {
            mEnableSimRadioPref.setChecked(false);
            if (getPreferenceScreen().findPreference(KEY_MANUAL_NETWORK_GSM_SELECTION) != null) {
                mManualNetworkGsmPref.setEnabled(false);
                mManualNetworkGsmPref.setChecked(!isGsmAutoNetowrkSelection(getActivity()));
            }
        }
    }

    public static boolean externalSlotInRoamingService() {
        String currentNetworkNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC, NUMERIC_UNKNOWN);
        Xlog.d(TAG, "externalSlotInHomeService numeric is: " + currentNetworkNumeric);
        // / M: When current network is CHINA TELE or MACAO TELE, return true
        return !(NUMERIC_UNKNOWN.equals(currentNetworkNumeric)
                || NUMERIC_CHINA_TELE.equals(currentNetworkNumeric)
                || NUMERIC_CHINA_MACAO_TELE.equals(currentNetworkNumeric)
                || NUMERIC_NO_NETWORK.equals(currentNetworkNumeric));
    }

    private void displayNetworkSelectionFailed(Throwable ex) {
        String status = null;
        if ((ex != null && ex instanceof CommandException) &&
                ((CommandException)ex).getCommandError() == CommandException.Error.ILLEGAL_SIM_OR_ME) {
            status = OP09SettingsMiscExtImp.replaceSimBySlotInner(getString(R.string.not_allowed));
        } else {
            status = getResources().getString(R.string.connect_later);
        }
        Toast.makeText(getActivity(), status, Toast.LENGTH_LONG).show();
    }

    private void displayNetworkSelectionSucceeded() {
        String status = getResources().getString(R.string.registration_done);
        Toast.makeText(getActivity(), status, Toast.LENGTH_LONG).show();
    }

    public static boolean isGsmAutoNetowrkSelection(Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(PREFERENCES_NAME, Context.MODE_PRIVATE);
        return sharedPreferences.getBoolean(GSM_AUTO_NETWORK_SELECTION, true);
    }

    public static void setGsmAutoNetowrkSelection(Context context, boolean autoSelect) {
        SharedPreferences.Editor editor = context.getSharedPreferences(PREFERENCES_NAME,
                Context.MODE_PRIVATE).edit();
        editor.putBoolean(GSM_AUTO_NETWORK_SELECTION, autoSelect);
        editor.commit();
    }
}
