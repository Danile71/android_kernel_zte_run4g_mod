
package com.mediatek.connectivity;

import android.content.Context;
import android.util.Base64;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.common.gba.NafSessionKey;
import com.mediatek.common.gba.IGbaService;

import java.security.Principal;
import org.apache.http.auth.BasicUserPrincipal;
import org.apache.http.auth.Credentials;

public class GbaCredentials implements Credentials {
    private final static String TAG = "GbaCredentials";    
    private BasicUserPrincipal mUserPrincipal;
    IGbaService mService;
    private String mPasswd;
    private final Context mContext;
    private String mNafAddress;
    private static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID0 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x01, (byte) 0x00, (byte) 0x2F};
    final protected static char[] hexArray = "0123456789abcdef".toCharArray();

    public GbaCredentials(Context context, String nafAddress){
        super();
        mContext = context;

        if(nafAddress.charAt(nafAddress.length() -1) == '/'){
           nafAddress = nafAddress.substring(0, nafAddress.length() - 1);
        }

        mNafAddress = nafAddress.toLowerCase();
        if(mNafAddress.indexOf("http://") != -1){
           mNafAddress = nafAddress.substring(7);
        }else if(mNafAddress.indexOf("https://") != -1){
           mNafAddress = nafAddress.substring(8);
        }

        Log.d(TAG, "nafAddress:" + mNafAddress);
        mUserPrincipal = null;
    }

    @Override
    public Principal getUserPrincipal() {
        Log.i(TAG, "getUserPrincipal");
        if(mUserPrincipal == null){
            Log.i(TAG, "Run GBA procedure");

            try {
                IBinder b = ServiceManager.getService("GbaService");
                if(b == null) {
                    Log.i("debug", "The binder is null");
                return null;
            }
                mService = IGbaService.Stub.asInterface(b);
            } catch (Exception e) {
                e.printStackTrace();
            }

            try{
                NafSessionKey nafSessionKey = mService.runGbaAuthentication(mNafAddress, DEFAULT_UA_SECURITY_PROTOCOL_ID0, false);
            if(nafSessionKey != null){
                    Log.i(TAG, "GBA Session Key:" + nafSessionKey);
                    mUserPrincipal = new BasicUserPrincipal(nafSessionKey.getBtid());
                    mPasswd = Base64.encodeToString(nafSessionKey.getKey(),Base64.NO_WRAP);
                }
            }catch(RemoteException re){

            }catch(Exception e){
                e.printStackTrace();
            }
        }
        return mUserPrincipal;
    }

    @Override
    public String getPassword() {
        Log.i(TAG, "mPasswd:" + mPasswd);
        return mPasswd;
    }
        
    private String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);        
    }
}