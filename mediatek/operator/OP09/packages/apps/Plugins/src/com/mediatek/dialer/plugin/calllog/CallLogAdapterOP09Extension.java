package com.mediatek.dialer.plugin.calllog;

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.util.Log;
import android.view.View;

import com.mediatek.dialer.calllogex.ContactInfoEx;
import com.mediatek.dialer.ext.CallLogAdapterExtension;
import com.mediatek.dialer.plugin.OP09DialerPlugin;
import com.mediatek.op09.plugin.R;

public class CallLogAdapterOP09Extension extends CallLogAdapterExtension {
 
    private static final String TAG = "CallLogAdapterOP09Extension";

    private Context mPluginContext;
    private OP09DialerPlugin mOP09DialerPlugin;

    public CallLogAdapterOP09Extension(Context pluginContext, OP09DialerPlugin oP09DialerPlugin) {
        mPluginContext = pluginContext;
        mOP09DialerPlugin = oP09DialerPlugin;
    }

    public boolean setListItemViewTag(View itemView, ContactInfoEx contactInfo,
                                      Cursor c, Intent callDetailIntent) {
        itemView.setTag(new CallLogInfo(contactInfo, callDetailIntent));
        return true;
    }

    public void bindViewPre(View view, Cursor c, int count, ContactInfoEx contactInfo) {
        if (0 == mOP09DialerPlugin.getTimezoneRawOffset()) {
            return;
        }
        contactInfo.date += mOP09DialerPlugin.getTimezoneOffset(contactInfo.date);
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
