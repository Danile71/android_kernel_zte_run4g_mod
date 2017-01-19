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

import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsFeatureManagerExt;

public class Op09MmsFeatureManagerExt extends DefaultMmsFeatureManagerExt {
    private static final String TAG = "Mms/Op09MmsFeatureManagerExt";

    public Op09MmsFeatureManagerExt(Context context) {
        super(context);
    }

    @Override
    public boolean isFeatureEnabled(int featureNameIndex) {
        switch (featureNameIndex) {
            case STRING_REPLACE_MANAGEMENT:
                return true;
            case SHOW_DUAL_TIME_FOR_MESSAGE_ITEM:
                return true;
            case MMS_TAB_SETTING:
                return true;
            case MMS_DUAL_SEND_BUTTON:
                return true;
            case MMS_VCARD_PREVIEW:
                return true;
            case CHANGE_LENGTH_REQUIRED_MMS_TO_SMS:
                return true;
            case MASS_TEXT_MSG:
                return true;
            case MMS_MULTI_COMPOSE:
                return true;
            case MMS_CANCEL_DOWNLOAD:
                return true;
            case SPLICE_MISSED_SMS:
                return true;
            case CLASS_ZERO_NEW_MODEL_SHOW_LATEST:
                return true;
            case MMS_LOW_MEMORY:
                return true;
            case WAKE_UP_SCREEN_WHEN_RECEIVE_MSG:
                return true;
            case ADVANCE_SEARCH_VIEW:
                return true;
            case DELIEVEEY_REPORT_IN_ROAMING:
                return true;
            case MMS_NUMBER_LOCATION:
                return true;
            case FORMAT_DATE_AND_TIME:
                return true;
            case FORMAT_NOTIFICATION_CONTENT:
                return true;
            case READ_SMS_FROM_DUAL_MODEL_UIM:
                return true;
            case SHOW_DATE_MANAGEMENT:
                return true;
            case MORE_STRICT_VALIDATION_FOR_SMS_ADDRESS:
                return true;
            case SHOW_PREVIEW_FOR_RECIPIENT:
                return true;
            case SHOW_DIALOG_FOR_NEW_SI_MSG:
                return true;
            case MMS_TRANSACTION_FAILED_NOTIFY:
                return true;
            case MMS_RETRY_SCHEDULER:
                return true;
            case SMS_PRIORITY:
                return true;
            case MMS_CC_RECIPIENTS:
                return true;
            case MMS_PLAY_FILING_TURNPAGE:
                return true;
            default:
                return super.isFeatureEnabled(featureNameIndex);
        }
    }
}
