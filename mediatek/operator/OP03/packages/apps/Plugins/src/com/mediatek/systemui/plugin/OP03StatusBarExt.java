
package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;

import com.mediatek.xlog.Xlog;
import com.mediatek.op03.plugin.R;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;
import android.telephony.TelephonyManager;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.NetworkType;

/**
 * M: OP03 implementation of Plug-in definition of Status bar.
 */
public class OP03StatusBarExt extends DefaultStatusBarPlugin {

    public OP03StatusBarExt(Context context) {
        super(context);
    }

    /*public boolean get3GPlusResources(boolean roaming, int dataType) {
       return true;
    }*/

    
    public NetworkType customizeNetworkType(boolean roaming, int dataNetType, NetworkType networkType){
        switch(dataNetType) {
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
                return NetworkType.Type_3G;
        }
        return networkType;
    }
    
    public DataType customizeDataType(boolean roaming, int dataNetType, DataType dataType){
        switch(dataNetType) {
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
                return DataType.Type_3G_PLUS;
        }
        return dataType;
    }
    

}
