package com.mediatek.miravision.ui;

import android.app.Activity;
import android.content.ContentResolver;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.UserHandle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.miravision.utils.CurrentUserTracker;

public class AalSettingsFragment extends PreferenceFragment implements OnPreferenceChangeListener,
        OnPreferenceClickListener {
    // Dialog IDs
    static final int DIALOG_ID_BRIGHTNESS = 0;
    static final int DIALOG_ID_SENSITIVITY = 1;

    private static final String TAG = "Miravision/AalSettingsFragment";

    // Preference keys
    private static final String KEY_LIGHT_SENSITIVE_PREF = "light_sensitive_bright_pref";
    private static final String KEY_BRIGHTNESS_PREF = "brightness_pref";
    private static final String KEY_SENSITIVITY_PREF = "sensitivity_pref";
    private static final String KEY_CONTENT_SENSITIVE_PREF = "content_sensitive_bright_pref";
    private static final String KEY_READABILITY_ENHANCER_PREF = "readability_enhancer_pref";

    private CheckBoxPreference mLightSensitivePref;
    private Preference mBrightnessPref;
    private Preference mSensitivityPref;
    private CheckBoxPreference mContentSensitivePref;
    private CheckBoxPreference mReadabilityEnhancerPref;
    private CurrentUserTracker mUserTracker;

    private ContentResolver mContentResolver;
    private final ContentObserver mAalModeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            if (selfChange) {
                return;
            }
            updateCheckboxPrefStatus();
            updatePreferenceStatus();
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.aal_settings);
        initializeAllPreferences();
        Activity activity = getActivity();
        if (activity != null) {
            mContentResolver = activity.getContentResolver();
        }
        mUserTracker = new CurrentUserTracker(activity) {
            @Override
            public void onUserSwitched(int newUserId) {
                updateCheckboxPrefStatus();
                updatePreferenceStatus();
            }
        };
        mContentResolver.registerContentObserver(Settings.System
                .getUriFor(Settings.System.AAL_FUNCTION), false, mAalModeObserver,
                UserHandle.USER_ALL);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateCheckboxPrefStatus();
        updatePreferenceStatus();
    }

    @Override
    public void onDestroy() {
        mContentResolver.unregisterContentObserver(mAalModeObserver);
        mUserTracker.stopTracking();
        super.onDestroy();
    }

    private void initializeAllPreferences() {
        mLightSensitivePref = (CheckBoxPreference) findPreference(KEY_LIGHT_SENSITIVE_PREF);
        mLightSensitivePref.setOnPreferenceChangeListener(this);
        mBrightnessPref = findPreference(KEY_BRIGHTNESS_PREF);
        mBrightnessPref.setOnPreferenceClickListener(this);
        mSensitivityPref = findPreference(KEY_SENSITIVITY_PREF);
        mSensitivityPref.setOnPreferenceClickListener(this);
        mContentSensitivePref = (CheckBoxPreference) findPreference(KEY_CONTENT_SENSITIVE_PREF);
        mContentSensitivePref.setOnPreferenceChangeListener(this);
        mReadabilityEnhancerPref = (CheckBoxPreference) findPreference(KEY_READABILITY_ENHANCER_PREF);
        mReadabilityEnhancerPref.setOnPreferenceChangeListener(this);
    }

    private void updateCheckboxPrefStatus() {
        Settings.System.ScreenBrightnessMode mode = Settings.System.getScreenBrightnessMode(
                mContentResolver, UserHandle.USER_CURRENT);
        Log.d(TAG, "updateCheckboxPrefStatus, mode AAL function = " + mode.getAALFunction());
        mLightSensitivePref.setChecked(mode.isAALFunctionEnabled(Settings.System.AAL_FUNC_LABC));
        mContentSensitivePref.setChecked(mode.isAALFunctionEnabled(Settings.System.AAL_FUNC_CABC));
        mReadabilityEnhancerPref
                .setChecked(mode.isAALFunctionEnabled(Settings.System.AAL_FUNC_DRE));
    }

    private void updatePreferenceStatus() {
        mSensitivityPref.setEnabled(mLightSensitivePref.isChecked());
        mBrightnessPref.setEnabled(!mLightSensitivePref.isChecked());
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "onPreferenceChange preference: " + preference.getKey() + " newValue: "
                + newValue);
        boolean returnVaule = true;
        boolean status = (Boolean) newValue;
        if (preference == mLightSensitivePref) {
            setAalFunction(mContentResolver, Settings.System.AAL_FUNC_LABC, status);
            mSensitivityPref.setEnabled(status);
            mBrightnessPref.setEnabled(!status);
        } else if (preference == mContentSensitivePref) {
            setAalFunction(mContentResolver, Settings.System.AAL_FUNC_CABC, status);
        } else if (preference == mReadabilityEnhancerPref) {
            setAalFunction(mContentResolver, Settings.System.AAL_FUNC_DRE, status);
        } else {
            returnVaule = false;
        }
        return returnVaule;
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        Log.d(TAG, "onPreferenceClick preference: " + preference.getKey());
        boolean returnVaule = true;
        if (preference == mBrightnessPref) {
            new BrightnessDialogFragment(getActivity(), DIALOG_ID_BRIGHTNESS).show(
                    getFragmentManager(), "brightness");
        } else if (preference == mSensitivityPref) {
            new BrightnessDialogFragment(getActivity(), DIALOG_ID_SENSITIVITY).show(
                    getFragmentManager(), "sensitivity");
        } else {
            returnVaule = false;
        }
        return returnVaule;
    }

    public static void setAalFunction(ContentResolver contentResolver, int function, boolean status) {
        int oldAal = Settings.System.getScreenBrightnessMode(contentResolver,
                UserHandle.USER_CURRENT).getAALFunction();
        Log.d(TAG, "setAalFunction, oldAal function = " + oldAal);

        int newAal = status ? (oldAal | function) : (oldAal & ~function);
        Log.d(TAG, "setAalFunction, newAal function = " + newAal);
        Settings.System.setAALFunction(contentResolver, function, newAal, UserHandle.USER_CURRENT);
    }
}
