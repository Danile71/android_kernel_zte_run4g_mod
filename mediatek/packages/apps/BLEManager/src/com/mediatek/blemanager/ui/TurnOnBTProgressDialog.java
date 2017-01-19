package com.mediatek.blemanager.ui;



import android.app.Activity;
import android.app.ProgressDialog;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;

public class TurnOnBTProgressDialog {

    private static final String TAG = BLEConstants.COMMON_TAG + "[TurnOnBTProgressDialog]";
    
    private static final int SHOW_TIMEOUT = 30 * 1000; // 30s
    
    private static final int SHOW_DIALOG = 1;
    private static final int DISMISS_DIALOG = 2;
    private static final int SHOW_TIMEOUT_FLAG = 3;
    
    private ProgressDialog mProDialog;
    private static TurnOnBTProgressDialog sInstance;
    private LocalBluetoothLEManager mLocalManager;
    
    private TurnOnBTProgressDialog(Activity activity) {
        mLocalManager = LocalBluetoothLEManager.getInstance(activity);
    }
    
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            int what = msg.what;
            switch (what) {
            case SHOW_DIALOG:
                Activity activity = (Activity)msg.obj;
                String str = activity.getResources().getString(R.string.turning_on_bluetooth);
                mProDialog = ProgressDialog.show(activity, "", str);
                mProDialog.setCancelable(false);
                
                Message msg1 = this.obtainMessage();
                msg1.what = SHOW_TIMEOUT_FLAG;
                mHandler.sendMessageDelayed(msg1, SHOW_TIMEOUT);
                break;
                
            case DISMISS_DIALOG:
                if (mProDialog != null && mProDialog.isShowing()) {
                    mProDialog.setCancelable(true);
                    mProDialog.dismiss();
                }
                break;
                
            case SHOW_TIMEOUT_FLAG:
                if (mProDialog != null && mProDialog.isShowing()) {
                    mProDialog.setCancelable(true);
                    mProDialog.dismiss();
                }
                mLocalManager.turnOffBluetooth();
                break;
                
                default:
                    Log.d(TAG, "mHandler unknown id");
                    return;
            }
        }
        
    };
    
    public static void show(Activity activity) {
        if (activity == null) {
            Log.d(TAG, "[show] activity is null");
            return;
        }
        if (sInstance == null) {
            sInstance = new TurnOnBTProgressDialog(activity);
        }
        
        sInstance.mLocalManager.turnOnBluetooth();
        
        Message msg = sInstance.mHandler.obtainMessage();
        msg.what = SHOW_DIALOG;
        msg.obj = activity;
        sInstance.mHandler.sendMessage(msg);
        
        
    }
    
    public static void dismiss() {
        if (sInstance != null) {
            Message msg = sInstance.mHandler.obtainMessage();
            msg.what = DISMISS_DIALOG;
            sInstance.mHandler.sendMessage(msg);
            sInstance.mHandler.removeMessages(SHOW_TIMEOUT_FLAG);
        }
    }
    
}
