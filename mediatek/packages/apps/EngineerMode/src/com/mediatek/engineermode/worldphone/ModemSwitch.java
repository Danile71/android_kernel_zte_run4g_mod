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

package com.mediatek.engineermode.worldphone;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.SystemProperties;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.LteDcConfigHandler;
import com.android.internal.telephony.worldphone.ModemSwitchHandler;
import com.android.internal.telephony.worldphone.LteModemSwitchHandler;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.android.internal.telephony.gemini.MTKPhoneFactory;
import com.android.internal.telephony.PhoneFactory;

public class ModemSwitch extends Activity implements View.OnClickListener {
    private static final String TAG = "EM/ModemSwitch";

    private RadioButton mRadioWg;
    private RadioButton mRadioTg;
    private RadioButton mRadioLwg;
    private RadioButton mRadioLtg;
    private RadioButton mRadioSglte;
    private RadioButton mRadioAuto;
    private TextView mText;
    private EditText mTimer;
    private Button mButtonSet;
    private Button mButtonSetTimer;
    private static IWorldPhone sWorldPhone = null;
    private static LteDcConfigHandler sLteDcConfigHandler = null;
    private AlertDialog alertDialog;
    private static int sProjectType;
    private static final int PROJ_TYPE_NOT_SUPPORT = 0;
    private static final int PROJ_TYPE_WORLD_PHONE = 1;
    private static final int PROJ_TYPE_SGLTE_LTTG  = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.modem_switch);

        if (PhoneFactory.isWorldPhone()) {
            Xlog.d(TAG, "World Phone Project");
            sWorldPhone = MTKPhoneFactory.getWorldPhone();
            sProjectType = PROJ_TYPE_WORLD_PHONE;
        } else if (PhoneFactory.isLteDcSupport()) {
            Xlog.d(TAG, "SGLTE LTTG Project");
            sLteDcConfigHandler = MTKPhoneFactory.getLteDcConfigHandler();
            sProjectType = PROJ_TYPE_SGLTE_LTTG;
        } else {
            Xlog.d(TAG, "Not Supported Project");
            sProjectType = PROJ_TYPE_NOT_SUPPORT;
        }
        mRadioWg = (RadioButton) findViewById(R.id.modem_switch_wg);
        mRadioTg = (RadioButton) findViewById(R.id.modem_switch_tg);
        mRadioLwg = (RadioButton) findViewById(R.id.modem_switch_fdd_csfb);
        mRadioLtg = (RadioButton) findViewById(R.id.modem_switch_tdd_csfb);
        mRadioSglte = (RadioButton) findViewById(R.id.modem_switch_sglte);
        mRadioAuto = (RadioButton) findViewById(R.id.modem_switch_auto);
        mButtonSet = (Button) findViewById(R.id.modem_switch_set);
        String optr = SystemProperties.get("ro.operator.optr");
        if (sProjectType == PROJ_TYPE_WORLD_PHONE) {
            if (PhoneFactory.isLteSupport()) {
                if (PhoneFactory.isLteDcSupport()) {
                    mRadioLtg.setVisibility(View.GONE);
                    if (PhoneFactory.isGeminiSupport()) {
                        if (optr != null && optr.equals("OP01")) {
                            mRadioLwg.setVisibility(View.GONE);
                        }
                    } else {
                        mRadioWg.setVisibility(View.GONE);
                    }
                } else {
                    mRadioWg.setVisibility(View.GONE);
                    mRadioTg.setVisibility(View.GONE);
                    mRadioSglte.setVisibility(View.GONE);
                }
            } else {
                mRadioLwg.setVisibility(View.GONE);
                mRadioLtg.setVisibility(View.GONE);
                mRadioSglte.setVisibility(View.GONE);
            }
        } else if (sProjectType == PROJ_TYPE_SGLTE_LTTG) {
            mRadioWg.setVisibility(View.GONE);
            mRadioLwg.setVisibility(View.GONE);
            mRadioLtg.setVisibility(View.GONE);
        } else if (sProjectType == PROJ_TYPE_NOT_SUPPORT) {
            mRadioWg.setVisibility(View.GONE);
            mRadioTg.setVisibility(View.GONE);
            mRadioLwg.setVisibility(View.GONE);
            mRadioLtg.setVisibility(View.GONE);
            mRadioSglte.setVisibility(View.GONE);
            mRadioAuto.setVisibility(View.GONE);
            mButtonSet.setVisibility(View.GONE);
        }
        mText = (TextView) findViewById(R.id.modem_switch_current_value);
        mTimer = (EditText) findViewById(R.id.modem_switch_timer);
        mButtonSet.setOnClickListener(this);
        mButtonSetTimer = (Button) findViewById(R.id.modem_switch_set_timer);
        mButtonSetTimer.setOnClickListener(this);
        alertDialog = new AlertDialog.Builder(this).create();
        alertDialog.setTitle("Switching Mode");
    }

    @Override
    protected void onResume() {
        Xlog.d(TAG, "onResume()");
        super.onResume();

        int modemType = getModemType();
        Xlog.d(TAG, "Get modem type: " + modemType);

        if (modemType == LteModemSwitchHandler.MD_TYPE_WG) {
            mText.setText(R.string.modem_switch_is_wg);
            mRadioWg.setChecked(true);
        } else if (modemType == LteModemSwitchHandler.MD_TYPE_TG) {
            mText.setText(R.string.modem_switch_is_tg);
            mRadioTg.setChecked(true);
        } else if (modemType == LteModemSwitchHandler.MD_TYPE_LWG) {
            mText.setText(R.string.modem_switch_is_fdd_csfb);
            mRadioLwg.setChecked(true);
        } else if (modemType == LteModemSwitchHandler.MD_TYPE_LTG) {
            mText.setText(R.string.modem_switch_is_tdd_csfb);
            mRadioLtg.setChecked(true);
        } else if (modemType == LteModemSwitchHandler.MD_TYPE_LTNG) {
            mText.setText(R.string.modem_switch_is_sglte);
            mRadioSglte.setChecked(true);
        } else {
            mText.setText(R.string.modem_switch_current_value);
            Toast.makeText(this, "Query Modem type failed: " + modemType, Toast.LENGTH_SHORT).show();
        }

        if (Settings.Global.getInt(getContentResolver(),
                Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, IWorldPhone.SELECTION_MODE_AUTO) == IWorldPhone.SELECTION_MODE_AUTO) {
            mRadioWg.setChecked(false);
            mRadioTg.setChecked(false);
            mRadioLwg.setChecked(false);
            mRadioLtg.setChecked(false);
            mRadioSglte.setChecked(false);
            mRadioAuto.setChecked(true);
        }

        int timer = Settings.Global.getInt(getContentResolver(),
                Settings.Global.WORLD_PHONE_FDD_MODEM_TIMER, 0);
        mTimer.setText(String.valueOf(timer));
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonSetTimer) {
            int timer = 0;
            try {
                timer = Integer.parseInt(mTimer.getText().toString());
            } catch (NumberFormatException e) {
                Xlog.w(TAG, "Invalid format: " + mTimer.getText());
                timer = 0;
            }
            Settings.Global.putInt(getContentResolver(),
                    Settings.Global.WORLD_PHONE_FDD_MODEM_TIMER, timer);
            Toast.makeText(ModemSwitch.this, "Set timer succeed.", Toast.LENGTH_SHORT).show();
            return;
        }

        int oldMdType = getModemType();
        int airplaneMode = Settings.Global.getInt(getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0);
        Xlog.d(TAG, "airplaneMode: " + airplaneMode);
        if (airplaneMode == 1) {
            Toast.makeText(ModemSwitch.this, "Modem switch is not allowed in flight mode", Toast.LENGTH_SHORT).show();
            return;
        }
        if (mRadioWg.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + LteModemSwitchHandler.MD_TYPE_WG);
            sWorldPhone.setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_WG);
            if (!PhoneFactory.isLteSupport()) {
                if (oldMdType != ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
                    ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
                }
            }
        } else if (mRadioTg.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + LteModemSwitchHandler.MD_TYPE_TG);
            if (sProjectType == PROJ_TYPE_WORLD_PHONE) {
                sWorldPhone.setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_TG);
                if (!PhoneFactory.isLteSupport()) {
                    if (oldMdType != ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                        ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
                    }
                }
            } else if (sProjectType == PROJ_TYPE_SGLTE_LTTG) {
                sLteDcConfigHandler.setModemSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_TG);
            }
        } else if (mRadioLwg.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + LteModemSwitchHandler.MD_TYPE_LWG);
            sWorldPhone.setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_LWG);
        } else if (mRadioLtg.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + LteModemSwitchHandler.MD_TYPE_LTG);
            sWorldPhone.setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_LTG);
        } else if (mRadioSglte.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + LteModemSwitchHandler.MD_TYPE_LTNG);
            if (sProjectType == PROJ_TYPE_WORLD_PHONE) {
                sWorldPhone.setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_LTNG);
            } else if (sProjectType == PROJ_TYPE_SGLTE_LTTG) {
                sLteDcConfigHandler.setModemSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, LteModemSwitchHandler.MD_TYPE_LTNG);
            }
        } else if (mRadioAuto.isChecked()) {
            Xlog.d(TAG, "Set modem type: auto");
            if (sProjectType == PROJ_TYPE_WORLD_PHONE) {
                sWorldPhone.setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_AUTO, 0);
            } else if (sProjectType == PROJ_TYPE_SGLTE_LTTG) {
                sLteDcConfigHandler.setModemSelectionMode(IWorldPhone.SELECTION_MODE_AUTO, 0);
            }
        } else {
            return;
        }

        int newMdType = getModemType();
        Xlog.d(TAG, "oldMdType: " + oldMdType + ", newMdType: " + newMdType);
        if (newMdType == LteModemSwitchHandler.MD_TYPE_WG) {
            mText.setText(R.string.modem_switch_is_wg);
        } else if (newMdType == LteModemSwitchHandler.MD_TYPE_TG) {
            mText.setText(R.string.modem_switch_is_tg);
        } else if (newMdType == LteModemSwitchHandler.MD_TYPE_LWG) {
            mText.setText(R.string.modem_switch_is_fdd_csfb);
        } else if (newMdType == LteModemSwitchHandler.MD_TYPE_LTG) {
            mText.setText(R.string.modem_switch_is_tdd_csfb);
        } else if (newMdType == LteModemSwitchHandler.MD_TYPE_LTNG) {
            mText.setText(R.string.modem_switch_is_sglte);
        }
        if (oldMdType != newMdType) {
            switchModemAlert(10000, 1000);
        } else {
            Toast.makeText(ModemSwitch.this, "Switch not executed", Toast.LENGTH_SHORT).show();
        }
    }

    private void switchModemAlert(long millisUntilFinished, long countDownInterval) {
        alertDialog.setMessage("Wait");
        alertDialog.setCanceledOnTouchOutside(false);
        alertDialog.show();

        new CountDownTimer(millisUntilFinished, countDownInterval) {
            @Override
            public void onTick(long millisUntilFinished) {
               alertDialog.setMessage("Wait " + (millisUntilFinished / 1000) + " seconds");
            }

            @Override
            public void onFinish() {
                alertDialog.cancel();
            }
        }.start();
    }

    private int getModemType() {
        if (PhoneFactory.isLteSupport()) {
            return LteModemSwitchHandler.getActiveModemType();
        } else {
            return ModemSwitchHandler.getModem();
        }
    }
}
