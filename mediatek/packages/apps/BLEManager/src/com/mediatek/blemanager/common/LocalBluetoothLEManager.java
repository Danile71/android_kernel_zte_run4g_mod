package com.mediatek.blemanager.common;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.util.SparseIntArray;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.bluetooth.BleAlertNotificationProfileService;
import com.mediatek.bluetooth.BleDeviceManager;
import com.mediatek.bluetooth.BleFindMeProfile;
import com.mediatek.bluetooth.BleGattDevice;
import com.mediatek.bluetooth.BleGattUuid;
import com.mediatek.bluetooth.BleManager;
import com.mediatek.bluetooth.BleProfile;
import com.mediatek.bluetooth.BleProfileService;
import com.mediatek.bluetooth.BleProfileServiceManager;
import com.mediatek.bluetooth.BleProximityProfileService;

import com.mediatek.bluetoothle.pxp.IProximityProfileServiceCallback;

public class LocalBluetoothLEManager {

    private static final String TAG = BLEConstants.COMMON_TAG + "[LocalBluetoothLEManager]";
    
    private static final int CATEGORY_ID_INCOMING_CALL = 3;
    private static final int CATEGORY_ID_MISSED_CALL = 4;
    private static final int CATEGORY_ID_SMS = 5;
    private static final int CATEGORY_ID_EMAIL = 1;
    
    public static final int PROFILE_ANP_ID = 100;
	public static final int PROFILE_PXP_ID = 200;
	public static final int PROFILE_DEVICE_MANAGER_SERVICE_ID = 300;

	public static final int PROFILE_CONNECTED = 10;
	public static final int PROFILE_DISCONNECTED = 11;
    
    private static final ArrayList<Integer> CATEGORY_IDS = new ArrayList<Integer>();
    
    private static LocalBluetoothLEManager sInstance;
    private LocalBluetoothAdapter mLocalAdapter;
    private BluetoothLEEventManager mEventManager;
    private CachedBluetoothLEDeviceManager mCachedDeviceManager;
    private ConcurrentHashMap<BluetoothDevice, Integer> mConnectionMap;
    private Context mContext;

    private BleManager mBleManager;
    private BleDeviceManager mBleDeviceManager;
    private BleProximityProfileService mProximityService;
    private BleAlertNotificationProfileService mAlertProfileService;;
    
    private BleProfileServiceManager mProfileServiceManager;
    
    private CachedBleGattDevice mCachedBleGattDevice;
    private ProximityProfileServiceCallback mPxpProfileServiceCallback;
    
    private CopyOnWriteArrayList<BluetoothCallback.BluetoothAdapterState> mAdapterCallbacks;
    private CopyOnWriteArrayList<BluetoothCallback.BluetoothLEDeviceScanned> mScannedCallbacks;
    private ArrayList<DeviceNameChangeListener> mScannedDeviceNameChangeListeners;
    private ArrayList<DeviceConnecttionChangeListener> mDeviceConnectionChangeListeners;;
    
    private ConcurrentHashMap<BluetoothDevice, ProximityProfileServiceCallback> mPxpCallbackMap;
    private CopyOnWriteArrayList<BluetoothDevice> mConnectingDevices;
    
    private ArrayList<ServiceConnectionListener> mServiceConnectionListeners;

    private boolean mIsFwkInited = false;
    private boolean mIsFwkClosed = false;
    
    private boolean mDeviceManagerServiceConnected = false;
    private boolean mPxpServiceConnected = false;
    private boolean mAnpServiceConnected = false;

    /**
     * constructor
     * 
     * used to initialize the local parameters.
     * 
     * @param context
     */
    private LocalBluetoothLEManager(Context context) {
        mContext = context;
        mLocalAdapter = LocalBluetoothAdapter.getInstance();
        
        mCachedDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        mCachedDeviceManager.registerDeviceListChangedListener(mCachedListListener);
        
        mEventManager = new BluetoothLEEventManager(context, this, mCachedDeviceManager);
        
        mAdapterCallbacks = new CopyOnWriteArrayList<BluetoothCallback.BluetoothAdapterState>();
        mScannedCallbacks = new CopyOnWriteArrayList<BluetoothCallback.BluetoothLEDeviceScanned>();
        mScannedDeviceNameChangeListeners = new ArrayList<DeviceNameChangeListener>();
        mDeviceConnectionChangeListeners = new ArrayList<DeviceConnecttionChangeListener>();

        mServiceConnectionListeners = new ArrayList<ServiceConnectionListener>();
        
        mConnectionMap = new ConcurrentHashMap<BluetoothDevice, Integer>();
        
        mConnectingDevices = new CopyOnWriteArrayList<BluetoothDevice>();
        
        mPxpProfileServiceCallback = new ProximityProfileServiceCallback();
        
        mPxpCallbackMap = new ConcurrentHashMap<BluetoothDevice, ProximityProfileServiceCallback>();
        Log.d(TAG, "[construnctor] localbluetoothlemanager created");
        
        CATEGORY_IDS.add(CATEGORY_ID_EMAIL);
        CATEGORY_IDS.add(CATEGORY_ID_INCOMING_CALL);
        CATEGORY_IDS.add(CATEGORY_ID_MISSED_CALL);
        CATEGORY_IDS.add(CATEGORY_ID_SMS);

        mBleManager = BleManager.getDefaultBleProfileManager();
        //get ble profile service manager which can launch service and shut down services
        mBleManager.getProfileServiceManager(mContext, mServiceManagerListener);

        if (mLocalAdapter.getBluetoothState() == BluetoothAdapter.STATE_ON) {
            Log.d(TAG, "[construnctor] BT is ont, call to init FWK");
            initFwk();
        }

        Intent intent = new Intent(mContext, TaskDetectService.class);
        mContext.startService(intent);
    }

    /**
     * 
     */
    public void close() {
        Log.d(TAG, "[close] do clear actions");
        if (!mIsFwkClosed) {
            mIsFwkInited = false;
            mIsFwkClosed = true;
            if (mProfileServiceManager != null) {
                Log.d(TAG, "[close] do shutdownService action");
                mProfileServiceManager.shutdownServices();
            }
            if (mBleManager != null) {
                if (mProximityService != null) {
                    mBleManager.closeProfileServiceProxy(
                            BleProfile.BLE_PROFILE_PXP, mProximityService);
                    mProximityService = null;
                    mPxpServiceConnected = false;
                    notifyServiceConnectionChanged(PROFILE_PXP_ID, PROFILE_DISCONNECTED);
                }
                if (mAlertProfileService != null) {
                    mBleManager.closeProfileServiceProxy(
                            BleProfile.BLE_PROFILE_ANP, mAlertProfileService);
                    mAlertProfileService = null;
                    mAnpServiceConnected = false;
                    notifyServiceConnectionChanged(PROFILE_ANP_ID, PROFILE_DISCONNECTED);
                }
                if (mBleDeviceManager != null) {
                    mBleManager.closeDeviceManager(mBleDeviceManager);
                    mBleDeviceManager = null;
                    mDeviceManagerServiceConnected = false;
                    notifyServiceConnectionChanged(PROFILE_DEVICE_MANAGER_SERVICE_ID,
                                PROFILE_DISCONNECTED);
                }
                if (mProfileServiceManager != null) {
                    mBleManager.closeProfileServiceManager(mProfileServiceManager);
                    mProfileServiceManager = null;
                }
            }
        }
        if (mConnectionMap != null) {
            mConnectionMap.clear();
        }
        if (this.mPxpCallbackMap != null) {
            mPxpCallbackMap.clear();
        }
        if (mAdapterCallbacks != null) {
            mAdapterCallbacks.clear();
        }
        if (mScannedCallbacks != null) {
            mScannedCallbacks.clear();
        }
        if (mScannedDeviceNameChangeListeners != null) {
            mScannedDeviceNameChangeListeners.clear();
        }
        if (mCachedBleGattDevice != null) {
            mCachedBleGattDevice.clearData();
            mCachedBleGattDevice = null;
        }
        if (mDeviceConnectionChangeListeners != null) {
            mDeviceConnectionChangeListeners.clear();
        }
        if (mServiceConnectionListeners != null) {
            mServiceConnectionListeners.clear();
        }
        sInstance = null;
        Intent intent = new Intent(mContext, TaskDetectService.class);
        mContext.stopService(intent);
        mIsFwkInited = false;
    }
    
