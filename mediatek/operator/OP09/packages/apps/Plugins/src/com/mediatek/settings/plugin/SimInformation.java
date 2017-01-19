package com.mediatek.settings.plugin;

import android.content.Context;
import android.os.ServiceManager;
import android.os.SystemProperties;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

class SimItem {
    public boolean mIsSim = true;
    public String mName = null;
    public int mColor = -1;
    public int mSlot = -1;
    public long mSimId = -1;
    public String mNumber = null;
    public int mDispalyNumberFormat = 0;
    public int mState = PhoneConstants.SIM_INDICATOR_NORMAL;

    /**
     * Construct of SimItem
     * 
     * @param name String name of sim card
     * @param color Int color of sim card
     * @param simID Long sim id of sim card
     */
    public SimItem(String name, int color, long simId) {
        mName = name;
        mColor = color;
        mIsSim = false;
        mSimId = simId;
    }

    /**
     *
     * @param siminfo SIMInfo
     */
    public SimItem(SimInfoRecord siminfo) {
        mIsSim = true;
        mName = siminfo.mDisplayName;
        mColor = siminfo.mColor;
        mSlot = siminfo.mSimSlotId;
        mSimId = siminfo.mSimInfoId;
        mNumber = siminfo.mNumber;
        mDispalyNumberFormat = siminfo.mDispalyNumberFormat;
    }
}

public class SimInformation {
    private static final String TAG = "OP09/SimInformation";

    private static final int COLORNUM = 7;

    private static final int TYPE_VOICECALL = 1;
    private static final int TYPE_VIDEOCALL = 2;
    private static final int TYPE_SMS = 3;
    private static final int TYPE_GPRS = 4;
    private static final int INTERNET_CALL_COLOR = 8;
    private static final int NO_COLOR = -1;

    private static final String OLD_NETWORK_MODE = "com.android.phone.OLD_NETWORK_MODE";
    private static final String NEW_NETWORK_MODE = "NEW_NETWORK_MODE";

    private static final String NETWORK_MODE_CHANGE_BROADCAST = "com.android.phone.NETWORK_MODE_CHANGE";
    private static final String NETWORK_MODE_CHANGE_RESPONSE = "com.android.phone.NETWORK_MODE_CHANGE_RESPONSE";

    private static final int SINGLE_SIM_CARD = 1;
    private static final int DOUBLE_SIM_CARD = 2;
    private static final String NATIVE_MCC_SIM1 = "460"; // 46003
    private static final String NATIVE_MCC_SIM2 = "455"; // 45502

    public static final int SIM_INSERTED_BASE = 0;
    public static final int NO_SIM_INSERTED = SIM_INSERTED_BASE + 1;
    public static final int ONLY_SLOT1_INSERTED =  SIM_INSERTED_BASE + 2;
    public static final int ONLY_SLOT2_INSERTED = SIM_INSERTED_BASE + 3;
    public static final int TWO_SIM_INSERTED = SIM_INSERTED_BASE + 4;

    private static int sG3SlotID = PhoneConstants.GEMINI_SIM_1;

    private Context mContext;
    private boolean mIsSlot1Insert = false;
    private boolean mIsSlot2Insert = false;
    private TelephonyManagerEx mTelephonyManagerEx;
    private ArrayList<SimInfoRecord> mSimInfoList;

    private ITelephony mTelephony;

    public SimInformation(Context context) {
        mContext = context;
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        mTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        mSimInfoList = (ArrayList<SimInfoRecord>) getSimInfo();
        int res = insertedSimSlot();
        Xlog.v(TAG, "insertedSimSlot(): " + res);
    }

    public void refresh() {
        mSimInfoList = (ArrayList<SimInfoRecord>) getSimInfo();
    }

    static int getStatusResource(int state) {
        switch (state) {
        case PhoneConstants.SIM_INDICATOR_RADIOOFF:
            return com.mediatek.internal.R.drawable.sim_radio_off;
        case PhoneConstants.SIM_INDICATOR_LOCKED:
            return com.mediatek.internal.R.drawable.sim_locked;
        case PhoneConstants.SIM_INDICATOR_INVALID:
            return com.mediatek.internal.R.drawable.sim_invalid;
        case PhoneConstants.SIM_INDICATOR_SEARCHING:
            return com.mediatek.internal.R.drawable.sim_searching;
        case PhoneConstants.SIM_INDICATOR_ROAMING:
            return com.mediatek.internal.R.drawable.sim_roaming;
        case PhoneConstants.SIM_INDICATOR_CONNECTED:
            return com.mediatek.internal.R.drawable.sim_connected;
        case PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
            return com.mediatek.internal.R.drawable.sim_roaming_connected;
        default:
            return -1;
        }
    }

    public int insertedSimSlot() {
        int result = NO_SIM_INSERTED;
        if (mSimInfoList == null || mSimInfoList.size() == 0) {
            result = NO_SIM_INSERTED;
        } else if (mSimInfoList.size() == 1) {
            SimInfoRecord simInfo = mSimInfoList.get(0);
            if (simInfo.mSimSlotId == PhoneConstants.GEMINI_SIM_1) {
                // Only SIM1 CDMA insearted
                result = ONLY_SLOT1_INSERTED;
            } else if (simInfo.mSimSlotId == PhoneConstants.GEMINI_SIM_2) {
                // Only SIM2 GSM insearted
                result = ONLY_SLOT2_INSERTED;
            }
        } else if (mSimInfoList.size() == 2) {
            result = TWO_SIM_INSERTED;
        }
        return result;
    }

