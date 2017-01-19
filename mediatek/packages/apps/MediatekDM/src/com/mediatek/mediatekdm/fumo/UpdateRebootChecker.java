package com.mediatek.mediatekdm.fumo;

import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.mediatekdm.DmConst.TAG;
import com.mediatek.mediatekdm.DmService;
import com.mediatek.mediatekdm.KickoffActor;
import com.mediatek.mediatekdm.PlatformManager;
import com.mediatek.mediatekdm.util.Path;

import java.io.File;

public class UpdateRebootChecker extends KickoffActor {

    public UpdateRebootChecker(Context context) {
        super(context);
    }

    @Override
    public void run() {
        final String bootTimestamp = FumoComponent.getTimeStamp(mContext, FumoComponent.BOOT_TIME_FILE_NAME);
        final String fotaFlagTimestamp= FumoComponent.getTimeStamp(mContext, FumoComponent.FOTA_FLAG_FILE);
        if (bootTimestamp != null && fotaFlagTimestamp != null && bootTimestamp.equalsIgnoreCase(fotaFlagTimestamp)) {
            Log.d(TAG.RECEIVER, "Not reboot, do nothing.");
            return;
        }

        // Check whether we need to report FUMO update result
        if (isUpdateReboot()) {
            // Get the result
            boolean isUpdateSuccessful = false;
            if (FeatureOption.MTK_EMMC_SUPPORT) {
                int otaResult = 0;
                try {
                    otaResult = PlatformManager.getInstance().getDmAgent().readOtaResult();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                isUpdateSuccessful = (otaResult == 1);
            } else {
                isUpdateSuccessful = FotaDeltaFiles.verifyUpdateStatus();
            }

            Intent intent = new Intent(FumoComponent.UPDATE_REBOOT_CHECK);
            intent.setClass(mContext, DmService.class);
            intent.putExtra("UpdateSucceeded", isUpdateSuccessful);
            mContext.startService(intent);
        }
    }

    private boolean isUpdateReboot() {
        Log.i(TAG.RECEIVER, "Check the existence of update flag file.");
        boolean ret = false;
        try {
            File updateFile = new File(Path.getPathInData(mContext, FumoComponent.FOTA_FLAG_FILE));
            if (updateFile.exists()) {
                Log.d(TAG.RECEIVER, "FOTA flag file exists.");
                ret = true;
            }
        } catch (SecurityException e) {
            Log.e(TAG.RECEIVER, e.toString());
            e.printStackTrace();
        }
        return ret;
    }
}
