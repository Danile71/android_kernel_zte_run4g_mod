package com.mediatek.phone.plugin.callsetting;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.cdma.TtyIntent;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.List;

/**
 * M: General call settings that has no relationship with specified SIM
 */
public class GeneralCallSettings extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String LOG_TAG = "CallSettings";

    private static final String FEATURE_TTY = "TTY";
    private static final String FEATURE_DUAL_MIC = "DUAL_MIC";
    private static final String FEATURE_IP_DIAL = "IP_DIAL";
    private static final String FEATURE_3G_SWITCH = "3G_SWITCH";
    private static final String FEATURE_VT_VOICE_RECORDING = "VT_VOICE_RECORDING";
    private static final String FEATURE_VT_VIDEO_RECORDING = "VT_VIDEO_RECORDING";
    private static final String FEATURE_PHONE_VOICE_RECORDING = "PHONE_VOICE_RECORDING";

    private static final String ESURFING_DIAL_ACTIVITY_PACKAGE = "com.mediatek.op09.plugin";
    private static final String ESURFING_DIAL_ACTIVITY_NAME = "com.mediatek.calloption.plugin.ESurfingGuideSelectActivity";

    private static final String DUALMIC_MODE = "Enable_Dual_Mic_Setting";
    private static final String BUTTON_OTHERS_MINUTE_REMINDER_KEY = "minute_reminder_key";
    private static final String BUTTON_OTHERS_DUAL_MIC_KEY = "dual_mic_key";
    private static final String BUTTON_TTY_KEY = "button_tty_mode_key";
    private static final String BUTTON_ESURFING_DIALING = "button_esurfing_dialing";
    private static final String BUTTON_ESURFING_DIALING_GUIDE = "button_esurfing_dialing_guide";
    private static final int PREFERREDTTYMODE = Phone.TTY_MODE_OFF;

    public static final String PHONE_PREFERENCE_NAME = "com.android.phone_preferences";

    private CheckBoxPreference mButtonMr;
    private CheckBoxPreference mButtonDualMic;
    private ListPreference mButtonTty;
    private CheckBoxPreference mButtonEsurfing;
    private Preference mButtonEsurfingGuide;

    private IntentFilter mIntentFilter;
    private BroadcastReceiver mReceiver;

    private ITelephony mTelephony;
    private ITelephonyEx mTelephonyEx;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Xlog.d(LOG_TAG, "mReceiver [action = " + action + "]");
                if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)
                        || Intent.ACTION_DUAL_SIM_MODE_CHANGED.equals(action)) {
                    updateScreen();
                }
            }
        };
        mTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        addPreferencesFromResource(R.xml.call_general_settings);
        mButtonMr = (CheckBoxPreference) findPreference(BUTTON_OTHERS_MINUTE_REMINDER_KEY);
        mButtonMr.setChecked(getMrState());
        mButtonDualMic = (CheckBoxPreference) findPreference(BUTTON_OTHERS_DUAL_MIC_KEY);
        if (!FeatureOption.MTK_DUAL_MIC_SUPPORT) {
            getPreferenceScreen().removePreference(mButtonDualMic);
        }

        if (mButtonMr != null) {
            mButtonMr.setOnPreferenceChangeListener(this);
        }

        if (mButtonDualMic != null) {
            mButtonDualMic.setOnPreferenceChangeListener(this);
        }

        mButtonTty = (ListPreference) findPreference(BUTTON_TTY_KEY);
        if (mButtonTty != null) {
            if (isSupportFeature(FEATURE_TTY)) {
                mButtonTty.setOnPreferenceChangeListener(this);
            } else {
                getPreferenceScreen().removePreference(mButtonTty);
                mButtonTty = null;
            }
        }

        mButtonEsurfing = (CheckBoxPreference) findPreference(BUTTON_ESURFING_DIALING);
        mButtonEsurfingGuide = (Preference) findPreference(BUTTON_ESURFING_DIALING_GUIDE);
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().registerReceiver(mReceiver, mIntentFilter);
        updateScreen();
    }

    @Override
    public void onPause() {
        super.onPause();
        getActivity().unregisterReceiver(mReceiver);
    }

    private void updateScreen() {
        if (mButtonTty != null) {
            int settingsTtyMode = Settings.Secure.getInt(
                    getActivity().getContentResolver(), Settings.Secure.PREFERRED_TTY_MODE,
                    Phone.TTY_MODE_OFF);
            mButtonTty.setValue(Integer.toString(settingsTtyMode));
            updatePreferredTtyModeSummary(settingsTtyMode);
        }
        if (mButtonEsurfing != null) {
            try {
                mButtonEsurfing.setChecked(Settings.System.getInt(getActivity().getContentResolver(),
                        Settings.System.ESURFING_DIALING) == 1 ? true : false);
            } catch (SettingNotFoundException e) {
                Xlog.e(LOG_TAG, "onResume query settings exception");
            }
        }

        boolean isAirplaneOn = Settings.System.getInt(
                getActivity().getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1) > 0;
        boolean hasRadioOnSim = false;
        if (!isAirplaneOn) {
            List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(getActivity());
            int simSize = simList.size();
            for (int index = 0; index < simSize; index++) {
                int slot = simList.get(index).mSimSlotId;
                if (slotRadioOn(slot)) {
                    hasRadioOnSim = true;
                    break;
                }
            }
        }
        mButtonEsurfingGuide.setEnabled(!isAirplaneOn && hasRadioOnSim);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference.equals(mButtonEsurfingGuide)) {
            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_MAIN);
            intent.setClassName(ESURFING_DIAL_ACTIVITY_PACKAGE, ESURFING_DIAL_ACTIVITY_NAME);
            startActivity(intent);
        } else if (preference.equals(mButtonEsurfing)) {
            Settings.System.putInt(getActivity().getContentResolver(), 
                    Settings.System.ESURFING_DIALING, mButtonEsurfing.isChecked() ? 1 : 0);
        } else if (preference.equals(mButtonMr)) {
            Xlog.d(LOG_TAG, "onPreferenceTreeClick mButtonMr.isChecked()=" + mButtonMr.isChecked());
            mButtonMr.setSummary(mButtonMr.isChecked() ? getString(R.string.minutereminder_turnon)
                    : getString(R.string.minutereminder_turnoff));
            setMrState(mButtonMr.isChecked());
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
       if (preference.equals(mButtonDualMic)) {
            Xlog.d(LOG_TAG, "onPreferenceChange mButtonDualMic.isChecked()=" + mButtonDualMic.isChecked());
            setDualMicMode(mButtonDualMic.isChecked() ? "0" : "1");
        } else if (preference.equals(mButtonTty)) {
            handleTTYChange(preference, objValue);
        }
        return true;
    }

    private void setDualMicMode(String dualMic) {
        Context context = getActivity();
        if (context != null) {
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            audioManager.setParameters(DUALMIC_MODE + "=" + dualMic);
        } else {
            Xlog.e(LOG_TAG, "setDualMicMode, dualMic set error, dualMic=" + dualMic);
        }
    }

    private void handleTTYChange(Preference preference, Object objValue) {
        int buttonTtyMode = Integer.valueOf((String) objValue).intValue();
        int settingsTtyMode = android.provider.Settings.Secure.getInt(getActivity().getContentResolver(),
                android.provider.Settings.Secure.PREFERRED_TTY_MODE, PREFERREDTTYMODE);
        Xlog.d(LOG_TAG, "handleTTYChange: requesting set TTY mode enable (TTY) to" + Integer.toString(buttonTtyMode));

        if (buttonTtyMode != settingsTtyMode) {
            switch (buttonTtyMode) {
            case Phone.TTY_MODE_OFF:
            case Phone.TTY_MODE_FULL:
            case Phone.TTY_MODE_HCO:
            case Phone.TTY_MODE_VCO:
                android.provider.Settings.Secure.putInt(getActivity().getContentResolver(),
                        android.provider.Settings.Secure.PREFERRED_TTY_MODE, buttonTtyMode);
                break;
            default:
                buttonTtyMode = Phone.TTY_MODE_OFF;
            }

            mButtonTty.setValue(Integer.toString(buttonTtyMode));
            updatePreferredTtyModeSummary(buttonTtyMode);
            Intent ttyModeChanged = new Intent(TtyIntent.TTY_PREFERRED_MODE_CHANGE_ACTION);
            ttyModeChanged.putExtra(TtyIntent.TTY_PREFFERED_MODE, buttonTtyMode);
            getActivity().sendBroadcast(ttyModeChanged);
        }
    }

    private void updatePreferredTtyModeSummary(int ttyMode) {
        String[] txts = getResources().getStringArray(R.array.tty_mode_entries);
        switch (ttyMode) {
        case Phone.TTY_MODE_OFF:
        case Phone.TTY_MODE_HCO:
        case Phone.TTY_MODE_VCO:
        case Phone.TTY_MODE_FULL:
            mButtonTty.setSummary(txts[ttyMode]);
            break;
        default:
            mButtonTty.setEnabled(false);
            mButtonTty.setSummary(txts[Phone.TTY_MODE_OFF]);
        }
    }

    public static boolean isSupportFeature(String feature) {
        if (feature == null) {
            return false;
        }
        if (feature.equals(FEATURE_TTY)) {
            return FeatureOption.MTK_TTY_SUPPORT;
        } else if (feature.equals(FEATURE_DUAL_MIC)) {
            return FeatureOption.MTK_DUAL_MIC_SUPPORT;
        } else if (feature.equals(FEATURE_IP_DIAL)) {
            return true;
        } else if (feature.equals(FEATURE_3G_SWITCH)) {
            return FeatureOption.MTK_GEMINI_3G_SWITCH;
        } else if (feature.equals(FEATURE_VT_VOICE_RECORDING)) {
            return true;
        } else if (feature.equals(FEATURE_VT_VIDEO_RECORDING)) {
            return true;
        } else if (feature.equals(FEATURE_PHONE_VOICE_RECORDING)) {
            return FeatureOption.MTK_PHONE_VOICE_RECORDING;
        }
        return false;
    }

    private boolean slotRadioOn(int slotId) {
        boolean isRadioOn = false;
        try {
            if (mTelephonyEx != null) {
                isRadioOn = mTelephonyEx.isRadioOn(slotId);
                Xlog.d(LOG_TAG, "Slot " + slotId + " is in radion state " + isRadioOn);
            }
        } catch (RemoteException e) {
            Xlog.w(LOG_TAG, "mTelephony exception");
        }
        return isRadioOn;
    }
    /// M: getMrState from phone's shared preferences
    private static boolean getMrState() {
        return PhoneFactory.getDefaultPhone().getContext().getSharedPreferences(PHONE_PREFERENCE_NAME, Context.MODE_PRIVATE)
                .getBoolean(BUTTON_OTHERS_MINUTE_REMINDER_KEY, false);
    }

    // M: setMrState to phone's shared preferences
    private static void setMrState(boolean newState) {
        PhoneFactory.getDefaultPhone().getContext().getSharedPreferences(PHONE_PREFERENCE_NAME, Context.MODE_PRIVATE).edit()
                .putBoolean(BUTTON_OTHERS_MINUTE_REMINDER_KEY, newState).commit();
    }
}
