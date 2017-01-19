package com.mediatek.mediatekdm.util;

import android.content.Context;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;
import com.mediatek.mediatekdm.DmConst.TAG;
import com.mediatek.mediatekdm.PlatformManager;

public final class Path {
    public static final String PATH_IN_SYSTEM = "/system/etc/dm/";
    public static final String TEST_PATH_IN_SYSTEM = PATH_IN_SYSTEM + "test/";
    public static final String PRODUCTIVE_PATH_IN_SYSTEM = PATH_IN_SYSTEM + "productive/";

    // SmsReg(Keep this sync with SmsReg)
    // TODO Use content provider to access information of SmsReg.
    public static final String SMSREG_CONFIG_FILE = "smsSelfRegConfig.xml";

    // Common
    public static final String DM_TREE_FILE = "tree.xml";
    public static final String DM_CONFIG_FILE = "config.xml";
    public static final String DM_APN_INFO_FILE = "DmApnInfo.xml";
    public static final String DM_OPERATION_FOLDER = "operations";

    //WIFIHS2.0
    public static final String DM_OSU_PACKAGE1_FILE = "OSUPackage1.xml";
    public static final String DM_REM_PACKAGE1_FILE = "RemPackage1.xml";
    public static final String DM_POLICY_PACKAGE1_FILE = "PolicyUpdatePackage1.xml";

    // LAWMO
    public static final String WIPE_FILE = "wipe";
    
    private static String getSwitchValue() {
        try {
            DmAgent agent = PlatformManager.getInstance().getDmAgent();
            if (agent != null) {
                byte[] switchValue = agent.getSwitchValue();
                if (switchValue != null) {
                    return new String(switchValue);
                } else {
                    return "0";
                }
            } else {
                Log.e(TAG.APPLICATION, "Get DmAgent fail.");
                return "-1";
            }
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }
    
    public static String getPathInSystem(String fileName) {
        if (getSwitchValue().equals("1")) {
            return PRODUCTIVE_PATH_IN_SYSTEM + fileName;
        } else {
            return TEST_PATH_IN_SYSTEM + fileName;
        }
    }
    
    public static String getPathInData(Context context, String fileName) {
        Log.d(TAG.APPLICATION, "Context " + context);
        Log.d(TAG.APPLICATION, "Files Dir " + context.getFilesDir());
        return context.getFileStreamPath(fileName).getAbsolutePath();
    }
}