package com.mediatek.mediatekdm.wfhs.io;

import android.content.Context;
import android.util.Log;

import com.mediatek.mediatekdm.DmController;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.NodeIoHandler;
import com.mediatek.mediatekdm.wfhs.MdmWifi;
import com.mediatek.mediatekdm.wfhs.WiFiHotSpotComponent;

import java.util.Arrays;
import java.util.List;

public class WifiNodeIOHandler implements NodeIoHandler {

    private static final String TAG = "DM/Wifi.WifiNodeIOHandler";
    
    private MdmWifi mMdmWifi;

    public WifiNodeIOHandler(Context context, String uri, String key,
            Integer[] ids, MdmWifi wifi) {
        this(context, uri, key, Arrays.asList(ids));
        mMdmWifi = wifi;
    }
    
    public WifiNodeIOHandler(Context context, String uri, String key,
            List<Integer> ids) {
        mContext = context;
        mUri = uri;
        mKey = key;
        mPreferSessionIds = ids;
    }

    @Override
    public int read(int offset, byte[] data) throws MdmException {
        Log.i(TAG, this.getClass().getName() + ".read");
        return 0;
    }

    @Override
    public void write(int offset, byte[] data, int totalSize)
            throws MdmException {
        Log.i(TAG, this.getClass().getName() + ".write");
        int sessionId = mMdmWifi.getSessionId();
        Log.i(TAG, "session id : " + sessionId);
        for (int id : mPreferSessionIds) {
            Log.i(TAG, "prefer id : " + id);
        }
        if (mPreferSessionIds.contains(sessionId)) {
            String str = new String(data);
            Log.i(TAG, "write : " + str);
            mMdmWifi.addResult(mKey, str);
        }
    }

    protected Context mContext;
    protected String mUri;
    protected String mKey;
    protected List<Integer> mPreferSessionIds;
}
