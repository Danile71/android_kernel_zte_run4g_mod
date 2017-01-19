package com.opera.max.loader;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

public class BootReceiver extends BroadcastReceiver {

    public static final String mSavingPkgName = "com.opera.max.loader";
    private static final String mSavingServiceName = "com.opera.max.loader.LoaderService";





    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            Intent intent2 = new Intent();
            intent2.setClassName(mSavingPkgName, mSavingServiceName);
            context.startService(intent2);
        }
    }
}

