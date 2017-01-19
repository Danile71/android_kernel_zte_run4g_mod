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

package com.mediatek.incallui.plugin;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Paint;
import android.os.SystemProperties;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;

import com.android.internal.telephony.PhoneConstants;
import com.android.services.telephony.common.Call;

import com.mediatek.incallui.ext.CallCardExtension;
import com.mediatek.op09.plugin.R;
import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

public class OP09CallCardExtension extends CallCardExtension {

    private static final String LOG_TAG = "OP09CallCardExtension";
    private static final boolean DBG = true;

    private Context mPluginContext;
    private LayoutInflater mLayoutInflater;
    //private View mCallCard;
    private RelativeLayout mCallAndSimIndicator;
    private RelativeLayout mPluginCallAndSimIndicator;
    private TextView mSimIndicator;
    private TextView mPluginSimIndicator;
    private LayoutParams mHostSimIndicatorParams;
    private int mSimIndicatorMarginRight;
    private int mSimIndicatorMaxWidth;

    public OP09CallCardExtension(Context pluginContext) {
        mPluginContext = pluginContext;
        mLayoutInflater = LayoutInflater.from(mPluginContext);
    }
 
    public void onViewCreated(Context context, View rootView) {
        if (DBG) {
            log("onFinishInflate()");
        }

        if (null != rootView) {
            mSimIndicator =
                (TextView) rootView.findViewById(
                        rootView.getResources().getIdentifier("simIndicator", "id",
                                                              rootView.getContext().getPackageName()));
            mCallAndSimIndicator =
                (RelativeLayout) rootView.findViewById(
                        rootView.getResources().getIdentifier("callStateAndSimIndicate", "id",
                                                              rootView.getContext().getPackageName()));
            mSimIndicatorMaxWidth = mPluginContext.getResources().getDimensionPixelSize(R.dimen.call_card_operator_name_max);

            //add sim indicator for CT on screen in host.
            mSimIndicatorMarginRight = mPluginContext.getResources().getDimensionPixelSize(R.dimen.call_banner_sim_indicator_icon_padding_right);
            mPluginCallAndSimIndicator = (RelativeLayout) mLayoutInflater.inflate(R.layout.call_card_ct, null);
            mPluginSimIndicator =
                           (TextView) mPluginCallAndSimIndicator.findViewById(R.id.pluginsimIndicator);
            mCallAndSimIndicator.addView(mPluginCallAndSimIndicator);
            mHostSimIndicatorParams = (LayoutParams)mSimIndicator.getLayoutParams();
            if (null != mHostSimIndicatorParams) {
                int rightPending = mSimIndicator.getMeasuredWidth() + mSimIndicatorMarginRight;
                mHostSimIndicatorParams.setMargins(0, 0, rightPending, 0);
                mSimIndicator.setLayoutParams(mHostSimIndicatorParams);
            }
        }
    }

    public void updatePrimaryDisplayInfo(Call call) {
        if (DBG) {
            log("displayMainCallStatus(), call = " + call);
        }
        if (null == call) {
            return;
        }
        SimInfoRecord simInfo = SIMInfoWrapper.getDefault().getSimInfoBySlot(call.getSlotId());
        String number = call.getNumber();
        boolean isEmergencyNumber = (SystemProperties.getInt("ro.evdo_dt_support", 0) == 1) ?
                        (PhoneNumberUtils.isEmergencyNumberExt(number, PhoneConstants.PHONE_TYPE_CDMA) ||
                                PhoneNumberUtils.isEmergencyNumberExt(number, PhoneConstants.PHONE_TYPE_GSM)) :
                        PhoneNumberUtils.isEmergencyNumber(number);

        if (isEmergencyNumber) {
            if (DBG) {
                log("displayMainCallStatus(), set sim indicator");
            }
            if (View.INVISIBLE == mSimIndicator.getVisibility()
                    || View.GONE == mSimIndicator.getVisibility()) {
                if (PhoneConstants.GEMINI_SIM_1 == call.getSlotId()) {
                    mSimIndicator.setBackgroundDrawable(
                            mPluginContext.getResources().getDrawable(R.drawable.ic_ecc_slot_1_orange));
                } else if (PhoneConstants.GEMINI_SIM_2 == call.getSlotId()) {
                    mSimIndicator.setBackgroundDrawable(
                            mPluginContext.getResources().getDrawable(R.drawable.ic_ecc_slot_2_blue));
                } else {
                    if (DBG) {
                        log("displayMainCallStatus(), slot id is not GEMINI_SIM_1 or GEMINI_SIM_2, just return");
                    }
                    return;
                }
                mSimIndicator.setVisibility(View.VISIBLE);
            } else if (null != simInfo) {
                mSimIndicator.setBackgroundResource(SimInfoManager.SimBackgroundLightSmallRes[simInfo.mColor]);
            }
        } else if (null != mSimIndicator && mSimIndicator.getVisibility() == View.VISIBLE) {
            if (call.getCallType() == Call.CALL_TYPE_SIP) {
                mSimIndicator.setBackgroundDrawable(
                            mPluginContext.getResources().getDrawable(R.drawable.dark_small_internet_call));
            } else if (null != simInfo) {
                mSimIndicator.setBackgroundResource(SimInfoManager.SimBackgroundLightSmallRes[simInfo.mColor]);
            }
        }
        //show operator name on screen, if name is null (ECC), not display it.
        if (null != mSimIndicator && mSimIndicator.getVisibility() == View.VISIBLE) {
            if (null != simInfo) {
                mPluginSimIndicator.setText(simInfo.mDisplayName);
            } else {
                mPluginSimIndicator.setText(mSimIndicator.getText());
            }
            if (0 == (mPluginSimIndicator.getText()).length()) {
                mHostSimIndicatorParams.setMargins(0, 0, 0, 0);
            } else {
                Paint paint = new Paint();
                paint.set(mPluginSimIndicator.getPaint());
                int inputWidth = (int) paint.measureText(mPluginSimIndicator.getText().toString());
                if (inputWidth > mSimIndicatorMaxWidth) {
                    inputWidth = mSimIndicatorMaxWidth;
                }
                int rightPending = inputWidth + mSimIndicatorMarginRight;
                mHostSimIndicatorParams.setMargins(0, 0, rightPending, 0);
            }
            mPluginCallAndSimIndicator.setVisibility(View.VISIBLE);
            mPluginSimIndicator.setVisibility(View.VISIBLE);

            mSimIndicator.setLayoutParams(mHostSimIndicatorParams);
        } else {
            mPluginCallAndSimIndicator.setVisibility(View.GONE);
        }
        mSimIndicator.setText("");
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

}
