package com.mediatek.settings.plugin;

import static android.provider.Settings.System.SCREEN_OFF_TIMEOUT;

import android.bluetooth.BluetoothAdapter;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.wifi.WifiManager;
import android.os.IPowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Settings;

import com.mediatek.audioprofile.AudioProfileManager;
import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 *
 * Description:
 * <p>
 * Customization for CT,Power Saving Mode
 * <p>
 * <p>
 * A proxy class implements all functions
 * <p>
 *
 */
public class PowerSavingManager {

    public static final String CT_POWER_SAVING_MODE = "ct_power_saving_mode";

    private static final String TAG = "Power Savings";
    private static final String KEY_CPU = "power_saving_cpu";
    private static final String KEY_BRIGHT = "power_saving_brightness";
    private static final String KEY_BRIGHT_AUTO = "power_saving_brightness_auto";
    private static final String KEY_BRIGHT_CONFIG = "power_saving_brightness_config";
    private static final String KEY_SLEEP = "power_saving_sleep";
    private static final String KEY_SLEEP_CONFIG = "power_saving_sleep_config";
    private static final String KEY_WIFI = "power_saving_wlan";
    private static final String KEY_BT = "power_saving_bluetooth";
    private static final String KEY_GPS = "power_saving_gps";
    private static final String KEY_SYNC = "power_saving_sync";
    private static final String KEY_TACTILE = "power_saving_tactile";

    private static final String ENABLE_CPU_POWER_SAVING_COMMAND = "/system/bin/thermal_manager /etc/.tp/thermal.conf";
    private static final String DISABLE_CPU_POWER_SAVING_COMMAND = "/system/bin/thermal_manager /etc/.tp/thermal.off.conf";
    private static final String CAT_CPU_POWER_SAVING_STATUS_COMMAND = "cat /data/.tp.settings";
    private static final int FALLBACK_SCREEN_TIMEOUT_VALUE = 30000;
    private static final int DEFAULT_BRIGHTNESS = 15;

    private Context mContext;
    private SharedPreferences mSharePref;
    private static PowerSavingManager sInstance;

    private static final String MODE_CHANGING_ACTION =
        "com.android.settings.location.MODE_CHANGING";
    private static final String NEW_MODE_KEY = "NEW_MODE";

