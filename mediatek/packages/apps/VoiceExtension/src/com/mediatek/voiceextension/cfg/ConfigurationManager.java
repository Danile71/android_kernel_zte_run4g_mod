package com.mediatek.voiceextension.cfg;

import java.io.File;
import java.util.HashMap;

import android.content.Context;
import android.os.FileUtils;
import android.util.Log;

import com.mediatek.voiceextension.common.CommonManager;

public class ConfigurationManager {

    private static ConfigurationManager sCfgMgr;
    private static byte[] sInstanceLock = new byte[0];
    private ConfigurationXml mConfigurationXml;
    private final HashMap<String, String> mPaths = new HashMap<String, String>();
    private boolean mIsInit = false;
    private Context mContext;

    private String mServiceDataDir;

    private final String mModelName = "ModelFile";
    private final String mDatabaseName = "Database";

    public static ConfigurationManager getInstance() {
        if (sCfgMgr == null) {
            synchronized (sInstanceLock) {
                if (sCfgMgr == null) {
                    sCfgMgr = new ConfigurationManager();
                }
            }
        }
        return sCfgMgr;
    }

    public void init(Context context) {
        if (!mIsInit) {
            mConfigurationXml = new ConfigurationXml(context);
            mConfigurationXml.readVoiceFilePathFromXml(mPaths);
            mContext = context;
            mServiceDataDir = mContext.getApplicationInfo().dataDir;
            String databasePath = mServiceDataDir + "/database/";
            mPaths.put(mDatabaseName, databasePath);
            makeDirForPath(databasePath);
            mIsInit = true;
            Log.i(CommonManager.TAG, "Cfg init success");
        }
    }

    public String getModelPath() {
        return mPaths.get(mModelName);
    }

    public String getDatabasePath() {
        return mPaths.get(mDatabaseName);
    }

    /**
     * create dir and file
     * 
     * @param path
     * @return
     */
    public boolean makeDirForPath(String path) {
        if (path == null) {
            return false;
        }
        try {
            File dir = new File(path);
            if (!dir.exists()) {
                dir.mkdirs();
                FileUtils.setPermissions(dir.getPath(), 0775, -1, -1); // dwxrwxr-x
            }
        } catch (NullPointerException ex) {
            return false;
        }
        return true;
    }

    /**
     * 
     * @param file
     * @return
     */
    public boolean makeDirForFile(String file) {
        if (file == null) {
            return false;
        }
        try {
            File f = new File(file);
            File dir = f.getParentFile();
            if (!dir.exists()) {
                dir.mkdirs();
                FileUtils.setPermissions(dir.getPath(), 0775, -1, -1); // dwxrwxr-x
            }
            FileUtils.setPermissions(f.getPath(), 0666, -1, -1); // -rw-rw-rw-
        } catch (NullPointerException ex) {
            return false;
        }
        return true;
    }

}
