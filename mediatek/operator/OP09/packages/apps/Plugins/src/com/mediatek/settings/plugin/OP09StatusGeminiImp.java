package com.mediatek.settings.plugin;

import com.mediatek.settings.ext.DefaultStatusGeminiExt;
import com.mediatek.xlog.Xlog;

public class OP09StatusGeminiImp extends DefaultStatusGeminiExt {

    private static final String TAG = "StatusGeminiImp";

    public OP09StatusGeminiImp() {
    }

    /**
     * Cusotmize network type name, it will be called when update Network Type
     */
    @Override
    public String customizeNetworkTypeName(String netWorkTypeName) {
        Xlog.d(TAG, "reNameNetworkTypeNameForCTSpec netWorkTypeName="
                + netWorkTypeName);
        if (null != netWorkTypeName) {
            return CurrentNetworkInfoStatus.renameNetworkTypeNameForCTSpec(netWorkTypeName);
        } else {
            return null;
        }
    }
}
