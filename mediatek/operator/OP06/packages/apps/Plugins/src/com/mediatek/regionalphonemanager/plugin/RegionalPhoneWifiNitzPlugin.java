package com.mediatek.regionalphonemanager.plugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.provider.Settings;

import com.mediatek.xlog.Xlog;
import com.mediatek.rpm.ext.ISettingsExt;
import com.mediatek.common.regionalphone.RegionalPhone;

public class RegionalPhoneWifiNitzPlugin implements ISettingsExt {
    private static final String TAG = "RegionalPhone";
    private Context mContext;	
    private Uri mUri = RegionalPhone.SETTINGS_URI;
    private static final String NITZ_AUTOUPDATE = "NITZAutoUpdate";
    private static final String WIFI_DEFAULT = "wifi";
    
    public void updateConfiguration(Context context) {
        mContext = context;
        Xlog.d(TAG, "RegionalPhoneWifiNitzPlugin");
        dataUpdate();
    }    
 
    /**
     * update configuration of NITZ and Wi-Fi 
     */
    private void dataUpdate(){  	
    	Cursor mCursor = mContext.getContentResolver().query(
    			mUri,
    			null,
    			null,
    			null,
    			null
    			);
    	if(null == mCursor){
    		Xlog.d(TAG, "Cursor == null");
    	}else if(mCursor.getCount()<1){
    		Xlog.d(TAG, "No data found");
    	}else{
    		mCursor.moveToNext();
    		int NITZEnabled = mCursor.getInt(mCursor.getColumnIndex(NITZ_AUTOUPDATE));
    		int wifiEnabled = mCursor.getInt(mCursor.getColumnIndex(WIFI_DEFAULT));
        	setNITZEnabler(NITZEnabled);
            setWifiEnabler(wifiEnabled);
    	}
	mCursor.close();
    }
    
    /**
     * Enable/Disable NITZ function
     * @param enabled
     * 0: Disable
     * 1: Enable
     */
    private void setNITZEnabler(int enabled){
    	Xlog.d(TAG, "setNITZ, new value is: "+enabled);
        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.AUTO_TIME, enabled);
        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.AUTO_TIME_GPS, 0);
    
    }
    
    /**
     * Enable/Disable Wi-Fi function
     * @param enabled
     * 0: Disable
     * 1: Enable
     */
    private void setWifiEnabler(int enabled){ 
    	boolean newEnabled = (enabled!=0 ? true:false);
    	WifiManager mWifiManager = (WifiManager) mContext.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager.setWifiEnabled(newEnabled)) {
            Xlog.d(TAG, "set Wifi success, new value is: "+ enabled);            
        } else {
        	Xlog.d(TAG, "set Wifi fail");
        }    	
    }
   
}
