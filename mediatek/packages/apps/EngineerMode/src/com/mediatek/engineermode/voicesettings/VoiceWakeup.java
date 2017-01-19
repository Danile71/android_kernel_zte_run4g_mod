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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERfETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

package com.mediatek.engineermode.voicesettings;

import android.app.Activity;
import android.view.View;
import android.os.Bundle;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.provider.Settings;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;

public class VoiceWakeup extends Activity {
    private static final String TAG = "EM/VOICE-WAKEUP";
    private RadioGroup mRgSwtich = null;

    /* 0: disabe function switch
     * 1: switch 1 
     * 2: switch 2
     */
    private int mSwitchInfo = 0;  
    private int mSwitchEnable = 0;
    
    
    private final RadioGroup.OnCheckedChangeListener mCheckedListener = new RadioGroup.OnCheckedChangeListener() {
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            if (group.equals(mRgSwtich)) {
                int state = 0;
                if (checkedId == R.id.voice_switch_1){
                    Elog.v(TAG, "check voice_switch_1");
                    state = VoiceCommandListener.VOW_MODE_WAKEUP_ANYONE;
                } else if (checkedId == R.id.voice_switch_2) {
                    Elog.v(TAG, "check voice_switch_2");
                    state = VoiceCommandListener.VOW_MODE_WAKEUP_COMMAND;
                }
                Settings.System.putInt(getContentResolver(), Settings.System.VOICE_WAKEUP_MODE, state);
            }
            
        }
    };
    
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.voice_wakeup);

        mSwitchInfo = Settings.System.getInt(getContentResolver(), Settings.System.VOICE_WAKEUP_MODE, 0);
        
        mRgSwtich = (RadioGroup) findViewById(R.id.voice_wakeup_function_switch);
        mRgSwtich.setOnCheckedChangeListener(mCheckedListener);
        
        if (mSwitchInfo == VoiceCommandListener.VOW_MODE_WAKEUP_ANYONE) {  // = 1 Keyword recognition mode
            mRgSwtich.check(R.id.voice_switch_1);
        } else if (mSwitchInfo == VoiceCommandListener.VOW_MODE_WAKEUP_COMMAND) { // = 2 Keyword + speaker recognition mode
            mRgSwtich.check(R.id.voice_switch_2);
        } else {
            Elog.w(TAG, "Wrong input switch info");
        }
    }
    @Override
    protected void onResume() {
        super.onResume();
        mSwitchEnable = Settings.System.getInt(getContentResolver(), Settings.System.VOICE_WAKEUP_COMMAND_STATUS, VoiceCommandListener.VOW_STATUS_NO_COMMAND_UNCHECKED);
        if (mSwitchEnable == VoiceCommandListener.VOW_STATUS_NO_COMMAND_UNCHECKED) { // no command and unchecked
            for (int i = 0; i < mRgSwtich.getChildCount(); i++) {
                mRgSwtich.getChildAt(i).setEnabled(true);
            }
        } else {
           Elog.v(TAG, "Disable switch info");
           for (int i = 0; i < mRgSwtich.getChildCount(); i++) {
                mRgSwtich.getChildAt(i).setEnabled(false);
            }
        }
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

}
