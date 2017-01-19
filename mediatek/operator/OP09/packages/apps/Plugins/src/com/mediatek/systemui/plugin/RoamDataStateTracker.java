package com.mediatek.systemui.plugin;

import android.content.Context;
import android.os.RemoteException;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.systemui.statusbar.util.SIMHelper;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

/**
 * Support Roam Data.
 */
public final class RoamDataStateTracker {
    private static final String TAG = "RoamDataStateTracker";

    private Context mContext;
    private boolean mEnabled;
    private long mSimId;

    RoamDataStateTracker(Context context) {
        mContext = context;
        SimInfoManager.SimInfoRecord simInfo = SIMHelper.getSIMInfoBySlot(
                context, PhoneConstants.GEMINI_SIM_1);
        if (simInfo != null) {
            mSimId = simInfo.mSimInfoId;
        }
    }

    public void toggleState() {
        Xlog.d(TAG, "RoamDataStateTracker toggleState : should enable = "
                + !mEnabled);
        mEnabled = !mEnabled;
        setDataRoaming(mEnabled);
    }

    public void setEnable(boolean enable) {
        mEnabled = enable;
    }

    private void setDataRoaming(boolean enable) {
        try {
            TelephonyManagerEx.getDefault().setDataRoamingEnabledGemini(enable,
                    PhoneConstants.GEMINI_SIM_1);
        } catch (RemoteException e) {
            return;
        }

        int roamingState;
        if (enable) {
            roamingState = SimInfoManager.DATA_ROAMING_ENABLE;
        } else {
            roamingState = SimInfoManager.DATA_ROAMING_DISABLE;
        }
        SimInfoManager.setDataRoaming(mContext, roamingState, PhoneConstants.GEMINI_SIM_1);
    }
}
