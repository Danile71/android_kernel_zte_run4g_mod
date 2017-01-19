
package com.mediatek.blemanager.provider;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.ui.ActivityUtils;

public class ActivityDbOperator {

    private static final String TAG = BLEConstants.COMMON_TAG + "[ActivityDbOperator]";

    private static ActivityDbOperator sInstance;
    private static final ArrayList<Integer> UX_ATTRIBUTE_LIST = new ArrayList<Integer>();
    private static final ArrayList<Integer> PXP_UX_ATTRIBUTE_LIST = new ArrayList<Integer>();
    private static final ArrayList<Integer> PXP_CLIENT_ATTRIBUTE_LIST = new ArrayList<Integer>();
    private static final ArrayList<Integer> ANS_ATTRIBUTE_LIST = new ArrayList<Integer>();

    private Context mContext;

    private ActivityDbOperator(Context context) {
        mContext = context;

        UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_NAME_ATTRIBUTE_FLAG);
        // UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_IMAGE_ATTRIBUTE_FLAG);
        // UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_AUTO_CONNECT_FLAG);
        // UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG);
        UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_SERVICE_LIST_CHANGE_FLAG);

        PXP_CLIENT_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG);
        PXP_CLIENT_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG);
        PXP_CLIENT_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG);
        PXP_CLIENT_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG);
        PXP_CLIENT_ATTRIBUTE_LIST
                .add(CachedBluetoothLEDevice.DEVICE_DISCONNECTION_WARNING_EANBLER_FLAG);

        PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG);
        PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_VOLUME_FLAG);
        PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_VIBRATION_ENABLER_FLAG);
        // PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_SUPPORT_OPTIONAL);
        PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_RANGE_INFO_DIALOG_ENABELR_FLAG);
        PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_RINGTONE_URI_FLAG);
        // PXP_UX_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_SERVICE_LIST_CHANGE_FLAG);

        ANS_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG);
        ANS_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG);
        ANS_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG);
        ANS_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG);
        // ANS_ATTRIBUTE_LIST.add(CachedBluetoothLEDevice.DEVICE_SERVICE_LIST_CHANGE_FLAG);
    }

    public static void initialization(Context context) {
        sInstance = new ActivityDbOperator(context);
    }

    public static ActivityDbOperator getInstance() {
        return sInstance;
    }

    public boolean isInDb(CachedBluetoothLEDevice device, int which) {
        String selection = BLEConstants.COLUMN_BT_ADDRESS + "='" + device.getDevice().getAddress()
                + "'";
        ContentResolver cr = mContext.getContentResolver();
        Uri uri = null;
        if (which == 0) {
            uri = BLEConstants.TABLE_UX_URI;
        }/*
          * else if (which == 1) { //uri = BLEConstants.TABLE_PXP_URI; } else if
          * (which == 2) { uri = BLEConstants.TABLE_ANS_URI; }
          */
        if (uri == null) {
            Log.d(TAG, "[isInDb] uri is null");
            return false;
        }
        Cursor queryCursor = cr.query(uri, null, selection, null, null);
        if (queryCursor == null) {
            return false;
        }
        if (queryCursor.getCount() == 0) {
            queryCursor.close();
            return false;
        }
        queryCursor.close();
        Log.d(TAG, "[isInDb] device is alread in db.");
        return true;
    }

    private byte[] getDefaultImage() {
        return ActivityUtils.getDefaultImage(mContext);
    }

    public void updateDeviceAttributeToDb(CachedBluetoothLEDevice cacheDevice, int which) {
        if (cacheDevice == null || cacheDevice.getDevice() == null) {
            Log.d(TAG, "[updateDeviceAttributeToDb] cacheDevice is null");
            return;
        }
        if (UX_ATTRIBUTE_LIST.contains(which)) {
            this.updateDeviceUxAttribute(cacheDevice, which);
        }
        if (PXP_UX_ATTRIBUTE_LIST.contains(which)) {
            this.updateDevicePxpAttribute(cacheDevice, which);
        }
//        if (ANS_ATTRIBUTE_LIST.contains(which)) {
//            this.updateDeviceAnsAttribute(cacheDevice, which);
//        }
        if (PXP_CLIENT_ATTRIBUTE_LIST.contains(which)) {
            this.updateClientPxpAttribute(cacheDevice);
        }
    }

    public void initDevice(final CachedBluetoothLEDevice device) {
        // Runnable r= new Runnable () {
        // @Override
        // public void run() {
        if (!checkParamter(device)) {
            Log.d(TAG, "[initDevice] device parameter is wrong");
            return;
        }
        if (isInDb(device, 0)) {
            Log.d(TAG, "[initDevice] device is already in db," + " no need to do initialization");
            return;
        }

        ContentValues values = new ContentValues();
        ContentResolver resolver = mContext.getContentResolver();
        values.put(BLEConstants.COLUMN_BT_ADDRESS, device.getDevice().getAddress());
        values.put(BLEConstants.DEVICE_SETTINGS.DEVICE_DISPLAY_ORDER,
                device.getDeviceLocationIndex());
        values.put(BLEConstants.DEVICE_SETTINGS.DEVICE_NAME, device.getDeviceName());
        values.put("image_byte_array", getDefaultImage());
        initPxpData(device, values);

        Uri uri = resolver.insert(BLEConstants.TABLE_UX_URI, values);
        device.setDeviceImage(uri);
        // }

        // };
        // new Thread(r).start();
    }

    private void updateDeviceUxAttribute(final CachedBluetoothLEDevice device, final int which) {
        if (!UX_ATTRIBUTE_LIST.contains(which)) {
            Log.d(TAG, "[updateDeviceUxAttribute] attribute is not in, no need to update");
            return;
        }
        if (!checkParamter(device)) {
            return;
        }
        Log.d(TAG, "[updateDeviceUxAttribute] device name : " + device.getDeviceName());
        Log.d(TAG, "[updateDeviceUxAttribute] which : " + which);
        Runnable r = new Runnable() {

            @Override
            public void run() {
                String selection = BLEConstants.COLUMN_BT_ADDRESS + "='"
                        + device.getDevice().getAddress() + "'";
                ContentResolver cr = mContext.getContentResolver();

                if (!isInDb(device, 0)) {
                    Log.d(TAG, "[updateDeviceUxAttribute] device is not is db");
                    return;
                }
                ContentValues values = new ContentValues();
                switch (which) {
                    case CachedBluetoothLEDevice.DEVICE_NAME_ATTRIBUTE_FLAG:
                        values.put(BLEConstants.DEVICE_SETTINGS.DEVICE_NAME, device.getDeviceName());
                        break;
                    case CachedBluetoothLEDevice.DEVICE_SERVICE_LIST_CHANGE_FLAG:
                        buildServiceList(device.getServiceList(), values);
                        break;

                    default:
                        return;
                }
                if (values.size() == 0) {
                    Log.d(TAG, "[updateDeviceUxAttribute] values is empty");
                    return;
                }
                Log.d(TAG, "[updateDeviceUxAttribute] values : " + values);
                cr.update(BLEConstants.TABLE_UX_URI, values, selection, null);
            }

        };
        new Thread(r).start();
    }

    public void updateDeviceImage(CachedBluetoothLEDevice cachedDevice, byte[] bytes) {
        if (bytes == null || bytes.length == 0) {
            Log.d(TAG, "[updateDeviceImage] bytes is wrong");
            return;
        }
        if (cachedDevice == null || cachedDevice.getDevice() == null) {
            Log.d(TAG, "[updateDeviceImage] cachedDevice is null");
            return;
        }
        Log.d(TAG, "[updateDeviceImage] call to update device image in db");
        ContentValues values = new ContentValues();
        values.put("image_byte_array", bytes);
        String where = BLEConstants.COLUMN_BT_ADDRESS + "='"
                + cachedDevice.getDevice().getAddress() + "'";
        this.mContext.getContentResolver().update(BLEConstants.TABLE_UX_URI, values, where, null);
    }

    private void buildServiceList(List<UUID> serviceList, ContentValues values) {
        if (serviceList == null || values == null) {
            Log.d(TAG, "[buildServiceList] parameter is wrong");
            return;
        }
        if (serviceList.size() == 0) {
            Log.d(TAG, "[buildServiceList] service list is empty");
            return;
        }
        StringBuilder builder = new StringBuilder();
        for (UUID uid : serviceList) {
            builder.append(uid.toString());
            builder.append(BLEConstants.SERVICE_LIST_SEPERATER);
        }
        values.put(BLEConstants.DEVICE_SETTINGS.DEVICE_SERVICE_LIST, builder.toString());
    }

    private void updateDevicePxpAttribute(final CachedBluetoothLEDevice device, final int which) {
        if (!PXP_UX_ATTRIBUTE_LIST.contains(which)) {
            Log.d(TAG, "[updateDevicePxpAttribute] attribute is not in, no need to update");
            return;
        }
        if (!checkParamter(device)) {
            return;
        }
        Log.d(TAG, "[updateDevicePxpAttribute] device name : " + device.getDeviceName());

        Runnable r = new Runnable() {

            @Override
            public void run() {
                if (!isInDb(device, 0)) {
                    Log.d(TAG, "[updateDevicePxpAttribute] device is not in db");
                    return;
                }
                updatePxpData(device, which);
            }

        };
        new Thread(r).start();
    }

    /**
     * used to init pxp data if the device service has been changed, should add
     * teh device into db
     * 
     * @param device
     */
    private void initPxpData(CachedBluetoothLEDevice device, ContentValues values) {
        if (device == null || device.getDevice() == null) {
            Log.d(TAG, "[initPxpData] device is null");
            return;
        }
        Log.d(TAG, "[initPxpData] enter to do init");

        values.put(BLEConstants.PXP_CONFIGURATION.RANGE_ALERT_INFO_DIALOG_ENABLER, device
                .getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_RANGE_INFO_DIALOG_ENABELR_FLAG));
        values.put(BLEConstants.PXP_CONFIGURATION.RINGTONE_ENABLER,
                device.getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG));
        values.put(BLEConstants.PXP_CONFIGURATION.VOLUME,
                device.getIntAttribute(CachedBluetoothLEDevice.DEVICE_VOLUME_FLAG));
        values.put(BLEConstants.PXP_CONFIGURATION.VIBRATION_ENABLER,
                device.getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_VIBRATION_ENABLER_FLAG));
        if (device.getRingtoneUri() != null) {
            values.put(BLEConstants.PXP_CONFIGURATION.RINGTONE, device.getRingtoneUri().toString());
        }
        // mContext.getContentResolver().insert(BLEConstants.TABLE_PXP_URI,
        // values);
    }

    private void updatePxpData(CachedBluetoothLEDevice device, int which) {
        Log.d(TAG, "[updatePxpData] enter");
        String selection = BLEConstants.COLUMN_BT_ADDRESS + "='" + device.getDevice().getAddress()
                + "'";
        ContentResolver cr = mContext.getContentResolver();
        ContentValues values = new ContentValues();
        switch (which) {
        // TODO update these parameters by api
            case CachedBluetoothLEDevice.DEVICE_RANGE_INFO_DIALOG_ENABELR_FLAG:
                values.put(BLEConstants.PXP_CONFIGURATION.RANGE_ALERT_INFO_DIALOG_ENABLER,
                        device.getBooleanAttribute(which));
                break;
            case CachedBluetoothLEDevice.DEVICE_RINGTONE_ENABLER_FLAG:
                values.put(BLEConstants.PXP_CONFIGURATION.RINGTONE_ENABLER,
                        device.getBooleanAttribute(which));
                break;
            case CachedBluetoothLEDevice.DEVICE_RINGTONE_URI_FLAG:
                values.put(BLEConstants.PXP_CONFIGURATION.RINGTONE, device.getRingtoneUri()
                        .toString());
                break;
            case CachedBluetoothLEDevice.DEVICE_VOLUME_FLAG:
                values.put(BLEConstants.PXP_CONFIGURATION.VOLUME, device.getIntAttribute(which));
                break;
            case CachedBluetoothLEDevice.DEVICE_VIBRATION_ENABLER_FLAG:
                values.put(BLEConstants.PXP_CONFIGURATION.VIBRATION_ENABLER,
                        device.getBooleanAttribute(which));
                break;
            default:
                return;
        }
        cr.update(BLEConstants.TABLE_UX_URI, values, selection, null);
    }

    private void updateClientPxpAttribute(final CachedBluetoothLEDevice device) {
        if (!checkParamter(device)) {
            return;
        }
        Log.d(TAG, "[updateClientPxpAttribute] device name : " + device.getDeviceName());

        Runnable r = new Runnable() {

            @Override
            public void run() {
                if (!isInDb(device, 0)) {
                    Log.d(TAG, "[updateClientPxpAttribute] device is not in db");
                    return;
                }
                // TODO update client pxp data by api
                Log.d(TAG, "[updateClientPxpAttribute] call to update client data");
                LocalBluetoothLEManager
                        .getInstance(mContext)
                        .updateClientPxpData(
                                device.getDevice(),
                                device.getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG),
                                device.getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG),
                                device.getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_DISCONNECTION_WARNING_EANBLER_FLAG),
                                device.getIntAttribute(CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG),
                                device.getIntAttribute(CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG));
            }

        };
        new Thread(r).start();
    }

    // TODO, should do delete action by calling api
    private void deletePxpData(CachedBluetoothLEDevice device) {
        if (device == null || device.getDevice() == null) {
            Log.d(TAG, "[deletePxpData] device is null");
            return;
        }

    }

    private void updateDeviceAnsAttribute(final CachedBluetoothLEDevice device, final int which) {
        if (!ANS_ATTRIBUTE_LIST.contains(which)) {
            Log.d(TAG, "[updateDeviceAnsAttribute] attribute is not in, no need to update");
            return;
        }
        Runnable r = new Runnable() {

            @Override
            public void run() {
                if (!checkParamter(device)) {
                    Log.d(TAG, "[updateDeviceAnsAttribute] parameter is wrong");
                    return;
                }
                Log.d(TAG, "[updateDeviceAnsAttribute] device name : " + device.getDeviceName());

                String selection = BLEConstants.COLUMN_BT_ADDRESS + "='"
                        + device.getDevice().getAddress() + "'";
                ContentResolver cr = mContext.getContentResolver();
                Cursor queryCursor = cr.query(BLEConstants.TABLE_ANS_URI, null, selection, null,
                        null);
                if (queryCursor == null) {
                    Log.d(TAG, "[updateDeviceAnsAttribute] queryCursor is null!!");
                    return;
                }
                if (queryCursor.getCount() == 0) {
                    Log.d(TAG, "[updateDeviceAnsAttribute] device is not in db!!");
                    queryCursor.close();
                    return;
                }
                queryCursor.close();

                ContentValues values = new ContentValues();
                switch (which) {
                    case CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG:
                        values.put(BLEConstants.ANS_CONFIGURATION.ANS_HOST_CALL_ALERT,
                                device.getBooleanAttribute(which) ? 3 : 0);
                        break;

                    case CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG:
                        values.put(BLEConstants.ANS_CONFIGURATION.ANS_HOST_MISSED_CALL_ALERT,
                                device.getBooleanAttribute(which) ? 3 : 0);
                        break;

                    case CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG:
                        values.put(BLEConstants.ANS_CONFIGURATION.ANS_HOST_EMAIL_ALERT,
                                device.getBooleanAttribute(which) ? 3 : 0);
                        break;

                    case CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG:
                        values.put(BLEConstants.ANS_CONFIGURATION.ANS_HOST_SMSMMS_ALERT,
                                device.getBooleanAttribute(which) ? 3 : 0);
                        break;

                    default:
                        return;
                }
                cr.update(BLEConstants.TABLE_ANS_URI, values, selection, null);

                Log.d(TAG, "[updateDeviceAnsAttribute] notify host configuration has been changed");
            }

        };
        new Thread(r).start();
    }

    public void deleteDevice(final CachedBluetoothLEDevice device) {
        Runnable r = new Runnable() {

            @Override
            public void run() {
                if (!checkParamter(device)) {
                    return;
                }
                String selection = BLEConstants.COLUMN_BT_ADDRESS + "='"
                        + device.getDevice().getAddress() + "'";
                ContentResolver cr = mContext.getContentResolver();
                Cursor queryCursor = cr.query(BLEConstants.TABLE_UX_URI, null, selection, null,
                        null);
                if (queryCursor == null) {
                    Log.d(TAG, "[updateDeviceAttribute] queryCursor is null!!");
                    return;
                }
                if (queryCursor.getCount() == 0) {
                    Log.d(TAG, "[updateDeviceAttribute] device is not in db!!");
                    queryCursor.close();
                    return;
                }

                queryCursor.close();

                cr.delete(BLEConstants.TABLE_UX_URI, selection, null);
                // TODO call API to delete device which in bluetoothle.db
                deletePxpData(device);
            }

        };

        new Thread(r).start();
    }

    private static boolean checkParamter(CachedBluetoothLEDevice device) {
        if (device == null) {
            return false;
        }
        if (device.getDevice() == null) {
            return false;
        }
        return true;
    }

}
