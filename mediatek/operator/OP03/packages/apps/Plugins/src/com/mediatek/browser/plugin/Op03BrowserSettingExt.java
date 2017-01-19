package com.mediatek.browser.plugin;

import android.os.Build;

import com.mediatek.browser.ext.DefaultBrowserSettingExt;
import com.mediatek.xlog.Xlog;

public class Op03BrowserSettingExt extends DefaultBrowserSettingExt {

    private static final String TAG = "Op03BrowserSettingExt";

    public String getOperatorUA(String defaultUA) {
        Xlog.i(TAG, "Enter: " + "getOperatorUA, default UA: " + defaultUA + " --OP03 implement");
        String op03UA = defaultUA;
        if (!defaultUA.contains("-orange")) {
            op03UA = defaultUA.replace(Build.MODEL, Build.MODEL + "-orange");
        }
        return op03UA;
    }

}
