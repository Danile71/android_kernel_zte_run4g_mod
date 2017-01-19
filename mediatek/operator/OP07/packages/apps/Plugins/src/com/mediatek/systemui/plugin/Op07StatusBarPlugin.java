package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

/**
 * M: OP03 implementation of Plug-in definition of Status bar.
 */
public class Op07StatusBarPlugin extends DefaultStatusBarPlugin {
    public Op07StatusBarPlugin(Context context) {
        super(context);
    }

    public Resources getPluginResources() {
        return this.getResources();
    }

    /*
     Code for refactoring for the isHspapDataDistinguishable()
    */
    public NetworkType customizeNetworkType(boolean roaming, int dataNetType, NetworkType networkType){
        switch(dataNetType) {
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return NetworkType.Type_4G;
    }
        return networkType;
    }

    public DataType customizeDataType(boolean roaming, int dataNetType, DataType dataType){
        switch(dataNetType) {
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return DataType.Type_4G;
    }
        return dataType;
    }
    
}