    /**
     * sigletone instance, get instance from this method.
     * 
     * @param context
     * @return
     */
    public static synchronized LocalBluetoothLEManager getInstance(Context context) {
        if (context == null) {
            Log.d(TAG, "constructor, context is null!!");
            return null;
        }
        if (sInstance == null) {
            LocalBluetoothAdapter adapter = LocalBluetoothAdapter.getInstance();
            if (adapter == null) {
                Log.d(TAG, "constructor, no bluetooth supported!!");
                return null;
            }
            sInstance = new LocalBluetoothLEManager(context);
        }
        return sInstance;
    }

    private void initFwk() {
        if (!mIsFwkInited) {
            Log.d(TAG, "[initFwk] mIsFwkInited is false, do init action");
//            mBleManager = BleManager.getDefaultBleProfileManager();
//            //get ble profile service manager which can launch service and shut down services
//            mBleManager.getProfileServiceManager(mContext, mServiceManagerListener);

            // get ble device manager, which will get device manager
            // in mDeviceManagerListener.onServiceConnected method
            mBleManager.getDeviceManager(mContext, mDeviceManagerListener);
            // get BleProximityProfileService, which will get it from
            // mProfileServiceListener.onServiceConnected method
            mBleManager.getProfileService(mContext,
                    BleProfile.BLE_PROFILE_PXP, mProfileServiceListener);
            
            mBleManager.getProfileService(mContext,
                    BleProfile.BLE_PROFILE_ANP, mProfileServiceListener);
            mIsFwkInited = true;
            mIsFwkClosed = false;
        } else {
            Log.d(TAG, "[initFwk] mIsFwkInited is true, no need init action");
        }
    }
    
    public boolean getServiceConnectionState(int profileId) {
    	if (profileId == PROFILE_DEVICE_MANAGER_SERVICE_ID) {
    		return this.mDeviceManagerServiceConnected;
    	}
    	if (profileId == this.PROFILE_PXP_ID) {
    		return this.mPxpServiceConnected;
    	}
    	if (profileId == this.PROFILE_ANP_ID) {
    		return this.mAnpServiceConnected;
    	}
    	return false;
    }
    
    /**
     * 
     * @param callback
     */
    public void registerBluetoothAdapterStateCallback(
            BluetoothCallback.BluetoothAdapterState callback) {
        if (callback == null) {
            Log.d(TAG, "[registerBluetoothAdapterStateCallback] callback is null");
            return;
        }
        if (mAdapterCallbacks.contains(callback)) {
            Log.d(TAG, "[registerBluetoothAdapterStateCallback]" +
                    " callback has been contained in list");
            return;
        }
        mAdapterCallbacks.add(callback);
    }
    
    /**
     * 
     * @param callback
     */
    public void registerBluetoothLEScanStateCallback(
            BluetoothCallback.BluetoothLEDeviceScanned callback) {
        if (callback == null) {
            Log.d(TAG, "[registerBluetoothLEScanStateCallback] callback is null");
            return;
        }
        if (mScannedCallbacks.contains(callback)) {
            Log.d(TAG, "[registerBluetoothLEScanStateCallback]" +
                        " callback has been contained in list");
            return;
        }
        mScannedCallbacks.add(callback);
    }

    /**
     * 
     * @param callback
     */
    public void unregisterAdaterStateCallback(BluetoothCallback.BluetoothAdapterState callback) {
        if (callback == null) {
            Log.d(TAG, "[unregisterAdaterStateCallback] callback is null");
            return;
        }
        if (!mAdapterCallbacks.contains(callback)) {
            Log.d(TAG, "[unregisterAdaterStateCallback] callback not contained in list");
            return;
        }
        mAdapterCallbacks.remove(callback);
    }
    
    /**
     * 
     * @param callback
     */
    public void unregisterScanStateCallback(BluetoothCallback.BluetoothLEDeviceScanned callback) {
        if (callback == null) {
            Log.d(TAG, "[unregisterScanStateCallback] callback is null");
            return;
        }
        if (!mScannedCallbacks.contains(callback)) {
            Log.d(TAG, "[unregisterScanStateCallback] callback not contained in list");
            return;
        }
        mScannedCallbacks.remove(callback);
    }

    /**
     * scanned device name has been changed listener
     *
     */
    public interface DeviceNameChangeListener {
        void onDeviceNameChange(BluetoothDevice device, String name);
    }
    
    public interface DeviceConnecttionChangeListener {
        void onDeviceConnectionStateChange(BluetoothDevice device, int state);
    }
    
    public interface ServiceConnectionListener {
		void onServiceConnectionChange(int profileService, int connection);
	}
    
    public void registerNameChangeListener(DeviceNameChangeListener listener) {
        if (listener == null) {
            return;
        }
        mScannedDeviceNameChangeListeners.add(listener);
    }
    
    public void unregisterNameChangeListener(DeviceNameChangeListener listener) {
        if (listener == null) {
            return;
        }
        mScannedDeviceNameChangeListeners.remove(listener);
    }
    
    public void registerConnectionStateChangeListener(
            DeviceConnecttionChangeListener listener) {
        if (listener == null) {
            return;
        }
        mDeviceConnectionChangeListeners.add(listener);
    }
    
    public void unregisterConnectionStateChangeListener(
            DeviceConnecttionChangeListener listener) {
        if (listener == null) {
            return;
        }
        mDeviceConnectionChangeListeners.remove(listener);
    }
    
    public void registerServiceConnectionListener(ServiceConnectionListener listener) {
		if (listener == null) {
			return;
		}
        mServiceConnectionListeners.add(listener);
	}
    
	public void unregisterServiceConnectionListener(ServiceConnectionListener listener) {
		if (listener == null) {
			return;
		}
		mServiceConnectionListeners.remove(listener);
	}
    
    private void notifyConnectionStateChange(BluetoothDevice device, int state) {
        if (mDeviceConnectionChangeListeners.size() == 0) {
            Log.d(TAG, "[notifyConnectionStateChange] size is 0, return");
            return;
        }
        for (DeviceConnecttionChangeListener listener : mDeviceConnectionChangeListeners) {
            listener.onDeviceConnectionStateChange(device, state);
        }
    }
    
