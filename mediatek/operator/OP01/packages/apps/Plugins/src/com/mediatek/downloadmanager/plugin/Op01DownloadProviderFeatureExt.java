package com.mediatek.downloadmanager.plugin;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.provider.Downloads;

import com.mediatek.downloadmanager.ext.DefaultDownloadProviderFeatureExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

import java.util.HashSet;

public class Op01DownloadProviderFeatureExt extends DefaultDownloadProviderFeatureExt {
    
    private static final String TAG = "DownloadProviderPluginEx";
    
    public static final String BROWSER_NOTIFICATION_PACKAGE = 
        "com.android.browser";        //Constants
    
    public static final String OMADL_NOTIFICATION_PACKAGE =
        "com.android.providers.downloads.ui";        //Constants
    
    public Op01DownloadProviderFeatureExt(Context context) {
        super(context);
    }
    
    /**
     * Used to show full download path in download notificaiton.
     * 
     * @param packageName The app package name which call download.
     * @param mimeType Download file mimetype
     * @param fullFileName The download file full name
     * @return it will return the notification text content.
     */
    @Override
    public String getNotificationText(String packageName,
            String mimeType, String fullFileName) {
        //
        Xlog.i(TAG, "Enter: " + "getNotificationText" + " --OP01 implement");
        // TODO Auto-generated method stub
        String  caption = "";
        if ((packageName.equals(BROWSER_NOTIFICATION_PACKAGE)
                || packageName.equals(OMADL_NOTIFICATION_PACKAGE))
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.message")
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.content")
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.rights+wbxml")
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.rights+xml")) {
          caption = getResources().getString(R.string.notification_download_complete_op01);
          caption = caption + fullFileName;
          return caption;
        } else {
            caption = getResources().getString(R.string.notification_download_complete);
            return caption;
        }
    }
    
    /**
     * Save download path to the downloadInfo.
     * 
     * @param downloadPath The download path element in downloadInfo. 
     * @param value The value that query form db.
     */
    @Override
    public void setDownloadPath(String downloadPath, String value) {
    	Xlog.i(TAG, "Enter: " + "setDownloadPath" + " --OP01 implement");
    	downloadPath = value;
    }
    
    /**
     * Set column value to ContentValues.
     * 
     * @param columnName Column name in table.
     * @param value Column value
     * @param contentValues ContentValues that will insert to.
     */
    @Override
    public void setDownloadPathSelectFileMager(String columnName, String value, ContentValues contentValues) {
        Xlog.i(TAG, "Enter: " + "setDownloadPathSelectFileMager" + " --OP01 implement");
        contentValues.put(columnName, value);
        Xlog.d(TAG, "DownloadList:getOmaDownloadClickHandler(): onClick():" + 
                "OP01 implement, selectedPath is " + value);
    }
    
    /**
     * Copy the given key and relevant value from one Contentvalues to another
     * 
     * @param key The key string which in ContentValues copy from.
     * @param from The ContentValues that copy from. 
     * @param to The CotentValues that copy to.
     */
    @Override
    public void copyContentValues(String key, ContentValues from, ContentValues to) {
        //
        Xlog.i(TAG, "Enter: " + "copyContentValues" + " --OP01 implement");
        String s = from.getAsString(key);
        if (s != null) {
            to.put(key, s);
        }
    }
}
