/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.voicecommand.business;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class BootCompletedReceiver extends BroadcastReceiver {
    private Context mContext;
    private Handler mHandler;
    private VoiceContactsObserver mVoiceContactsObserver;
    private VoiceWakeupMode mVoiceWakeupModeObserver;
    private VoiceWakeupCmdStatus mVoiceWakeupCmdStatusObserver;

    public BootCompletedReceiver(Context context, Handler handler) {
        mContext = context;
        mHandler = handler;
    }

    public void onReceive(Context arg0, Intent arg1) {
        // TODO Auto-generated method stub
        Log.i(VoiceCommandManagerStub.TAG, "BootCompletedReceiver onReceive");
        registerObserver();
    }

    public void registerObserver() {
        Message msg = mHandler.obtainMessage();
        msg.what = VoiceCommandBusiness.ACTION_MAIN_VOICE_BROADCAST_BOOT_COMPLETED;
        mHandler.sendMessage(msg);

        mVoiceContactsObserver = new VoiceContactsObserver(mContext, mHandler);
        if (FeatureOption.MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            mContext.getContentResolver().registerContentObserver(
                    VoiceContactsObserver.CONTACTS_URI, true,
                    mVoiceContactsObserver);
        }

        mVoiceWakeupModeObserver = new VoiceWakeupMode(mContext, mHandler);
        mVoiceWakeupCmdStatusObserver = new VoiceWakeupCmdStatus(mContext, mHandler);
        if (FeatureOption.MTK_VOW_SUPPORT) {
            mContext.getContentResolver().registerContentObserver(
                    Settings.System.getUriFor(VoiceWakeup.VOICE_WAKEUP_MODE), true,
                    mVoiceWakeupModeObserver);
            mContext.getContentResolver().registerContentObserver(
                    Settings.System.getUriFor(VoiceWakeup.VOICE_WAKEUP_COMMAND_STATUS),
                    true, mVoiceWakeupCmdStatusObserver);
        }
    }

    /**
     * Release all variables when service is destroy.
     */
    public void handleDataRelease() {
        if (FeatureOption.MTK_VOICE_CONTACT_SEARCH_SUPPORT
                && mVoiceContactsObserver != null) {
            mContext.getContentResolver().unregisterContentObserver(
                    mVoiceContactsObserver);
        }

        if (FeatureOption.MTK_VOW_SUPPORT && mVoiceWakeupModeObserver != null
                && mVoiceWakeupCmdStatusObserver != null) {
            mContext.getContentResolver().unregisterContentObserver(
                    mVoiceWakeupModeObserver);
            mContext.getContentResolver().unregisterContentObserver(
                    mVoiceWakeupCmdStatusObserver);
        }
    }

    /**
     * Send voice wakeup init message to MD32.
     */
    public void sendWakeupInitMessage() {
        if (mVoiceWakeupModeObserver != null) {
            Handler handler = mVoiceWakeupModeObserver.getVoiceWakeupModeHandler();
            if (handler != null) {
                handler.sendEmptyMessage(VoiceWakeup.MSG_GET_WAKEUP_INIT);
            }
        }
    }
}
