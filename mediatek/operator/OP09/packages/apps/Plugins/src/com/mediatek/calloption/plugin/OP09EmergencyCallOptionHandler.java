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

package com.mediatek.calloption.plugin;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.provider.Settings;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.ListAdapter;

import com.android.internal.telephony.PhoneConstants;
import com.android.phone.Constants;
import com.mediatek.calloption.CallOptionBaseHandler;
import com.mediatek.calloption.CallOptionUtils;
import com.mediatek.calloption.Request;
import com.mediatek.calloption.SimPickerAdapter;
import com.mediatek.calloption.SimPickerAdapter.ItemHolder;
import com.mediatek.op09.plugin.R;
import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;

import java.util.ArrayList;
import java.util.List;

public class OP09EmergencyCallOptionHandler extends CallOptionBaseHandler
                                            implements DialogInterface.OnClickListener,
                                                       DialogInterface.OnDismissListener,
                                                       DialogInterface.OnCancelListener {

    private static final String TAG = "OP09EmergencyCallOptionHandler";

    private Request mRequest;
    private Context mPluginContext;

    public OP09EmergencyCallOptionHandler(Context pluginContext) {
        mPluginContext = pluginContext;
    }

    @Override
    public void handleRequest(final Request request) {
        log("handleRequest()");

        mRequest = request;

        String number = CallOptionUtils.getInitialNumber(request.getApplicationContext(), request.getIntent());
        if (Intent.ACTION_CALL_EMERGENCY.equals(request.getIntent().getAction())) {
            if (-1 != request.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1)) {
                request.getResultHandler().onPlaceCallDirectly(request.getIntent());
            } else {
                showSimSelectionDialog();
            }
            return;
        }

        // for double G network case
        if (!OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_1) &&
                !OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_2)) {
            if (PhoneNumberUtils.isEmergencyNumber(number)) {
                request.getResultHandler().onPlaceCallDirectly(request.getIntent());
            } else {
                if (null != mSuccessor) {
                    mSuccessor.handleRequest(request);
                }
            }
            return;
        }

        TelephonyManager telephony = TelephonyManager.getDefault();
        int slotIntent = request.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);

        if (-1 != slotIntent && 1 < SIMInfoWrapper.getDefault().getInsertedSimCount()) {
            if (PhoneNumberUtils.isEmergencyNumberExt(number, TelephonyManagerEx.getDefault().getPhoneType(slotIntent))) {
                request.getResultHandler().onPlaceCallDirectly(request.getIntent());
            } else {
                if (null != mSuccessor) {
                    mSuccessor.handleRequest(request);
                }
            }
        } else {
            boolean isCdmaECC = PhoneNumberUtils.isEmergencyNumberExt(number, PhoneConstants.PHONE_TYPE_CDMA);
            boolean isGsmECC = PhoneNumberUtils.isEmergencyNumberExt(number, PhoneConstants.PHONE_TYPE_GSM);
            if (-1 != slotIntent && 1 == SIMInfoWrapper.getDefault().getInsertedSimCount()) {
                if (OP09CallOptionUtils.isCDMAPhoneTypeBySlot(slotIntent)) {
                    log("Insert one sim in CDMA slot");
                    if (!isCdmaECC && isGsmECC) {
                        int slotGsm = OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_1) ?
                                PhoneConstants.GEMINI_SIM_2 : PhoneConstants.GEMINI_SIM_1;
                        log("The number is GSM ECC but not CDMA ECC, set slot = " + slotGsm);
                        request.getIntent().putExtra(Constants.EXTRA_SLOT_ID, slotGsm);
                        request.getResultHandler().onPlaceCallDirectly(request.getIntent());
                    } else if (isCdmaECC && !isGsmECC) {
                        log("The number is CDMA ECC but not GSM ECC");
                        request.getResultHandler().onPlaceCallDirectly(request.getIntent());
                    } else if (isCdmaECC && isGsmECC) {
                        log("The number is CDMA ECC and GSM ECC");
                        showSimSelectionDialog();
                    } else {
                        log("The number is NOT CDMA ECC and GSM ECC");
                        if (null != mSuccessor) {
                            mSuccessor.handleRequest(request);
                        }
                    }
                } else {
                    log("Insert one sim in GSM slot");
                    if (isCdmaECC) {
                        log("The number is CDMA ECC");
                        showSimSelectionDialog();
                    } else if (isGsmECC) {
                        log("The number is GSM ECC");
                        request.getResultHandler().onPlaceCallDirectly(request.getIntent());
                    } else {
                        log("The number is NOT CDMA ECC and GSM ECC");
                        if (null != mSuccessor) {
                            mSuccessor.handleRequest(request);
                        }
                    }
                }
            } else if (-1 == slotIntent || 0 == SIMInfoWrapper.getDefault().getInsertedSimCount()) {
                log("not insert any sim");
                if (isCdmaECC && isGsmECC) {
                    log("The number is CDMA ECC and GSM ECC");
                    showSimSelectionDialog();
                } else if (isCdmaECC && !isGsmECC) {
                    int slotCdma = OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_1) ?
                            PhoneConstants.GEMINI_SIM_1 : PhoneConstants.GEMINI_SIM_2;
                    log("The number is CDMA ECC but not GSM ECC, set slot = " + slotCdma);
                    request.getIntent().putExtra(Constants.EXTRA_SLOT_ID, slotCdma);
                    request.getResultHandler().onPlaceCallDirectly(request.getIntent());
                } else if (!isCdmaECC && isGsmECC) {
                    int slotGsm = OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_1) ?
                            PhoneConstants.GEMINI_SIM_2 : PhoneConstants.GEMINI_SIM_1;
                    log("The number is GSM ECC but not CDMA ECC, set slot = " + slotGsm);
                    request.getIntent().putExtra(Constants.EXTRA_SLOT_ID, slotGsm);
                    request.getResultHandler().onPlaceCallDirectly(request.getIntent());
                } else {
                    log("The number is NOT CDMA ECC and GSM ECC");
                    if (null != mSuccessor) {
                        mSuccessor.handleRequest(request);
                    }
                }
            } else {
                if (null != mSuccessor) {
                    mSuccessor.handleRequest(request);
                }
            }
        }
    }

    private void showSimSelectionDialog() {
        log("showSimSelectionDialog()");
        SimPickerAdapter simPickerAdapter
                = new OP09SimPickerAdapter(mPluginContext, mRequest.getActivityContext(), -1, mRequest.isMultipleSim());
        simPickerAdapter.setItems(createSimPickerItemHolder(mRequest.getActivityContext(),
                                                            false, mRequest.isMultipleSim()));

        AlertDialog.Builder builder = new AlertDialog.Builder(mRequest.getActivityContext());
        builder.setSingleChoiceItems(simPickerAdapter, -1, this)
               .setTitle(mPluginContext.getString(R.string.sim_manage_call_via));
        mDialog = builder.create();
        mDialog.setOnDismissListener(this);
        mDialog.setOnCancelListener(this);
        mDialog.show();
    }

    public void onClick(DialogInterface dialog, int which) {
        final AlertDialog alert = (AlertDialog) dialog;
        final ListAdapter listAdapter = alert.getListView().getAdapter();
        final int slot = ((Integer)listAdapter.getItem(which)).intValue();

        log("onClick() is called, slot = " + slot);
        dialog.dismiss();
        mRequest.getIntent().putExtra(Constants.EXTRA_SLOT_ID, slot);
        mRequest.getResultHandler().onPlaceCallDirectly(mRequest.getIntent());
    }

    public void onDismiss(DialogInterface dialog) {
        log("onDismiss()");
    }

    public void onCancel(DialogInterface dialog) {
        log("onCancel()");
        mRequest.getResultHandler().onHandlingFinish();
    }

    public List<ItemHolder> createSimPickerItemHolder(Context context, boolean addInternet, boolean isMultiSim) {
        List<SimInfoRecord> simInfos = SimInfoManager.getInsertedSimInfoList(context);
        ArrayList<ItemHolder> itemHolders = new ArrayList<ItemHolder>();
        ItemHolder temp = null;

        TelephonyManager telephony = TelephonyManager.getDefault();
        if (!CallOptionUtils.isSimInsert(mRequest, PhoneConstants.GEMINI_SIM_1)) {
            int phoneType = Integer.valueOf(TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_1));
            temp = new ItemHolder(new OP09SimPickerAdapter.NoSimItemInfo(phoneType, PhoneConstants.GEMINI_SIM_1),
                                  OP09SimPickerAdapter.ITEM_TYPE_NO_SIM);
            itemHolders.add(temp);
        } else {
            temp = new ItemHolder(SIMInfoWrapper.getDefault().getSimInfoBySlot(PhoneConstants.GEMINI_SIM_1),
                                  SimPickerAdapter.ITEM_TYPE_SIM);
            itemHolders.add(temp);
        }
        if (!CallOptionUtils.isSimInsert(mRequest, PhoneConstants.GEMINI_SIM_2)) {
            int phoneType = Integer.valueOf(TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_2));
            temp = new ItemHolder(new OP09SimPickerAdapter.NoSimItemInfo(phoneType, PhoneConstants.GEMINI_SIM_2),
                                  OP09SimPickerAdapter.ITEM_TYPE_NO_SIM);
            itemHolders.add(temp);
        } else {
            temp = new ItemHolder(SIMInfoWrapper.getDefault().getSimInfoBySlot(PhoneConstants.GEMINI_SIM_2),
                                  SimPickerAdapter.ITEM_TYPE_SIM);
            itemHolders.add(temp);
        }
        log("item holder size = " + itemHolders.size());
        return itemHolders;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
