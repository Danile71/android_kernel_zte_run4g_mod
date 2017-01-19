/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.phone.plugin;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.media.ToneGenerator;
import android.net.Uri;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.phone.common.HapticFeedback;
import com.android.phone.Constants;
import com.mediatek.op09.plugin.R;
import com.mediatek.calloption.plugin.OP09CallOptionUtils;
import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.phone.ext.EmergencyDialerExtension;
import com.mediatek.phone.ext.IEmergencyDialer;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;

public class EmergencyDialerOP09Extension extends EmergencyDialerExtension
                                          implements View.OnClickListener {

    private static final String LOG_TAG = "EmergencyDialerOP09Extension";
    private static final boolean DBG = true;

    private static final String ID = "id";
    private static final String ID_NAME_TOP = "top";
    private static final String ID_NAME_DIAL_BUTTON_CONTAINER = "dialButtonContainer";

    private static final String BOOLEAN = "bool";
    private static final String BOOLEAN_NAME_CONFIG_ENABLE_DIALER_KEY_VIBRATION = "config_enable_dialer_key_vibration";

    private static final int BAD_EMERGENCY_NUMBER_DIALOG = 0;


    private Activity mActivity;
    private Context mPluginContext;
    private IEmergencyDialer mEmergencyDialer;

    private HapticFeedback mHaptic;

    private ImageButton mDialButtonLeft;
    private ImageButton mDialButtonRight;
    private EditText mDigits;
    private LinearLayout mNewDialButtonContainer;

    private BroadcastReceiver mReceiver;

    private boolean mIsShowDialButton;

    private int mLastClickedDialButtonId;

    public EmergencyDialerOP09Extension(Context pluginContext) {
        mPluginContext = pluginContext;
        mHaptic = new HapticFeedback();
    }

    public void onCreate(Activity activity, IEmergencyDialer emergencyDialer) {
        mActivity = activity;
        mEmergencyDialer = emergencyDialer;

        mReceiver = new EmergencyDialerBroadcastReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        mActivity.registerReceiver(mReceiver, intentFilter);

        Resources resource = mActivity.getResources();
        String packageName = mActivity.getPackageName();

        LinearLayout emergencyDialerLayout =
                (LinearLayout) mActivity.findViewById(resource.getIdentifier(ID_NAME_TOP, ID, packageName));
        if (null != emergencyDialerLayout) {
            View dialButtonContainer
                    = emergencyDialerLayout.findViewById(resource.getIdentifier(ID_NAME_DIAL_BUTTON_CONTAINER,
                                                         ID, packageName));
            if (null != dialButtonContainer) {
                emergencyDialerLayout.removeView(dialButtonContainer);
                LayoutInflater inflater = LayoutInflater.from(mPluginContext);
                mNewDialButtonContainer = (LinearLayout)inflater.inflate(R.layout.dialer_button_container, null);
                if (null != mNewDialButtonContainer) {
                    mNewDialButtonContainer.setId(
                            resource.getIdentifier(ID_NAME_DIAL_BUTTON_CONTAINER, ID, packageName));
                    emergencyDialerLayout.addView(mNewDialButtonContainer);
                    mDialButtonLeft = (ImageButton)mNewDialButtonContainer.findViewById(R.id.dialButtonLeft);
                    if (null != mDialButtonLeft) {
                        mDialButtonLeft.setOnClickListener(this);
                    }
                    mDialButtonRight = (ImageButton)mNewDialButtonContainer.findViewById(R.id.dialButtonRight);
                    if (null != mDialButtonRight) {
                        mDialButtonRight.setOnClickListener(this);
                    }
                }
            }
        }

        mIsShowDialButton = resource.getBoolean(resource.getIdentifier("config_show_onscreen_dial_button",
                                                "bool", packageName));
        if (!mIsShowDialButton) {
            mNewDialButtonContainer.setVisibility(View.INVISIBLE);
        }

        mDigits = (EditText) mActivity.findViewById(resource.getIdentifier("digits",
                                                    ID, packageName));
        mHaptic.init(mActivity,
                resource.getBoolean(resource.getIdentifier(BOOLEAN_NAME_CONFIG_ENABLE_DIALER_KEY_VIBRATION,
                                    BOOLEAN, packageName)));
        updateCallButtons();
    }

    private void updateCallButtons() {
        if (!mIsShowDialButton) {
            mNewDialButtonContainer.setVisibility(View.INVISIBLE);
        }

        // for pin lock, SIM me lock case
        int slot = mActivity.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);
        if (-1 != slot) {
            SimInfoRecord simInfo = SIMInfoWrapper.getDefault().getSimInfoBySlot(slot);
            if (null != simInfo && simInfo.mColor >= 0) {
                ImageButton dialButtonShow = (PhoneConstants.GEMINI_SIM_1 == slot) ? mDialButtonLeft : mDialButtonRight;
                ImageButton dialButtonHide = (PhoneConstants.GEMINI_SIM_1 != slot) ? mDialButtonLeft : mDialButtonRight;
                dialButtonHide.setVisibility(View.GONE);
                dialButtonShow.setVisibility(View.VISIBLE);
                if (PhoneConstants.GEMINI_SIM_1 == slot) {
                    dialButtonShow.setImageDrawable(
                            mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_1_orange));
                } else if (PhoneConstants.GEMINI_SIM_2 == slot) {
                    dialButtonShow.setImageDrawable(
                            mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_2_blue));
                } else {
                    log("slot is larger than 0 or 1");
                }
            }
            return;
        } else {
            mDialButtonLeft.setImageDrawable(
                    mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_1_orange));
            mDialButtonRight.setImageDrawable(
                    mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_2_blue));
            mDialButtonLeft.setVisibility(View.VISIBLE);
            mDialButtonRight.setVisibility(View.VISIBLE);
        }
    }

    public boolean updateDialAndDeleteButtonStateEnabledAttr() {
        if (null != mDialButtonLeft) {
            mDialButtonLeft.setEnabled(!isDigitEmpty());
        }
        if (null != mDialButtonRight) {
            mDialButtonRight.setEnabled(!isDigitEmpty());
        }
        return true;
    }

    public void onClick(View view) {

        switch (view.getId()) {

            case R.id.dialButtonLeft:
                mHaptic.vibrate();
                log("click dialButtonLeft button");
                mLastClickedDialButtonId = R.id.dialButtonLeft;
                if (null != mEmergencyDialer) {
                    mEmergencyDialer.placeCall();
                }
                return;

            case R.id.dialButtonRight:
                mHaptic.vibrate();
                log("click dialButtonRight button");
                mLastClickedDialButtonId = R.id.dialButtonRight;
                if (null != mEmergencyDialer) {
                    mEmergencyDialer.placeCall();
                }
                return;

            default:
                break;
        }
    }

    public boolean placeCall(String lastNumber) {
        boolean isECCNumber = false;
        if (R.id.dialButtonLeft == mLastClickedDialButtonId) {
            isECCNumber = PhoneNumberUtils.isEmergencyNumberExt(lastNumber,
                    TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_1));
        } else if (R.id.dialButtonRight == mLastClickedDialButtonId) {
            isECCNumber = PhoneNumberUtils.isEmergencyNumberExt(lastNumber,
                    TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_2));
        } else {
            log("This extension do not click any dial button, but it's placeCall() is called, it's strange");
            return false;
        }
        if (isECCNumber) {
            log("placing call to " + lastNumber);

            // place the call if it is a valid number
            if (null == lastNumber || !TextUtils.isGraphic(lastNumber)) {
                mEmergencyDialer.playTone(ToneGenerator.TONE_PROP_NACK);
                return true;
            }
            Intent intent = new Intent(Intent.ACTION_CALL_EMERGENCY);
            intent.setData(Uri.fromParts(Constants.SCHEME_TEL, lastNumber, null));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            if (R.id.dialButtonLeft == mLastClickedDialButtonId) {
                intent.putExtra(Constants.EXTRA_SLOT_ID, PhoneConstants.GEMINI_SIM_1);
            } else if (R.id.dialButtonRight == mLastClickedDialButtonId) {
                intent.putExtra(Constants.EXTRA_SLOT_ID, PhoneConstants.GEMINI_SIM_2);
            } else {
                log("This extension do not click any dial button, but it's placeCall() is called, it's strange");
                return false;
            }
            mActivity.startActivity(intent);
            mActivity.finish();
        } else {
            log("rejecting bad requested number " + lastNumber);

            // erase the number and throw up an alert dialog.
            if (null != mDigits) {
                mDigits.getText().delete(0, mDigits.getText().length());
            }
            mActivity.showDialog(BAD_EMERGENCY_NUMBER_DIALOG);
        }
        return true;
    }

    private boolean isDigitEmpty() {
        return (null == mDigits) ? true : mDigits.length() == 0;
    }

    public void onDestroy() {
        if (null != mActivity && null != mReceiver) {
            mActivity.unregisterReceiver(mReceiver);
        }
        mActivity = null;
        mEmergencyDialer = null;
    }

    private class EmergencyDialerBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("EmergencyDialerBroadcastReceiver, onReceive action = " + action);

            if (TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)) {
                updateCallButtons();
            }
        }
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
