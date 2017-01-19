package com.mediatek.blemanager.ui;


import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;
import android.widget.Toast;

import java.util.ArrayList;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.BluetoothCallback;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.bluetooth.BleFindMeProfile;
import com.mediatek.bluetooth.BleProfileServiceManager;

public class PairedDeviceListActivity extends PreferenceActivity {

    private static final String TAG = BLEConstants.COMMON_TAG + "[PairedDeviceListActivity]";
    
    private static final String ALERT_SETTING_PREFERENCE = "alert_set_preference";
    private static final String DEVICE_MANAER_PREFERENCE = "device_manager_preference";
    
    private static final String SETTING_EXTRA = "current_device";
    
    private static final int START_SCAN = 1;
    private static final int CONNECT_ACTION = 2;
    private static final int DISCONNECT_ACTION = 3;
    
    private Context mContext;
    
    private ActivityStarter mAlertSettingStarter;
    private ActivityStarter mDeviceManagerStarter;
    
    private ImageButton mDeviceFindButton;
    private ImageButton mDeviceConnectButton;
    private NonChangeSwitchPreference mAlertSettingPreference;
    private Preference mDeviceManagerPreference;
    private BleStageView mStageView;

    private CachedBluetoothLEDeviceManager mDeviceManager;
    private LocalBluetoothLEManager mManager;
    private CachedBluetoothLEDevice mCachedDevice;

    private boolean mDeviceManagerServiceConnected;
	private boolean mPxpServiceConnected;
    
    private int mCurrentLocationIndex;
    private int mClickLocationIndex;
    private ProgressDialog mProgressDialog;
    private ArrayList<BluetoothDevice> mConnectedDeviceList;
    
    private ScanAction mScanAction;
    private ConnectAction mConnectAction;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        this.addPreferencesFromResource(R.xml.paired_device_activity_preference);
        this.setContentView(R.layout.paired_device_activity);

        mAlertSettingPreference =
            (NonChangeSwitchPreference)findPreference(ALERT_SETTING_PREFERENCE);
        mDeviceManagerPreference = findPreference(DEVICE_MANAER_PREFERENCE);

        mContext = this;
        mDeviceFindButton = (ImageButton)this.findViewById(R.id.btn_find_me);
        mDeviceConnectButton = (ImageButton)this.findViewById(R.id.btn_connect);
        mStageView = (BleStageView)this.findViewById(R.id.ble_stage_view);
        
        mConnectedDeviceList = new ArrayList<BluetoothDevice>();
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        int size = mDeviceManager.getCachedDevicesCopy().size();
        Log.d(TAG, "[onCreate] devicemanager size : " + size);
        
        mManager = LocalBluetoothLEManager.getInstance(this.getApplicationContext());
        if (mManager.getCurrentState() != BluetoothAdapter.STATE_ON) {
//            mProgressDialog = ProgressDialog.show(this, "", "Turning on bt...");
            mManager.registerBluetoothAdapterStateCallback(mAdapterStateCallback);
//            mManager.turnOnBluetooth();
            TurnOnBTProgressDialog.show(this);
        }
        
