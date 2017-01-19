package com.mediatek.settings.plugin;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Switch;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

import java.text.NumberFormat;

/**
 * 
 * 
 * Description:
 * <p>
 * Customization for CT,Power Saving Mode
 * <p>
 * <p>
 * Main screen
 * <p>
 * 
 */
public class PowerSavingSettings extends PreferenceActivity implements OnCheckedChangeListener {

    private static final String TAG = "Power Savings";

    private static final String KEY_BRIGHT_PREFERENCE = "power_saving_brightness";
    private static final String KEY_SLEEP_PREFERENCE = "power_saving_sleep";
    private static final String KEY_BRIGHT_AUTO = "power_saving_brightness_auto";
    private static final String KEY_BRIGHT_CONFIG = "power_saving_brightness_config";
    private static final String KEY_SLEEP_CONFIG = "power_saving_sleep_config";

    private static final int TEN = 10;
    private static final int BRIGHTNESS_DIALOG = 100;
    private static final int SLEEP_DIALOG = 101;

    private int mScreenBrightnessDim; // 15
    private static final int MAXIMUM_BACKLIGHT = android.os.PowerManager.BRIGHTNESS_ON;

    private PowerSavingSettingPref mBrightnessPreference;
    private PowerSavingSettingPref mSleepPreference;
    private Switch mEnabledSwitch; // the switch of power saving mode

    private PowerSavingManager mPowerSavingManager;
    private SharedPreferences mSharePref;
    private Editor mEditor;

    private int mOldBrightnessProgress;
    private boolean mOldBrightnessAutoChecked;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Xlog.d(TAG, "-->PowerSavingSettings onCreate()");
        addPreferencesFromResource(R.xml.power_saving_mode);
        initUI();

