package com.mediatek.blemanager;


import android.app.Application;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

import java.util.ArrayList;

import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.ActivityDbOperator;
import com.mediatek.blemanager.provider.BLEConstants;

public class BLEApp extends Application {

    private static final String TAG = BLEConstants.COMMON_TAG + "[BLEApp]";
    
    private CachedBluetoothLEDeviceManager mDeviceManager;
    private LocalBluetoothLEManager mManager;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate enter!!");
        mManager = LocalBluetoothLEManager.getInstance(this);
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        ActivityDbOperator.initialization(this.getApplicationContext());
        initDevices();
//        verifyDeviceList();
    }

    @Override
    public void onLowMemory() {
        // TODO Auto-generated method stub
        Log.d(TAG, "[onLowMemory] enter to do clear actions");
        if (mManager != null) {
            mManager.close();
        } else {
            Log.e(TAG, "[onLowMemory] mManager is null");
        }
        super.onLowMemory();
    }

    @Override
    public void onTerminate() {
        // TODO Auto-generated method stub
        Log.d(TAG, "[onTerminate] enter to do clear actions");
        if (mManager != null) {
            mManager.close();
        } else {
            Log.e(TAG, "[onTerminate] mManager is null");
        }
        super.onTerminate();
    }

    /**
     * init device from db, and construct the cached manager
     */
    private void initDevices() {
        ContentResolver cr = this.getContentResolver();
//        Cursor c = cr.query(BLEConstants.TABLE_UX_URI, null, null, null, null);
//        if (c == null || c.getCount() == 0) {
//            doNonInitialization();
//        }
        Cursor c1 = cr.query(BLEConstants.TABLE_UX_URI, null, null, null, null);
        
        doInitialization(c1);
//        if (c != null) {
//            c.close();
//        }
        if (c1 != null) {
            c1.close();
        }
    }

    private void doInitialization(Cursor cursor) {
        Log.d(TAG, "[doInitialization] enter!!");
        if (cursor == null || cursor.getCount() == 0) {
            Log.d(TAG, "[doInitialization] cursor is null or empty");
            return;
        }
        if (cursor.moveToFirst()) {
             do {
                int disOrder = cursor.getInt(
                        cursor.getColumnIndex(BLEConstants.DEVICE_SETTINGS.DEVICE_DISPLAY_ORDER));
                String deviceAddress = cursor.getString(
                        cursor.getColumnIndex(BLEConstants.COLUMN_BT_ADDRESS));
                String name = cursor.getString(
                        cursor.getColumnIndex(BLEConstants.DEVICE_SETTINGS.DEVICE_NAME));
                BluetoothDevice device =
                    BluetoothAdapter.getDefaultAdapter().getRemoteDevice(deviceAddress);
                Uri imageUri = Uri.parse(BLEConstants.TABLE_UX_URI_STRING + "/" +
                        cursor.getInt(cursor.getColumnIndex(BLEConstants.COLUMN_ID)));
                Log.d(TAG, "[doInitialization] imageUri : " + imageUri);
                CachedBluetoothLEDevice cachedDevice = mDeviceManager.addDevice(device, disOrder);
                if (cachedDevice != null) {
                    cachedDevice.setInitFromDb(true);
                    cachedDevice.setDeviceName(name);
                    cachedDevice.setDeviceImage(imageUri);
                    getServiceList(cachedDevice, cursor);
                    initPxpConfiguration(cachedDevice, deviceAddress);
//                    initAnsConfiguration(cachedDevice, deviceAddress);
                    cachedDevice.setInitFromDb(false);
                }
            } while(cursor.moveToNext());
        }
    }
    
    private void getServiceList(CachedBluetoothLEDevice cachedDevice, Cursor cursor) {
        ArrayList<String> retList = new ArrayList<String>();
        if (cachedDevice == null || cursor == null || cursor.getCount() == 0) {
            Log.d(TAG, "[getServiceList] parameter is wrong");
            return;
        }
        String str = cursor.getString(
                cursor.getColumnIndex(BLEConstants.DEVICE_SETTINGS.DEVICE_SERVICE_LIST));
        if (str == null || str.trim().length() == 0) {
            Log.d(TAG, "[getServiceList] service list is null or empty");
            return;
        }
        String[] list = str.split(BLEConstants.SERVICE_LIST_SEPERATER);
        for (String s : list) {
            if (s.trim().length() != 0) {
                retList.add(s);
            }
        }
        cachedDevice.setServiceListFromDb(retList);
        return;
    }

    private void initPxpConfiguration(CachedBluetoothLEDevice cachedDevice, String address) {
        initPxpConfigurationFromUxTable(cachedDevice, address);
//        initPxpConfigurationFromClientTable(cachedDevice, address);
    }
    
    private void initPxpConfigurationFromUxTable(
            CachedBluetoothLEDevice cachedDevice, String address) {
        String selection = BLEConstants.COLUMN_BT_ADDRESS + "='" + address + "'";
        Cursor cursor = this.getContentResolver().query(
                BLEConstants.TABLE_UX_URI, null, selection, null, null);
        if (cursor == null) {
            Log.d(TAG, "[initPxpConfiguration] cursor is null");
            return;
        }
        if (cursor.getCount() == 0) {
            cursor.close();
            Log.d(TAG, "[initPxpConfiguration] cursor is null");
            return;
        }
        if (cursor.moveToFirst()) {
            cachedDevice.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG,
                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
                            BLEConstants.PXP_CONFIGURATION.RINGTONE_ENABLER))));
            cachedDevice.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_VIBRATION_ENABLER_FLAG,
                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
                            BLEConstants.PXP_CONFIGURATION.VIBRATION_ENABLER))));
            cachedDevice.setBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_RANGE_INFO_DIALOG_ENABELR_FLAG,
                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
                            BLEConstants.PXP_CONFIGURATION.RANGE_ALERT_INFO_DIALOG_ENABLER))));
            cachedDevice.setIntAttribute(CachedBluetoothLEDevice.DEVICE_VOLUME_FLAG,
                    cursor.getInt(cursor.getColumnIndex(BLEConstants.PXP_CONFIGURATION.VOLUME)));
            
            String str = cursor.getString(
                    cursor.getColumnIndex(BLEConstants.PXP_CONFIGURATION.RINGTONE));
            if (str != null) {
                cachedDevice.setRingtoneUri(Uri.parse(str));
            }
        }
        cursor.close();
    }
    
