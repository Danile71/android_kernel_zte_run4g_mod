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

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.R.integer;
import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class VoiceWakeupMode extends ContentObserver {
    private Context mContext;
    private Handler mainHandler;
    private HandlerThread mHandlerThread;
    private Handler mVoiceWakeupModeHandler;
    private ConfigurationManager mVoiceConfigMgr;

    public VoiceWakeupMode(Context context, Handler handler) {
        super(handler);
        mContext = context;
        mainHandler = handler;
        mVoiceConfigMgr = ConfigurationManager.getInstance(mContext);
        mHandlerThread = new HandlerThread("VoiceWakeupModeThread");
        mHandlerThread.start();
        mVoiceWakeupModeHandler = new VoiceWakeupModeHandler(mHandlerThread.getLooper());
        mVoiceWakeupModeHandler.sendEmptyMessage(VoiceWakeup.MSG_GET_WAKEUP_INIT);
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        Log.d(VoiceCommandManagerStub.TAG, "onChange uri : " + uri);
        // TODO Auto-generated method stub
        if (mVoiceWakeupModeHandler
                .hasMessages(VoiceWakeup.MSG_GET_WAKEUP_MODE)) {
            mVoiceWakeupModeHandler
                    .removeMessages(VoiceWakeup.MSG_GET_WAKEUP_MODE);
        }
        mVoiceWakeupModeHandler
                .sendEmptyMessage(VoiceWakeup.MSG_GET_WAKEUP_MODE);
    }

    private class VoiceWakeupModeHandler extends Handler {
        public VoiceWakeupModeHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            switch (msg.what) {
            case VoiceWakeup.MSG_GET_WAKEUP_INIT: {
                sendInitToMainHandler();
            }
                break;
            case VoiceWakeup.MSG_GET_WAKEUP_MODE: {
                sendModeToMainHandler();
            }
                break;
            default:
                break;
            }
        }
    }

    private void sendInitToMainHandler() {
        if (!FeatureOption.MTK_VOW_SUPPORT) {
            Log.i(VoiceCommandManagerStub.TAG,
                    "Voice Wakeup feature is off, can not send init to mainHandler");
            return;
        }

        if (mainHandler
                .hasMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT)) {
            mainHandler
                    .removeMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT);
        }
        // Query database after remove the wake up msg of main handler
        int mode = VoiceWakeup.getWakeupMode(mContext);
        int cmdStatus = VoiceWakeup.getWakeupCmdStatus(mContext);
        String patternPath = mVoiceConfigMgr
                .getVoiceRecognitionPatternFilePath(mode);
        int[] commandIds = getCommandIdList(getDirectory(patternPath), ".dat");
        Log.i(VoiceCommandManagerStub.TAG, "First send init commandIds: "
                + Arrays.toString(commandIds));

        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT;
        Bundle bundle = DataPackage.packageSendInfo(mode, cmdStatus, commandIds);
        message.mExtraData = bundle;

        Message msg = mainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        msg.obj = message;
        mainHandler.sendMessage(msg);
    }

    private void sendModeToMainHandler() {
        if (mainHandler
                .hasMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE)) {
            mainHandler
                    .removeMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE);
        }
        // Query database after remove the wake up msg of main handler
        int mode = VoiceWakeup.getWakeupMode(mContext);
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE;
        Bundle bundle = DataPackage.packageSendInfo(mode);
        message.mExtraData = bundle;

        Message msg = mainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        msg.obj = message;
        mainHandler.sendMessage(msg);
    }

    private List<String> getDirectory(String path) {
        List<String> fileNameList = new ArrayList<String>();
        if (path == null) {
            return fileNameList;
        }
        File file = new File(path);
        File flist[] = file.listFiles();
        if (flist == null || flist.length == 0) {
            return fileNameList;
        }
        for (File f : flist) {
            if (f.isDirectory()) {
                getDirectory(f.getPath());
            } else {
                String name = f.getName();
                fileNameList.add(name);
            }
        }
        return fileNameList;
    }

    private int[] getCommandIdList(List<String> fileNameList, String end) {
        List<Integer> commandIdList = new ArrayList<Integer>();
        for (int i = 0; i < fileNameList.size(); i++) {
            String path = fileNameList.get(i);
            if (path == null) {
                continue;
            } else {
                String subPath = path.substring(0, path.indexOf(end));
                try {
                    Integer commandId = Integer.valueOf(subPath);
                    commandIdList.add(commandId);
                } catch (NumberFormatException e) {
                    Log.e(VoiceCommandManagerStub.TAG,
                            "getCommandIdList error " + e.toString());
                }
            }
        }
        int[] commandIds = new int[commandIdList.size()];
        for (int i = 0; i < commandIdList.size(); i++) {
            commandIds[i] = commandIdList.get(i).intValue();
        }
        return commandIds;
    }

    public Handler getVoiceWakeupModeHandler() {
        return mVoiceWakeupModeHandler;
    }
}