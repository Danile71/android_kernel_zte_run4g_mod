package com.mediatek.browser.plugin;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;

import com.mediatek.browser.ext.DefaultBrowserUrlExt;
import com.mediatek.xlog.Xlog;

public class Op09BrowserUrlExt extends DefaultBrowserUrlExt {

    private static final String TAG = "Op09BrowserUrlExt";

    private Context mContext;

    public Op09BrowserUrlExt(Context context) {
        super();
        mContext = context;
    }

    public boolean redirectCustomerUrl(String url) {
        Xlog.i(TAG, "Enter: " + "redirectCustomerUrl" + " --OP09 implement");
        if (url.startsWith("estore:")) {
            try {
                PackageManager pm = mContext.getPackageManager();
                pm.getPackageInfo("com.eshore.ezone", PackageManager.GET_ACTIVITIES);
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intent);
            } catch (NameNotFoundException e) {
                Intent newUrl = new Intent(Intent.ACTION_VIEW,
                        Uri.parse("http://3g.189store.com/general"));
                newUrl.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(newUrl);
            }  
            return true;
        }
        return false;
    }
}
