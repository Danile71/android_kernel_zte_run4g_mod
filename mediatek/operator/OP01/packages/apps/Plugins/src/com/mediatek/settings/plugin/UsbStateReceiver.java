package com.mediatek.op01.plugin;


import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;

import android.hardware.usb.UsbManager;
import android.os.INetworkManagementService;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.RemoteException;

import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;


public class UsbStateReceiver extends BroadcastReceiver {
    private final static String TAG = "UsbStateReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals(UsbManager.ACTION_USB_STATE)) {
            Log.i(TAG, "receive");
            boolean isUsbConnected = intent.getBooleanExtra(UsbManager.USB_CONNECTED, false);
            INetworkManagementService nwService = INetworkManagementService.Stub.asInterface(
                ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE));
            if (!isUsbConnected) {
                Log.i(TAG, "disconnected, nwservice = " + nwService);
                if (null != nwService) {
                    Log.i(TAG, "nsiot unblock");
                    try {
                        nwService.clearIotFirewall();
                    } catch (RemoteException e) {
                        Log.e(TAG, "nsiot exception");
                    }
                }
            } else if ((SystemProperties.get("gsm.sim.ril.testsim").equals("1")
                        || ((FeatureOption.MTK_GEMINI_SUPPORT)
                           && SystemProperties.get("gsm.sim.ril.testsim.2").equals("1")))
                       && SystemProperties.get("sys.usb.config").contains("acm")) {
                    if (null != nwService) {
                        Log.i(TAG, "nsiot block");
                        try {
                            nwService.setIotFirewall();
                        } catch (RemoteException e) {
                            Log.e(TAG, "nsiot exception");
                        }
                    }
            }
        }
    }
}

