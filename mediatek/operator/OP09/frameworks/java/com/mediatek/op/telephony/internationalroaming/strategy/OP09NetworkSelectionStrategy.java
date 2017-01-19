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

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.ServiceState;
import android.text.TextUtils;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.common.telephony.internationalroaming.IInternationalRoamingController;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.telephony.TelephonyManagerEx;

public class OP09NetworkSelectionStrategy extends DefaultNetworkSelectionStrategy {
    private static final String TAG = "[OP09NWSelectionStrategy]";

    private static final int EVENT_GSM2_VOICE_CALL_ENDED = 0;

    // OP09 new spec(2014-04-14) requires the device not to register on CDMA
    // network except some specified nations, Japan and South Korea is in the
    // list for GSM prefered device.
    private static final String[] OPERATOR_SPECIFIC_SUPPORT_CDMA_NATION = {
            "440", "450"
    };

    private CommandsInterface mDualModePhoneCM;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_GSM2_VOICE_CALL_ENDED:
                    handleGsm2CallEnd();
                    break;
                    
                default:
                    logd("Should never run into this case: msg = " + msg);
                    break;
            }
        }
    };

    public OP09NetworkSelectionStrategy(IInternationalRoamingController controller,
            Context context, Phone dualModePhone, Phone gsmPhone) {
        super(controller, context, dualModePhone, gsmPhone);

        mDualModePhoneCM = ((PhoneBase) (((PhoneProxy) dualModePhone).getActivePhone())).mCi;
    }

    @Override
    public boolean needToBootOnGsm() {
        logd("needToBootOnGsm...");
        return false;
    }

    @Override
    public boolean needToBootOnCdma() {
        logd("needToBootOnCdma: hasSearchedOnCdma = " + mController.hasSearchedOnCdma());
        return false;
    }

    @Override
    public int onPreSwitchPhone() {
        logd("onPreSwitchPhone...");

        return InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS;
    }

    @Override
    public void onPostSwitchPhone() {
        logd("onPostSwitchPhone...");
    }

    @Override
    public void onGsmSuspend(String[] plmnString, int suspendedSession) {
        for (int i = 0; i < plmnString.length; i++) {
            logd("onGsmSuspend: i = " + i + ", plmnString=" + plmnString[i]
                    + ",suspendedSession = " + suspendedSession);
            if (mController.isHomeNetwork(plmnString[i])) {
                logd("Switch to CDMA mode because current is in home land: " + plmnString[i]);
                // We don't check whether switch phone successfully, it means
                // there maybe bug exists if it failes, we need to check the log
                // to fix it in that case.
                mController.blockSwitchPhone(false);
                mController.switchPhone(InternationalRoamingConstants.SIM_SWITCH_MODE_CDMA, false);
                return;
            }
        }

        mController.resumeRegistration(InternationalRoamingConstants.RESUME_NW_GSM,
                suspendedSession);
    }

    @Override
    public void onCdmaPlmnChanged(String plmnNumericCdma) {
        logd("onCdmaPlmnChanged: plmnNumericCdma = " + plmnNumericCdma);
        if ((plmnNumericCdma.startsWith("2134") && plmnNumericCdma.length() == 7)
                || plmnNumericCdma.startsWith("0000")) {
            // Re-get plmn for special operator which doesn't release plmn when
            // network searched.
            plmnNumericCdma = ((PhoneBase) (((PhoneProxy) mDualModePhone).getActivePhone()))
                    .checkMccBySidLtmOff(plmnNumericCdma);
            logd("checkMccBySidLtmOff: new  plmnNumericCdma = " + plmnNumericCdma);
        }

        if (isCdmaResumeAllowed(plmnNumericCdma)) {
            logd("Resume CDMA register.");
            mController.resumeRegistration(InternationalRoamingConstants.RESUME_NW_CDMA, 0);
        } else {
            if (isGsm2InVoiceCall()) {
                logd("Pending switch phone to GSM due to GSM(2) is in voice call, switch phone after call ended.");
                // Register for GSM(2) voice call end.
                ((PhoneBase) ((PhoneProxy) mGsmPhone).getActivePhone()).getCallTracker()
                        .unregisterForVoiceCallEnded(mHandler);
                ((PhoneBase) ((PhoneProxy) mGsmPhone).getActivePhone()).getCallTracker()
                        .registerForVoiceCallEnded(mHandler, EVENT_GSM2_VOICE_CALL_ENDED, null);
            } else {
                final int ret = mController.switchPhone(
                        InternationalRoamingConstants.SIM_SWITCH_MODE_GSM, false);
                logd("CDMA 2nd network selection try to switch to GSM mode: ret = " + ret);

                // If we switch phone failed due to single card or other
                // conditions, resume registration.
                if (ret != InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS) {
                    mController.resumeRegistration(InternationalRoamingConstants.RESUME_NW_CDMA, 0);
                }
            }
        }
    }

    @Override
    public void onNoService(int phoneType) {
        logd("onNoService: phoneType = " + phoneType);

        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            // If GSM2 no service or register in home network or in a voice
            // call, keep searching with current mode.
            if (isGsm2NoService() || isGsm2RegInHome() || isGsm2InVoiceCall()) {
                logd("Keep searching CDMA signal due to there is no GSM service"
                        + "or GSM(2) is registered in home or GSM(2) is in a voice call.");
                return;
            }
        }

        mController.switchPhone(InternationalRoamingConstants.SIM_SWITCH_MODE_INVERSE, false);
    }

    /**
     * Whether we should resume CDMA registration, Force switch phone by
     * user(Settings) or has searched on GSM with no signal.
     * 
     * @param plmnNumericCdma
     * @return True if CDMA resume allowed.
     */
    private boolean isCdmaResumeAllowed(String plmnNumericCdma) {
        final boolean isCdmaResumeAllowed = isSingleCdmaCard() 
                || invaldOperatorNumeric(plmnNumericCdma)
                || isHomeNetwork(plmnNumericCdma)
                || isCdmaRegAllowedInRoaming(plmnNumericCdma);
        logd("isCdmaResumeAllowed: isCdmaResumeAllowed = " + isCdmaResumeAllowed);
        return isCdmaResumeAllowed;
    }

    /**
     * Whether the inserted SIM is single CDMA card.
     * 
     * @return
     */
    private boolean isSingleCdmaCard() {
        return mController.getDualModePhoneCardType() == InternationalRoamingConstants.CARD_TYPE_SINGLE_CDMA;
    }

    /**
     * @param plmnNumericCdma
     * @return
     */
    private boolean invaldOperatorNumeric(String plmnNumericCdma) {
        return TextUtils.isEmpty(plmnNumericCdma) || !Character.isDigit(plmnNumericCdma.charAt(0))
                || plmnNumericCdma.startsWith("000");
    }

    /**
     * Whether the suspend network is home network.
     * 
     * @param plmnNumericCdma
     * @return
     */
    private boolean isHomeNetwork(String plmnNumericCdma) {
        logd("isHomeNetwork: " + mController.isHomeNetwork(plmnNumericCdma));
        return mController.isHomeNetwork(plmnNumericCdma);
    }

    /**
     * We need to keep in CDMA mode if GSM phone(SIM2) is in no service state
     * due to OP09's new requirements.
     * 
     * @return True if GSM(2) is in no service state with SIM inserted, or else
     *         false.
     */
    private boolean isGsm2NoService() {
        // Whether there is SIM inserted but no service for GSM phone.
        if (TelephonyManagerEx.getDefault().hasIccCard(mGsmPhone.getMySimId())
                && mGsmPhone.getServiceState().getRegState() ==
                ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING) {
            logd("GSM(2) is in no service state with SIM inserted.");
            return true;
        }

        return false;
    }

    /**
     * We need to keep in CDMA mode if GSM(2) has registered in home network due
     * to OP09's new requirements.
     * 
     * @return True if the GSM2 phone is registered in home, or else false.
     */
    private boolean isGsm2RegInHome() {
        if (mGsmPhone.getServiceState().getRegState() == ServiceState.REGISTRATION_STATE_HOME_NETWORK
                || mGsmPhone.getServiceState().getRegState() == ServiceState.REGISTRATION_STATE_ROAMING) {
            String plmnNumericGsm = SystemProperties
                    .get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC_2);
            if (mController.isHomeNetwork(plmnNumericGsm)) {
                logd("GSM(2) is regestered in home network, plmn = " + plmnNumericGsm);
                return true;
            }
        }

        return false;
    }

    /**
     * Whether the GSM phone is in voice call.
     * 
     * @return True if the GSM2 phone is in voice call, or else false.
     */
    private boolean isGsm2InVoiceCall() {
        logd("GSM(2) call sate = " + mGsmPhone.getState());
        return mGsmPhone.getState() != PhoneConstants.State.IDLE;
    }

    private void handleGsm2CallEnd() {
        ((PhoneBase) ((PhoneProxy) mGsmPhone).getActivePhone()).getCallTracker()
                .unregisterForVoiceCallEnded(mHandler);

        // We need to switch phone to GSM because the GSM is
        // preferred when in roaming network, We are delayed because
        // of voice call in GSM2.
        final int phoneType = mDualModePhone.getPhoneType();
        logd("Handle EVENT_GSM2_VOICE_CALL_ENDED: phoneType = " + phoneType);
        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (!isGsm2RegInHome()) {
                mController.switchPhone(InternationalRoamingConstants.SIM_SWITCH_MODE_GSM, false);
            }
        }
    }

    /**
     * Whether CDMA network registeration is allowed in roaming network.
     * 
     * @return True only if current is a nation which is in the operator support
     *         list and has searched GSM before.
     */
    private boolean isCdmaRegAllowedInRoaming(String plmnNumericCdma) {
        return isOperatorSpecifySupportNation(plmnNumericCdma)
                && (mController.ignoreSearchedState() || mController.hasSearchedOnGsm());
    }

    /**
     * Whether current is in a opertor specify support CDMA nation.
     * 
     * @return True if the mcc is in the operator support list.
     */
    private boolean isOperatorSpecifySupportNation(String plmnNumericCdma) {
        if (!((PhoneProxy) mDualModePhone).isWCDMAPrefered()) {
            for (String mccNumeric : OPERATOR_SPECIFIC_SUPPORT_CDMA_NATION) {
                if (plmnNumericCdma.startsWith(mccNumeric)) {
                    logd("Operator support roaming nation: plmnNumericCdma = " + plmnNumericCdma);
                    return true;
                }
            }
        }
        return false;
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }
}
