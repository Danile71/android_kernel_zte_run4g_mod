package com.mediatek.op.server;

import android.app.AlertDialog;
import android.content.Context;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.SimInfo;
import android.telephony.TelephonyManager;
import android.view.WindowManager;

import com.android.internal.telephony.ITelephony;
import com.mediatek.internal.R;
import com.mediatek.xlog.Xlog;

public class OP09ConnectivityRoamingExt extends DefConnectivityRoamingExt {
    private static final String TAG = "OP09ConnectivityRoamingExt";

    @Override
    public void setFirstRoamingFlag(Context context, boolean roaming) {
        Xlog.i(TAG, "setFirstRoamingFlagxx, roaming=" + roaming);
        int roamingFlag = roaming ? 1 : 0;
        Settings.System.putInt(context.getContentResolver(),
                Settings.System.DATA_CONNECTION_FIRST_ROAMING, roamingFlag);
    }

    @Override
    public boolean isFirstEnterRoamingArea(Context context) {
        int roamingFlag = Settings.System.getInt(context.getContentResolver(),
                Settings.System.DATA_CONNECTION_FIRST_ROAMING, 0);
        Xlog.i(TAG, "isFirstEnterRoamingArea, roamingFlag=" + roamingFlag);
        return roamingFlag == 1;
    }

    @Override
    public void popupAlertDialog(Context context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        //builder.setTitle(R.string.alert_prompt);
        builder.setMessage(R.string.roaming_message);
        builder.setPositiveButton(R.string.ok, null);
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        dialog.show();
    }

    @Override
    public void setMobileDataDisabled(Context context, Handler handler, int msg) {
        Settings.Secure.putInt(context.getContentResolver(),
                Settings.Global.MOBILE_DATA, 0);
        int preSlotId = Settings.System.getInt(context.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SETTING,
                Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
        int currentSlotId = preSlotId;
        try {
            ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager
                    .getService(Context.TELEPHONY_SERVICE));
            if (telephony == null) {
                Xlog.i(TAG, "null in Telephony");
                return;
            }
            if (telephony.getSimState(preSlotId) != TelephonyManager.SIM_STATE_READY) {
                currentSlotId = SimInfo.SLOT_NONE;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            return;
        }
        Settings.System.putInt(context.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SETTING, Integer
                        .valueOf(currentSlotId + 1));
        Long currenSimId = getIdBySlot(context, currentSlotId);
        Settings.System.putLong(context.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SIM_SETTING, currenSimId);
        Xlog.i(TAG, "setMobileDataDisabled, preSlotId=" + preSlotId
                + ", currentSlotId=" + currentSlotId + ", preSimId=" + currenSimId);
        handler.sendMessage(handler.obtainMessage(msg, preSlotId, SimInfo.SLOT_NONE));
    }

    private long getIdBySlot(Context context, int slotId) {
        long simId = 0;
        SIMInfo simInfo = SIMInfo.getSIMInfoBySlot(context, slotId);
        if (null == simInfo) {
            Xlog.e(TAG, "[getIdBySlot] simInfo is null");
        } else {
            simId = simInfo.mSimId;
        }
        return simId;
    }
}
