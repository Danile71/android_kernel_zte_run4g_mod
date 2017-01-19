package com.mediatek.downloadmanager.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.media.MediaFile;
import android.os.Environment;

import java.io.File;

import com.mediatek.downloadmanager.ext.DefaultDownloadProviderFeatureExt;
import com.mediatek.xlog.Xlog;


public class Op09DownloadProviderFeatureExt extends DefaultDownloadProviderFeatureExt {
    
    private static final String TAG = "DownloadProviderPluginEx";
    
    private static final String DEFAULT_DOWNLOAD_FOLDER_OP09 = "Download";
    
    public Op09DownloadProviderFeatureExt(Context context) {
        super(context);
    }
    
    /**
     * Get the default download dir according to mimetype.
     * 
     * @param mimeType The mimetype of donwload file.
     * @return Directory string
     */
    @Override
    public String getStorageDirectory(String mimeType) {
        Xlog.i(TAG, "Enter: " + "getStorageDirectory" + " --OP09 implement");
        
        int fileType = MediaFile.getFileTypeForMimeType(mimeType);
        String selectionStr = null;

        if (mimeType.startsWith("audio/") || MediaFile.isAudioFileType(fileType)) {
            selectionStr = "Music";
        } else if (mimeType.startsWith("image/") || MediaFile.isImageFileType(fileType)) {
            selectionStr = "Picture";
        } else if (mimeType.startsWith("video/") || MediaFile.isVideoFileType(fileType)) {
            selectionStr = "Video";
        } else if (mimeType.startsWith("text/") || mimeType.equalsIgnoreCase("application/msword")
                || mimeType.equalsIgnoreCase("application/vnd.ms-powerpoint")
                || mimeType.equalsIgnoreCase("application/pdf")) {
            selectionStr = "Document";
        } else if (mimeType.equalsIgnoreCase("application/vnd.android.package-archive")) {
            selectionStr = "Application";
        } else {
            selectionStr = "Others";
        }
       
        selectionStr = DEFAULT_DOWNLOAD_FOLDER_OP09 + File.separator + selectionStr;

        Xlog.i(TAG, "mimeType is: " + mimeType + "MediaFileType is: " + fileType + 
                "folder is: " + selectionStr);
        return selectionStr;
    }
  
    /**
     * Finish current activity.
     * 
     * @param activity Current activity instance.
     */
    public void finishCurrentActivity(Activity activity){
        Xlog.i(TAG, "Enter: " + "finishCurrentActivity" + " --OP09 implement");
        activity.finish();
    }
}
