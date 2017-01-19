package com.mediatek.phone.plugin;

import android.os.Build;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.calloption.plugin.OP09CallOptionUtils;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op09.plugin.R;
import com.mediatek.phone.plugin.OP09PhoneGlobalsExtension;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class CdmaInfoSpecification extends PreferenceActivity {

    private static final String TAG = "CdmaInfoSpecification";

    private static final String FILENAME_MSV = "/sys/board_properties/soc/msv";

    private static final String KEY_PRODUCT_MODEL = "product_model";
    private static final String KEY_HARDWARE_VERSION = "hardware_version";
    private static final String KEY_SOFTWARE_VERSION = "software_version";
    public static final String KEY_CDMA_INFO = "cdma_info";
    public static final String KEY_PRL_VERSION = "prl_version";
    public static final String KEY_SID = "sid";
    public static final String KEY_NID = "nid";
    public static final String KEY_MEID = "meid";
    public static final String KEY_UIM_ID = "uim_id";
    public static final String KEY_ESN = "esn";
    public static final String IS_CDMA_PHONE = "cdma";
    public static final String IS_MEID = "is_meid";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.cdma_info_specifications);
        setPhoneValuesToPreferences();
        setCDMAValuesToPreference(getIntent().getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1));
    }

    private void setPhoneValuesToPreferences() {
        Xlog.d(TAG, "setPhoneValuesToPreferences()");
        PreferenceScreen parent = (PreferenceScreen) getPreferenceScreen();
        Preference preference = parent.findPreference(KEY_PRODUCT_MODEL);
        if (null != preference) {
            preference.setSummary(Build.MODEL + getMsvSuffix());
        }
        preference = parent.findPreference(KEY_SOFTWARE_VERSION);
        if (null != preference) {
            preference.setSummary(Build.DISPLAY);
        }
    }

    private void setCDMAValuesToPreference(int slot) {
        Xlog.d(TAG, "setCDMAValuesToPreference(), slot = " + slot);
        if (-1 == slot || !getIntent().getBooleanExtra(IS_CDMA_PHONE, false)) {
            Xlog.d(TAG, "slotId = " + slot);
            Preference preference = findPreference(KEY_CDMA_INFO);
            if (null != preference) {
                preference.setEnabled(false);
                preference.setShouldDisableView(true);
            }
            preference = findPreference(KEY_PRL_VERSION);
            if (null != preference) {
                preference.setEnabled(false);
                preference.setShouldDisableView(true);
            }
            preference = findPreference(KEY_SID);
            if (null != preference) {
                preference.setEnabled(false);
                preference.setShouldDisableView(true);
            }
            preference = findPreference(KEY_NID);
            if (null != preference) {
                preference.setEnabled(false);
                preference.setShouldDisableView(true);
            }
            preference = findPreference(KEY_MEID);
            if (null != preference) {
                preference.setEnabled(false);
                preference.setShouldDisableView(true);
            }
            preference = findPreference(KEY_UIM_ID);
            if (null != preference) {
                preference.setEnabled(false);
                preference.setShouldDisableView(true);
            }
            return;
        }

        Preference preference = findPreference(KEY_PRL_VERSION);
        if (null != preference) {
            preference.setSummary(getIntent().getStringExtra(KEY_PRL_VERSION));
        }
        preference = findPreference(KEY_SID);
        if (null != preference) {
            preference.setSummary(getIntent().getStringExtra(KEY_SID));
        }
        preference = findPreference(KEY_NID);
        if (null != preference) {
            preference.setSummary(getIntent().getStringExtra(KEY_NID));
        }
        preference = findPreference(KEY_MEID);
        if (null != preference) {
            if (getIntent().getBooleanExtra(IS_MEID, false)) {
                preference.setTitle(getString(R.string.current_meid));
                preference.setSummary(getIntent().getStringExtra(KEY_MEID));
            } else {
                preference.setTitle(getString(R.string.current_esn));
                preference.setSummary(getIntent().getStringExtra(KEY_ESN));
            }
        }
        preference = findPreference(KEY_UIM_ID);
        if (null != preference) {
            preference.setSummary(getIntent().getStringExtra(KEY_UIM_ID));
        }
    }

    /**
     * Returns " (ENGINEERING)" if the msv file has a zero value, else returns "".
     * @return a string to append to the model number description.
     */
    private String getMsvSuffix() {
        // Production devices should have a non-zero value. If we can't read it, assume it's a
        // production device so that we don't accidentally show that it's an ENGINEERING device.
        try {
            String msv = readLine(FILENAME_MSV);
            // Parse as a hex number. If it evaluates to a zero, then it's an engineering build.
            if (Long.parseLong(msv, 16) == 0) {
                return " (ENGINEERING)";
            }
        } catch (IOException ioe) {
            // Fail quietly, as the file may not exist on some devices.
        } catch (NumberFormatException nfe) {
            // Fail quietly, returning empty string should be sufficient
        }
        return "";
    }

    /**
     * Reads a line from the specified file.
     * @param filename the file to read from
     * @return the first line, if any.
     * @throws IOException if the file couldn't be read
     */
    private String readLine(String filename) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(filename), 256);
        try {
            return reader.readLine();
        } finally {
            reader.close();
        }
    }
}