        initActivity();
        mConnectAction = new ConnectAction(this);
        mScanAction = new ScanAction(this);
        
    }

    @Override
    protected void onResume() {
        super.onResume();
        this.mStageView.onResume();
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        this.mStageView.onPause();
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "[onStart] enter!!");
        super.onStart();
        this.mDeviceManagerServiceConnected =
        	mManager.getServiceConnectionState(LocalBluetoothLEManager.PROFILE_DEVICE_MANAGER_SERVICE_ID);
        this.mPxpServiceConnected = mManager.getServiceConnectionState(LocalBluetoothLEManager.PROFILE_PXP_ID);
        
        Log.d(TAG, "[onStart] mDeviceManagerServiceConnected : " + mDeviceManagerServiceConnected);
        Log.d(TAG, "[onStart] mPxpServiceConnected : " + mPxpServiceConnected);
        
        mDeviceManager.registerDeviceListChangedListener(mCachedDeviceListChange);
        mManager.registerServiceConnectionListener(mServiceConnectionListener);
        registerCachedDeviceListener();
        update3DView(true);
        updateActivityState();
        
    }

    @Override
    protected void onStop() {
        super.onStop();
        
        update3DView(false);
        mManager.unregisterAdaterStateCallback(mAdapterStateCallback);
        mDeviceManager.unregisterDeviceListChangedListener(mCachedDeviceListChange);
        mManager.unregisterServiceConnectionListener(mServiceConnectionListener);
        unregisterCachedDeviceListener();
    }

    @Override
    protected void onDestroy() {
        //TODO Auto-generated method stub
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // TODO Auto-generated method stub
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
            if (mManager != null) {
                Log.d(TAG, "[onDestroy] call to close LocalBluetoothLEManager");
                mManager.close();
            }
            this.finish();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        
        menu.clear();
        
        MenuItem item = menu.add(0, 0, 0, R.string.find_all_text);
        item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        
        if (mManager != null) {
//            if (mManager.getCurrentState() == BluetoothAdapter.STATE_ON) {
                if (mManager.getBackgroundMode() == BleProfileServiceManager.RET_ENABLED) {
                    menu.add(0, 2, 0, R.string.disable_background_service);
                } else if (mManager.getBackgroundMode() == BleProfileServiceManager.RET_DISABLED) {
                    menu.add(0, 1, 0, R.string.enable_background_service);
                }
//            }
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
        case 0:
            if (mManager.getCurrentState() != BluetoothAdapter.STATE_ON) {
                showToast(1);
                break;
            }
            if (!mManager.getServiceConnectionState(LocalBluetoothLEManager.PROFILE_DEVICE_MANAGER_SERVICE_ID)) {
            	showToast(2);
            	break;
            }
            Intent intent = new Intent(this, FindAllActivity.class);
            startActivity(intent);
            break;
            
        case 1:
            if (mManager != null) {
                Log.d(TAG, "[onOptionsItemSelected] set background mode to be true");
                mManager.setBackgroundMode(true);
            }
            break;
            
        case 2:
            if (mManager != null) {
                Log.d(TAG, "[onOptionsItemSelected] set background mode to be false");
                mManager.setBackgroundMode(false);
            }
            break;
            
            default:
                break;
        }
        return true;
    }
    
    private void registerCachedDeviceListener() {
        for (CachedBluetoothLEDevice device : mDeviceManager.getCachedDevicesCopy()) {
            if (device != null) {
                device.registerAttributeChangeListener(mDeviceAttributeListener);
            }
        }
    }
    
    private void unregisterCachedDeviceListener() {
        for (CachedBluetoothLEDevice device : mDeviceManager.getCachedDevicesCopy()) {
            if (device != null) {
                device.unregisterAttributeChangeListener(mDeviceAttributeListener);
            }
        }
    }
    
    /**
     * while enter onStart call this to add device
     * while enter onStop, call this method to remove device
     * @param load
     */
    private void update3DView(final boolean load) {
        this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                ArrayList<CachedBluetoothLEDevice> devicelist =
                    mDeviceManager.getCachedDevicesCopy();
                if (load) {
                    for (CachedBluetoothLEDevice device : devicelist) {
                        mStageView.addDevice(device);
                    }
                } else {
                    for (CachedBluetoothLEDevice device : devicelist) {
                        mStageView.removeDevice(device);
                    }
                }
            }
        });
        
    }

    private BluetoothCallback.BluetoothAdapterState mAdapterStateCallback
                = new BluetoothCallback.BluetoothAdapterState() {
            
            @Override
            public void onBluetoothStateChanged(int state) {
                // TODO Auto-generated method stub
                if (state == BluetoothAdapter.STATE_ON) {
                    mManager.unregisterAdaterStateCallback(mAdapterStateCallback);
                    TurnOnBTProgressDialog.dismiss();
                } else if (state == BluetoothAdapter.STATE_OFF) {
                    TurnOnBTProgressDialog.dismiss();
                    mManager.unregisterAdaterStateCallback(mAdapterStateCallback);
                }
            }
            
            @Override
            public void onBluetoothScanningStateChanged(boolean started) {
                // TODO Auto-generated method stub
                
            }
        };
    
    
    private void initActivity() {
        mDeviceFindButton.setOnClickListener(mFindButtonClickListener);
        mDeviceConnectButton.setOnClickListener(mConnectButtonClickListener);
        mAlertSettingStarter = new ActivityStarter();
        mDeviceManagerStarter = new ActivityStarter();
        mAlertSettingPreference.setOnPreferenceClickListener(mAlertSettingStarter);
        mAlertSettingPreference.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference arg0, Object arg1) {
                boolean checked = mAlertSettingPreference.isChecked();
                mCachedDevice.setBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG, !checked);
                mAlertSettingPreference.setChecked(
                        mCachedDevice.getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG));
                return true;
            }
        });
        mDeviceManagerPreference.setOnPreferenceClickListener(mDeviceManagerStarter);

        mStageView.setOnBleEventListener(new OnBleEventListener() {

            @Override
            public void onClick(int locationIndex) {
                // TODO Auto-generated method stub
                if (mCachedDevice == null) {
                    if (mManager.getCurrentState() == BluetoothAdapter.STATE_ON) {
                        Log.d(TAG, "[mStageView.onClick] locationIndex : " + locationIndex);
                        mClickLocationIndex = locationIndex;
                            mScanAction.doScanAction(mClickLocationIndex);
                    } else {
                        Log.d(TAG, "[mStageView.onClick] BT is off");
                        showToast(1);
                    }
                } else {
                    Log.d(TAG, "[mStageView.onClick] device : " + mCachedDevice.getDeviceName());
                }
            }

            @Override
            public void onFocus(int locationIndex) {
                // TODO Auto-generated method stub
                Log.d(TAG, "[mStageView.onFocus] locationIndex : " + locationIndex);
                mCurrentLocationIndex = locationIndex;
                updateActivityState();
            }
            
        });
    }
    
    /**
     * while swip the 3D view, update the UX state
     */
    private void updateActivityState() {
        this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                Log.d(TAG, "[updateActivityState] mCurrentDisplay : " + mCurrentLocationIndex);
                mStageView.refresh();
                mCachedDevice = mDeviceManager.getCachedDeviceFromDisOrder(mCurrentLocationIndex);
                Log.d(TAG, "[updateActivityState] mCachedDevice : " + mCachedDevice);
                // update device manager preference
                if (mCachedDevice == null) {
                    mDeviceManagerPreference.setEnabled(false);
                } else {
                    mDeviceManagerPreference.setEnabled(true);
                }
                // update alert setting preference
                if (mCachedDevice != null) {
                    if (!mCachedDevice.isSupportLinkLost()) {
                        mAlertSettingPreference.setEnabled(false);
                        mAlertSettingPreference.setChecked(false);
                    } else {
                    	if (mPxpServiceConnected) {
                    		mAlertSettingPreference.setEnabled(true);
                    	} else {
                    		mAlertSettingPreference.setEnabled(false);
                    	}
                        mAlertSettingPreference.setChecked(
                                mCachedDevice.getBooleanAttribute(
                                        CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG));
                    }
                } else {
                    mAlertSettingPreference.setChecked(false);
                    mAlertSettingPreference.setEnabled(false);
                }
                // update device find button & connect button
                if (mCachedDevice == null) {
                    mDeviceFindButton.setVisibility(View.GONE);
                    mDeviceConnectButton.setImageResource(R.drawable.bt_connect_disable);
                    mDeviceConnectButton.setVisibility(View.VISIBLE);
                    mDeviceConnectButton.setEnabled(false);
                } else {
                    if (mCachedDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
                        mDeviceConnectButton.setImageResource(R.drawable.bt_connect_normal);
                        mDeviceConnectButton.setVisibility(View.GONE);
                        mDeviceFindButton.setVisibility(View.VISIBLE);
                        if (mCachedDevice.isSupportFmp()) {
                        	if (mDeviceManagerServiceConnected) {
                        		mDeviceFindButton.setEnabled(true);
                        	} else {
                        		mDeviceFindButton.setEnabled(false);
                        	}
                            if (mCachedDevice.getBooleanAttribute(
                                    CachedBluetoothLEDevice.DEVICE_RINGTONE_ALARM_STATE_FLAG)) {
                                mDeviceFindButton.setImageResource(R.drawable.bt_find_pressed);
                            } else {
                                mDeviceFindButton.setImageResource(R.drawable.bt_find_normal);
                            }
                        } else {
                            mDeviceFindButton.setImageResource(R.drawable.bt_find_disable);
                            mDeviceFindButton.setEnabled(false);
                        }
                    } else if (mCachedDevice.getConnectionState()
                                == BluetoothGatt.STATE_DISCONNECTED) {
                        //TODO should add for do connect action
                        mDeviceConnectButton.setImageResource(R.drawable.bt_connect_normal);
                        mDeviceConnectButton.setEnabled(true);
                        mDeviceConnectButton.setVisibility(View.VISIBLE);
                        mDeviceFindButton.setVisibility(View.GONE);
                    } else if (mCachedDevice.getConnectionState()
                                == BluetoothGatt.STATE_CONNECTING) {
                        mDeviceConnectButton.setImageResource(R.drawable.bt_connect_pressed);
                        mDeviceConnectButton.setEnabled(true);
                        mDeviceConnectButton.setVisibility(View.VISIBLE);
                        mDeviceFindButton.setVisibility(View.GONE);
                    }
                }
            }
        });
        
    }
    
    /**
     * CachedBluetoothLEDeviceManager cached listener, if the cache has been changed
     * remove all cached device in 3D view, and the add all device to 3D view,
     * meanwhile update the activity state with the top-current device configuration
     */
    private CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener mCachedDeviceListChange =
            new CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener() {

                @Override
                public void onDeviceAdded(CachedBluetoothLEDevice device) {
                    // TODO Auto-generated method stub
                    device.registerAttributeChangeListener(mDeviceAttributeListener);
                    update3DView(false);
                    update3DView(true);
                    updateActivityState();
                }

                @Override
                public void onDeviceRemoved(CachedBluetoothLEDevice device) {
                    // TODO Auto-generated method stub
                    update3DView(false);
                    update3DView(true);
                    updateActivityState();
                }
        
    };
    
    /**
     * Find me button action listener
     * 
     * if device is on fmp state, do stop find action
     * if device is not on fmp state, do start find action
     */
    private View.OnClickListener mFindButtonClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            // do find device action.
            if (mCachedDevice == null) {
                Log.d(TAG, "[mFindButtonClickListener] mCachedDevice is null");
                return;
            }
            
            boolean currentPxpState = mCachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_PXP_ALARM_STATE_FLAG);
            boolean currentFmpState = mCachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG);
            Log.d(TAG, "[mFindButtonClickListener] currentFmpState : " + currentFmpState);
            Log.d(TAG, "[mFindButtonClickListener] currentPxpState : " + currentPxpState);
            if (currentPxpState) {
                // call PXP stop alert API
                Log.d(TAG, "[mFindButtonClickListener] call pxp service to stop" +
                            " remote device alert");
                mManager.stopRemoteDeviceAlert(mCachedDevice.getDevice());
                mCachedDevice.onDevicePxpAlertStateChange(false);
            } else {
                if (currentFmpState) {
                    Log.d(TAG, "[mFindButtonClickListener] find device with LEVEL_NO");
                    mManager.findTargetDevice(BleFindMeProfile.LEVEL_NO, mCachedDevice.getDevice());
//                    mCachedDevice.setBooleanAttribute(
//                            CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, false);
                } else {
                    Log.d(TAG, "[mFindButtonClickListener] find device with LEVEL_HIGH");
                    mManager.findTargetDevice(
                            BleFindMeProfile.LEVEL_HIGH, mCachedDevice.getDevice());
//                    mCachedDevice.setBooleanAttribute(
//                            CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, true);
                }
            }
            if (mCachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_RINGTONE_ALARM_STATE_FLAG)) {
                Log.d(TAG, "[mFindButtonClickListener] set find button to be pressed");
                mDeviceFindButton.setImageResource(R.drawable.bt_find_pressed);
            } else {
                Log.d(TAG, "[mFindButtonClickListener] set find button to be normal");
                mDeviceFindButton.setImageResource(R.drawable.bt_find_normal);
            }
        }
    };

    private void showToast(final int which) {
        this.runOnUiThread(new Runnable() {
                public void run() {
                	String str = null;
                	if (which == 1) {
                		str = "BT is off, please turn it on to move on";
                	} else if (which == 2) {
                		str = "Device Manager service is not ready, cann't do find action";
                	}
                	if (str == null) {
                		Log.e(TAG, "[showToast] str is null");
                		return;
                	}
                    Toast.makeText(PairedDeviceListActivity.this, str, Toast.LENGTH_SHORT).show();
                }
            });
    }
    
    /**
     * device connect action
     * 
     * if device is disconnected, click the button do connect action without progress dialog
     */
    private View.OnClickListener mConnectButtonClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if (mCachedDevice == null) {
                Log.d(TAG, "[mConnectButtonClickListener] mCachedDevice is null");
                return;
            }
            if (mManager.getCurrentState() != BluetoothAdapter.STATE_ON) {
                showToast(1);
                return;
            }
            // while the device is connecting state, click the button to cancel the connect action
            // while the device is disconected state, click to do connect action
            Log.d(TAG, "[mConnectButtonClickListener] connect button clicked, state : " +
                    mCachedDevice.getConnectionState());
            if (mCachedDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTING) {
                Log.d(TAG, "[mConnectButtonClickListener] do disconnect action" +
                            " while STATE_CONNECTING");
                mManager.disconnectGattDevice(mCachedDevice.getDevice());
            } else if (mCachedDevice.getConnectionState() == BluetoothGatt.STATE_DISCONNECTED) {
                Log.d(TAG, "[mConnectButtonClickListener] do connect action" +
                            " while STATE_DISCONNECTED");
                mConnectAction.connect(mCachedDevice.getDevice(),
                        mCachedDevice.getDeviceLocationIndex(), false);
            }
            
