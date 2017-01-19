/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.swla;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;



public class SwlaActivity extends Activity {

    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    
    private static final int MSG_ASSERT = 1;
    private static final int MSG_SWLA_ENABLE = 2;
    private static final int MSG_ASSERT1 = 3;
    private static final int MSG_ASSERT2 = 4;

    private static final String TAG = "SWLA";
    private boolean mPaused = false;
    private Button mAssertBtn1, mAssertBtn2;
    private Button mEnableSwlaBtn;
    
    private final Handler mATCmdHander = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            AsyncResult ar;
            switch (msg.what) {
            case MSG_ASSERT:
                mAssertBtn1.setEnabled(true);
                if (mPaused) {
                    return;
                }
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(SwlaActivity.this, "Assert Modem Success.", Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(SwlaActivity.this, "Assert Modem Failed.", Toast.LENGTH_LONG).show();
                }
                break;
            case MSG_ASSERT1:
            case MSG_ASSERT2:
                mAssertBtn1.setEnabled(true);
                mAssertBtn2.setEnabled(true);
                String sucStr = "Assert Modem1 Success.";
                String failStr = "Assert Modem1 Failed.";
                if (mPaused) {
                    return;
                }
                ar = (AsyncResult) msg.obj;
                if(msg.what == MSG_ASSERT2) {
                    sucStr = "Assert Modem2 Success.";
                    failStr = "Assert Modem2 Failed.";
                }
                if (ar.exception == null) {
                    Toast.makeText(SwlaActivity.this, sucStr, Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(SwlaActivity.this, failStr, Toast.LENGTH_LONG).show();
                }
                break;
                
            case MSG_SWLA_ENABLE:
                mEnableSwlaBtn.setEnabled(true);
                if (mPaused) {
                    return;
                }
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(SwlaActivity.this, "Enable Softwore LA Success", Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(SwlaActivity.this, "Enable Softwore LA Failed.", Toast.LENGTH_LONG).show();
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.swla_activity);
        mAssertBtn1 = (Button) findViewById(R.id.swla_assert_btn1);
        mAssertBtn2 = (Button) findViewById(R.id.swla_assert_btn2);
        if(PhoneFactory.isDualTalkMode()){
            mAssertBtn1.setText("Assert Modem1");
            mAssertBtn2.setText("Assert Modem2");
        } else {
            mAssertBtn2.setVisibility(View.GONE);
        }
        mEnableSwlaBtn = (Button) findViewById(R.id.swla_swla_btn);
        mAssertBtn1.setOnClickListener(new ButtonListener());
        mAssertBtn2.setOnClickListener(new ButtonListener());
        mEnableSwlaBtn.setOnClickListener(new ButtonListener());

        mPhone = PhoneFactory.getDefaultPhone();
        if(PhoneFactory.isDualTalkMode()){
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        }
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        mPaused = false;
    }

    @Override
    protected void onPause() {
        mPaused = true;
        super.onPause();
    }
    
    class ButtonListener implements View.OnClickListener {

        public void onClick(View v) {
            switch (v.getId()) {
            case R.id.swla_assert_btn1:
                if(!PhoneFactory.isDualTalkMode()){
                    mAssertBtn1.setEnabled(false);
                    sendATCommad("0", MSG_ASSERT);
                } else {
                    mAssertBtn1.setEnabled(false);
                    mAssertBtn2.setEnabled(false);
                    sendATCommadExt("0", MSG_ASSERT1, PhoneConstants.GEMINI_SIM_1);  
                }
                break;
             case R.id.swla_assert_btn2:
                if(PhoneFactory.isDualTalkMode()){
                    mAssertBtn1.setEnabled(false);
                    mAssertBtn2.setEnabled(false);
                    sendATCommadExt("0", MSG_ASSERT2, PhoneConstants.GEMINI_SIM_2);  
                }
                break;
            case R.id.swla_swla_btn:
                mEnableSwlaBtn.setEnabled(false);
                sendATCommad("1", MSG_SWLA_ENABLE);
                break;
            default:
                break;
            }
        }
    };

    private void sendATCommad(String str, int message) {

        String aTCmd[] = new String[2];
        aTCmd[0] = "AT+ESWLA=" + str;
        aTCmd[1] = "";
        mPhone.invokeOemRilRequestStrings(aTCmd, mATCmdHander.obtainMessage(message));
        Elog.i(TAG, "Send ATCmd : " + aTCmd[0]);
    }
    private void sendATCommadExt(String str, int message, int simType) {
        String aTCmd[] = new String[2];
        aTCmd[0] = "AT+ESWLA=" + str;
        aTCmd[1] = "";
        int targSim = simType;
        if (mGeminiPhone.get3GSimId() == 1) {
            if (simType == PhoneConstants.GEMINI_SIM_1) {
                targSim = PhoneConstants.GEMINI_SIM_2;
            } else if (simType == PhoneConstants.GEMINI_SIM_2) {
                targSim = PhoneConstants.GEMINI_SIM_1;
            } else {
                Elog.d(TAG, "Invalid simType:" + simType);
            }
        }
        mGeminiPhone.invokeOemRilRequestStringsGemini(aTCmd,
                    mATCmdHander.obtainMessage(message), targSim);
        Elog.i(TAG, "Send ATCmd : " + aTCmd[0] + " simType:" + simType + " targSim:" + targSim);
    }
}