        /** Init PowerSavingManager, pass the context */
        mPowerSavingManager = PowerSavingManager.getInstance(this);
        mScreenBrightnessDim = getResources().getInteger(com.android.internal.R.integer.config_screenBrightnessDim);
        mSharePref = PreferenceManager.getDefaultSharedPreferences(this);
        mEditor = mSharePref.edit();
    }

    @Override
    public void onResume() {
        super.onResume();
        updateUI();
    }

    private void initUI() {
        mBrightnessPreference = (PowerSavingSettingPref) findPreference(KEY_BRIGHT_PREFERENCE);
        mBrightnessPreference.setImageViewClickLister(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                showDialog(BRIGHTNESS_DIALOG);
            }
        });
        mSleepPreference = (PowerSavingSettingPref) findPreference(KEY_SLEEP_PREFERENCE);
        mSleepPreference.setImageViewClickLister(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                showDialog(SLEEP_DIALOG);
            }
        }); // TODO

        mEnabledSwitch = new Switch(this);
        final int padding = getResources().getDimensionPixelSize(R.dimen.action_bar_switch_padding); // 24
        mEnabledSwitch.setPadding(0, 0, padding, 0);
        mEnabledSwitch.setOnCheckedChangeListener(this);

        getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar().setCustomView(
                mEnabledSwitch,
                new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT,
                        Gravity.CENTER_VERTICAL | Gravity.RIGHT));
    }

    private void updateUI() {
        boolean isPoSavingChecked = mPowerSavingManager.isPowerSavingModeOn();
        Xlog.d(TAG, "PowerSavingSettings onResume(), Power Saving Switch: " + isPoSavingChecked);
        if (isPoSavingChecked != mEnabledSwitch.isChecked()) {
            mEnabledSwitch.setChecked(isPoSavingChecked);
        }
        // if (!isPoSavingChecked) {
        getPreferenceScreen().setEnabled(isPoSavingChecked);
        // } // TODO why?
        /** mOldBrightnessProgress equals current progress + mScreenBrightnessDim */
        mOldBrightnessProgress = mSharePref.getInt(KEY_BRIGHT_CONFIG, -1);
        if (mOldBrightnessProgress == -1) {
            mOldBrightnessProgress = (MAXIMUM_BACKLIGHT - mScreenBrightnessDim) / TEN + mScreenBrightnessDim;
        } else {
            mBrightnessPreference.setSummary(getBrightnessPercent(mOldBrightnessProgress - mScreenBrightnessDim));
        }
        mOldBrightnessAutoChecked = mSharePref.getBoolean(KEY_BRIGHT_AUTO, false);

        int timeOutPosition = mSharePref.getInt(KEY_SLEEP_CONFIG, 0);
        mSleepPreference.setSummary(getString(R.string.screen_timeout_summary, getResources().getStringArray(
                R.array.screen_timeout_entries)[timeOutPosition]));
    }

    private String getBrightnessPercent(float currentBrightness) {
        float brightnessPercent = currentBrightness / (float) (MAXIMUM_BACKLIGHT - mScreenBrightnessDim);
        NumberFormat nf = NumberFormat.getPercentInstance();
        // nf.setMinimumFractionDigits(2);
        String percent = nf.format(brightnessPercent);
        Xlog.d(TAG, "Max brightness: " + MAXIMUM_BACKLIGHT + "Current Brightness" + currentBrightness + "Precent progress: "
                + brightnessPercent + "Percent: " + percent + "%");
        return percent;
    }

    private void saveBrightness(int config, boolean auto) {
        mEditor.putInt(KEY_BRIGHT_CONFIG, config);
        mEditor.putBoolean(KEY_BRIGHT_AUTO, auto);
        mEditor.commit();
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        boolean isPoSavingChecked = mPowerSavingManager.isPowerSavingModeOn();
        Xlog.d(TAG, "-->Switch onCheckedChanged() -- " + isChecked + ", " + isPoSavingChecked);
        if (isPoSavingChecked == isChecked) {
            // Maybe the power savings state is changed in other way, then enter this screen,the switch will change.
            // Don't care this change
            Xlog.d(TAG, "-->Don't care this change -- ");
            return;
        }
        getPreferenceScreen().setEnabled(isChecked);
        /** Turn on or turn off power saving mode */
        mPowerSavingManager.turnOn(isChecked);
    }

    private Dialog createBrightnessDialog() {
        // /init the dialog view @{
        LayoutInflater inflater = LayoutInflater.from(this);
        View view = inflater.inflate(R.layout.power_saving_dialog_brightness, null);
        final CheckBox autoCheckBox = (CheckBox) view.findViewById(R.id.power_saving_brightness_checkbox);
        final SeekBar seekBar = (SeekBar) view.findViewById(R.id.power_saving_brightness_seekbar);
        autoCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton dialog, boolean checked) {
                Xlog.d(TAG, "BrightnessDialog1: onClick checkbox -> " + checked);
                seekBar.setEnabled(!checked);
                mEditor.putBoolean(KEY_BRIGHT_AUTO, checked);
                mEditor.commit();
                mPowerSavingManager.setBrightnessAutoMode(checked ? 1 : 0);
            }
        });

        boolean currentAutomode = mSharePref.getBoolean(KEY_BRIGHT_AUTO, false);
        autoCheckBox.setChecked(currentAutomode);

        int currentBrightness = mSharePref.getInt(KEY_BRIGHT_CONFIG, -1);
        Xlog.d(TAG, "onCreateDialog() current brightness is " + currentBrightness);
        if (currentBrightness == -1) {
            /** set default value 10% */
            seekBar.setProgress((MAXIMUM_BACKLIGHT - mScreenBrightnessDim) / TEN);
        } else {
            seekBar.setProgress(currentBrightness - mScreenBrightnessDim);
        }
        seekBar.setEnabled(!currentAutomode);
        seekBar.setMax(MAXIMUM_BACKLIGHT - mScreenBrightnessDim);
        seekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // N/A
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // N/A
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromTouch) {
                Xlog.d(TAG, "BrightnessDialog1: onProgressChanged -> " + progress);
                // mEditor.putInt(KEY_BRIGHT_CONFIG, progress + mScreenBrightnessDim);
                // mEditor.commit();
                mPowerSavingManager.setBrightness(progress + mScreenBrightnessDim);
            }
        });
        // /init the dialog view done @}

        AlertDialog brightnessDialog = new AlertDialog.Builder(this).setView(view).setTitle(R.string.brightness).setIcon(
                R.drawable.ic_settings_display).setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mOldBrightnessAutoChecked = autoCheckBox.isChecked();
                        mOldBrightnessProgress = seekBar.getProgress() + mScreenBrightnessDim;
                        mBrightnessPreference.setSummary(getBrightnessPercent((float) seekBar.getProgress()));
                        saveBrightness(mOldBrightnessProgress, mOldBrightnessAutoChecked);
                    }
                }).setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Xlog.d(TAG, "-->brightnessDialog -- onClick Cancel");
                // restore
                mPowerSavingManager.setBrightness(mOldBrightnessProgress);
                mPowerSavingManager.setBrightnessAutoMode(mOldBrightnessAutoChecked ? 1 : 0);
                Xlog.d(TAG, " -- onClick Cancel, old brightness is: " + mOldBrightnessProgress);
                mBrightnessPreference.setSummary(getBrightnessPercent(mOldBrightnessProgress - mScreenBrightnessDim));

                seekBar.setProgress(mOldBrightnessProgress - mScreenBrightnessDim);
                autoCheckBox.setChecked(mOldBrightnessAutoChecked);
                saveBrightness(mOldBrightnessProgress, mOldBrightnessAutoChecked);
            }
        }).setOnCancelListener(new DialogInterface.OnCancelListener() {

            public void onCancel(DialogInterface dialog) {
                Xlog.d(TAG, "-->brightnessDialog -- OnCancel()");
                mOldBrightnessAutoChecked = autoCheckBox.isChecked();
                mOldBrightnessProgress = seekBar.getProgress() + mScreenBrightnessDim;
                saveBrightness(mOldBrightnessProgress, mOldBrightnessAutoChecked);
                mBrightnessPreference
                        .setSummary(getBrightnessPercent((float) mOldBrightnessProgress - mScreenBrightnessDim));
            }
        }).create();
        return brightnessDialog;
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case BRIGHTNESS_DIALOG:
            return createBrightnessDialog();
        case SLEEP_DIALOG:
            int checkedItem = mSharePref.getInt(KEY_SLEEP_PREFERENCE + "_config", 0);
            return new AlertDialog.Builder(this).setSingleChoiceItems(R.array.screen_timeout_entries, checkedItem,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int position) {
                            Xlog.d(TAG, "Position:" + " : " + position);
                            mEditor.putInt(KEY_SLEEP_CONFIG, position);
                            mEditor.commit();
                            mPowerSavingManager.setSleepTime();
                            mSleepPreference.setSummary(getString(R.string.screen_timeout_summary, getResources()
                                    .getStringArray(R.array.screen_timeout_entries)[position]));
                            dialog.dismiss();
                        }
                    }).setPositiveButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int position) {
                    dialog.dismiss();
                }
            }).create();
        default:
            break;
        }
        return null;
    }

    // public void onClick(View imageView) {
    // // TODO Auto-generated method stub
    // // Preference preference = (Preference) imageView.getParent();
    // String key = ((Preference) imageView.getParent()).getKey();
    // if (KEY_BRIGHT_PREFERENCE.equals(key)) {
    // showDialog(BRIGHTNESS_DIALOG);
    // } else if (KEY_SLEEP_PREFERENCE.equals(key)) {
    // showDialog(SLEEP_DIALOG);
    // } else {
    // Xlog.e(TAG, "onClick(),preference key:" + " : " + key);
    // }
    // }
}
