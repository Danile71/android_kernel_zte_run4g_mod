package com.mediatek.mms.op09;

import android.content.ContentValues;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.provider.Telephony.Sms;

import com.mediatek.mms.ext.IMmsSmsMessageSender;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class MmsSmsMessageSenderTest extends BasicCase {
    private static final long DATE_SENT = 1359092173;
    private static IMmsSmsMessageSender sMessageSender;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        sMessageSender = (IMmsSmsMessageSender)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IMmsSmsMessageSender");
    }

    public void test001AddMessageToUri() {
        long dateSent = System.currentTimeMillis();

        assertNotNull( sMessageSender.addMessageToUri(mContext.getContentResolver(),
                Uri.parse("content://sms/queued"), TEST_ADDRESS, SMS_CONTENT, null,
                dateSent, true, false, 12345, mSimIdCdma, -dateSent));
    }
}
