package com.mediatek.mediatekdm.wfhs;

import java.nio.channels.InterruptibleChannel;

import android.R.integer;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.mediatek.mediatekdm.DmController;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.NodeExecuteHandler;

public class CertificateEnrollExecuteHandler implements NodeExecuteHandler {

    private static final String TAG = "DM/Wifi.CertificateEnroll";
    
    private MdmWifi mMdmWifi;

    public CertificateEnrollExecuteHandler(Context context, MdmWifi wifi) {
        mContext = context;
        mMdmWifi = wifi;
    }

    @Override
    public int execute(byte[] data, String correlator) throws MdmException {
        String str = new String(data);
        Log.i(TAG, "execute [data] : " + str);
        Log.i(TAG, "execute [correlator] : " + correlator);
        resetOSUSessionId();
        getCertificate(str);
        mMdmWifi.waitForExecute();
        Log.d(TAG, "execute done");
        Object response = mMdmWifi.getExecuteResponse();
        if(response != null) {
            Integer result = (Integer) response;
            Log.i(TAG, "execute [response] : " + result.intValue());
            if(result.intValue() != 0) {
                return -1;
            }
        }
        return 0;
    }

    private void getCertificate(String str) {
        Intent intent = new Intent();
        int sessionId = mMdmWifi.getSessionId();
        Log.i(TAG, "execute [sessionId] : " + sessionId);
        DmXMLStringParser dmXMLStringParser = new DmXMLStringParser(str);
        String enrollURL = dmXMLStringParser.getValByTagName("spp:enrollmentServerURI");
        Log.i(TAG, "execute [enrollURL] : " + enrollURL);

        switch (sessionId) {
            case MdmWifi.SessionID.PROVISION_CERTIFICATE:
                String enrollUsername = dmXMLStringParser.getValByTagName("spp:estUserID");
                Log.i(TAG, "execute [enrollUsername] : " + enrollUsername);
                String enrollPassword = dmXMLStringParser.getValByTagName("spp:estPassword");
                Log.i(TAG, "execute [enrollPassword] : " + enrollPassword);
                intent.setAction(MdmWifi.SessionAction.ENROLLMENT);
                intent.putExtra("enrollurl", enrollURL);
                intent.putExtra("enrollusername", enrollUsername);
                intent.putExtra("enrollpassword", enrollPassword);
                break;
            case MdmWifi.SessionID.REMEDIATION_UPDATE_CERTIFICATE:
            case MdmWifi.SessionID.REMEDIATION_USER_CERTIFICATE:   
            case MdmWifi.SessionID.REMEDIATION_UPDATE_CERTIFICATE_UNPW:
                MdmWifi.sRemSimProvision = false;
                intent.setAction(MdmWifi.SessionAction.REENROLLMENT);
                intent.putExtra("reenrollurl", enrollURL);
                break;
            default:
                Log.e(TAG, "error session id to do certificate enrollment");
                break;
        }
        mContext.sendBroadcast(intent);
    }
    private void resetOSUSessionId() {
        if(MdmWifi.sSessionAction.equals(MdmWifi.SessionAction.PROVISION_START)) {
        	mMdmWifi.setSessionId(
                    MdmWifi.SessionID.PROVISION_CERTIFICATE);
            Log.i(TAG, "resetOSUSessionId is : " + 
            		mMdmWifi.getSessionId());
        }
    }
    private Context mContext;
}
