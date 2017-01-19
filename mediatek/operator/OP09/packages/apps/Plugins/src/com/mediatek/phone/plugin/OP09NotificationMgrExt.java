package com.mediatek.phone.plugin;

import android.app.PendingIntent;

import com.mediatek.phone.ext.DefaultNotificationMgrExt;
import com.mediatek.xlog.Xlog;

/**
 * Operator plugin for Phone NotificationMgr.
 */
public class OP09NotificationMgrExt extends DefaultNotificationMgrExt {

    private static final String TAG = "OP09NotificationMgrExt";

    @Override
    public PendingIntent onShowNetworkSelection(PendingIntent def) {
        /**
         * In OP09, when NotificationMgr call showNetworkSelection, it should
         * post no PendingIntent.
         */
        Xlog.d(TAG, "onShowNetWorkSelection return null");
        return null;
    }

}
