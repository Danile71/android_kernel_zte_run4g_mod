package com.mediatek.recovery;

import com.mediatek.common.recovery.AbstractRecoveryMethod;

import android.content.pm.UserInfo;
import android.util.Slog;
import android.os.Environment;
import com.android.server.pm.Installer;
import com.android.server.pm.PackageManagerService;

import java.io.File;
import java.util.List;

public class PackageManagerRecovery extends AbstractRecoveryMethod {
    private final String TAG = "PackageManagerRecovery";
    private Installer mInstaller = new Installer();
    private File dataDir = Environment.getDataDirectory();
    private File mAppDataDir = new File(dataDir, "data");
    private File mUserAppDataDir = new File(dataDir, "user");
    private File mSystemDir = new File(dataDir, "system");
    private File mSettingsFilename = new File(mSystemDir, "packages.xml");
    private File mBackupSettingsFilename = new File(mSystemDir, "packages-backup.xml");

    @Override
    public int doRecover(Object param) {
        Slog.d(TAG, "doRecover called, path=" + param);
        mInstaller.ping();
        recoverSettings(PackageManagerService.sUserManager.getUsers(false));
        return RECOVER_METHOD_SUCCESS;
    }

    /** M: [PMS Recovery] Recover packages.xml & packages-backup.xml @{ */
    private void recoverSettings(List<UserInfo> users) {
        /// M: [PMS Recovery] We will remove the broken file first
        try {
            if (mBackupSettingsFilename.exists()) {
                Slog.d(TAG, "Recovery remove settings backup file");
                mBackupSettingsFilename.delete();
            }
            if (mSettingsFilename.exists()) {
                Slog.d(TAG, "Recovery try to remove settings file");
                mSettingsFilename.delete();
            }
        } catch (Exception e){
            Slog.e(TAG, "Recovery cannot remove settings or backup file");
        }
              
        for(int i = 0; i < users.size(); i++) {
            UserInfo user = users.get(i);
            File dataPath;
            if (user.id == 0) {
                dataPath = mAppDataDir;
            } else {
                dataPath = getDataPathForUser(user.id);
            }
            String[] files = dataPath.list();
            if (files == null) {
                return;
            }
            File lastUserStateFile, lastUserStateBackupFile, recoverFile;
            lastUserStateFile = getUserPackagesStateFile(user.id);
            lastUserStateBackupFile = getUserPackagesStateBackupFile(user.id);
            recoverFile = getUserPackagesStateRecoveryFile(user.id);
            
            /// M: [PMS Recovery] Backup restriction files for each user.
            /// We will restore these files latter when recovery is finished
            if (lastUserStateBackupFile.exists()) {
                lastUserStateBackupFile.renameTo(recoverFile);
            } else if (lastUserStateFile.exists()) {
                lastUserStateFile.renameTo(recoverFile);
            }
            /// M: [PMS Recovery] For each user data folder, call installd to change the uid to root
            for (int j = 0; j < files.length; j++) {
                if (mInstaller.changeUidRoot(files[j], user.id) < 0) {
                    Slog.e(TAG, "Change data folder uid to root of package " + files[j] + " failed");
                }
            }
        }
    }

    /// M: PMS Recovery] Package restriction files restore
    private void restoreRestrictions(List<UserInfo> users) {
        Slog.d(TAG, "Trying to restore package restriction file for each user");
        for(int i = 0; i < users.size(); i++) {
            UserInfo user = users.get(i);
            File lastUserStateFile, lastUserStateBackupFile, recoverFile;
            lastUserStateFile = getUserPackagesStateFile(user.id);
            lastUserStateBackupFile = getUserPackagesStateBackupFile(user.id);
            recoverFile = getUserPackagesStateRecoveryFile(user.id);
            if (recoverFile.exists()) {
                if (lastUserStateBackupFile.exists()) {
                    Slog.d(TAG, "Restore to restriction backup file");
                    lastUserStateBackupFile.delete();
                    recoverFile.renameTo(lastUserStateBackupFile);
                } else if (lastUserStateFile.exists()) {
                    Slog.d(TAG, "Restore to restriction file");
                    lastUserStateFile.delete();
                    recoverFile.renameTo(lastUserStateFile);
                }
            } else {
                Slog.w(TAG, "User: " + user.id + " restriction file doesn't exist, skip restore");
            }
        }
    }
    /** @} */

    private File getDataPathForUser(int userId) {
        return new File(mUserAppDataDir.getAbsolutePath() + File.separator + userId);
    }

    private File getUserPackagesStateFile(int userId) {
        return new File(Environment.getUserSystemDirectory(userId), "package-restrictions.xml");
    }

    private File getUserPackagesStateBackupFile(int userId) {
        return new File(Environment.getUserSystemDirectory(userId),
                "package-restrictions-backup.xml");
    }

    /** M: [PMS Recovery] Util function to get user package restriction file path @{ */
    private File getUserPackagesStateRecoveryFile(int userId) {
        return new File(Environment.getUserSystemDirectory(userId),
                "package-restrictions-recovery.xml");
    }
    /** @} */
}
