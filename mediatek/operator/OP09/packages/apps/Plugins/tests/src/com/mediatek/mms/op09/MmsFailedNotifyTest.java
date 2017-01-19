package com.mediatek.mms.op09;

import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.mms.ext.IMmsFailedNotify;
import com.mediatek.pluginmanager.PluginManager;

public class MmsFailedNotifyTest extends BasicCase {
    private static IMmsFailedNotify sMmsFailedNotify;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        sMmsFailedNotify = (IMmsFailedNotify)PluginManager.createPluginObject(
                mInstrumentation.getTargetContext(),
                "com.mediatek.mms.ext.IMmsFailedNotify");
    }

    public void test001PopUpToast() throws InterruptedException {
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.REQUEST_RESPONSE_TEXT, null);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.REQUEST_RESPONSE_TEXT, "Some reason from server.");
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.DATA_OCCUPIED, null);
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.CONNECTION_FAILED, null);
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.GATEWAY_NO_RESPONSE, null);
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.HTTP_ABNORMAL, null);
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.CANCEL_DOWNLOAD, null);
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.DISABLE_DELIVERY_REPORT, null);
        delay(DELAY_TIME);
        sMmsFailedNotify.popupToast(mContext, IMmsFailedNotify.SEND_TIMEOUT, null);
    }

}
