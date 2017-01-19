package com.mediatek.miravision.ui;

import android.content.Context;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.widget.ImageView;

public class ImagePreference extends PreferenceCategory {

    public ImagePreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setLayoutResource(R.layout.preference_image);
    }

    public ImagePreference(Context context) {
        super(context);
        setLayoutResource(R.layout.preference_image);
    }

    public ImagePreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_image);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        setLayoutResource(R.layout.preference_image);
    }
}
