/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.misc;

import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.widget.Toast;

import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class MiscConfig extends PreferenceActivity {
    private static final String TAG = "EM/MiscConfig";
    private static final String KEY_MISC_CONFIG = Settings.Global.TELEPHONY_MISC_FEATURE_CONFIG;
    private static final int INDEX_2G_ONLY_ROAMING = 0;

    private CheckBoxPreference mPreferences[];
    private int mConfig = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.misc);

        String[] features = getResources().getStringArray(R.array.misc_features);
        mPreferences = new CheckBoxPreference[features.length];
        for (int i = 0; i < features.length; i++) {
            mPreferences[i] = new CheckBoxPreference(this);
            mPreferences[i].setSummary(features[i]);
            mPreferences[i].setPersistent(false);
            getPreferenceScreen().addPreference(mPreferences[i]);
            Elog.d(TAG, "WorldPhone: " + PhoneFactory.isWorldPhone());
            if (i == INDEX_2G_ONLY_ROAMING && PhoneFactory.isWorldPhone()) {
                mPreferences[i].setEnabled(false);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mConfig = Settings.Global.getInt(getContentResolver(), KEY_MISC_CONFIG, 0);
        Elog.d(TAG, "Get " + KEY_MISC_CONFIG + " = " + mConfig);
        for (int i = 0; i < mPreferences.length; i++) {
            mPreferences[i].setChecked((mConfig & (1 << i)) != 0);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        Elog.d(TAG, "Enter onPreferenceTreeClick()");
        for (int i = 0; i < mPreferences.length; i++) {
            if (mPreferences[i].equals(preference)) {
                if (mPreferences[i].isChecked()) {
                    mConfig |= (1 << i);
                } else {
                    mConfig &= ~(1 << i);
                }
                Elog.d(TAG, "Set " + KEY_MISC_CONFIG + " = " + mConfig);
                Settings.Global.putInt(getContentResolver(), KEY_MISC_CONFIG, mConfig);
                return true;
            }
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }
}
