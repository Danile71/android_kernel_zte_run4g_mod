package com.mediatek.browser.plugin;

import android.content.Context;

import com.mediatek.browser.ext.DefaultBrowserSiteNavigationExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

public class Op01BrowserSiteNavigationExt extends DefaultBrowserSiteNavigationExt {

    private static final String TAG = "Op01BrowserSiteNavigationExt";

    private Context mContext;

    public Op01BrowserSiteNavigationExt(Context context) {
        super();
        mContext = context;
    }

    public CharSequence[] getPredefinedWebsites() {
        Xlog.i(TAG, "Enter: " + "getPredefinedWebsites" + " --OP01 implement");
        return mContext.getResources().getTextArray(R.array.predefined_websites_op01);
    }

}
