package com.mediatek.systemui.plugin;

import android.content.Context;
import android.test.InstrumentationTestCase;


import com.mediatek.op02.plugin.R;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.systemui.ext.IconIdWrapper;
import com.mediatek.systemui.ext.NetworkType;

public class Op02StatusBarPluginTest extends InstrumentationTestCase {
    
    private static Op02StatusBarPlugin sStatusBarPlugin = null;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mContext = this.getInstrumentation().getContext();
        sStatusBarPlugin = (Op02StatusBarPlugin) PluginManager.createPluginObject(mContext,
                "com.mediatek.systemui.ext.IStatusBarPlugin");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        sStatusBarPlugin = null;
    }

    public void testCustomizeSignalStrengthNullIconGemini() {
        IconIdWrapper tempIconIdWrapper = new IconIdWrapper();
        sStatusBarPlugin.customizeSignalStrengthNullIconGemini(tempIconIdWrapper, 0);
        assertEquals(R.drawable.stat_sys_gemini_signal_null_sim1, tempIconIdWrapper.getIconId());

        tempIconIdWrapper = new IconIdWrapper();
        sStatusBarPlugin.customizeSignalStrengthNullIconGemini(tempIconIdWrapper, 1);
        assertEquals(R.drawable.stat_sys_gemini_signal_null_sim2, tempIconIdWrapper.getIconId());
    }

    public void testCustomizeSignalIndicatorIconGemini() {
        IconIdWrapper tempIconIdWrapper = new IconIdWrapper();
        sStatusBarPlugin.customizeSignalIndicatorIconGemini(tempIconIdWrapper, 0);
        assertEquals(R.drawable.stat_sys_gemini_signal_indicator_sim1, tempIconIdWrapper.getIconId());

        tempIconIdWrapper = new IconIdWrapper();
        sStatusBarPlugin.customizeSignalIndicatorIconGemini(tempIconIdWrapper, 1);
        assertEquals(R.drawable.stat_sys_gemini_signal_indicator_sim2, tempIconIdWrapper.getIconId());
    }

    public void testCustomizeDataNetworkTypeIconGemini() {
        IconIdWrapper icon = new IconIdWrapper();
        sStatusBarPlugin.customizeDataNetworkTypeIconGemini(icon, NetworkType.Type_1X3G, -1);
        assertEquals(-1, icon.getIconId());

        sStatusBarPlugin.customizeDataNetworkTypeIconGemini(icon, NetworkType.Type_1X3G, 1);
        assertTrue(-1 == icon.getIconId());

        sStatusBarPlugin.customizeDataNetworkTypeIconGemini(icon, NetworkType.Type_3G, 1);
        assertTrue(-1 != icon.getIconId());
    }
}
