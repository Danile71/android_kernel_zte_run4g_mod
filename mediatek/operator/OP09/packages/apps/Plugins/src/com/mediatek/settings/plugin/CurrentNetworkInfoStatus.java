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

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.provider.Telephony.SIMInfo;
import android.telephony.PhoneNumberUtils;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.text.TextUtils;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.cdma.CDMAPhone;
import com.android.internal.telephony.gemini.GeminiNetworkSubUtil;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.gsm.GSMPhone;

import com.mediatek.CellConnService.CellConnMgr;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.List;

/**
 * CT customization, to show current SIM card's network related information
 */
public class CurrentNetworkInfoStatus extends PreferenceActivity {

    private static final String KEY_OPERATOR_NAME = "operator_name";
    private static final String KEY_CURRENT_OPERATORS_MCCMNC = "current_operators_mccmnc";
    // For slot 1 only
    private static final String KEY_CURRENT_SIDNID = "current_sidnid";
    // For slot 1 only
    private static final String KEY_CURRENT_CELLID = "current_cellid";
    private static final String KEY_NETWORK_TYPE = "network_type";
    private static final String KEY_SIGNAL_STRENGTH = "signal_strength";
    // For slot 1 GSM mode only
    private static final String KEY_CT_SLOT1_GSM_PNL_VERSION = "ct_slot1_gsm_pnl_version";
    // For slot 1 and slot 2 GMS mode only
    private static final String KEY_SMS_CENTER = "sms_center";
    // For slot 1 only
    private static final String KEY_CT_PRL_VERSION = "ct_prl_version";
    // For slot 1 CDMA mode only
    private static final String KEY_CT_SLOT1_CDMA_PNL_VERSION = "ct_slot1_cdma_pnl_version";

    /// For getting the saved Cdma Prl version
    private static final String PRL_VERSION_KEY_NAME = "cdma.prl.version";

    private static final String TAG = "CurrentNetworkInfoStatus";

    private static final int EVENT_SIGNAL_STRENGTH_CHANGED = 200;
    private static final int EVENT_SERVICE_STATE_CHANGED = 300;

    /** For get oplmn version. */
    private static final int EVENT_GET_OPLMN_VERSION = 400;

    private static final int PLMN_NORMAL_LEN = 5;
    private static final int PLMN_SPECIAL_LEN = 6;
    private static final int MCC_LEN = 3;
    private static final int MNC_MAX_LEN = 3;

    private static final int RESUME_HANDLE_CELLCONN_DELAY_TIME = 500;

    private CDMAPhone mCdmaPhone;
    private GSMPhone mGsmPhone;
    private GeminiPhone mGeminiPhone;

    private SignalStrength mSignalStrength;
    private Preference mSignalStrengthPreference;

    private TelephonyManager mTelephonyManager;
    private TelephonyManagerEx mTelephonyManagerEx;
    // SimId, get from the intent extra
    private int mSimId = 0;
    private boolean mIsUnlockFollow = false;
    private boolean mIsShouldBeFinished = false;

    private int mServiceState;

    private CellConnMgr mCellMgr;

    private ScAsyncTask mScAyncTask;

