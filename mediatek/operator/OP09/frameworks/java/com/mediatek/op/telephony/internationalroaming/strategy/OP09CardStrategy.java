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

package com.mediatek.op.telephony.internationalroaming.strategy;

import android.content.Context;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.SimInfo;
import android.text.TextUtils;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.uicc.SpnOverride;

import com.mediatek.common.telephony.internationalroaming.IInternationalRoamingController;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.telephony.SimInfoManager;

public class OP09CardStrategy extends DefaultCardStrategy {
    private static final String TAG = "[OP09CardStrategy]";

    private static final String CHINA_TELECOME_MAINLAND_PLMN = "46003";
    private static final String CHINA_TELECOME_MACCO_PLMN = "45502";
    private static final String CHINA_TELECOME_FAKE_PLMN = "46099";
    private static final int PLMN_STRING_LENGTH = 5;

    public OP09CardStrategy(IInternationalRoamingController controller, Context context,
            Phone dualModePhone, Phone gsmPhone) {
        super(controller, context, dualModePhone, gsmPhone);
    }

    @Override
    public int parseCardType(String cdmaImsi, String gsmImsi) {
        logd("parseCardType: cdmaImsi = " + cdmaImsi + ", gsmImsi = " + gsmImsi);
        
        if (TextUtils.isEmpty(cdmaImsi) && !TextUtils.isEmpty(gsmImsi)) {
            return InternationalRoamingConstants.CARD_TYPE_SINGLE_GSM;
        } else if (!TextUtils.isEmpty(cdmaImsi) && TextUtils.isEmpty(gsmImsi)) {
            return InternationalRoamingConstants.CARD_TYPE_SINGLE_CDMA;
        } else if (!TextUtils.isEmpty(cdmaImsi) && !TextUtils.isEmpty(gsmImsi)) {
            if (isChineseCdmaCard(cdmaImsi)) {
                String gsmMccMnc = gsmImsi.substring(0, PLMN_STRING_LENGTH);
                logd("parseCardType: gsmMccMnc = " + gsmMccMnc);
                if (gsmMccMnc.equals(CHINA_TELECOME_FAKE_PLMN) || gsmImsi.equals(cdmaImsi)
                        || !SpnOverride.getInstance().containsCarrier(gsmMccMnc)) {
                    return InternationalRoamingConstants.CARD_TYPE_SINGLE_CDMA;
                }
            }
            return InternationalRoamingConstants.CARD_TYPE_DUAL_MODE;
        } else {
            return InternationalRoamingConstants.CARD_TYPE_UNKNOWN;
        }
    }

    @Override
    public void onSimImsiLoaded(int dualSimId, String cdmaImsi, String gsmImsi) {
        logd("onSimImsiLoaded: dualSimId = " + dualSimId + ", cdmaImsi = " + cdmaImsi
                + ", gsmImsi = " + gsmImsi);
        
        setOperatorForNewSim(dualSimId, cdmaImsi);
    }

    /**
     * Set the operator name of SIMInfo, this can be used for application to
     * distiguish whether the card provider is OP09 or others.
     * 
     * @param dualSimId
     * @param cdmaImsi
     */
    private void setOperatorForNewSim(int dualSimId, String cdmaImsi) {
        SIMInfo simInfo = SIMInfo.getSIMInfoBySlot(mContext, dualSimId);
        logd("setOperatorForNewSim: dualSimId = " + dualSimId + ", cdmaImsi = " + cdmaImsi
                + ", simInfo = " + simInfo);
        
        if (simInfo != null) {
            if (isChineseCdmaCard(cdmaImsi)) {
                SimInfoManager.setOperatorById(mContext, SimInfo.OPERATOR_OP09, simInfo.mSimId);
            } else {
                SimInfoManager.setOperatorById(mContext, SimInfo.OPERATOR_OTHERS, simInfo.mSimId);
            }
        }
    }

    /**
     * Whether the card is Chinese CDMA card.
     * 
     * @param cdmaImsi
     * @return True if the card is Chinese CDMA card.
     */
    private boolean isChineseCdmaCard(String cdmaImsi) {
        logd("isChineseCdmaCard, cdmaImsi = " + cdmaImsi);
        boolean chineseCdmaCard = !TextUtils.isEmpty(cdmaImsi)
                && (cdmaImsi.startsWith(CHINA_TELECOME_MAINLAND_PLMN) || cdmaImsi.startsWith(CHINA_TELECOME_MACCO_PLMN));
        return chineseCdmaCard;
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }
}
