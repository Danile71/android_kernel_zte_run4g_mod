
package com.mediatek.datatransfer.utils;

import android.app.DialogFragment;
import android.content.Context;
import android.os.Environment;
import android.os.Looper;
import android.os.RemoteException;
import android.os.storage.StorageManager;
import com.mediatek.storage.StorageManagerEx;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.datatransfer.R;
import com.mediatek.datatransfer.utils.Constants.LogTag;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class SDCardUtils {

    public final static int MINIMUM_SIZE = 512;
    public final static String CLASS_TAG = "SDCardUtils";

    public static String getBackupStoragePath(Context context) {
        String storagePath = null;
        StorageManager storageManager = StorageManager.from(context);

        storagePath = StorageManagerEx.getExternalStoragePath();
        if (storagePath == null || storagePath.isEmpty()) {
            storagePath = StorageManagerEx.getInternalStoragePath();
            MyLogger.logE(CLASS_TAG, "Can't find External Storage use Internal Storage instead.");
        }
        if (!Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(storagePath))) {
            MyLogger.logE(CLASS_TAG, storagePath + "  MEDIA_UNMOUNTED!!!");
            storagePath = StorageManagerEx.getInternalStoragePath();
            MyLogger.logE(CLASS_TAG, storagePath + "  getInternalStoragePath it's state is "
                    + storageManager.getVolumeState(storagePath));
            if (!Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(storagePath))) {
                return "";
            }
        }
        return storagePath;
    }

    public static String getInternalStoragePath(Context context) {
        String storagePath = null;
        StorageManager storageManager = StorageManager.from(context);

        storagePath = StorageManagerEx.getInternalStoragePath();
        if (storagePath != null && !storagePath.isEmpty()
                && Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(storagePath))) {
            return storagePath;
        }
        return storagePath == null ? "" : storagePath;
    }

    /**
     * @Deprecated Use the new {@link #getBackupStoragePath} class with
     * @return true SD card Available
     */
    @Deprecated
    public static String getExternalStoragePath(Context context) {
        String storagePath = null;
        StorageManager storageManager = StorageManager.from(context);

        storagePath = StorageManagerEx.getExternalStoragePath();
        if (storagePath == null || storagePath.isEmpty()) {
            MyLogger.logE("SDCardUtils", "storagePath is null");
            return null;
        }
        if (!Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(storagePath))) {
            return null;
        }
        return storagePath;
    }

    public static String getSDStatueMessage(Context context) {
        String message = context.getString(R.string.nosdcard_notice);
        String status = Environment.getExternalStorageState();
        if (status.equals(Environment.MEDIA_SHARED) ||
                status.equals(Environment.MEDIA_UNMOUNTED)) {
            message = context.getString(R.string.sdcard_busy_message);
        }
        return message;
    }

    public static String getStoragePath(Context context) {
        // This method has been out-time
        // String storagePath = getExternalStoragePath();
        String storagePath = getBackupStoragePath(context);
        if (storagePath == null || storagePath.trim().equals("")) {
            return null;
        }
        MyLogger.logD(CLASS_TAG, "getExternalStoragePath: path is " + storagePath);
        storagePath = makePersonalDataPath(storagePath);
        MyLogger.logD(CLASS_TAG, "getStoragePath: path is " + storagePath);
        return checkFile(storagePath);
    }

    public static List<String> getRestoreStoragePath(Context context) {
        List<String> result = new ArrayList<String>();
        // This method has been out-time
        // String storagePath = getExternalStoragePath();
        String storagePath = getBackupStoragePath(context);
        String internalPath = getInternalStoragePath(context);
        if (!storagePath.startsWith(internalPath)) {
            if (null != checkFile(makePersonalDataPath(internalPath))) {
                result.add(checkFile(makePersonalDataPath(internalPath)));
            }
        }
        MyLogger.logD(CLASS_TAG, "getExternalStoragePath: path is " + storagePath);
        storagePath = makePersonalDataPath(storagePath);
        MyLogger.logD(CLASS_TAG, "getStoragePath: path is " + storagePath);
        String tempPath = checkFile(storagePath);
        if (tempPath != null) {
            result.add(tempPath);
        }
        return result;
    }

    private static String makePersonalDataPath(String storagePath) {
        return storagePath + File.separator + "ct_backup" + File.separator + "backup_all"
                + File.separator;
    }

    private static String checkFile(String storagePath) {
        File file = new File(storagePath);
        if (file != null) {

            if (file.exists() && file.isDirectory()) {
                File temp = new File(storagePath + File.separator
                        + ".BackupRestoretemp");
                boolean ret;
                if (temp.exists()) {
                    ret = temp.delete();
                } else {
                    try {
                        ret = temp.createNewFile();
                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.e(LogTag.LOG_TAG,
                                "getStoragePath: " + e.getMessage());
                        ret = false;
                    } finally {
                        temp.delete();
                    }
                }
                if (ret) {
                    return storagePath;
                } else {
                    return null;
                }

            } else if (file.mkdirs()) {
                return storagePath;
            } else {
                MyLogger.logE(CLASS_TAG, "file.mkdir(): FAILED!");
            }
        } else {
            MyLogger.logE(LogTag.LOG_TAG,
                    "getStoragePath: path is not ok");
            return null;
        }
        return null;
    }

    /**
     * @Deprecated Use the new {@link #getBackupStoragePath} class with
     * @return true SD card Available
     */
    @Deprecated
    public static String getPersonalDataBackupPath(Context context) {// Context
        // context
        String path = getStoragePath(context);
        if (path != null) {
            return path + File.separator;
        }

        return path;
    }

    public static String getAppsBackupPath(Context context) {// Context context
        String path = getStoragePath(context);
        if (path != null) {
            return path + ModulePath.FOLDER_APP;
        }
        return path;
    }

    /**
     * @Deprecated Use the new {@link #isStorageAvailable} class with
     * @return true SD card Available
     */
    @Deprecated
    public static boolean isSdCardAvailable(Context context) { // Context
        // context
        return (getStoragePath(context) != null);
    }

    /**
     * @return
     */
    public static boolean isStorageAvailable(Context context) {// Context
        // context
        return (getStoragePath(context) != null);
    }

    public static long getAvailableSize(String file) {
        android.os.StatFs stat = new android.os.StatFs(file);
        long count = stat.getAvailableBlocks();
        long size = stat.getBlockSize();
        long totalSize = count * size;
        Log.v(LogTag.LOG_TAG, "file remain size = " + totalSize);
        return totalSize;
    }
}
