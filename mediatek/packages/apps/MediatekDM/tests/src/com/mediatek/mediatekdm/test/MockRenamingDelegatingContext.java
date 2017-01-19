
package com.mediatek.mediatekdm.test;

import android.content.Context;
import android.test.RenamingDelegatingContext;
import android.util.Log;

import java.io.File;

public class MockRenamingDelegatingContext extends RenamingDelegatingContext {
    public static final String DEFAULT_PREFIX = "test_";

    private static final String TAG = "MDMTest/MockRenamingDelegatingContext";
    private final Object mSync = new Object();
    private final Context mTargetContext;
    private final String mFilePrefix;

    public MockRenamingDelegatingContext(Context targetContext, String prefix) {
        super(targetContext, prefix);
        makeExistingFilesAndDbsAccessible();
        Log.d(TAG, "MockRenamingDelegatingContext(" + targetContext + ", " + prefix + ")");
        mTargetContext = targetContext;
        mFilePrefix = prefix;
    }

    public MockRenamingDelegatingContext(Context targetContext) {
        this(targetContext, DEFAULT_PREFIX);
    }

//    @Override
//    public File getDir(String name, int mode) {
//        return mTargetContext.getDir(renamedFileName(name), mode);
//    }
//
//    @Override
//    public File getFilesDir() {
//        File filesDir = new File(mTargetContext.getFilesDir(), renamedFileName("files"));
//        synchronized (mSync) {
//            if (!filesDir.exists()) {
//                if (!filesDir.mkdirs()) {
//                    Log.w(TAG, "Unable to create files directory");
//                    return null;
//                }
//                FileUtils.setPermissions(
//                        filesDir.getPath(),
//                        FileUtils.S_IRWXU | FileUtils.S_IRWXG | FileUtils.S_IXOTH,
//                        -1,
//                        -1);
//            }
//        }
//        return filesDir;
//    }
//
    private String renamedFileName(String name) {
        return mFilePrefix + name;
    }

    public void clearRenamedFiles() {
        File filesDir = mTargetContext.getFilesDir();
        Log.d(TAG, "Parent is " + filesDir.getParentFile());
        Utilities.removeDirectoryRecursively(filesDir.getParentFile(), mFilePrefix);
    }
}
