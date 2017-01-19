package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.systemui.plugin.Op07StatusBarPlugin;

import android.content.ContextWrapper;
import android.content.res.Resources;

import com.mediatek.op07.plugin.R;

import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.IStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

public class Op07StatusBarPluginTest extends InstrumentationTestCase {
    
    private static Op07StatusBarPlugin mStatusBarPlugin = null;
    private Context context;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        context = this.getInstrumentation().getContext();
        mStatusBarPlugin = (Op07StatusBarPlugin)PluginManager.createPluginObject(context, "com.mediatek.systemui.ext.IStatusBarPlugin");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mStatusBarPlugin = null;
    }

    public void testgetSignalStrengthIcon() {
        int nSignalStrengthIcon = mStatusBarPlugin.getSignalStrengthIcon(true, 0, 0);
        assertEquals(-1, nSignalStrengthIcon);
    }

    public void testgetSignalStrengthIconGemini() {
        int nSignalStrengthIconGemini = mStatusBarPlugin.getSignalStrengthIconGemini(0, 0);
        assertEquals(-1, nSignalStrengthIconGemini);
    }

    public void testgetSignalStrengthIconSingle() {
        int nSignalStrengthIconGemini = mStatusBarPlugin.getSignalStrengthIconGemini(0, 0, 0);
        assertEquals(-1, nSignalStrengthIconGemini);
    }

    public void testgetSignalStrengthNullIconGemini() {
        int nSignalStrengthNullIconGemini = mStatusBarPlugin.getSignalStrengthNullIconGemini(0);
        assertEquals(-1, nSignalStrengthNullIconGemini);
    }

    public void testgetSignalStrengthSearchingIconGemini() {
        int nSignalStrengthSearchingIconGemini = mStatusBarPlugin.getSignalStrengthSearchingIconGemini(0);
        assertEquals(-1, nSignalStrengthSearchingIconGemini);
    }

    public void testgetSignalIndicatorIconGemini() {
        int nSignalIndicatorIconGemini = mStatusBarPlugin.getSignalIndicatorIconGemini(0);
        assertEquals(-1, nSignalIndicatorIconGemini);
    }

    public void testgetDataTypeIconListGemini() {
        int[] ngetDataTypeIconListGemini = mStatusBarPlugin.getDataTypeIconListGemini(true, DataType.Type_G);
        assertEquals(null, ngetDataTypeIconListGemini);
    }

    public void testisHspaDataDistinguishable() {
        boolean isisHspaDataDistinguishable = mStatusBarPlugin.isHspaDataDistinguishable();
        assertTrue(!isisHspaDataDistinguishable);
    }

    public void testgetDataNetworkTypeIconGemini() {
        int nDataNetworkTypeIconGemini = mStatusBarPlugin.getDataNetworkTypeIconGemini(null, 0);
        assertEquals(-1, nDataNetworkTypeIconGemini);
    }

    public void testgetDataActivityIconList() {
        int[] nDataActivityIconList = mStatusBarPlugin.getDataActivityIconList();
        assertEquals(null, nDataActivityIconList);
    }

    public void testsupportDataTypeAlwaysDisplayWhileOn() {
        boolean issupportDataTypeAlwaysDisplayWhileOn = mStatusBarPlugin.supportDataTypeAlwaysDisplayWhileOn();
        assertTrue(!issupportDataTypeAlwaysDisplayWhileOn);
    }

    public void testsupportDisableWifiAtAirplaneMode() {
        boolean issupportDisableWifiAtAirplaneMode = mStatusBarPlugin.supportDisableWifiAtAirplaneMode();
        assertTrue(!issupportDisableWifiAtAirplaneMode);
    }

    public void testget3gDisabledWarningString() {
        String s3gDisabledWarningString = mStatusBarPlugin.get3gDisabledWarningString();
        assertEquals(null, s3gDisabledWarningString);
    }
}
