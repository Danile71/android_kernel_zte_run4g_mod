/*
 * Copyright (C) 2011 The Android Open Source Project
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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.bluetooth.BleAlertNotificationProfileService;

//import com.mediatek.bluetooth.BleProximityProfileService;

/**
 * BluetoothEventManager receives broadcasts and callbacks from the Bluetooth
 * API and dispatches the event on the UI thread to the right class in the
 * Settings.
 */
final class BluetoothLEEventManager {
    private static final String TAG = BLEConstants.COMMON_TAG + "[BluetoothLEEventManager]";

    private final IntentFilter mAdapterIntentFilter;
    private final Map<String, Handler> mHandlerMap;
    private Context mContext;
    private LocalBluetoothLEManager mManager;
    private CachedBluetoothLEDeviceManager mDeviceManager;

    private static final String[] ANS_CONFIGURATION_PROJECTION = {
            BLEConstants.ANS_CONFIGURATION.ANS_HOST_CALL_ALERT,
            BLEConstants.ANS_CONFIGURATION.ANS_HOST_MISSED_CALL_ALERT,
            BLEConstants.ANS_CONFIGURATION.ANS_HOST_SMSMMS_ALERT,
            BLEConstants.ANS_CONFIGURATION.ANS_HOST_EMAIL_ALERT,

            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_CALL_ALERT,
            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_EMAIL_ALERT,
            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_MISSED_CALL_ALERT,
            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_SMSMMS_ALERT
    };

    interface Handler {
        void onReceive(Context context, Intent intent, BluetoothDevice device);
    }

    void addHandler(String action, Handler handler) {
        mHandlerMap.put(action, handler);
        mAdapterIntentFilter.addAction(action);
    }

    BluetoothLEEventManager(Context context, LocalBluetoothLEManager manager,
            CachedBluetoothLEDeviceManager deviceManager) {
        mAdapterIntentFilter = new IntentFilter();
        mHandlerMap = new HashMap<String, Handler>();
        mContext = context;
        mManager = manager;
        mDeviceManager = deviceManager;

        // Bluetooth on/off broadcasts
        addHandler(BluetoothAdapter.ACTION_STATE_CHANGED, new AdapterStateChangedHandler());

        // Discovery broadcasts
        addHandler(BluetoothAdapter.ACTION_DISCOVERY_STARTED, new ScanningStateChangedHandler(true));
        addHandler(BluetoothAdapter.ACTION_DISCOVERY_FINISHED, new ScanningStateChangedHandler(
                false));
        // addHandler(BluetoothDevice.ACTION_FOUND, new DeviceFoundHandler());
        // addHandler(BluetoothDevice.ACTION_DISAPPEARED, new
        // DeviceDisappearedHandler());
        addHandler(BluetoothDevice.ACTION_NAME_CHANGED, new NameChangedHandler());

        // Pairing broadcasts
        addHandler(BluetoothDevice.ACTION_BOND_STATE_CHANGED, new BondStateChangedHandler());
        addHandler(BleAlertNotificationProfileService.ACTION_REMOTE_CHANGE,
                new AnsRemoteDataChangedHandler());

        // addHandler(BluetoothDevice.ACTION_PAIRING_CANCEL, new
        // PairingCancelHandler());

        // Fine-grained state broadcasts
        // addHandler(BluetoothDevice.ACTION_CLASS_CHANGED, new
        // ClassChangedHandler());
        // addHandler(BluetoothDevice.ACTION_UUID, new UuidChangedHandler());

        mContext.registerReceiver(mBroadcastReceiver, mAdapterIntentFilter);

    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.v(TAG, "Received " + intent.getAction());
            String action = intent.getAction();
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            Handler handler = mHandlerMap.get(action);
            if (handler != null) {
                handler.onReceive(context, intent, device);
            }
        }
    };

    private class AdapterStateChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
            Log.d(TAG, "[AdapterStateChangedHandler] state : " + state);
            mManager.onAdapterStateChanged(state);
        }
    }

    private class ScanningStateChangedHandler implements Handler {
        private final boolean mStarted;

        ScanningStateChangedHandler(boolean started) {
            mStarted = started;
        }

        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            mManager.onAdapterScanningStateChanged(mStarted);
            Log.d(TAG, "scanning state change to " + mStarted);
        }
    }

    private class DeviceDisappearedHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            mManager.onDeviceDeleted(device);
        }
    }

    private class NameChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            String name = intent.getStringExtra(BluetoothDevice.EXTRA_NAME);
            if (mDeviceManager.findDevice(device) == null) {
                Log.d(TAG, "[NameChangedHandler] device : " + device + ", name : " + name);
                mManager.onScannedDeviceNameChanged(device, name);
            } else {
                mDeviceManager.onDeviceNameChanged(device, name);
            }
        }
    }

    private class BondStateChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            if (device == null) {
                Log.e(TAG, "ACTION_BOND_STATE_CHANGED with no EXTRA_DEVICE");
                return;
            }
            int bondState = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                    BluetoothDevice.ERROR);
            Log.d(TAG, "[BondStateChangedHandler] bondState : " + bondState);
            Log.d(TAG, "[BondStateChangedHandler] device : " + device.getAddress());
            // if (bondState == BluetoothDevice.BOND_NONE) {
            // Log.d(TAG,
            // "[BondStateChangedHandler] start to disconnect device");
            // mManager.disconnectGattDevice(device);
            // }
        }
    }

    // private class ClassChangedHandler implements Handler {
    // public void onReceive(Context context, Intent intent,
    // BluetoothDevice device) {
    // mDeviceManager.onBtClassChanged(device);
    // }
    // }

    private class UuidChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            mDeviceManager.onDeviceUuidChanged(device);
        }
    }

    private class PairingCancelHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            if (device == null) {
                Log.e(TAG, "ACTION_PAIRING_CANCEL with no EXTRA_DEVICE");
                return;
            }
            // int errorMsg = R.string.bluetooth_pairing_error_message;
            // CachedBluetoothLEDevice cachedDevice =
            // mDeviceManager.findDevice(device);
            // Utils.showError(context, cachedDevice.getName(), errorMsg);
        }
    }

    private class AnsRemoteDataChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            if (device == null) {
                Log.d(TAG, "[AnsRemoteDataChangedHandler] device is null");
                return;
            }
            
