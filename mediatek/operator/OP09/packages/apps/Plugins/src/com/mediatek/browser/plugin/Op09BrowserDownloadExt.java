package com.mediatek.browser.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.content.Context;
import android.database.Cursor;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Environment;
import android.os.StatFs;

import com.mediatek.browser.ext.DefaultBrowserDownloadExt;
import com.mediatek.op09.plugin.R;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.xlog.Xlog;

public class Op09BrowserDownloadExt extends DefaultBrowserDownloadExt {

    private static final String TAG = "Op09BrowserDownloadExt";

    private static final String DEFAULT_DOWNLOAD_DIRECTORY_OP09 = "/storage/sdcard0/Download";
    private static final String DEFAULT_DOWNLOAD_SDCARD2_DIRECTORY_OP09 = "/storage/sdcard1/Download";

    private static final String EXTRA_STORAGE_DIR = "/storage/sdcard1";
    private static final long LowSpaceThreshold = 10 * 1024 * 1024;

    private Context mContext;

    public Op09BrowserDownloadExt(Context context) {
        super();
        mContext = context;
    }

    public void setRequestDestinationDir(String downloadPath, Request mRequest,
                    String filename, String mimeType) {
        Xlog.i(TAG, "Enter: " + "setRequestDestinationDir" + " --OP09 implement");

        String path = null;
        if (downloadPath.equalsIgnoreCase(DEFAULT_DOWNLOAD_DIRECTORY_OP09)
                || downloadPath.equalsIgnoreCase(DEFAULT_DOWNLOAD_SDCARD2_DIRECTORY_OP09)) {
            String folder = getStorageDirectoryForOperator(mimeType);
            path = "file://" + downloadPath + "/" + folder + "/" + filename;
        } else {
            path = "file://" + downloadPath + "/" + filename;
        }

        Uri downloadUri = Uri.parse(path);
        Xlog.i(TAG, "For OP09: selected download full path is: " + path + " MimeType is: "
                + mimeType + " and Uri is: " + downloadUri);
        mRequest.setDestinationUri(downloadUri);
    }

    public String getStorageDirectoryForOperator(String mimeType) {

        Xlog.i(TAG, "Enter: " + "getStorageDirectoryForOperator" + " --OP09 implement");

        // if mimeType is null, do not set sub dir.
        if (mimeType == null) {
            return "Others";
        }

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

        Xlog.d(TAG, "mimeType is: " + mimeType + ",MediaFileType is: " +
            fileType + ",folder is: " + selectionStr);

        return selectionStr;
    }

    public boolean checkStorageBeforeDownload(Activity activity, String downloadPath, long contentLength) {
        Xlog.i(TAG, "Enter: "  + "checkStorageBeforeDownload" + " --OP09 implement");
        if (contentLength <= 0) {
            return false;
        }

        Xlog.i(TAG, "before checkIfHaveAvailableStoreage(),contentLength: " + contentLength);
        return checkIfHaveAvailableStoreage(downloadPath, activity, contentLength);
    }

    private boolean checkIfHaveAvailableStoreage(String path, Activity activity, long contentLength) {
        String downloadPath = Uri.parse(path).getPath();
        String externalPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        if (downloadPath != null && downloadPath.startsWith(externalPath)) {
            if (availableStorage(externalPath, activity) < contentLength) {
                Xlog.i(TAG, "external storage is download path, can to download because of low storeage " +
                         "and will popup low storeage dialog");

                new AlertDialog.Builder(activity)
                    .setTitle(mContext.getResources().getString(R.string.low_storage_dialog_title_on_external))
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setMessage(mContext.getResources().getString(R.string.low_storage_dialog_msg_on_external))
                    .setPositiveButton(android.R.string.ok, null)
                    .show();
                return true;
            }
        } else if (downloadPath != null && downloadPath.startsWith(EXTRA_STORAGE_DIR)) {
            if (availableStorage(EXTRA_STORAGE_DIR, activity) < contentLength) {
                Xlog.i(TAG, "extra storage is download path, can to download because of low storeage " +
                        "and will popup low storeage dialog");

                new AlertDialog.Builder(activity)
                    .setTitle(mContext.getResources().getString(R.string.low_storage_dialog_title_on_extra))
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setMessage(mContext.getResources().getString(R.string.low_storage_dialog_msg_on_extra))
                    .setPositiveButton(android.R.string.ok, null)
                    .show();
                return true;
            }
        }

        return false;
    }

    private long availableStorage(String path, Activity activity) {
        long availableStorage = getAvailableBytesInFileSystemAtGivenRoot(path) - getDownloadsInProgressWillOccupyBytes(activity) - LowSpaceThreshold;
        Xlog.i(TAG, "check storage before download, availableStorage : " + availableStorage + ", about" + availableStorage / (1 * 1024 * 1024) + "M");
        return availableStorage;
    }

    private long getAvailableBytesInFileSystemAtGivenRoot(String path) {
        StatFs stat = new StatFs(path);
        long availableBlocks = (long)stat.getAvailableBlocks();
        long size = stat.getBlockSize() * availableBlocks;
        return size;
    }

    private long getDownloadsInProgressWillOccupyBytes(Activity activity) {
        long downloadsWillOccupyBytes = 0l;
        Cursor cursor = null;
        DownloadManager manager = (DownloadManager)activity.getSystemService(Context.DOWNLOAD_SERVICE);
        try {
           cursor = manager.query(new DownloadManager.Query().setFilterByStatus(DownloadManager.STATUS_RUNNING));
           if (cursor != null) {
               for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
                   long downloadID = cursor.getLong(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_ID));
                   long totalBytes = cursor.getLong(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_TOTAL_SIZE_BYTES));
                   long currentBytes = cursor.getLong(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR));
                   if (totalBytes > 0 && currentBytes > 0 && totalBytes - currentBytes > 0) {
                       downloadsWillOccupyBytes += totalBytes - currentBytes;
                       Xlog.i(TAG, "Download id :" + downloadID + " in downloading, totalBytes: " + totalBytes + ",currentBytes: " + currentBytes);
                   }
               }
           }
        } catch (IllegalStateException e) {
            Xlog.i(TAG, "getDownloadsInProgressWillOccupyBytes: query encounter exception");
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        Xlog.i(TAG, "getDownloadsInProgressWillOccupyBytes: return downloadsWillOccupyBytes:" + downloadsWillOccupyBytes);
        return downloadsWillOccupyBytes;
    }

}
