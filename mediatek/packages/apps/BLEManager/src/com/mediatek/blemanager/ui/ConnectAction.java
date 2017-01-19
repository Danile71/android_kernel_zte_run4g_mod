package com.mediatek.blemanager.ui;


import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;

public class ConnectAction {
    
    private static final String TAG = BLEConstants.COMMON_TAG + "[ConnectAction]";
    
    private static final int CONNECT_TIMEOUT = 30 * 1000;  // 30s
    
    private static final int CONNECT_ACTION = 1;
    private static final int DISCONNECT_ACTION = 2;
    private static final int CONNECT_TIMEOUT_OVER = 3;
    private static final int DIALOG_DISMISS = 4;
    
    private Activity mActivity;

    private LocalBluetoothLEManager mLocalManager;
//    private CachedBluetoothLEDevice mCachedDevice;
    private CachedBluetoothLEDeviceManager mDeviceManager;
    
    private boolean mShowDialog = true;
    
    public ConnectAction(Activity activity) {
        if (activity == null) {
            throw new IllegalArgumentException("activity is null");
        }
        mActivity = activity;
        mLocalManager = LocalBluetoothLEManager.getInstance(mActivity);
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        
    }
    
    public void connect(BluetoothDevice device, int localIndex, boolean showDialog) {
        if (device == null) {
            Log.d(TAG, "[connec] device is null");
            return;
        }
        if (localIndex < 0) {
            Log.d(TAG, "[connec] localIndex < 0");
            return;
        }
        
        mHandler.removeMessages(CONNECT_ACTION);
        mHandler.removeMessages(DISCONNECT_ACTION);
        mHandler.removeMessages(CONNECT_TIMEOUT_OVER);
        
        mShowDialog = showDialog;
        
        Message msg = mHandler.obtainMessage();
        msg.what = CONNECT_ACTION;
        msg.obj = device;
        msg.arg2 = localIndex;
        mHandler.sendMessage(msg);
    }
    
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            int what = msg.what;
            switch(what) {
            case CONNECT_ACTION:
                Log.d(TAG, "[handleMessage] handle CONNECT_ACTION");
                BluetoothDevice device1 = (BluetoothDevice)msg.obj;
                doConnectAction(device1, msg.arg2);
                break;
                
            case DISCONNECT_ACTION:
                Log.d(TAG, "[handleMessage] handle DISCONNECT_ACTION");
                BluetoothDevice device2 = (BluetoothDevice)msg.obj;
                doDisconnectAction(device2);
                mHandler.removeMessages(CONNECT_TIMEOUT_OVER);
                break;
                
            case CONNECT_TIMEOUT_OVER:
                Log.d(TAG, "[handleMessage] handle CONNECT_TIMEOUT_OVER");
                BluetoothDevice device3 = (BluetoothDevice)msg.obj;
                doDisconnectAction(device3);
                // do show a toast to notify user connect failed
                String devicename = null;
                CachedBluetoothLEDevice cachedDevice = mDeviceManager.findDevice(device3);
                if (cachedDevice == null) {
                    devicename = device3.getName();
                } else {
                    devicename = cachedDevice.getDeviceName();
                }
                if (devicename != null) {
                    Toast.makeText(mActivity, "Failed to connect " +
                            devicename, Toast.LENGTH_SHORT).show();
                }
                break;
                
            case DIALOG_DISMISS:
                Log.d(TAG, "[handleMessage] handle DIALOG_DISMISS");
                ConnectProgressAlertDialog.dismiss();
                break;
                
                default:
                    Log.d(TAG, "[mHandler] unknown id");
                    return;
            }
        }
        
    };
    
//    private CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener mDeviceListListener =
//            new CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener() {
//                
//                @Override
//                public void onDeviceRemoved(CachedBluetoothLEDevice device) {
//                    // TODO Auto-generated method stub
//                    
//                }
//                
//                @Override
//                public void onDeviceAdded(CachedBluetoothLEDevice device) {
//                    // TODO Auto-generated method stub
//                    Log.d(TAG, "[onDeviceAdded] mCachedDevice : " + mCachedDevice);
//                    if (mCachedDevice == null) {
//                        Log.d(TAG, "enter to dismiss dialog and remove timeout" +
//                                    " message & unregister mDeviceListListener");
//                        Message msg = mHandler.obtainMessage();
//                        msg.what = DIALOG_DISMISS;
//                        mHandler.sendMessage(msg);
//                        mHandler.removeMessages(CONNECT_TIMEOUT_OVER);
//                        mDeviceManager.unregisterDeviceListChangedListener(mDeviceListListener);
//                    }
//                }
//            };
    
