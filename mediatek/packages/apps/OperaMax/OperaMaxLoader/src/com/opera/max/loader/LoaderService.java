package com.opera.max.loader;

import java.util.ArrayList;
import java.util.Locale;
import java.util.Map;
import java.util.HashMap;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.telephony.TelephonyManager;

import com.opera.max.sdk.saving.ISavingService;
import com.opera.max.sdk.saving.IStateListener;
import com.mediatek.common.operamax.ILoaderService;
import com.mediatek.common.operamax.ILoaderStateListener;


import com.opera.max.sdk.SDKUtils;

import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.os.SystemProperties;


import android.util.Log;



public class LoaderService extends Service {

    
    private static final String TAG = "LoaderService";


    private final static int LOAD_PRODUCT_AUTO = 0;
    private final static int LOAD_PRODUCT_OUPENG = 1;
    private final static int LOAD_PRODUCT_OPERA = 2;

    private static final int loadProduct = LOAD_PRODUCT_AUTO;
    
    private static final String EXTRA_SHOW_NOTIFICATION = "EXTRA_SHOW_NOTIFICATION";

    private static final String SAVING_OUPENG_PACKGAGE_NAME = "com.oupeng.max";
    private static final String SAVING_OPERA_PACKGAGE_NAME = "com.opera.max";
    private static final String SAVING_SERVICE_CLASS_NAME = "com.opera.max.sdk.saving.SavingService";

    private String savingPackageName = SAVING_OPERA_PACKGAGE_NAME;

    private static final int SUSPEND_REQUEST_NONE = 0;
    private static final int SUSPEND_REQUEST_START_SAVING = 1;
    private static final int SUSPEND_REQUEST_STOP_SAVING = 2;

    private static final int TUNNEL_STATE = 1;
    private static final int SAVING_STATE = 2;

    private static final int SAVING_SERVICE_EXCEPTION = 4;

    private int suspendRequest = SUSPEND_REQUEST_NONE;
    
    private static final String OPERATER_NONE = "NONE";    
    private static final String OPERATER_CMCC = "OP01";
    private static final String OPERATER_CU = "OP02";
    private static final String OPERATER_CT = "OP09";

    private static final int TUNNEL_STATE_ON = 1;
    private static final int TUNNEL_STATE_OFF = 2;

    private static final int SAVING_STATE_ON = 1;
    private static final int SAVING_STATE_OFF = 2;
    private static final int SAVING_STATE_PAUSE = 3;
    private static final int SAVING_STATE_ABORT = 4;


    private static final int SETTINGS_STATE_ON = 1;
    private static final int SETTINGS_STATE_OFF = 0;


    private static final long DEBUG_SAVING_MODE = 1 << 0;

    private static long DEBUG = DEBUG_SAVING_MODE;

    //private ArrayList<ILoaderStateListener> stateListeners = new ArrayList<ILoaderStateListener>();
    private Map<IBinder, ILoaderStateListener> stateListeners = new HashMap<IBinder, ILoaderStateListener>();

    private final ILoaderService.Stub binder = new ILoaderService.Stub() {
        @Override
        public void startSaving() {
            if (!SDKUtils.isAuthorized()) {
                return;
            }            

//            Settings.System.putInt(getContentResolver(), 
//            Settings.System.DATA_SAVING_KEY,
//            SETTINGS_STATE_ON);

            if(savingService != null) {
            
                try {
                    if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
                        Log.d(TAG, "startSaving M");
                    }
                    savingService.startSaving();

                } catch (RemoteException e) {
                    Log.e(TAG, "startSaving RemoteException");
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection, Context.BIND_AUTO_CREATE);
                suspendRequest = SUSPEND_REQUEST_START_SAVING;
            }

        }

