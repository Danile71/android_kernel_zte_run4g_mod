package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.PreferenceGroup;
import android.util.AttributeSet;

public class NormalPreferenceGroup extends PreferenceGroup {
    public NormalPreferenceGroup(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public NormalPreferenceGroup(Context context, AttributeSet attrs) {
        super(context, attrs, com.android.internal.R.attr.preferenceStyle);
    }

    public NormalPreferenceGroup(Context context) {
        this(context, null);
    }
}
