package com.mediatek.phone.plugin.callsetting;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

/**
 * Description:
 * <p>
 * Customization for CT,CDMA call setting
 * <p>
 * <p>
 * 1. voice mail setting 2. ip prefix setting 3. other setting 4. call forward setting
 * <p>
 */
public class CdmaCallSettings extends PreferenceFragment {

    private static final String TAG = "CallSettings";

    private static final String KEY_VOICEMAIL = "button_voicemail_key";
    private static final String KEY_IP_PREFIX = "button_ip_prefix_key";
    private static final String KEY_ADDITIONAL = "button_more_expand_key";
    private static final String KEY_CALL_FORWARD = "button_cf_expand_key";

    private TelephonyManager mTelephonyManager;

    private int mTargetSlot;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mTelephonyManager = (TelephonyManager) getActivity().getSystemService(Context.TELEPHONY_SERVICE);
        addPreferencesFromResource(R.xml.cdma_settings);
        mTargetSlot = CallSettings.getSimSlotIdByTag(getTag());
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_MAIN);
        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mTargetSlot);
        if (preference.getKey().equals(KEY_VOICEMAIL)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.VoiceMailSetting");
        } else if (preference.getKey().equals(KEY_IP_PREFIX)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.IpPrefixPreference");
        } else if (preference.getKey().equals(KEY_ADDITIONAL)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.CdmaAdditionalCallOptions");
        } else if (preference.getKey().equals(KEY_CALL_FORWARD)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.CdmaCallForwardOptions");
        }
        Xlog.d(TAG, "onPreferenceTreeClick preference.getKey()=" + preference.getKey());
        startActivity(intent);
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }
}
