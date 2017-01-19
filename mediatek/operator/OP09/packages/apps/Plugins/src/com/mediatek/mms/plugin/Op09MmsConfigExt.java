/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.content.Context;

import java.util.ArrayList;

import com.google.android.mms.pdu.PduHeaders;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsConfigExt;

public class Op09MmsConfigExt extends DefaultMmsConfigExt {

    private static final String TAG = "OP09MmsConfigExt";
    private static int sSmsToMmsTextThreshold = 11;
    private static final int[] OP09_DEFAULT_RETRY_SCHEME = {
        0, 5 * 60 * 1000, 5 * 60 * 1000, 5 * 60 * 1000};
    private static final int[] OP09_SEND_RETRY_SCHEME = {
        0, 60 * 1000, 60 * 1000, 60 * 1000};
    private static int sMaxTextLimit = 5000;

    public int getSmsToMmsTextThreshold() {
        return sSmsToMmsTextThreshold;
    }

    public int[] getMmsRetryScheme(int messageType) {
        if (messageType == PduHeaders.MESSAGE_TYPE_SEND_REQ) {
            return OP09_SEND_RETRY_SCHEME.clone();
        }
        return OP09_DEFAULT_RETRY_SCHEME.clone();
    }

    public boolean isSupportCBMessage(Context context, int simId) {
        return !MessageUtils.isCDMAType(context, simId);
    }

    public boolean isAllowRetryForPermanentFail() {
        MmsLog.d(TAG, "isAllowRetryForPermanentFail: true");
        return true;
    }

    public boolean isAllowDRWhenRoaming(Context context, int slotId) {
        MmsLog.d(TAG, "isAllowDRWhenRoaming() slotId = " + slotId);

        if (!MessageUtils.isCDMAType(context, slotId)) {
            return true;
        }

        return !MessageUtils.isInternationalRoamingStatus(context, slotId);
    }

    public boolean isSupportTabSetting() {
        return true;
    }

    public int getMaxTextLimit() {
        MmsLog.d(TAG, "get MaxTextLimit: " + sMaxTextLimit);
        return sMaxTextLimit;
    }

    public void setExtendedAudioType(ArrayList<String> audioType) {
        if (audioType == null) {
            return;
        }
        audioType.add("audio/sp-midi");
    }
}
