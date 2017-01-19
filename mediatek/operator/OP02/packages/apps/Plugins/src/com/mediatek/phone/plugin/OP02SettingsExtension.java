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
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.OperatorInfo;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.phone.ext.SettingsExtension;

import java.util.ArrayList;
import java.util.List;

public class OP02SettingsExtension extends SettingsExtension {

    private static final String TAG = "OP02SettingsExtension";

    private static final String CU_NUMERIC_1 = "46001";
    private static final String CU_NUMERIC_2 = "46009";
    private static final String CT_4G_NUMERIC = "46011";
    ///M: GSM_R is for CMCC (railway data simcard),CU need forbidden it
    private static final String GSM_R = "46020";

    private Context mOp02Context;
    private ITelephony mITelephony;

    public OP02SettingsExtension(Context context) {
        mOp02Context = context;
        mITelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        Log.d(TAG, "OP02SettingsExtension");
    }

    /**
     * If user insert CU card, set CMCC/CT operator networks as forbidden
     */
    @Override
    public List<OperatorInfo> customizeNetworkList(List<OperatorInfo> operatorInfoList, int slotId) {
        if (operatorInfoList == null || operatorInfoList.size() == 0) {
            Log.d(TAG, "customizeNetworkList return null list");
            return operatorInfoList;
        }
        String iccNumeric;
        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            iccNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC);
        } else {
            iccNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2);
        }
        Log.d(TAG, "customizeNetworkList slotId" + slotId
                + ", iccNumeric=" + iccNumeric);
        if (iccNumeric == null || (!iccNumeric.equals(CU_NUMERIC_1) && !iccNumeric.equals(CU_NUMERIC_2))) {
            return operatorInfoList;
        }
        String cmccOpName = mOp02Context.getResources().getString(com.mediatek.R.string.oper_long_46000);
        String ctOpName = mOp02Context.getResources().getString(com.mediatek.R.string.oper_long_46003);
        List<OperatorInfo> newOPInfoList = new ArrayList<OperatorInfo>(operatorInfoList.size());
        for (OperatorInfo operatorInfo : operatorInfoList) {
            Log.d(TAG, "customizeNetworkList operatorInfo=" + operatorInfo.toString());
            // CT new 4G network is under testing, its network alphalong name is "46011 4G" and does not
            // follow 3gpp spec to return forbidden when CU card performs network search.
            // We need to check this network and set it as forbidden here
            if (operatorInfo.getOperatorAlphaLong().contains(cmccOpName)
                    || operatorInfo.getOperatorAlphaLong().contains(ctOpName)
                    || operatorInfo.getOperatorAlphaLong().contains(CT_4G_NUMERIC)
                    || operatorInfo.getOperatorAlphaLong().contains(GSM_R)) {
                newOPInfoList.add(new OperatorInfo(operatorInfo.getOperatorAlphaLong(),
                        operatorInfo.getOperatorAlphaShort(), operatorInfo.getOperatorNumeric(), "forbidden"));
            } else {
                newOPInfoList.add(operatorInfo);
            }
        }
        return newOPInfoList;
    }
}
