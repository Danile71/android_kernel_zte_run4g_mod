package com.mediatek.mediatekdm.wfhs;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.mediatek.mediatekdm.DmController;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.NodeExecuteHandler;

public class NegotiateClientCertExecuteHandler implements NodeExecuteHandler {

    private static final String TAG = "DM/Wifi.NegotiateClientCert";
    
    private MdmWifi mMdmWifi;

    public NegotiateClientCertExecuteHandler(Context context, MdmWifi wifi) {
        mContext = context;
        mMdmWifi = wifi;
    }

    @Override
    public int execute(byte[] data, String correlator) throws MdmException {
        String str = new String(data);
        Log.i(TAG, "execute [data] : " + str);
        Log.i(TAG, "execute [correlator] : " + correlator);
        // reset session id will send to client
        resetOSUSessionId();
        negotiateClientCertTLS(str);
        mMdmWifi.waitForExecute();
        Log.d(TAG, "execute done");
        return 0;
    }

    private void negotiateClientCertTLS(String str) {
        Intent intent = new Intent(MdmWifi.SessionAction.NEGOTIATE);
        mContext.sendBroadcast(intent);
    }
    private void resetOSUSessionId() {
        if(MdmWifi.sSessionAction.equals(MdmWifi.SessionAction.PROVISION_START)) {
        	mMdmWifi.setSessionId(
                    MdmWifi.SessionID.PROVISION_DEVICE_CERTIFICATE);
            Log.i(TAG, "resetOSUSessionId is : " + 
            		mMdmWifi.getSessionId());
        }
    }
    private Context mContext;
}
