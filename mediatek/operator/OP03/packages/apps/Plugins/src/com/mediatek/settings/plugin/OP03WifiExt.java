package com.mediatek.settings.plugin;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.preference.ListPreference;
import android.provider.Settings;
import android.provider.Settings.Global;
import com.mediatek.settings.ext.DefaultWifiExt;
import com.mediatek.xlog.Xlog;


public class OP03WifiExt extends DefaultWifiExt {

	private static final String TAG = "WifiExt";
    
	public OP03WifiExt(Context context){
		super(context);
    }
 		
	public void setSleepPolicyPreference(ListPreference sleepPolicyPref, String[] entriesArray, String[] valuesArray) {		
		String[] newEntries = {entriesArray[1], entriesArray[2]};
		String[] newValues = {valuesArray[1],valuesArray[2]};
			
		sleepPolicyPref.setEntries(newEntries);
		sleepPolicyPref.setEntryValues(newValues);
		Xlog.d(TAG,"setSleepPolicyPreference");
	}
}

