package com.mediatek.settings.plugin;

import android.content.Context;
import android.util.AttributeSet;

/**
 * Used in ManualNetworkSelection to show GSM available network as a list
 * 
 */
public class CarrierRadioPreference extends RadioPreference {

    private static final String TAG = "RadioButtonPreference";

    private String mCarrierNumeric;
    private int mCarrierRate;

    public CarrierRadioPreference(Context context) {
        super(context, "title", "summary");
    }

    public CarrierRadioPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CarrierRadioPreference(Context context, String title, String summary) {
        super(context, title, summary, false);
    }

    public CarrierRadioPreference(Context context, String title, String summary, boolean isChecked) {
        super(context);
    }

    public void setCarrierNumeric(String numeric) {
        mCarrierNumeric = numeric;
    }

    public String getCarrierNumeric() {
        return mCarrierNumeric;
    }

    public void setCarrierRate(int rate) {
        mCarrierRate = rate;
    }

    public int getCarrierRate() {
        return mCarrierRate;
    }
}
