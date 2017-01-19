package com.mediatek.blemanager.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;

import java.util.ArrayList;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.BluetoothCallback;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;


public class ScanAction {

    public static final String TAG = BLEConstants.COMMON_TAG + "[ScanAction]";
    public static final int DEVICE_TYPE_LE = 2;

    private static final int SCANNING_DEVICE_OVERTIME = 60 * 1000; // 30s
    private static final int CONNECT_DEVICE_OVERTIME = 30 * 1000; // 30s

    private static final int START_SCANNING_DEVICE_FLAG = 10;
    private static final int STOP_SCANNING_DEVICE_FLAG = 20;
    private static final int SCANNING_DEVICE_OVER_DELAY_FLAG = 30;
    private static final int CONNECT_DEVICE_FLAG = 40;
//    private static final int CONNECT_DEVICE_OVER_DELAY_FLAG = 50;
//    private static final int DISCONNECT_DEVICE_FLAG = 60;
    private static final int SCAN_DEVICE_ADD_FLAG = 70;
    private static final int SCAN_DEVICE_REMOVE_FLAG = 80;

    private static final int UPDATE_UI_FLAG = 100;
    private static final int DISMISS_DELETE_DEVICE_FLAG = 110;
    private static final int DELETE_DEVICE_FLAG = 120;
    private int mMaxiumConnectedCount = 4;
    private static final String PERSIST_KEY = "persist.bt.lemaxdevice";

    private ArrayList<BluetoothDevice> mScannedDeviceList;
    private ArrayList<BluetoothDevice> mConnectedDeviceList;

    private LocalBluetoothLEManager mLocalManager;
    private CachedBluetoothLEDeviceManager mDeviceManager;
    private BluetoothManager mBluetoothManager;

    private Activity mActivity;
    private AlertDialog mScanDialog;
    private ConnectAction mConnectAction;
    
    private int mLocationIndex;
    private int mCount = 0;