//    private void initPxpConfigurationFromClientTable(
//            CachedBluetoothLEDevice cachedDevice, String address) {
//        String selection = BLEConstants.COLUMN_BT_ADDRESS + "='" + address + "'";
//        Cursor cursor = this.getContentResolver().query(
//                BLEConstants.TABLE_CLIENT_URI, null, selection, null, null);
//        if (cursor == null) {
//            Log.d(TAG, "[initPxpConfiguration] cursor is null");
//            return;
//        }
//        if (cursor.getCount() == 0) {
//            cursor.close();
//            Log.d(TAG, "[initPxpConfiguration] cursor is null");
//            return;
//        }
//        if (cursor.moveToFirst()) {
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.PXP_CONFIGURATION.ALERT_ENABLER))));
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_DISCONNECTION_WARNING_EANBLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.PXP_CONFIGURATION.DISCONNECTION_WARNING_ENABLER))));
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.PXP_CONFIGURATION.RANGE_ALERT_ENABLER))));
//            cachedDevice.setIntAttribute(CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG,
//                    cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.PXP_CONFIGURATION.RANGE_VALUE)));
//            cachedDevice.setIntAttribute(CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG,
//                    cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.PXP_CONFIGURATION.RANGE_TYPE)));
//        }
//        cursor.close();
//    }
    
//    private void initAnsConfiguration(CachedBluetoothLEDevice cachedDevice, String address) {
//        String selection = BLEConstants.COLUMN_BT_ADDRESS + "='" + address + "'";
//        Cursor cursor = this.getContentResolver().query(
//                BLEConstants.TABLE_ANS_URI, null, selection, null, null);
//        if (cursor == null) {
//            Log.d(TAG, "[initPxpConfiguration] cursor is null");
//            return;
//        }
//        if (cursor.getCount() == 0) {
//            Log.d(TAG, "[initPxpConfiguration] cursor is null");
//            cursor.close();
//            return;
//        }
//        if (cursor.moveToFirst()) {
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_HOST_CALL_ALERT))));
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_HOST_MISSED_CALL_ALERT))));
//            cachedDevice.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_HOST_EMAIL_ALERT))));
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_HOST_SMSMMS_ALERT))));
//            
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_REMOTE_INCOMING_CALL_FLAGE,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_CALL_ALERT))));
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_REMOTE_MISSED_CALL_FLAGE,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_MISSED_CALL_ALERT))));
//            cachedDevice.setBooleanAttribute(
//                    CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_MESSAGE_FLAGE,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_SMSMMS_ALERT))));
//            cachedDevice.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_EMAIL_FLAGE,
//                    int2Boolean(cursor.getInt(cursor.getColumnIndex(
//                            BLEConstants.ANS_CONFIGURATION.ANS_REMOTE_EMAIL_ALERT))));
//        }
//        cursor.close();
//    }
    
    private boolean int2Boolean(int value) {
        if (value == 0) {
            return false;
        }
        return true;
    }
}
