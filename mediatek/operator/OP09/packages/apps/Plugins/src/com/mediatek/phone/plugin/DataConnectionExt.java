/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.phone.plugin;

import android.content.Context;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Telephony.SIMInfo;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.phone.ext.DefaultDataConnection;
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
     * @param name
     *            String name of sim card
     * @param color
     *            int color of sim card
     * @param simID
     *            long sim id of sim card
     */
    public SimItem(String name, int color, long simId) {
        mName = name;
        mColor = color;
        mIsSim = false;
        mSimId = simId;

    }

    /**
     * 
     * @param siminfo
     *            SIMInfo
     */
    public SimItem(SIMInfo siminfo) {
        mIsSim = true;
        mName = siminfo.mDisplayName;
        mColor = siminfo.mColor;
        mSlot = siminfo.mSlot;
        mSimId = siminfo.mSimId;
        mNumber = siminfo.mNumber;
        mDispalyNumberFormat = siminfo.mDispalyNumberFormat;
    }
}



public class DataConnectionExt extends DefaultDataConnection {

    private static final String TAG = "DataConnectionExt";

    private static final int SINGLE_SIM_CARD = 1;
    private static final int DOUBLE_SIM_CARD = 2;
    private static final String NATIVE_MCC_SIM1 = "460"; // 46003
    private static final String NATIVE_MCC_SIM2 = "455"; // 45502

    private boolean mIsSlot1Insert = false;
    private boolean mIsSlot2Insert = false;

    private TelephonyManagerEx mTelephonyManagerEx;

    public boolean bShowDataConn(Context context, boolean bRoaming, int simSlot) {
       
        Xlog.d(TAG, "in bShowDataConn");
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        boolean isRoaming = isInternationalRoamingStatus(context);
        Xlog.d(TAG, "isRoaming:" + isRoaming);
        if (isRoaming) { // International Roaming
            int selectedMainSimSlot = Settings.System.getInt(
                    context.getContentResolver(), Settings.System.CT_MAIN_SIM_SELECTION, -1);
            if (simSlot == selectedMainSimSlot) {
                Xlog.d(TAG,"bShowDataConn Roaming true: bRoaming=" 
                        + bRoaming + " | selectedMainSimSlot=" + selectedMainSimSlot + " simSlot=" + simSlot);
                return true;
            } else {
                Xlog.d(TAG,"bShowDataConn Roaming false: bRoaming=" 
                        + bRoaming + " | selectedMainSimSlot=" + selectedMainSimSlot + " simSlot=" + simSlot);
                return false;
            }
        } else { // Native
            if (simSlot == PhoneConstants.GEMINI_SIM_1) { // Slot1: add
                Xlog.d(TAG,"bShowDataConn Native true: bRoaming=" + bRoaming + " | simSlot=" + simSlot);
                return true;
            } else { // Slot2: don't add
                Xlog.d(TAG,"bShowDataConn Native false: bRoaming=" + bRoaming + " | simSlot=" + simSlot);
                return false;
            }
        }
    }

    public int getGprsRadioInPreferenceProperty(int commonPosition, int simSlot) {

         if (simSlot == PhoneConstants.GEMINI_SIM_1 || simSlot == PhoneConstants.GEMINI_SIM_2) {
             Xlog.i(TAG, "TEST:mGprsSimSetting.setInitValue(0)");
             //mGprsSimSetting.setInitValue(0);
             return 0;
         } else {
             Xlog.i(TAG, "TEST:mGprsSimSetting.setInitValue(1)");
             //mGprsSimSetting.setInitValue(1);
             return 1;
         }
    }

    private boolean isInternationalRoamingStatus(Context context) {
        Xlog.d(TAG, "in isInternationalRoamingStatus");
        boolean isRoaming = false;
        ArrayList<SIMInfo> simInfoList = (ArrayList<SIMInfo>) getSimInfo(context);
        if (simInfoList != null && simInfoList.size() == DOUBLE_SIM_CARD) { // Two SIMs inserted
            Xlog.i(TAG, "Two sims");
            isRoaming = isCdmaRoaming();
        } else if (simInfoList != null && simInfoList.size() == SINGLE_SIM_CARD) { // One SIM inserted
            Xlog.i(TAG, "One sim");
            SIMInfo simInfo = simInfoList.get(0);
            if (simInfo.mSlot == PhoneConstants.GEMINI_SIM_1) { // Only SIM1 CDMA insearted
                Xlog.i(TAG, "One sim1");
                isRoaming = isCdmaRoaming();

            } else if (simInfo.mSlot == PhoneConstants.GEMINI_SIM_2) { // Only SIM2 GSM insearted
                Xlog.i(TAG, "One sim2");
                isRoaming = mTelephonyManagerEx.isNetworkRoaming(simInfo.mSlot);
            }
        } else { // Error: no SIM inserted
            Xlog.i(TAG, "Error: no SIM inserted");
        }
        return isRoaming;
    }

    

    public List<SIMInfo> getSimInfo(Context context) {
        Xlog.d(TAG, "getSimInfo()");
        List<SIMInfo> siminfoList = new ArrayList<SIMInfo>();
        List<SIMInfo> simList = SIMInfo.getInsertedSIMList(context);
        int simSlot = 0;
        SIMInfo tempSiminfo;
        siminfoList.clear();
        if (simList.size() == DOUBLE_SIM_CARD) {
            if (simList.get(0).mSlot > simList.get(1).mSlot) {
                Collections.swap(simList, 0, 1);
            }
            for (int i = 0; i < simList.size(); i++) {
                siminfoList.add(simList.get(i));
            }
            mIsSlot1Insert = true;
            mIsSlot2Insert = true;
        } else if (simList.size() == SINGLE_SIM_CARD) {
            siminfoList.add(simList.get(0));
            if (siminfoList.get(0).mSlot == PhoneConstants.GEMINI_SIM_1) {
                mIsSlot1Insert = true;
            } else {
                mIsSlot2Insert = true;
            }
        }
        // for debug purpose to show the actual sim information
        for (int i = 0; i < siminfoList.size(); i++) {
            tempSiminfo = siminfoList.get(i);
        }
        return siminfoList;
    }
    private boolean isCdmaRoaming() {
        boolean res = false;

        Xlog.i(TAG, "in isCdmaRoaming");
        String numeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
        Xlog.i(TAG, "isCdmaRoaming numeric :" + numeric);
        if (numeric != null && !numeric.equals("-1") && numeric.length() >= 3) {
            String mcc = numeric.substring(0,3);
            Xlog.i(TAG, "mcc=" + mcc);
            if (NATIVE_MCC_SIM1.equals(mcc) || NATIVE_MCC_SIM2.equals(mcc)) {
                res = false;
            } else {
                res = true;
            }
        }
        return res;
    }

}
