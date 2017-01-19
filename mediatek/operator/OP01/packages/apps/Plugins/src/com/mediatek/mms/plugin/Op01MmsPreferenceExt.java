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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;

import com.google.android.mms.InvalidHeaderValueException;
import com.google.android.mms.pdu.AcknowledgeInd;
import com.google.android.mms.pdu.NotifyRespInd;
import com.google.android.mms.pdu.PduHeaders;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.encapsulation.EncapsulationConstant;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsPreferenceExt;
import com.mediatek.mms.ext.IGeneralPreferenceHost;
import com.mediatek.mms.ext.IMmsPreferenceExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;

import android.telephony.SmsManager;
import android.preference.Preference.OnPreferenceClickListener;
import android.provider.Telephony.Mms;
import android.telephony.SmsManager;
import android.database.Cursor;
import android.util.Log;

import java.util.List;

public class Op01MmsPreferenceExt extends DefaultMmsPreferenceExt {
    private static final String TAG = "Mms/Op01MmsPreferenceExt";
    private static final boolean MTK_GEMINI_SUPPORT = EncapsulationConstant.USE_MTK_PLATFORM ? FeatureOption.MTK_GEMINI_SUPPORT
            : false;
    private static final String PREFERENCE_NAME = "preference";
    private static final String PREFERENCE_NAME_SMS_VALIDITY_PERIOD = "sms_validity_period";
    private static final String SMS_VALIDITY_PERIOD_PREFERENCE_KEY = "pref_key_sms_validity_period";
    private static final String MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY = "pref_key_mms_enable_to_send_delivery_reports";
    public static final String SMS_FORWARD_WITH_SENDER = "pref_key_forward_with_sender";


