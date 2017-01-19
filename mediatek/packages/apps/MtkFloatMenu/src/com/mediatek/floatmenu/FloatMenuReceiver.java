package com.mediatek.floatmenu;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.xlog.Xlog;

public class FloatMenuReceiver extends BroadcastReceiver {
    private static final String ACTION = "com.mediatek.wfd.connection";
    private static final String KEY_CONNECTED = "connected";
    private static final String KEY_UIBC_TOUCH_MOUSE = "uibc_touch_mouse";
    private static final String TAG = FloatMenuService.class.getSimpleName();

    public FloatMenuReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Xlog.v(TAG, "receiver: " + action);
        if (ACTION.equals(intent.getAction())) {
            boolean wfdConnected = intent.getIntExtra(KEY_CONNECTED, 0) == 1;
            boolean uibcTouch = intent.getIntExtra(KEY_UIBC_TOUCH_MOUSE, 0) == 1;
            Xlog.v(TAG, "wfd: " + wfdConnected + " uibc: " + uibcTouch);
            Intent i = new Intent(context, FloatMenuService.class);
            i.putExtra(FloatMenuService.KEY_CONNECTION, true);
            if (wfdConnected) {
                if (uibcTouch) {
                    Xlog.v(TAG, "start float menu");
                    context.startService(i);
                    return;
                }
            }
            Xlog.v(TAG, "stop float menu");
            context.stopService(i);
        }

    }
}
