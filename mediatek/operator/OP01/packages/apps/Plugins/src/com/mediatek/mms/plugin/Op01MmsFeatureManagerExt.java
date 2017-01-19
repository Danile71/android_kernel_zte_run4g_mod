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
import android.content.ContextWrapper;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.mms.ext.DefaultMmsFeatureManagerExt;
import com.mediatek.mms.ext.IMmsFeatureManagerExt;
import com.mediatek.xlog.Xlog;

public class Op01MmsFeatureManagerExt extends DefaultMmsFeatureManagerExt {
    private static final String TAG = "Mms/Op01MmsFeatureManagerExt";
    
    public Op01MmsFeatureManagerExt(Context context) {
        super(context);
    }

    public boolean isFeatureEnabled(int featureNameIndex){
        switch(featureNameIndex){
            // Op01 case:
            case MMS_ATTACH_ENHANCE:
            case SMS_APPEND_SENDER:
            case MMS_ENABLE_REPORT_ALLOWED:
            case MMS_ENABLE_FOLDER_MODE:
            case DISPLAY_STORAGE_STATUS:
            case MMS_RETRY_FOR_PERMANENTFAIL:
            case MMS_RETAIN_RETRY_INDEX_WHEN_INCALL:
            case SUPPORT_AUTO_SELECT_SIMID:
            case SUPPORT_ASYNC_UPDATE_WALLPAPER:
            case SUPPORT_MESSAGING_NOTIFICATION_PROXY:
            case MMS_ENABLE_SYNC_START_PDP:
            case MMS_ENABLE_ADD_TOP_BOTTOM_SLIDE:
                return true;
            case EXIT_COMPOSER_AFTER_FORWARD:
            case SMS_ENABLE_CONCATENATE_LONG_SIM_SMS:
                return false;

            case MMS_ENABLE_GEMINI_MULTI_TRANSACTION:
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    return true;
                } else {
                    return false;
                }

            default:
                return super.isFeatureEnabled(featureNameIndex);
        }
    }
}