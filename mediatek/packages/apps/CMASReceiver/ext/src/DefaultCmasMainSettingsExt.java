
package com.mediatek.cmas.ext;


import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;


public class DefaultCmasMainSettingsExt implements ICmasMainSettingsExt{
	
	private static final String TAG = "CellBroadcastReceiver/DefaultCmasMainSettingsExt";
	
    public DefaultCmasMainSettingsExt(Context context) {
    	//super(context);
    }
    
    public float getAlertVolume(int msgId){
    	Log.d(TAG, "Default getAlertVolume");
    	return 1.0f;
    }
    
    public boolean getAlertVibration(int msgId){
        Log.d(TAG, "Default getAlertVibration");    	
    	return true;
    }
    
    public boolean setAlertVolumeVibrate(int msgId, boolean currentValue){
    	Log.d(TAG, "Default setAlertVolumeVibrate");
    	return currentValue;
    }
    
    public void addAlertSoundVolumeAndVibration(PreferenceActivity prefActivity){
    	Log.d(TAG, "Default addAlertSoundVolumeAndVibration"); 	
	}

}
