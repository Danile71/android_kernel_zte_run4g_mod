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
package com.mediatek.telephony;

import static android.Manifest.permission.READ_PHONE_STATE;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.Rlog;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.gemini.MTKPhoneFactory;
import com.android.internal.telephony.gsm.LteDcPhone;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.worldphone.LteModemSwitchHandler;
import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.telephony.SimInfoUpdate;
import com.mediatek.telephony.WorldPhoneUtil;

/**
 *@hide
 */
public class LteWorldPhoneOm extends Handler implements IWorldPhone {
    private static Object sLock = new Object();
    private static Context sContext;
    private static Phone sPhone;
    private static Phone[] sGsmPhone;
    private static Phone[] sLtePhone;
    private static String sOperatorSpec;
    private static String sPlmnSs;
    private static String sLastPlmn;
    private static String[] sImsi;
    private static String[] sNwPlmnStrings;
    private static String[] sNwPlmnStringsDc;
    private static int sVoiceRegState;
    private static int sDataRegState;
    private static int sRilVoiceRegState;
    private static int sRilDataRegState;
    private static int sRilVoiceRadioTechnology;
    private static int sRilDataRadioTechnology;
    private static int sUserType;
    private static int sRegion;
    private static int sDenyReason;
    private static int sSuspendId;
    private static int sSuspendIdDc;
    private static int sMajorSlot;
    private static int sSimInsertedStatus;
    private static int[] sIccCardType;
    private static boolean sVoiceCapable;
    private static boolean[] sIsInvalidSim;
    private static boolean[] sSuspendWaitImsi;
    private static boolean[] sSuspendWaitImsiDc;
    private static boolean[] sFirstSelect;
    private static CommandsInterface[] sGsmCi;
    private static CommandsInterface[] sLteCi;
    private static UiccController[] sUiccController;
    private static IccRecords[] sIccRecordsInstance;
    private static ServiceState sServiceState;
    private static ServiceState sServiceStateDc;
    private static LteModemSwitchHandler sLteModemSwitchHandler;
    private static final boolean sIsLteDcSupport = PhoneFactory.isLteDcSupport();
    private static final String[] PLMN_TABLE_TYPE1 = {
        "46000", "46002", "46007", "46008"
    };
    private static final String[] PLMN_TABLE_TYPE3 = {
        "46001", "46006", "46009", "45407", 
        "46003", "46005", "45502"
    };
    private static final String[] MCC_TABLE_DOMESTIC = {
        "460"
    };
    private static int sTddStandByCounter;
    private static int sFddStandByCounter;
    private static boolean sWaitInTdd;
    private static boolean sWaitInFdd;
    private static final int PREFERRED_RAT_USER   = 0;
    private static final int PREFERRED_RAT_SYSTEM = 1;
    private static final int STATUS_SIM1_INSERTED = 0x01;
    private static final int sDefaultBootuUpModem = LteModemSwitchHandler.MD_TYPE_UNKNOWN;
    private static final int[] FDD_STANDBY_TIMER = {
        60
    };
    private static final int[] TDD_STANDBY_TIMER = {
        40
    };

