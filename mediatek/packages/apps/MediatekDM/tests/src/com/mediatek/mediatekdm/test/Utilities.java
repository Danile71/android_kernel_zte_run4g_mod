
package com.mediatek.mediatekdm.test;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

public class Utilities {
    public static final String TAG = "MDMTest/Utilities";

    public static Context getTestPackageContext(Context currentContext) {
        try {
            Context result = null;
            result = currentContext.createPackageContext("com.mediatek.mediatekdm.test",
                    Context.CONTEXT_IGNORE_SECURITY);
            Log.w(TAG, "Result: " + result);
            return result;
        } catch (NameNotFoundException e) {
            throw new Error(e);
        }
    }

    public static void removeDirectoryRecursively(File directory) {
        Log.v(TAG, "+removeDirectoryRecursively(" + directory + ")");
        File[] children = directory.listFiles();
        for (File child : children) {
            if (child.isFile()) {
                Log.v(TAG, "Remove file " + child);
                child.delete();
            } else if (child.isDirectory()) {
                removeDirectoryRecursively(child);
                child.delete();
            }
        }
        Log.w(TAG, "-removeDirectoryRecursively()");
    }

    public static void removeDirectoryRecursively(File directory, String pattern) {
        Log.v(TAG, "+removeDirectoryRecursively(" + directory + ")");
        File[] children = directory.listFiles();
        for (File child : children) {
            if (child.isFile()) {
                if (child.getAbsolutePath().contains(pattern)) {
                    Log.v(TAG, "Remove file " + child);
                    child.delete();
                }
            } else if (child.isDirectory()) {
                removeDirectoryRecursively(child, pattern);
                if (child.getAbsolutePath().contains(pattern)) {
                    child.delete();
                }
            }
        }
        Log.w(TAG, "-removeDirectoryRecursively()");
    }

    public static void copyFile(File src, File dst) {
        Log.i(TAG, "Copy from " + src + " to " + dst);
        final int bufferSize = 1024 * 10;
        FileInputStream in = null;
        FileOutputStream out = null;

        try {
            in = new FileInputStream(src);
            out = new FileOutputStream(dst);
            byte[] buffer = new byte[bufferSize];
            while (true) {
                int ins = in.read(buffer);
                if (ins == -1) {
                    in.close();
                    out.flush();
                    out.close();
                    break;
                } else {
                    out.write(buffer, 0, ins);
                    out.flush();
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "IOException " + e);
            throw new Error(e);
        } finally {
            try {
                if (in != null) {
                    in.close();
                    in = null;
                }
                if (out != null) {
                    out.close();
                    out = null;
                }
            } catch (IOException e) {
                Log.e(TAG, "IOException 2 " + e);
                throw new Error(e);
            }
        }
    }
}