    void onScannedDeviceNameChanged(BluetoothDevice device, String name) {
        Log.d(TAG, "[onScannedDeviceNameChanged] name : " + name);
        
        for (DeviceNameChangeListener listener : mScannedDeviceNameChangeListeners) {
            listener.onDeviceNameChange(device, name);
        }
    }
    
    private void notifyServiceConnectionChanged(int profile, int connection) {
    	for (ServiceConnectionListener listener : mServiceConnectionListeners) {
			listener.onServiceConnectionChange(profile, connection);
		}
	}

     public void setBackgroundMode(boolean isBackground) {
         boolean success = false;
         if (mProfileServiceManager != null) {
             Log.d(TAG, "[setBackgroundMode] set background  to be : " + isBackground);
             success = mProfileServiceManager.setBackgroundMode(isBackground);
         } else {
             Log.e(TAG, "[setBackgroundMode] mProfileServiceManager is null");
         }
         Log.d(TAG, "[setBackgroundMode] success : " + success);
     }
     
     public int getBackgroundMode() {
         int mode = BleProfileServiceManager.RET_ENABLED;
         if (mProfileServiceManager != null) {
             mode = mProfileServiceManager.getBackgroundMode();
         } else {
             Log.e(TAG, "[getBackgroundMode] mProfileServiceManager is null");
         }
         Log.d(TAG, "[getBackgroundMode] mode : " + mode);
         return mode;
     }
    
    /**
     * used to turn on bluetooth
     */
    public void turnOnBluetooth() {
        mLocalAdapter.setBluetoothEnabled(true);
    }
    
    /**
     * used to turn off bluetooth
     */
    public void turnOffBluetooth() {
        mLocalAdapter.setBluetoothEnabled(false);
    }
    
    /**
     * start to ble device scan action
     * @param order
     */
    public void startLEScan(int order) {
//        mScanOrder = order;
        mLocalAdapter.startScanning(true, mLeCallback);
    }
    
    /**
     * stop ble device scan action
     */
    public void stopLEScan() {
        mLocalAdapter.stopScanning(mLeCallback);
    }
    
    /**
     * Get bluetooth adapter current state
     * @return
     */
    public int getCurrentState() {
        return mLocalAdapter.getBluetoothState();
    }
    
    /**
     * used to connect gatt device.
     * 
     * first should get {@link BleGattDevice} from {@link CachedBleGattDevice}
     * if the device is not in CachedBleGattDevice, just create BleGattDevice from
     * {@link BleGattDeviceManager.createGattDevce} method.
     * if the device in CachedBleGattDevice, just return the BleGattDevice from the cached map.
     * 
     * @param device {@link BluetoothDevice} remote BluetoothDevice which will do connect action
     * @param autoConnect whether the device can do auto-connect or not.
     * @param locationIndex the locationIndex which will be showed in 3D view.
     */
    public void connectGattDevice(BluetoothDevice device, boolean autoConnect, int locationIndex) {
        if (device == null) {
            throw new IllegalArgumentException("connectGattDevice device is null");
        }
        
        CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
        if (cachedDevice != null) {
            if (cachedDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTING) {
                Log.d(TAG, "[connectGattDevice] cached device is connecting," +
                        " no need do connect again");
                return;
            }
        } else {
            Log.d(TAG, "[connectGattDevice] call to add gatt device in FWK");
            addGattDevice(device);
        }
        
        if (mCachedBleGattDevice != null) {
            // get BleGattDevice from CachedBleGattDevice
            BleGattDevice gattDevice =
                mCachedBleGattDevice.getBleGattDevice(mBleDeviceManager, device, mGattDeviceCallback);
            if (gattDevice != null) {
                Log.d(TAG, "[connectGattDevice] start to connect gatt device");
                gattDevice.connect();
                // to make sure the device can be updated
                if (mConnectionMap.containsKey(device)) {
                    mConnectionMap.remove(device);
                }
                mConnectionMap.put(device, locationIndex);
                mCachedDeviceManager.onDeviceConnectionStateChanged(
                        device, BluetoothGatt.STATE_CONNECTING);
            } else {
                Log.d(TAG, "[connectGattDevice] gattDevice is null");
            }
        } else {
            Log.d(TAG, "[connectGattDevice] mCachedBleGattDevice is null");
            Log.d(TAG, "[connectGattDevice] mCachedBleGattDevice is null, add to pending list");
            if (mConnectingDevices.contains(device)) {
                mConnectingDevices.remove(device);
            }
            mConnectingDevices.add(device);
            if (mConnectionMap.containsKey(device)) {
                mConnectionMap.remove(device);
            }
            mConnectionMap.put(device, locationIndex);
            mCachedDeviceManager.onDeviceConnectionStateChanged(
                    device, BluetoothGatt.STATE_CONNECTING);
        }
    }
    
    /**
     * disconnect the gatt device
     * @param device
     */
    public void disconnectGattDevice(BluetoothDevice device) {
        if (device == null) {
            throw new IllegalArgumentException("disconnectGattDevice device is null");
        }
        
        if (mCachedBleGattDevice != null) {
            BleGattDevice gattDevice =
                mCachedBleGattDevice.getBleGattDevice(mBleDeviceManager, device, mGattDeviceCallback);
            if (gattDevice != null) {
                Log.d(TAG, "[disconnectGattDevice] start to disconnect gatt device");
                gattDevice.disconnect();
                mConnectionMap.remove(device);
            } else {
                Log.d(TAG, "[disconnectGattDevice] gattDevice is null");
            }
        } else {
            Log.d(TAG, "[disconnectGattDevice] mCachedBleGattDevice is null");
        }
        if (mConnectingDevices.contains(device)) {
            mConnectingDevices.remove(device);
        }
    }
    
    /**
     * 
     * @param state
     */
    void onAdapterStateChanged(int state) {
        for (BluetoothCallback.BluetoothAdapterState callback : mAdapterCallbacks) {
            callback.onBluetoothStateChanged(state);
        }
        mLocalAdapter.syncBluetoothState();
        if (mLocalAdapter.getBluetoothState() == BluetoothAdapter.STATE_ON) {
            Log.d(TAG, "[onAdapterStateChanged] adapter state changed to STATE_ON");
            initFwk();
            if (mProfileServiceManager != null) {
                Log.d(TAG, "[onAdapterStateChanged] call launch service");
                mProfileServiceManager.launchServices();
            } else {
                Log.d(TAG, "[onAdapterStateChanged] STATE_ON mProfileServiceManager is null");
            }
        } else if (mLocalAdapter.getBluetoothState() == BluetoothAdapter.STATE_OFF) {
            Log.d(TAG, "[onAdapterStateChanged] adapter state changed to STATE_OFF");
            mIsFwkInited = false;
            if (!mIsFwkClosed) {
                mIsFwkClosed = true;
                if (mBleManager != null) {
                    if (mProximityService != null) {
                        mBleManager.closeProfileServiceProxy(
                                BleProfile.BLE_PROFILE_PXP, mProximityService);
                        mProximityService = null;
                    }
                    if (mAlertProfileService != null) {
                        mBleManager.closeProfileServiceProxy(
                                BleProfile.BLE_PROFILE_ANP, mAlertProfileService);
                        mAlertProfileService = null;
                    }
                    if (mBleDeviceManager != null) {
                        mBleManager.closeDeviceManager(mBleDeviceManager);
                        mBleDeviceManager = null;
                    }
//                    if (mProfileServiceManager != null) {
//                        mBleManager.closeProfileServiceManager(mProfileServiceManager);
//                    }
                }
            }
            if (mProfileServiceManager != null) {
                Log.d(TAG, "[onAdapterStateChanged] call shutdown service");
                mProfileServiceManager.shutdownServices();
            } else {
                Log.d(TAG, "[onAdapterStateChanged] STATE_OFF mProfileServiceManager is null");
            }
            if (mCachedBleGattDevice != null) {
                mCachedBleGattDevice.clearData();
                mCachedBleGattDevice = null;
            }
        }
    }
    