    public LteWorldPhoneOm() {
        logd("Constructor invoked");
        sOperatorSpec = SystemProperties.get("ro.operator.optr", NO_OP);
        logd("Operator Spec:" + sOperatorSpec);
        sPhone = MTKPhoneFactory.getDefaultPhone();
        sGsmPhone = new Phone[PhoneConstants.GEMINI_SIM_NUM];
        sGsmCi = new CommandsInterface[PhoneConstants.GEMINI_SIM_NUM];
        if (sIsLteDcSupport) {
            sLtePhone = new Phone[PhoneConstants.GEMINI_SIM_NUM];
            sLteCi = new CommandsInterface[PhoneConstants.GEMINI_SIM_NUM];
        }
        if (PhoneFactory.isGeminiSupport()) {
            logd("Gemini Project");
            for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                sGsmPhone[i] = ((PhoneProxy)(((GeminiPhone)sPhone).getPhonebyId(i))).getActivePhone();
                if (sIsLteDcSupport) {
                    sLtePhone[i] = MTKPhoneFactory.getLteDcPhone(i);
                }
            }
        } else {
            logd("Single Card Project");
            sGsmPhone[PhoneConstants.GEMINI_SIM_1] = ((PhoneProxy)sPhone).getActivePhone();
            if (sIsLteDcSupport) {
                sLtePhone[PhoneConstants.GEMINI_SIM_1] = MTKPhoneFactory.getLteDcPhone(PhoneConstants.GEMINI_SIM_1);
            }
        }
        for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (sIsLteDcSupport) {
                sLteCi[i] = ((PhoneBase)sLtePhone[i]).mCi;
                sLteCi[i].setOnPlmnChangeNotification(this, EVENT_REG_PLMN_CHANGED_SGLTE_1 + i, null);
                sLteCi[i].setOnGSMSuspended(this, EVENT_REG_SUSPENDED_SGLTE_1 + i, null);
                sLteCi[i].registerForOn(this, EVENT_RADIO_ON_SGLTE_1 + i, null);
            }
            sGsmCi[i] = ((PhoneBase)sGsmPhone[i]).mCi;
            sGsmCi[i].setOnPlmnChangeNotification(this, EVENT_REG_PLMN_CHANGED_1 + i, null);
            sGsmCi[i].setOnGSMSuspended(this, EVENT_REG_SUSPENDED_1 + i, null);
            sGsmCi[i].registerForOn(this, EVENT_RADIO_ON_1 + i, null);
            sGsmCi[i].setInvalidSimInfo(this, EVENT_INVALID_SIM_NOTIFY_1 + i, null);
        }

        sLteModemSwitchHandler = new LteModemSwitchHandler(sGsmCi);
        logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));

        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        intentFilter.addAction(ACTION_SHUTDOWN_IPO);
        if (PhoneFactory.isSimSwitchSupport()) {
            logd("SIM Switch Supported");
            intentFilter.addAction(TelephonyIntents.EVENT_CAPABILITY_SWITCH_DONE);
        } else {
            logd("SIM Switch Not Supported");
        }
        if (PhoneFactory.isGeminiSupport()) {
            intentFilter.addAction(TelephonyIntents.ACTION_SIM_INSERTED_STATUS);
        }
        intentFilter.addAction(ACTION_ADB_SWITCH_MODEM);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        sContext = sPhone.getContext();
        sContext.registerReceiver(mWorldPhoneReceiver, intentFilter);
        sVoiceCapable = sContext.getResources().getBoolean(com.android.internal.R.bool.config_voice_capable);
        logd("sVoiceCapable: " + sVoiceCapable);

        sUiccController = new UiccController[PhoneConstants.GEMINI_SIM_NUM];
        sIccRecordsInstance = new IccRecords[PhoneConstants.GEMINI_SIM_NUM];
        sImsi = new String[PhoneConstants.GEMINI_SIM_NUM];
        sIccCardType = new int[PhoneConstants.GEMINI_SIM_NUM];
        sIsInvalidSim = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        sSuspendWaitImsi = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        sSuspendWaitImsiDc = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        sFirstSelect = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        resetAllProperties();
        sSimInsertedStatus = -1;
        sTddStandByCounter = 0;
        sFddStandByCounter = 0;
        sWaitInTdd = false;
        sWaitInFdd = false;
        sRegion = REGION_UNKNOWN;
        sLastPlmn = null;
        if (getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("Auto select disable");
            sMajorSlot = AUTO_SELECT_DISABLE;
            Settings.Global.putInt(sContext.getContentResolver(), 
                    Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, SELECTION_MODE_MANUAL);
        } else {
            logd("Auto select enable");
            Settings.Global.putInt(sContext.getContentResolver(), 
                    Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, SELECTION_MODE_AUTO);
        }
        FDD_STANDBY_TIMER[sFddStandByCounter] = Settings.Global.getInt(
                sContext.getContentResolver(), Settings.Global.WORLD_PHONE_FDD_MODEM_TIMER, FDD_STANDBY_TIMER[sFddStandByCounter]);
        Settings.Global.putInt(sContext.getContentResolver(), 
                Settings.Global.WORLD_PHONE_FDD_MODEM_TIMER, FDD_STANDBY_TIMER[sFddStandByCounter]);
        logd("FDD_STANDBY_TIMER = " + FDD_STANDBY_TIMER[sFddStandByCounter] + "s");
        logd("sDefaultBootuUpModem = " + sDefaultBootuUpModem);
    }

    private boolean isAllowCampOn(String plmnString, int slotId) {
        logd("[isAllowCampOn]+ " + plmnString);
        logd("User type:" + sUserType);
        logd(WorldPhoneUtil.iccCardTypeToString(sIccCardType[slotId]));
        sRegion = getRegion(plmnString);
        int mdType = LteModemSwitchHandler.getActiveModemType();
        logd(LteModemSwitchHandler.modemToString(mdType));
        if (sUserType == sType1User) {
            if (sRegion == REGION_DOMESTIC) {
                if (mdType == LteModemSwitchHandler.MD_TYPE_LTNG
                        || mdType == LteModemSwitchHandler.MD_TYPE_LTG
                        || mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                    sDenyReason = CAMP_ON_NOT_DENIED;
                    logd("Camp on OK");
                    logd("[isAllowCampOn]-");
                    return true;
                } else if (mdType == LteModemSwitchHandler.MD_TYPE_LWG
                        || mdType == LteModemSwitchHandler.MD_TYPE_WG) {
                    sDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD;
                    logd("Camp on REJECT");
                    logd("[isAllowCampOn]-");
                    return false;
                }
            } else if (sRegion == REGION_FOREIGN) {
                if (mdType == LteModemSwitchHandler.MD_TYPE_LTNG
                        || mdType == LteModemSwitchHandler.MD_TYPE_LTG
                        || mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                    sDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                    logd("Camp on REJECT");
                    logd("[isAllowCampOn]-");
                    return false;
                } else if (mdType == LteModemSwitchHandler.MD_TYPE_LWG
                        || mdType == LteModemSwitchHandler.MD_TYPE_WG) {
                    sDenyReason = CAMP_ON_NOT_DENIED;
                    logd("Camp on OK");
                    logd("[isAllowCampOn]-");
                    return true;
                }
            } else {
                logd("Unknow region");
            }
        } else if (sUserType == sType2User || sUserType == sType3User) {
            if (mdType == LteModemSwitchHandler.MD_TYPE_LTNG
                    || mdType == LteModemSwitchHandler.MD_TYPE_LTG
                    || mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                sDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                logd("Camp on REJECT");
                logd("[isAllowCampOn]-");
                return false;
            } else if (mdType == LteModemSwitchHandler.MD_TYPE_LWG
                    || mdType == LteModemSwitchHandler.MD_TYPE_WG) {
                sDenyReason = CAMP_ON_NOT_DENIED;
                logd("Camp on OK");
                logd("[isAllowCampOn]-");
                return true;
            }
        } else {
            logd("Unknown user type");
        }
        sDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
        logd("Camp on REJECT");
        logd("[isAllowCampOn]-");

        return false;
    }

    private void handleNoService() {
        logd("[handleNoService]+ Can not find service");
        logd("Type" + sUserType + " user");
        logd(WorldPhoneUtil.regionToString(sRegion));
        int mdType = LteModemSwitchHandler.getActiveModemType();
        logd(LteModemSwitchHandler.modemToString(mdType));
        IccCardConstants.State iccState;
        if (PhoneFactory.isGeminiSupport()) {
            iccState = ((GeminiPhone)sPhone).getPhonebyId(sMajorSlot).getIccCard().getState();
        } else {
            iccState = sPhone.getIccCard().getState();
        }
        if (iccState == IccCardConstants.State.READY) {
            if (sUserType == sType1User) {
                if (mdType == LteModemSwitchHandler.MD_TYPE_LTNG
                        || mdType == LteModemSwitchHandler.MD_TYPE_LTG
                        || mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                    if (TDD_STANDBY_TIMER[sTddStandByCounter] >= 0) {
                        if (!sWaitInTdd) {
                            sWaitInTdd = true;
                            logd("Wait " + TDD_STANDBY_TIMER[sTddStandByCounter] + "s. Timer index = " + sTddStandByCounter);
                            postDelayed(mTddStandByTimerRunnable, TDD_STANDBY_TIMER[sTddStandByCounter] * 1000);
                        } else {
                            logd("Timer already set:" + TDD_STANDBY_TIMER[sTddStandByCounter] + "s");
                        }
                    } else {
                        logd("Standby in TDD modem");
                    }
                } else if (mdType == LteModemSwitchHandler.MD_TYPE_LWG
                        || mdType == LteModemSwitchHandler.MD_TYPE_WG) {
                    if (FDD_STANDBY_TIMER[sFddStandByCounter] >= 0) {
                        if (!sWaitInFdd) {
                            sWaitInFdd = true;
                            logd("Wait " + FDD_STANDBY_TIMER[sFddStandByCounter] + "s. Timer index = " + sFddStandByCounter);
                            postDelayed(mFddStandByTimerRunnable, FDD_STANDBY_TIMER[sFddStandByCounter] * 1000);
                        } else {
                            logd("Timer already set:" + FDD_STANDBY_TIMER[sFddStandByCounter] + "s");
                        }
                    } else {
                        logd("Standby in FDD modem");
                    }
                }
            } else if (sUserType == sType2User || sUserType == sType3User) {
                if (mdType == LteModemSwitchHandler.MD_TYPE_LWG
                        || mdType == LteModemSwitchHandler.MD_TYPE_WG) {
                    logd("Standby in FDD modem");
                } else {
                    logd("Should not enter this state");
                }
            } else {
                logd("Unknow user type");
            }
        } else {
            logd("IccState not ready");
        }
        logd("[handleNoService]-");

        return;
    }

    private Runnable mTddStandByTimerRunnable = new Runnable() {
        public void run() {
            sTddStandByCounter++;
            if (sTddStandByCounter >= TDD_STANDBY_TIMER.length) {
                sTddStandByCounter = TDD_STANDBY_TIMER.length - 1;
            }
            logd("TDD time out!");
            handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
        }
    };

    private Runnable mFddStandByTimerRunnable = new Runnable() {
        public void run() {
            sFddStandByCounter++;
            if (sFddStandByCounter >= FDD_STANDBY_TIMER.length) {
                sFddStandByCounter = FDD_STANDBY_TIMER.length - 1;
            }
            logd("FDD time out!");
            handleSwitchModem(LteModemSwitchHandler.MD_TYPE_TDD);
        }
    };

    private void removeModemStandByTimer() {
        if (sWaitInTdd) {
            logd("Remove TDD wait timer. Set sWaitInTdd = false");
            sWaitInTdd = false;
            removeCallbacks(mTddStandByTimerRunnable);
        }
        if (sWaitInFdd) {
            logd("Remove FDD wait timer. Set sWaitInFdd = false");
            sWaitInFdd = false;
            removeCallbacks(mFddStandByTimerRunnable);
        }
    }

    private void searchForDesignateService(String strPlmn) {
        if (strPlmn == null) {
            logd("[searchForDesignateService]- null source");
            return;
        }
        strPlmn = strPlmn.substring(0, 5);
        for (String mccmnc : PLMN_TABLE_TYPE1) {
            if (strPlmn.equals(mccmnc)) {
                logd("Find TD service");
                logd("sUserType:" + sUserType + " sRegion:" + sRegion);
                logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_TDD);
                break;
            }
        }

        return;
    }

    public void handleMessage(Message msg) {
        AsyncResult ar = (AsyncResult)msg.obj;
        switch (msg.what) {
            case EVENT_RADIO_ON_1:
                logd("handleMessage : <EVENT_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_REG_PLMN_CHANGED_1:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_1, false);
                break;
            case EVENT_REG_SUSPENDED_1:
                logd("handleMessage : <EVENT_REG_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_1, false);
                break;
            case EVENT_RADIO_ON_SGLTE_1:
                logd("handleMessage : <EVENT_RADIO_ON_SGLTE>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_REG_PLMN_CHANGED_SGLTE_1:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED_SGLTE>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_1, true);
                break;
            case EVENT_REG_SUSPENDED_SGLTE_1:
                logd("handleMessage : <EVENT_REG_SUSPENDED_SGLTE>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_1, true);
                break;
            case EVENT_RADIO_ON_2:
                logd("handleMessage : <EVENT_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_REG_PLMN_CHANGED_2:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_2, false);
                break;
            case EVENT_REG_SUSPENDED_2:
                logd("handleMessage : <EVENT_REG_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_2, false);
                break;
            case EVENT_RADIO_ON_SGLTE_2:
                logd("handleMessage : <EVENT_RADIO_ON_SGLTE>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_REG_PLMN_CHANGED_SGLTE_2:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED_SGLTE>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_2, true);
                break;
            case EVENT_REG_SUSPENDED_SGLTE_2:
                logd("handleMessage : <EVENT_REG_SUSPENDED_SGLTE>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_2, true);
                break;
            case EVENT_SET_RAT_GSM_ONLY:
                logd("handleMessage : <EVENT_SET_RAT_GSM_ONLY>");
                if (ar.exception == null) {
                    logd("Set RAT=2G only ok");
                } else {
                    logd("Set RAT=2G only fail " + ar.exception);
                }
                break;
            case EVENT_SET_RAT_WCDMA_PREF:
                logd("handleMessage : <EVENT_SET_RAT_WCDMA_PREF>");
                if (ar.exception == null) {
                    logd("Set RAT=2/3G auto ok");
                } else {
                    logd("Set RAT=2/3G auto fail " + ar.exception);
                }
                break;
            case EVENT_SET_RAT_3_4G_PREF:
                logd("handleMessage : <EVENT_SET_RAT_3_4G_PREF>");
                if (ar.exception == null) {
                    logd("Set RAT=2/3/4G auto ok");
                } else {
                    logd("Set RAT=2/3/4G auto fail " + ar.exception);
                }
                break;
            case EVENT_INVALID_SIM_NOTIFY_1:
                logd("handleMessage : <EVENT_INVALID_SIM_NOTIFY>");
                handleInvalidSimNotify(PhoneConstants.GEMINI_SIM_1, ar);
                break;
            case EVENT_INVALID_SIM_NOTIFY_2:
                logd("handleMessage : <EVENT_INVALID_SIM_NOTIFY>");
                handleInvalidSimNotify(PhoneConstants.GEMINI_SIM_2, ar);
                break;
            default:
                logd("Unknown msg:" + msg.what);
        }
    }

    private void handleInvalidSimNotify(int slotId, AsyncResult ar) {
        logd("Slot" + slotId);
        if (ar.exception == null && ar.result != null) {
            String[] invalidSimInfo = (String[]) ar.result;
            String plmn = invalidSimInfo[0];
            int cs_invalid = Integer.parseInt(invalidSimInfo[1]);
            int ps_invalid = Integer.parseInt(invalidSimInfo[2]);
            int cause = Integer.parseInt(invalidSimInfo[3]);
            int testMode = -1;
            testMode = SystemProperties.getInt("gsm.gcf.testmode", 0);
            if(testMode != 0) {
                logd("Invalid SIM notified during test mode: " + testMode);
                return;
            }
            logd("testMode:" + testMode + ", cause: " + cause + ", cs_invalid: " + cs_invalid + ", ps_invalid: " + ps_invalid + ", plmn: " + plmn);
            if(sVoiceCapable && cs_invalid == 1) {
                if(sLastPlmn == null) {
                    logd("CS reject, invalid SIM");
                    sIsInvalidSim[slotId] = true;
                    return;
                }
            }
            if(ps_invalid == 1) {
                if (sLastPlmn == null) {
                    logd("PS reject, invalid SIM");
                    sIsInvalidSim[slotId] = true;
                    return;
                }
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handleRadioOn(int slotId) {
        logd("Slot" + slotId);
        if (sMajorSlot == UNKNOWN_3G_SLOT) {
            sMajorSlot = getCapabilitySim();
        }
        setDesiredRatMode(slotId);
    }

    private void handlePlmnChange(AsyncResult ar, int slotId, boolean isDcEvent) {
        logd("Slot" + slotId);
        if (sMajorSlot == UNKNOWN_3G_SLOT) {
            sMajorSlot = getCapabilitySim();
        }
        if (ar.exception == null && ar.result != null) {
            String[] plmnString = (String[])ar.result;
            if (slotId == sMajorSlot) {
                if (isDcEvent) {
                    sNwPlmnStringsDc = plmnString;
                } else {
                    sNwPlmnStrings = plmnString;
                }
            }
            for (int i = 0; i < plmnString.length; i++) {
                logd("plmnString[" + i + "]=" + plmnString[i]);
            }
            if (sMajorSlot == slotId && sUserType == sType1User
                    && sDenyReason != DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD) {
                searchForDesignateService(plmnString[0]);
            }
            // To speed up performance in foreign countries, once get PLMN(no matter which slot)
            // determine region right away and switch modem type if needed
            sRegion = getRegion(plmnString[0]);
            if (sUserType != sType3User && sRegion == REGION_FOREIGN
                    && sMajorSlot != AUTO_SELECT_DISABLE && sMajorSlot != NO_3G_CAPABILITY) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handlePlmnSuspend(AsyncResult ar, int slotId, boolean isDcEvent) {
        logd("Slot" + slotId);
        if (ar.exception == null && ar.result != null) {
            if (isDcEvent) {
                sSuspendIdDc = ((int[]) ar.result)[0];
                logd("Suspending with Id=" + sSuspendIdDc);
            } else {
                sSuspendId = ((int[]) ar.result)[0];
                logd("Suspending with Id=" + sSuspendId);
            }
            if (sMajorSlot == slotId) {
                if (sUserType != sUnknownUser) {
                    resumeCampingProcedure(slotId, isDcEvent);
                } else {
                    if (isDcEvent) {
                        sSuspendWaitImsiDc[slotId] = true;
                    } else {
                        sSuspendWaitImsi[slotId] = true;
                    }
                    logd("User type unknown, wait for IMSI");
                }
            } else {
                logd("Not 3/4G slot, camp on OK");
                replyCampOn(slotId, isDcEvent);
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private final BroadcastReceiver mWorldPhoneReceiver = new  BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            logd("[Receiver]+");
            String action = intent.getAction();
            logd("Action: " + action);
            int slotId;
            if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, PhoneConstants.GEMINI_SIM_1);
                logd("slotId: " + slotId + " simStatus: " + simStatus);
                if (simStatus.equals(IccCardConstants.INTENT_VALUE_ICC_IMSI)) {
                    int currentMd = LteModemSwitchHandler.getActiveModemType();
                    if (sMajorSlot == UNKNOWN_3G_SLOT) {
                        sMajorSlot = getCapabilitySim();
                    }
                    sUiccController[slotId] = UiccController.getInstance(slotId);
                    if (sUiccController[slotId] != null) {
                        sIccRecordsInstance[slotId] = sUiccController[slotId].getIccRecords(UiccController.APP_FAM_3GPP);
                    } else {
                        logd("Null sUiccController");
                        return;
                    }
                    if (sIccRecordsInstance[slotId] != null) {
                        sImsi[slotId] = sIccRecordsInstance[slotId].getIMSI();
                    } else {
                        logd("Null sIccRecordsInstance");
                        return;
                    }
                    sIccCardType[slotId] = getIccCardType(slotId);
                    logd("sImsi[" + slotId + "]:" + sImsi[slotId]);
                    if (slotId == sMajorSlot) {
                        logd("3/4G slot");
                        sUserType = getUserType(sImsi[slotId]);
                        if (sFirstSelect[slotId]) {
                            sFirstSelect[slotId] = false;
                            if (sUserType == sType1User) {
                                if (sRegion == REGION_DOMESTIC) {
                                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_TDD);
                                } else if (sRegion == REGION_FOREIGN) {
                                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
                                } else {
                                    logd("Region unknown");
                                }
                            } else if (sUserType == sType2User || sUserType == sType3User) {
                                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
                            }
                        }
                        if (currentMd == LteModemSwitchHandler.getActiveModemType()) {
                            setDesiredRatMode(slotId);
                        }
                        if (sSuspendWaitImsi[slotId]) {
                            sSuspendWaitImsi[slotId] = false;
                            logd("IMSI fot slot" + slotId + " now ready, resuming PLMN:" 
                                    + sNwPlmnStrings[0] + " with ID:" + sSuspendId);
                            resumeCampingProcedure(slotId, false);
                        }
                        if (sSuspendWaitImsiDc[slotId]) {
                            sSuspendWaitImsiDc[slotId] = false;
                            logd("IMSI fot slot" + slotId + " now ready, resuming PLMN:" 
                                    + sNwPlmnStringsDc[0] + " with ID:" + sSuspendIdDc);
                            resumeCampingProcedure(slotId, true);
                        }
                    } else {
                        logd("Not 3/4G slot");
                        getUserType(sImsi[slotId]);
                    }
                } else if (simStatus.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
                    sLastPlmn = null;
                    sImsi[slotId] = "";
                    sFirstSelect[slotId] = true;
                    sIsInvalidSim[slotId] = false;
                    sSuspendWaitImsi[slotId] = false;
                    sSuspendWaitImsiDc[slotId] = false;
                    sIccCardType[slotId] = ICC_CARD_TYPE_UNKNOWN;
                    if (slotId == sMajorSlot) {
                        logd("3/4G Sim removed, no world phone service");
                        removeModemStandByTimer();
                        sUserType = sUnknownUser;
                        sDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
                        sMajorSlot = UNKNOWN_3G_SLOT;
                    } else {
                        logd("Slot" + slotId + " is not 3/4G slot");
                    }
                }
            } else if (action.equals(TelephonyIntents.EVENT_CAPABILITY_SWITCH_DONE)) {
                if (sMajorSlot != AUTO_SELECT_DISABLE) {
                    sMajorSlot = intent.getIntExtra(TelephonyIntents.EXTRA_CAPABILITY_SIM,
                            PhoneConstants.GEMINI_SIM_1);
                }
                handleSimSwitched();
            } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                sServiceState = ServiceState.newFromBundle(intent.getExtras());
                if (sServiceState != null) {
                    slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, PhoneConstants.GEMINI_SIM_1);
                    sPlmnSs = sServiceState.getOperatorNumeric();
                    sVoiceRegState = sServiceState.getVoiceRegState();
                    sRilVoiceRegState = sServiceState.getRilVoiceRegState();
                    sRilVoiceRadioTechnology = sServiceState.getRilVoiceRadioTechnology();
                    sDataRegState = sServiceState.getDataRegState();
                    sRilDataRegState = sServiceState.getRilDataRegState();
                    sRilDataRadioTechnology = sServiceState.getRilDataRadioTechnology();
                    logd("slotId: " + slotId + ", " + WorldPhoneUtil.iccCardTypeToString(sIccCardType[slotId]));
                    logd("sMajorSlot: " + sMajorSlot);
                    logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));
                    logd("sPlmnSs: " + sPlmnSs);
                    logd("sVoiceRegState: " + WorldPhoneUtil.stateToString(sVoiceRegState));
                    logd("sRilVoiceRegState: " + WorldPhoneUtil.regStateToString(sRilVoiceRegState));
                    logd("sRilVoiceRadioTech: " + sServiceState.rilRadioTechnologyToString(sRilVoiceRadioTechnology));
                    logd("sDataRegState: " + WorldPhoneUtil.stateToString(sDataRegState));
                    logd("sRilDataRegState: " + WorldPhoneUtil.regStateToString(sRilDataRegState));
                    logd("sRilDataRadioTech: " + sServiceState.rilRadioTechnologyToString(sRilDataRadioTechnology));
                    if (slotId == sMajorSlot) {
                        if (isNoService()) {
                            handleNoService();
                        } else if (isInService()) {
                            sLastPlmn = sPlmnSs;
                            removeModemStandByTimer();
                        }
                    }
                } else {
                    logd("Null sServiceState");
                }
            } else if (action.equals(TelephonyIntents.ACTION_SIM_INSERTED_STATUS)) {
                sSimInsertedStatus = intent.getIntExtra(SimInfoUpdate.INTENT_KEY_NEW_SIM_SLOT, -1);
                logd("sSimInsertedStatus: " + sSimInsertedStatus);
            } else if (action.equals(ACTION_SHUTDOWN_IPO)) {
                if (sDefaultBootuUpModem == LteModemSwitchHandler.MD_TYPE_UNKNOWN) {
                    if (sIsLteDcSupport
                            && LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LWG) {
                        LteModemSwitchHandler.reloadModem(sGsmCi[PhoneConstants.GEMINI_SIM_1], LteModemSwitchHandler.MD_TYPE_WG);
                        logd("Reload to WG modem");
                    }
                } else if (sDefaultBootuUpModem == LteModemSwitchHandler.MD_TYPE_TDD) {
                    if (sIsLteDcSupport) {
                        LteModemSwitchHandler.reloadModem(sGsmCi[PhoneConstants.GEMINI_SIM_1], LteModemSwitchHandler.MD_TYPE_LTNG);
                        logd("Reload to SGLTE modem");
                    } else {
                        LteModemSwitchHandler.reloadModem(sGsmCi[PhoneConstants.GEMINI_SIM_1], LteModemSwitchHandler.MD_TYPE_LTG);
                        logd("Reload to TDD CSFB modem");
                    }
                } else if (sDefaultBootuUpModem == LteModemSwitchHandler.MD_TYPE_FDD) {
                    if (sIsLteDcSupport) {
                        LteModemSwitchHandler.reloadModem(sGsmCi[PhoneConstants.GEMINI_SIM_1], LteModemSwitchHandler.MD_TYPE_WG);
                        logd("Reload to WG modem");
                    } else {
                        LteModemSwitchHandler.reloadModem(sGsmCi[PhoneConstants.GEMINI_SIM_1], LteModemSwitchHandler.MD_TYPE_LWG);
                        logd("Reload to FDD CSFB modem");
                    }
                }
            } else if (action.equals(ACTION_ADB_SWITCH_MODEM)) {
                int toModem = intent.getIntExtra(TelephonyIntents.INTENT_KEY_MD_TYPE, LteModemSwitchHandler.MD_TYPE_UNKNOWN);
                logd("toModem: " + toModem);
                if (toModem == LteModemSwitchHandler.MD_TYPE_WG
                        || toModem == LteModemSwitchHandler.MD_TYPE_TG
                        || toModem == LteModemSwitchHandler.MD_TYPE_LWG
                        || toModem == LteModemSwitchHandler.MD_TYPE_LTG
                        || toModem == LteModemSwitchHandler.MD_TYPE_LTNG) { 
                    setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, toModem);
                } else {
                    setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_AUTO, toModem);
                }
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                if (intent.getBooleanExtra("state", false) == false) {
                    logd("Leave flight mode");
                    sLastPlmn = null;
                    for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                        sIsInvalidSim[i] = false;
                    }
                } else {
                    logd("Enter flight mode");
                    for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                        sFirstSelect[i] = true;
                    }
                }
            }
            logd("[Receiver]-");
        }
    };

    private boolean isInService() {
        boolean inService = false;

        if (sVoiceRegState == ServiceState.STATE_IN_SERVICE
                || sDataRegState == ServiceState.STATE_IN_SERVICE) {
            inService = true;
        }
        logd("inService: " + inService);

        return inService;
    }

    private boolean isNoService() {
        boolean noService = false;

        if (sVoiceRegState == ServiceState.STATE_OUT_OF_SERVICE
                && sRilVoiceRegState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING
                && sDataRegState == ServiceState.STATE_OUT_OF_SERVICE) {
            if (LteModemSwitchHandler.isLteDcMode()) {
                if (sRilDataRegState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING
                        || sRilDataRegState == ServiceState.REGISTRATION_STATE_UNKNOWN) {
                    noService = true;
                }
            } else {
                if (sRilDataRegState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING) {
                    noService = true;
                }
            }
        } else {
            noService = false;
        }
        logd("noService: " + noService);

        return noService;
    }

    private void handleSimSwitched() {
        if (sMajorSlot == NO_3G_CAPABILITY) {
            logd("3/4G capability turned off");
            removeModemStandByTimer();
            sUserType = sUnknownUser;
        } else if (sMajorSlot == AUTO_SELECT_DISABLE) {
            logd("Auto Network Selection Disabled");
            removeModemStandByTimer();
        } else if (sMajorSlot == UNKNOWN_3G_SLOT) {
            logd("3/4G SIM unknown");
        } else {
            logd("Auto Network Selection Enabled");
            logd("3/4G capability in slot" + sMajorSlot);
            if (sImsi[sMajorSlot].equals("")) {
                // may caused by receive 3g switched intent when boot up
                logd("3/4G slot IMSI not ready");
                sUserType = sUnknownUser;
                return;
            }
            int oldMdType = LteModemSwitchHandler.getActiveModemType();
            sUserType = getUserType(sImsi[sMajorSlot]);
            if (sUserType == sType1User) {
                if (sNwPlmnStrings != null) {
                    sRegion = getRegion(sNwPlmnStrings[0]);
                } else if (sNwPlmnStringsDc != null) {
                    sRegion = getRegion(sNwPlmnStringsDc[0]);
                }
                if (sRegion == REGION_DOMESTIC) {
                    sFirstSelect[sMajorSlot] = false;
                    sIccCardType[sMajorSlot] = getIccCardType(sMajorSlot);
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_TDD);
                } else if (sRegion == REGION_FOREIGN) {
                    sFirstSelect[sMajorSlot] = false;
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
                } else {
                    logd("Unknown region");
                }
            } else if (sUserType == sType2User || sUserType == sType3User) {
                sFirstSelect[sMajorSlot] = false;
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
            } else {
                logd("Unknown user type");
            }
            int newMdType = LteModemSwitchHandler.getActiveModemType();
            if (oldMdType == newMdType) {
                setDesiredRatMode(sMajorSlot);
            }
        }
    }

    public void setNetworkSelectionMode(int mode, int modemType) {
        Settings.Global.putInt(sContext.getContentResolver(), 
                Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, mode);
        if (mode == SELECTION_MODE_AUTO) {
            logd("Network Selection <AUTO>");
            sMajorSlot = getCapabilitySim();
            handleSimSwitched();
        } else {
            logd("Network Selection <MANUAL>");
            sMajorSlot = AUTO_SELECT_DISABLE;
            handleSwitchModem(modemType);
            if (modemType == LteModemSwitchHandler.getActiveModemType()) {
                removeModemStandByTimer();
                setDesiredRatMode(getCapabilitySim());
            }
        }
    }

    private void handleSwitchModem(int toModem) {
        if (sIsInvalidSim[GeminiPhone.get3GSimId()] && sMajorSlot != AUTO_SELECT_DISABLE) {
            logd("Invalid SIM, switch not executed!");
            return;
        }
        if (toModem == LteModemSwitchHandler.MD_TYPE_TDD) {
            if (sIsLteDcSupport) {
                if (sIccCardType[sMajorSlot] == ICC_CARD_TYPE_USIM) {
                    int userPreferredRat = getPreferredRatMode(PREFERRED_RAT_USER);
                    if (userPreferredRat == Phone.NT_MODE_WCDMA_PREF
                            || userPreferredRat == Phone.NT_MODE_GSM_UMTS) {
                        toModem = LteModemSwitchHandler.MD_TYPE_TG;
                    } else {
                        toModem = LteModemSwitchHandler.MD_TYPE_LTNG;
                    }
                } else if (sIccCardType[sMajorSlot] == ICC_CARD_TYPE_SIM) {
                    toModem = LteModemSwitchHandler.MD_TYPE_TG;
                }
            } else {
                toModem = LteModemSwitchHandler.MD_TYPE_LTG;
            }
        } else if (toModem == LteModemSwitchHandler.MD_TYPE_FDD) {
            if (sIsLteDcSupport) {
                if (sSimInsertedStatus == STATUS_SIM1_INSERTED) {
                    toModem = LteModemSwitchHandler.MD_TYPE_LWG;
                } else {
                    toModem = LteModemSwitchHandler.MD_TYPE_WG;
                }
            } else {
                toModem = LteModemSwitchHandler.MD_TYPE_LWG;
            }
        }
        if (sMajorSlot == AUTO_SELECT_DISABLE) {
            logd("Storing modem type: " + toModem);
            sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(toModem, null);
        } else {
            if (sDefaultBootuUpModem == LteModemSwitchHandler.MD_TYPE_UNKNOWN) {
                if (sIsLteDcSupport && toModem == LteModemSwitchHandler.MD_TYPE_LWG) {
                    logd("Storing modem type: " + LteModemSwitchHandler.MD_TYPE_WG);
                    sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(LteModemSwitchHandler.MD_TYPE_WG, null);
                } else {
                    logd("Storing modem type: " + toModem);
                    sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(toModem, null);
                }
            } else if (sDefaultBootuUpModem == LteModemSwitchHandler.MD_TYPE_FDD) {
                if (sIsLteDcSupport) {
                    logd("Storing modem type: " + LteModemSwitchHandler.MD_TYPE_WG);
                    sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(LteModemSwitchHandler.MD_TYPE_WG, null);
                } else {
                    logd("Storing modem type: " + LteModemSwitchHandler.MD_TYPE_LWG);
                    sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(LteModemSwitchHandler.MD_TYPE_LWG, null);
                }
            } else if (sDefaultBootuUpModem == LteModemSwitchHandler.MD_TYPE_TDD) {
                if (sIsLteDcSupport) {
                    logd("Storing modem type: " + LteModemSwitchHandler.MD_TYPE_LTNG);
                    sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(LteModemSwitchHandler.MD_TYPE_LTNG, null);
                } else {
                    logd("Storing modem type: " + LteModemSwitchHandler.MD_TYPE_LTG);
                    sGsmCi[PhoneConstants.GEMINI_SIM_1].storeModemType(LteModemSwitchHandler.MD_TYPE_LTG, null);
                }
            }
        }
        if (toModem == LteModemSwitchHandler.getActiveModemType()) {
            if (toModem == LteModemSwitchHandler.MD_TYPE_WG) {
                logd("Already in WG modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_TG) {
                logd("Already in TG modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LWG) {
                logd("Already in FDD CSFB modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LTG) {
                logd("Already in TDD CSFB modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LTNG) {
                logd("Already in SGLTE modem");
            }
            return;
        } else {
            for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                if (sGsmPhone[i].getState() != PhoneConstants.State.IDLE) {
                    logd("Phone" + i + " is not idle, modem switch not allowed");
                    return;
                }
            }
            removeModemStandByTimer();
            if (toModem == LteModemSwitchHandler.MD_TYPE_WG
                    || toModem == LteModemSwitchHandler.MD_TYPE_TG) {
                if (toModem == LteModemSwitchHandler.MD_TYPE_WG) {
                    logd("Switching to WG modem");
                } else {
                    logd("Switching to TG modem");
                }
                if (sIsLteDcSupport) {
                    int slotId = sMajorSlot;
                    if (sMajorSlot == AUTO_SELECT_DISABLE) {
                        slotId = getCapabilitySim();
                    }
                    if (slotId != NO_3G_CAPABILITY && slotId != UNKNOWN_3G_SLOT) {
                        int ratToSet = getPreferredRatMode(PREFERRED_RAT_USER);
                        if (ratToSet == -1) {
                            ratToSet = Phone.NT_MODE_WCDMA_PREF;
                        } else {
                            if (contain4gRat(ratToSet)) {
                                ratToSet = Phone.NT_MODE_WCDMA_PREF;
                            }
                        }
                        logd("ratToSet: " + ratToSet);
                        Settings.Global.putInt(sContext.getContentResolver(),
                                Settings.Global.PREFERRED_NETWORK_MODE, ratToSet);
                        sGsmCi[slotId].setPreferredNetworkTypeRIL(ratToSet);
                    } else {
                        logd("Capability SIM unavailable -> value: " + slotId);
                    }
                }
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LWG) {
                logd("Switching to FDD CSFB modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LTG) {
                logd("Switching to TDD CSFB modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LTNG) {
                logd("Switching to SGLTE modem");
            }
            sLteModemSwitchHandler.switchModem(toModem);
            resetNetworkProperties();
        }
    }

    private void resumeCampingProcedure(int slotId, boolean isDcEvent) {
        logd("Resume camping slot" + slotId);
        String plmnString;
        if (isDcEvent) {
            plmnString = sNwPlmnStringsDc[0];
        } else {
            plmnString = sNwPlmnStrings[0];
        }
        if (isAllowCampOn(plmnString, slotId)) {
            removeModemStandByTimer();
            replyCampOn(slotId, isDcEvent);
        } else {
            logd("Because: " + WorldPhoneUtil.denyReasonToString(sDenyReason));
            if (sDenyReason == DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_FDD);
            } else if (sDenyReason == DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_TDD);
            }
        }
    }

    private void replyCampOn(int slotId, boolean isDcEvent) {
        if (isDcEvent) {
            sLteCi[slotId].setResumeRegistration(sSuspendIdDc, null);
        } else {
            sGsmCi[slotId].setResumeRegistration(sSuspendId, null);
        }
    }

    private int getCapabilitySim() {
        //int majorSlot = sPhone.getCapabilitySIM(TelephonyCapabilities.CAPABILITY_34G);
        int majorSlot = sPhone.get3GCapabilitySIM();
        logd("Major slot = " + majorSlot);

        return majorSlot;
    }

    private int getIccCardType(int slotId) {
        int simType;
        String simString = "Unknown";

        if (PhoneFactory.isGeminiSupport()) {
            simString = ((GeminiPhone)sPhone).getPhonebyId(slotId).getIccCard().getIccCardType();
        } else {
            simString = sPhone.getIccCard().getIccCardType();
        }
        if (simString.equals("SIM")) {
            logd("IccCard type: SIM");
            simType = ICC_CARD_TYPE_SIM;
        } else if (simString.equals("USIM")) {
            logd("IccCard type: USIM");
            simType = ICC_CARD_TYPE_USIM;
        } else {
            logd("IccCard type: Unknown");
            simType = ICC_CARD_TYPE_UNKNOWN;
        }

        return simType;
    }

    public int onNetworkModeChanged(int currentNwMode, int newNwMode) {
        logd("[onNetworkModeChanged]+ currentNwMode: " + currentNwMode + ", newNwMode: " + newNwMode);
        logd("sMajorSlot = " + sMajorSlot);
        logd("sUserType = " + sUserType);
        logd("sRegion = " + WorldPhoneUtil.regionToString(sRegion));
        int mdType = LteModemSwitchHandler.getActiveModemType();
        logd(LteModemSwitchHandler.modemToString(mdType));
        if (sMajorSlot == AUTO_SELECT_DISABLE
                || sMajorSlot == NO_3G_CAPABILITY
                || sMajorSlot == UNKNOWN_3G_SLOT) {
            logd("3/4G capability disabled, ignore");
            return mdType;
        } else {
            if (sIsLteDcSupport && sUserType == sType1User
                    && sIccCardType[sMajorSlot] == ICC_CARD_TYPE_USIM) {
                if (mdType == LteModemSwitchHandler.MD_TYPE_LTNG) {
                    if (newNwMode == Phone.NT_MODE_WCDMA_PREF
                            || newNwMode == Phone.NT_MODE_GSM_UMTS) {
                        handleSwitchModem(LteModemSwitchHandler.MD_TYPE_TG);
                        mdType = LteModemSwitchHandler.MD_TYPE_TG;
                    }
                } else if (mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                    if (newNwMode == Phone.NT_MODE_LTE_GSM_WCDMA) {
                        handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTNG);
                        mdType = LteModemSwitchHandler.MD_TYPE_LTNG;
                    }
                } else {
                    logd("Ignore");
                }
            } else {
                logd("Ignore");
            }
        }
        logd("[onNetworkModeChanged]- mdType: " + mdType);

        return mdType;
    }

    private static int getModemSelectionMode() {
        return Settings.Global.getInt(sContext.getContentResolver(), 
                    Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, SELECTION_MODE_AUTO);
    }

    private int getPreferredRatMode(int type) {
        int mode;
        switch (type) {
            case PREFERRED_RAT_USER:
                mode = Settings.Global.getInt(sContext.getContentResolver(),
                                Settings.Global.USER_PREFERRED_NETWORK_MODE, -1);
                break;
            case PREFERRED_RAT_SYSTEM:
                mode = Settings.Global.getInt(sContext.getContentResolver(),
                                Settings.Global.PREFERRED_NETWORK_MODE, Phone.PREFERRED_NT_MODE);
                break;
            default:
                mode = -1;
                break;
        }
        return mode;
    }

    private void setDesiredRatMode(int slotId) {
        logd("[setDesiredRatMode]+");
        int majorSim = getCapabilitySim();
        int protocolSim = GeminiPhone.get3GSimId();
        logd("slotId: " + slotId);
        logd("[" + majorSim + ", " + protocolSim + "]");
        if (getModemSelectionMode() == SELECTION_MODE_AUTO) {
            if (majorSim == NO_3G_CAPABILITY) {
                logd("3/4G capability off");
                logd("[setDesiredRatMode]-");
                return;
            } else if (majorSim == slotId) {
                if (sUserType == sUnknownUser) {
                    logd("User type unknown");
                    logd("[setDesiredRatMode]-");
                    return;
                }
                if (sIccCardType[slotId] == ICC_CARD_TYPE_UNKNOWN) {
                    logd("Icc card type unknown");
                    logd("[setDesiredRatMode]-");
                    return;
                }
            } else {
                logd("Not 3/4G slot");
                logd("[setDesiredRatMode]-");
                return;
            }
        } else {
            if (majorSim == NO_3G_CAPABILITY) {
                logd("3/4G capability off");
                if (slotId != protocolSim) {
                    logd("[setDesiredRatMode]-");
                    return;
                }
            } else if (majorSim == slotId) {
                if (sIccCardType[slotId] == ICC_CARD_TYPE_UNKNOWN) {
                    logd("Icc card type unknown");
                    logd("[setDesiredRatMode]-");
                    return;
                }
            } else {
                logd("Not 3/4G slot");
                logd("[setDesiredRatMode]-");
                return;
            }
        }
        int mdType = LteModemSwitchHandler.getActiveModemType();
        int ratToSet = getPreferredRatMode(PREFERRED_RAT_USER);
        logd("ratToSet: " + ratToSet);
        if (ratToSet == -1) {
            if (sIccCardType[slotId] == ICC_CARD_TYPE_USIM) {
                if (mdType == LteModemSwitchHandler.MD_TYPE_WG
                        || mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                    setRatMode(Phone.NT_MODE_WCDMA_PREF, slotId);
                } else if (mdType == LteModemSwitchHandler.MD_TYPE_LWG
                        || mdType == LteModemSwitchHandler.MD_TYPE_LTG
                        || mdType == LteModemSwitchHandler.MD_TYPE_LTNG) {
                    setRatMode(Phone.NT_MODE_LTE_GSM_WCDMA, slotId);
                }
            } else if (sIccCardType[slotId] == ICC_CARD_TYPE_SIM) {
                setRatMode(Phone.NT_MODE_WCDMA_PREF, slotId);
            }
        } else {
            if (contain4gRat(ratToSet)) {
                if (sIccCardType[slotId] == ICC_CARD_TYPE_SIM
                        || mdType == LteModemSwitchHandler.MD_TYPE_WG
                        || mdType == LteModemSwitchHandler.MD_TYPE_TG) {
                    setRatMode(Phone.NT_MODE_WCDMA_PREF, slotId);
                } else {
                    setRatMode(ratToSet, slotId);
                }
            } else {
                setRatMode(ratToSet, slotId);
            }
        }
        logd("[setDesiredRatMode]-");
    }

    private void setRatMode(int ratMode, int slotId) {
        int mCurrentNetworkMode = getPreferredRatMode(PREFERRED_RAT_SYSTEM);
        if (ratMode == mCurrentNetworkMode) {
            logd("[setRatMode] Already in desired rat mode: " + mCurrentNetworkMode);
            return;
        }
        int eventId = 0;
        if (ratMode == Phone.NT_MODE_LTE_GSM_WCDMA) {
            logd("[setRatMode] Setting slot" + slotId + " RAT=2/3/4G auto");
            eventId = EVENT_SET_RAT_3_4G_PREF;
        } else if (ratMode == Phone.NT_MODE_WCDMA_PREF) {
            logd("[setRatMode] Setting slot" + slotId + " RAT=2/3G auto");
            eventId = EVENT_SET_RAT_WCDMA_PREF;
        } else if (ratMode == Phone.NT_MODE_GSM_ONLY) {
            logd("[setRatMode] Setting slot" + slotId + " RAT=2G only");
            eventId = EVENT_SET_RAT_GSM_ONLY;
        }
        if (slotId == GeminiPhone.get3GSimId()) {
            Settings.Global.putInt(sContext.getContentResolver(),
                    Settings.Global.PREFERRED_NETWORK_MODE, ratMode);
        }
        sGsmPhone[slotId].setPreferredNetworkType(ratMode, obtainMessage(eventId));
    }

    private boolean contain4gRat(int ratMode) {
        if (ratMode == Phone.NT_MODE_LTE_GSM_WCDMA
                || ratMode == Phone.NT_MODE_GSM_WCDMA_LTE
                || ratMode == Phone.NT_MODE_LTE_ONLY
                || ratMode == Phone.NT_MODE_LTE_WCDMA
                || ratMode == Phone.NT_MODE_LTE_GSM) {
            return true;
        }
        return false;
    }

    private void resetAllProperties() {
        logd("[resetAllProperties]");
        sNwPlmnStrings = null;
        sNwPlmnStringsDc = null;
        for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            sFirstSelect[i] = true;
        }
        sDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
        resetSimProperties();
        resetNetworkProperties();
    }

    private void resetNetworkProperties() {
        logd("[resetNetworkProperties]");
        synchronized (sLock) {
            for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                sSuspendWaitImsi[i] = false;
                sSuspendWaitImsiDc[i] = false;
            }
        }
    }

    private void resetSimProperties() {
        logd("[resetSimProperties]");
        synchronized (sLock) {
            for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                sImsi[i] = "";
                sIccCardType[i] = ICC_CARD_TYPE_UNKNOWN;
                sIsInvalidSim[i] = false;
            }
            sUserType = sUnknownUser;
            if (PhoneFactory.isSimSwitchSupport()) {
                sMajorSlot = UNKNOWN_3G_SLOT;
            } else {
                sMajorSlot = DEFAULT_3G_SLOT;
            }
        }
    }

    private int getUserType(String imsi) {
        if (imsi != null && !imsi.equals("")) {
            imsi = imsi.substring(0, 5);
            for (String mccmnc : PLMN_TABLE_TYPE1) {
                if (imsi.equals(mccmnc)) {
                    logd("[getUserType] Type1 user");
                    return sType1User;
                }
            }
            for (String mccmnc : PLMN_TABLE_TYPE3) {
                if (imsi.equals(mccmnc)) {
                    logd("[getUserType] Type3 user");
                    return sType3User;
                }
            }
            logd("[getUserType] Type2 user");
            return sType2User;
        } else {
            logd("[getUserType] null imsi");
            return sUnknownUser;
        }
    }

    private int getRegion(String srcMccOrPlmn) {
        String currentMcc;
        if (srcMccOrPlmn == null) {
            logd("[getRegion] null source");
            return REGION_UNKNOWN;
        }
        currentMcc = srcMccOrPlmn.substring(0, 3);
        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (currentMcc.equals(mcc)) {
                logd("[getRegion] REGION_DOMESTIC");
                return REGION_DOMESTIC;
            }
        }
        logd("[getRegion] REGION_FOREIGN");
        return REGION_FOREIGN;
    }

    public void disposeWorldPhone() {
        sContext.unregisterReceiver(mWorldPhoneReceiver);
        for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            sGsmCi[i].unSetOnPlmnChangeNotification(this);
            sGsmCi[i].unSetOnGSMSuspended(this);
            sGsmCi[i].unregisterForOn(this);
            if (sIsLteDcSupport) {
                sLteCi[i].unSetOnPlmnChangeNotification(this);
                sLteCi[i].unSetOnGSMSuspended(this);
                sLteCi[i].unregisterForOn(this);
            }
        }
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[LteWPOM]" + msg);
    }
}
