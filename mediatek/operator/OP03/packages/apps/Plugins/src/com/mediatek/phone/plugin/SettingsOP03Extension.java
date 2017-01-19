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
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.mediatek.op03.plugin.R;
import com.mediatek.phone.ext.SettingsExtension;
import com.mediatek.xlog.Xlog;

public class SettingsOP03Extension extends SettingsExtension {

    private static SettingsOP03Extension mInstance;
    private final static int DATA_ENABLE_ALERT_DIALOG = 100;
    private final static int DATA_DISABLE_ALERT_DIALOG = 200;
    private static final int PROGRESS_DIALOG = 400;
    private static final int DATA_STATE_CHANGE_TIMEOUT = 2001;
    private static final int RESET_DATA_CONNECTION = 2002;
    public static final String BUTTON_OP03_2G_ONLY = "button_prefer_op03_2g_key";
    public static final String BUTTON_NETWORK_MODE_LTE_KEY = "button_network_mode_LTE_key";
    private final Context mContext;

    public SettingsOP03Extension(Context context) {
        mContext = context;
    }
/*
    public static SettingsOP03Extension getInstance() {
        if (null == mInstance) {
            mInstance = new SettingsOP03Extension();
        }
        return mInstance;
    }
*/
    public void customizeFeatureForOperator(PreferenceScreen prefSet){
        Preference buttonPreferredNetworkModeEx = prefSet.findPreference(BUTTON_NETWORK_MODE_EX_KEY);
        Preference buttonPreferredNetworkMode = (ListPreference) prefSet.findPreference(
                BUTTON_PREFERED_NETWORK_MODE);
        Preference buttonGsmUmtsPreferredNetworkMode = (ListPreference) prefSet.findPreference(
                BUTTON_NETWORK_MODE_KEY);
        Preference buttonLtePreferredNetworkMode = (ListPreference) prefSet.findPreference(
                BUTTON_NETWORK_MODE_LTE_KEY);

        if (buttonPreferredNetworkModeEx != null) {
            prefSet.removePreference(buttonPreferredNetworkModeEx);
        }
        if (buttonPreferredNetworkMode != null) {
            prefSet.removePreference(buttonPreferredNetworkMode);
        }
        if (buttonGsmUmtsPreferredNetworkMode != null) {
            prefSet.removePreference(buttonGsmUmtsPreferredNetworkMode);
        }
        if (buttonLtePreferredNetworkMode != null) {
            prefSet.removePreference(buttonLtePreferredNetworkMode);
        }
        
        if (prefSet.findPreference(BUTTON_OP03_2G_ONLY) == null) {
        CheckBoxPreference buttonPreferredGSMOnly = new Use2GOnlyCheckBoxPreference(mContext, prefSet);
        buttonPreferredGSMOnly.setKey(BUTTON_OP03_2G_ONLY);
        buttonPreferredGSMOnly.setTitle(R.string.prefer_2g);
        buttonPreferredGSMOnly.setPersistent(false);
        prefSet.addPreference(buttonPreferredGSMOnly);
    }
    }

    public void dataEnableReminder(boolean isCheckedBefore, boolean isRoamingEnabled, PreferenceActivity activity) {
        if (isCheckedBefore) {
            activity.showDialog(DATA_ENABLE_ALERT_DIALOG);
        }else{
            activity.showDialog(DATA_DISABLE_ALERT_DIALOG);
        }
    }
    
    public Dialog onCreateAlertDialog(int dialogId, final Activity activity, final Handler timeoutHandler) {
        int message = (dialogId == DATA_ENABLE_ALERT_DIALOG ?
                R.string.networksettings_tips_data_enabled
                : R.string.networksettings_tips_data_disabled);
        return new AlertDialog.Builder(activity)
        .setMessage(mContext.getText(message))
        .setTitle(android.R.string.dialog_alert_title)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                ConnectivityManager connService = (ConnectivityManager) activity.getSystemService(Context.CONNECTIVITY_SERVICE);
                if (connService != null) {
                    activity.showDialog(PROGRESS_DIALOG);
                    boolean isConnected = connService.getMobileDataEnabled();
                    connService.setMobileDataEnabled(!isConnected);
                    timeoutHandler.sendMessageDelayed(timeoutHandler.obtainMessage(DATA_STATE_CHANGE_TIMEOUT), 30000);
                }
            }
        })
        .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                timeoutHandler.sendMessage(timeoutHandler.obtainMessage(DATA_STATE_CHANGE_TIMEOUT));
            }
        })
        .setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                timeoutHandler.sendMessage(timeoutHandler.obtainMessage(DATA_STATE_CHANGE_TIMEOUT));
            }
        })
        .create();
    }
    
    public boolean onPreferenceTreeClick(PreferenceScreen prefSet, Preference preference) {
        Preference buttonPreferredGSMOnly = (CheckBoxPreference) prefSet.findPreference(BUTTON_OP03_2G_ONLY);
        if (preference == buttonPreferredGSMOnly) {        
            return true;
        } else {
            return false;
        }
    }
}
