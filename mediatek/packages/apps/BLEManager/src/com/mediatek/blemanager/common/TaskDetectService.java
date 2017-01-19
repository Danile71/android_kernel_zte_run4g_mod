package com.mediatek.blemanager.common;


import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.mediatek.blemanager.provider.BLEConstants;

public class TaskDetectService extends Service {

    private static final String TAG = BLEConstants.COMMON_TAG + "[TaskDetectService]";
    
    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        Log.d(TAG, "[onCreate] enter");
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // TODO Auto-generated method stub
        Log.d(TAG, "[onStartCommand] enter");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        // TODO Auto-generated method stub
        // do shutdown service action
        Log.d(TAG, "[onTaskRemoved] enter to close");
        LocalBluetoothLEManager.getInstance(this.getApplicationContext()).close();
        super.onTaskRemoved(rootIntent);
    }

}
