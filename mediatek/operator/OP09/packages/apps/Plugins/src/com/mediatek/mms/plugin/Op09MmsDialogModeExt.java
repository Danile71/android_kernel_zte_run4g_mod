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
import android.graphics.drawable.Drawable;
import android.widget.TextView;

import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsDialogModeExt;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;

public class Op09MmsDialogModeExt extends DefaultMmsDialogModeExt {
    private static final String TAG = "Mms/OP09MmsDialogModeExt";

    public Op09MmsDialogModeExt(Context context) {
        super(context);
    }

    public String getNotificationContentString(String from, String subject, String msgSizeTxt, String expireTxt) {
        return from + "\n" + subject + "\n" + msgSizeTxt + "\n" + expireTxt;
    }

    public void setSimTypeDrawable(Context context, long simId, TextView textView) {
        Drawable simTypeDraw = null;
        SimInfoManager.SimInfoRecord simInfo = SimInfoManager.getSimInfoById(context, simId);
        int slotId = -1;
        if (simInfo != null) {
            slotId = simInfo.mSimSlotId;
            simTypeDraw = getResources().getDrawable(simInfo.mSimBackgroundLightSmallRes);
        } else {
            simTypeDraw = getResources().getDrawable(R.drawable.sim_light_not_activated);
        }
        MmsLog.d(TAG, "showSimType for DialogModel; slotId:" + slotId);
        if (textView != null) {
            textView.setText("");
            textView.setBackgroundDrawable(simTypeDraw);
        }
    }

}
