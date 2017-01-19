package com.mediatek.ims;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.common.gba.IGbaService;
import com.mediatek.common.gba.NafSessionKey;

public final class GbaManager{
    private static final String TAG = "GbaManager";

    private final Context mContext;
    private static IGbaService mService;
    private static GbaManager mGbaManager = null;

    public static final int IMS_GBA_NONE     = 0;
    public static final int IMS_GBA_ME       = 1;
    public static final int IMS_GBA_U        = 2;
    
    public static final String IMS_GBA_KS_NAF       = "Ks_NAF";
    public static final String IMS_GBA_KS_EXT_NAF   = "Ks_ext_NAF";

    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID0 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00};
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID1 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x01}; //MBMS
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID2 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x02};
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID3 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x03}; //MBMS

    /**
     * Helpers to get the default GbaManager.
     */
    public static GbaManager getDefaultGbaManager(Context context) {
        if (context == null) {
            throw new IllegalArgumentException("context cannot be null");
        }

        try {
            IBinder b = ServiceManager.getService("GbaService");
            if(b == null) {
                Log.i("debug", "The binder is null");
                return null;
            }
            mService = IGbaService.Stub.asInterface(b);
            mGbaManager = new GbaManager(context);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return mGbaManager;
    }

    GbaManager(Context context) {
        mContext = context;        
    }
    
    public int getGbaSupported(){
        try {
            return mService.getGbaSupported();
        } catch (RemoteException e) {
            return 0;
        }
    }
    
    public boolean isGbaKeyExpired(String nafFqdn, byte[] nafSecurProtocolId){
        try {
            return mService.isGbaKeyExpired(nafFqdn, nafSecurProtocolId);
        } catch (RemoteException e) {
            return true;
        }
    }

    public NafSessionKey runGbaAuthentication(String nafFqdn, byte[] nafSecurProtocolId, boolean forceRun){
        try {
            return mService.runGbaAuthentication(nafFqdn, nafSecurProtocolId, forceRun);
        } catch (RemoteException e) {
            return null;
        }
    }
}