//            mDeviceConnectButton.setImageResource(R.drawable.bt_connect_pressed);
        }
    };
    
    private CachedBluetoothLEDevice.DeviceAttributeChangeListener mDeviceAttributeListener =
            new CachedBluetoothLEDevice.DeviceAttributeChangeListener() {

                @Override
                public void onDeviceAttributeChange(
                        CachedBluetoothLEDevice device, int which) {
                    // TODO Auto-generated method stub
                    updateActivityState();
                }
        
    };
    
	private LocalBluetoothLEManager.ServiceConnectionListener mServiceConnectionListener
		= new LocalBluetoothLEManager.ServiceConnectionListener() {
		public void onServiceConnectionChange(int profileService, int connection) {
			Log.d(TAG, "[mServiceConnectionListener] profileService : "
					+ profileService);
			Log.d(TAG, "[mServiceConnectionListener] connection : "
					+ connection);
			if (profileService == LocalBluetoothLEManager.PROFILE_DEVICE_MANAGER_SERVICE_ID) {
				if (connection == LocalBluetoothLEManager.PROFILE_CONNECTED) {
					mDeviceManagerServiceConnected = true;
				} else if (connection == LocalBluetoothLEManager.PROFILE_DISCONNECTED) {
					mDeviceManagerServiceConnected = false;
				}
				updateActivityState();
			} else if (profileService == LocalBluetoothLEManager.PROFILE_PXP_ID) {
				if (connection == LocalBluetoothLEManager.PROFILE_CONNECTED) {
					mPxpServiceConnected = true;
				} else if (connection == LocalBluetoothLEManager.PROFILE_DISCONNECTED) {
					mPxpServiceConnected = false;
				}
				updateActivityState();
			}
		}
		};
    
    private class ActivityStarter implements Preference.OnPreferenceClickListener {

        @Override
        public boolean onPreferenceClick(Preference preference) {
            Intent intent;
            String key = preference.getKey();
            if (key.equals(ALERT_SETTING_PREFERENCE)) {
                intent = new Intent(PairedDeviceListActivity.this, AlertSettingPreference.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP
                        | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                intent.putExtra(SETTING_EXTRA, mCurrentLocationIndex);
                mContext.startActivity(intent);
            } else if (key.equals(DEVICE_MANAER_PREFERENCE)) {
                intent = new Intent(PairedDeviceListActivity.this, DeviceManagerActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP
                        | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                intent.putExtra(SETTING_EXTRA, mCurrentLocationIndex);
                mContext.startActivity(intent);
            }
            return false;
        }
    }

}
