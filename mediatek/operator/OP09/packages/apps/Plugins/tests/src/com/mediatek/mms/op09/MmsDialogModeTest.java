package com.mediatek.mms.op09;

import android.widget.TextView;

import com.mediatek.mms.ext.IMmsDialogMode;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class MmsDialogModeTest extends BasicCase {
    private static IMmsDialogMode sMmsDialogMode;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        sMmsDialogMode = (IMmsDialogMode)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IMmsDialogMode");
    }

    public void test001SetSimTypeDrawable() {
        if (checkSims()) {
            TextView textView = new TextView(mContext);
            textView.setText("China Telecom");
            sMmsDialogMode.setSimTypeDrawable(mContext, mSimIdCdma, textView);
            sMmsDialogMode.setSimTypeDrawable(mContext, mSimIdGsm, textView);
        }

        assertNotNull(sMmsDialogMode.getNotificationContentString(
                TEST_ADDRESS, "Test Suject", "100kb", "2013-12-31"));
    }
}
