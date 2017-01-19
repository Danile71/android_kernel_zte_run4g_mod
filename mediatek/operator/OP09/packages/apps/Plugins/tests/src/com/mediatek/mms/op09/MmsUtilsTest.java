package com.mediatek.mms.op09;

import android.text.format.DateUtils;
import android.widget.TextView;

import com.mediatek.mms.ext.IMmsUtils;
import com.mediatek.mms.op09.Op09MmsPluginTestRunner.BasicCase;
import com.mediatek.pluginmanager.PluginManager;

public class MmsUtilsTest extends BasicCase {
    private static final long DATE = 1359436497;
    private static final String TEST_STRING = "String for test.";
    private static IMmsUtils sMmsUtils;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        sMmsUtils = (IMmsUtils)PluginManager.createPluginObject(mContext,
                "com.mediatek.mms.ext.IMmsUtils");
    }

    // This case need to set time format of the phone to be 24 hour type. 
    public void test001FormatDateAndTimeStampString() {
        sMmsUtils.formatDateAndTimeStampString(mContext, DATE , System.currentTimeMillis(), false, TEST_STRING);
        sMmsUtils.formatDateAndTimeStampString(mContext, DATE ,
                System.currentTimeMillis() - 90000000, false, TEST_STRING);
        sMmsUtils.formatDateAndTimeStampString(mContext, DATE ,
            System.currentTimeMillis() - 180000000, false, TEST_STRING);
        sMmsUtils.formatDateAndTimeStampString(mContext, DATE ,
                System.currentTimeMillis() - 60000, false, TEST_STRING);
        sMmsUtils.formatDateAndTimeStampString(mContext, DATE , DATE, false, TEST_STRING);
        sMmsUtils.formatDateAndTimeStampString(mContext, DATE, 0, true, TEST_STRING);
        sMmsUtils.formatDateAndTimeStampString(mContext, 0, 0, false, TEST_STRING);
    }

    public void test002GetSendButtonResourceIdBySlotId() {
        if (checkSims()) {
            assertNotNull(sMmsUtils.getSendButtonResourceIdBySlotId(mContext, 0));
            assertNotNull(sMmsUtils.getSendButtonResourceIdBySlotId(mContext, 1));
            assertNotNull(sMmsUtils.getSendButtonResourceIdBySlotId(mContext, 0, true));
            assertNotNull(sMmsUtils.getSendButtonResourceIdBySlotId(mContext, 1, false));
        }
    }

    public void test003ShowSimTypeBySimId() {
        if (checkSims()) {
            TextView textView = new TextView(mContext);
            textView.setText("China Telecom");
            sMmsUtils.showSimTypeBySimId(mContext, mSimIdCdma, textView);
            sMmsUtils.showSimTypeBySimId(mContext, mSimIdGsm, textView);
        }
    }

    // This case need to set time format of the phone to be 24 hour type and time zone set to GMT+8.
    public void test004FormatDateTime() {
        assertEquals("15:39", sMmsUtils.formatDateTime(mContext, DATE * 1000, DateUtils.FORMAT_SHOW_TIME));
    }

    public void test005GetActivatedButtonIconBySlotId() {
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(0, true, 0));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(0, true, -1));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(0, false, 0));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(0, false, -1));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(1, true, 0));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(1, true, -1));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(1, false, 0));
        assertNotNull(sMmsUtils.getActivatedButtonIconBySlotId(1, false, -1));
    }

    public void test006IsWellFormedSmsAddress() {
        assertFalse(sMmsUtils.isWellFormedSmsAddress(""));
        assertFalse(sMmsUtils.isWellFormedSmsAddress("(86)15313706372"));
        assertFalse(sMmsUtils.isWellFormedSmsAddress("+(86)15313706372"));
        assertTrue(sMmsUtils.isWellFormedSmsAddress("+8615313706372"));
    }

}
