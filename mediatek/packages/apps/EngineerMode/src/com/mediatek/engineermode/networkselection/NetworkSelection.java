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

package com.mediatek.engineermode.networkselection;

import android.app.Activity;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class NetworkSelection extends Activity {
    private static final String TAG = "NetworkSelection";

    private static final int GLOBAL_MODE_INDEX = 0;
    private static final int CDMA_ONLY_INDEX = 1;
    private static final int GSM_ONLY_INDEX = 2;

//    private static final int GLOBAL_MODE = 0;
//    private static final int CDMA_ONLY = 1;
//    private static final int GSM_ONLY = 2;

    public static final String SLOT_ID_KEY = "SLOT";

    private Spinner mNetworkType = null;
    private int mSlotId = PhoneConstants.GEMINI_SIM_1;
//    private ITelephony mTelephony;
    private ITelephonyEx mTelephonyEx;
    private int mCurrentType = -1;

    AdapterView.OnItemSelectedListener mNetworkHandler = new OnItemSelectedListener() {

        public void onItemSelected(AdapterView<?> arg0, View arg1, int pos, long arg3) {
            if (mTelephonyEx == null || mCurrentType == arg0.getSelectedItemPosition()) {
                return;
            }

            Xlog.i(TAG, "setMode. slot" + mSlotId + ", mode " + pos);
            mCurrentType = arg0.getSelectedItemPosition();

            try {
                switch (pos) {
                case GLOBAL_MODE_INDEX:
                    mTelephonyEx.setInternationalNetworkMode(InternationalRoamingConstants.NW_MODE_INTERNATIONAL);
                    break;
                case CDMA_ONLY_INDEX:
                    mTelephonyEx.setInternationalNetworkMode(InternationalRoamingConstants.NW_MODE_CDMA_ONLY);
                    break;
                case GSM_ONLY_INDEX:
                    mTelephonyEx.setInternationalNetworkMode(InternationalRoamingConstants.NW_MODE_GSM_ONLY);
                    break;
                default:
                    break;
                }
            } catch (RemoteException e) {
                Xlog.e(TAG, "setInternationalNetworkMode failed. slot" + mSlotId);
                Toast.makeText(NetworkSelection.this, "Failed to set International Network Mode", Toast.LENGTH_SHORT).show();
                return;
            }

        }

        public void onNothingSelected(AdapterView<?> arg0) {
            // Do nothing
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.networkselection);

        mNetworkType = (Spinner) findViewById(R.id.networktype);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, getResources()
                        .getStringArray(R.array.ct_network_selection));
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mNetworkType.setAdapter(adapter);
        mNetworkType.setOnItemSelectedListener(mNetworkHandler);

        if (mTelephonyEx == null) {
            mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
        }
        if (mTelephonyEx == null) {
            Xlog.i(TAG, "getService() failed");
            finish();
            return;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (mTelephonyEx == null) {
            return;
        }

        int mode = -1;

        try {
            mode = mTelephonyEx.getInternationalNetworkMode();
        } catch (RemoteException e) {
            Xlog.e(TAG, "getInternationalNetworkMode failed. slot " + mSlotId);
            Toast.makeText(NetworkSelection.this, "Failed to get International Network Mode", Toast.LENGTH_SHORT).show();
            return;
        }

        Xlog.i(TAG, "getMode. slot" + mSlotId + ", mode " + mode);

        if (mode < 0 || mode >= mNetworkType.getCount()) {
            Toast.makeText(NetworkSelection.this, "Modem returned invalid network type: " + mode, Toast.LENGTH_SHORT).show();
            return;
        }

        mNetworkType.setSelection(mode);
        mCurrentType = mNetworkType.getSelectedItemPosition();
    }

}
