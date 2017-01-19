package com.mediatek.browser.plugin;

import android.content.Context;

import com.mediatek.browser.ext.DefaultBrowserSettingExt;
import com.mediatek.op09.plugin.R;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.xlog.Xlog;

public class Op09BrowserSettingExt extends DefaultBrowserSettingExt {

	private static final String TAG = "Op09BrowserSettingExt";

    private static final String DEFAULT_DOWNLOAD_DIRECTORY_OP09 = "/storage/sdcard0/Download";
    private static final String DEFAULT_DOWNLOAD_FOLDER_OP09 = "/Download";

    private Context mContext;

    public Op09BrowserSettingExt(Context context) {
        super();
        mContext = context;
    }

    public String getCustomerHomepage() {
        Xlog.i(TAG, "Enter: " + "getCustomerHomepage" + " --OP09 implement");
        return mContext.getResources().getString(R.string.homepage_base_site_navigation);
    }

    public String getDefaultDownloadFolder() {
        Xlog.i(TAG, "Enter: " + "getDefaultDownloadFolder()" + " --OP09 implement");
        String defaultDownloadPath = DEFAULT_DOWNLOAD_DIRECTORY_OP09;
        String defaultStorage = StorageManagerEx.getDefaultPath();
        if (null != defaultStorage) {
            defaultDownloadPath = defaultStorage + DEFAULT_DOWNLOAD_FOLDER_OP09; 
        }
        Xlog.v(TAG, "device default storage is: " + defaultStorage + 
                " defaultPath is: " + defaultDownloadPath);
        return defaultDownloadPath;
    }

    public boolean getDefaultLoadPageMode() {
        Xlog.i(TAG, "Enter: " + "getDefaultLoadPageMode" + " --OP09 implement");
        return false;
    }

}
