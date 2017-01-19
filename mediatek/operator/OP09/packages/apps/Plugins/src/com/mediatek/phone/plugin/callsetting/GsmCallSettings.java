package com.mediatek.phone.plugin.callsetting;

import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.op09.plugin.R;

/**
 *
 * Description:
 * <p>
 * Customization for CT, GSM call setting
 * <p>
 *
 */
public class GsmCallSettings extends PreferenceFragment {

    private static final String TAG = "GsmCallSettings";

    private static final String KEY_VOICEMALL_PREFIX = "button_voicemail_key";
    private static final String KEY_IP_PREFIX = "button_ip_prefix_key";
    private static final String KEY_FORWARD_PREFIX = "button_cf_expand_key";
    private static final String KEY_BARRING_PREFIX = "button_cb_expand_key";
    private static final String KEY_FDN_PREFIX = "button_fdn_key";
    private static final String KEY_MORE_PREFIX = "button_more_expand_key";

    private int mTargetSlot;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.gsm_settings);
        mTargetSlot = CallSettings.getSimSlotIdByTag(getTag());
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_MAIN);
        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mTargetSlot);
        if (preference.getKey().equals(KEY_VOICEMALL_PREFIX)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.VoiceMailSetting");
        } else if (preference.getKey().equals(KEY_IP_PREFIX)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.IpPrefixPreference");
        } else if (preference.getKey().equals(KEY_FORWARD_PREFIX)) {
            intent.setClassName("com.android.phone", "com.android.phone.GsmUmtsCallForwardOptions");
        } else if (preference.getKey().equals(KEY_BARRING_PREFIX)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.CallBarring");
        } else if (preference.getKey().equals(KEY_FDN_PREFIX)) {
            intent.setClassName("com.android.phone", "com.mediatek.settings.FdnSetting2");
        } else if (preference.getKey().equals(KEY_MORE_PREFIX)) {
            intent.setClassName("com.android.phone", "com.android.phone.GsmUmtsAdditionalCallOptions");
        }
        startActivity(intent);
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }
}
