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

package com.mediatek.dm;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnMultiChoiceClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.Spanned;
import android.util.Log;
import android.widget.EditText;

import com.mediatek.dm.R;
import com.mediatek.dm.DmConst.TAG;
import com.mediatek.dm.util.DialogFactory;
import com.redbend.vdm.MmiInputQuery;

public class DmNiInfoActivity extends Activity {

    public static final String EXTRA_TYPE = "Type";
    private int mItem;
    private boolean[] mCheckedItem;
    private Context mContext;
    private static Integer sUiVisible;
    private static Integer sUiInteract;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;
        String opName = DmCommonFun.getOperatorName();

        if (DmConst.OperatorName.CU.equalsIgnoreCase(opName)) {
            sUiVisible = R.string.usermode_visible_cu;
            sUiInteract = R.string.usermode_interact_cu;
        } else if (DmConst.OperatorName.CMCC.equalsIgnoreCase(opName)) {
            sUiVisible = R.string.usermode_visible_cmcc;
            sUiInteract = R.string.usermode_interact_cmcc;
        } else {
            sUiVisible = R.string.usermode_visible_cu;
            sUiInteract = R.string.usermode_interact_cu;
        }
        Intent intent = getIntent();
        int type = intent.getIntExtra(EXTRA_TYPE, 0);
        // Bundle mBundle = intent.getExtras();
        Log.d(TAG.MMI, "DmNiInfoActivity type " + type);
        showDialog(type);
        registerReceiver(mBroadcastReceiver, new IntentFilter(DmConst.IntentAction.DM_CLOSE_DIALOG));
    }

    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mBroadcastReceiver);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DmConst.ServerMessage.TYPE_ALERT_1100:
            return DialogFactory.newAlert(this).setTitle(R.string.app_name)
                    .setMessage(DmInfoMsg.sViewContext.displayText)
                    .setNeutralButton(R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_ALERT_1100, onClick NeutralButton");
                            DmInfoMsg.sObserver.notifyInfoMsgClosed();
                            finish();
                        }
                    }).create();
        case DmConst.ServerMessage.TYPE_ALERT_1101:
            Log.i(TAG.MMI, "displayText: " + DmConfirmInfo.sViewContext.displayText);
            return DialogFactory.newAlert(this).setTitle(R.string.app_name)
                    .setMessage(DmConfirmInfo.sViewContext.displayText)
                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_ALERT_1101, onClick PositiveButton");
                            if (DmService.getInstance() != null) {
                                DmService.getInstance().cancleNiaAlarm();
                            }
                            DmConfirmInfo.sObserver.notifyConfirmationResult(true);
                            finish();
                        }
                    }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_ALERT_1101, onClick NegativeButton");
                            DmConfirmInfo.sObserver.notifyConfirmationResult(false);
                            finish();
                        }
                    }).create();
        case DmConst.ServerMessage.TYPE_ALERT_1102:
            Log.i(TAG.MMI, "ALERT_1102 recieved popup a dialog to let user to input some reply");
            Log.i(TAG.MMI, "displayTest: " + DmInputQueryInfo.sViewContext.displayText);

            final EditText et = initInputQueryEditText();

            return DialogFactory.newAlert(this).setTitle(R.string.app_name)
                    .setMessage(DmInputQueryInfo.sViewContext.displayText).setView(et)
                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_ALERT_1102, onClick PositiveButton");
                            if (DmService.getInstance() != null) {
                                DmService.getInstance().cancleNiaAlarm();
                            }

                            Editable editable = et.getText();
                            int etLength = editable.length();
                            char dest[] = new char[etLength];
                            editable.getChars(0, etLength, dest, 0);
                            String inputResult = new String(dest);

                            DmInputQueryInfo.sObserver.notifyInputResult(inputResult);

                            finish();
                        }
                    }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_ALERT_1102, onClick NegativeButton");
                            DmInputQueryInfo.sObserver.notifyCancelEvent();
                            finish();
                        }
                    }).create();
        case DmConst.ServerMessage.TYPE_ALERT_1103_1104:
            if (DmChoiceList.sChecked) {
                final int listLength = DmChoiceList.sStringArray.length;
                mCheckedItem = new boolean[listLength];
                for (int i = 0; i < listLength; i++) {
                    mCheckedItem[i] = ((DmChoiceList.sSelected & (1 << i)) > 0);
                }
                return DialogFactory
                        .newAlert(this)
                        .setTitle(R.string.app_name)
                        .setMultiChoiceItems(DmChoiceList.sStringArray, mCheckedItem,
                                new OnMultiChoiceClickListener() {
                                    public void onClick(DialogInterface arg0, int arg1, boolean arg2) {
                                        Log.v(TAG.MMI,
                                                new StringBuilder(
                                                        "TYPE_ALERT_1103_1104, onClick MultiChoiceItems, item ")
                                                        .append(arg1).append(" is ").append(arg2)
                                                        .toString());
                                        mCheckedItem[arg1] = arg2;
                                    }
                                })
                        .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                int newSelection = 0;
                                for (int i = 0; i < listLength; i++) {
                                    if (mCheckedItem[i]) {
                                        newSelection |= 1 << i;
                                    }
                                }

                                Log.v(TAG.MMI,
                                        "TYPE_ALERT_1103_1104, onClick MultiChoice PositiveButton,result="
                                                + newSelection);
                                DmChoiceList.sObserver.notfiyChoicelistSelection(newSelection);
                                finish();
                            }
                        })
                        .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                Log.v(TAG.MMI,
                                        "TYPE_ALERT_1103_1104, onClick MultiChoice NegativeButton");
                                DmChoiceList.sObserver.notifyCancelEvent();
                                finish();
                            }
                        }).create();
            } else {
                mItem = DmChoiceList.sSelected;
                return DialogFactory
                        .newAlert(this)
                        .setTitle(R.string.app_name)
                        .setSingleChoiceItems(DmChoiceList.sStringArray,
                                DmChoiceList.sSelected - 1, new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        Log.v(TAG.MMI,
                                                "TYPE_ALERT_1103_1104, onClick SingleChoiceItems, item is "
                                                        + whichButton);
                                        mItem = 1 << whichButton;
                                    }
                                })
                        .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                Log.v(TAG.MMI,
                                        "TYPE_ALERT_1103_1104, onClick SingleChoice PositiveButton,selected "
                                                + mItem);

                                DmChoiceList.sObserver.notfiyChoicelistSelection(mItem);
                                finish();
                            }
                        })
                        .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                Log.v(TAG.MMI,
                                        "TYPE_ALERT_1103_1104, onClick SingleChoice NegativeButton");
                                DmChoiceList.sObserver.notifyCancelEvent();
                                finish();
                            }
                        }).create();
            }
        case DmConst.ServerMessage.TYPE_UIMODE_VISIBLE:
            return DialogFactory.newAlert(this).setTitle(R.string.app_name).setMessage(sUiVisible)
                    .setNeutralButton(R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_UIMODE_VISIBLE, onClick NeutralButto");
                            startService();
                            finish();
                        }
                    }).create();
        case DmConst.ServerMessage.TYPE_UIMODE_INTERACT:
            return DialogFactory.newAlert(this).setTitle(R.string.app_name).setMessage(sUiInteract)
                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_UIMODE_INTERACT, onClick PositiveButton");
                            startService();
                            finish();
                        }
                    }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Log.v(TAG.MMI, "TYPE_UIMODE_INTERACT, onClick NegativeButton");
                            if (DmService.getInstance() != null) {
                                DmService.getInstance().userCancled();
                            }
                            finish();
                        }
                    }).create();
        default:
            break;
        }
        return null;
    }

    private EditText initInputQueryEditText() {
        Log.i(TAG.MMI, "start to init the input query eidt text");

        EditText et = new EditText(this);
        et.setText(DmInputQueryInfo.defaultString);

        InputFilter filter = new InputFilter() {
            public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                    int dstart, int dend) {
                for (int i = start; i < end; i++) {
                    if (!Character.isLetterOrDigit(source.charAt(i))) {
                        return "";
                    }
                }
                return null;
            }
        };

        InputFilter lenFilter = null;

        if (DmInputQueryInfo.maxLen > 0) {
            lenFilter = new InputFilter.LengthFilter(DmInputQueryInfo.maxLen);
            et.setFilters(new InputFilter[] { lenFilter });
        }

        MmiInputQuery.InputType inputType = DmInputQueryInfo.inputType;
        int echoType = 0x0;

        if (DmInputQueryInfo.echoType == MmiInputQuery.EchoType.MASKED) {
            echoType = InputType.TYPE_TEXT_VARIATION_PASSWORD;
        }

        switch (inputType) {
        case ALPHANUMERIC:
            et.setFilters(new InputFilter[] { filter });
            break;
        case DATE:
            et.setRawInputType(InputType.TYPE_CLASS_DATETIME
                    | InputType.TYPE_DATETIME_VARIATION_DATE);
            break;
        case IP_ADDRESS:
            et.setRawInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL);
            break;
        case NUMERIC:
            et.setRawInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED
                    | InputType.TYPE_NUMBER_FLAG_DECIMAL);
            break;
        case PHONE:
            et.setInputType(InputType.TYPE_CLASS_PHONE);
            break;
        case TIME:
            et.setInputType(InputType.TYPE_CLASS_DATETIME | InputType.TYPE_DATETIME_VARIATION_TIME);
            break;
        case UNDEFINED:
            et.setInputType(InputType.TYPE_CLASS_TEXT | echoType);
            break;
        default:
            break;

        }

        return et;
    }

    private void startService() {
        Intent serviceIntent = new Intent(mContext, DmService.class);
        serviceIntent.setAction(DmConst.IntentAction.DM_NIA_START);
        startService(serviceIntent);
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            DmNiInfoActivity.this.finish();
        }
    };
}
