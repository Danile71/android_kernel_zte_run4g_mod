package com.mediatek.op.server;

import android.content.Context;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.provider.Telephony.SimInfo;

import com.mediatek.xlog.Xlog;
import com.mediatek.common.server.IConnectivityRoamingExt;

public class DefConnectivityRoamingExt implements IConnectivityRoamingExt {

    private static final String TAG = "DefConnectivityRoamingExt";

    @Override
    public void setFirstRoamingFlag(Context context, boolean roaming) {
    }

    @Override
    public boolean isFirstEnterRoamingArea(Context context) {
        Xlog.i(TAG, "isFirstEnterRoamingArea");
        return false;
    }

    @Override
    public void popupAlertDialog(Context context) {
        Xlog.i(TAG, "popupAlertDialog");
    }

    @Override
    public void setMobileDataDisabled(Context context, Handler handler, int msg) {
        Xlog.i(TAG, "setMobileDataDisabled");
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        cm.setMobileDataEnabledGemini(SimInfo.SLOT_NONE);
    }
}
