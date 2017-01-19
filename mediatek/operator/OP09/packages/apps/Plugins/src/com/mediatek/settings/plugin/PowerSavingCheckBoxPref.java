package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.CheckBoxPreference;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

/**
 * 
 * 
 * Description:
 * <p>
 * Customization for CT,Power Saving Mode
 * <p>
 * <p>
 * To control general items: CPU, WLAN, BT, GPS, Sync, Feedback
 * <p>
 * 
 */
public class PowerSavingCheckBoxPref extends CheckBoxPreference {

    private static final String TAG = "Power Savings";

    public PowerSavingCheckBoxPref(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.power_saving_checkbox_item);
        // setWidgetLayoutResource(R.layout.preference_inputmethod_widget);
        Xlog.d(TAG, "--PowerSavingCheckBoxPref  new attrs");
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        Xlog.d(TAG, "--PowerSavingCheckBoxPref  onBindView");
        View poSavingItem = view.findViewById(R.id.power_saving_checkbox_pref);
        poSavingItem.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if (!isEnabled()) {
                    return;
                }
                setChecked(!isChecked());
                if (isChecked()) {
                    /** Turn off/on related functions through current preference key */
                    PowerSavingManager.getInstance(getContext()).turnOnPowerSavingMode(getKey());
                }
            }
        });
    }
}
