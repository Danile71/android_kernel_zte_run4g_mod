package com.mediatek.oobe.plugin;

import com.mediatek.oobe.ext.DefaultOobeMiscExt;
import com.mediatek.xlog.Xlog;

public class OobeMiscExt extends DefaultOobeMiscExt {
    private static final String TAG = "OobeMiscExt";

    public String replaceSimToSimUim(String simString) {
        if (simString.contains("SIM")) {
            simString = simString.replaceAll("SIM", "UIM/SIM");
        }
        Xlog.d(TAG, "oobe replace string: " + simString);
        return simString;
    }

}
