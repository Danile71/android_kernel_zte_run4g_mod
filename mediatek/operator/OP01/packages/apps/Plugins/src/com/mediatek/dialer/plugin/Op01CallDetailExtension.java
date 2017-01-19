package com.mediatek.dialer.plugin;

import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.mediatek.dialer.ext.DefaultCallDetailExtension;

public class Op01CallDetailExtension extends DefaultCallDetailExtension {
    private static final String TAG = "Op01CallDetailExtension";

    /**
     * for op01 
     * @param durationView the duration text
     */
    @Override
    public void setDurationViewVisibility(TextView durationView) {
        Log.i(TAG, "setDurationViewVisibility : GONE");
        durationView.setVisibility(View.GONE);
    }

}