    /**
     * 
     * @param started
     */
    void onAdapterScanningStateChanged(boolean started) {
        for (BluetoothCallback.BluetoothAdapterState callback : mAdapterCallbacks) {
            callback.onBluetoothScanningStateChanged(started);
        }
    }
    
    /**
     * When remote device scanned, add the device to the list view which will show in
     * alert dialog
     * @param device
     */
    private void onDeviceAdded(BluetoothDevice device) {
        for (BluetoothCallback.BluetoothLEDeviceScanned callback : mScannedCallbacks) {
            callback.onScannedBleDeviceAdded(device);
        }
    }
    
    /**
     * When remote device disappeared, delete the device from the list view which will show in
     * alert dialog
     * @param device
     */
    void onDeviceDeleted(BluetoothDevice device) {
        for (BluetoothCallback.BluetoothLEDeviceScanned callback : mScannedCallbacks) {
            callback.onScannedBleDeviceRemoved(device);
        }
    }
    
    /**
     * initialization FMP & PXP profile interface which used to register a callback
     * to receive profile action.
     * 
     * @param device
     */
    private void initProfileInterfaces(BleGattDevice device) {
        if (device == null) {
            Log.d(TAG, "[initProfileInterfaces] device is null");
            return;
        }
        if (device.getService(BleGattUuid.Service.IMMEDIATE_ALERT) != null) {
            BleFindMeProfile fmpProfile =
                (BleFindMeProfile)device.asProfileInterface(BleProfile.BLE_PROFILE_FMP);
            if (fmpProfile == null) {
                Log.d(TAG, "[initProfileInterfaces] fmpProfile is null");
                return;
            }
//            fmpProfile.registerProfileCallback(mFmpProfileCallback);
        }
    }
    
    /**
     * Find target device according to {@link BluetoothDevice}.
     * 
     * if the device is not in cached device manager, cann't find the target device
     * if the device is not support FMP profile, cann't find the target device
     * 
     * if the device is alerted, {@link mFmpProfileCallback.onTargetAlerted} will be called
     * 
     * @param level should only be BleFindMeProfile.LEVEL_NO,BleFindMeProfile.LEVEL_MIDDLE,
     *          BleFindMeProfile.LEVEL_HIGH.
     * 
     * @param device {@link BluetoothDevice}
     */
    public void findTargetDevice(int level, BluetoothDevice device) {
        if (level != BleFindMeProfile.LEVEL_NO && level != BleFindMeProfile.LEVEL_MILD && 
                level != BleFindMeProfile.LEVEL_HIGH) {
            Log.d(TAG, "[findTargetDevice] level is wrong defination");
            return;
        }
        if (device == null) {
            Log.d(TAG, "[findTargetDevice] device is null");
            return;
        }
        CachedBluetoothLEDevice cachedDevice = this.mCachedDeviceManager.findDevice(device);
        if (cachedDevice == null) {
            Log.d(TAG, "[findTargetDevice] cachedDevice is null");
            return;
        }
        // TODO this maybe has a error, if the service list is empty
        //      or the service has not been found
        if (!cachedDevice.isSupportFmp()) {
            Log.d(TAG, "[findTargetDevice] cachedDevice is not support FMP");
            return;
        }
        if (cachedDevice.getConnectionState() != BleGattDevice.STATE_CONNECTED) {
            Log.d(TAG, "[findTargetDevice] cachedDevice is not in connected state, return");
            return;
        }
        BleGattDevice gattDevice =
            mCachedBleGattDevice.getBleGattDevice(mBleDeviceManager, device, mGattDeviceCallback);
        if (gattDevice == null) {
            Log.e(TAG, "[findTargetDevice] gattDevice is null");
            return;
        }
        BleFindMeProfile fmpProfile =
            (BleFindMeProfile)gattDevice.asProfileInterface(BleProfile.BLE_PROFILE_FMP);
        if (fmpProfile == null) {
            Log.d(TAG, "[findTargetDevice] fmpProfile is null");
            return;
        }
        Log.d(TAG, "[findTargetDevice] start to find gatt device");
        if (level == BleFindMeProfile.LEVEL_HIGH) {
            Log.d(TAG, "[findTargetDevice] level is LEVEL_HIGH");
            cachedDevice.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, true);
        } else if (level == BleFindMeProfile.LEVEL_NO) {
            Log.d(TAG, "[findTargetDevice] level is LEVEL_NO");
            cachedDevice.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, false);
        }
        fmpProfile.findTarget(level);
    }
    
    /**
     * Which used to stop remote device alert, which will be used in PXP service
     * 
     * @param device
     */
    public void stopRemoteDeviceAlert(BluetoothDevice device) {
        if (device == null) {
            Log.d(TAG, "[stopRemoteDeviceAlert] device is null");
            return;
        }
        if (mProximityService == null) {
            Log.d(TAG, "[stopRemoteDeviceAlert] mProximityService is null");
            return;
        }
        boolean b = mProximityService.stopRemoteAlert(device);
        Log.d(TAG, "[stopRemoteDeviceAlert] return result : " + b);
    }
    
    private void updateDevicePxpState(CachedBluetoothLEDevice cachedDevice) {
        if (cachedDevice == null) {
            Log.d(TAG, "[updateDevicePxpState] cachedDevice is null");
            return;
        }
        if (mProximityService == null) {
            Log.d(TAG, "[updateDevicePxpState] mProximityService is null");
            return;
        }
        if (cachedDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
            if (cachedDevice.isSupportPxpOptional()) {
                cachedDevice.onDevicePxpAlertStateChange(mProximityService
                        .queryAlertStatus(cachedDevice.getDevice()));
                cachedDevice.setIntAttribute(
                        CachedBluetoothLEDevice.DEVICE_CURRENT_TX_POWER_FLAG,
                        mProximityService.queryDistanceValue(cachedDevice.getDevice()));

                if (!mPxpCallbackMap.containsKey(cachedDevice.getDevice())) {
                    boolean b = mProximityService.registerStatusChangeCallback(
                            cachedDevice.getDevice(),
                            mPxpProfileServiceCallback);
                    if (b) {
                        Log.d(TAG, "[updateDevicePxpState] add to hashmap");
                        mPxpCallbackMap.put(cachedDevice.getDevice(),
                                mPxpProfileServiceCallback);
                    }
                }
            }
        }
        
        BleProximityProfileService.DevicePxpParams param =
            mProximityService.getPxpParameters(cachedDevice.getDevice());
        if (param != null) {
            cachedDevice.setBooleanAttribute(
                CachedBluetoothLEDevice.DEVICE_ALERT_SWITCH_ENABLER_FLAG,
                param.mAlertEnabler == 0 ? false : true);
            cachedDevice.setBooleanAttribute(
                CachedBluetoothLEDevice.DEVICE_DISCONNECTION_WARNING_EANBLER_FLAG,
                param.mDisconnEnabler == 0 ? false : true);
            cachedDevice.setIntAttribute(
                CachedBluetoothLEDevice.DEVICE_IN_OUT_RANGE_ALERT_FLAG,
                param.mRangeType);
            cachedDevice.setBooleanAttribute(
                CachedBluetoothLEDevice.DEVICE_RANGE_ALERT_ENABLER_FLAG,
                param.mRangeAlertEnabler == 0 ? false : true);
            cachedDevice.setIntAttribute(
                CachedBluetoothLEDevice.DEVICE_RANGE_VALUE_FLAG,
                param.mRangeValue);
        } else {
            Log.d(TAG, "[updateDevicePxpState] getPxpParameters returns null");
        }
    }
    
