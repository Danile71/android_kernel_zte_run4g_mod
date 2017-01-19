
package com.mediatek.bluetooth;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseIntArray;

import com.mediatek.bluetoothle.anp.IAlertNotificationProfileService;

import java.util.ArrayList;

public class BleAlertNotificationProfileService extends BleProfileService {

    private static final String TAG = "BleProximityProfileService";
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

    public static final String ACTION_REMOTE_CHANGE = "com.mediatek.ble.ans.REMOTE_STATE_CHANGED";
    public static final int CATEGORY_ERROR_VALUE = Integer.MIN_VALUE;
    public static final int CATEGORY_ID_EMAIL = 1;
    public static final int CATEGORY_ID_INCOMING_CALL = 3;
    public static final int CATEGORY_ID_MISSED_CALL = 4;
    public static final int CATEGORY_ID_SMS = 5;
    public static final int CATEGORY_VAULE_ALL_ALERT_DISABLED = 0x00;
    public static final int CATEGORY_VALUE_NEW_ALERT_ENABLED = 0x01;
    public static final int CATEGORY_VALUE_UNREAD_ALERT_ENABLED = 0x02;
    public static final int CATEGORY_VALUE_ALL_ALERT_ENABLED = CATEGORY_VALUE_NEW_ALERT_ENABLED
            | CATEGORY_VALUE_UNREAD_ALERT_ENABLED;

    private Context mContext;
    private ProfileServiceListener mServiceListener;
    private IAlertNotificationProfileService mService;

    private final ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            if (DBG) {
                Log.d(TAG, "Proxy object connected");
            }
            mService = IAlertNotificationProfileService.Stub.asInterface(service);
            if (null != mServiceListener) {
                mServiceListener.onServiceConnected(BleProfile.BLE_PROFILE_ANP,
                        BleAlertNotificationProfileService.this);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            if (DBG) {
                Log.d(TAG, "Proxy object disconnected");
            }
            mService = null;
            if (null != mServiceListener) {
                mServiceListener.onServiceDisconnected(BleProfile.BLE_PROFILE_ANP);
            }
        }
    };

    BleAlertNotificationProfileService(Context ctxt, ProfileServiceListener listener) {
        mContext = ctxt;
        mServiceListener = listener;
        doBind();
    }

    /* package */void close() {
        synchronized (mConnection) {
            if (mService != null) {
                try {
                    mService = null;
                    mContext.unbindService(mConnection);
                } catch (Exception e) {
                    Log.e(TAG, "", e);
                }
            }
        }
        mServiceListener = null;
    }

    boolean doBind() {
        if (VDBG) {
            Log.v(TAG, "doBind");
        }
        Intent intent = new Intent(IAlertNotificationProfileService.class.getName());
        intent.setClassName("com.mediatek.bluetoothle", "com.mediatek.bluetoothle"
                + ".anp.AlertNotificationProfileService");
        if (!mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE)) {
            Log.e(TAG, "Could not bind to ProximityProfileService with " + intent);
            return false;
        }
        return true;
    }

    /**
     * get UI category value setting
     * 
     * @param address the address of BluetoothDevice need get
     * @param categorylist a list of category ID
     * @return key is category ID, value is the category value get from ANS
     */
    public SparseIntArray getDeviceSettings(String address,
            ArrayList<Integer> categorylist) {

        try {
            if (VDBG) {
                Log.v(TAG, "getDeviceSettings:" + address);
            }
            if (categorylist != null) {
                int listSize = categorylist.size();
                int[] categoryArray = new int[listSize];
                for (int i = 0; i < listSize; i++) {
                    categoryArray[i] = categorylist.get(i);
                }
                int[] resultArray = null;
                if (mService != null) {
                    resultArray = mService.getDeviceSettings(address, categoryArray);
                }
                if (resultArray == null) {
                    return null;
                }
                int arraySize = resultArray.length;
                if (listSize != arraySize) {
                    return null;
                }
                SparseIntArray resultSparseIntArray = new SparseIntArray();
                for (int i = 0; i < listSize; i++) {
                    resultSparseIntArray.put(categorylist.get(i), resultArray[i]);
                }
                return resultSparseIntArray;
            }
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return null;
    }

    /**
     * Get the category settings that set by remote device
     * 
     * @param address the address of BluetoothDevice need get
     * @param categorylist a list of category ID
     * @return key is category ID, value is the category value get from ANS
     */
    public SparseIntArray getRemoteSettings(String address,
            ArrayList<Integer> categorylist) {
        try {
            if (VDBG) {
                Log.v(TAG, "getRemoteSettings:" + address);
            }
            if (categorylist != null) {
                int listSize = categorylist.size();
                int[] categoryArray = new int[listSize];
                for (int i = 0; i < listSize; i++) {
                    categoryArray[i] = categorylist.get(i);
                }
                int[] resultArray = null;
                if (mService != null) {
                    resultArray = mService.getRemoteSettings(address, categoryArray);
                }
                if (resultArray == null) {
                    return null;
                }
                int arraySize = resultArray.length;
                if (listSize != arraySize) {
                    return null;
                }
                SparseIntArray resultSparseIntArray = new SparseIntArray();
                for (int i = 0; i < listSize; i++) {
                    resultSparseIntArray.put(categorylist.get(i), resultArray[i]);
                }
                return resultSparseIntArray;
            }
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return null;
    }

    /**
     * update UI category value setting
     * 
     * @param address the address of BluetoothDevice need update
     * @param categoryIdValues key is category ID, value is the category value need update to ANS
     * @return updating is successful
     */
    public boolean updateDeviceSettings(String address, SparseIntArray categoryIdValues) {

        try {
            if (VDBG) {
                Log.v(TAG, "updateDeviceSetting: " + address);
            }
            if (categoryIdValues != null) {
                int size = categoryIdValues.size();
                int[] categoryArray = new int[size];
                int[] valueArray = new int[size];
                for (int i = 0; i < size; i++) {
                    categoryArray[i] = categoryIdValues.keyAt(i);
                    valueArray[i] = categoryIdValues.valueAt(i);
                }
                if (mService != null) {
                    return mService.updateDeviceSettings(address, categoryArray, valueArray);
                } else {
                    return false;
                }
            }
        } catch (RemoteException e) {
            Log.v(TAG, e.toString());
        }
        return false;
    }
}
