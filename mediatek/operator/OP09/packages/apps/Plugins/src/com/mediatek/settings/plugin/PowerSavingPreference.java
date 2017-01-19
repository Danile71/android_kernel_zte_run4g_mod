package com.mediatek.settings.plugin;

import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

public class PowerSavingPreference extends Preference implements OnCheckedChangeListener {

    private static final String TAG = "Power Savings";

    private Switch mPowerSavingSwitch;
    private PowerSavingManager mPowerSavingManager;

    /**
     * 
     * @param context
     *            com.android.settings
     */
    public PowerSavingPreference(Context context) {
        super(context);
        Xlog.d(TAG, "new PowerSavingPreference");
        setLayoutResource(R.layout.power_saving_mode);
        setTitle(R.string.power_saving_mode);
        Intent intent = new Intent();
        /** Start an activity from other context, need add flags as follow */
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setClassName(context, PowerSavingSettings.class.getName());
        setIntent(intent);
        mPowerSavingManager = PowerSavingManager.getInstance(context);
    }

    @Override
    public View getView(View convertView, ViewGroup parent) {
        Xlog.d(TAG, "PowerSavingPreference getView()");
        View view = super.getView(convertView, parent);
        mPowerSavingSwitch = (Switch) view.findViewById(R.id.power_saving_switch);
        if (mPowerSavingSwitch != null) {
            boolean isHaveChecked = mPowerSavingManager.isPowerSavingModeOn();
            if (isHaveChecked != mPowerSavingSwitch.isChecked()) {
                mPowerSavingSwitch.setChecked(isHaveChecked);
            }
            mPowerSavingSwitch.setOnCheckedChangeListener(this);
        } else {
            Xlog.e(TAG, " -- > Power Saving Switch == null");
        }
        return view;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        boolean isPoSavingChecked = mPowerSavingManager.isPowerSavingModeOn();
        Xlog.d(TAG, "switch onCheckedChanged: " + isChecked + ", " + isPoSavingChecked);
        if (isPoSavingChecked == isChecked) {
            // Don't care this change
            Xlog.i(TAG, "-->Don't care this change -- ");
            return;
        }
        mPowerSavingManager.turnOn(isChecked);
    }
}