//    public boolean isPxpProfileServiceReady() {
//        if (mProximityService == null) {
//            return false;
//        }
//        return true;
//    }
//    
//    public boolean isAnpProfileServiceReady() {
//        if (mAlertProfileService == null) {
//            return false;
//        }
//        return true;
//    }
    
    void updateAnpData(CachedBluetoothLEDevice device) {
        if (device == null) {
            Log.d(TAG, "[updateAnpData] device is null");
            return;
        }
        if (mAlertProfileService == null) {
            Log.d(TAG, "[updateAnpData] mAlertProfileService is null");
            return;
        }
        SparseIntArray hostSparce = mAlertProfileService.getDeviceSettings(
                device.getDevice().getAddress(), CATEGORY_IDS);
        SparseIntArray remoteSparce = mAlertProfileService.getRemoteSettings(
                device.getDevice().getAddress(), CATEGORY_IDS);
        if (hostSparce == null) {
            Log.d(TAG, "[updateAnpData] getDeviceSettings return null");
            return;
        }
        if (hostSparce.size() == 0) {
            Log.d(TAG, "[updateAnpData] getDeviceSettings return size is 0");
            return;
        }
        
        Log.d(TAG, "[updateAnpData] updater anp host data");
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG,
                hostSparce.get(this.CATEGORY_ID_INCOMING_CALL) == 0 ? false : true);
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG,
                hostSparce.get(this.CATEGORY_ID_MISSED_CALL) == 0 ? false : true);
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG,
                hostSparce.get(this.CATEGORY_ID_SMS) == 0 ? false : true);
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG,
                hostSparce.get(this.CATEGORY_ID_EMAIL) == 0 ? false : true);

        if (remoteSparce == null) {
            Log.d(TAG, "[updateAnpData] getRemoteSettings return null");
            return;
        }
        if (remoteSparce.size() == 0) {
            Log.d(TAG, "[updateAnpData] getRemoteSettings return size is 0");
            return;
        }
        Log.d(TAG, "[updateAnpData] update anp remote data");
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_REMOTE_INCOMING_CALL_FLAGE,
                remoteSparce.get(this.CATEGORY_ID_INCOMING_CALL) == 0 ? false : true);
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_REMOTE_MISSED_CALL_FLAGE,
                remoteSparce.get(this.CATEGORY_ID_MISSED_CALL) == 0 ? false : true);
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_MESSAGE_FLAGE,
                remoteSparce.get(this.CATEGORY_ID_SMS) == 0 ? false : true);
        device.setBooleanAttribute(CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_EMAIL_FLAGE,
                remoteSparce.get(this.CATEGORY_ID_EMAIL) == 0 ? false : true);

    }

    /**
    * used to update UX configuration to ans table
    */
    private void updateAnpDataToClientTable(CachedBluetoothLEDevice cachedDevice,
                int whichAttribute) {
        if (cachedDevice == null) {
            Log.d(TAG, "[updateAnpDataToClientTable] cachedDevice is null");
            return;
        }
        if (cachedDevice.getDevice() == null) {
            Log.d(TAG, "[updateAnpDataToClientTable] cachedDevice.getDevice is null");
            return;
        }
        if (mAlertProfileService == null) {
            Log.d(TAG, "[updateAnpDataToClientTable] mAlertProfileService is null");
            return;
        }
        int whichCategory = -1;
        int value = 0;
        switch (whichAttribute) {
        case CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG:
            whichCategory = CATEGORY_ID_INCOMING_CALL;
            value = cachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG) ? 3 : 0;
            break;

        case CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG:
            whichCategory = CATEGORY_ID_MISSED_CALL;
            value = cachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG) ? 3 : 0;
            break;

        case CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG:
            whichCategory = CATEGORY_ID_SMS;
            value = cachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG) ? 3 : 0;
            break;

        case CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG:
            whichCategory = CATEGORY_ID_EMAIL;
            value = cachedDevice.getBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG) ? 3 : 0;
            break;

        default:
            break;
        }
        if (whichCategory == -1) {
            Log.d(TAG, "[updateAnpDataToClientTable] whichCategory is -1, not recognize");
            return;
        }
        SparseIntArray s = new SparseIntArray();
        s.put(whichCategory, value);
        Log.d(TAG, "[updateAnpDataToClientTable] call to update to client table");
        mAlertProfileService.updateDeviceSettings(
                cachedDevice.getDevice().getAddress(), s);
    }

    
    /**
     * {@link CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener}
     * which used to update the device connection state while the app is first to load
     * if the {@link CachedBleGattDevice} is initialized before than CachedBluetoothLEDeviceManager
     * to add the device, the device state should be update from this callback,
     * which is from {@link BleDeviceManagerService} which is running
     * all the time in the background.
     * 
     * while all cached device connection state has been updated, unregister this callback
     * to release resource.
     * 
     */
    private CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener mCachedListListener =
            new CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener() {
                
                @Override
                public void onDeviceRemoved(CachedBluetoothLEDevice device) {
                    // TODO Auto-generated method stub
                    if (mProximityService != null) {
                        mProximityService.unregisterStatusChangeCallback(
                                device.getDevice(), mPxpProfileServiceCallback);
                        mPxpCallbackMap.remove(device.getDevice());
                    }
                    removeGattDevice(device.getDevice());
                    device.unregisterAttributeChangeListener(mDeviceAttributeListener);
                }
                
                @Override
                public void onDeviceAdded(CachedBluetoothLEDevice device) {
                    // TODO Auto-generated method stub
                    
                    if (device == null) {
                        Log.d(TAG, "[onDeviceAdded] device is null");
                        return;
                    }
                    device.registerAttributeChangeListener(mDeviceAttributeListener);
                    // update device connection state and device service list
                    // while add device happened after device manager service connected
                    if (mCachedBleGattDevice != null) {
                        BleGattDevice gattDevice =
                            mCachedBleGattDevice.getBleGattDevice(mBleDeviceManager,
                                    device.getDevice(), mGattDeviceCallback);
                        if (gattDevice != null) {
                            int state = gattDevice.getState();
                            Log.d(TAG, "[onDeviceAdded] state : " + state);
                            device.onConnectionStateChanged(state);
                            if (device.getServiceList().size() == 0) {
                                device.onServiceDiscovered(gattDevice.getServices());
                            }
                            initProfileInterfaces(gattDevice);
                        } else {
                            Log.e(TAG, "[onDeviceAdded] gattDevice is null");
                        }
                    } else {
                        Log.d(TAG, "[onDeviceAdded] mCachedBleGattDevice is null," +
                                " cann't update cached device connect state");
                    }
                    
                    // update device state alert state & distance value from
                    // proximity profile service
                    // while get proximity profile service is before add device.
                    // TODO if device is support pxp or not, maybe change to TxPower and IAS
                    updateDevicePxpState(device);
                    updateAnpData(device);
                }
            };
    
    /**
     * which used to bind {@link BleDeviceManagerService} which is running in the background.
     * after bind success, the {@link onServiceConnected} will be called, then initialize
     * the mBleDeviceManager and {@link CachedBleGattDevice}.
     * 
     * if the mCachedDeviceManager.addDevice is called before this callback.
     * after this callback returned, should update all the cached device connection state.
     * 
     */
    private BleDeviceManager.DeviceManagerListener mDeviceManagerListener =
        new BleDeviceManager.DeviceManagerListener() {
        
        public void onServiceConnected(BleDeviceManager proxy) {
            if (proxy == null) {
                Log.d(TAG, "[onServiceConnected] proxy is null");
                return;
            }
            Log.d(TAG, "[onServiceConnected] set proxy");
            mBleDeviceManager = proxy;
            mCachedBleGattDevice = new CachedBleGattDevice(mContext);
            mDeviceManagerServiceConnected = true;
            notifyServiceConnectionChanged(PROFILE_DEVICE_MANAGER_SERVICE_ID, PROFILE_CONNECTED);

            if (mCachedDeviceManager.getCachedDevicesCopy().size() != 0) {
                for (CachedBluetoothLEDevice device : mCachedDeviceManager.getCachedDevicesCopy()) {
                    BleGattDevice gattDevice =
                        mCachedBleGattDevice.getBleGattDevice(mBleDeviceManager,
                            device.getDevice(), mGattDeviceCallback);
                    if (gattDevice != null) {
                        int state = gattDevice.getState();
                        Log.d(TAG, "[onServiceConnected] update cached device" +
                                    " connection state : " + state);
                        device.onConnectionStateChanged(state);
                        if (device.getServiceList().size() == 0) {
                            device.onServiceDiscovered(gattDevice.getServices());
                        }
                        initProfileInterfaces(gattDevice);
                    } else {
                        Log.e(TAG, "[onServiceConnected] gattDevice is null");
                    }
                }
            }
            if (mConnectingDevices.size() != 0) {
                Log.d(TAG, "[mDeviceManagerListener] start to connect pending devices");
                for (BluetoothDevice device : mConnectingDevices) {
                    BleGattDevice gattDevice =
                        mCachedBleGattDevice.getBleGattDevice(mBleDeviceManager, device, mGattDeviceCallback);
                    if (gattDevice != null) {
                        gattDevice.connect();
                    } else {
                        Log.e(TAG, "[mDeviceManagerListener] pending devices gattDevice is null");
                    }
                }
                mConnectingDevices.clear();
            }
        }
        
        public void onServiceDisconnected() {
            Log.d(TAG, "[onServiceDisconnected] reset proxy");
            mBleDeviceManager = null;
            if (mCachedBleGattDevice != null) {
                mCachedBleGattDevice.clearData();
                mCachedBleGattDevice = null;
            }
            mDeviceManagerServiceConnected = false;
            notifyServiceConnectionChanged(PROFILE_DEVICE_MANAGER_SERVICE_ID, PROFILE_DISCONNECTED);
        }
    };
    
    /**
     * which used to communicate with BleProximityProfie background service
     * 
     * while service connected, call to query distance state, and alert state
     */
    private BleProfileService.ProfileServiceListener mProfileServiceListener =
            new BleProfileService.ProfileServiceListener() {

        public void onServiceConnected(int profile, BleProfileService proxy) {
            if (proxy == null) {
                Log.d(TAG, "[mProfileServiceListener] onServiceConnected, proxy is null");
                return;
            }
            Log.d(TAG, "[mProfileServiceListener] onServiceConnected, profile : " + profile);
            if (profile == BleProfile.BLE_PROFILE_PXP) {
                if (proxy instanceof BleProximityProfileService) {
                	mPxpServiceConnected = true;
                	notifyServiceConnectionChanged(PROFILE_PXP_ID, PROFILE_CONNECTED);
                    Log.d(TAG,
                            "[mProfileServiceListener] onServiceConnected, init mProximityService");
                    mProximityService = (BleProximityProfileService) proxy;
                    
                    // if cached device manager is not empty, try to update device
                    // distance & alert state from pxp service
                    if (mCachedDeviceManager.getCachedDevicesCopy().size() != 0) {
                        Log.d(TAG, "[mProfileServiceListener] onServiceConnected do" +
                            " pxp data update");
                        for (CachedBluetoothLEDevice cachedDevice :
                            mCachedDeviceManager.getCachedDevicesCopy()) {
                            // check device support pxp or not
                            // TODO maybe change to TxPower, and IAS
                            updateDevicePxpState(cachedDevice);
                        }
                    } else {
                        Log.d(TAG, "[mProfileServiceListener] onServiceConnected," +
                            " cached device manager is empty");
                    }
                } else {
                    Log.d(TAG, "[mProfileServiceListener] onServiceConnected," +
                            " proxy is not match BleProximityProfileService");
                }
            } else if (profile == BleProfile.BLE_PROFILE_ANP) {
                if (proxy instanceof BleAlertNotificationProfileService) {
                	mAnpServiceConnected = true;
                    mAlertProfileService = (BleAlertNotificationProfileService) proxy;

                    notifyServiceConnectionChanged(PROFILE_ANP_ID, PROFILE_CONNECTED);
                    if (mCachedDeviceManager.getCachedDevicesCopy().size() != 0) {
                        Log.d(TAG, "[mProfileServiceListener] onServiceConnected do" +
                            " anp data initialization");
                        for (CachedBluetoothLEDevice cachedDevice :
                            mCachedDeviceManager.getCachedDevicesCopy()) {
                            updateAnpData(cachedDevice);
                        }
                    } else {
                        Log.d(TAG, "[mProfileServiceListener] onServiceConnected, " +
                            "cached device is 0");
                    }
                } else {
                    Log.d(TAG, "[mProfileServiceListener] onServiceConnected," +
                        " proxy is not match BleAlertNotificationProfileService");
                }
            } else {
                Log.d(TAG, "[mProfileServiceListener] onServiceConnected," +
                        "profile not match PXP & ANP");
            }
            
        }

        public void onServiceDisconnected(int profile) {
            if (profile == BleProfile.BLE_PROFILE_PXP) {
            	mPxpServiceConnected = false;
            	notifyServiceConnectionChanged(PROFILE_PXP_ID, PROFILE_DISCONNECTED);
                mProximityService = null;
                mPxpCallbackMap.clear();
            } else if (profile == BleProfile.BLE_PROFILE_ANP) {
            	mAnpServiceConnected = false;
            	notifyServiceConnectionChanged(PROFILE_ANP_ID, PROFILE_DISCONNECTED);
                mAlertProfileService = null;
            }
        }
    };
    
    /**
     * mServiceManagerListener which to used to make mProfileServiceManager to be proxy
     * while the service is connected, do launch service action.
     * while the service is disconnected, so shutdown service action.
     */
    private BleProfileServiceManager.ProfileServiceManagerListener mServiceManagerListener =
            new BleProfileServiceManager.ProfileServiceManagerListener() {
        public void onServiceConnected(BleProfileServiceManager proxy) {
            if (proxy == null) {
                Log.e(TAG, "[mServiceManagerListener] onServiceConnected service" +
                            " manager proxy is null");
                return;
            }
            Log.d(TAG, "[mServiceManagerListener] onServiceConnected set service manager proxy");
            mProfileServiceManager = proxy;
            
            if (mProfileServiceManager != null) {
                if (mLocalAdapter.getBluetoothState() == BluetoothAdapter.STATE_ON) {
                    Log.d(TAG, "[mServiceManagerListener] onServiceConnected" +
                            " do launch service action");
                    mProfileServiceManager.launchServices();
                } else {
                    Log.d(TAG, "[mServiceManagerListener] BT is off, do not launch service");
                }
            }
        }
        
        public void onServiceDisconnected(BleProfileServiceManager proxy) {
            Log.d(TAG, "[mServiceManagerListener] onServiceDisconnected set" +
                        " service manager to be null");
            mProfileServiceManager = null;
        }
    };

    private void addGattDevice(BluetoothDevice device) {
        if (device == null) {
            Log.d(TAG, "[addGattDevice] device is null");
            return;
        }
        if (this.mBleDeviceManager != null) {
            Log.d(TAG, "[addGattDevice] call to add device");
            mBleDeviceManager.addGattDevice(device);
        }
    }

    private void removeGattDevice(BluetoothDevice device) {
        if (device == null) {
            Log.d(TAG, "[removeGattDevice] device is null");
            return;
        }
        if (this.mBleDeviceManager != null) {
            Log.d(TAG, "[removeGattDevice] call to delete device");
            mBleDeviceManager.deleteGattDevice(device);
        }
    }

    public void updateClientPxpData(BluetoothDevice device,
            boolean alertEnabler, boolean rangeAlertEnabler,
            boolean disconnectWarningEnabler,
            int rangeType, int rangeValue) {
        if (device == null) {
            Log.d(TAG, "[updateClientPxpData] device is null");
            return;
        }
        Log.d(TAG, "[updateClientPxpData] alertEnabler : " + alertEnabler
              + ", rangeAlertEnabler : " + rangeAlertEnabler +
              ", disconnectWarningEnabler : " + disconnectWarningEnabler
              + ", rangeType : " + rangeType + ", rangeValue : " + rangeValue);
        if (mProximityService != null) {
            mProximityService.setPxpParameters(device, alertEnabler ? 1 : 0,
                rangeAlertEnabler ? 1 : 0, rangeType, rangeValue,
                disconnectWarningEnabler ? 1 : 0);
        } else {
            Log.d(TAG, "[updateClientPxpData] mProximityService is null, cann't update pxp data");
        }
    }
    
    /**
     * ble gatt device callback
     * used to update device connection state & service discovered callback
     * while connection state has been changed, if the device is not in cache,
     * should add the device to cache, and update state to be actual connection state
     * 
     * if the device is in cache, should only update the device connection state.
     * 
     * service discovery callback, while the connection state is connected, begin
     * to discovery the device services, after discovered, the callback {@link onServicesDiscovered}
     * will be called. and then update the services to be the cached device.
     * 
     */
    private BleGattDevice.BleGattDeviceCallback mGattDeviceCallback = 
        new BleGattDevice.BleGattDeviceCallback() {

        /**
         * device connection state change callback.
         * 
         * @param gattDevice
         * @param status
         * @param newState
         */
        public void onConnectionStateChange(BleGattDevice gattDevice,
                int status, int newState) {
            if (gattDevice == null) {
                Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange] gattDevice is null");
                return;
            }
            Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange] device address : "
                    + gattDevice.getDevice());
            Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange] status : "
                    + status + ", newState : " + newState);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange] status is GATT_SUCCESS");
                BluetoothDevice device = gattDevice.getDevice();
                if (device == null) {
                    Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                " gattDevice is null!!");
                    return;
                }

                if (newState == BleGattDevice.STATE_CONNECTED) {
                    Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                            " new state is STATE_CONNECTED");
                    if (mCachedDeviceManager.findDevice(device) == null) {
                        if (mConnectionMap.containsKey(device)) {
                            Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                    " call to add device to cacher");
                            mCachedDeviceManager.addDevice(device, mConnectionMap.get(device));
                        } else {
                            Log.e(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                        " device is not in map");
                            return;
                        }
                    }
                    mCachedDeviceManager.onDeviceConnectionStateChanged(
                            device, BleGattDevice.STATE_CONNECTED);
                    Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                " start to discover device services");
                    gattDevice.discoverServices();
                    CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
                    if (cachedDevice != null) {
                        cachedDevice.setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_AUTO_CONNECT_FLAG, true);
                        updateDevicePxpState(cachedDevice);
                    }
                } else if (newState == BleGattDevice.STATE_DISCONNECTED) {
                    Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                            " new state is STATE_DISCONNECTED");
                    CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
                    if (cachedDevice == null) {
                        Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                " call to remove gatt device in FWK");
                        removeGattDevice(device);
                    } else {
                        Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                " update cache device connection state to be STATE_DISCONNECTED");
                        mCachedDeviceManager.onDeviceConnectionStateChanged(
                                device, BleGattDevice.STATE_DISCONNECTED);
                        if (mProximityService != null) {
                            mProximityService.unregisterStatusChangeCallback(
                                    device, mPxpProfileServiceCallback);
                            mPxpCallbackMap.remove(device);
                        }
                    }
                    if (mConnectionMap.containsKey(device)) {
                        mConnectionMap.remove(device);
                    }
                    if (gattDevice != null) {
                        Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                                    " call to close gatt device");
                        gattDevice.close();
                        if (mCachedBleGattDevice != null) {
                            mCachedBleGattDevice.removeDevice(device);
                        }
                    }
                } else {
                    mCachedDeviceManager.onDeviceConnectionStateChanged(device, newState);
                }
                notifyConnectionStateChange(device, newState);
            } else if (status == BluetoothGatt.GATT_FAILURE) {
                Log.d(TAG, "[mGattDeviceCallback.onConnectionStateChange]" +
                            " status return GATT_FAILURE!!");
            }
        }
    
        /**
         * service discovery callback
         * 
         * @param gattDevice
         * @param status
         */
        public void onServicesDiscovered(BleGattDevice gattDevice, int status) {
            if (gattDevice == null) {
                Log.d(TAG, "[mGattDeviceCallback.onServicesDiscovered] gatt device is null");
                return;
            }
            BluetoothDevice device = gattDevice.getDevice();
            if (device == null) {
                Log.d(TAG, "[mGattDeviceCallback.onServicesDiscovered] device is null");
                return;
            }
            if (status == BluetoothGatt.GATT_SUCCESS) {
                List<BluetoothGattService> services = gattDevice.getServices();
                if (services == null || services.size() == 0) {
                    Log.d(TAG, "[mGattDeviceCallback.onServicesDiscovered]" +
                                " service list is null or empty");
                    return;
                }
                mCachedDeviceManager.onDeviceServiceDiscoveried(device, services);
                CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
                updateDevicePxpState(cachedDevice);
                initProfileInterfaces(gattDevice);
            } else {
                Log.d(TAG, "[mGattDeviceCallback.onServicesDiscovered] status : GATT_FAILURE");
            }
        }
    };
    