        @Override
        public void stopSaving() {
            if (!SDKUtils.isAuthorized()) {
                return;
            }
//            Settings.System.putInt(getContentResolver(), 
//            Settings.System.DATA_SAVING_KEY,
//            SETTINGS_STATE_OFF);
            if(savingService != null) {
                try {
                    if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
                        Log.d(TAG, "stopSaving M");
                    }
                    savingService.stopSaving();
                    
                } catch (RemoteException e) {
                    Log.e(TAG, "stopSaving RemoteException");
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection, Context.BIND_AUTO_CREATE);
                suspendRequest = SUSPEND_REQUEST_STOP_SAVING;
            }
        }

        @Override
        public int getTunnelState() {
            if (!SDKUtils.isAuthorized()) {
                return -1;
            }

            if(savingService != null) {
                try {
                    return savingService.getTunnelState();
                } catch (RemoteException e) {
                    Log.e(TAG, "getTunnelState RemoteException");
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection, Context.BIND_AUTO_CREATE);
            }

            return 2;
        }

        @Override
        public int getSavingState() {
            if (!SDKUtils.isAuthorized()) {
                return -1;
            }

            if(savingService != null) {
                try {
                    return savingService.getSavingState();
                } catch (RemoteException e) {
                    Log.e(TAG, "getSavingState RemoteException");
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection, Context.BIND_AUTO_CREATE);
            }

            return 2;
        }

        @Override
        public void registerStateListener(ILoaderStateListener listener) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            //stateListeners.add(listener);
            stateListeners.put(listener.asBinder(), listener);
        }

        @Override
        public void unregisterStateListener(ILoaderStateListener listener) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            //stateListeners.remove(listener);
            stateListeners.remove(listener.asBinder());
        }

        @Override
        public void launchOperaMAX() {
            if(savingService != null) {
                try {
                    savingService.launchOperaMAX();
                } catch (RemoteException e) {
                    Log.e(TAG, "launchOperaMAX RemoteException");
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection, Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void addDirectedApp(String packageName) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            if (savingService != null) {
                try {
                    savingService.addDirectedApp(packageName);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void removeDirectedApp(String packageName) {
            if (savingService != null) {
                try {
                    savingService.removeDirectedApp(packageName);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void removeAllDirectedApps() {
            if (savingService != null) {
                try {
                    savingService.removeAllDirectedApps();
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public boolean isAppDirected(String packageName) {
            boolean result = false;
            if (savingService != null) {
                try {
                    result = savingService.isAppDirected(packageName);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
            return result;
        }

        @Override
        public String[] getDirectedAppList() {
            String[] list = null;
            if (savingService != null) {
                try {
                    list = savingService.getDirectedAppList();
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
            return list;
        }

        @Override
        public void addDirectedHost(String host) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            if (savingService != null) {
                try {
                    savingService.addDirectedHost(host);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void removeDirectedHost(String host) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            if (savingService != null) {
                try {
                    savingService.removeDirectedHost(host);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void removeAllDirectedHosts() {
            if (savingService != null) {
                try {
                    savingService.removeAllDirectedHosts();
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public boolean isHostDirected(String host) {
            boolean result = false;
            if (savingService != null) {
                try {
                    result = savingService.isHostDirected(host);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
            return result;
        }

        @Override
        public String[] getDirectedHostList() {
            String[] list = null;
            if (savingService != null) {
                try {
                    list = savingService.getDirectedHostList();
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
            return list;
        }

        @Override
        public void addDirectedHeaderField(String key, String value) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            if (savingService != null) {
                try {
                    savingService.addDirectedHeaderField(key, value);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void removeDirectedHeaderField(String key, String value) {
            if (!SDKUtils.isAuthorized()) {
                return;
            }

            if (savingService != null) {
                try {
                    savingService.removeDirectedHeaderField(key, value);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public void removeAllDirectedHeaderFields() {
            if (savingService != null) {
                try {
                    savingService.removeAllDirectedHeaderFields();
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
        }

        @Override
        public boolean isHeaderFieldDirected(String key, String value) {
            boolean result = false;
            if (savingService != null) {
                try {
                    result = savingService.isHeaderFieldDirected(key, value);
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
            return result;
        }

        @Override
        public String[] getDirectedHeaderFieldList() {
            String[] list = null;
            if (savingService != null) {
                try {
                    list = savingService.getDirectedHeaderFieldList();
                } catch (RemoteException e) {
                }
            }
            else {
                bindService(getSavingServiceIntent(), savingServiceConnection,
                        Context.BIND_AUTO_CREATE);
            }
            return list;
        }
    };

    private ISavingService savingService;

    private IStateListener.Stub stateListener = new IStateListener.Stub() {
        @Override
        public void onTunnelState(int state) {
            notifyStateListener(TUNNEL_STATE, state);
            
            if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
                Log.d(TAG, "onTunnelState ON(1)/OFF(2): " + state);
            }

            if (state == TUNNEL_STATE_ON) {
                //Settings.System.putInt(getContentResolver(), 
                //Settings.System.DATA_SAVING_KEY,
                //SETTINGS_STATE_ON);
            } else if (state == TUNNEL_STATE_OFF){
                //Settings.System.putInt(getContentResolver(), 
                //Settings.System.DATA_SAVING_KEY,
                //SETTINGS_STATE_OFF);
            }
        }

        @Override
        public void onSavingState(int state) {
            notifyStateListener(SAVING_STATE, state);
            
            if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
                Log.d(TAG, "onSavingState ON(1)/OFF(2)/PAUSE(3): " + state);
            }

            if (state == SAVING_STATE_ON || state == SAVING_STATE_PAUSE) {
                Settings.System.putInt(getContentResolver(), 
                Settings.System.DATA_SAVING_KEY,
                SETTINGS_STATE_ON);
            } else if (state == SAVING_STATE_OFF){
                Settings.System.putInt(getContentResolver(), 
                Settings.System.DATA_SAVING_KEY,
                SETTINGS_STATE_OFF);
            }


            
        }
    };

    private ServiceConnection savingServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            savingService = ISavingService.Stub.asInterface(service);
            try {
                savingService.registerStateListener(stateListener);
                notifyStateListener(TUNNEL_STATE, savingService.getTunnelState());
                notifyStateListener(SAVING_STATE, savingService.getSavingState());
                runSuspendRequest();
            } catch(RemoteException e) { 
                Log.e(TAG, "onServiceConnected RemoteException");
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            savingService = null;
            notifyStateListener(SAVING_STATE, SAVING_SERVICE_EXCEPTION);
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        if(binder != null) {
            return binder;
        }
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        selectLoadProduct();
        SDKUtils.Initialize(this);
        bindService(getSavingServiceIntent(), savingServiceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        try {
        
            int state = Settings.System.getInt(getContentResolver(),
                    Settings.System.DATA_SAVING_KEY);

            if (state == 1){                
                suspendRequest = SUSPEND_REQUEST_START_SAVING;
            }
            else {
                suspendRequest = SUSPEND_REQUEST_STOP_SAVING;
            }

        } catch (SettingNotFoundException e) {
            suspendRequest = SUSPEND_REQUEST_STOP_SAVING;
        }
        
        if (savingService != null) {
            runSuspendRequest();
        }
        
        return START_STICKY;

    }

    


    @Override
    public void onDestroy() {
        if(savingService != null) {
            try {
                savingService.unregisterStateListener(stateListener);
                unbindService(savingServiceConnection);
            } catch (RemoteException e) {
                Log.e(TAG, "onDestroy RemoteException");
            }
        }
        savingService = null;
        super.onDestroy();
    }

    private Intent getSavingServiceIntent() {
        Intent intent = new Intent(SAVING_SERVICE_CLASS_NAME);
        intent.setClassName(savingPackageName, SAVING_SERVICE_CLASS_NAME);
        intent.putExtra(EXTRA_SHOW_NOTIFICATION, true);
        return intent;
    }

    private void notifyStateListener(int stateType, int state) {
    
        for (Map.Entry<IBinder, ILoaderStateListener> entry : stateListeners.entrySet()) {
            ILoaderStateListener listener =  entry.getValue();
            try {
                switch (stateType)
                {
                    case TUNNEL_STATE:
                        listener.onTunnelState(state);
                        break;
                    case SAVING_STATE:
                        listener.onSavingState(state);
                        break;
                    default:
                        break;
                }
            } catch (RemoteException e) {
                stateListeners.remove(entry.getKey());
            }
        }


     /*
        ArrayList<ILoaderStateListener> invalidListeners = new ArrayList<ILoaderStateListener>();

        for (int i = 0; i < stateListeners.size(); i++) {
            try {
                switch (stateType)
                {
                    case TUNNEL_STATE:
                        stateListeners.get(i).onTunnelState(state);
                        break;
                    case SAVING_STATE:
                        stateListeners.get(i).onSavingState(state);
                        break;
                    default:
                        break;
                }
            } catch (RemoteException e) {
                invalidListeners.add(stateListeners.get(i));
            }
        }

        //Remove invalid listeners.
        for (int i = 0; i < invalidListeners.size(); i++) {
            stateListeners.remove(invalidListeners.get(i));
        }
        invalidListeners.clear();
        */
    }

    private void selectLoadProduct() {

        String operator = SystemProperties.get("ro.operator.optr", OPERATER_NONE);
        String debug = SystemProperties.get("operamax.debug.select", "0");
        String language = SystemProperties.get("ro.product.locale.language", "");
        String region = SystemProperties.get("ro.product.locale.region", "");

        if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
            Log.d(TAG, "selectLoadProduct()");
            Log.d(TAG, "Operator: " + operator);
            Log.d(TAG, "debug: " + debug);
            Log.d(TAG, "language: " + language);
            Log.d(TAG, "region: " + region);
            Log.d(TAG, "loadProduct: " + loadProduct);
        }




        int selected = LOAD_PRODUCT_AUTO;

        if (debug.equals("1")) {
            selected = LOAD_PRODUCT_OUPENG;
        }
        else if (debug.equals("2")) {
            selected = LOAD_PRODUCT_OPERA;
        }

        if (selected == LOAD_PRODUCT_AUTO) {
            selected = loadProduct;
        }

        if (selected == LOAD_PRODUCT_AUTO) {
            if (OPERATER_CMCC.equals(operator) || OPERATER_CU.equals(operator) || OPERATER_CT.equals(operator)){
                selected = LOAD_PRODUCT_OUPENG;
            } 
            else {
                selected = LOAD_PRODUCT_OPERA;
            }
        }
        
        if (selected == LOAD_PRODUCT_AUTO) {
            if (language.equals("zh") && region.toLowerCase().equals("cn")) {
                selected = LOAD_PRODUCT_OUPENG;
            } else {
                selected = LOAD_PRODUCT_OPERA;
            }
        }

        if (selected == LOAD_PRODUCT_OUPENG) {
            savingPackageName = SAVING_OUPENG_PACKGAGE_NAME;
        }
        else {
            savingPackageName = SAVING_OPERA_PACKGAGE_NAME;
        }
        
        if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
            Log.d(TAG, "savingPackageName: " + savingPackageName);

        }        
    }

    private void runSuspendRequest() {
        switch (suspendRequest) {
            case SUSPEND_REQUEST_START_SAVING:
                try {
                    if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
                        Log.d(TAG, "startSaving in runSuspendRequest");
                    }
                    savingService.startSaving();
                } catch (RemoteException e) {
                    Log.e(TAG, "startSaving RemoteException in runSuspendRequest");
                }
                break;
            case SUSPEND_REQUEST_STOP_SAVING:
                try {
                    if ((DEBUG & DEBUG_SAVING_MODE) != 0) {
                        Log.d(TAG, "stopSaving in runSuspendRequest in runSuspendRequest");
                    }
                    savingService.stopSaving();
                } catch (RemoteException e) {                    
                    Log.e(TAG, "stopSaving RemoteException in runSuspendRequest");
                }
                break;
            default:
                break;
        }
        suspendRequest = SUSPEND_REQUEST_NONE;
    }

}
