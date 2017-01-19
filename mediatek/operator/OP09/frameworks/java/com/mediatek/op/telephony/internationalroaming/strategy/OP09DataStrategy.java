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

package com.mediatek.op.telephony.internationalroaming.strategy;

import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Telephony;
import android.text.TextUtils;
import android.view.WindowManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.ITelephonyProviderExt;
import com.mediatek.common.telephony.internationalroaming.IInternationalRoamingController;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;

import com.mediatek.internal.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;

// TODO: there are many database operations in this strategy, consider to move then to background thread.
public class OP09DataStrategy extends DefaultDataStrategy {
    private static final String TAG = "[OP09DataStrategy]";

    private static final int NUMERIC_MCC_LEN = 3;
    private static final int NUMERIC_MNC_LEN = 2;
    private static final String CT_WAP_PROXY = "10.0.0.200";
    private static final String CT_WAP_PORT = "80";

    private static final String CT_NET = "ctnet";
    private static final String CT_WAP = "ctwap";
    private static final String CT_NW_03 = "46003";
    private static final String CT_NW_12 = "46012";
    private static final String CT_NW_13 = "46013";
    private static final String CT_MCC = "460";
    private static final String MACAO_MCC_MNC = "45502";

    private static final String VODAFONE = "20404";
    private static final String VODAFONE_MCC = "204";
    private static final String VODAFONE_MNC = "04";

    private static final String SELECTION_QUERY_VODAFONE_APN = "apn = 'ctnet' "
            + "and mcc = '204' and type = 'default'";
    private static final String SELECTION_QUERY_CT_MAINLAND_APN = "apn = 'ctwap' "
            + "and mcc = '460' and type like '%default%' and proxy = '10.0.0.200' "
            + "and sourcetype <> 1";
    private static final String SELECTION_QUERY_CT_MAINLAND_MACAO_APN = "apn = 'ctwap' "
            + "and (mcc = '460' or mcc = '455') and type like '%default%' and sourcetype <> 1";

    private static final Uri PREFERAPN_NO_UPDATE_URI_SIM1 = Uri
            .parse("content://telephony/carriers_sim1/preferapn_no_update");
    private static final Uri PREFERAPN_NO_UPDATE_URI_SIM2 = Uri
            .parse("content://telephony/carriers_sim2/preferapn_no_update");

    private boolean mWapApnProxyExist;
    private ITelephonyProviderExt mTelephonyProviderExt;

    public OP09DataStrategy(IInternationalRoamingController controller, Context context,
            Phone dualModePhone, Phone gsmPhone) {
        super(controller, context, dualModePhone, gsmPhone);

        mTelephonyProviderExt = MediatekClassFactory.createInstance(ITelephonyProviderExt.class,
                context);
        mWapApnProxyExist = wapApnProxyExists();
    }

    @Override
    public void onRegisterRoamingNetwork(String plmn) {
        super.onRegisterRoamingNetwork(plmn);

        setPreferedApn();
        final int slotId = getDefaultDataSlotId();
        final String lastMcc = mController.getLastNetworkMcc();
        logd("onRegisterRoamingNetwork: plmn = " + plmn + ", lastMcc = " + lastMcc + ",slotId = " + slotId);

        // Only disable roaming data if the MCC changed, roaming between
        // different nation.
        if (((TextUtils.isEmpty(lastMcc) || lastMcc.startsWith("000")) && !plmn.startsWith(CT_MCC))
                || (!TextUtils.isEmpty(plmn) && !TextUtils.isEmpty(lastMcc)
                        && !plmn.startsWith(lastMcc) && lastMcc.startsWith(CT_MCC))) {
            setFirstRoamingFlag(true);
            if (slotId == PhoneFactory.getExternalModemSlot()) {
                // M: here, the data connection is in CT SIM card.
                popupAlertDialog();
                setFirstRoamingFlag(false);
            }
        }
    }

