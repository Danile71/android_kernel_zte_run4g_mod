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
package com.mediatek.telephony;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.Rlog;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.telephony.LteWorldPhoneOp01;
import com.mediatek.telephony.WorldPhoneOp01;
import com.mediatek.telephony.WorldPhoneOm;

/**
 *@hide
 */
public class WorldPhoneWrapper implements IWorldPhone {
    private static int sOperatorSpec = -1;
    private static WorldPhoneOm sWorldPhoneOm = null;
    private static WorldPhoneOp01 sWorldPhoneOp01 = null;
    private static LteWorldPhoneOp01 sLteWorldPhoneOp01 = null;
    private static LteWorldPhoneOm sLteWorldPhoneOm = null;

    public WorldPhoneWrapper() {
        String optr = SystemProperties.get("ro.operator.optr");
        if (optr != null && optr.equals("OP01")) {
            if (PhoneFactory.isLteSupport()) {
                sOperatorSpec = POLICY_OP01_LTE;
            } else {
                sOperatorSpec = POLICY_OP01;
            }
        } else {
            if (PhoneFactory.isLteSupport()) {
                sOperatorSpec = POLICY_OM_LTE;
            } else {
                sOperatorSpec = POLICY_OM;
            }
        }
        logd("sOperatorSpec: " + sOperatorSpec);
        if (sOperatorSpec == POLICY_OP01) {
            sWorldPhoneOp01 = new WorldPhoneOp01();
        } else if (sOperatorSpec == POLICY_OP01_LTE) {
            sLteWorldPhoneOp01 = new LteWorldPhoneOp01();
        } else if (sOperatorSpec== POLICY_OM) {
            sWorldPhoneOm = new WorldPhoneOm();
        } else if (sOperatorSpec == POLICY_OM_LTE) {
            sLteWorldPhoneOm = new LteWorldPhoneOm();
        } else {
            logd("Unknown World Phone Spec");
        }
    }

    public void setNetworkSelectionMode(int mode, int modemType) {
        if (sOperatorSpec == POLICY_OP01) {
            sWorldPhoneOp01.setNetworkSelectionMode(mode, modemType);
        } else if (sOperatorSpec == POLICY_OP01_LTE) {
            sLteWorldPhoneOp01.setNetworkSelectionMode(mode, modemType);
        } else if (sOperatorSpec== POLICY_OM) {
            sWorldPhoneOm.setNetworkSelectionMode(mode, modemType);
        } else if (sOperatorSpec == POLICY_OM_LTE) {
            sLteWorldPhoneOm.setNetworkSelectionMode(mode, modemType);
        } else {
            logd("Unknown World Phone Spec");
        }
    }

    public int onNetworkModeChanged(int currentNwMode, int newNwMode) {
        if (sOperatorSpec == POLICY_OP01) {
            return sWorldPhoneOp01.onNetworkModeChanged(currentNwMode, newNwMode);
        } else if (sOperatorSpec == POLICY_OP01_LTE) {
            return sLteWorldPhoneOp01.onNetworkModeChanged(currentNwMode, newNwMode);
        } else if (sOperatorSpec== POLICY_OM) {
            return sWorldPhoneOm.onNetworkModeChanged(currentNwMode, newNwMode);
        } else if (sOperatorSpec == POLICY_OM_LTE) {
            sLteWorldPhoneOm.onNetworkModeChanged(currentNwMode, newNwMode);
        } else {
            logd("Unknown World Phone Spec");
        }
        return 0;
    }

    public void disposeWorldPhone() {
        if (sOperatorSpec == POLICY_OP01) {
            sWorldPhoneOp01.disposeWorldPhone();
        } else if (sOperatorSpec == POLICY_OP01_LTE) {
            sLteWorldPhoneOp01.disposeWorldPhone();
        } else if (sOperatorSpec== POLICY_OM) {
            sWorldPhoneOm.disposeWorldPhone();
        } else if (sOperatorSpec == POLICY_OM_LTE) {
            sLteWorldPhoneOm.disposeWorldPhone();
        } else {
            logd("Unknown World Phone Spec");
        }
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[WPO_WRAPPER]" + msg);
    }
}
