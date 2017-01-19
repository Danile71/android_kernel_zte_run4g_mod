package com.mediatek.blemanager.common;


import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.util.Log;

import java.util.concurrent.ConcurrentHashMap;

import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.bluetooth.BleDeviceManager;
import com.mediatek.bluetooth.BleGattDevice;
import com.mediatek.bluetooth.BleGattDevice.BleGattDeviceCallback;

class CachedBleGattDevice {

    private static final String TAG = BLEConstants.COMMON_TAG + "[CachedBleGattDevice]";
    
    private ConcurrentHashMap<BluetoothDevice, BleGattDevice> mGattDeviceMap;
    private Context mContext;
    
    CachedBleGattDevice(Context context) {
        if (context == null) {
            Log.d(TAG, "[constructor] context is null");
            return;
        }
        mContext = context;
        mGattDeviceMap = new ConcurrentHashMap<BluetoothDevice, BleGattDevice>();
    }
    
    BleGattDevice getBleGattDevice(BleDeviceManager deviceManager, BluetoothDevice device,
            BleGattDeviceCallback callback) {
        if (device == null) {
            Log.d(TAG, "[getBleGattDevice] device is null");
            return null;
        }
        if (mGattDeviceMap == null) {
            Log.e(TAG, "[getBleGattDevice] mGattDeviceMap is null");
            return null;
        }
        if (deviceManager == null) {
            Log.e(TAG, "[getBleGattDevice] deviceManager is null");
            return null;
        }
        BleGattDevice bleDevice = mGattDeviceMap.get(device);
        if (bleDevice == null) {
            bleDevice = deviceManager.createGattDevice(mContext, device, callback);
            mGattDeviceMap.put(device, bleDevice);
        }
        return bleDevice;
    }
    
    void removeDevice(BluetoothDevice device) {
        if (device == null) {
            Log.e(TAG, "[removeDevice] device is null");
            return;
        }
        if (mGattDeviceMap != null) {
            if (mGattDeviceMap.containsKey(device)) {
                mGattDeviceMap.remove(device);
            }
        }
    }
    
    void clearData() {
        if (mGattDeviceMap != null) {
            mGattDeviceMap.clear();
        }
    }
    
}
