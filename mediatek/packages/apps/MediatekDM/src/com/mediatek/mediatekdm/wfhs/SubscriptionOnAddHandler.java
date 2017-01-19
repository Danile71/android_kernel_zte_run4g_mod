package com.mediatek.mediatekdm.wfhs;

import android.util.Log;

import com.mediatek.mediatekdm.mdm.NodeOnAddHandler;

public class SubscriptionOnAddHandler implements NodeOnAddHandler {

    private static final String TAG = "DM/Wifi.SubscriptionOnAddHandler";
    
    private MdmWifi mMdmWifi;
    public SubscriptionOnAddHandler(MdmWifi wifi) {
    	mMdmWifi = wifi;
    }

    public void onAdd(String uri) {
        Log.d(TAG, uri + " has been added into dm tree");
        mMdmWifi.onPerProviderSubscriptionMoAdded(uri);
    }
}
