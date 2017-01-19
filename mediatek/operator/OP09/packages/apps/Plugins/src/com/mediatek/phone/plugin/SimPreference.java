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

package com.mediatek.phone.plugin;

import android.content.Context;
import android.os.ServiceManager;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;

public class SimPreference extends Preference {
    private int mSimColor;
    private int mSimSlot;
    private String mSimName;
    private String mSimNumber;
    private String mSimIconNumber;

    public SimPreference(Context context) {
        this(context, null);
    }

    public SimPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onBindView(View contentView) {
        super.onBindView(contentView);

        ImageView imageStatus = (ImageView)contentView.findViewById(R.id.simStatus);
        try {          
            ITelephony iTelephony = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
            /// M: LEGO refactory start @{
            ITelephonyEx telephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
            /// @}
            if (imageStatus != null && iTelephony != null) {
                /// M: LEGO refactory start @{
                int res = getSimStatusImge(telephonyEx.getSimIndicatorState(mSimSlot));
                /// @}
                if (res == -1) {
                    imageStatus.setVisibility(View.GONE);
                } else {
                    imageStatus.setVisibility(View.VISIBLE);
                    imageStatus.setImageResource(res);              
                }
            }
        } catch (android.os.RemoteException e) {
            imageStatus.setVisibility(View.GONE);
        }

        RelativeLayout viewSim = (RelativeLayout)contentView.findViewById(R.id.simIcon);
        if (viewSim != null) {
            if ((mSimColor >= 0) && (mSimColor <= 3)) {
                viewSim.setBackgroundResource(SimInfoManager.SimBackgroundDarkRes[mSimColor]);
            } else {
                viewSim.setBackgroundDrawable(null);
            }
        }   

        TextView simNumber = (TextView)contentView.findViewById(R.id.showNumber);
        if (simNumber != null) {
            if (mSimNumber != null && !mSimNumber.isEmpty()) {
                simNumber.setText(mSimNumber);
            } else {
                simNumber.setVisibility(View.GONE);
            }
        }        

        TextView operatorName = (TextView)contentView.findViewById(R.id.operatorName);
        if (operatorName != null) {
            operatorName.setText(mSimName);
        }

        TextView simIconNumber = (TextView)contentView.findViewById(R.id.simNum);
        if (simIconNumber != null) {
            simIconNumber.setText(mSimIconNumber);
        }
    }

    public void setSimColor(int color) {
        mSimColor = color; 
    }

    public void setSimSlot(int slot) {
        mSimSlot = slot; 
    }

    public void setSimName(String name) {
        mSimName = name; 
    }

    public void setSimNumber(String number) {
        mSimNumber = number; 
    }

    public void setSimIconNumber(String number) {
        mSimIconNumber = number; 
    }

    private int getSimStatusImge(int state) {
        int resId = 0;
        switch (state) {
            case PhoneConstants.SIM_INDICATOR_RADIOOFF:
                resId = com.mediatek.internal.R.drawable.sim_radio_off;
                break;
            case PhoneConstants.SIM_INDICATOR_LOCKED:
                resId = com.mediatek.internal.R.drawable.sim_locked;
                break;
            case PhoneConstants.SIM_INDICATOR_INVALID:
                resId = com.mediatek.internal.R.drawable.sim_invalid;
                break;
            case PhoneConstants.SIM_INDICATOR_SEARCHING:
                resId = com.mediatek.internal.R.drawable.sim_searching;
                break;
            case PhoneConstants.SIM_INDICATOR_ROAMING:
                resId = com.mediatek.internal.R.drawable.sim_roaming;
                break;
            case PhoneConstants.SIM_INDICATOR_CONNECTED:
                resId = com.mediatek.internal.R.drawable.sim_connected;
                break;
            case PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
                resId = com.mediatek.internal.R.drawable.sim_roaming_connected;
                break;
            default:
                resId = -1;
                break;
        }
        return resId;
    }
}
