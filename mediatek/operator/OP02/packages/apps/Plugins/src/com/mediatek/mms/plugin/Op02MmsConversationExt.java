/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.mms.ext.DefaultMmsConversationExt;
import com.mediatek.op02.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class Op02MmsConversationExt extends DefaultMmsConversationExt {

    private static final String TAG = "Mms/Op02MmsConversationExt";

    private int mMenuSimSms = 0;

    public static final String ACTION_VIEW_SELECT_CARD = "com.android.mms.ui.SelectCardPreferenceActivity";
    public static final String ACTION_VIEW_SIM_MESSAGES = "com.android.mms.ui.ManageSimMessages";
    public static final String SMS_MANAGE_SIM_MESSAGES  = "pref_key_manage_sim_messages";

    public static final int TITLE_INDEX_STRING_MANAGE_SIM_MESSAGE = 1;

    public Op02MmsConversationExt(Context context) {
        super(context);
    }

    public void addOptionMenu(Menu menu, int base){
        Drawable sMenuIcon = 
            getResources().getDrawable(R.drawable.ic_menu_sim_sms);
        mMenuSimSms = base + 1;
        menu.add(0, mMenuSimSms, 0, getString(R.string.menu_sim_sms)).setIcon(
                    R.drawable.ic_menu_sim_sms);

        Xlog.d(TAG, "Add Menu: " + getString(R.string.menu_sim_sms));
        return;
    }

    public void onPrepareOptionsMenu(Menu menu) {
        MenuItem item = menu.findItem(mMenuSimSms);
        if(!Op02MmsUtils.isSmsEnabled(this) ||
           !Op02MmsUtils.isSimInserted(this) ||
            Op02MmsUtils.isAirplaneOn(this)) {
            item.setEnabled(false);
            Xlog.d(TAG, "Menu sim sms entry is disabled");
        } else {
            item.setEnabled(true);
            Xlog.d(TAG, "Menu sim sms entry is enabled");
        }
    }

    public boolean onOptionsItemSelected(MenuItem item){
        if(item.getItemId() == mMenuSimSms){
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                List<SimInfoRecord> listSimInfo = SimInfoManager.getInsertedSimInfoList(this);
                if (listSimInfo.size() > 1) {
                    Intent simSmsIntent = new Intent(ACTION_VIEW_SELECT_CARD);
                    simSmsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    simSmsIntent.putExtra("preference", SMS_MANAGE_SIM_MESSAGES);
                    simSmsIntent.putExtra("preferenceTitleIndex", TITLE_INDEX_STRING_MANAGE_SIM_MESSAGE);
                    startActivity(simSmsIntent);
                } else if (listSimInfo.size() == 1) {
                    Intent simSmsIntent = new Intent(ACTION_VIEW_SIM_MESSAGES);
                    simSmsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    simSmsIntent.putExtra("SlotId", listSimInfo.get(0).mSimSlotId);
                    startActivity(simSmsIntent);
                }
            } else {
                Intent it = new Intent(ACTION_VIEW_SIM_MESSAGES);
                // this flag is needed. unless use host activity's context.
                it.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(it);
            }
        } else {
            return false;
        }
        return true;
    }
}

