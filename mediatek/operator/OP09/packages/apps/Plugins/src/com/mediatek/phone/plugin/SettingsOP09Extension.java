/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.phone.plugin;

import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op09.plugin.R;
import com.mediatek.phone.ext.SettingsExtension;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;


public class SettingsOP09Extension extends SettingsExtension {

    private static final String TAG = "OP09PhonePlugin";

    private static final String KEY_DATA_CONN = "data_connection_setting";
    private static final String KEY_CT_APN = "pref_ct_apn_key";
    private static final String KEY_MANUAL_NETWORK_SELECTION = "manual_network_selection";

    private static final String ACTION_MAIN = "android.intent.action.MAIN";
    private static final String CT_PLUGIN_PACKAGE = "com.mediatek.op09.plugin";
    private static final String CT_PLUGIN_CLASS_MULTI_SIM = "com.mediatek.phone.plugin.MultipleSimActivity";
    private static final String SETTINGS_PACKAGE = "com.android.settings";
    private static final String APNSETTINGS_CLASS = "com.android.settings.ApnSettings";
    private static final String CT_MANUAL_CLASS = "com.mediatek.settings.plugin.ManualNetworkSelection";

    private Context mOp09Context;

    public SettingsOP09Extension(Context context) {
        mOp09Context = context;
    }

    public boolean needCustomizeNetworkSelection() {
        Xlog.e(TAG, "needCustomizeNetworkSelection()");
        return true;
    }

    @Override
    public void loadManualNetworkSelectionPreference(PreferenceActivity prefActivity,
            PreferenceScreen preferenceScreen) {
        Xlog.d(TAG, "loadManualNetworkSelectionPreference() OP09");
        NetworkSelectionOptions networkSelectionOptions = new NetworkSelectionOptions(mOp09Context, prefActivity,
                preferenceScreen);
    }

    public String replaceSimToSimUim(String simString) {
        if (simString.contains("SIM")) {
            simString = simString.replaceAll("SIM", "UIM/SIM");
        }
        Xlog.d(TAG, "op09 replace string: " + simString);
        return simString;
    }

    public String replaceSimToUim(String simString) {
        if (simString.contains("SIM")) {
            simString = simString.replaceAll("SIM", "UIM");
        }

        if (simString.contains("sim")) {
            simString = simString.replaceAll("sim", "uim");
        }
        Xlog.d(TAG, "op09 replace string: " + simString);
        return simString;
    }

    public String replaceSim1ToUim(String simString) {
        if (simString.contains("SIM1")) {
            simString = simString.replaceAll("SIM1", "UIM");
        }

        if (simString.contains("sim1")) {
            simString = simString.replaceAll("sim1", "uim");
        }
        Xlog.d(TAG, "op09 replace string: " + simString);
        return simString;
    }

    public String replaceSimToCard(String simString) {
        if (simString.contains("SIM")) {
            simString = simString.replaceAll("SIM", "card");
        }
        Xlog.d(TAG, "op09 replace string: " + simString);
        return simString;
    }

    /**
     * 
     * @param simString
     * @param slotId
     *            if slotId = 0 , replace SIM to UIM.
     * @return
     */
    public String replaceSimBySlot(String simString, int slotId) {
        if (slotId == 0) {
            return replaceSimToUim(simString);
        }
        Xlog.d(TAG, "op09 replace string: " + simString);
        return simString;
    }

    public void switchPref(Preference manuSelect, Preference autoSelect) {
        manuSelect.setOrder(autoSelect.getOrder() + 1);
        manuSelect.setTitle(mOp09Context.getString(R.string.manual_select_title));
        autoSelect.setTitle(mOp09Context.getString(R.string.select_automatically));
    }
    
    public String getManualSelectDialogMsg(String defaultMsg) {
        return mOp09Context.getString(R.string.manual_select_dialog_msg);
    }

    @Override
    public boolean shouldPublicRemoteBinder() {
        return true;
    }

    @Override
    public boolean reloadPreference(PreferenceActivity prefActivity,
            PreferenceScreen preferenceScreen,
            Preference buttonPreferredNetworkModeEx,
            Preference preference3GSwitch, Preference plmnPreference) {
        Xlog.d(TAG, "reloadPreference() OP09");
        NetworkSelectionOptions networkSelectionOptions = new NetworkSelectionOptions(
                mOp09Context, prefActivity, preferenceScreen);
        if (buttonPreferredNetworkModeEx != null) {
            preferenceScreen.removePreference(buttonPreferredNetworkModeEx);
        }
        if (preference3GSwitch != null) {
            preferenceScreen.removePreference(preference3GSwitch);
        }
        if (plmnPreference != null) {
            preferenceScreen.removePreference(plmnPreference);
        }

        // CT spec requires to remove Mobile data setting entry in MobileNetworkSetting
        Preference dataPreference = preferenceScreen.findPreference(KEY_DATA_CONN);
        if (dataPreference != null) {
            preferenceScreen.removePreference(dataPreference);
        }
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (null != (PreferenceScreen) preferenceScreen.findPreference(KEY_CT_APN) &&
            preference == (PreferenceScreen) preferenceScreen.findPreference(KEY_CT_APN)) {
            preference.getContext().startActivity(getPreferenceClickIntent(preference, SETTINGS_PACKAGE, APNSETTINGS_CLASS));
            return true;
        } else if (null != (PreferenceScreen) preferenceScreen.findPreference(KEY_MANUAL_NETWORK_SELECTION)
                && preference == (PreferenceScreen) preferenceScreen.findPreference(KEY_MANUAL_NETWORK_SELECTION)) {
            preference.getContext().startActivity(getPreferenceClickIntent(preference, CT_PLUGIN_PACKAGE, CT_MANUAL_CLASS));
            return true;
        }
        return false;
    }

    private Intent getPreferenceClickIntent(Preference preference, String packageName, String className) {
        Intent it = new Intent();
        it.setAction(ACTION_MAIN);
        it.setClassName(CT_PLUGIN_PACKAGE, CT_PLUGIN_CLASS_MULTI_SIM);
        it.putExtra(MultipleSimActivity.INIT_TITLE_NAME_STR, preference.getTitle());
        it.putExtra(MultipleSimActivity.TARGET_PACKAGE, packageName);
        it.putExtra(MultipleSimActivity.TARGET_CLASS, className);
        return it;
    }

    private boolean isRadioOn(GeminiPhone geminiPhone, int slot) {
        return geminiPhone.isRadioOnGemini(slot);
    }

    ///M: For CT feature, when sim1 registe GSM network in IR ,
    ///GsmUmtsCallForwardOptions only show 3 items(common is 4 items) @{
    public void removeCFNRc(PreferenceScreen prefSet,
            Preference preference, int slotId) {
        TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
        if (FeatureOption.EVDO_DT_SUPPORT
                && slotId == PhoneConstants.GEMINI_SIM_1
                && (telephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_1) == PhoneConstants.PHONE_TYPE_GSM)) {
            prefSet.removePreference(preference);
        }
    }
    /// @}
}
