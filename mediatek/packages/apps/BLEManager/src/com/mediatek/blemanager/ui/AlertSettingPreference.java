
package com.mediatek.blemanager.ui;



import android.app.ActionBar;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.Toast;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.blemanager.ui.IconSeekBarPreference.OnSeekBarProgressChangedListener;

public class AlertSettingPreference extends PreferenceActivity {

    private static final String TAG = BLEConstants.COMMON_TAG + "[AlertSettingPreference]";

    private static final String RANGE_ALERT_CHECK_PREFERENCE = "range_alert_check_preference";
    private static final String RINGTONE_PREFERENCE = "ringtone_preference";
    private static final String DISCONNECT_WARNING_PREFERENCE = "disconnect_warning_preference";
    private static final String VIBRATION_PREFERENCE = "vibration_preference";
    private static final String VOLUME_SEEK_BAR_PREFERENCE = "volume_seek_bar_preference";
    private static final String RANGE_ALERT_COMPOSE_PREFERENCE = "range_alert_compose_preference";
    
    private static final String INTENT_EXTRA_CURRENT_DEVICE = "current_device";
    
    private static final int REQUEST_CODE_SELLECT_RINGTONE = 1;
    
    private NonChangeCheckBoxPreference mRingtonePreference;
    private CheckBoxPreference mDisWarningPreference;
    private CheckBoxPreference mRangeAlertPreference;
    private CheckBoxPreference mVibrationPreference;
    private IconSeekBarPreference mSeekBarPreference;
    private RangeComposePreference mComposePreference;

    private SeekBar mSeekBar;
    
    private Switch mAlertSwitch;
    
    private boolean mShowInforDialog = false;
    
    private CachedBluetoothLEDeviceManager mDeviceManager;
    private CachedBluetoothLEDevice mCachedDevice;
    
