/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.blemanager.common;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattService;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import com.mediatek.blemanager.provider.ActivityDbOperator;
import com.mediatek.blemanager.provider.BLEConstants;

/**
 * CachedBluetoothDeviceManager manages the set of remote Bluetooth devices.
 */
public final class CachedBluetoothLEDeviceManager {
    private static final String TAG = BLEConstants.COMMON_TAG + "[CachedBluetoothLEDeviceManager]";

    private static CachedBluetoothLEDeviceManager sInstance;
    private CopyOnWriteArrayList<CachedBluetoothLEDevice> mCachedDevices =
            new CopyOnWriteArrayList<CachedBluetoothLEDevice>();
    
    private CopyOnWriteArrayList<CachedDeviceListChangedListener> mChangedListeners =
            new CopyOnWriteArrayList<CachedDeviceListChangedListener>();

    private CachedBluetoothLEDeviceManager() {

    }
    
    public static CachedBluetoothLEDeviceManager getInstance() {
        if (sInstance == null) {
            sInstance = new CachedBluetoothLEDeviceManager();
        }
        return sInstance;
    }

    /**
     * on cached device list changed listener such as add, remove
     *
     */
    public interface CachedDeviceListChangedListener {
        void onDeviceAdded(CachedBluetoothLEDevice device);
        void onDeviceRemoved(CachedBluetoothLEDevice device);
    }

    /**
     * register changed callback
     * @param listener
     */
    public void registerDeviceListChangedListener(CachedDeviceListChangedListener listener) {
        if (listener == null) {
            return;
        }
        if (!mChangedListeners.contains(listener)) {
            mChangedListeners.add(listener);
        }
    }
    
    /**
     * unregister changed callback;
     * @param listener
     */
    public void unregisterDeviceListChangedListener(CachedDeviceListChangedListener listener) {
        if (listener == null) {
            return;
        }
        if (mChangedListeners.contains(listener)) {
            mChangedListeners.remove(listener);
        }
    }

    /**
     * notify device has been add to the list
     * @param newDevice
     */
    private void notifyDeviceAdded(CachedBluetoothLEDevice newDevice) {
        if (mChangedListeners != null && mChangedListeners.size() != 0) {
            for (CachedDeviceListChangedListener listener : mChangedListeners) {
                listener.onDeviceAdded(newDevice);
            }
        }
    }
    
    /**
     * notify device has been removed from the list
     * @param device
     */
    private void notifyDeviceRemoved(CachedBluetoothLEDevice device) {
        if (mChangedListeners != null && mChangedListeners.size() != 0) {
            for (CachedDeviceListChangedListener listener : mChangedListeners) {
                listener.onDeviceRemoved(device);
            }
        }
        if (device != null && device.getDevice() != null) {
            ActivityDbOperator.getInstance().deleteDevice(device);
        }
    }

    /**
     * 
     * @return
     */
    public synchronized ArrayList<CachedBluetoothLEDevice> getCachedDevicesCopy() {
        return new ArrayList<CachedBluetoothLEDevice>(mCachedDevices);
    }

    /**
     * 
     * @param device
     * @return
     */
    public CachedBluetoothLEDevice findDevice(BluetoothDevice device) {
        if (device == null) {
            Log.d(TAG, "[findDevice] device is null!!");
            return null;
        }
        if (mCachedDevices.size() == 0) {
            Log.d(TAG, "[findDevice] device list is empty!!");
            return null;
        }
        for (CachedBluetoothLEDevice cachedDevice : mCachedDevices) {
            if (cachedDevice.getDevice().equals(device)) {
                return cachedDevice;
            }
        }
        return null;
    }

    /**
     * 
     * @param disOrder
     * @return
     */
    public CachedBluetoothLEDevice getCachedDeviceFromDisOrder(int disOrder) {
        Log.d(TAG, "[getCachedDeviceFromDisOrder] disOrder : " + disOrder);
        if (mCachedDevices.size() == 0) {
            Log.d(TAG, "[getCachedDeviceFromDisOrder] device list is empty!!");
            return null;
        }
        for (CachedBluetoothLEDevice cachedDevice : mCachedDevices) {
            if (cachedDevice.getDeviceLocationIndex() == disOrder) {
                return cachedDevice;
            }
        }
        return null;
    }

