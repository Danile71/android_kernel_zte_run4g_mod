package com.mediatek.blemanager.ui;

import android.content.Context;
import android.preference.SwitchPreference;
import android.util.AttributeSet;

public class NonChangeSwitchPreference extends SwitchPreference {

    public NonChangeSwitchPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onClick() {
        
    }

}
