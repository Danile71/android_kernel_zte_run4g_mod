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

import java.util.ArrayList;

import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.Telephony.Sms;

import com.mediatek.mms.ext.DefaultMmsSmsMessageSenderExt;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.telephony.SmsManagerEx;

public class Op09MmsSmsMessageSenderExt extends DefaultMmsSmsMessageSenderExt {
    private static final String TAG = "Mms/OP09MmsSmsMessageSenderExt";

    private static final String PREF_SMS_PRIORITY_KEY = "pref_key_sms_priority";
    private static final String BUNDLE_SMS_PRIORITY_KEY = "priority";

    private static final String SMS_PRIORITY_NORMAL = "Normal";
    private static final String SMS_PRIORITY_INTERACTIVE = "Interactive";
    private static final String SMS_PRIORITY_URGENT = "Urgent";
    private static final String SMS_PRIORITY_EMERGENCY = "Emergency";

    public Op09MmsSmsMessageSenderExt(Context context) {
        super(context);
    }

    public Uri addMessageToUri(ContentResolver resolver, Uri uri, String address,
            String body, String subject, Long date, boolean read, boolean deliveryReport,
            long threadId, int simId, long ipmsgId) {
        MmsLog.d(TAG, "uri:" + uri + " ipmsgId:" + ipmsgId);
        ContentValues values = new ContentValues(10);

        values.put(Sms.ADDRESS, address);
        values.put(Sms.READ, read ? Integer.valueOf(1) : Integer.valueOf(0));
        values.put(Sms.SUBJECT, subject);
        values.put(Sms.BODY, body);
        values.put(Sms.SEEN, read ? Integer.valueOf(1) : Integer.valueOf(0));

        if (date != null) {
            values.put(Sms.DATE, date);
        }

        if (deliveryReport) {
            values.put(Sms.STATUS, Sms.STATUS_PENDING);
        }
        if (threadId != -1L) {
            values.put(Sms.THREAD_ID, threadId);
        }

        if (simId != -1) {
            values.put(Sms.SIM_ID, simId);
        }

        values.put(Sms.IPMSG_ID, ipmsgId);

        return resolver.insert(uri, values);
    }

    public void sendSMSWithPriority(Context context, String destAddr, String scAddr, ArrayList<String> parts,
            int slotId, ArrayList<PendingIntent> sentIntents, ArrayList<PendingIntent> deliveryIntents) {
        SharedPreferences spref = PreferenceManager.getDefaultSharedPreferences(context);
        String priority = spref.getString(PREF_SMS_PRIORITY_KEY, SMS_PRIORITY_NORMAL);
        MmsLog.d(TAG, "sendSMSWithPriority(), priority: " + priority);
        Bundle extraParams = new Bundle();

        if (priority.equals(SMS_PRIORITY_NORMAL)) {
            extraParams.putInt(BUNDLE_SMS_PRIORITY_KEY, 0);
        } else if (priority.equals(SMS_PRIORITY_INTERACTIVE)) {
            extraParams.putInt(BUNDLE_SMS_PRIORITY_KEY, 1);
        } else if (priority.equals(SMS_PRIORITY_URGENT)) {
            extraParams.putInt(BUNDLE_SMS_PRIORITY_KEY, 2);
        } else if (priority.equals(SMS_PRIORITY_EMERGENCY)) {
            extraParams.putInt(BUNDLE_SMS_PRIORITY_KEY, 3);
        }

        SmsManagerEx.getDefault().sendMultipartTextMessageWithExtraParams(
                destAddr, scAddr, parts, extraParams, sentIntents, deliveryIntents, slotId);
    }

}
