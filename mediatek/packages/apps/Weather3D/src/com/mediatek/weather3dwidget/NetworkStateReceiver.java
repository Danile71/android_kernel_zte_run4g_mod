package com.mediatek.weather3dwidget;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class NetworkStateReceiver extends BroadcastReceiver {
    private static final String TAG = "W3D/NetworkStateReceiver";

    public void onReceive(Context context, Intent intent) {
        LogUtil.i(TAG, "Network connectivity change");

        boolean isNetworkAvailable = Util.isNetworkAvailable(context);

        if (isNetworkAvailable) {
            Intent notifyIntent = new Intent(context, UpdateService.class);
            notifyIntent.setAction(WeatherWidgetAction.ACTION_NETWORK_CONNECTED);
            context.startService(notifyIntent);
        }
    }
}
