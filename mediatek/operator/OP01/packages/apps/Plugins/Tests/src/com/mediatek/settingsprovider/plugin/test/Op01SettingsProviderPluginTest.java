package com.mediatek.settingsprovider.plugin.test;

import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;
import android.test.InstrumentationTestCase;

import com.mediatek.op01.plugin.R;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.providers.settings.ext.IDatabaseHelperExt;
import com.mediatek.settingsprovider.plugin.Op01DatabaseHelperExt;

public class Op01SettingsProviderPluginTest extends InstrumentationTestCase {
    
    private static Op01DatabaseHelperExt sDb01Ext = null;
    private Context mContext;
    private ContentResolver mCr;
    @Override
    public void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        mCr = mContext.getContentResolver();
        Object plugin = PluginManager.createPluginObject(mContext, IDatabaseHelperExt.class.getName());
        if (plugin instanceof Op01DatabaseHelperExt) {
            sDb01Ext = (Op01DatabaseHelperExt) plugin;
        }
    }

    // test the function of getResBoolean(Context context, String name, String defaultValue)
    public void test01_getBooleanValue() {
        // test the auto_time
        String autoTime = Settings.System.AUTO_TIME;
        int autoTimeId = R.bool.def_auto_time_op01;
        String autoTimevalue = sDb01Ext.getResBoolean(mContext, autoTime, "1");
        assertEquals("0",autoTimevalue);
        // test the auto_time_zone
        String autoTimeZone = Settings.System.AUTO_TIME_ZONE;
        int autoTimeZoneId = R.bool.def_auto_time_zone_op01;
        String autoTimeZonevalue = sDb01Ext.getResBoolean(mContext, autoTimeZone, "1");
        assertEquals("0",autoTimeZonevalue);
        // test the haptic_feedback_enabled
        String feedback = Settings.System.HAPTIC_FEEDBACK_ENABLED;
        int feedbackId = R.bool.def_haptic_feedback_op01;
        String feedbakcValue = sDb01Ext.getResBoolean(mContext, feedback, "1");
        assertEquals("0",feedbakcValue);
        // test the haptic_feedback_enabled
        String batteryPer = Settings.Secure.BATTERY_PERCENTAGE;
        int batteryId = R.bool.def_battery_percentage_op01;
        String batteryValue = sDb01Ext.getResBoolean(mContext, batteryPer, "0");
        assertEquals("1",batteryValue);
        // test the others
        String testName = "testBool";
        String value = sDb01Ext.getResBoolean(mContext, testName, "0");
        assertEquals("0",value);
    }
   
    @Override    
    public void tearDown() throws Exception {
        super.tearDown();
        sDb01Ext = null;
    }

}