//    private BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
//
//        @Override
//        public void onConnectionStateChange(BluetoothGatt gatt, int status,
//                int newState) {
//            // TODO Auto-generated method stub
//            Log.d(TAG, "[mGattCallback.onConnectionStateChange] status : "
//                  + status +", newState : " + newState);
//            if (status == BluetoothGatt.GATT_SUCCESS) {
//                if (newState == BluetoothGatt.STATE_CONNECTED) {
//                    Log.d(TAG, "[mGattCallback.onConnectionStateChange]
//                          new state is BluetoothGatt.STATE_CONNECTED");
//                    BluetoothDevice device = gatt.getDevice();
//                    if (device == null) {
//                        Log.d(TAG, "[mGattCallback.onConnectionStateChange] device is null!!");
//                        return;
//                    }
//                    GattConnection conn = mConnectionMap.get(device);
//
//                    if (mCachedDeviceManager.findDevice(device) == null) {
//                        mCachedDeviceManager.addDevice(device, conn.mLocationIndex);
//                    }
//                    mCachedDeviceManager.onDeviceConnectionStateChanged(device, newState);
//                    Log.d(TAG, "[mGattCallback.onConnectionStateChange]
//                              start to discovery services");
//                    gatt.discoverServices();
//                } else if (newState == BluetoothGatt.STATE_DISCONNECTED) {
//                    Log.d(TAG, "[mGattCallback.onConnectionStateChange]
//                        new state is BluetoothGatt.STATE_DISCONNECTED");
//                    BluetoothDevice device = gatt.getDevice();
//                    if (device == null) {
//                        Log.d(TAG, "[mGattCallback.onConnectionStateChange] device is null!!");
//                        return;
//                    }
//                    mCachedDeviceManager.onDeviceConnectionStateChanged(device, newState);
//                }
//            } else if (status == BluetoothGatt.GATT_FAILURE) {
//                Log.d(TAG, "[mGattCallback.onConnectionStateChange] status is failure");
//            }
//        }
//
//        @Override
//        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
//            // TODO Auto-generated method stub
//            Log.d(TAG, "[onServicesDiscovered] status : " + status);
//            if (status == BluetoothGatt.GATT_SUCCESS) {
//                BluetoothDevice device = gatt.getDevice();
//                if (device == null) {
//                    Log.d(TAG, "[onServicesDiscovered] device is null");
//                    return;
//                }
//                List<BluetoothGattService> services = gatt.getServices();
//                if (services == null || services.size() == 0) {
//                    Log.d(TAG, "[onServicesDiscovered] service is null or empty");
//                    return;
//                }
//                Log.d(TAG, "[mGattCallback.onConnectionStateChange]
//                        service discoveried, and is not empty");
//                mCachedDeviceManager.onDeviceServiceDiscoveried(device, services);
//            } else if (status == BluetoothGatt.GATT_FAILURE) {
//                Log.d(TAG, "[onServicesDiscovered] gatt status is BluetoothGatt.GATT_FAILURE");
//            }
//        }
//        
//    };
    
    
    private class ProximityProfileServiceCallback extends IProximityProfileServiceCallback.Stub {
        
        public void onAlertStatusChange(String address, boolean isAlert) {
            if (address == null || address.trim().length() == 0) {
                Log.d(TAG, "[onDistanceValueChange] address is null or empty");
                return;
            }
            Log.d(TAG, "[onAlertStatusChange] address : " + address + ", isAlert : " + isAlert);
            BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
            if (adapter == null) {
                Log.d(TAG, "[onAlertStatusChange] adapter is null");
                return;
            }
            BluetoothDevice device = adapter.getRemoteDevice(address);
            if (device == null) {
                Log.d(TAG, "[onAlertStatusChange] device is null");
                return;
            }
            CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                Log.d(TAG, "[onAlertStatusChange] cachedDevice is null");
                return;
            }
            cachedDevice.onDevicePxpAlertStateChange(isAlert);
        }
        
        public void onDistanceValueChange(String address, int value) {
            if (address == null || address.trim().length() == 0) {
                Log.d(TAG, "[onDistanceValueChange] address is null or empty");
                return;
            }
            Log.d(TAG, "[onDistanceValueChange] address : " + address + ", value : " + value);
            BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
            if (adapter == null) {
                Log.d(TAG, "[onDistanceValueChange] adapter is null");
                return;
            }
            BluetoothDevice device = adapter.getRemoteDevice(address);
            if (device == null) {
                Log.d(TAG, "[onDistanceValueChange] device is null");
                return;
            }
            CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                Log.d(TAG, "[onDistanceValueChange] cachedDevice is null");
                return;
            }
            cachedDevice.setIntAttribute(
                    CachedBluetoothLEDevice.DEVICE_CURRENT_TX_POWER_FLAG, value);
        }
    };
    
    
    /**
     * find me profile callback, if the device is alerted should update the cached device
     * fmp state
     */
    private BleFindMeProfile.ProfileCallback mFmpProfileCallback =
            new BleFindMeProfile.ProfileCallback() {
        public void onTargetAlerted(BluetoothDevice device, boolean isSuccess) {
            Log.d(TAG, "[onTargetAlerted] isSuccess : " + isSuccess);
            if (device == null) {
                Log.d(TAG, "[onTargetAlerted] device is null");
                return;
            }
            CachedBluetoothLEDevice cachedDevice = mCachedDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                Log.e(TAG, "[onTargetAlerted] cachedDevice is null");
                return;
            }
            cachedDevice.setBooleanAttribute(
                    CachedBluetoothLEDevice.DEVICE_FMP_STATE_FLAG, isSuccess);
        }
    };

    private CachedBluetoothLEDevice.DeviceAttributeChangeListener mDeviceAttributeListener
        = new CachedBluetoothLEDevice.DeviceAttributeChangeListener() {

            @Override
            public void onDeviceAttributeChange(CachedBluetoothLEDevice device, int which) {
                // TODO Auto-generated method stub
                if (which != CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG
                        && which != CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG
                        && which != CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG
                        && which != CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG) {
                    Log.d(TAG, "[mDeviceAttributeListener] which is no need to update");
                    return;
                }
                Log.d(TAG, "[mDeviceAttributeListener] update anp configuration data");
                updateAnpDataToClientTable(device, which);
            }
        
    };
    
    /**
     * used to get scan le device callback.while ble device scanned, the callback will be called.
     */
    private BluetoothAdapter.LeScanCallback mLeCallback = new BluetoothAdapter.LeScanCallback() {

        @Override
        public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
            String name = device.getName();
            Log.d(TAG, "[onLeScan] name : " + name);
            Log.d(TAG, "[onLeScan] address : " + device.getAddress());
            onDeviceAdded(device);
        }
    };
}