    @Override
    public void onRegisterHomeNetwork(String plmn) {
        super.onRegisterHomeNetwork(plmn);

        int slotId = getDefaultDataSlotId();
        logd("onRegisterHomeNetwork: plmn = " + plmn + ", slotId =" + slotId);
        if (slotId == PhoneFactory.getExternalModemSlot()) {
            setFirstRoamingFlag(false);
            restoreWapApnProxyIfNeeded();
        }
    }

    @Override
    public long insertGsmApnForDualModeCard() {
        // for sim1's dualmode part-gsm international card
        long checkId = insertIrGsmApn(Telephony.Carriers.SIM2Carriers.CONTENT_URI);
        logd("insertGsmApnForDualModeCard: checkId = " + checkId);
        return checkId;
    }

    @Override
    public void dispose() {

    }

    private long insertIrGsmApn(Uri geminiUri) {
        long apnId = -1;
        Cursor cursor = null;
        
        try {
            cursor = mContext.getContentResolver().query(geminiUri, null,
                    SELECTION_QUERY_VODAFONE_APN, null, null);
            if (cursor != null) {
                if (cursor.getCount() > 0 && cursor.moveToFirst()) {
                    apnId = cursor.getLong(cursor.getColumnIndex(Telephony.Carriers._ID));
                }
                logd("find apn count: " + cursor.getCount() + ", search apnId=" + apnId);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        if (apnId == -1) {
            ContentValues values = new ContentValues();
            String apnName = mContext.getResources().getString(R.string.ct_net_name);
            values.put(Telephony.Carriers.NAME, apnName);
            values.put(Telephony.Carriers.NUMERIC, VODAFONE);
            values.put(Telephony.Carriers.MCC, VODAFONE_MCC);
            values.put(Telephony.Carriers.MNC, VODAFONE_MNC);
            values.put(Telephony.Carriers.APN, CT_NET);
            values.put(Telephony.Carriers.TYPE, "default");
            values.put(Telephony.Carriers.SOURCE_TYPE, 0);
            values.put(Telephony.Carriers.CARRIER_ENABLED, true);
            values.put(Telephony.Carriers.BEARER, 0);

            ContentResolver resolver = mContext.getContentResolver();
            Uri insertResult = resolver.insert(geminiUri, values);
            apnId = ContentUris.parseId(insertResult);

            values = new ContentValues();
            apnName = mContext.getResources().getString(R.string.ct_wap_name);
            values.put(Telephony.Carriers.NAME, apnName);
            values.put(Telephony.Carriers.NUMERIC, VODAFONE);
            values.put(Telephony.Carriers.MCC, VODAFONE_MCC);
            values.put(Telephony.Carriers.MNC, VODAFONE_MNC);
            values.put(Telephony.Carriers.APN, CT_WAP);
            values.put(Telephony.Carriers.TYPE, "default,mms");
            values.put(Telephony.Carriers.MMSPROXY, CT_WAP_PROXY);
            values.put(Telephony.Carriers.MMSPORT, CT_WAP_PORT);
            values.put(Telephony.Carriers.MMSC, "http://mmsc.vnet.mobi");
            values.put(Telephony.Carriers.SOURCE_TYPE, 0);
            values.put(Telephony.Carriers.CARRIER_ENABLED, true);
            values.put(Telephony.Carriers.BEARER, 0);

            resolver.insert(geminiUri, values);
        }
        logd("insertIrGsmApn: apnId = " + apnId);
        return apnId;
    }

    /**
     * Check if the proxy of wap APN exists in APN database.
     * 
     * @return True if the wap APN exists, or else false.
     */
    private boolean wapApnProxyExists() {
        boolean ret = false;
        Cursor cursor = null;
        final int externalModemSlot = PhoneFactory.getExternalModemSlot();
        Uri uri = Telephony.Carriers.SIM1Carriers.CONTENT_URI;
        if (externalModemSlot == PhoneConstants.GEMINI_SIM_2) {
            uri = Telephony.Carriers.SIM2Carriers.CONTENT_URI;
        }

        try {
            cursor = mContext.getContentResolver().query(uri, null,
                    SELECTION_QUERY_CT_MAINLAND_APN, null, null);
            if (cursor != null && cursor.getCount() > 0) {
                ret = true;
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("wapApnProxyExists: ret = " + ret);
        return ret;
    }

    private void resetWapApnProxyIfNeeded() {
        logd("resetWapApnProxyIfNeeded: mWapApnProxyExist = " + mWapApnProxyExist);
        if (mWapApnProxyExist) {
            setWapApnForSpecialCase("", "");
            mWapApnProxyExist = false;
        }
    }

    private void restoreWapApnProxyIfNeeded() {
        logd("restoreWapApnProxyIfNeeded: mWapApnProxyExist = " + mWapApnProxyExist);
        if (!mWapApnProxyExist) {
            setWapApnForSpecialCase(CT_WAP_PROXY, CT_WAP_PORT);
            mWapApnProxyExist = true;
        }
    }

    private void setWapApnForSpecialCase(String proxy, String port) {
        logd("setWapApnForSpecialCase: proxy = " + proxy + ", port = " + port);
        ContentResolver updateResolver = mContext.getContentResolver();
        ContentValues updateValues = new ContentValues(2);
        updateValues.put(Telephony.Carriers.PROXY, proxy);
        updateValues.put(Telephony.Carriers.PORT, port);

        final int externalModemSlot = PhoneFactory.getExternalModemSlot();
        if (externalModemSlot == PhoneConstants.GEMINI_SIM_1) {
            updateResolver.update(Telephony.Carriers.SIM1Carriers.CONTENT_URI, updateValues,
                    SELECTION_QUERY_CT_MAINLAND_MACAO_APN, null);
        } else {
            updateResolver.update(Telephony.Carriers.SIM2Carriers.CONTENT_URI, updateValues,
                    SELECTION_QUERY_CT_MAINLAND_MACAO_APN, null);
        }
    }

    private int getDefaultDataSlotId() {
        // TODO: whether we need to check if I-card slot imsi is ready.
        if (!mIsICardImsiReady) {
            logd("Imsi is not ready");
            return -1;
        }

        boolean retVal = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.MOBILE_DATA, 1) == 1;
        logd("getDefaultDataSlotId, MOBILE_DATA, returning " + retVal);

        int slotId = -1;
        if (retVal) {
             slotId = Settings.System.getInt(mContext.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SETTING,
                Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
        }
        logd("Data defuault slotId:" + slotId);
        return slotId;
    }

    private String getPlmnFromImsiIfNeeded(String plmn) {
        if (plmn == null || plmn.equals("") || plmn.equals("00000")) {
            logd("Telephony properties are not ready yet, phoneType="
                    + mDualModePhone.getPhoneType());

            if (mDualModePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                plmn = mDualPhoneCdmaImsi;
            } else if (mDualModePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                plmn = mDualPhoneGsmImsi;
            } else {
                plmn = mOpNumeric;
            }
        }
        logd("plmnNumeric:" + plmn);
        return plmn;
    }

    // Set ctnet to default apn.
    private boolean setPreferedApn() {
        long apnId = -1;
        Uri geminiUri;
        Uri preferApnUri;
        Cursor cursor = null;
        String plmnNumeric = "00000";
        String selection = null;
        final int externalModemSlot = PhoneFactory.getExternalModemSlot();
        logd("setPreferedApn: externalModemSlot = " + externalModemSlot);

        if (externalModemSlot == PhoneConstants.GEMINI_SIM_1) {
            plmnNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC);
            geminiUri = Telephony.Carriers.SIM1Carriers.CONTENT_URI;
            preferApnUri = PREFERAPN_NO_UPDATE_URI_SIM1;
        } else {
            plmnNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2);
            geminiUri = Telephony.Carriers.SIM2Carriers.CONTENT_URI;
            preferApnUri = PREFERAPN_NO_UPDATE_URI_SIM2;
        }

        plmnNumeric = getPlmnFromImsiIfNeeded(plmnNumeric);

        if (plmnNumeric.startsWith(CT_NW_03) || 
            plmnNumeric.startsWith(CT_NW_12) || 
            plmnNumeric.startsWith(CT_NW_13)) {
            // China CT card
            final String mncString = plmnNumeric.substring(NUMERIC_MCC_LEN, NUMERIC_MCC_LEN
                    + NUMERIC_MNC_LEN);
            logd("setPreferedApn,  mncString = " + mncString);
            selection = "apn = 'ctnet' and mcc = '460' and mnc = '" + mncString
                    + "' and type like '%default%'";
        } else if (plmnNumeric.startsWith(MACAO_MCC_MNC)) {
            // Maco CT card
            selection = "apn = 'ctnet' and mcc = '455' and type like '%default%'";
        } else if (plmnNumeric.startsWith(VODAFONE) || // Dualmode - GSM port
                mController.getDualModePhoneCardType() == InternationalRoamingConstants.CARD_TYPE_DUAL_MODE) {
            apnId = insertIrGsmApn(geminiUri);
        } else {
            logd("setPreferedApn,  but we don't care the plmn = " + plmnNumeric);
        }

        ContentResolver resolver = mContext.getContentResolver();
        if (selection != null) {
            try {
                cursor = resolver.query(geminiUri, null, selection, null, null);
                if (cursor != null) {
                    if (cursor.getCount() > 0 && cursor.moveToFirst()) {
                        apnId = cursor.getLong(cursor.getColumnIndex(Telephony.Carriers._ID));
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        logd("setPreferedApn, apnId = " + apnId);

        // set prefered apn
        if (apnId > 0) {
            resetWapApnProxyIfNeeded();
            int delResult = resolver.delete(preferApnUri, null, null);
            logd("setPreferredApn: delete:" + delResult);

            ContentValues values = new ContentValues(1);
            values.put("apn_id", apnId);
            Uri result = resolver.insert(preferApnUri, values);
            return true;
        }

        return false;
    }

    private void disableDataForRoaming(int slotId) {
        // if CDMA roaming and default data is slot1, disable data
        logd("Disable data when first roaming");
        ConnectivityManager connMgr = (ConnectivityManager) mContext
                .getSystemService(mContext.CONNECTIVITY_SERVICE);
        connMgr.setMobileDataEnabled(false);

        if (slotId >= PhoneConstants.GEMINI_SIM_1 && slotId <= PhoneConstants.GEMINI_SIM_2) {
            try {
                TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
                if (telephonyManagerEx != null) {
                    telephonyManagerEx.setDataRoamingEnabledGemini(false, slotId);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            SimInfoRecord siminfo = SimInfoManager.getSimInfoBySlot(mContext, slotId);
            if (siminfo != null) {
                SimInfoManager.setDataRoaming(mContext, SimInfoManager.DATA_ROAMING_DISABLE, siminfo.mSimInfoId);
            } else {
                logd("disableDataForRoaming, we can't get siminfo by getSimInfoBySlot function");
            }
        }
    }

    private void setFirstRoamingFlag(boolean roaming) {
        logd("setFirstRoamingFlagxx, roaming=" + roaming);
        int roamingFlag = roaming ? 1 : 0;
        Settings.System.putInt(mContext.getContentResolver(),
                Settings.System.DATA_CONNECTION_FIRST_ROAMING, roamingFlag);
    }

    private void popupAlertDialog() {
        logd("popupAlertDialog for data enabled on roaming network.");
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        //builder.setTitle(R.string.alert_prompt);
        builder.setMessage(R.string.roaming_message);
        builder.setPositiveButton(R.string.ok, null);
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        dialog.show();
    } 

    protected String getLogTag() {
        return TAG;
    }
}
