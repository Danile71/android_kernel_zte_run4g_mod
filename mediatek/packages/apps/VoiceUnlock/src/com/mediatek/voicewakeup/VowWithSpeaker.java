package com.mediatek.voicewakeup;

import android.app.Activity;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;

import com.android.internal.widget.LockPatternUtils;
import com.mediatek.voiceunlock.R;
import com.mediatek.voiceunlock.SettingsPreferenceFragment;
import com.mediatek.voiceunlock.VoiceUnlockPreference;
import com.mediatek.voicewakeup.Utils.VowCommandInfo;
import com.mediatek.xlog.Xlog;

public class VowWithSpeaker extends PreferenceActivity {

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VowWithSpeakerFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        CharSequence msg = getText(R.string.voice_wake_up_set);
        showBreadCrumbs(msg, msg);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return (VowWithSpeakerFragment.class.getName().equals(fragmentName));
    }

    public static class VowWithSpeakerFragment extends SettingsPreferenceFragment {

        private static final String TAG = "VowWithSpeaker";
        private static final String BASE_PREF_KEY_VOW = "base_pref_vow";
        // must be the same value with ChooseLockGeneric {
        private static final int CONFIRM_EXISTING_REQUEST = 100;
        private static final String CONFIRM_CREDENTIALS = "confirm_credentials";
        // }
        private Context mContext;
        private Utils mUtils;

        String[] mCommandList;
        // wakeup keyword
        public static final String KEY_COMMAND_LISTS = "command_lists";
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mContext = getActivity().getBaseContext();
            mCommandList = getActivity().getIntent().getStringArrayExtra(KEY_COMMAND_LISTS);
            mUtils = Utils.getInstance();
            mUtils.setContext(mContext);
            addPreferencesFromResource(R.xml.voice_wakeup);
            initPreferences();
            // confirm password
            if (needConfirmPassword()) {
                Intent intent = new Intent();
                intent.setClassName("com.android.settings",
                        "com.android.settings.ChooseLockGeneric");
                intent.putExtra(CONFIRM_CREDENTIALS, false);
                intent.putExtra(LockPatternUtils.LOCKSCREEN_WEAK_FALLBACK_FOR, "VOW");
                startActivityForResult(intent, CONFIRM_EXISTING_REQUEST);
            }
        }

        @Override
        public void onActivityResult(int requestCode, int resultCode, Intent data) {
            super.onActivityResult(requestCode, resultCode, data);
            if (resultCode == Activity.RESULT_CANCELED) {
                getActivity().finish();
            }
        }

        // pattern/pin/password need confirmed 
        // match with the function launchConfirmationActivity of ChooseLockSettingsHelper.java in Settings
        private boolean needConfirmPassword() {
            LockPatternUtils lockPatternUtils = new LockPatternUtils(getActivity());
            boolean launched = false;
            Xlog.d(TAG,"get keyguard = " + lockPatternUtils.getKeyguardStoredPasswordQuality());
            switch (lockPatternUtils.getKeyguardStoredPasswordQuality()) {
            case DevicePolicyManager.PASSWORD_QUALITY_SOMETHING:
                if (!lockPatternUtils.isLockPatternEnabled() || !lockPatternUtils.savedPatternExists()) {
                	launched = false;
                } else {
                    launched = true;
                }
                break;
            case DevicePolicyManager.PASSWORD_QUALITY_NUMERIC:
            case DevicePolicyManager.PASSWORD_QUALITY_ALPHABETIC:
            case DevicePolicyManager.PASSWORD_QUALITY_ALPHANUMERIC:
            case DevicePolicyManager.PASSWORD_QUALITY_COMPLEX:
            	if (!lockPatternUtils.isLockPasswordEnabled()) {
                    launched = false;
            	} else {
                    launched = true;
            	}
                break;
            }
            return launched;
        }

        @Override
        public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
                Preference preference) {
            final int key = Integer.valueOf(preference.getKey().substring(
                    BASE_PREF_KEY_VOW.length()));
            Xlog.d(TAG, "onPreferenceTreeClick commandId = " + key);
            final VoiceUnlockPreference vPreference = (VoiceUnlockPreference) preference;
            if (mUtils.mKeyCommandInfoMap.get(key) == null) {
                return false;
            }

            int commandId = mUtils.mKeyCommandInfoMap.get(key).mId;
            if (vPreference.isChecked()) {
                Intent intent = new Intent();
                intent.putExtra(Utils.KEY_COMMAND_ID, commandId);
                intent.setClass(mContext, VowCommandActions.class);
                intent.putExtra(Utils.KEY_COMMAND_SUMMARY, vPreference.getSummary());
                intent.putExtra(Utils.KEY_COMMAND_TITLE,vPreference.getTitle());
                startActivity(intent);
            } else {
                // record and with speaker id mode
                mUtils.updateCommand(this, commandId,
                        mUtils.mKeyCommandInfoMap.get(key).mLaunchedApp.flattenToShortString(),
                        Utils.COMMAND_TYPE_RECORD, Utils.VOW_WITH_SPEAKER_MODE, null);
            }
            return true;
        }

        @Override
        public void onResume() {
            super.onResume();
            updateCommandStatusAndSummary();
        }

        @Override
        public void onPause() {
            super.onPause();
        }

        private void initPreferences() {
            Xlog.d(TAG, "initPreferences ");
            mUtils.mKeyCommandInfoMap.clear();
            if (mCommandList == null) {
                Xlog.e(TAG, "error, mCommandList == null ");
            	return;
            }
            int id = 0;
            for (int i = 0; i < mCommandList.length; i++) {
                String commandStr = mCommandList[i];
                // create preference base on command info
                VowCommandInfo info = getVowCommandInfo(commandStr, id);
                if (info != null) {
                    createPreference(info);
                    // put to map
                    mUtils.mKeyCommandInfoMap.put(id, info);
                }
                id++;
            }
        }

        private VowCommandInfo getVowCommandInfo(String commandStr, int id) {
            String packageStr = null;
            String classStr = null;
            Xlog.d(TAG, "getVowCommandInfo commandStr: " + commandStr);
            try {
                packageStr = commandStr.substring(0, commandStr.indexOf("/"));
                classStr = commandStr.substring(commandStr.indexOf("/") + 1);
            } catch (IndexOutOfBoundsException e) {
                return null;
            }
            Xlog.d(TAG, "getVowCommandInfo packageStr: " + packageStr + " classStr: " + classStr);
            ComponentName component = new ComponentName(packageStr, classStr);
            String label = mUtils.getAppLabel(component);
            if (label == null) {
                return null;
            }
            Xlog.d(TAG, "getVowCommandInfo label: " + label);
            // init VoiceWakeUpCommandInfo
            VowCommandInfo info = new VowCommandInfo(id);
            info.mLaunchedApp = component;
            info.mPreferTitle = mContext.getString(R.string.voice_wake_up_command_title, label);
            info.mPreferSummary = mContext.getString(R.string.voice_wake_up_command_summary, label);
            return info;
        }

        private void createPreference(VowCommandInfo info) {
            VoiceUnlockPreference preference = new VoiceUnlockPreference(mContext);
            preference.setProfileKey(BASE_PREF_KEY_VOW + info.mId);
            preference.setTitle(info.mPreferTitle);
            preference.setSummary(info.mPreferSummary);
            getPreferenceScreen().addPreference(preference);
        }

        private void updateCommandStatusAndSummary() {
            Xlog.d(TAG, "updateCommandStatus ");
            for (int i = 0; i < mUtils.mKeyCommandInfoMap.size(); i++) {
                Xlog.d(TAG, "updateCommandStatus key: " + i);
                boolean checked = Settings.System.getVoiceCommandValue(mContext
                        .getContentResolver(), Settings.System.BASE_VOICE_WAKEUP_COMMAND_KEY,
                        mUtils.mKeyCommandInfoMap.get(i).mId) != null;
                ((VoiceUnlockPreference) findPreference(BASE_PREF_KEY_VOW
                        + mUtils.mKeyCommandInfoMap.get(i).mId)).setChecked(checked);
            }
        }
    }
}
