package com.mediatek.systemui.plugin;

import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.op01.plugin.R;

import com.mediatek.pluginmanager.PluginManager;

public class Op01StatusBarPluginTest extends InstrumentationTestCase {

    private static Op01StatusBarPlugin sStatusBarPlugin = null;
    private Context mContext;
    static final int[] DATA_ACTIVITY = {
        R.drawable.stat_sys_signal_not_inout,
        R.drawable.stat_sys_signal_in,
        R.drawable.stat_sys_signal_out,
        R.drawable.stat_sys_signal_inout
    };

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mContext = this.getInstrumentation().getContext();
        sStatusBarPlugin = (Op01StatusBarPlugin) PluginManager.createPluginObject(mContext,
                "com.mediatek.systemui.ext.IStatusBarPlugin");
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        sStatusBarPlugin = null;
    }

    public void testisHspaDataDistinguishable() {
        boolean isisHspaDataDistinguishable = sStatusBarPlugin.isHspaDataDistinguishable();
        assertTrue(isisHspaDataDistinguishable);
    }

    public void testcustomizeAutoInSimChooser() {
        boolean isCustomizeAutoInSimChooser = sStatusBarPlugin.customizeAutoInSimChooser(false);
        assertTrue(isCustomizeAutoInSimChooser);
    }

    public void testcustomizeEnableBluetoothtAirplaneMode() {
        boolean isCustomizeEnableBluetoothtAirplaneMode = sStatusBarPlugin.customizeEnableBluetoothtAirplaneMode(false);
        assertTrue(isCustomizeEnableBluetoothtAirplaneMode);
    }

}