    public Op01MmsPreferenceExt(Context context) {
        super(context);
    }
/*
    public void configMmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
        Preference delivery_report_preference;

    }
*/
    public void configSmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
    	  Xlog.d(TAG, "configSmsPreference");
        addSmsValidityPeriodPreference(hostActivity, pC, simCount);
        addForwardWithSenderPreference(hostActivity, pC);
    }

    public void configSmsPreferenceEditorWhenRestore(Activity hostActivity, SharedPreferences.Editor editor) {
        Xlog.d(TAG, "configSmsPreferenceEditorWhenRestore");
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
         // if not mtk platform, no this feature
            return;
        }
        if (MTK_GEMINI_SUPPORT) {
            List<SimInfoManager.SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(hostActivity);
            if (simList != null) {
                int simCount = simList.size();
                for (int index = 0; index < simCount; index ++) {
                    int slotId = simList.get(index).mSimSlotId;
                    String key = getSmsValidityKeyBySlotId(slotId);
                    editor.remove(key);
                }
            }
        }

        Log.d(TAG,"set SMS_FORWARD_WITH_SENDER true");
        editor.putBoolean(SMS_FORWARD_WITH_SENDER, true);
    }

    public void configMmsPreferenceEditorWhenRestore(Activity hostActivity, SharedPreferences.Editor editor) {
        Xlog.d(TAG, "configMmsPreferenceEditorWhenRestore");
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
         // if not mtk platform, no this feature
            return;
        }
        if (MTK_GEMINI_SUPPORT) {
            List<SimInfoManager.SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(hostActivity);
            if (simList != null) {
                int simCount = simList.size();
                for (int index = 0; index < simCount; index ++) {
                    Log.d(TAG,"set simId_MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY false");
                    Long simId = simList.get(index).mSimInfoId;
                    editor.putBoolean(Long.toString(simId) + "_" + MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
                }
            }
        } else {
            Log.d(TAG,"set MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY false");
            editor.putBoolean(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
        }
    }

    public Bundle getSmsValidityParamBundleWhenSend(Context context, int slotId) {
        Xlog.d(TAG, "getSmsValidityParamBundleWhenSend, slotId = " + slotId);
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
            // if not mtk platform, no this feature
            return null;
        }
        if (MTK_GEMINI_SUPPORT) {
            Bundle extra = new Bundle();
            int valid = getSmsValiditybySlotId(context, slotId);
            extra.putInt(SmsManager.EXTRA_PARAMS_VALIDITY_PERIOD, valid);
            return extra;
        } else {
            return null;
        }
    }

    // add sms validity period feature
    private void addSmsValidityPeriodPreference(final Activity hostActivity, PreferenceCategory pC, int simCount) {
        Preference sms_validate_preference;
        Xlog.d(TAG, "addSmsValidityPeriodPreference: simCount = " + simCount);
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
         // if not mtk platform, no this feature
            return;
        }
        if (MTK_GEMINI_SUPPORT) {
            final List<SimInfoManager.SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(hostActivity);
            Preference p = new Preference(hostActivity);
            p.setTitle(getString(R.string.sms_validity_period));
            p.setSummary(getString(R.string.sms_validity_period));
            int smsValidityOrder = 4;
            p.setOrder(smsValidityOrder);
            int count = pC.getPreferenceCount();
            //reset order
            for (int index = 0; index < count; index ++) {
                Preference preference = pC.getPreference(index);
                if (preference != null) {
                    int pOrder = preference.getOrder();
                    if (pOrder >= smsValidityOrder) {
                        preference.setOrder(pOrder +1);
                    }
                }
            }
            if (simCount > 1) {
                p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        Xlog.d(TAG, "addSmsValidityPeriodPreference: onPreferenceClick, preference = " + preference);
                        Intent intent = new Intent();
                        intent.setClassName(hostActivity, "com.android.mms.ui.SelectCardPreferenceActivity");
                        intent.putExtra(PREFERENCE_NAME, PREFERENCE_NAME_SMS_VALIDITY_PERIOD);
                        hostActivity.startActivity(intent);
                        return true;
                    }
                });
            }else{
                if (simCount == 0) {
                    p.setEnabled(false);
                } else {
                    p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                        @Override
                        public boolean onPreferenceClick(Preference preference) {
                            Xlog.d(TAG, "addSmsValidityPeriodPreference: onPreferenceClick, preference = " + preference);
                            int slotId = simList.get(0).mSimSlotId;
                            showSmsValidityPeriodDialog(hostActivity, slotId);
                            return true;
                        }
                    });
                }
            }
            pC.addPreference(p);
        }
    }

    public void configSelectCardPreferenceTitle(Activity hostActivity) {
        Intent intent = hostActivity.getIntent();
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null && preferenceName.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            hostActivity.setTitle(getString(R.string.sms_validity_period));
        }
    }   
    public void configSelectCardPreference(Activity hostActivity, PreferenceCategory pC, Intent intent) {
        if (intent == null) {
            throw new RuntimeException("configSelectCardPreference: intent cannot be null");
        }
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null && preferenceName.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            hostActivity.setTitle(getString(R.string.sms_validity_period));
        }
    }

    public void handleSelectCardPreferenceTreeClick(Activity hostActivity, PreferenceScreen preferenceScreen, Preference preference, final int slotId) {
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
            // if not mtk platform, no this feature
           return;
        }
        Intent intent = hostActivity.getIntent();
        String intentKey = intent.getStringExtra(PREFERENCE_NAME);
        if (intentKey != null && intentKey.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            showSmsValidityPeriodDialog(hostActivity, slotId);
        }
    }

    private String getSmsValidityKeyBySlotId(int slotId) {
        return Long.toString(slotId) + "_" + SMS_VALIDITY_PERIOD_PREFERENCE_KEY;
    }
    private void showSmsValidityPeriodDialog(Context context, int slotId) {
        final CharSequence[] entries = getResources().getTextArray(R.array.sms_validity_peroid_entries);
        final String validityKey = getSmsValidityKeyBySlotId(slotId);
        final SharedPreferences pref= PreferenceManager.getDefaultSharedPreferences(context);
        int index = pref.getInt(validityKey, 0);
        AlertDialog.Builder builder = new AlertDialog.Builder(context)
            .setTitle(getResources().getText(R.string.sms_validity_period))
            .setNegativeButton(android.R.string.cancel, null)
            .setSingleChoiceItems(entries, index, new DialogInterface.OnClickListener() {
             public void onClick(DialogInterface dialog, int item) {
                 Xlog.d(TAG, "showSmsValidityPeriodDialog->onClick: item =  " +item);
                SharedPreferences.Editor editor = pref.edit();
                editor.putInt(validityKey, item);
                editor.commit();
                dialog.dismiss();
            }
        });
        builder.show();
        builder.create();
    }

    private int getSmsValiditybySlotId(Context context, int slotId) {
        final int[] validities = {SmsManager.VALIDITY_PERIOD_NO_DURATION,
                                    SmsManager.VALIDITY_PERIOD_ONE_HOUR,
                                    SmsManager.VALIDITY_PERIOD_SIX_HOURS,
                                    SmsManager.VALIDITY_PERIOD_TWELVE_HOURS,
                                    SmsManager.VALIDITY_PERIOD_ONE_DAY,
                                    SmsManager.VALIDITY_PERIOD_MAX_DURATION};
        SharedPreferences spref = PreferenceManager.getDefaultSharedPreferences(context);
        final String validityKey = getSmsValidityKeyBySlotId(slotId);
        int index = spref.getInt(validityKey, 0);
        if (index > validities.length) {
            index = 0;
        }
        Xlog.d(TAG, "getSmsValiditybySlotId: result index = " + index + ", validity ="+validities[index]);
        return validities[index];
    }
    
    // Add for forward sms with append sender
    private void addForwardWithSenderPreference(Context context, PreferenceCategory smsCategory) {
        Log.d(TAG,"Call addForwardWithSenderPref");
        CheckBoxPreference sp = new CheckBoxPreference(context);
        sp.setKey(SMS_FORWARD_WITH_SENDER);
        sp.setTitle(getString(R.string.sms_forward_setting));
        sp.setSummary(getString(R.string.sms_forward_setting_summary));
        smsCategory.addPreference(sp);
    }

    public String formatSmsBody(Context context, String smsBody, String nameAndNumber, int boxId) {
        Log.d(TAG,"Call formatSmsBody 1");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean smsForwardWithSender = prefs.getBoolean(SMS_FORWARD_WITH_SENDER, true);
        Log.d(TAG, "forwardMessage(): SMS Forward With Sender ?= " + smsForwardWithSender);
        if (smsForwardWithSender) {
//          if (boxId == Mms.MESSAGE_BOX_INBOX) {
//              smsBody += "\n" + getString(R.string.forward_from);
//              smsBody += nameAndNumber;
//          }
            if (boxId == Mms.MESSAGE_BOX_INBOX) {
                nameAndNumber = nameAndNumber + ":\n";
                nameAndNumber += smsBody;
            } else {
                nameAndNumber = getString(R.string.forward_byself) + ":\n";
                nameAndNumber += smsBody;
            }
            return nameAndNumber;
        }
        return smsBody;
    }

    public String formatSmsBody(Context context, String smsBody, String nameAndNumber, Cursor cursor) {
        Log.d(TAG,"Call formatSmsBody 2");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean smsForwardWithSender = prefs.getBoolean(SMS_FORWARD_WITH_SENDER, true);
        Log.d(TAG, "forwardMessage(): SMS Forward With Sender ?= " + smsForwardWithSender);
        
        if (smsForwardWithSender) {
            long mStatus = cursor.getLong(cursor.getColumnIndexOrThrow("status"));
            if (mStatus == SmsManager.STATUS_ON_ICC_READ
                            || mStatus == SmsManager.STATUS_ON_ICC_UNREAD) {
                   // it is a inbox sms
                   Log.d(TAG, "It is a inbox sms");
                   smsBody += "\n" + getString(R.string.forward_from);
                   smsBody += nameAndNumber;
            }
        }
        return smsBody;
    }

    public void configGeneralPreference(Activity hostActivity, PreferenceCategory pC) {
        Xlog.d(TAG, "configGeneralPreference");
        addStorageStatusPreference(hostActivity, pC);
    }

    private void addStorageStatusPreference(final Activity hostActivity, PreferenceCategory pC){
        Xlog.d(TAG, "addStorageStatusPreference");
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
         // if not mtk platform, no this feature
            return;
        }

        Preference p = new Preference(hostActivity);
        p.setTitle(getString(R.string.pref_title_storage_status));
        p.setSummary(getString(R.string.pref_title_storage_status));
        p.setOrder(0);
        int count = pC.getPreferenceCount();            
        //reset order           
        for (int index = 0; index < count; index ++) {
            Preference preference = pC.getPreference(index);
            if (preference != null) {
                int pOrder = preference.getOrder();
                if (pOrder >= 0) {
                    preference.setOrder(pOrder +1);
                }
            }
        }
        p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    Xlog.d(TAG, "addStorageStatusPreference: onPreferenceClick, preference = " + preference);
                    showStorageStatusDialog(hostActivity);
                    return true;
                }
            });
        pC.addPreference(p);
    }

    private void showStorageStatusDialog(Activity hostActivity) {
            final String memoryStatus = mGeneralPreferenceHost.getStorageStatus(hostActivity);
            String status = getResources().getString(R.string.pref_title_storage_status);
            Drawable icon = getResources().getDrawable(R.drawable.ic_dialog_info_holo_light);
            new AlertDialog.Builder(hostActivity)
                    .setTitle(status)
                    .setIcon(icon)
                    .setMessage(memoryStatus)
                    .setPositiveButton(android.R.string.ok, null)
                    .setCancelable(true)
                    .show();
    }

    private IGeneralPreferenceHost mGeneralPreferenceHost;
    
    public void setGeneralPreferenceHost(IGeneralPreferenceHost host){
        mGeneralPreferenceHost = host;
    }

    public void configMmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
        Xlog.d(TAG, "configMmsPreference");
        addMmsDeliveryReportPreference(hostActivity, pC, simCount);
    }

    private void addMmsDeliveryReportPreference(final Activity hostActivity, PreferenceCategory pC, int simCount){
        Xlog.d(TAG, "addMmsDeliveryReportPreference: simCount = " + simCount);
        if (!EncapsulationConstant.USE_MTK_PLATFORM) {
         // if not mtk platform, no this feature
            return;
        }

        CheckBoxPreference cp = new CheckBoxPreference(hostActivity);
        cp.setTitle(getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        cp.setSummary(getString(R.string.pref_summary_mms_enable_to_send_delivery_reports));
        cp.setOrder(2);

        Preference p = new Preference(hostActivity);
        p.setKey(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
        p.setTitle(getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        p.setSummary(getString(R.string.pref_summary_mms_enable_to_send_delivery_reports));
        p.setOrder(2);

        if (MTK_GEMINI_SUPPORT) {
            final List<SimInfoManager.SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(hostActivity);
            final SharedPreferences pref= PreferenceManager.getDefaultSharedPreferences(hostActivity);

            if (simCount > 1) {
                p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        Xlog.d(TAG, "addMmsDeliveryReportPreference: onPreferenceClick, preference = " + preference);
                        Intent intent = new Intent();
                        intent.setClassName(hostActivity, "com.android.mms.ui.MultiSimPreferenceActivity");
                        intent.putExtra(PREFERENCE_NAME, MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
                        hostActivity.startActivity(intent);
                        return true;
                    }
                });
                pC.addPreference(p);
            }else{
                if (simCount == 0) {
                    cp.setEnabled(false);
                } else {
                    Long simId = simList.get(0).mSimInfoId;
                    cp.setKey(Long.toString(simId) + "_" + MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
                    
                    // get the stored value
                    SharedPreferences sp = getSharedPreferences("com.android.mms_preferences", MODE_WORLD_READABLE);
                    cp.setChecked(sp.getBoolean(cp.getKey(), false));
                }
                pC.addPreference(cp);
            }
        }else {
            cp.setKey(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
            pC.addPreference(cp);
        }
        int count = pC.getPreferenceCount();            
        //reset order           
        for (int index = 0; index < count; index ++) {
            Preference preference = pC.getPreference(index);
            if (preference != null) {
                int pOrder = preference.getOrder();
                if (pOrder >= 2) {
                    preference.setOrder(pOrder +1);
                }
            }
        }
    }


    public boolean configMmsPreferenceState(Activity hostActivity, String preference,long simId, CheckBoxPreference cp) {
        Xlog.d(TAG, "configMmsPreferenceState[hostActivity]" + hostActivity);
        Xlog.d(TAG, "configMmsPreferenceState[preference]" + preference);
        Xlog.d(TAG, "configMmsPreferenceState[simId]" + simId);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostActivity);
        if (preference != null && preference.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            boolean checked = prefs.getBoolean(Long.toString(simId) + "_" + MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
            cp.setChecked(checked);
            return true;
        }
        return false;
    }

    public void configMultiSimPreferenceTitle(Activity hostActivity) {
        Intent intent = hostActivity.getIntent();
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null && preferenceName.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            hostActivity.setTitle(getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        }

    } 


    public boolean setMmsPreferenceState(Activity hostActivity, String preference, long simId, boolean checked) {
        Xlog.d(TAG, "setMmsPreferenceState[hostActivity]" +hostActivity);
        Xlog.d(TAG, "setMmsPreferenceState[preference]" +preference);
        Xlog.d(TAG, "setMmsPreferenceState[simId]" +simId);
        Xlog.d(TAG, "setMmsPreferenceState[checked]" +checked);
        SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(hostActivity).edit();
        if (preference.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            editor.putBoolean(Long.toString(simId) + "_" + MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, checked);
            editor.apply();
            return true;
        }
        return false;
    }


    private boolean isEnableSendDeliveryReport(Context mContext, int mSimId) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        if (MTK_GEMINI_SUPPORT) {
            return prefs.getBoolean(Integer.toString(mSimId)+ "_" + 
                    MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY,
                    false);
        } else {
            return prefs.getBoolean(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
        }
    }
        
    public void setAcknowledgeDeliveryReport(Context mContext, int mSimId, AcknowledgeInd acknowledgeInd) {
        // X-Mms-Report-Allowed Optional
        boolean reportAllowed = isEnableSendDeliveryReport(mContext, mSimId);
        Xlog.d(TAG, "reportAllowed: " + reportAllowed);
        
        try {
            acknowledgeInd.setReportAllowed(reportAllowed ? PduHeaders.VALUE_YES : PduHeaders.VALUE_NO);
        } catch(InvalidHeaderValueException ihve) {
            Xlog.e(TAG, "acknowledgeInd.setReportAllowed Failed !!");
        }
    }

    public void setNotifyRespDeliveryReport(Context mContext, int mSimId, NotifyRespInd notifyRespInd) {
       // X-Mms-Report-Allowed Optional
       boolean reportAllowed = isEnableSendDeliveryReport(mContext, mSimId);
       Xlog.d(TAG, "reportAllowed: " + reportAllowed);
       
       try {
           notifyRespInd.setReportAllowed(reportAllowed ? PduHeaders.VALUE_YES : PduHeaders.VALUE_NO);
       } catch (InvalidHeaderValueException ihve) {
           // do nothing here
           Xlog.e(TAG, "notifyRespInd.setReportAllowed Failed !!");
       }
    }




   // public void setDefaultAppendSender(SharedPreferences.Editor editor) {
   //     Log.d(TAG,"Call setDefaultAppendSender");
   //     editor.putBoolean(SMS_FORWARD_WITH_SENDER, true);
   //    return;
   // }
}

