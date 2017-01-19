/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.lte;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * For setting lte network mode
 */
public class LteNetworkMode extends Activity implements AdapterView.OnItemSelectedListener {
    private static final String TAG = "EM/LteNetworkMode";
    private static final int EVENT_QUERY_NETWORKMODE_DONE = 101;
    private static final int EVENT_SET_NETWORKMODE_DONE = 102;

    private static final int GSM_ONLY_INDEX = 0;
    private static final int WCDMA_ONLY_INDEX = 1;
    private static final int GSM_WCDMA_INDEX = 2;
    private static final int LTE_GSM_INDEX = 3;
    private static final int LTE_ONLY_INDEX = 4;
    private static final int LTE_WCDMA_INDEX = 5;
    private static final int LTE_WCDMA_GSM_INDEX = 6;
    private static final int WCDMA_GSM_LTE_INDEX = 7;

    private static final int WCDMA_PREFERRED = Phone.NT_MODE_WCDMA_PREF;
    private static final int WCDMA_ONLY = Phone.NT_MODE_WCDMA_ONLY;
    private static final int GSM_ONLY = Phone.NT_MODE_GSM_ONLY; 
    private static final int GSM_WCDMA = Phone.NT_MODE_GSM_UMTS;
    private static final int LTE_GSM = Phone.NT_MODE_LTE_GSM;
    private static final int LTE_ONLY = Phone.NT_MODE_LTE_ONLY;
    private static final int LTE_WCDMA = Phone.NT_MODE_LTE_WCDMA;
    private static final int LTE_WCDMA_GSM = Phone.NT_MODE_LTE_GSM_WCDMA;
    private static final int WCDMA_GSM_LTE = Phone.NT_MODE_GSM_WCDMA_LTE;

    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    ITelephonyEx mITelephonyEx = null;
    private int mSlot = PhoneConstants.GEMINI_SIM_1;
    private int mCurrentPos = 0;
    private int sUserSelectMode = -1;

