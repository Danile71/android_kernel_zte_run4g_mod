package com.mediatek.systemui.plugin;

import android.content.Context;
import android.widget.TextView;

import com.mediatek.op09.plugin.R;
import com.mediatek.systemui.ext.BehaviorSet;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.IconIdWrapper;
import com.mediatek.systemui.ext.NetworkType;

/**
 * M: OP09 implementation of Plug-in definition of Status bar.
 */
public class Op09StatusBarPlugin extends DefaultStatusBarPlugin {

    public Op09StatusBarPlugin(Context context) {
        super(context);
    }

    public void customizeSignalStrengthNullIconGemini(IconIdWrapper icon, int slotId) {
        icon.setResources(this.getResources());
        icon.setIconId(R.drawable.stat_sys_gemini_signal_null);
    }

    public void customizeDataTypeIconListGemini(IconIdWrapper[] iconList, boolean roaming, DataType dataType) {
        int[] resIconList = null;
        resIconList = TelephonyIcons.DATA[dataType.getTypeId()];
        for (int i = 0; i < iconList.length; i++) {
            iconList[i].setResources(this.getResources());
            iconList[i].setIconId(resIconList[i]);
        }
    }

    public void customizeDataNetworkTypeIconGemini(IconIdWrapper icon, NetworkType networkType, int simColorId) {
        icon.setResources(this.getResources());
        int typeId = networkType.getTypeId();
        if (typeId >= 0 && typeId <= 3) {
            typeId = TelephonyIcons.NETWORK_TYPE[typeId][simColorId];
            icon.setIconId(typeId);
        }
    }

    public void customizeDataActivityIcon(IconIdWrapper icon, int simColor, int dataActivity) {
        icon.setResources(this.getResources());
        if (simColor >= 0 && simColor < TelephonyIcons.DATA_ACTIVITY.length) {
            icon.setIconId(TelephonyIcons.DATA_ACTIVITY[simColor][dataActivity]);
        }
    }

    public void customizeWifiSignalStrengthIconList(IconIdWrapper wifiIconId, int level) {
        wifiIconId.setResources(this.getResources());
        wifiIconId.setIconId(TelephonyIcons.WIFI_STRENGTH_ICON[level]);
    }

    public void customizeWifiInOutIcon(IconIdWrapper icon, int wifiActivity) {
        icon.setResources(this.getResources());
        if (wifiActivity >= 0 && wifiActivity <= 3) {
            icon.setIconId(TelephonyIcons.WIFI_INOUT_ICON[wifiActivity]);
        }
    }

    public BehaviorSet customizeBehaviorSet() {
        return BehaviorSet.OP09_BS;
    }

    public boolean customizeMobileGroupVisible(boolean isSimInserted) {
        return true;
    }

    public void customizeTextAllCaps(TextView tv) {
        tv.setAllCaps(false);
    }

    public boolean needCustomizeMobileStateTracker() {
        return true;
    }

    public boolean hideAlwaysAskView() {
        return true;
    }

    public void customizeMobileResourceIconId(IconIdWrapper icon, int type) {
        icon.setResources(this.getResources());
        int resId = -1;
        switch(type) {
        case 0:
            resId = R.drawable.ic_qs_mobile_off;
            break;
        case 1:
            resId = R.drawable.ic_qs_mobile_enable;
            break;
        case 2:
            resId = R.drawable.ic_qs_mobile_disable;
            break;
        default:
            resId = -1;
        }
        icon.setIconId(resId);
    }
}