//    private CachedBluetoothLEDevice.DeviceAttributeChangeListener mAttributeListener = 
//            new CachedBluetoothLEDevice.DeviceAttributeChangeListener() {
//                
//                @Override
//                public void onDeviceAttributeChange(CachedBluetoothLEDevice device,
//                        int which) {
//                    // TODO Auto-generated method stub
//                    if (mCachedDevice != null) {
//                        if (which == CachedBluetoothLEDevice.DEVICE_CONNECTION_STATE_CHANGE_FLAG) {
//                            if (device.getConnectionState() != BluetoothGatt.STATE_CONNECTING &&
//                                    device.getConnectionState() != BluetoothGatt.STATE_DISCONNECTING) {
//                                Log.d(TAG, "enter to dismiss dialog and remove timeout" +
//                                        " message & unregister mAttributeListener");
//                                Message msg = mHandler.obtainMessage();
//                                msg.what = DIALOG_DISMISS;
//                                mHandler.sendMessage(msg);
//                                mHandler.removeMessages(CONNECT_TIMEOUT_OVER);
//                                mCachedDevice.unregisterAttributeChangeListener(mAttributeListener);
//                            }
//                        }
//                    }
//                }
//            };

    private LocalBluetoothLEManager.DeviceConnecttionChangeListener mConnectionListener
        = new LocalBluetoothLEManager.DeviceConnecttionChangeListener() {

            @Override
            public void onDeviceConnectionStateChange(BluetoothDevice device, int state) {
                // TODO Auto-generated method stub
                if (device == null) {
                    Log.d(TAG, "[mConnectionListener] device is null");
                    return;
                }
                Log.d(TAG, "[mConnectionListener] state : " + state);
                if (state == BluetoothGatt.STATE_CONNECTED
                        || state == BluetoothGatt.STATE_DISCONNECTED) {
                    Log.d(TAG, "[mConnectionListener] enter to dismiss dialog," +
                            "remove timeout message and unregister connection listener");
                    Message msg = mHandler.obtainMessage();
                    msg.what = DIALOG_DISMISS;
                    mHandler.sendMessage(msg);
                    mHandler.removeMessages(CONNECT_TIMEOUT_OVER);
                    mLocalManager.unregisterConnectionStateChangeListener(this);
                } else {
                    Log.d(TAG, "[mConnectionListener] device state is connecing" +
                                " or disconnecting");
                }
            }
        
    };
    
    private void doConnectAction(final BluetoothDevice device, final int localIndex) {
        if (localIndex < 0) {
            Log.d(TAG, "[doConnectAction] mLocationIndex < 0");
            return;
        }
//        mDeviceManager.registerDeviceListChangedListener(mDeviceListListener);
//        mCachedDevice = mDeviceManager.findDevice(device);
//        if (mCachedDevice != null) {
//            mCachedDevice.registerAttributeChangeListener(mAttributeListener);
//        }
        mLocalManager.registerConnectionStateChangeListener(mConnectionListener);
        Log.d(TAG, "[doConnectAction] mLocationIndex : " + localIndex);
        Runnable r = new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                Log.d(TAG, "[doConnectAction] start to connect gatt device");
                mLocalManager.connectGattDevice(device, false, localIndex);
            }
        };
        new Thread(r).start();
        
        if (mShowDialog) {
            ConnectProgressAlertDialog.show(mActivity,/* "connecting...", */
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {
                            // TODO Auto-generated method stub
                            Log.d(TAG, "[ConnectProgressAlertDialog.CANCEL]" +
                                " cancel button clicked, do disconnect action");
                            Message msg = mHandler.obtainMessage();
                            msg.what = DISCONNECT_ACTION;
                            msg.obj = device;
                            mHandler.sendMessageDelayed(msg, 0);
                        }
                    });
        }
        
        Message msg = mHandler.obtainMessage();
        msg.what = CONNECT_TIMEOUT_OVER;
        msg.obj = device;
        mHandler.sendMessageDelayed(msg, CONNECT_TIMEOUT);
    }
    
    private void doDisconnectAction(final BluetoothDevice device) {
        if (mShowDialog) {
            ConnectProgressAlertDialog.dismiss();
        }
//        mDeviceManager.unregisterDeviceListChangedListener(mDeviceListListener);
//        if (mCachedDevice != null) {
//            mCachedDevice.unregisterAttributeChangeListener(mAttributeListener);
//        }
        Runnable r = new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                Log.d(TAG, "[doDisconnectAction] start to disconnect gatt device");
                mLocalManager.disconnectGattDevice(device);
            }
        };
        new Thread(r).start();
        mHandler.removeMessages(CONNECT_TIMEOUT_OVER);
    }
}
