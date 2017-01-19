package com.mediatek.op.wifi;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.UserHandle;
import android.provider.Settings;
import android.text.TextUtils;

import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class WifiFwkExtOP03 extends DefaultWifiFwkExt {
    private static final String TAG = "WifiFwkExtOP03";

    public WifiFwkExtOP03(Context context) {
        super(context);
    }

    public void init() {
         super.init();
    }

    public boolean needRandomSsid() {
        Xlog.d(TAG, "needRandomSsid =yes");
        return true;
    }
    
    public void setCustomizedWifiSleepPolicy(Context context) {    	
    	int sleepPolicy = Settings.Global.getInt(context.getContentResolver(), Settings.Global.WIFI_SLEEP_POLICY, Settings.Global.WIFI_SLEEP_POLICY_NEVER);
    	Xlog.d(TAG, "Before--> setCustomizedWifiSleepPolicy:"+sleepPolicy);
    	if (sleepPolicy == Settings.Global.WIFI_SLEEP_POLICY_NEVER)
     	sleepPolicy = Settings.Global.WIFI_SLEEP_POLICY_NEVER_WHILE_PLUGGED;
     		
     	Settings.Global.putInt(context.getContentResolver(), Settings.Global.WIFI_SLEEP_POLICY, sleepPolicy);
     Xlog.d(TAG, "After--> setCustomizedWifiSleepPolicy is:"+sleepPolicy);
    }
    
}
