package com.mediatek.settings.plugin;

import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.telephony.TelephonyManager;
import android.text.InputFilter;

import com.mediatek.settings.ext.DefaultSimManagementExt;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class OP09SimManagementExtImp extends DefaultSimManagementExt {

    private static final String TAG = "SimManagementExtImp";

    private static final int SINGLE_SIM_CARD = 1;
    private static final int DOUBLE_SIM_CARD = 2;

    private static final String KEY_GPRS_SIM_SETTING = "gprs_sim_setting";
    private static final String KEY_ROAMING_SETTING = "roaming_settings";

    private static final int SIM_NUMBER_MAX_LEN_CDMA = 15;
    private static final int SIM_NUMBER_MAX_LEN_GSM = 19;

    private static final InputFilter[] CDMA_FILTERS = 
        new InputFilter[] {new InputFilter.LengthFilter(SIM_NUMBER_MAX_LEN_CDMA)};
    private static final InputFilter[] GSM_FILTERS = 
        new InputFilter[] {new InputFilter.LengthFilter(SIM_NUMBER_MAX_LEN_GSM)};

    /**
     * Dual sim indicator new design, remove sim color editor preference
     */
    @Override
    public void customizeSimColorEditPreference(PreferenceFragment pref,
            String key) {
        pref.getPreferenceScreen().removePreference(pref.findPreference(key));
    }

    /**
     * CT spec: don't show sms item
     */
    @Override
    public void customizeSmsChoiceArray(List<String> smsList) {
        smsList.clear();
    }

    /**
     * CT spec: don't show Voice item
     */
    @Override
    public void customizeVoiceChoiceArray(List<String> voiceList,
            boolean voipAvailable) {
        voiceList.clear();
    }

    @Override
    public void updateDefaultSettingsItem(PreferenceGroup prefGroup) {
        Preference pref = prefGroup.findPreference(KEY_GPRS_SIM_SETTING);
        if (pref != null) {
            prefGroup.removePreference(pref);
        }
    }

    @Override
    public void updateSimManagementPref(PreferenceGroup parent) {
        super.updateSimManagementPref(parent);
        Preference pref = parent.findPreference(KEY_ROAMING_SETTING);
        if (null != pref) {
            parent.removePreference(pref);
        }
    }

    @Override
    public boolean enableSwitchForSimInfoPref() {
        Xlog.d(TAG, "enableSwitchForSimInfoPref return false");
        return false;
    }

     @Override
    public void updateSimNumberMaxLength(EditTextPreference editTextPreference, int slotId) {
        if (editTextPreference != null) {
            TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
            if (telephonyManagerEx != null && telephonyManagerEx.getPhoneType(slotId) == TelephonyManager.PHONE_TYPE_CDMA) {
                editTextPreference.getEditText().setFilters(CDMA_FILTERS);
            } else if (telephonyManagerEx != null && 
                    telephonyManagerEx.getPhoneType(slotId) == TelephonyManager.PHONE_TYPE_GSM) {
                editTextPreference.getEditText().setFilters(GSM_FILTERS);
            }
        }
    }
}
