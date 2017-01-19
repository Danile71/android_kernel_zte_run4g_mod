package com.mediatek.deskclock.plugin;

import android.content.Context;
import android.util.Log;

import com.mediatek.deskclock.ext.DefaultAlarmControllerExt;

/**
 * M: OP implementation of Plug-in definition of Desk Clock.
 */
public class Op01AlarmControllerExt extends DefaultAlarmControllerExt {

    private static final String TAG = "Op01AlarmControllerExt";

    @Override
    public void vibrate(Context context) {
        Log.v(TAG, "Do not vibrate when in call state");
    }
}