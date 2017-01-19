package com.mediatek.systemui.plugin;

import android.content.Context;

import com.mediatek.systemui.ext.BehaviorSet;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;

import com.mediatek.systemui.ext.IconIdWrapper;

/**
 * M: OP01 implementation of Plug-in definition of Status bar.
 */
public class Op01StatusBarPlugin extends DefaultStatusBarPlugin {
    public Op01StatusBarPlugin(Context context) {
        super(context);
    }

    public boolean isHspaDataDistinguishable() {
        return false;
    }

    public void customizeDataActivityIcon(IconIdWrapper icon, int simColor, int dataActivity) {
        icon.setResources(this.getResources());
        icon.setIconId(TelephonyIcons.DATA_ACTIVITY[dataActivity]);
    }

    public boolean customizeAutoInSimChooser(boolean isEnabled) {
        return true;
    }

    public BehaviorSet customizeBehaviorSet() {
        return BehaviorSet.OP01_BS;
    }

    public boolean customizeHspaDistinguishable(boolean distinguishable) {
        return false;
    }
}
