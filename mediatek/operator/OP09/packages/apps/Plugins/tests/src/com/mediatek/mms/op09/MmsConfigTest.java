package com.mediatek.mms.op09;

import com.mediatek.mms.ext.IMmsConfig;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class MmsConfigTest extends BasicCase {
    private static IMmsConfig sMmsConfig;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        sMmsConfig = (IMmsConfig)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IMmsConfig");
    }

    public void test001APIs() {
        assertEquals(11, sMmsConfig.getSmsToMmsTextThreshold());

        int[] retryScheme = sMmsConfig.getMmsRetryScheme();
        assertEquals(5 * 60 * 1000, retryScheme[retryScheme.length - 1]);

        assertTrue(sMmsConfig.isSupportVCardPreview());

        assertFalse(sMmsConfig.isSupportCBMessage(mContext, 0));
        assertTrue(sMmsConfig.isSupportCBMessage(mContext, 1));

        assertTrue(sMmsConfig.isSupportCTFeature());

        assertTrue(sMmsConfig.isAllowRetryForPermanentFail());

        /// M: If the test sim card is under international roaming status,
        ///    the API will return false. And then you should change the assert statement.
        assertTrue(sMmsConfig.isAllowDRWhenRoaming(mContext, 0));
    }
}