    /**
     *
     * @param context context
     * @return the single instance
     */
    public static synchronized PowerSavingManager getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new PowerSavingManager(context);
        }
        return sInstance;
    }

    /**
     * 
     * @param context Settings context
     */
    public PowerSavingManager(Context context) {
        mContext = context;
        mSharePref = PreferenceManager.getDefaultSharedPreferences(mContext);
    }

    /**
     * 
     * @param isTurnOn Turn on or turn off power savings
     */
    public void turnOn(boolean isTurnOn) {
        setPowerSavingModeOn(isTurnOn);
        if (isTurnOn) {
            turnOnPowerSavingMode();
        } else {
            turnOffPowerSavingMode();
        }
    }

    /**
     * Set power saving mode on or off, only set the on/off flag
     * @param enable True to turn on, false to turn off
     */
    private void setPowerSavingModeOn(boolean enable) {
        Settings.System.putInt(mContext.getContentResolver(),
                CT_POWER_SAVING_MODE, enable ? 1 : 0);
    }

    /**
     * Whether power saving mode is on or off
     */
    public boolean isPowerSavingModeOn() {
        int mode = Settings.System.getInt(mContext.getContentResolver(),
                CT_POWER_SAVING_MODE, 0);
        return mode == 1;
    }

    /**
     * According user's configuration turn on power saving mode.
     */
    private void turnOnPowerSavingMode() {

        if (mSharePref.getBoolean(KEY_CPU, true)) {
            Xlog.d(TAG, "CPU saving is checked");
            enableCpuSaving(true);
        }

        if (mSharePref.getBoolean(KEY_WIFI, true)) {
            Xlog.d(TAG, "Turn off wifi is checked");
            turnOffWifi();
        }

        if (mSharePref.getBoolean(KEY_BT, true)) {
            Xlog.d(TAG, "Turn off Bluetooth is checked");
            turnOffBlueTooth();
        }

        if (mSharePref.getBoolean(KEY_GPS, true)) {
            Xlog.d(TAG, "Turn off GPS is checked");
            turnOffGps();
        }

        if (mSharePref.getBoolean(KEY_SLEEP, true)) {
            Xlog.d(TAG, "Sleep is checked");
            setSleepTime();
        }

        if (mSharePref.getBoolean(KEY_BRIGHT, true)) {
            Xlog.d(TAG, "Brightness is checked");
            turnOffBrightness();
        }

        if (mSharePref.getBoolean(KEY_SYNC, true)) {
            Xlog.d(TAG, "Sync is checked");
            turnOffSync();
        }

        if (mSharePref.getBoolean(KEY_TACTILE, true)) {
            Xlog.d(TAG, "Turn off Tactile feedback is checked");
            turnOffTactileFeedback();
        }
    }

    /**
     * 
     * @param key
     *            the preference key
     */
    public void turnOnPowerSavingMode(String key) {
        if (key == null) {
            return;
        }
        if (KEY_CPU.endsWith(key)) {
            Xlog.d(TAG, "Enable CPU saving is checked");
            enableCpuSaving(true);
        } else if (KEY_WIFI.endsWith(key)) {
            Xlog.d(TAG, "Turn off wifi is checked");
            turnOffWifi();
        } else if (KEY_BT.endsWith(key)) {
            Xlog.d(TAG, "Turn off Bluetooth is checked");
            turnOffBlueTooth();
        } else if (KEY_GPS.endsWith(key)) {
            Xlog.d(TAG, "Turn off GPS is checked");
            turnOffGps();
        } else if (KEY_SLEEP.endsWith(key)) {
            Xlog.d(TAG, "Sleep is checked");
            setSleepTime();
        } else if (KEY_BRIGHT.endsWith(key)) {
            Xlog.d(TAG, "Brightness is checked");
            turnOffBrightness();
        } else if (KEY_SYNC.endsWith(key)) {
            Xlog.d(TAG, "Sync is checked");
            turnOffSync();
        } else if (KEY_TACTILE.endsWith(key)) {
            Xlog.d(TAG, "Turn off Tactile feedback is checked");
            turnOffTactileFeedback();
        } else {
            Xlog.w(TAG, "Wrong preference key: " + key);
        }
    }

    /**
     * According user's configuration turn off power saving mode.
     * 
     * 1. Turn off CPU saving mode.
     */
    private void turnOffPowerSavingMode() {
        if (!mSharePref.getBoolean("CPU", true)) {
            Xlog.d(TAG, "Turn Off Power Saving Mode!");
            Xlog.d(TAG, "Disable CPU saving...");
            enableCpuSaving(false);
        }
    }

    /**
     * 
     * @return the result of turn off WLAN.
     */
    public boolean turnOffWifi() {
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager.isWifiEnabled()) {
            Xlog.d(TAG, "Wifi is enable,try trun off ...");
            return wifiManager.setWifiEnabled(false);
        }
        return true;
    }

    /**
     * 
     * @return the result of turn off BT.
     */
    public boolean turnOffBlueTooth() {
        BluetoothAdapter blueToothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (blueToothAdapter != null && blueToothAdapter.getState() == BluetoothAdapter.STATE_ON) {
            Xlog.d(TAG, "Bluetooth is enable,try trun off ...");
            blueToothAdapter.disable();
        }
        return true;
    }

    /**
     * 
     * Refer to Juan Xia's personal branch ALPS.JB.p99_CodeBase1 PowerUsageSummary.java
     * 
     * @param isStart
     *            enable or disable
     */
    public void enableCpuSaving(boolean isStart) {
        String command = isStart ? ENABLE_CPU_POWER_SAVING_COMMAND : DISABLE_CPU_POWER_SAVING_COMMAND;

        try {
            Xlog.d(TAG, "PowerSavingTASK doInBackground");
            java.lang.Process process = Runtime.getRuntime().exec(ENABLE_CPU_POWER_SAVING_COMMAND);
            int value = process.waitFor();
            Xlog.d(TAG, "PowerSavingTASK command result is " + value);
        } catch (IOException e) {
            Xlog.d(TAG, "PowerSavingTASK IOException" + e);
        } catch (InterruptedException e) {
            Xlog.d(TAG, "PowerSavingTASK InterruptedException" + e);
        }
    }

    /**
     * Refer to PowerUsageSummary.java
     * 
     * @return CPU state
     */
    public int getCpuState() {
        // int EXEC_COMMAND_SUCCESS = 0;
        // int EXEC_COMMAND_FAIL = 1;

        // String POWER_SAVING_MODE_FILE = "/etc/.tp/thermal.conf";
        int result = 1;
        String resultString = "";
        BufferedReader bufferedReader = null;
        try {
            java.lang.Process process = Runtime.getRuntime().exec(CAT_CPU_POWER_SAVING_STATUS_COMMAND);
            bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            try {
                if (process.waitFor() != 0) {
                    Xlog.d(TAG, "exit value = " + process.exitValue());
                    result = 1;
                } else {
                    resultString = bufferedReader.readLine();
                    result = 0;
                }
            } catch (InterruptedException e) {
                Xlog.e(TAG, "exe shell command InterruptedException: " + e.getMessage());
                result = 1;
            }
        } catch (IOException e) {
            Xlog.e(TAG, "exe shell command IOException: " + e.getMessage());
            result = 1;
        } finally {
            if (null != bufferedReader) {
                try {
                    bufferedReader.close();
                } catch (IOException e) {
                    Xlog.e(TAG, "close reader in finally block exception: " + e.getMessage());
                }
            }
        }
        Xlog.d(TAG, "Result is " + resultString);
        return result;
    }

    /**
     * 
     * @return time out
     */
    public int getTimoutValue() {
        int currentValue = Settings.System.getInt(mContext.getContentResolver(), SCREEN_OFF_TIMEOUT,
                FALLBACK_SCREEN_TIMEOUT_VALUE);
        Xlog.d(TAG, "getTimoutValue()---currentValue=" + currentValue);
        int bestMatch = 0;
        int timeout = 0;
        // final CharSequence[] valuesTimeout = mScreenTimeoutPreference.getEntryValues();
        final CharSequence[] valuesTimeout = mContext.getResources().getStringArray(R.array.screen_timeout_values);
        for (int i = 0; i < valuesTimeout.length; i++) {
            timeout = Integer.parseInt(valuesTimeout[i].toString());
            if (currentValue == timeout) {
                return currentValue;
            } else {
                if (currentValue > timeout) {
                    bestMatch = i;
                }
            }
        }
        Xlog.d(TAG, "getTimoutValue()---bestMatch=" + bestMatch);
        return Integer.parseInt(valuesTimeout[bestMatch].toString());
    }

    /*
     * <!-- Display settings. The delay in inactivity before the screen is turned off. These are shown in a list dialog. -->
     * <string-array name="screen_timeout_entries"> <item>15 seconds</item> <item>30 seconds</item> <item>1 minute</item>
     * <item>2 minutes</item> <item>5 minutes</item> <item>10 minutes</item> <item>30 minutes</item> </string-array>
     * 
     * <!-- Do not translate. --> <string-array name="screen_timeout_values" translatable="false"> <!-- Do not translate. -->
     * <item>15000</item> <!-- Do not translate. --> <item>30000</item> <!-- Do not translate. --> <item>60000</item> <!-- Do
     * not translate. --> <item>120000</item> <!-- Do not translate. --> <item>300000</item> <!-- Do not translate. -->
     * <item>600000</item> <!-- Do not translate. --> <item>1800000</item> </string-array>
     */

    /**
     * 
     * Refer DisplaySettings KEY SCREEN_OFF_TIMEOUT
     */
    public void setSleepTime() {
        int index = mSharePref.getInt(KEY_SLEEP_CONFIG, 0);
        String[] valuesTimeout = mContext.getResources().getStringArray(R.array.screen_timeout_values);
        int value = Integer.parseInt(valuesTimeout[index].toString());
        try {
            Settings.System.putInt(mContext.getContentResolver(), SCREEN_OFF_TIMEOUT, value);
            Xlog.d(TAG, "set Sleep Time : " + value);
        } catch (NumberFormatException e) {
            Xlog.e(TAG, "could not persist screen timeout setting", e);
        }
    }

    /**
     * 
     * Refer com.android.settings.LocationSettings KEY mGps
     */
    public void turnOffGps() {
        Intent intent = new Intent(MODE_CHANGING_ACTION);
        intent.putExtra(NEW_MODE_KEY, Settings.Secure.LOCATION_MODE_OFF);
        mContext.sendBroadcast(intent, android.Manifest.permission.WRITE_SECURE_SETTINGS);
        Settings.Secure.putInt(mContext.getContentResolver(), 
                Settings.Secure.LOCATION_MODE, Settings.Secure.LOCATION_MODE_OFF);
    }

    /**
     * Refer com.mediatek.audioprofile.AudioProfileSettings KEY updateActivePreference Editprofile.java KEY mProfileManager
     */
    public void turnOffTactileFeedback() {
        AudioProfileManager mProfileManager = (AudioProfileManager) mContext.getSystemService(Context.AUDIOPROFILE_SERVICE);
        String key = mProfileManager.getActiveProfileKey();
        mProfileManager.setVibrationEnabled(key, false);
        mProfileManager.setDtmfToneEnabled(key, false);
        mProfileManager.setSoundEffectEnabled(key, false);
        mProfileManager.setLockScreenEnabled(key, false);
        mProfileManager.setHapticFeedbackEnabled(key, false);
    }

    /**
     * To set brightness by IPowerManager
     * 
     * @param brightness
     *            value
     */
    //
    // public void setBrightness(int brightness) { try { IPowerManager power =
    // IPowerManager.Stub.asInterface(ServiceManager.getService("power")); // Only set backlight value when screen is on if
    // (power != null && power.isScreenOn()) { power.setBacklightBrightness(brightness); } } catch (RemoteException doe) {
    // Xlog.e(TAG, "set backlight brightness fail", doe); } Settings.System.putInt(mContext.getContentResolver(),
    // Settings.System.SCREEN_BRIGHTNESS, brightness); }

    /**
     * M: Set the brightness of devices
     * 
     * @param brightness
     *            value
     * 
     */
    public void setBrightness(int brightness) {
        try {
            IPowerManager power = IPowerManager.Stub.asInterface(ServiceManager.getService("power"));
            // Only set backlight value when screen is on
            if (power != null) {
                if (power.isScreenOn()) {
                    power.setTemporaryScreenBrightnessSettingOverride(brightness);
                } else {
                    power.setTemporaryScreenBrightnessSettingOverride(-1);
                }
            }
        } catch (RemoteException doe) {
            Xlog.e(TAG, "set backlight brightness fail", doe);
        }
        Settings.System.putInt(mContext.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, brightness);
    }

    /**
     * 
     * @param mode
     *            auto?1:0
     */
    public void setBrightnessAutoMode(int mode) {
        Settings.System.putInt(mContext.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, mode);
    }

    /**
     * 
     * @return the current brightness
     */
    public int getBrightness() {
        return Settings.System.getInt(mContext.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, mContext
                .getResources().getInteger(com.android.internal.R.integer.config_screenBrightnessDim));
    }

    /**
     * 
     * @return the current brightness auto mode
     */
    public int getBrightnessAutoMode() {
        return Settings.System.getInt(mContext.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, 0);
    }

    /**
     * set brightness
     */
    public void turnOffBrightness() {
        if (mSharePref.getBoolean(KEY_BRIGHT_AUTO, false)) {
            setBrightnessAutoMode(1);
        } else {
            // Backlight range is from 0 - 255. Need to make sure that user
            // doesn't set the backlight to 0 and get stuck
            int brightnessValue = mSharePref.getInt(KEY_BRIGHT_CONFIG, DEFAULT_BRIGHTNESS);
            /** Sync brightness config with Settings display module */
            setBrightnessAutoMode(0);
            setBrightness(brightnessValue);
        }
    }

    /**
     * Refer DataUsageSummary.java [KEY setRestrictBackground]
     */
    public void turnOffSync() {
        if (ContentResolver.getMasterSyncAutomatically()) {
            ContentResolver.setMasterSyncAutomatically(false);
        }
    }

}
