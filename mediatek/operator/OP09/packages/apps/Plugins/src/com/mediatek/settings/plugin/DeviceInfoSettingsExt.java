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
package com.mediatek.settings.plugin;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.preference.PreferenceScreen;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op09.plugin.R;
import com.mediatek.settings.ext.DefaultDeviceInfoSettingsExt;


public class DeviceInfoSettingsExt extends DefaultDeviceInfoSettingsExt {
    private static final String TAG = "DeviceInfoSettingsExt";
    private static final String E_PUSH_KEY = "cdma_epush";
    private static final String KEY_STATUS_INFO = "status_info";
    private static final String KEY_STATUS_INFO_GEMINI = "status_info_gemini";
    private Context mContext;

    public DeviceInfoSettingsExt(Context context) {
        mContext = context;
    }

    @Override
    public void addEpushPreference(PreferenceScreen root) {
        Preference preferenceScreen = getEpushLayout(root.getContext());
        if (null != preferenceScreen) {
            int order = Integer.MAX_VALUE;
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                order = root.findPreference(KEY_STATUS_INFO_GEMINI).getOrder() + 1;
            } else {
                order = root.findPreference(KEY_STATUS_INFO).getOrder() + 1;
            }
            preferenceScreen.setOrder(order);
            root.addPreference(preferenceScreen);
            setEpushEnabledOrNot(preferenceScreen);
        }
    }

    /**
     *
     * @return E push xml perference
     */
    private Preference getEpushLayout(Context context) {
        PreferenceScreen pref = new PreferenceScreen(context, null);
        pref.setKey(E_PUSH_KEY);
        pref.setTitle(mContext.getResources().getString(R.string.cdma_e_push_title));
        pref.setSummary(mContext.getResources().getString(R.string.cdma_e_push_summary));
        return pref;
    }

    /**
     * enable or disable the epush preference,when the 3part app(com.ctc.epush) install,
     * enable the epush preference,or disable epush preference 
     * @param preferenceScreen,is epush preference
     */
    private void setEpushEnabledOrNot(Preference preferenceScreen) {
        Intent intent = new Intent(Intent.ACTION_MAIN, null);
        ComponentName cn = new ComponentName("com.ctc.epush", "com.ctc.epush.IndexActivity");
        intent.setComponent(cn);
        if (mContext.getPackageManager().resolveActivity(intent, 0) == null) {
            if (preferenceScreen != null) {
                // root.removePreference(cdmaEpushPrefs);
                preferenceScreen.setEnabled(false);
            }
        }
    }
}