    public ScanAction(Activity activity) {
        if (activity == null) {
            throw new IllegalArgumentException("[ScanAction] activity is null");
        }
        mActivity = activity;
        mScannedDeviceList = new ArrayList<BluetoothDevice>();
        mConnectedDeviceList = new ArrayList<BluetoothDevice>();
        
        mConnectAction = new ConnectAction(mActivity);
        
        mLocalManager = LocalBluetoothLEManager.getInstance(mActivity);
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        if (mBluetoothManager == null) {
            mBluetoothManager =
                (BluetoothManager)(mActivity.getSystemService(Context.BLUETOOTH_SERVICE));
        }

//        mLocalManager.registerBluetoothLEScanStateCallback(mScannedCallback);
//        mLocalManager.registerNameChangeListener(mNameChangeListener);
        
    }
    
//    public interface DeviceConnected {
//        public void onDeviceConnected();
//    }

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            int what = msg.what;
            switch (what) {
            case START_SCANNING_DEVICE_FLAG:
                showScanDialog();
                doStartScanAction();
                sendHandlerMessage(SCANNING_DEVICE_OVER_DELAY_FLAG, SCANNING_DEVICE_OVERTIME);
                break;

            case STOP_SCANNING_DEVICE_FLAG:
                doStopScanAction();
                mHandler.removeMessages(SCANNING_DEVICE_OVER_DELAY_FLAG);
                ScanDeviceAlertDialog.dismiss();
                break;

            case SCANNING_DEVICE_OVER_DELAY_FLAG:
                doStopScanAction();
                if (mScannedDeviceList.size() == 0) {
                    mScannedDeviceList.clear();
                    ScanDeviceAlertDialog.notifyUi();
                    ScanDeviceAlertDialog.dismiss();
                } else {
                    ScanDeviceAlertDialog.hideProgressBar();
                }
                mHandler.removeMessages(SCANNING_DEVICE_OVER_DELAY_FLAG);
                break;

            case UPDATE_UI_FLAG:
//                ScanDeviceAlertDialog.notifyUi();
                break;

            case CONNECT_DEVICE_FLAG:
                mHandler.removeMessages(SCANNING_DEVICE_OVER_DELAY_FLAG);
                BluetoothDevice device = (BluetoothDevice) msg.obj;
                doConnectAction(device);
                break;

            case SCAN_DEVICE_ADD_FLAG:
//                Log.d(TAG, "[mHandler.SCAN_DEVICE_ADD_FLAG] begin!!");
//                BluetoothDevice deviceToAdd = (BluetoothDevice)msg.obj;
//                if (!mScannedDeviceList.contains(deviceToAdd)) {
//                    if (mDeviceManager.findDevice(deviceToAdd) == null) {
//                        Log.d(TAG, "[mHandler.SCAN_DEVICE_ADD_FLAG] call to add device");
//                        mScannedDeviceList.add(deviceToAdd);
//                    }
//                }
//                ScanDeviceAlertDialog.notifyUi();
                break;
                
            case SCAN_DEVICE_REMOVE_FLAG:
//                BluetoothDevice deviceToRemove = (BluetoothDevice)msg.obj;
//                mScannedDeviceList.remove(deviceToRemove);
//                ScanDeviceAlertDialog.notifyUi();
                break;
            case DISMISS_DELETE_DEVICE_FLAG:
                ScanDeviceAlertDialog.dismiss();
                break;
                
            case DELETE_DEVICE_FLAG:
                BluetoothDevice deleteDevice = (BluetoothDevice) msg.obj;
                if (deleteDevice != null) {
                    Log.d(TAG, "from delete dialog,enter delete operation");
                    mLocalManager.disconnectGattDevice(deleteDevice);
                    deleteDevice.removeBond();
                    CachedBluetoothLEDevice cacheDevice = mDeviceManager.findDevice(deleteDevice);
                    if (cacheDevice != null) {
                        Log.d(TAG, "remove delete device from cache");
                        mDeviceManager.removeDevice(cacheDevice);
                    }
                }
                break;
            default:
                break;
            }
        }

    };

    private void showScanDialog() {
        mScannedDeviceList.clear();
        mCount = 0;
        ArrayList<BluetoothDevice> deviceListInManager = new ArrayList<BluetoothDevice>();
        for (CachedBluetoothLEDevice cachedevice : mDeviceManager
                .getCachedDevicesCopy()) {
            if (cachedevice != null) {
                deviceListInManager.add(cachedevice.getDevice());
            }
        }
        for (BluetoothDevice connectedDevice : mBluetoothManager
                .getConnectedDevices(BluetoothProfile.GATT)) {
            if ((deviceListInManager != null)
                    && (!deviceListInManager.contains(connectedDevice))) {
                mScannedDeviceList.add(connectedDevice);
                updateScanDialog(UPDATE_UI_FLAG, connectedDevice);
            }
            Log.v(TAG, "connectdevice=" + connectedDevice.getAddress()
                    + "name=" + connectedDevice.getName());
            mCount++;
        }
        Log.v(TAG, "count=" + mCount);
        for (BluetoothDevice bondedDevice : BluetoothAdapter
                .getDefaultAdapter().getBondedDevices()) {
            if ((!mScannedDeviceList.contains(bondedDevice))
                    && (bondedDevice.getType() == DEVICE_TYPE_LE)) {
                mScannedDeviceList.add(bondedDevice);
                updateScanDialog(UPDATE_UI_FLAG, bondedDevice);
            }
        }
        mScanDialog =
            ScanDeviceAlertDialog.show(ScanDeviceAlertDialog.SCAN,mActivity, mScannedDeviceList,
                new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        sendHandlerMessage(STOP_SCANNING_DEVICE_FLAG, 0);
                    }
                }, new AdapterView.OnItemClickListener() {

                    @Override
                    public void onItemClick(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        // TODO Auto-generated method stub
                        // do connect action
                        BluetoothDevice device = mScannedDeviceList.get(arg2);
                        Log.d(TAG, "[onItemClick] device : " + device.getName());
                        Message msg = mHandler.obtainMessage();
                        if (isNeedConnect(device)) {
                            Log.d(TAG, "isNeedConnect(device) is true ");
                            msg.what = CONNECT_DEVICE_FLAG;
                            msg.obj = device;
                            mHandler.sendMessage(msg);
                            mHandler.removeMessages(SCANNING_DEVICE_OVER_DELAY_FLAG);
                        } else {
                            Log.d(TAG, "showDeleteDeviceDialog() ");
                            showDeleteDeviceDialog();
                        }
                    }
                });
    }

    private void showDeleteDeviceDialog() {
        mConnectedDeviceList.clear();
        for (BluetoothDevice connectedDevice :
            mBluetoothManager.getConnectedDevices(BluetoothProfile.GATT)) {
            mConnectedDeviceList.add(connectedDevice);
            updateScanDialog(UPDATE_UI_FLAG, connectedDevice);
        }

        mScanDialog = ScanDeviceAlertDialog.show(ScanDeviceAlertDialog.DELETE, mActivity,
                    mConnectedDeviceList,
                new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        sendHandlerMessage(DISMISS_DELETE_DEVICE_FLAG, 0);
                    }
                }, new AdapterView.OnItemClickListener() {

                    @Override
                    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                        BluetoothDevice device = mConnectedDeviceList.get(arg2);
                        Log.d(TAG, "[onItemClick] device : " + device.getName());
                        final Message msg = mHandler.obtainMessage();
                        msg.what = DELETE_DEVICE_FLAG;
                        msg.obj = device;
                        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);
                        builder.setTitle(R.string.delete_text);
                        builder.setMessage(R.string.device_delete_dialog_message);
                        builder.setPositiveButton(R.string.yes,
                                new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                mHandler.sendMessage(msg);
                                sendHandlerMessage(DISMISS_DELETE_DEVICE_FLAG, 0);
                            }
                        });
                        builder.setNegativeButton(R.string.no, null);
                        builder.create().show();

                    }
                });

    }

    private boolean isNeedConnect(BluetoothDevice device) {
        int receriveCount = SystemProperties.getInt(PERSIST_KEY, 4);
        Log.i(TAG, "receive remote set device count=" + receriveCount);
        if (receriveCount > 0 && receriveCount < 5) {
            mMaxiumConnectedCount = receriveCount;
        }
        Log.i(TAG, "at last, real maxium count=" + mMaxiumConnectedCount);
        if ((mBluetoothManager.getConnectionState(device, BluetoothProfile.GATT) == BluetoothProfile.STATE_CONNECTED)
                || (mCount < mMaxiumConnectedCount)) {
            return true;
        }
        return false;
    }
    private void doStartScanAction() {
        mLocalManager.registerBluetoothLEScanStateCallback(mScannedCallback);
        mLocalManager.registerNameChangeListener(mNameChangeListener);
        Runnable r = new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                mLocalManager.startLEScan(0);
            }
        };
        new Thread(r).start();
    }
    
    private void doStopScanAction() {
        mLocalManager.unregisterScanStateCallback(mScannedCallback);
        mLocalManager.unregisterNameChangeListener(mNameChangeListener);
        Runnable r = new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                mLocalManager.stopLEScan();
            }
        };
        new Thread(r).start();
    }
    
    private void doConnectAction(final BluetoothDevice device) {
        ScanDeviceAlertDialog.dismiss();
        sendHandlerMessage(STOP_SCANNING_DEVICE_FLAG, 0);
        mConnectAction.connect(device, mLocationIndex, true);
    }

    private void sendHandlerMessage(int what, long delayTime) {
        Message msg = mHandler.obtainMessage();
        msg.what = what;
        mHandler.sendMessageDelayed(msg, delayTime);
    }
    
    private void updateScanDialog(final int id, final BluetoothDevice device) {
        if (device == null) {
            Log.d(TAG, "[updateScanDialog] device is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                switch (id) {
                case UPDATE_UI_FLAG:
                    Log.d(TAG, "[updateScanDialog] UPDATE_UI_FLAG !!");
                    break;

                case SCAN_DEVICE_ADD_FLAG:
                    Log.d(TAG, "[updateScanDialog] SCAN_DEVICE_ADD_FLAG !!");
                    if (!mScannedDeviceList.contains(device)) {
                        if (mDeviceManager.findDevice(device) == null) {
                            Log.d(TAG, "[updateScanDialog] call to add device");
                            mScannedDeviceList.add(device);
                        }
                    }
                    break;

                case SCAN_DEVICE_REMOVE_FLAG:
                    Log.d(TAG, "[updateScanDialog] SCAN_DEVICE_REMOVE_FLAG !!");
                    if (!mScannedDeviceList.contains(device)) {
                        Log.d(TAG, "[updateScanDialog] NOT contained in the list");
                        return;
                    }
                    mScannedDeviceList.remove(device);
                    break;

                    default:
                        Log.d(TAG, "[updateScanDialog] unrecongnized id");
                        return;
                }
                Log.d(TAG, "[updateScanDialog] call to notify scan dialog");
                ScanDeviceAlertDialog.notifyUi();
            }

        });
    }

    private BluetoothCallback.BluetoothLEDeviceScanned mScannedCallback =
            new BluetoothCallback.BluetoothLEDeviceScanned() {

        @Override
        public void onScannedBleDeviceRemoved(BluetoothDevice device) {
            // TODO Auto-generated method stub
            Log.d(TAG, "onScannedBleDeviceRemoved enter");
//            Message msg = mHandler.obtainMessage();
//            msg.what = SCAN_DEVICE_REMOVE_FLAG;
//            msg.obj = device;
//            mHandler.sendMessage(msg);
            updateScanDialog(SCAN_DEVICE_REMOVE_FLAG, device);
        }

        @Override
        public void onScannedBleDeviceAdded(BluetoothDevice device) {
            // TODO Auto-generated method stub
            Log.d(TAG, "onScannedBleDeviceAdded enter");
//            Message msg = mHandler.obtainMessage();
//            msg.what = SCAN_DEVICE_ADD_FLAG;
//            msg.obj = device;
//            mHandler.sendMessage(msg);
            updateScanDialog(SCAN_DEVICE_ADD_FLAG, device);
        }
    };

    private LocalBluetoothLEManager.DeviceNameChangeListener mNameChangeListener =
            new LocalBluetoothLEManager.DeviceNameChangeListener() {
        @Override
        public void onDeviceNameChange(BluetoothDevice device, String name) {
            // TODO Auto-generated method stub
            Log.d(TAG, "[onDeviceNameChange] device : " + device.getAddress()
                    + ", name : " + name);
//            sendHandlerMessage(UPDATE_UI_FLAG, 0);
            updateScanDialog(UPDATE_UI_FLAG, device);
        }
    };

    public void doScanAction(int localIndex) {
        mLocationIndex = localIndex;
        sendHandlerMessage(START_SCANNING_DEVICE_FLAG, 0);
    }

}
