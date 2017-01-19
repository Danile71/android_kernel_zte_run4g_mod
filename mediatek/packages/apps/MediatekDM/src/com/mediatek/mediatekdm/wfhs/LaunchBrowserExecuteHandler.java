package com.mediatek.mediatekdm.wfhs;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import com.mediatek.mediatekdm.DmController;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.NodeExecuteHandler;

public class LaunchBrowserExecuteHandler implements NodeExecuteHandler {

    private static final String TAG = "DM/Wifi.LaunchBrowser";
    
    private MdmWifi mMdmWifi;

    public LaunchBrowserExecuteHandler(Context context, MdmWifi wifi) {
        mContext = context;
        mMdmWifi = wifi;
    }

    @Override
    public int execute(byte[] data, String correlator) throws MdmException {
        String uri = new String(data);
        Log.d(TAG, "launch browser to : " + uri);
        launchBrowserToURI(uri);
        mMdmWifi.waitForExecute();
        
        Log.d(TAG, "execute done");
        return 0;
    }

    private void launchBrowserToURI(String uri) {
        String validUri = validUri(uri);
        Log.d(TAG, "valid uri : " + validUri);
        Intent intent = new Intent(MdmWifi.SessionAction.LAUNCH_BROWSER, Uri.parse(validUri));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }
    
    private String validUri(String uri) {
        uri = uri.replaceAll("&amp;", "&");
        if (uri.contains("RedirectURI=null")) {
            uri = uri.replace("RedirectURI=null", "RedirectURI=" + MdmWifi.sRedirectUrl);
        }
	return uri;
    }
    private Context mContext;
}
