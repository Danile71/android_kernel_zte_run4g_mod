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

package com.mediatek.engineermode.ims;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class ImsActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "EM/Ims";
    private static final String PROP_APN = "volte.emergency.pdn.name";
    private static final String PROP_TYPE = "volte.emergency.pdn.protocol";
    private static final String PROP_SS_MODE = "persist.radio.ss.mode";
    private static final String PROP_SS_DISABLE_METHOD = "persist.radio.ss.xrdm";
    private static final String PROP_SS_CFNUM = "persist.radio.xcap.cfn";
    private static final String TYPE_IP = "IP";
    private static final String TYPE_IPV6 = "IPV6";
    private static final String TYPE_IPV4V6 = "IPV4V6";
    private static final String MODE_SS_XCAP = "Prefer XCAP";
    private static final String MODE_SS_CS = "Prefer CS";
    private static final int DISABLE_MODE_DELETE_RULE = 1;
    private static final int DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG = 2;

    private RadioButton mRadioIp;
    private RadioButton mRadioIpv6;
    private RadioButton mRadioIpv4v6;
    private RadioButton mRadioSSXcap;
    private RadioButton mRadioSSCs;
    private RadioButton mRadioSSDisableTag;
    private RadioButton mRadioSSDisableDel;
    private TextView mImsStatus;
    private EditText mApn;
    private EditText mXcapCFNum;
    private Button mButtonSetApn;
    private Button mButtonSetType;
    private Button mButtonSetSSMode;
    private Button mButtonSetSSDisable;
    private Button mButtonSetXcapCFNum;
    private Toast mToast = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Xlog.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ims);

        mImsStatus = (TextView) findViewById(R.id.ims_status);
        mApn = (EditText) findViewById(R.id.ims_pdn_apn);
        mXcapCFNum = (EditText) findViewById(R.id.ims_ss_cf_num);
        mButtonSetApn = (Button) findViewById(R.id.ims_pdn_set_apn);
        mButtonSetApn.setOnClickListener(this);
        mRadioIp = (RadioButton) findViewById(R.id.ims_pdn_type_ip);
        mRadioIpv6 = (RadioButton) findViewById(R.id.ims_pdn_type_ipv6);
        mRadioIpv4v6 = (RadioButton) findViewById(R.id.ims_pdn_type_ipv4v6);
        mButtonSetType = (Button) findViewById(R.id.ims_pdn_set_type);
        mButtonSetType.setOnClickListener(this);
        mRadioSSXcap = (RadioButton) findViewById(R.id.ims_ss_mode_xcap);
        mRadioSSCs = (RadioButton) findViewById(R.id.ims_ss_mode_cs);
        mButtonSetSSMode = (Button) findViewById(R.id.ims_ss_set_mode);
        mButtonSetSSMode.setOnClickListener(this);
        mRadioSSDisableTag = (RadioButton) findViewById(R.id.ims_ss_disable_tag);
        mRadioSSDisableDel = (RadioButton) findViewById(R.id.ims_ss_disable_del);
        mButtonSetSSDisable = (Button) findViewById(R.id.ims_ss_set_disable);
        mButtonSetSSDisable.setOnClickListener(this);
        mButtonSetXcapCFNum = (Button) findViewById(R.id.ims_set_ss_cf_num);
        mButtonSetXcapCFNum.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        Xlog.d(TAG, "onResume()");
        super.onResume();

        Phone phone = PhoneFactory.getDefaultPhone();
        if (phone != null) {
            boolean status = phone.getImsRegInfo();
            Xlog.d(TAG, "getImsRegInfo(): " + status);
            mImsStatus.setText(getString(R.string.ims_status) + (status ? "true" : "false"));
        } else {
            Xlog.e(TAG, "getDefaultPhone() failed");
            showToast("Get IMS registration status failed");
        }

        String name = SystemProperties.get(PROP_APN, "");
        String type = SystemProperties.get(PROP_TYPE, TYPE_IPV4V6);
        String ssmode = SystemProperties.get(PROP_SS_MODE, MODE_SS_XCAP);
        String ssdisableMethod = SystemProperties.get(PROP_SS_DISABLE_METHOD,
                Integer.toString(DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG));
        String xcapCFNum = SystemProperties.get(PROP_SS_CFNUM, "");
        Xlog.d(TAG, PROP_APN + ": " + name);
        Xlog.d(TAG, PROP_TYPE + ": " + type);
        Xlog.d(TAG, PROP_SS_MODE + ": " + ssmode);
        Xlog.d(TAG, PROP_SS_DISABLE_METHOD + ": " + ssdisableMethod);
        Xlog.d(TAG, PROP_SS_CFNUM + ":" + xcapCFNum);

        mApn.setText(name);
        if (TYPE_IP.equals(type)) {
            mRadioIp.setChecked(true);
        } else if (TYPE_IPV6.equals(type)) {
            mRadioIpv6.setChecked(true);
        } else if (TYPE_IPV4V6.equals(type)) {
            mRadioIpv4v6.setChecked(true);
        } else {
            showToast("Got invalid IP type: \"" + type + "\"");
        }

        if (MODE_SS_XCAP.equals(ssmode)) {
            mRadioSSXcap.setChecked(true);
        } else if (MODE_SS_CS.equals(ssmode)) {
            mRadioSSCs.setChecked(true);
        } else {
            showToast("Got invalid SS Mode: \"" + ssmode + "\"");
        }

        if (DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG == Integer.parseInt(ssdisableMethod)) {
            mRadioSSDisableTag.setChecked(true);
        } else if (DISABLE_MODE_DELETE_RULE == Integer.parseInt(ssdisableMethod)) {
            mRadioSSDisableDel.setChecked(true);
        } else {
            showToast("Got invalid SS Disable Method: \"" + ssdisableMethod + "\"");
        }

        mXcapCFNum.setText(xcapCFNum);
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonSetApn) {
            SystemProperties.set(PROP_APN, mApn.getText().toString());
            Xlog.d(TAG, "Set " + PROP_APN + " = " + mApn.getText().toString());
            showToast("Set APN done");
        } else if (v == mButtonSetType) {
            String type = "";
            if (mRadioIp.isChecked()) {
                type = TYPE_IP;
            } else if (mRadioIpv6.isChecked()) {
                type = TYPE_IPV6;
            } else if (mRadioIpv4v6.isChecked()) {
                type = TYPE_IPV4V6;
            } else {
                return;
            }
            SystemProperties.set(PROP_TYPE, type);
            Xlog.d(TAG, "Set " + PROP_TYPE + " = " + type);
            showToast("Set IP Type done");
        } else if (v == mButtonSetSSMode) {
            String ssmode = "";
            if (mRadioSSXcap.isChecked()) {
                ssmode = MODE_SS_XCAP;
            } else if (mRadioSSCs.isChecked()) {
                ssmode = MODE_SS_CS;
            } else {
                return;
            }
            SystemProperties.set(PROP_SS_MODE, ssmode);
            Xlog.d(TAG, "Set " + PROP_SS_MODE+ " = " + ssmode);
            showToast("Set SS Mode done");
        } else if (v == mButtonSetSSDisable) {
            String ssdisableMethod = "";
            if (mRadioSSDisableTag.isChecked()) {
                ssdisableMethod = Integer.toString(DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG);
            } else if (mRadioSSDisableDel.isChecked()) {
                ssdisableMethod = Integer.toString(DISABLE_MODE_DELETE_RULE);
            } else {
                return;
            }
            SystemProperties.set(PROP_SS_DISABLE_METHOD, ssdisableMethod);
            Xlog.d(TAG, "Set " + PROP_SS_DISABLE_METHOD + " = " + ssdisableMethod);
            showToast("Set SS Disable done");
        } else if (v == mButtonSetXcapCFNum) {
            SystemProperties.set(PROP_SS_CFNUM, mXcapCFNum.getText().toString());
            Xlog.d(TAG, "Set " + PROP_SS_CFNUM + " = " + mXcapCFNum.getText().toString());
            showToast("Set SS CF Number done");
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
