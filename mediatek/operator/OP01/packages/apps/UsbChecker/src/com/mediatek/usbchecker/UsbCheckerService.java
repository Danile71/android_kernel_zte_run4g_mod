package com.mediatek.usbchecker;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.IBinder;
import android.provider.Settings;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.usbchecker.UsbCheckerConstants;

public class UsbCheckerService extends Service{

    private static final String TAG = "UsbChecker/Service";

    private static final String SIM_INTENT = "android.intent.action.SIM_STATE_CHANGED";
    private static final String USB_INTENT = "android.hardware.usb.action.USB_STATE";
    private static final String BOOT_INTENT = "android.intent.action.BOOT_COMPLETED";
    public static final String PROPERTY_USB_ACTIVATION = "persist.sys.usb.activation";
    public static final String PROPERTY_USB_CHARGING = "sys.usb.charging";

    // whether plugged in with USB cable 
    private boolean mUsbPlugin = false;
    
    private static  boolean mBlockingByOOBE = false;

    public static final int simNumber;

    static {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                simNumber = 4;
            } else if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                simNumber = 3;
            } else {
                simNumber = 2;
            }
        } else {
            simNumber = 1;
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        if (getActivateState()) {
            Log.d(TAG, "onStartCommand()  Already activation, Return directly.");
            return START_NOT_STICKY;
        }

        String action = intent.getAction();
        Log.d(TAG, "startService with intent:" + action);

        if (SIM_INTENT.equals(action)) {

            String state = intent.getStringExtra("ss");
            int slotId = intent.getIntExtra("simId", -1);
            Log.d(TAG, "Sim Intent extra. state=" + state + ". slotId=" + slotId);
            if (slotId == -1 || state.equals("NOT_READY")) {
                //do nothing.
                Log.d(TAG, "Do noting");
            } else if (!state.equals("ABSENT") && !state.equals("UNKNOWN")) {
                if (true == mUsbPlugin) {
                    setActivateState();
                    enableUsb();
                    finishAlertActivity();
                }
            }
        } else if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {
            int plugType = intent.getIntExtra("plugged", 0);
            Log.d(TAG, "Usb Intent extra. plugType=" + plugType);

            boolean isPluggin = plugType == 0 ? false : true;

            if (isPluggin == mUsbPlugin) {
                //Do nothing for USB plug in state not change.
            } else if (0 == plugType) {
                //close activity.
                mUsbPlugin = false;
                finishAlertActivity();
            } else {
                mUsbPlugin = true;

                if (hasIccCard()) {
                    setActivateState();
                    enableUsb();
                } else {
                    startAlertActivity();
                }
            }
        } else if (USB_INTENT.equals(action)) {
            if (isUsbEnabled()) {
                disableUsb();
            }
        } else if (BOOT_INTENT.equals(action)) {
            if (isUsbEnabled()) {
                disableUsb();
            }
        } else if (UsbCheckerConstants.INTENT_IPO_SHUTDOWN.equals(action)) {
            Log.i(TAG, "IPO shutdown, reset mUsbPlugin false.");
            mUsbPlugin = false;
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate()");

        BroadcastReceiver mPluginFilterReceiver = new UsbCheckerReceiver();

        IntentFilter pluginFilter = new IntentFilter();
        pluginFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(mPluginFilterReceiver, pluginFilter);

    }

    /**
    * Activate when pluged in and also has sim
    */
    private void setActivateState() {
        SystemProperties.set(PROPERTY_USB_ACTIVATION, "yes");
        Intent intent = new Intent(UsbCheckerConstants.INTENT_USB_ACTIVATION);
        sendBroadcast(intent);
        Log.d(TAG, "setActivateState() yes and sendBroadCast:" + UsbCheckerConstants.INTENT_USB_ACTIVATION);
    }

    /**
     * PROPERTY_USB_ACTIVATION: whether has sim when plugged in with USB cable before.
     * Property will be reset if factory reset or reinstall system.
     * @return yes - has sim once; no - never has
     */
    static public boolean getActivateState() {
        String activate = SystemProperties.get(PROPERTY_USB_ACTIVATION, "no");
        Log.d(TAG, "getActivateState=" + activate);
        if (activate.equals("yes")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * PROPERTY_USB_CHARGING: whether is charging only, will be reset when power on
     * @return yes - only charging; no - charing and could communicate
     */
    private void enableUsb() {
        Log.d(TAG, "enableUsb()");
        SystemProperties.set(PROPERTY_USB_CHARGING, "no");
        //Toast.makeText(this, "enableUsb()", Toast.LENGTH_SHORT).show();
    }

    /**
     * Set to only charging (disable communication)
     */
    private void disableUsb() {
        Log.d(TAG, "disableUsb()");
        SystemProperties.set(PROPERTY_USB_CHARGING, "yes");
        //Toast.makeText(this, "disableUsb()", Toast.LENGTH_SHORT).show();
    }

    /**
     * Set to charging and enable communication
     */
    private boolean isUsbEnabled() {
        String ret = SystemProperties.get(PROPERTY_USB_CHARGING, "no");
        Log.d(TAG, "isUsbEnabled()=" + ret);
        if (ret.equals("no")) {
            return true;
        } else {
            return false;
        }
    }

    private ContentObserver mOobeObserver = new ContentObserver(new Handler()) {

    	@Override
        public void onChange(boolean selfChange) {
            int oobe = Settings.System.getInt(getContentResolver(), "oobe_has_run", 0);
            Log.d(TAG, "OOBE on change:" + oobe);
            if (oobe == 1) {
                getContentResolver().unregisterContentObserver(mOobeObserver);
                Log.d(TAG, "OOBE on change. plugin state:" + mUsbPlugin);
                Log.d(TAG, "OOBE on change. mBlockingByOOBE:" + mBlockingByOOBE);
                if (true == mUsbPlugin && mBlockingByOOBE) {
                    Intent activityIntent  = new Intent();
                    activityIntent.setClassName("com.mediatek.usbchecker", 
                            "com.mediatek.usbchecker.UsbCheckerActivity");
                    activityIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(activityIntent);
                    //Toast.makeText(UsbCheckerService.this, "alert delay displayed !!!!!", Toast.LENGTH_LONG).show();
                }
                mBlockingByOOBE = false;
            }            
        }
    }; 

    private void startAlertActivity() {
        Log.d(TAG, "startAlertActivity()");

        try {
            ApplicationInfo info = getPackageManager().getApplicationInfo("com.mediatek.oobe", 0);
            Log.d(TAG, "find OOBE packages =" + info);
            if (info != null) {
                int oobe = Settings.System.getInt(getContentResolver(), "oobe_has_run", 0);
                Log.d(TAG, "oobe state=" + oobe);
                if (oobe == 0) {
                    Log.d(TAG, "blocking alert activity by waiting OOBE");
                    mBlockingByOOBE = true;
                    //Toast.makeText(this, "alert blocking by OOBE", Toast.LENGTH_LONG).show();
                    getContentResolver().registerContentObserver(
                            Settings.System.getUriFor("oobe_has_run"), true, mOobeObserver);
                    return;
                }
            }
        } catch(NameNotFoundException e) {
            Log.d(TAG, "fail to find oobe");
        }
        
        Intent activityIntent  = new Intent();
        activityIntent.setClassName("com.mediatek.usbchecker", 
            "com.mediatek.usbchecker.UsbCheckerActivity");
        activityIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(activityIntent);
    }

    private void finishAlertActivity() {
        Log.d(TAG, "finishAlertActivity()");
        Intent intent = new Intent(UsbCheckerConstants.INTENT_USB_CHECKER_FINISH);
        sendBroadcast(intent);
    }

    private boolean hasIccCard() {
       
        try {
            TelephonyManagerEx telephonyManagerEx = new TelephonyManagerEx(this);
            if (telephonyManagerEx == null) {
                Log.e(TAG, "TelephonyManagerEx is null");
                return false;
            }
            for (int i = 0 ; i <  simNumber ; i++) {
                if (telephonyManagerEx.hasIccCard(i)) {
                    Log.d(TAG, "Slot(" + i + ") has iccCard");
                    return true;
                }
            }

        } catch (NullPointerException e) {
            Log.e(TAG, "TelephonyManager(Ex) is not ready");
            return false;
        }
        Log.d(TAG, "No card plug ined: " + simNumber);
        return false;
    }
}