//            String address = intent.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
//            if (address == null || address.trim().length() == 0) {
//                Log.d(TAG, "[AnsRemoteDataChangedHandler] bt address is wrong");
//                return;
//            }
//            BluetoothDevice bdevice =
//                    BluetoothAdapter.getDefaultAdapter().getRemoteDevice(address);
//            if (bdevice == null) {
//                Log.d(TAG, "[AnsRemoteDataChangedHandler] device is null");
//                return;
//            }
            CachedBluetoothLEDevice cachedDevice = mDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                Log.d(TAG, "[AnsRemoteDataChangedHandler] bdevice is contained in cacher");
                return;
            }
            mManager.updateAnpData(cachedDevice);
        }
    }

    private void updateAnsSettings(final BluetoothDevice device) {
        if (device == null) {
            Log.d(TAG, "[updateAnsSettings] device is null");
            return;
        }
        final CachedBluetoothLEDevice cachedDevice = this.mDeviceManager.findDevice(device);
        if (cachedDevice == null) {
            Log.d(TAG, "[updateAnsSettings] cachedDevice is null");
            return;
        }
        Runnable r = new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                String selection = BLEConstants.COLUMN_BT_ADDRESS + "='" + device.getAddress()
                        + "'";
                Cursor cursor = mContext.getContentResolver().query(BLEConstants.TABLE_ANS_URI,
                        ANS_CONFIGURATION_PROJECTION, selection, null, null);
                if (cursor == null) {
                    Log.d(TAG, "[updateAnsSettings] cursor is null");
                    return;
                }
                if (cursor.getCount() == 0) {
                    cursor.close();
                    Log.d(TAG, "[updateAnsSettings] cursor count is 0");
                    return;
                }
                cursor.moveToFirst();
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_HOST_CALL_ALERT))));
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_HOST_MISSED_CALL_ALERT))));
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_HOST_SMSMMS_ALERT))));
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_HOST_EMAIL_ALERT))));

                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_REMOTE_INCOMING_CALL_FLAGE,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_CALL_ALERT))));
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_REMOTE_MISSED_CALL_FLAGE,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_MISSED_CALL_ALERT))));
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_EMAIL_FLAGE,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_EMAIL_ALERT))));
                cachedDevice
                        .setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_MESSAGE_FLAGE,
                                getBoolean(cursor.getInt(cursor
                                        .getColumnIndex(BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_SMSMMS_ALERT))));

                cursor.close();
            }

        };
        new Thread(r).start();
    }

    private boolean getBoolean(int it) {
        if (it == 0) {
            return false;
        }
        return true;
    }
}
