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

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity.Header;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Switch;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.op09.plugin.R;
import com.mediatek.settings.ext.DefaultSettingsMiscExt;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class OP09SettingsMiscExtImp extends DefaultSettingsMiscExt {

    private static final String TAG = "SettingsMiscExt";
    private static final String TAB_SIM_1 = "sim1";
    private static final String TAB_SIM_2 = "sim2";
    private static final String TAB_SIM_1_INDEX = "1";
    private static final String TAB_SIM_2_INDEX = "2";
    private static final String TAB_MOBILE = "mobile";
    private static final String INTERNATIONAL_ROAMING = "com.mediatek.OP09.INTERNATIONAL_ROAMING";
    private static final String DATA_CONNECTION_ACTIVITY_INTENT = "com.mediatek.OP09.DATA_CONNECTION_SETTING";

    private static final String CUSTOMIZE_ITEM_NAME = "customize_item_name";
    private static final String INTERNATIONAL_ROAMING_ITEM_NAME = "international_roaming";
    private static final String DATA_CONNECTION_ITEM_NAME = "data_connection";

    private static final int COLORNUM = 2;

    private Context mContext;
    private DataConnectionEnabler mDataConnecitonEnabler;

    public OP09SettingsMiscExtImp(Context context) {
        super(context);
        mContext = context;
        Xlog.d(TAG, "SettingsMiscExt this=" + this);
    }

    public void updateDefaultSimPreferenceLayout(Preference pref) {
        pref.setLayoutResource(R.layout.default_sim_preference);
    }

    public LayoutInflater getLayoutInflater(Context context) {
        return (LayoutInflater) this
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void updateDefaultSimPreferenceSimIndicator(View viewContainer,
            int color) {
        ImageView simIndicatorView = (ImageView) viewContainer
                .findViewById(R.id.sim_indicator);
        if (simIndicatorView != null && color != -1) {
            simIndicatorView.setImageResource(getSimColorResource(color));
            simIndicatorView.setVisibility(View.VISIBLE);
        } else {
            simIndicatorView.setVisibility(View.GONE);
        }
    }

    public void insertSimIndicatorInMobileDataSummaryItem(View container,
            View titleView, int slotId) {
        if (container instanceof RelativeLayout) {
            RelativeLayout viewContainer = (RelativeLayout) container;
            ImageView simIndicator = new ImageView(mContext);
            simIndicator.setId(R.id.data_usage_mobile_item_sim_indicator_view);
            RelativeLayout.LayoutParams simIndicatorLayoutParams = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            simIndicatorLayoutParams.addRule(RelativeLayout.CENTER_VERTICAL);
            simIndicatorLayoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
            int marginRight = (int) mContext.getResources().getDimension(
                    R.dimen.data_usage_summary_sim_indicator_margin_right);
            simIndicatorLayoutParams.rightMargin = marginRight;
            simIndicator.setLayoutParams(simIndicatorLayoutParams);
            int paddingLeft = (int) mContext.getResources().getDimension(
                    R.dimen.data_usage_summary_sim_indicator_padding_left);
            simIndicator.setPadding(paddingLeft, 0, 0, 0);
            simIndicator
                    .setImageResource(slotId == PhoneConstants.GEMINI_SIM_1 ? R.drawable.ic_datausage_sim_1
                            : R.drawable.ic_datausage_sim_2);
            viewContainer.addView(simIndicator, 0);
            RelativeLayout.LayoutParams titleLayoutParams = (RelativeLayout.LayoutParams) titleView
                    .getLayoutParams();
            titleLayoutParams.removeRule(RelativeLayout.ALIGN_PARENT_LEFT);
            titleLayoutParams.addRule(RelativeLayout.RIGHT_OF,
                    R.id.data_usage_mobile_item_sim_indicator_view);
        }
    }

    private static int getSimColorResource(int color) {
        if ((color >= 0) && (color <= COLORNUM)) {
            return SimInfoManager.SimBackgroundDarkSmallRes[color];
        } else {
            return -1;
        }
    }

    /**
     * Add customize headers in settings, lick International Roaming header
     * and data connection header
     */
    @Override
    public void addCustomizedItem(List<Header> target, int index) {
        target.add(index, createIRHeader());
        target.add(index++, createDataConnectionHeader());
    }

    // Create International Roaming header
    private Header createIRHeader() {
        Header header = new Header();
        header.title = mContext
                .getString(R.string.international_roaming_summary_title);
        header.iconRes = R.drawable.ic_international_roaming_set;
        header.intent = new Intent(INTERNATIONAL_ROAMING);
        if (header.extras == null) {
            header.extras = new Bundle();
        }
        header.extras.putString(CUSTOMIZE_ITEM_NAME, INTERNATIONAL_ROAMING_ITEM_NAME);
        return header;
    }

    // Create Data connection header
    private Header createDataConnectionHeader() {
        Header header = new Header();
        header.title = mContext
                .getString(R.string.data_connection_summary_title);
        header.iconRes = R.drawable.ic_data_connection_set;
        header.intent = new Intent(DATA_CONNECTION_ACTIVITY_INTENT);
        if (header.extras == null) {
            header.extras = new Bundle();
        }
        header.extras.putString(CUSTOMIZE_ITEM_NAME, DATA_CONNECTION_ITEM_NAME);
        return header;
    }

    @Override
    public void initSwitherControlers() {
        mDataConnecitonEnabler = new DataConnectionEnabler(mContext, new Switch(mContext));
        Xlog.d(TAG, "initSwitherControlers mDataConnecitonEnabler=" + mDataConnecitonEnabler + ", this=" + this);
    }

    @Override
    public void setEnablerSwitch(Header header, Switch switchs) {
        if (header != null && header.iconRes == R.drawable.ic_data_connection_set && mDataConnecitonEnabler != null) {
            mDataConnecitonEnabler.setSwitch(switchs);
        }
    }

    @Override
    public boolean isSwitcherHeaderType(Header header) {
        return header != null && header.iconRes == R.drawable.ic_data_connection_set;
    }


    @Override
    public void resumeEnabler() {
        Xlog.d(TAG, "resumeEnabler mDataConnecitonEnabler=" + mDataConnecitonEnabler + ", this=" + this);
        if (mDataConnecitonEnabler != null) {
            mDataConnecitonEnabler.resume();
        }
    }

    @Override
    public void pauseEnabler() {
        Xlog.d(TAG, "pauseEnabler mDataConnecitonEnabler=" + mDataConnecitonEnabler + ", this=" + this);
        if (mDataConnecitonEnabler != null) {
            mDataConnecitonEnabler.pause();
        }
    }

    @Override
    public boolean needCustomizeHeaderIcon(Header header) {
        return (header.extras != null && header.extras.containsKey(CUSTOMIZE_ITEM_NAME));
    }

    @Override
    public void customizeHeaderIcon(ImageView iconView, Header header) {
        String customizeItemName = header.extras.getString(CUSTOMIZE_ITEM_NAME);
        Xlog.d(TAG, "getCustomizedItemIcon: customizeItemName = " + customizeItemName);
        if (INTERNATIONAL_ROAMING_ITEM_NAME.equals(customizeItemName)) {
            // We need to use setImageDrawable instead of setImageResource to
            // use the right context.
            iconView.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_international_roaming_set));
        } else if (DATA_CONNECTION_ITEM_NAME.equals(customizeItemName)) {
            iconView.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_data_connection_set));
        }
    }

    /**
     * Customize strings which contains 'SIM', replace 'SIM' by
     * 'UIM/SIM','UIM','card' etc.
     */
    @Override
    public String customizeSimDisplayString(String simString, int slotId) {

        if (-1 == slotId) {
            return replaceSimToSimUim(simString);
        }
        if (PhoneConstants.GEMINI_SIM_1 == slotId) {
            return replaceSimBySlotInner(simString);
        }
        return simString;
    }

    public static String replaceSimBySlotInner(String simString) {
        if (simString.contains("SIM")) {
            simString = simString.replaceAll("SIM", "UIM");
        }
        if (simString.contains("sim")) {
            simString = simString.replaceAll("sim", "uim");
        }
        return simString;
    }

    private String replaceSimToSimUim(String simString) {
        if (simString.contains("SIM")) {
            simString = simString.replaceAll("SIM", "UIM/SIM");
        }
        Xlog.d(TAG, "op09 replace string: " + simString);
        return simString;
    }
}