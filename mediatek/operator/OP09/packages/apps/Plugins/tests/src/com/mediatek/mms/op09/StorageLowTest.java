package com.mediatek.mms.op09;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;

import com.mediatek.mms.ext.IStorageLow;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class StorageLowTest extends BasicCase {
    private IStorageLow mStorageLow;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mStorageLow = (IStorageLow)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IStorageLow");
    }

    public void test001ShowNotification() {
        assertEquals("Message memory low", mStorageLow.getNotificationTitle());
        assertEquals("Message memory is nearly full. Please delete old messages.",
                mStorageLow.getNotificationBody());
    }

}