    private Spinner mPreferredNetworkSpinner = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lte_network_mode);

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
        mITelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,
                getResources().getStringArray(R.array.lte_preferred_network_labels));
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        mPreferredNetworkSpinner = (Spinner) findViewById(R.id.networkModeSwitching);
        mPreferredNetworkSpinner.setAdapter(adapter);
        mPreferredNetworkSpinner.setOnItemSelectedListener(this);
    }

    @Override
    protected void onResume() {
        Xlog.d(TAG, "onResume()");
        super.onResume();
        mSlot = GeminiPhone.get3GSimId();
        Xlog.d(TAG, "get3GSimId(): " + mSlot);
        mSlot = getIntent().getIntExtra("mSlot", mSlot);

        if (mSlot < 0) {
            Xlog.d(TAG, "3G/4G is Off.");
            mSlot = PhoneConstants.GEMINI_SIM_1;
        }
        Message msg = mHandler.obtainMessage(EVENT_QUERY_NETWORKMODE_DONE);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone.getPhonebyId(mSlot).getPreferredNetworkType(msg);
        } else {
            mPhone.getPreferredNetworkType(msg);
        }
        /*try {
            //mITelephonyEx.getPreferredNetworkTypeLteDc(msg);
            mPhone.getPreferredNetworkType(msg);
        } catch (RemoteException e) {
            Toast.makeText(this, R.string.query_preferred_fail, Toast.LENGTH_SHORT).show();
        }*/
    }

    @Override
    public void onItemSelected(AdapterView parent, View v, int pos, long id) {
        Xlog.d(TAG, "onItemSelected() " + pos);
        if (mCurrentPos == pos) {
            // Prevent this function being callbacked in onResume()
            Xlog.d(TAG, "mCurrentPos == pos");
            return;
        }
        mCurrentPos = pos;

        int mode = 0;
        switch (pos) {
        case GSM_ONLY_INDEX:
            mode = GSM_ONLY;
            break;
        case WCDMA_ONLY_INDEX:
            mode = WCDMA_ONLY;
            break;
        case GSM_WCDMA_INDEX:
            mode = GSM_WCDMA;
            break;
        case LTE_GSM_INDEX:
            mode = LTE_GSM;
            break;
        case LTE_ONLY_INDEX:
            mode = LTE_ONLY;
            break;
        case LTE_WCDMA_INDEX:
            mode = LTE_WCDMA;
            break;
        case LTE_WCDMA_GSM_INDEX:
            mode = LTE_WCDMA_GSM;
            break;
        case WCDMA_GSM_LTE_INDEX:
            mode = WCDMA_GSM_LTE;
            break;
        default:
            return;
        }

        sUserSelectMode = mode;
        Settings.Global.putInt(getContentResolver(),
                Settings.Global.USER_PREFERRED_NETWORK_MODE, sUserSelectMode);
        Message msg = mHandler.obtainMessage(EVENT_SET_NETWORKMODE_DONE);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone.setPreferredNetworkTypeGemini(mode, msg, mSlot);
        } else {
            mPhone.setPreferredNetworkType(mode, msg);
        }
        /*try {
            //mITelephonyEx.setPreferredNetworkTypeLteDc(mode, msg);
            mPhone.setPreferredNetworkType(mode,msg);
        } catch (RemoteException e) {
            Toast.makeText(this, R.string.query_preferred_fail, Toast.LENGTH_SHORT).show();
        }*/
    }

    @Override
    public void onNothingSelected(AdapterView parent) {
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            Xlog.d(TAG, "handleMessage() " + msg.what);
            AsyncResult ar;
            switch (msg.what) {
            case EVENT_QUERY_NETWORKMODE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int type = ((int[]) ar.result)[0];
                    int pos = 0;
                    Xlog.d(TAG, "Get Preferred Type " + type);
                    switch (type) {
                    case GSM_ONLY:
                        pos = GSM_ONLY_INDEX;
                        break;
                    case WCDMA_ONLY:
                        pos = WCDMA_ONLY_INDEX;
                        break;
                    case WCDMA_PREFERRED:
                    case GSM_WCDMA:
                        pos = GSM_WCDMA_INDEX;
                        break;
                    case LTE_GSM:
                        pos = LTE_GSM_INDEX;
                        break;
                    case LTE_ONLY:
                        pos = LTE_ONLY_INDEX;
                        break;
                    case LTE_WCDMA:
                        pos = LTE_WCDMA_INDEX;
                        break;
                    case LTE_WCDMA_GSM:
                        pos = LTE_WCDMA_GSM_INDEX;
                        break;
                    case WCDMA_GSM_LTE:
                        pos = WCDMA_GSM_LTE_INDEX;
                        break;
                    default:
                        Toast.makeText(LteNetworkMode.this, R.string.query_preferred_fail, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    mCurrentPos = pos;
                    mPreferredNetworkSpinner.setSelection(pos, true);
                } else {
                    Xlog.d(TAG, "handleMessage() ar.exception:" + ar.exception);
                    Toast.makeText(LteNetworkMode.this, R.string.query_preferred_fail, Toast.LENGTH_SHORT).show();
                }
                break;
            case EVENT_SET_NETWORKMODE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Toast.makeText(LteNetworkMode.this, R.string.query_preferred_fail, Toast.LENGTH_SHORT).show();

                    Message msg1 = mHandler.obtainMessage(EVENT_QUERY_NETWORKMODE_DONE);
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        mGeminiPhone.getPhonebyId(mSlot).getPreferredNetworkType(msg1);
                    } else {
                        mPhone.getPreferredNetworkType(msg1);
                    }
                    /*try {
                        //mITelephonyEx.getPreferredNetworkTypeLteDc(msg1);
                        mPhone.getPreferredNetworkType(msg1);
                    } catch (RemoteException e) {
                    }*/
                } else {
                    Xlog.d(TAG, "sUserSelectMode: " + sUserSelectMode);
                    Settings.Global.putInt(getContentResolver(),
                        Settings.Global.USER_PREFERRED_NETWORK_MODE, sUserSelectMode);
                }
                break;
            default:
                break;
            }
        }
    };
}
