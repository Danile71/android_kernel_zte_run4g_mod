package com.mediatek.recovery;

import java.io.File;

import com.mediatek.common.recovery.AbstractRecoveryMethod;
import android.util.Slog;

public class UserManagerRecovery extends AbstractRecoveryMethod {
    private final String TAG = "UserManagerRecoveryMethod";
    
    @Override
    public int doRecover(Object param) {
        Slog.d(TAG, "doRecover called, path=" + param);
        File userlist = new File("/data/system/users/userlist.xml");
        if (userlist.exists()) {
            if (!userlist.delete()) {
                Slog.e(TAG, "remove userlist.xml failed");
                return RECOVER_METHOD_FAILED; 
            }
        }
        return RECOVER_METHOD_SUCCESS;
    }

}
