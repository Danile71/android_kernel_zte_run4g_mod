package com.mediatek.keyguard.plugin;

import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.keyguard.ext.DefaultCarrierTextExt;
import com.mediatek.telephony.TelephonyManagerEx;

import com.mediatek.xlog.Xlog;

public class OP02CarrierTextExt extends DefaultCarrierTextExt {
    public static final String TAG = "OP02CarrierTextExt";

    @Override
    public CharSequence customizeCarrierText(CharSequence carrierText, CharSequence simMessage, int simId) {
        Bundle bd = null;
        try {
            ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(ServiceManager.checkService("phoneEx"));
            if (phoneEx != null) {
                bd = phoneEx.getServiceState(simId);
                ServiceState ss = ServiceState.newFromBundle(bd);
                Xlog.i(TAG, " ss.isEmergencyOnly()=" + ss.isEmergencyOnly()
                        + " for simId=" + simId);
                if (ss.isEmergencyOnly()) {
                    return simMessage;
                }
            }
        } catch (RemoteException e) {
            Xlog.i(TAG, "getServiceState error e:" + e.getMessage());
        }
        return super.customizeCarrierText(carrierText, simMessage, simId);
    }

    @Override
    public boolean showCarrierTextWhenSimMissing(boolean isSimMissing, int simId) {
        ///init sim card info
        int simcardNumber = getSimNumber();
        int[] simstate = new int[getNumOfSim()];
        simstate = initSimCardInfo();
        Xlog.i(TAG, "showCarrierText() simcardNumber = " + simcardNumber);
        if (simcardNumber == 0 || simcardNumber == 1) {
            return isSimMissing;
        } else if (simcardNumber == 2) {
            for (int i = 0; i < simstate.length; i++) {
                Xlog.i(TAG, "showCarrierText() simcardNumber == 2 simstate[] = " + simstate[i] + " i = " + i);
                if (simstate[i] == TelephonyManager.SIM_STATE_PIN_REQUIRED ||
                        simstate[i ] == TelephonyManager.SIM_STATE_PUK_REQUIRED) {
                    Xlog.i(TAG, "return isSimMissing simstate[i] = PIN or PUK");
                    return isSimMissing;
                }
            }
            boolean oneCardOutOfService = isOneCardOutOfService();
            Xlog.i(TAG, "oneCardOutOfService = " + oneCardOutOfService);
            if (!oneCardOutOfService) {
                return isSimMissing;
            } else {
                /// M: when one sim state is not in service && ! emerycallonly,sim1 must show ,so reture false
                Xlog.i(TAG, "simId = " + simId + " return = " + (PhoneConstants.GEMINI_SIM_1 != simId));
                return PhoneConstants.GEMINI_SIM_1 != simId;
            }
        }
        return isSimMissing;
    }

    private int getSimNumber() {
        Xlog.i(TAG, "getSimNumber() start ");
        int simNumber = 0;
        ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(ServiceManager.checkService("phoneEx"));
        ITelephony tmex = ITelephony.Stub.asInterface(android.os.ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        try {
            if (null != phoneEx && null != tmex) {
                Bundle bd = null;
                for (int i = PhoneConstants.GEMINI_SIM_1; i < getNumOfSim(); i++) {
                    Xlog.i(TAG, "getSimNumber() i = " + i + " getSimNumber() tmex.getSimState(i) = " + tmex.getSimState(i));
                    Xlog.i(TAG, "getSimNumber() i = " + i + "getSimNumber() isSIMInserted(i) = " + isSIMInserted(i));
                    bd = phoneEx.getServiceState(i);
                    ServiceState ss = ServiceState.newFromBundle(bd);
                    Xlog.i(TAG, "getSimNumber() i = " + i + "getSimNumber() ss.isEmergencyOnly() = " + ss.isEmergencyOnly());
                    Xlog.i(TAG, "getSimNumber() i = " + i + "getSimNumber() ss.getState() = " + ss.getState());

                    if (isSIMInserted(i) &&
                            ((TelephonyManager.SIM_STATE_PIN_REQUIRED ==  tmex.getSimState(i) ||
                            TelephonyManager.SIM_STATE_PUK_REQUIRED == tmex.getSimState(i) || 
                            TelephonyManager.SIM_STATE_READY == tmex.getSimState(i) || 
                            TelephonyManager.SIM_STATE_NETWORK_LOCKED == tmex.getSimState(i))
                            ///for radio off
                            || ss.getState() == ServiceState.STATE_POWER_OFF)) {
                        simNumber ++;
                    }
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        Xlog.i(TAG, "getSimNumber() end ");
        return simNumber;
    }

    private int[] initSimCardInfo() {
        int[] simstate = null;
        ITelephony tmex = ITelephony.Stub.asInterface(android.os.ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        try {
            if (null != tmex) {
                simstate = new int[getNumOfSim()];
                for (int i = PhoneConstants.GEMINI_SIM_1; i < getNumOfSim(); i++) {
                    simstate[i] = tmex.getSimState(i);
                    Xlog.i(TAG, "i = " + i + " tmex.getSimState(i) = " + tmex.getSimState(i));
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return simstate;
    }

    private static int getNumOfSim() {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            return PhoneConstants.GEMINI_SIM_NUM;
        } else {
            return 1;
        }
    }

    private boolean isSIMInserted(int slotId) {
            final TelephonyManagerEx mTelephonyManagerEx = TelephonyManagerEx.getDefault();
            if (mTelephonyManagerEx != null && mTelephonyManagerEx.hasIccCard(slotId)) {
                return true;
            }
        return false;
    }

    private boolean isOneCardOutOfService() {
        Bundle bd = null;
        ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(ServiceManager.checkService("phoneEx"));
        try {
            if (null != phoneEx) {
                for (int i = PhoneConstants.GEMINI_SIM_1; i < getNumOfSim(); i++) {
                    bd = phoneEx.getServiceState(i);
                    ServiceState ss = ServiceState.newFromBundle(bd);
                    Xlog.i(TAG, "isOneCardOutOfService ss.getState() = " + ss.getState() + " for simId= " + i);
                    if (ServiceState.STATE_IN_SERVICE != ss.getState() && !ss.isEmergencyOnly()) {
                        return true;
                    }
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return false;
    }
}
