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

package com.mediatek.voicecommand.ui.settings;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.widget.Toast;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.business.VoiceWakeup;
import com.mediatek.voicecommand.cfg.VoiceWakeupInfo;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.xlog.Xlog;

public class VoiceUiSettings extends PreferenceActivity implements
        OnSharedPreferenceChangeListener {

    private static final String TAG = "VoiceUiSettings";

    public static final String KEY_VOICE_UI_FOR_PLAY_COMMAND = "voice_ui_key";
    public static final String KEY_VOICE_WAKEUP_FOR_PLAY_COMMAND = "voice_wakeup_key";
    private static final String KEY_VOICE_APP_COMMAND_CATEGORY = "voice_app_command";
    private static final String KEY_VOICE_APP_WAKEUP_CATEGORY = "voice_app_wakeup";
    private static final String KEY_VOICE_UI_LANGUAGE = "language_settings";
    private static final String KEY_VOICE_UI_FOR_APP_CATEGORY = "voice_ui_app";
    private static final String KEY_VOICE_APP_WAKEUP = "voice_app_wakeup_preference";

//    private static final String VOICE_CONTROL_ENABLED = "voice_control_enabled";
//    // default to English
//    private static final String VOICE_UI_SUPPORT_LANGUAGES = "voice_ui_support_languages";

    private String[] mSupportLangs;
    private String[] mSupportWeakups;

    // private Switch mEnabledSwitch;
    private PreferenceCategory mVoiceUiAppCategory;
    private PreferenceCategory mVoiceWakeAppCategory;
    private SwitchPreference mWakeupPref;
    private Preference mLanguagePref;

    // data get from framework
    private List<String> mFeatureList;
    private List<SwitchPreference> mFeaturePrefs = new ArrayList<SwitchPreference>();
    private ConfigurationManager mVoiceConfigMgr;
    private boolean isSystemLanguage = false;

    private int mWakeupMode = 1;
    // wakeup id 0: camera, 1: contacts, 2: googleNow
    public static final String KEY_COMMAND_ID = "command_id";
    // wakeup package name
    public static final String KEY_COMMAND_PACKAGENAME = "command_packagename";
    // wakeup class name
    public static final String KEY_COMMAND_CLASSNAME = "command_classname";
    // wakeup keyword
    public static final String KEY_COMMAND_KEYWORD = "command_keyword";
    // launched app component string
    public static final String KEY_COMMAND_VALUE = "command_value";
    // 0: record, 1: modify
    public static final String KEY_COMMAND_TYPE = "command_type";
    public static final int COMMAND_TYPE_RECORD = 0;
    // 0: VoiceUnlock, 1: anyone, 2: command
    public static final String KEY_COMMAND_MODE = "command_mode";
    // wakeup command list in command mode
    public static final String KEY_COMMAND_LISTS = "command_lists";
    
    private boolean exceptedWakeupStatus;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.voice_ui_settings);
        mVoiceUiAppCategory = (PreferenceCategory) findPreference(KEY_VOICE_APP_COMMAND_CATEGORY);
        mVoiceWakeAppCategory = (PreferenceCategory) findPreference(KEY_VOICE_APP_WAKEUP_CATEGORY);

        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        if (mVoiceConfigMgr == null) {
            Xlog.e(TAG, "ConfigurationManager is null");
        }

        isSystemLanguage = mVoiceConfigMgr.getIsSystemLanguage();
        Xlog.d(TAG, "VoiceUiSettings isSytemLanguage is " + isSystemLanguage);

        mLanguagePref = findPreference(KEY_VOICE_UI_LANGUAGE);
        if (isSystemLanguage) {
            mVoiceUiAppCategory.removePreference(mLanguagePref);
        } else {
            mSupportLangs = mVoiceConfigMgr.getLanguageList();
        }

        getActionBar().setTitle(R.string.voice_ui_title);
        mFeatureList = Arrays.asList(mVoiceConfigMgr.getFeatureNameList());

        createPreferenceHierarchy(mFeatureList);
        createWakeupPreference();

        getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM);

        mVoiceUiAppCategory.setEnabled(true);
        mVoiceWakeAppCategory.setEnabled(true);

        if (!FeatureOption.MTK_VOICE_UI_SUPPORT) {
            getPreferenceScreen().removePreference(mVoiceUiAppCategory);
        }
        if (!FeatureOption.MTK_VOW_SUPPORT) {
            getPreferenceScreen().removePreference(mVoiceWakeAppCategory);
        }
    }

    private void createPreferenceHierarchy(List<String> featureList) {

        for (int i = 0; i < featureList.size(); i++) {
            SwitchPreference appPref = new SwitchPreference(this) {
                @Override
                protected void onClick() {
                    // TODO Auto-generated method stub
                    // super.onClick();
                }
            };
            if (appPref != null) {
                String featureName = featureList.get(i);
                int processID = mVoiceConfigMgr.getProcessID(featureName);
                int TitleId = VoiceUiResUtil
                        .getProcessTitleResourceId(processID);
                if (TitleId != 0) {
                    appPref.setTitle(TitleId);
                } else {
                    appPref.setTitle("error");
                }
                // appPref.setSummary(fetchSummary(featureName));
                int IconId = VoiceUiResUtil.getIconId(processID);
                if (IconId != 0) {
                    appPref.setIcon(IconId);
                }
                appPref.setPersistent(true);
                appPref.setKey(featureName);
                appPref.setOrder(i + 1);
                mVoiceUiAppCategory.addPreference(appPref);
                mFeaturePrefs.add(appPref);
            }
        }
    }

    private void createWakeupPreference() {
        mWakeupPref = new SwitchPreference(this) {
            @Override
            protected void onClick() {
                // TODO Auto-generated method stub
                // super.onClick();
            }
        };
        if (mWakeupPref != null) {
            mWakeupPref.setTitle(fetchWakeupTitle());
            mWakeupPref.setPersistent(true);
            mWakeupPref.setKey(KEY_VOICE_APP_WAKEUP);
            mVoiceWakeAppCategory.addPreference(mWakeupPref);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Xlog.d(TAG, "onResume called");

        // boolean isEnable = mVoiceConfigMgr.getVoiceControlEnable();
        int[] featureEnableArray = mVoiceConfigMgr.getFeatureEnableArray();
        if (featureEnableArray != null) {
            Xlog.d(TAG,
                    "feature enabled array = " + featureEnableArray.toString());
        }

        // update summary
        for (int i = 0; i < mFeaturePrefs.size(); i++) {
            SwitchPreference appPref = mFeaturePrefs.get(i);

            boolean isEnabled = mVoiceConfigMgr.isProcessEnable(appPref
                    .getKey());
            appPref.setChecked(isEnabled);

        }

        if (!isSystemLanguage) {
            int languageIndex = mVoiceConfigMgr.getCurrentLanguage();
            Xlog.d(TAG, "Default language is " + mSupportLangs[languageIndex]);
            mLanguagePref.setSummary(mSupportLangs[languageIndex]);
        }

        mWakeupMode = mVoiceConfigMgr.getWakeupMode();
        if (mWakeupMode == VoiceCommandListener.VOW_MODE_WAKEUP_ANYONE) {
            mWakeupPref.setSummary(fetchAnyoneSummary());
        } else if (mWakeupMode == VoiceCommandListener.VOW_MODE_WAKEUP_COMMAND) {
            mWakeupPref.setSummary(fetchCommandSummary());
        }

        int cmdStatus = mVoiceConfigMgr.getWakeupCmdStatus();
        int enabledStatus = VoiceWakeup.getWakeupEnableStatus(cmdStatus);
        boolean isEnabled = (enabledStatus == 1) ? true : false;
        Xlog.d(TAG, "onResume cmdStatus: " + cmdStatus
                + ", enabledStatus: " + enabledStatus + ", isEnable: "
                + isEnabled);
        mWakeupPref.setChecked(isEnabled);
        exceptedWakeupStatus = isEnabled;

        PreferenceManager.getDefaultSharedPreferences(this)
                .registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        PreferenceManager.getDefaultSharedPreferences(this)
                .unregisterOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
            String key) {
        // TODO Auto-generated method stub
        if (mFeatureList.contains(key)) {
            final String processName = key;
            boolean enable = sharedPreferences.getBoolean(key, false);
            Xlog.d(TAG, "onSharedPreferenceChanged set enable " + processName
                    + " " + enable);
            mVoiceConfigMgr.updateFeatureEnable(processName, enable);
        } else if (KEY_VOICE_APP_WAKEUP.equals(key)) {
            boolean enable = sharedPreferences.getBoolean(key, false);
            Xlog.d(TAG,
                    "onSharedPreferenceChanged set wakeup by command enable "
                            + enable);
            // Handle the click event when set switch enable.
            if (mWakeupMode == VoiceCommandListener.VOW_MODE_WAKEUP_ANYONE) {
                if (enable) {
                    wakeupAnyoneEnableEvent();
                } else {
                    handleWakeupDisableEvent();
                }
            } else if (mWakeupMode == VoiceCommandListener.VOW_MODE_WAKEUP_COMMAND) {
                if (enable) {
                    if (exceptedWakeupStatus) {
                        return;
                    }
                    exceptedWakeupStatus = false;
                    mWakeupPref.setChecked(false);
                    wakeupCommandEnableEvent();
                } else {
                    if (!exceptedWakeupStatus) {
                        return;
                    }
                    exceptedWakeupStatus = false;
                    handleWakeupDisableEvent();
                }
            }
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {

        if (KEY_VOICE_UI_LANGUAGE.equals(preference.getKey())) {
            Intent intent = new Intent(
                    "com.mediatek.voicecommand.VOICE_CONTROL_LANGUAGE");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            return true;

        } else if (mFeatureList.contains(preference.getKey())) {
            final String processName = preference.getKey();
            Intent intent = new Intent(
                    "com.mediatek.voicecommand.VOICE_UI_COMMAND_PLAY");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(KEY_VOICE_UI_FOR_PLAY_COMMAND, processName);
            startActivity(intent);
            // mVoiceConfigMgr.updateFeatureEnable(processName, enable);
            return true;

        } else if (KEY_VOICE_APP_WAKEUP.equals(preference.getKey())) {
            if (mWakeupMode == VoiceCommandListener.VOW_MODE_WAKEUP_ANYONE) {
                startVoiceWakeupAnyone();
            } else if (mWakeupMode == VoiceCommandListener.VOW_MODE_WAKEUP_COMMAND) {
                startVoiceWakeupCommand();
            }
            return true;

        } else {
            Xlog.d(TAG, "onPreferenceClick not support click this preference "
                    + preference.getKey());
        }

        // mEnabledSwitch.setChecked(mVoiceConfigMgr.getVoiceControlEnable());
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    /*
     * Start Voice Wakeup Anyone Training Activity.
     * 
     */
    private void startVoiceWakeupAnyone() {
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr
                .getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            Xlog.d(TAG,
                    "wakeupInfo is null, can not start Wakeup Anyone activity");
            return;
        }
        // Now we need only one app to launch in wakeup anyone mode
        VoiceWakeupInfo wakeupInfo = wakeupInfos[0];
        Intent intent = new Intent("com.mediatek.vow.VOW_NO_SPEAKER_ID");
        intent.putExtra(KEY_COMMAND_ID, wakeupInfo.mID);
        intent.putExtra(KEY_COMMAND_PACKAGENAME, wakeupInfo.mPackageName);
        intent.putExtra(KEY_COMMAND_CLASSNAME, wakeupInfo.mClassName);
        intent.putExtra(KEY_COMMAND_KEYWORD, wakeupInfo.mKeyWord);
        Xlog.d(TAG,
                "startVoiceWakeupAnyone wakeupInfo mID:" + wakeupInfo.mID
                        + ", mPackageName" + wakeupInfo.mPackageName
                        + ", mClassName" + wakeupInfo.mClassName
                        + ", mKeyWord:" + Arrays.toString(wakeupInfo.mKeyWord));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startNewActivity(intent);
    }

    /*
     * Start Voice Wakeup Anyone Training Record Activity.
     * 
     */
    private void startVoiceWakeupAnyoneRecord() {
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr
                .getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            Xlog.d(TAG,
                    "wakeupInfo is null, can not start Wakeup Anyone Recordactivity");
            return;
        }
        // Now we need only one app to launch in wakeup anyone mode
        VoiceWakeupInfo wakeupInfo = wakeupInfos[0];
        ComponentName component = new ComponentName(wakeupInfo.mPackageName,
                wakeupInfo.mClassName);
        String componetStr = component.flattenToShortString();

        Intent intent = new Intent(
                "com.mediatek.voicewakeup.VOW_COMMAND_RECORD");
        intent.putExtra(KEY_COMMAND_ID, wakeupInfo.mID);
        intent.putExtra(KEY_COMMAND_VALUE, componetStr);
        intent.putExtra(KEY_COMMAND_TYPE, COMMAND_TYPE_RECORD);
        intent.putExtra(KEY_COMMAND_MODE, mWakeupMode);
        // Now we need only one keyword
        intent.putExtra(KEY_COMMAND_KEYWORD, wakeupInfo.mKeyWord);
        Xlog.d(TAG, "startVoiceWakeupAnyoneRecord wakeupInfo mID:"
                + wakeupInfo.mID + ", componetStr:" + componetStr
                + ", commandType:" + COMMAND_TYPE_RECORD + ", mKeyWord:"
                + Arrays.toString(wakeupInfo.mKeyWord));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startNewActivity(intent);
    }

    /*
     * Start Voice Wakeup Command Training Activity.
     * 
     */
    private void startVoiceWakeupCommand() {
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr
                .getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            Xlog.d(TAG,
                    "wakeupInfo is null, can not start Wakeup Command Activity");
            return;
        }
        List<String> commandList = new ArrayList<String>();
        for (int i = 0; i < wakeupInfos.length; i++) {
            VoiceWakeupInfo wakeupInfo = wakeupInfos[i];
            if (wakeupInfo != null) {
                commandList.add(wakeupInfo.mPackageName + "/"
                        + wakeupInfo.mClassName);
            }
        }
        Intent intent = new Intent("com.mediatek.vow.VOW_WITH_SPEAKER_ID");
        intent.putExtra(KEY_COMMAND_LISTS,
                commandList.toArray(new String[commandList.size()]));
        Xlog.d(TAG, "startVoiceWakeupCommand wakeupInfo" + "commandList:"
                + commandList);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startNewActivity(intent);
    }

    private void startNewActivity(Intent intent) {
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Toast.makeText(VoiceUiSettings.this, R.string.voice_wakeup_missing,
                    Toast.LENGTH_SHORT).show();
        }
    }

    private void wakeupAnyoneEnableEvent() {
        int cmdStatus = mVoiceConfigMgr.getWakeupCmdStatus();
        if (cmdStatus == VoiceCommandListener.VOW_STATUS_NO_COMMAND_UNCHECKED) {
            startVoiceWakeupAnyoneRecord();
        } else if (cmdStatus == VoiceCommandListener.VOW_STATUS_COMMAND_UNCHECKED) {
            WakeupStatusTask asyncTask = new WakeupStatusTask(
                    VoiceCommandListener.VOW_STATUS_COMMAND_CHECKED);
            asyncTask.execute();
        }
    }

    private void wakeupCommandEnableEvent() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setIcon(android.R.drawable.ic_dialog_alert);
        builder.setTitle(R.string.confirm_dialog_title);
        builder.setMessage(R.string.confirm_dialog_message);
        builder.setPositiveButton(R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        int cmdStatus = mVoiceConfigMgr.getWakeupCmdStatus();
                        if (cmdStatus == VoiceCommandListener.VOW_STATUS_NO_COMMAND_UNCHECKED) {
                            startVoiceWakeupCommand();
                        } else if (cmdStatus == VoiceCommandListener.VOW_STATUS_COMMAND_UNCHECKED) {
                            WakeupStatusTask asyncTask = new WakeupStatusTask(
                                    VoiceCommandListener.VOW_STATUS_COMMAND_CHECKED);
                            asyncTask.execute();
                        }
                        exceptedWakeupStatus = true;
                        dialog.dismiss();
                        mWakeupPref.setChecked(true);
                    }
                });
        builder.setNegativeButton(R.string.cancel,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        dialog.dismiss();
                    }
                });
        builder.show();
    }

    /*
     * Handle the click event when set switch disable.
     */
    private void handleWakeupDisableEvent() {
        int cmdStatus = mVoiceConfigMgr.getWakeupCmdStatus();
        if (cmdStatus == VoiceCommandListener.VOW_STATUS_NO_COMMAND_UNCHECKED
                || cmdStatus == VoiceCommandListener.VOW_STATUS_COMMAND_UNCHECKED) {
            return;
        }
        WakeupStatusTask asyncTask = new WakeupStatusTask(
                VoiceCommandListener.VOW_STATUS_COMMAND_UNCHECKED);
        asyncTask.execute();
    }

    /*
     * fetch summary of wakeup by anyone
     * 
     * @return String
     */
    private String fetchAnyoneSummary() {

        StringBuilder labels = new StringBuilder();
        ArrayList<String> wakeupLabels = new ArrayList<String>();
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr
                .getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            return new String("Error");
        }
        for (int i = 0; i < wakeupInfos.length; i++) {
            VoiceWakeupInfo wakeupInfo = wakeupInfos[i];
            if (wakeupInfo != null) {
                String wakeupLabel = mVoiceConfigMgr.getAppLabel(
                        wakeupInfo.mPackageName, wakeupInfo.mClassName);
                wakeupLabels.add(wakeupLabel);
            }
        }

        for (int i = 0; i < wakeupLabels.size(); i++) {
            labels.append(wakeupLabels.get(i));
            if (i < wakeupLabels.size() - 1) {
                labels.append(",");
            }
        }

        int resId = VoiceUiResUtil.getWakeupAnyoneResourceId();
        if (resId == 0) {
            return new String("Error");
        }
        String summary = getString(resId, labels.toString());
        Xlog.d(TAG, "fetchCommandSummary summary:" + summary);
        return summary;
    }

    /*
     * fetch summary of wakeup by command owner
     * 
     * @return String
     */
    private String fetchCommandSummary() {

        int resId = VoiceUiResUtil.getWakeupCommandResourceId();
        if (resId == 0) {
            return new String("Error");
        }

        String summary = getString(resId);
        Xlog.d(TAG, "fetchCommandSummary summary:" + summary);
        return summary;
    }

    /*
     * fetch title of wakeup
     * 
     * @return String
     */
    private String fetchWakeupTitle() {

        int resId = VoiceUiResUtil.getWakeupTitleResourceId();
        if (resId == 0) {
            return new String("Error");
        }

        String title = getString(resId);
        Xlog.d(TAG, "fetchWakeupTitle title:" + title);
        return title;
    }

    private class WakeupStatusTask extends AsyncTask<Void, Void, Void> {
        int cmdStatus = 0;

        private WakeupStatusTask(int cmdStatus) {
            this.cmdStatus = cmdStatus;
        }
        @Override
        protected Void doInBackground(Void... params) {
            // TODO Auto-generated method stub
            VoiceWakeup.setWakeupCmdStatus(VoiceUiSettings.this, cmdStatus);
            return null;
        }
    }
}
