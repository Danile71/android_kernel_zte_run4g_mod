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
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.RemoteException;
import android.os.ServiceManager;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.op09.plugin.R;



import java.util.Iterator;
import java.util.Map.Entry;

import com.mediatek.mms.ext.DefaultMmsManageSimMessageExt;

public class Op09MmsManageSimMessageExt extends DefaultMmsManageSimMessageExt {

    private static final String TAG = "OP09/OP09MmsManageSimMessageExt";
//    private static final int i=1;
    
    public Op09MmsManageSimMessageExt(Context context) {
        super(context);
    }

    private boolean isUnactivatedMessage(int index) {
        int temp = (index & (0x01 << 10));
        return (temp == (0x01 << 10));
    }

    public boolean isInternationalCard(int slotId) {
        // TODO Auto-generated method stub
        ITelephonyEx telephony = ITelephonyEx.Stub.asInterface(ServiceManager.checkService("phoneEx"));
        try {
            if (telephony != null) {
                int cardType = telephony.getInternationalCardType(slotId);
                if (cardType == InternationalRoamingConstants.CARD_TYPE_DUAL_MODE) {
                    return true;
                }
            }
        } catch (RemoteException e) {
            MmsLog.e(TAG, "Error to getInternationalCardType");
        }
        return false;
    }

    public boolean canBeOperated(Cursor cursor) {
        if (cursor == null) {
            return false;
        }
        try {
            int index = cursor.getInt(cursor.getColumnIndex("index_on_icc"));
            MmsLog.d(TAG, "canBeOperated: index:" + index);
            return !isUnactivatedMessage(index);
        } catch (SQLiteException e) {
            MmsLog.e(TAG, "error to canBeOperated");
        }
        return true;
    }

    public boolean hasIncludeUnoperatedMessage(Iterator<Entry<String, Boolean>> it) {
        // TODO Auto-generated method stub
        if (it == null) {
            return false;
        }
        while (it.hasNext()) {
            Entry<String, Boolean> entry = (Entry<String, Boolean>) it.next();
            if (entry.getValue()) {
                if (isUnactivatedMessage(Integer.parseInt(entry.getKey()))) {
                    return true;
                }
            }
        }
        return false;
    }

    public String[] filterUnoperatedMsgs(String[] simMsgIndex) {
        // TODO Auto-generated method stub
        if (simMsgIndex == null || simMsgIndex.length < 1) {
            return simMsgIndex;
        }
        String[] temp = new String[simMsgIndex.length];
        int index = 0;
        for (String msgIndex : simMsgIndex) {
            if (!isUnactivatedMessage(Integer.parseInt(msgIndex))) {
                temp[index] = msgIndex;
                index++;
            }
        }
        return temp;
    }

    public Uri getAllContentUriForInternationalCard(int slotId) {
        // TODO Auto-generated method stub
        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            return Uri.parse("content://sms/icc_international");
        } else if (slotId == PhoneConstants.GEMINI_SIM_2) {
            return Uri.parse("content://sms/icc2_international");
        }
        return null;
    }

}
