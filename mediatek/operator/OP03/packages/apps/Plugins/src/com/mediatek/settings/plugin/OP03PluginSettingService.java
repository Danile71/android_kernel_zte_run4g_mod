package com.mediatek.op03.plugin;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import com.mediatek.settings.plugin.IOP03SettingsInterface;
import android.content.SharedPreferences;
import android.content.Context;
import com.mediatek.xlog.Xlog;
public class OP03PluginSettingService extends Service {

    SharedPreferences mPref = null;
    private final String prefName = "OP03SettingsPreference";
    private final String PLMN_SETTING_STATE ="plmn_setting_state";
    static final String TAG = "OP03PluginSettingService";
    @Override
    public void onCreate() {
    super.onCreate();
    mPref = getSharedPreferences(prefName,Context.MODE_WORLD_WRITEABLE);
    Xlog.d(TAG,"service created");
    }
	
    @Override
    public IBinder onBind(Intent arg0) {
        return mBinder;
    }

    private final IOP03SettingsInterface.Stub mBinder = new IOP03SettingsInterface.Stub() {
        public String getSettingparameter(String key) throws RemoteException {
	    Xlog.d(TAG,"into getSettingparameter");
            String str = mPref.getString(PLMN_SETTING_STATE,"does not exist");
            Xlog.d(TAG,str);
	    return str;
	}
    public String setSettingparameter(String key, String value) throws RemoteException{
        SharedPreferences.Editor editor = mPref.edit();
        Xlog.d(TAG,"into setSettingparameter(String");
        editor.putString(PLMN_SETTING_STATE, value);
        Boolean bool = editor.commit();
	return null;
    }
    };
	
}