    private int mCurrent = -1;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = this.getIntent();
        mCurrent = intent.getIntExtra(INTENT_EXTRA_CURRENT_DEVICE, -1);
        if (mCurrent == -1) {
            Toast.makeText(this, "device is wrong", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        mCachedDevice = mDeviceManager.getCachedDeviceFromDisOrder(mCurrent);
        if (mCachedDevice == null) {
            finish();
            return;
        }
        
        addPreferencesFromResource(R.xml.alert_setting_preference);

        mRangeAlertPreference =
            (CheckBoxPreference)this.findPreference(RANGE_ALERT_CHECK_PREFERENCE);
        mRingtonePreference = (NonChangeCheckBoxPreference)this.findPreference(RINGTONE_PREFERENCE);
        mDisWarningPreference =
            (CheckBoxPreference)this.findPreference(DISCONNECT_WARNING_PREFERENCE);
        mVibrationPreference = (CheckBoxPreference)this.findPreference(VIBRATION_PREFERENCE);
        mSeekBarPreference = (IconSeekBarPreference)this.findPreference(VOLUME_SEEK_BAR_PREFERENCE);
        mComposePreference =
            (RangeComposePreference)this.findPreference(RANGE_ALERT_COMPOSE_PREFERENCE);
        mCachedDevice.registerAttributeChangeListener(mListener);
        initPreferences();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onStart() {
        super.onStart();
        initActivityState();
        initActionBar();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            if (requestCode == REQUEST_CODE_SELLECT_RINGTONE) {
                Uri uri = data.getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
                if (uri == null) {
                    Log.d(TAG, "[onActivityResult] uri is null");
                    return;
                }
                Log.d(TAG, "[onActivityResult] uri : " + uri);
                Ringtone r = RingtoneManager.getRingtone(this, uri);
                String title = r.getTitle(this);
                Log.d(TAG, "[onActivityResult] r : " + r);
                Log.d(TAG, "[onActivityResult] title : " + title);
                mCachedDevice.setRingtoneUri(uri);
                String summ = RingtoneManager.getRingtone(this,
                        mCachedDevice.getRingtoneUri()).getTitle(this);
                mRingtonePreference.setSummary(summ);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        if (mComposePreference != null) {
            mComposePreference.clear();
        }
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        return true;
    }
    
    private void initActionBar() {
        ActionBar bar = this.getActionBar();
        Uri uri = mCachedDevice.getDeviceImage();//mDeviceManager.getDeviceImage(mCurrent);
//        bar.setIcon(ActivityUtils.getDrawableIcon(mCachedDevice.getDeviceTagId()));
        bar.setTitle(mCachedDevice.getDeviceName());
        View v = LayoutInflater.from(this).inflate(R.layout.alert_setting_action_bar_switch, null);
        bar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        bar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT,
                Gravity.CENTER_VERTICAL | Gravity.RIGHT));
        mAlertSwitch = (Switch)v.findViewById(R.id.alert_setting_menu_switch);
        mAlertSwitch.setChecked(mCachedDevice.getBooleanAttribute(
                CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG));
        mAlertSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
                mCachedDevice.setBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG, arg1);
                updatePreferences();
            }
        });
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
        case android.R.id.home:
            this.finish();
            break;
        default:
            break;
        }
        return true;
    }

    /**
     * do preference initialization about click listeners.
     */
    private void initPreferences() {
        mSeekBarPreference.setOnProgressChanged(new OnSeekBarProgressChangedListener() {
            @Override
            public void onProgressChanged(final int progress) {
                AlertSettingPreference.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mCachedDevice.setIntAttribute(
                                CachedBluetoothLEDevice.DEVICE_VOLUME_FLAG, progress);
                    }
                });
            }
        });
        mRangeAlertPreference.setOnPreferenceClickListener(mPreferenceClickListener);
        mDisWarningPreference.setOnPreferenceClickListener(mPreferenceClickListener);
        mVibrationPreference.setOnPreferenceClickListener(mPreferenceClickListener);
        mComposePreference.setChangeListener(new ComposeListener());

        mRingtonePreference.setOnPreferenceClickListener(mPreferenceClickListener);
        mRingtonePreference.setCheckStateChangeListener(
                mCachedDevice.getBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG),
                mRingtoneCheckStateChangeListener);
    }

    private void initActivityState() {
        Intent intent = getIntent();
        updatePreferences();
    }
    
    private void updatePreferences() {
        boolean checked = mCachedDevice.getBooleanAttribute(
                CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG);
        if (checked) {
            mDisWarningPreference.setEnabled(true);
            mVibrationPreference.setEnabled(true);
            if (mCachedDevice.isSupportPxpOptional()) {
                mRangeAlertPreference.setEnabled(true);
            } else {
                mRangeAlertPreference.setEnabled(false);
            }
            mRingtonePreference.setEnabled(true);
            mSeekBarPreference.setEnabled(true);
        } else {
            mDisWarningPreference.setEnabled(false);
            mVibrationPreference.setEnabled(false);
            mRangeAlertPreference.setEnabled(false);
            mRingtonePreference.setEnabled(false);
            mSeekBarPreference.setEnabled(false);
        }
        mRangeAlertPreference.setChecked(
                mCachedDevice.getBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG));
        mDisWarningPreference.setChecked(
                mCachedDevice.getBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_DISCONNECTION_WARNING_EANBLER_FLAG));
        Ringtone r = RingtoneManager.getRingtone(this, mCachedDevice.getRingtoneUri());
        mRingtonePreference.setSummary(r.getTitle(this));
        mSeekBarPreference.setProgress(
                mCachedDevice.getIntAttribute(CachedBluetoothLEDevice.DEVICE_VOLUME_FLAG));
        mVibrationPreference.setChecked(
                mCachedDevice.getBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_VIBRATION_ENABLER_FLAG));
        if (checked && mRangeAlertPreference.isChecked()) {
            mComposePreference.setEnabled(true);
        } else {
            mComposePreference.setEnabled(false);
        }
        mComposePreference.setState(mComposePreference.isEnabled(),
                mCachedDevice.getIntAttribute(CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG),
                mCachedDevice.getIntAttribute(
                        CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG));
    }
    
    private class ComposeListener
        implements RangeComposePreference.ComposePreferenceChangedListener {

        @Override
        public void onSeekBarProgressChanged(final int startPorgress, final int stopProgress) {
            AlertSettingPreference.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "[onSeekBarProgressChanged] startPorgress : " + startPorgress
                            + ", stopProgress : " + stopProgress);
                    if (startPorgress == stopProgress) {
                        Log.d(TAG, "[onSeekBarProgressChanged] progress not changed!!");
                        return;
                    }
                    mCachedDevice.setIntAttribute(
                            CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG, stopProgress);
                }
            });
        }

        @Override
        public void onRangeChanged(final boolean outRangeChecked) {
            AlertSettingPreference.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "[onRangeChanged] outRangeChecked : " + outRangeChecked);
                    if (!mCachedDevice.getBooleanAttribute(
                            CachedBluetoothLEDevice.DEVICE_RANGE_INFO_DIALOG_ENABELR_FLAG)) {
                        showInformationDialog();
                    }
                    int outChecked;
                    if (outRangeChecked) {
                        outChecked = CachedBluetoothLEDevice.OUT_OF_RANGE_ALERT_VALUE;
                    } else {
                        outChecked = CachedBluetoothLEDevice.IN_RANGE_ALERT_VALUE;
                    }
                    mCachedDevice.setIntAttribute(
                            CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG, outChecked);
                }
            });
        }
    }
    
    private Preference.OnPreferenceClickListener mPreferenceClickListener =
        new Preference.OnPreferenceClickListener() {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            if (preference.getKey().equals(RANGE_ALERT_CHECK_PREFERENCE)) {
                mCachedDevice.setBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG,
                        mRangeAlertPreference.isChecked());
                if (mCachedDevice.getBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG)) {
                    if (mCachedDevice.getBooleanAttribute(
                            CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG)) {
                        mComposePreference.setEnabled(true);
                    } else {
                        mComposePreference.setEnabled(false);
                    }
                } else {
                    mComposePreference.setEnabled(false);
                }
                mComposePreference.setState(mComposePreference.isEnabled(),
                        mCachedDevice.getIntAttribute(
                                CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG),
                        mCachedDevice.getIntAttribute(
                                CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG));
            }
            if (preference.getKey().equals(DISCONNECT_WARNING_PREFERENCE)) {
                mCachedDevice.setBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_DISCONNECTION_WARNING_EANBLER_FLAG,
                        mDisWarningPreference.isChecked());
            }
            if (preference.getKey().equals(VIBRATION_PREFERENCE)) {
                mCachedDevice.setBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_VIBRATION_ENABLER_FLAG,
                        mVibrationPreference.isChecked());
            }
            if (preference.getKey().equals(RINGTONE_PREFERENCE)) {
                showRingtoneSelector();
            }
            return true;
        }
    };
    
    private NonChangeCheckBoxPreference.OnCheckStateChangeListener
        mRingtoneCheckStateChangeListener =
            new NonChangeCheckBoxPreference.OnCheckStateChangeListener() {
                @Override
                public void onCheckdChangeListener(boolean checked) {
                    // TODO Auto-generated method stub
                    mCachedDevice.setBooleanAttribute(
                            CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG, checked);
                    mRingtonePreference.setCheckState(
                            mCachedDevice.getBooleanAttribute(
                                    CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG));
                }
    };

    private void showInformationDialog() {
        View v = LayoutInflater.from(this).inflate(R.layout.range_information_dialog_layout, null);
        final CheckBox cb = (CheckBox)v.findViewById(R.id.range_information_dialog_check_box);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setView(v);
        builder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (cb.isChecked()) {
                    mCachedDevice.setBooleanAttribute(
                            CachedBluetoothLEDevice.DEVICE_RANGE_INFO_DIALOG_ENABELR_FLAG, true);
                }
            }
        });
        builder.create().show();
    }
    
    private void showRingtoneSelector() {
        Intent intent = new Intent(RingtoneManager.ACTION_RINGTONE_PICKER);
        intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_DEFAULT, false);
        intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_SILENT, false);
        intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TYPE, RingtoneManager.TYPE_ALARM);
        intent.putExtra(RingtoneManager.EXTRA_RINGTONE_EXISTING_URI,
                mCachedDevice.getRingtoneUri());
        this.startActivityForResult(intent, REQUEST_CODE_SELLECT_RINGTONE);
    }
    
    private CachedBluetoothLEDevice.DeviceAttributeChangeListener mListener = 
            new CachedBluetoothLEDevice.DeviceAttributeChangeListener() {
                @Override
                public void onDeviceAttributeChange(CachedBluetoothLEDevice device,
                        int which) {
                    // TODO Auto-generated method stub
                }
            };

}
