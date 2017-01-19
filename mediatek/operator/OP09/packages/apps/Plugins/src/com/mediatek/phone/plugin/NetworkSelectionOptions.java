/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.phone.plugin;

import android.content.Context;
import android.content.Intent;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.mediatek.op09.plugin.R;

/**
 * List of Network-specific settings screens.
 */
public class NetworkSelectionOptions {
    private static final String LOG_TAG = "NetworkSelectionOptions";

    private static final String CT_PAN_KEY = "pref_ct_apn_key";
    private static final String MANUAL_NETWORK_SELECTION_KEY = "manual_network_selection";
    
    private static final String APN_SETTINGS_PACKAGE = "com.android.settings";
    private static final String APN_SETTINGS_CLASS = "com.android.settings.ApnSettings";
    private static final String NETWORK_SELECTION_PACKAGE = "com.android.phone";
    private static final String NETWORK_SELECTION_CLASS = "com.mediatek.settings.plugin.ManualNetworkSelection";
    
    private PreferenceActivity mPrefActivity;
    private PreferenceScreen mPrefScreen;
    private Context mOP09Context;

    public NetworkSelectionOptions(Context op09Context, PreferenceActivity prefActivity,
            PreferenceScreen prefScreen) {
        mOP09Context = op09Context;
        mPrefActivity = prefActivity;
        mPrefScreen = prefScreen;
        create();
    }

    protected void create() {
        PreferenceScreen prefAPN = new PreferenceScreen(mPrefActivity, null);
        prefAPN.setKey(CT_PAN_KEY);
        prefAPN.setTitle(mOP09Context.getResources().getText(R.string.apn_settings));
        prefAPN.setPersistent(false);
        Intent targetIntent = new Intent();
        targetIntent.setAction(Intent.ACTION_MAIN);
        targetIntent.setClassName(APN_SETTINGS_PACKAGE, APN_SETTINGS_CLASS);
        prefAPN.setIntent(targetIntent);
        mPrefScreen.addPreference(prefAPN);
    }
}