    /**
     * 
     * @param device
     * @param locationIndex
     * @return
     */
    public CachedBluetoothLEDevice addDevice(BluetoothDevice device, int locationIndex) {
        if (mCachedDevices.size() == 4) {
            Log.d(TAG, "[addDevice] device list is full!!");
            return null;
        }
        CachedBluetoothLEDevice cachedDevice = findDevice(device);
        if (cachedDevice != null) {
            Log.d(TAG, "[addDevice] device has been added to de manager," +
                    " just return the cached device");
            return cachedDevice;
        }
        CachedBluetoothLEDevice newDevice = new CachedBluetoothLEDevice(device, locationIndex);
        mCachedDevices.add(newDevice);
        notifyDeviceAdded(newDevice);
        return newDevice;
    }

    /**
     * 
     * @param device
     * @return
     */
    public boolean removeDevice(CachedBluetoothLEDevice device) {
        if (device == null) {
            Log.d(TAG, "[removeDevice] device is null!!");
            return false;
        }
        if (mCachedDevices.size() == 0) {
            Log.d(TAG, "[removeDevice] mCachedDevices size is 0!!");
            return false;
        }
        CachedBluetoothLEDevice cachedDevice = findDevice(device.getDevice());
        if (cachedDevice == null) {
            Log.d(TAG, "[removeDevice] not exist in device list!!");
            return false;
        }
        boolean ret = mCachedDevices.remove(cachedDevice);
        notifyDeviceRemoved(cachedDevice);
        return ret;
    }
    
    public CachedBluetoothLEDevice removeDevice(int disOrder) {
        CachedBluetoothLEDevice device = this.getCachedDeviceFromDisOrder(disOrder);
        if (device == null) {
            return null;
        }
        mCachedDevices.remove(device);
        notifyDeviceRemoved(device);
        return device;
    }
    
    /**
     * Get cached device which supported fmp
     * @return
     */
    public ArrayList<CachedBluetoothLEDevice> getFmpDevices() {
        ArrayList<CachedBluetoothLEDevice> retValue = new ArrayList<CachedBluetoothLEDevice>();
        for (CachedBluetoothLEDevice cachedDevice : mCachedDevices) {
            if (cachedDevice.isSupportFmp()) {
                retValue.add(cachedDevice);
            }
        }
        return retValue;
    }
    
    /**
     * Get cached device which supported pxp
     * @return
     */
    public ArrayList<CachedBluetoothLEDevice> getPxpDevices() {
        ArrayList<CachedBluetoothLEDevice> retValue = new ArrayList<CachedBluetoothLEDevice>();
        for (CachedBluetoothLEDevice cachedDevice : mCachedDevices) {
            if (cachedDevice.isSupportPxpOptional()) {
                retValue.add(cachedDevice);
            }
        }
        return retValue;
    }
    
    /**
     * get cached device which supported ans
     * @return
     */
//    public ArrayList<CachedBluetoothLEDevice> getAnsDevices() {
//        ArrayList<CachedBluetoothLEDevice> retValue = new ArrayList<CachedBluetoothLEDevice>();
//        for(CachedBluetoothLEDevice cachedDevice : mCachedDevices) {
//            if (cachedDevice.isSupportAns()) {
//                retValue.add(cachedDevice);
//            }
//        }
//        return retValue;
//    }
    
    /**
     * 
     * @param device
     * @param state
     */
    void onDeviceConnectionStateChanged(BluetoothDevice device, int state) {
        if (device == null) {
            return;
        }
        CachedBluetoothLEDevice cachedDevice = findDevice(device);
        if (cachedDevice == null) {
            Log.d(TAG, "[onDeviceConnectionStateChanged] cachedDevice is null");
            return;
        }
        Log.d(TAG, "[onDeviceConnectionStateChanged] connect state : " + state);
        cachedDevice.onConnectionStateChanged(state);
    }
    
    /**
     * When remote device name has been changed, and receiver received the name changed
     * broadcast, then will call this method, and update the device name.
     * @param device
     */
    void onDeviceNameChanged(BluetoothDevice device, String name) {
        CachedBluetoothLEDevice cachedDevice = findDevice(device);
        if (cachedDevice == null) {
            return;
        }
        Log.d(TAG, "[onDeviceNameChanged] name : " + name);
        cachedDevice.setDeviceName(name);
    }
    
    /**
     * 
     * @param device
     */
    void onDeviceUuidChanged(BluetoothDevice device) {
        
    }
    
    /**
     * When remote device services changed, then update the service list
     * @param device
     */
    void onDeviceServiceDiscoveried(BluetoothDevice device, List<BluetoothGattService> services) {
        CachedBluetoothLEDevice cachedDevice = findDevice(device);
        if (cachedDevice == null) {
            return;
        }
        // used to update device service list;
        ArrayList<BluetoothGattService> serList = new ArrayList<BluetoothGattService>(services);
        cachedDevice.onServiceDiscovered(serList);
    }

}
