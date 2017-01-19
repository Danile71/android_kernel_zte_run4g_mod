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

package com.mediatek.csm;

import android.app.PendingIntent;
import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.ServiceState;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.IPhoneStateListener;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.telephony.SmsManagerEx;
import com.mediatek.telephony.TelephonyManagerEx;

public class PhoneManager {
    private static ITelephony sPhone;
    private static ITelephonyEx sPhoneEx;
    private static ITelephonyRegistry sPhoneRegistry;
    private static ITelephonyRegistry sPhoneRegistry2;

    private PhoneManager() {}

    public static boolean isSimInserted(int simId) {
        boolean ret = false;
        try {
            ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager
                    .getService(Context.TELEPHONY_SERVICEEX));
            ret = iTel.hasIccCard(simId);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return ret;
    }

    public static boolean isRadioOn(int simId) {
        boolean ret = false;
        try {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                ret = getITelephonyEx().isRadioOn(simId);
            } else {
                ret = getITelephony().isRadioOn();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return ret;
    }

    public static int getSimState(int simId) {
        int state = TelephonyManager.SIM_STATE_UNKNOWN;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            state = TelephonyManagerEx.getDefault().getSimState(simId);
        } else {
            state = TelephonyManager.getDefault().getSimState();
        }
        return state;
    }

    public static int getPhoneType(int simId) {
        int type = TelephonyManager.PHONE_TYPE_NONE;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            type = TelephonyManagerEx.getDefault().getPhoneType(simId);
        } else {
            type = TelephonyManager.getDefault().getPhoneType();
        }
        return type;
    }

    public static int getCallState(int simId) {
        int state = TelephonyManager.CALL_STATE_IDLE;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            state = TelephonyManagerEx.getDefault().getCallState(simId);
        } else {
            state = TelephonyManager.getDefault().getCallState();
        }
        return state;
    }

    public static void endCall(int simId) {
        try {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                getITelephony().endCallGemini(simId);
            } else {
                getITelephony().endCall();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public static void sendTextMessage(String addr, String msg, int simId,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            SmsManagerEx.getDefault().sendTextMessage(addr, null, msg,
                    sentIntent, deliveryIntent, simId);
        } else {
            SmsManager.getDefault().sendTextMessage(addr, null, msg,
                    sentIntent, deliveryIntent);
        }
    }

    /**
     * Just only support master sim slot up to now
     *
     * @param turnOn
     */
    public static void setRadioPower(boolean turnOn) {
        try {
            getITelephony().setRadio(turnOn);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public static int getServiceState(int simId) {
        int ret = ServiceState.STATE_IN_SERVICE;
        Bundle data = null;
        try {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                data = getITelephonyEx().getServiceState(simId);
            } else {
                data = getITelephony().getServiceState();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (data != null) {
            ret = data.getInt("state");
        }
        return ret;
    }

    public static void listen(String pkg, IPhoneStateListener callback, int events,
            boolean notifyNow, int simId) {
        try {
            getITelephonyRegistry(simId).listen(pkg, callback, events, notifyNow);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public static ITelephony getITelephony() {
        if (sPhone == null) {
            sPhone = ITelephony.Stub.asInterface(ServiceManager
                    .getService(Context.TELEPHONY_SERVICE));
        }
        return sPhone;
    }

    public static ITelephonyEx getITelephonyEx() {
        if (sPhoneEx == null) {
            sPhoneEx = ITelephonyEx.Stub.asInterface(ServiceManager
                    .getService(Context.TELEPHONY_SERVICEEX));
        }
        return sPhoneEx;
    }

    public static ITelephonyRegistry getITelephonyRegistry(int slot) {
        ITelephonyRegistry registry = null;
        if (slot == PhoneConstants.GEMINI_SIM_1) {
            if (sPhoneRegistry == null) {
                sPhoneRegistry = ITelephonyRegistry.Stub.asInterface(ServiceManager
                        .getService("telephony.registry"));
            }
            registry = sPhoneRegistry;
        } else if (slot == PhoneConstants.GEMINI_SIM_2) {
            if (sPhoneRegistry2 == null) {
                sPhoneRegistry2 = ITelephonyRegistry.Stub.asInterface(ServiceManager
                        .getService("telephony.registry2"));
            }
            registry = sPhoneRegistry2;
        }

        return registry;
    }
}
