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

import android.app.AlertDialog;
import android.content.Context;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op07.plugin.R;
import com.mediatek.phone.ext.SettingsExtension;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class SettingsOP07Extension extends SettingsExtension {

    private AlertDialog.Builder mDialogBuild; 
    private static final String TAG = "SettingsOP07Extension";
    private static final String NETWORK_CLASS = "com.android.phone.NetworkSetting";
    private static final String BUTTON_CARRIER_SEL = "button_carrier_sel_key";
    
    private Context mContext;
    
    public SettingsOP07Extension(Context context) {
        mContext = context;
    }

    public void dataEnableReminder(boolean isDataEnabled, boolean isRoamingEnabled,
            PreferenceActivity activity) {
        boolean isInRoaming = ((TelephonyManager) activity.getSystemService(activity.TELEPHONY_SERVICE)).isNetworkRoaming();
        if (isDataEnabled && isInRoaming && !isRoamingEnabled) {
        if (mDialogBuild == null){
            Xlog.d(TAG,"create new dialog");
                mDialogBuild = new AlertDialog.Builder(activity);
                mDialogBuild.setMessage(mContext.getString(R.string.data_conn_under_roaming_hint)).setTitle(
                        android.R.string.dialog_alert_title).setIcon(
                        android.R.drawable.ic_dialog_alert).setNegativeButton(
                        android.R.string.ok, null).create();
        }
        mDialogBuild.show();  
    }
        super.dataEnableReminder(isDataEnabled, isRoamingEnabled, activity);
        }

    public void customizeFeatureForOperator(PreferenceScreen prefSet) {
        Preference buttonPreferredNetworkModeEx = prefSet.findPreference(BUTTON_NETWORK_MODE_EX_KEY);
        Preference buttonPreferredNetworkMode = (ListPreference) prefSet.findPreference(
                BUTTON_PREFERED_NETWORK_MODE);
        Preference buttonPreferredGSMOnly = (CheckBoxPreference) prefSet.findPreference(BUTTON_2G_ONLY);
        Preference buttonGsmUmtsPreferredNetworkMode = (ListPreference) prefSet.findPreference(
                BUTTON_NETWORK_MODE_KEY);
        if(buttonPreferredNetworkModeEx !=null)
        {
            prefSet.removePreference(buttonPreferredNetworkModeEx);
        }
        if(buttonPreferredNetworkMode !=null)
        {        
            prefSet.removePreference(buttonPreferredNetworkMode);
        }
        if(buttonPreferredGSMOnly !=null)
        {        
            prefSet.removePreference(buttonPreferredGSMOnly);
        }
        if (buttonGsmUmtsPreferredNetworkMode != null) {
            prefSet.removePreference(buttonGsmUmtsPreferredNetworkMode);
        }
    }

    public void removeNMOp(PreferenceScreen root, boolean isShowPlmn) {
        Preference carrierSelPref = (PreferenceScreen) root.findPreference(BUTTON_CARRIER_SEL);
        if(isShowPlmn){
            root.addPreference(carrierSelPref);
        }else{
            root.removePreference(carrierSelPref);
        }
    }

    public void removeNMOpFor3GSwitch(PreferenceScreen prefSc,
            Preference networkMode) {
            prefSc.removePreference(networkMode);
    }

    public void removeNMOpForMultiSim(Phone phone, List<SimInfoRecord> simList,
            String targetClass) {
        if(!NETWORK_CLASS.equals(targetClass)){
            return;
        }
        GeminiPhone dualPhone = null;
        if (phone instanceof GeminiPhone){
            dualPhone = (GeminiPhone)phone;
        }

        int size = simList.size();
        if(size == 2){
            SimInfoRecord temp1=simList.get(0);
            SimInfoRecord temp2=simList.get(1);
            simList.clear();
            if(dualPhone.getPhonebyId(temp1.mSimSlotId).isCspPlmnEnabled())
            {
                simList.add(temp1);
            }
            if(dualPhone.getPhonebyId(temp2.mSimSlotId).isCspPlmnEnabled())
            {
                simList.add(temp2);
            }
        }
    }

    public void disableDataRoaming(CheckBoxPreference buttonDataRoam,
            boolean isEnabled) {
    	Xlog.d(TAG,"OP07 plug in disableDataRoaming---isEnabled="+isEnabled);
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            buttonDataRoam.setEnabled(isEnabled);
        } 
    }

    public String getRoamingMessage(Context context, int res) {
        String message = mContext.getString(R.string.data_roaming_warning);
        Xlog.d(TAG, "Op07 getRoamingMessage with message = " + message);
        return message;
    }

    public void setRoamingSummary(CheckBoxPreference f, int resIdOn,
            int resIdOff) {
        String summary = mContext.getString(R.string.data_roaming_warning);
        f.setSummaryOn(summary);
        f.setSummaryOff(summary);
        Xlog.d(TAG,"Op07 setRoamingSummary with summary=" + summary);
    } 
}
