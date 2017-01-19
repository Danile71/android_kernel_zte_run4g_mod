package com.mediatek.mms.plugin;

import android.content.Context;
import android.util.Log;

import com.mediatek.mms.ext.DefaultMmsRetrieveTransactionExt;

public class Op01MmsRetrieveTransactionExt extends DefaultMmsRetrieveTransactionExt {

    private static final String TAG = "Mms/OP01MmsRetrieveTransactionExt";

    public Op01MmsRetrieveTransactionExt(Context context) {
        super(context);
    }

    public void sendExpiredResIfNotificationExpired() {
        Log.d(TAG, "do nothing");
    }

}