    private String mUnknown;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case EVENT_GET_OPLMN_VERSION:
                ar = (AsyncResult)msg.obj;
                if (ar.exception != null) {
                    Xlog.d(TAG, "EVENT_GET_OPLMN_VERSION, ar.exception = " + ar.exception); 
                } else {
                    String oplmnVersion = (String)ar.result;
                    Xlog.d(TAG, "EVENT_GET_OPLMN_VERSION, oplmnVersion = " + oplmnVersion + " mCdmaPhone = " + mCdmaPhone);
                    if (mCdmaPhone != null) {
                        setSummaryText(KEY_CT_SLOT1_CDMA_PNL_VERSION, oplmnVersion);
                    } else {
                        setSummaryText(KEY_CT_SLOT1_GSM_PNL_VERSION, oplmnVersion);
                    }
                }
                break;
            default:
                break;
            }
        }
    };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean airplaneMode = intent.getBooleanExtra("state", false);
                if (airplaneMode) {
                    mCellMgr.handleCellConn(mSimId, CellConnMgr.REQUEST_TYPE_SIMLOCK);
                }
            } else if (Intent.ACTION_DUAL_SIM_MODE_CHANGED.equals(action)) {
                int dualMode = intent.getIntExtra(Intent.EXTRA_DUAL_SIM_MODE, -1);
                if ((dualMode == GeminiNetworkSubUtil.MODE_FLIGHT_MODE)
                        || (dualMode != GeminiNetworkSubUtil.MODE_DUAL_SIM && dualMode != mSimId)) {
                    mCellMgr.handleCellConn(mSimId, CellConnMgr.REQUEST_TYPE_SIMLOCK);
                }
            }
        }
    };

    // unlock sim pin/ me lock
    private Runnable mServiceComplete = new Runnable() {
        public void run() {
            int nRet = mCellMgr.getResult();
            if (mCellMgr.RESULT_OK != nRet && mCellMgr.RESULT_STATE_NORMAL != nRet) {
                Xlog.d(TAG, "mCellMgr Result is not OK");
                mIsShouldBeFinished = true;
                CurrentNetworkInfoStatus.this.finish();
                return;
            }

            mIsUnlockFollow = false;
        }
    };

    // related to mobile network type and mobile network state
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            updateNetworkType();
        }

        @Override
        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            if (signalStrength.getMySimId() == mSimId) {
                mSignalStrength = signalStrength;
                Xlog.d(TAG, "onSignalStrengthsChanged, mSimId : " + mSimId + " mSignalStrength : " + mSignalStrength);
                updateSignalStrength();
            }
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            if (serviceState.getMySimId() == mSimId) {
                mServiceState = serviceState.getState();
                Xlog.d(TAG, "onServiceStateChanged, mSimId : " + mSimId + " mServiceState : " + mServiceState);
                updateServiceState(serviceState);
                updateSignalStrength();
                setMccMnc();
            }
        }
    };

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        mCellMgr = new CellConnMgr(mServiceComplete);
        mCellMgr.register(this);
        addPreferencesFromResource(R.xml.current_networkinfo_status);

        // get the correct simId according to the intent extra
        Intent it = getIntent();
        mSimId = it.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
        Xlog.d(TAG, "onCreate, mSimId is : " + mSimId);
        /*
         * M: simId == -1 is not supproted happen and usually it is caused by 3rd party app so to compatibilty add this catch
         */
        if (mSimId == -1) {
            mSimId = dealWith3AppLaunch();
        }

        mUnknown = getResources().getString(R.string.device_info_default);
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mTelephonyManagerEx = new TelephonyManagerEx(this);
        mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();

        // Note - missing in zaku build, be careful later...
        mSignalStrengthPreference = findPreference(KEY_SIGNAL_STRENGTH);

        initPreferences();
    }

    private void initPreferences() {
        int phoneType = mTelephonyManagerEx.getPhoneType(mSimId);
        Xlog.d(TAG, "initPreferences, slotId = " + mSimId + " phoneType = " + phoneType);

        Phone phone = mGeminiPhone.getPhonebyId(mSimId);
        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            mCdmaPhone = (CDMAPhone) (((PhoneProxy) phone).getActivePhone());
            removePreference(KEY_CT_SLOT1_GSM_PNL_VERSION);
            removePreference(KEY_SMS_CENTER);
        } else if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
            mGsmPhone = (GSMPhone) (((PhoneProxy) phone).getActivePhone());
            if (mSimId == PhoneFactory.getExternalModemSlot()) {
                removePreference(KEY_CURRENT_SIDNID);
                removePreference(KEY_CT_SLOT1_CDMA_PNL_VERSION);
            } else {
                removePreference(KEY_CURRENT_SIDNID);
                removePreference(KEY_CURRENT_CELLID);
                removePreference(KEY_CT_PRL_VERSION);
                removePreference(KEY_CT_SLOT1_CDMA_PNL_VERSION);
                removePreference(KEY_CT_SLOT1_GSM_PNL_VERSION);
            }
        }
    }

    private void removePreference(String key) {
        Preference preference = findPreference(key);
        if (preference != null) {
            getPreferenceScreen().removePreference(preference);
        }
    }

    private void updatePreferences() {
        setMccMnc();
        getSidNidPrlValue();
        getCellIdValue();
        getSmsServiceCenter();
        getOplmnVersion();
    }

    private void setMccMnc() {
        String numeric;
        switch (mSimId) {
        case PhoneConstants.GEMINI_SIM_1:
            numeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC, "-1");
            break;
        case PhoneConstants.GEMINI_SIM_2:
            numeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC_2, "-1");
            break;
        default:
            numeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC, "-1");
            break;
        }

        Xlog.d(TAG, "setMccMnc, numeric=" + numeric);
        if ((numeric.length() == PLMN_NORMAL_LEN) || (numeric.length() == PLMN_SPECIAL_LEN)) {
            String mcc = numeric.substring(0, MCC_LEN);
            String mnc = numeric.substring(MNC_MAX_LEN);
            setSummaryText(KEY_CURRENT_OPERATORS_MCCMNC, mcc + "," + mnc);
        }
    }

    private void getSidNidPrlValue() {
        if (mCdmaPhone != null) {
            setSummaryText(KEY_CURRENT_SIDNID, mCdmaPhone.getSid() + "," + mCdmaPhone.getNid());
        }
        // Get the saved Cdma Prl version and set text.
        final ContentResolver cr = mGeminiPhone.getContext().getContentResolver();
        String prlVersion = android.provider.Settings.System.getString(cr, PRL_VERSION_KEY_NAME);
        setSummaryText(KEY_CT_PRL_VERSION, prlVersion);
        Xlog.d(TAG, "getSidNidPrlValue: key = " + PRL_VERSION_KEY_NAME + ", prlVersion = " + prlVersion);
    }

    // Only for Slot 1
    private void getCellIdValue() {
        if (mSimId == PhoneFactory.getExternalModemSlot()) {
            if (mCdmaPhone != null) {
                CdmaCellLocation cellLocation = (CdmaCellLocation) mCdmaPhone.getCellLocation();
                setSummaryText(KEY_CURRENT_CELLID, Integer.toString(cellLocation.getBaseStationId()));
            } else if (mGsmPhone != null) {
                GsmCellLocation cellLocation = (GsmCellLocation) mGsmPhone.getCellLocation();
                setSummaryText(KEY_CURRENT_CELLID, Integer.toString(cellLocation.getCid()));
            }
        }
    }

    // Only for GSM mode
    private void getSmsServiceCenter() {
        if (mGsmPhone != null) {
            mScAyncTask = new ScAsyncTask();
            mScAyncTask.execute();
        }
    }

    // Only for slot 1
    private void getOplmnVersion() {
        Xlog.d(TAG, "getOplmnVersion: mSimId = " + mSimId + 
            " PhoneFactory.getExternalModemSlot() = " + PhoneFactory.getExternalModemSlot());
        if (mSimId == PhoneFactory.getExternalModemSlot()) {
            PhoneProxy phoneProxy = (PhoneProxy)mGeminiPhone.getPhonebyId(PhoneConstants.GEMINI_SIM_2);
            GSMPhone gsmPhone2 = (GSMPhone)phoneProxy.getActivePhone();
            Xlog.d(TAG, "getOplmnVersion: mSimId = " + mSimId + " gsmPhone2 = " + gsmPhone2);
            if (gsmPhone2 != null) {
                gsmPhone2.mCi.getOplmnVersion(mHandler.obtainMessage(EVENT_GET_OPLMN_VERSION));
            }
        }
    }

    @Override
    protected void onDestroy() {
        mCellMgr.unregister();
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mIsShouldBeFinished) {
            finish();
            return;
        }
        if (!mIsUnlockFollow) {
            mIsUnlockFollow = true;
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mCellMgr.handleCellConn(mSimId, CellConnMgr.REQUEST_TYPE_SIMLOCK);
                }
            }, RESUME_HANDLE_CELLCONN_DELAY_TIME);
        }
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
        // related to my phone number, may be null or empty
        /// M: LEGO refactory start @{
        String rawNumber = mTelephonyManagerEx.getLine1Number(mSimId);
        /// @}

        String formattedNumber = null;
        if (!TextUtils.isEmpty(rawNumber)) {
            formattedNumber = PhoneNumberUtils.formatNumber(rawNumber);
        }

        // after registerIntent, it will receive the message, so do not need to update signalStrength and service state
        ServiceState serviceState = getPhoneBySlotId(mGeminiPhone, mSimId).getServiceState();
        updateServiceState(serviceState);
        mServiceState = serviceState.getState();
        mSignalStrength = getPhoneBySlotId(mGeminiPhone, mSimId).getSignalStrength();
        updateSignalStrength();
        mTelephonyManagerEx.listen(mPhoneStateListener, PhoneStateListener.LISTEN_DATA_CONNECTION_STATE
                | PhoneStateListener.LISTEN_SIGNAL_STRENGTHS | PhoneStateListener.LISTEN_SERVICE_STATE, mSimId);

        updatePreferences();
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mScAyncTask != null) {
            mScAyncTask.cancel(true);
        }        
        if (mIsShouldBeFinished) {
            finish();
            return;
        }
        unregisterReceiver(mReceiver);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
    }

    private void setSummaryText(String preference, String text) {
        if (TextUtils.isEmpty(text)) {
            text = this.getResources().getString(R.string.device_info_default);
        }
        // some preferences may be missing
        Preference p = findPreference(preference);
        if (p != null) {
            p.setSummary(text);
        }
    }

    private void updateNetworkType() {
        // Whether EDGE, UMTS, etc...
        String netWorkTypeName = mTelephonyManager.getNetworkTypeNameGemini(mSimId);
        netWorkTypeName = renameNetworkTypeNameForCTSpec(netWorkTypeName);
        Preference p = findPreference(KEY_NETWORK_TYPE);
        if (p != null) {
            p.setSummary((netWorkTypeName.equals("UNKNOWN")) ? mUnknown : netWorkTypeName);
        }
    }

    /**
     * CT spec requires that network type should apply to spec
     * "CDMA - EvDo rev. 0" -> "CDMA EVDO"
     * "CDMA - EvDo rev. A" -> "CDMA EVDO"
     * "CDMA - EvDo rev. B" -> "CDMA EVDO"
     * "CDMA - 1xRTT" -> "1x"
     * "GPRS" -> "GSM"
     * "HSDPA" -> "WCDMA"
     * "HSUPA" -> "WCDMA"
     * "HSPA" -> "WCDMA"
     * "HSPA+" -> "WCDMA"
     * "UMTS" -> "WCDMA"
     */
    static String renameNetworkTypeNameForCTSpec(String netWorkTypeName) {
        Xlog.d(TAG, "renameNetworkTypeNameForCTSpec, netWorkTypeName=" + netWorkTypeName);
        if (netWorkTypeName.equals("CDMA - EvDo rev. 0")
                || netWorkTypeName.equals("CDMA - EvDo rev. A")
                || netWorkTypeName.equals("CDMA - EvDo rev. B")) {
            return "CDMA EVDO";
        } else if (netWorkTypeName.equals("CDMA - 1xRTT")) {
            return "CDMA 1x";
        } else if (netWorkTypeName.equals("GPRS")
                || netWorkTypeName.equals("EDGE")) {
            return "GSM";
        } else if (netWorkTypeName.equals("HSDPA")
                || netWorkTypeName.equals("HSUPA")
                || netWorkTypeName.equals("HSPA")
                || netWorkTypeName.equals("HSPA+")
                || netWorkTypeName.equals("UMTS")) {
            return "WCDMA";
        } else {
            return netWorkTypeName;
        }
    }

    private void updateServiceState(ServiceState serviceState) {
        setSummaryText(KEY_OPERATOR_NAME, serviceState.getOperatorAlphaLong());
    }

    void updateSignalStrength() {
        Xlog.d(TAG, "updateSignalStrength()");
        // TODO PhoneStateIntentReceiver is deprecated and PhoneStateListener
        // should probably used instead.

        // not loaded in some versions of the code (e.g., zaku)
        if (mSignalStrengthPreference != null) {
            Xlog.d(TAG, "updateSignalStrength, ServiceState : " + mServiceState);
            if ((ServiceState.STATE_OUT_OF_SERVICE == mServiceState) || (ServiceState.STATE_POWER_OFF == mServiceState)) {
                Xlog.d(TAG, "updateSignalStrength, ServiceState is Not ready, set signalStrength 0");
                mSignalStrengthPreference.setSummary("0");
            }

            Resources r = getResources();
            boolean isGsmSignal = true;
            int signalDbm = 0;
            int signalAsu = 0;
            if (mSignalStrength != null) {
                isGsmSignal = mSignalStrength.isGsm();
                if (isGsmSignal) {
                    signalDbm = mSignalStrength.getGsmSignalStrengthDbm();
                    signalAsu = mSignalStrength.getGsmSignalStrength();
                } else {
                    signalDbm = mSignalStrength.getCdmaDbm();
                    signalAsu = (int) ((signalDbm + 113) * 0.5);
                }
                Xlog.d(TAG, "updateSignalStrength, SignalStrength is " + signalDbm + " dbm , " + signalAsu + " asu");
                signalDbm = (-1 == signalDbm) ? 0 : signalDbm;
                signalAsu = (-1 == signalAsu) ? 0 : signalAsu;
            }

            Xlog.d(TAG, "updateSignalStrength, SignalStrength is " + signalDbm + " dbm , " + signalAsu + " asu");
            mSignalStrengthPreference.setSummary(String.valueOf(signalDbm) + " "
                    + r.getString(R.string.radioInfo_display_dbm) + "   " + String.valueOf(signalAsu) + " "
                    + r.getString(R.string.radioInfo_display_asu));
        }
    }

    private int dealWith3AppLaunch() {
        List<SIMInfo> simList = SIMInfo.getInsertedSIMList(this);
        int slotID;
        if (simList.size() == 0) {
            slotID = -1;
        } else if (simList.size() == 1) {
            slotID = simList.get(0).mSlot;
        } else {
            slotID = simList.get(0).mSlot;
            for (SIMInfo temp : simList) {
                if (slotID > temp.mSlot) {
                    slotID = temp.mSlot;
                }
            }
        }
        Xlog.d(TAG, "dealWith3AppLaunch() slotID=" + slotID);
        return slotID;
    }

    class ScAsyncTask extends AsyncTask {
        @Override
        protected String doInBackground(Object... params) {
            return mTelephonyManagerEx.getScAddress(mSimId);
        }

        @Override
        protected void onPostExecute(Object result) {
            super.onPostExecute(result);

            String gotScNumber = (String) result;
            Xlog.d(TAG, this + " Sms Service Center: " + gotScNumber);
            setSummaryText(KEY_SMS_CENTER, gotScNumber);
        }
    }

    /**
     * Get phone by slotId
     * @param phone
     * @param slotId
     * @return
     */
    public static Phone getPhoneBySlotId(Phone geminiPhone, int slotId) {
        if (geminiPhone != null && geminiPhone instanceof GeminiPhone) {
            return ((GeminiPhone) geminiPhone).getPhonebyId(slotId);
        } else {
            Xlog.w(TAG, "getPhoneBySlotId phone is null or type is not GeminiPhone, geminiPhone=" + geminiPhone);
            return null;
        }
    }
}