    static int getSimColorResource(int color) {
        if ((color >= 0) && (color <= COLORNUM)) {
            return SimInfoManager.SimBackgroundDarkRes[color];
        } else {
            return -1;
        }
    }

    public boolean getSlotRomaingStatus(int slotId) {
        boolean bSlotRoaming = mTelephonyManagerEx.isNetworkRoaming(slotId);
        Xlog.d(TAG, "bSlotRoaming:" + bSlotRoaming);
        return bSlotRoaming;
    }

    public boolean isInternationalRoamingStatus() {
        boolean isRoaming = false;
        if (mSimInfoList != null && mSimInfoList.size() == DOUBLE_SIM_CARD) {
            // Two SIMS inserted
            isRoaming = mTelephonyManagerEx.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1);
            Xlog.d(TAG, "isInternationalRoamingStatus() statement1");
        } else if (mSimInfoList != null && mSimInfoList.size() == SINGLE_SIM_CARD) {
            // One SIM inserted
            SimInfoRecord simInfo = mSimInfoList.get(0);
            isRoaming = mTelephonyManagerEx.isNetworkRoaming(simInfo.mSimSlotId);
            Xlog.d(TAG, "isInternationalRoamingStatus() statement2");
        } else {
            // Error: no SIM inserted
            Xlog.d(TAG, "isInternationalRoamingStatus() statement3");
        }
        return isRoaming;
    }

    /**
     * Describe <code>getSlot1PhoneTypeGemini</code> method here.
     *
     * @return an <code>int</code> value
     * PhoneConstants.PHONE_TYPE_NONE
     * PhoneConstants.PHONE_TYPE_CDMA
     * PhoneConstants.PHONE_TYPE_GSM
     * PhoneConstants.PHONE_TYPE_SIP
     */
    public int getSlot1PhoneTypeGemini() {
        int insertedSim = insertedSimSlot();
        int result = PhoneConstants.PHONE_TYPE_NONE;
        boolean hasCdmaSim = false;
        if (mSimInfoList != null && mSimInfoList.size() == DOUBLE_SIM_CARD) {
            // Two SIMs inserted
            hasCdmaSim = true;
        } else if (mSimInfoList != null
                && mSimInfoList.size() == SINGLE_SIM_CARD) { // One SIM inserted
            SimInfoRecord simInfo = mSimInfoList.get(0);
            if (simInfo.mSimSlotId == PhoneConstants.GEMINI_SIM_1) {
                // Only SIM1 CDMA insearted
                hasCdmaSim = true;
            } else if (simInfo.mSimSlotId == PhoneConstants.GEMINI_SIM_2) {
                // Only SIM2 GSM insearted
                hasCdmaSim = false;
            }
        } else { // Error: no SIM inserted
            hasCdmaSim = false;
        }
        if (hasCdmaSim) {
            result = mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_1);
        }
        return result;
    }
    
    public List<SimInfoRecord> getSimInfo() {
        Xlog.d(TAG, "getSimInfo()");
        List<SimInfoRecord> siminfoList = new ArrayList<SimInfoRecord>();
        List<SimInfoRecord> simList = SimInfoManager.getAllSimInfoList(mContext);
        int simSlot = 0;
        SimInfoRecord tempSiminfo;
        siminfoList.clear();
        if (simList.size() == DOUBLE_SIM_CARD) {
            if (simList.get(0).mSimSlotId > simList.get(1).mSimSlotId) {
                Collections.swap(simList, 0, 1);
            }
            for (int i = 0; i < simList.size(); i++) {
                siminfoList.add(simList.get(i));
            }
            mIsSlot1Insert = true;
            mIsSlot2Insert = true;
        } else if (simList.size() == SINGLE_SIM_CARD) {
            siminfoList.add(simList.get(0));
            if (siminfoList.get(0).mSimSlotId == PhoneConstants.GEMINI_SIM_1) {
                mIsSlot1Insert = true;
            } else {
                mIsSlot2Insert = true;
            }
        }
        // for debug purpose to show the actual sim information
        for (int i = 0; i < siminfoList.size(); i++) {
            tempSiminfo = siminfoList.get(i);
            Xlog.i(TAG, "siminfo.mDisplayName = " + tempSiminfo.mDisplayName);
            Xlog.i(TAG, "siminfo.mSlot = " + tempSiminfo.mSimSlotId);
            Xlog.i(TAG, "siminfo.mColor = " + tempSiminfo.mColor);
            Xlog.i(TAG, "siminfo.mSimId = " + tempSiminfo.mSimInfoId);
        }
        return siminfoList;
    }
    public boolean isSlot1Inserted() {
        return mIsSlot1Insert;
    }
    public boolean isSlot2Inserted() {
        return mIsSlot2Insert;
    }

    private boolean isCdmaRoaming() {
        boolean res = false;
        String numeric = SystemProperties.get(
                TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
        Xlog.d(TAG, "numeric :" + numeric);
        if (numeric != null && !numeric.equals("-1") && numeric.length() >= 3) {
            String mcc = numeric.substring(0, 3);
            if (NATIVE_MCC_SIM1.equals(mcc) || NATIVE_MCC_SIM2.equals(mcc)) { // 46003,
                res = false;
            } else {
                res = true;
            }
        }
        return res;
    }

    public boolean isSupportNitz() {
        boolean bSupportNitz = true;
        String nitzStr = SystemProperties.get("gsm.nitz.time", "");
        if (nitzStr == null || nitzStr.equals("")) {
            // Do not support NITZ:
            bSupportNitz = false;
        }
        return bSupportNitz;
    }
}
