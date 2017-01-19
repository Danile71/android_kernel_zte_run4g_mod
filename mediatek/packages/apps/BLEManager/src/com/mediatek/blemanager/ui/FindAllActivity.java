package com.mediatek.blemanager.ui;


import android.app.Activity;
import android.bluetooth.BluetoothGatt;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.Toast;


import java.util.ArrayList;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;

import com.mediatek.bluetooth.BleFindMeProfile;

public class FindAllActivity extends Activity {

    private static final String TAG = BLEConstants.COMMON_TAG + "[FindAllActivity]";
    
    private GridView mDeviceList;
    private ImageView mAlertButton;
    
    private LayoutInflater mInflater;

    private ArrayList<CachedBluetoothLEDevice> mSupportFmpDeviceList;
    private DeviceListAdapter mAdapter;
    private CachedBluetoothLEDeviceManager mDeviceManager;
    private LocalBluetoothLEManager mManager;
    
    private boolean mClear = false;
    
    private boolean mAlertingState = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!checkFmpDevice()) {
            finish();
            Toast.makeText(this, "No FMP devices founded", Toast.LENGTH_SHORT).show();
            return;
        }
        
        this.setContentView(R.layout.find_all_activity_layout);
        mManager = LocalBluetoothLEManager.getInstance(this);
        
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        mDeviceList = (GridView)this.findViewById(R.id.device_grid_list);
        mAlertButton = (ImageView)this.findViewById(R.id.find_all_alert_btn);
        
        mInflater = LayoutInflater.from(this);
        mAdapter = new DeviceListAdapter();
        mDeviceList.setAdapter(mAdapter);
    }

    @Override
    protected void onStart() {
        super.onStart();
        initDeviceList();
        
        if (mSupportFmpDeviceList != null && mSupportFmpDeviceList.size() != 0) {
            for (CachedBluetoothLEDevice device : mSupportFmpDeviceList) {
                boolean curr = device.getBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG);
                if (device.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
                    if (!curr) {
                        Log.d(TAG, "[onStart] begin to find device : " + device.getDeviceName());
                        mManager.findTargetDevice(BleFindMeProfile.LEVEL_HIGH, device.getDevice());
                        device.setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, !curr);
                    }
                }
            }
        }
        
        mAlertButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                // do find action
                if (mSupportFmpDeviceList != null && mSupportFmpDeviceList.size() != 0) {
                    for (CachedBluetoothLEDevice device : mSupportFmpDeviceList) {
                        Log.d(TAG, "mAlertButton ClickListener eneter, mAlertingState : " + mAlertingState);
                        if (mAlertingState) {
                            Log.d(TAG, "mAlertButton ClickListener, set all device to be LEVEL_NO");
                            mManager.findTargetDevice(BleFindMeProfile.LEVEL_NO, device.getDevice());
                        } else {
                            Log.d(TAG, "mAlertButton ClickListener, set all device to be LEVEL_HIGH");
                            mManager.findTargetDevice(BleFindMeProfile.LEVEL_HIGH, device.getDevice());
                        }
//                        boolean currentState = device.getBooleanAttribute(
//                                CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG);
//                        if (device.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
//                            if (currentState) {
//                                Log.d(TAG, "[mAlertButton] start to find LEVEL_NO");
//                                mManager.findTargetDevice(
//                                        BleFindMeProfile.LEVEL_NO, device.getDevice());
//                            } else {
//                                Log.d(TAG, "[mAlertButton] start to find LEVEL_HIGH");
//                                mManager.findTargetDevice(
//                                        BleFindMeProfile.LEVEL_HIGH, device.getDevice());
//                            }
//                            Log.d(TAG, "[mAlertButton] set current state to be : " + !currentState);
//                            device.setBooleanAttribute(
//                                    CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, !currentState);
//                        } else if (device.getConnectionState() == BluetoothGatt.STATE_DISCONNECTED) {
//                            device.setBooleanAttribute(
//                                    CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, false);
//                        }
                    }
                }
                updateAlertButtonState();
            }
        });
        
        mDeviceList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                // TODO Auto-generated method stub
                CachedBluetoothLEDevice cacheDevice = mSupportFmpDeviceList.get(arg2);
                if (cacheDevice != null) {
                    if (cacheDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
                        boolean currentState = cacheDevice.getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG);
                        if (currentState) {
                            Log.d(TAG, "[onItemClick] start to find LEVEL_NO device : " +
                                    cacheDevice.getDeviceName());
                            mManager.findTargetDevice(BleFindMeProfile.LEVEL_NO,
                                    cacheDevice.getDevice());
                        } else {
                            Log.d(TAG, "[onItemClick] start to find LEVEL_HIGH device : " +
                                    cacheDevice.getDeviceName());
                            mManager.findTargetDevice(BleFindMeProfile.LEVEL_HIGH,
                                    cacheDevice.getDevice());
                        }
//                        Log.d(TAG, "[onItemClick] set current state to be : " + !currentState);
//                        cacheDevice.setBooleanAttribute(
//                                CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, !currentState);
                    }
                }
                updateAlertButtonState();
            }
            
        });
        updateAlertButtonState();
    }
    
    @Override
    protected void onStop() {
        super.onStop();
        if (mSupportFmpDeviceList != null) {
            if (mSupportFmpDeviceList.size() != 0) {
                for (CachedBluetoothLEDevice device : mSupportFmpDeviceList) {
                    device.unregisterAttributeChangeListener(mChangeListener);
                }
            }
            mSupportFmpDeviceList.clear();
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        // used to clear back ground thread, which used to flashing the background image.
        mClear = true;
        mAdapter.notifyDataSetChanged();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        // resume the clear action, which to avoid that if the activity is covered
        // by dialog or notification.
        // can keep flashing.
        mClear = false;
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            break;
        default:
            break;
        }
        return true;
    }
    
    private void updateAlertButtonState() {
        if (mSupportFmpDeviceList == null || mSupportFmpDeviceList.size() == 0) {
            Log.d(TAG, "[updateAlertButtonState] mSupportFmpDeviceList size is 0");
            return;
        }
        int connectedNum = 0;
        int alertingNum = 0;
        for (CachedBluetoothLEDevice cachedDevice : mSupportFmpDeviceList) {
            if (cachedDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
                connectedNum ++;
            }
            if (cachedDevice.getBooleanAttribute(CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG)) {
                alertingNum ++;
            }
        }
        Log.d(TAG, "[updateAlertButtonState] connected device number : " + connectedNum);
        Log.d(TAG, "[updateAlertButtonState] alerting device number : " + alertingNum);
        if (connectedNum == 0) {
            this.mAlertButton.setEnabled(false);
            this.mAlertButton.setImageResource(R.drawable.bt_find_disable);
            mAlertingState = false;
        } else {
            if (alertingNum == 0) {
                this.mAlertButton.setEnabled(true);
                this.mAlertButton.setImageResource(R.drawable.bt_find_normal);
                mAlertingState = false;
            } else {
                this.mAlertButton.setEnabled(true);
                this.mAlertButton.setImageResource(R.drawable.bt_find_pressed);
                mAlertingState = true;
            }
        }
    }
    
    /**
     * check if any device support fmp profile, if the stored device contains at least one
     * will return true, otherwise return false,then finish the activity.
     * @return
     */
    private boolean checkFmpDevice() {
        mSupportFmpDeviceList = CachedBluetoothLEDeviceManager.getInstance().getFmpDevices();
        if (mSupportFmpDeviceList.size() == 0) {
            return false;
        }
        return true;
    }
    
    /**
     * register the device attribute change listener for each device
     */
    private void initDeviceList() {
        int number = 0;
        if (mSupportFmpDeviceList != null && mSupportFmpDeviceList.size() != 0) {
            for (CachedBluetoothLEDevice device : mSupportFmpDeviceList) {
                device.registerAttributeChangeListener(mChangeListener);
            }
        }
    }
    
    /**
     * handle the device attribute changed action
     */
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            switch (what) {
            case CachedBluetoothLEDevice.DEVICE_NAME_ATTRIBUTE_FLAG:
            case CachedBluetoothLEDevice.DEVICE_CONNECTION_STATE_CHANGE_FLAG:
            case CachedBluetoothLEDevice.DEVICE_DISTANCE_FLAG:
            case CachedBluetoothLEDevice.DEVICE_IMAGE_ATTRIBUTE_FLAG:
            case CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG:
                mAdapter.notifyDataSetChanged();
                break;
                
                default:
                    break;
            }
            updateAlertButtonState();
        }
        
    };
    
    /**
     * device attribute changed listener
     */
    private CachedBluetoothLEDevice.DeviceAttributeChangeListener mChangeListener =
            new CachedBluetoothLEDevice.DeviceAttributeChangeListener() {
        
        @Override
        public void onDeviceAttributeChange(CachedBluetoothLEDevice device, int which) {
            // TODO Auto-generated method stub
            Log.d(TAG, "[onAttributeChanged] disOrder : " +
                    device.getDeviceLocationIndex() + ", whichAttribute : " + which);
            Message msg = mHandler.obtainMessage();
            msg.what = which;
            mHandler.sendMessage(msg);
        }
    };
    
    /**
     * device list adapter
     *
     */
    private class DeviceListAdapter extends BaseAdapter {

        ArrayList<ViewHolder> mHolders;

        public DeviceListAdapter() {
            mHolders = new ArrayList<ViewHolder>();
        }

        @Override
        public int getCount() {
            int size = mSupportFmpDeviceList.size();
            if (size == 0) {
                Log.d(TAG, "[getCount] mHolder.size : " + mHolders.size());
                for (ViewHolder holder : this.mHolders) {
                    holder.mDeviceImage.setDeviceAlertState(mHandler, false);
                }
            }
            return size;
        }

        @Override
        public Object getItem(int arg0) {
            return mSupportFmpDeviceList.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return arg0;
        }

        @Override
        public View getView(int arg0, View arg1, ViewGroup arg2) {
            ViewHolder holder;
            if (arg1 == null) {
                arg1 = mInflater.inflate(R.layout.find_all_device_item, null);
                holder = new ViewHolder();
                holder.mDeviceImage = (ComposeDeviceImage)arg1.findViewById(R.id.fa_device_image);
                arg1.setTag(holder);
                Log.d(TAG, "[getView] add holder to mHolders");
                mHolders.add(holder);
            } else {
                holder = (ViewHolder) arg1.getTag();
            }
            holder.mDeviceImage.setDeviceImage(mSupportFmpDeviceList.get(arg0).getDeviceImage());
            holder.mDeviceImage.setDeviceName(mSupportFmpDeviceList.get(arg0).getDeviceName());
            if (mSupportFmpDeviceList.get(arg0).isSupportPxpOptional()) {
                holder.mDeviceImage.setDeviceSignal(
                        mSupportFmpDeviceList.get(arg0).getIntAttribute(
                                CachedBluetoothLEDevice.DEVICE_DISTANCE_FLAG));
            } else {
                holder.mDeviceImage.setDeviceSignal(Integer.MAX_VALUE);
            }
            if (mClear) {
                holder.mDeviceImage.setDeviceAlertState(mHandler, false);
            } else {
                holder.mDeviceImage.setDeviceAlertState(mHandler,
                        mSupportFmpDeviceList.get(arg0).getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG));
            }
            
            holder.mDeviceImage.setDeviceConnectionState(
                    mSupportFmpDeviceList.get(arg0).getConnectionState());
            if (mSupportFmpDeviceList.get(arg0).getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
                holder.mDeviceImage.setEnabled(true);
                holder.mDeviceImage.setAlpha((float) 255);
            } else if (mSupportFmpDeviceList.get(arg0).getConnectionState() == BluetoothGatt.STATE_DISCONNECTED) {
                holder.mDeviceImage.setEnabled(false);
                holder.mDeviceImage.setAlpha((float) 125);
            }
            return arg1;
        }
        
        private class ViewHolder {
            private ComposeDeviceImage mDeviceImage;
        }
        
    }
    
}
