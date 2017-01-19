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

package com.mediatek.op.telephony;

import android.util.Log;
import android.content.Context;
import android.telephony.ServiceState;
import com.mediatek.common.telephony.IServiceStateExt;
import android.telephony.SignalStrength;
import com.mediatek.common.featureoption.FeatureOption;
import android.content.res.Resources;

//[ALPS01577029]-START
//To support auto switch rat mode to 2G only for 3M TDD csfb project when we are not in china
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.gemini.GeminiPhone;
import android.os.SystemProperties;
import android.provider.Settings;
//[ALPS01577029]-END


import java.util.Map;

public class ServiceStateExt implements IServiceStateExt {
    static final String TAG = "GSM";

    public ServiceStateExt() {
    }

    public ServiceStateExt(Context context) {
    }

    public void onPollStateDone(ServiceState oldSS, ServiceState newSS, int oldGprsState, int newGprsState) {
    }

    public String onUpdateSpnDisplay(String plmn, int radioTechnology, int simId) {
        /* ALPS00362903 */
        if(FeatureOption.MTK_NETWORK_TYPE_ALWAYS_ON == true){
            // for LTE
            if (radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_LTE && 
                plmn != Resources.getSystem().getText(com.android.internal.R.string.
                    lockscreen_carrier_default).toString()){
                plmn = plmn + " 4G";
            }
            /* ALPS00492303 */
            //if (radioTechnology > 2 && plmn != null){
            else if (radioTechnology > ServiceState.RIL_RADIO_TECHNOLOGY_EDGE &&
                     plmn != Resources.getSystem().getText(com.android.internal.R.string.
                    lockscreen_carrier_default).toString()){
                plmn = plmn + " 3G";
            }
        }

        return plmn;
    }

    public boolean isRegCodeRoaming(boolean originalIsRoaming, int mccmnc, String numeric) {
        return originalIsRoaming;
    }

    public boolean isImeiLocked(){
        return false;
    }		

    public boolean isBroadcastEmmrrsPsResume(int value) {
        return false;
    }

    public boolean needEMMRRS() {
        return false;
    }

    public boolean needSpnRuleShowPlmnOnly() {
        //[ALPS01679495]-start: don't show SPN for CTA case
        if (FeatureOption.MTK_CTA_SUPPORT){
            return true;
        }
        //[ALPS01679495]-end
        return false;
    }
    
    public boolean needBrodcastACMT(int error_type,int error_cause) {
        return false;
    }
	
    public boolean needRejectCauseNotification(int cause){
        return false;    
    }

    public boolean needIgnoredState(int state,int new_state,int cause){
        if((state == ServiceState.STATE_IN_SERVICE) && (new_state == 2)){
            /* Don't update for searching state, there shall be final registered state update later */						
            Log.i(TAG,"set dontUpdateNetworkStateFlag for searching state");                  
            return true;
        }	   

        /* -1 means modem didn't provide <cause> information. */
        if(cause != -1){
            // [ALPS01384143] need to check if previous state is IN_SERVICE for invalid sim 
            if((state == ServiceState.STATE_IN_SERVICE)  && (new_state == 3) && (cause != 0)){
            //if((new_state == 3) && (cause != 0)){
                /* This is likely temporarily network failure, don't update for better UX */			
                Log.i(TAG,"set dontUpdateNetworkStateFlag for REG_DENIED with cause");                  
                return true;
            }
        }	
		
        Log.i(TAG,"clear dontUpdateNetworkStateFlag");       	   

        return false;
    }	

    public boolean ignoreDomesticRoaming(){
        return false;
    }

    public int mapGsmSignalLevel(int asu,int GsmRscpQdbm){
        int level;
        // [ALPS01055164] -- START , for 3G network
        if (GsmRscpQdbm < 0) {
            // 3G network
            if (asu <= 5 || asu == 99) level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
            else if (asu >= 15) level = SignalStrength.SIGNAL_STRENGTH_GREAT;
            else if (asu >= 12)  level = SignalStrength.SIGNAL_STRENGTH_GOOD;
            else if (asu >= 9)  level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
            else level = SignalStrength.SIGNAL_STRENGTH_POOR;
        // [ALPS01055164] -- END
        } else {
            // 2G network 
            if (asu <= 2 || asu == 99) level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
            else if (asu >= 12) level = SignalStrength.SIGNAL_STRENGTH_GREAT;
            else if (asu >= 8)  level = SignalStrength.SIGNAL_STRENGTH_GOOD;
            else if (asu >= 5)  level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
            else level = SignalStrength.SIGNAL_STRENGTH_POOR;
        }
        return level;
    }

    //[ALPS01440836][ALPS01594704]-START: change level mapping rule of signal for CMCC
    public int mapLteSignalLevel(int mLteRsrp, int mLteRssnr, int mLteSignalStrength){        
        /*
         * TS 36.214 Physical Layer Section 5.1.3 TS 36.331 RRC RSSI = received
         * signal + noise RSRP = reference signal dBm RSRQ = quality of signal
         * dB= Number of Resource blocksxRSRP/RSSI SNR = gain=signal/noise ratio
         * = -10log P1/P2 dB
         */
        
        int rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN, rsrpIconLevel = -1, snrIconLevel = -1;
        if (mLteRsrp > -44) rsrpIconLevel = -1;
        else if (mLteRsrp >= -85) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (mLteRsrp >= -95) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (mLteRsrp >= -105) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else if (mLteRsrp >= -115) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        else if (mLteRsrp >= -140) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;

        /*
         * Values are -200 dB to +300 (SNR*10dB) RS_SNR >= 13.0 dB =>4 bars 4.5
         * dB <= RS_SNR < 13.0 dB => 3 bars 1.0 dB <= RS_SNR < 4.5 dB => 2 bars
         * -3.0 dB <= RS_SNR < 1.0 dB 1 bar RS_SNR < -3.0 dB/No Service Antenna
         * Icon Only
         */
        if (mLteRssnr > 300) snrIconLevel = -1;
        else if (mLteRssnr >= 130) snrIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (mLteRssnr >= 45) snrIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (mLteRssnr >= 10) snrIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else if (mLteRssnr >= -30) snrIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        else if (mLteRssnr >= -200)
            snrIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;

        Log.i(TAG, "getLTELevel - rsrp:" + mLteRsrp + " snr:" + mLteRssnr + " rsrpIconLevel:"
                + rsrpIconLevel + " snrIconLevel:" + snrIconLevel);

        /* Choose a measurement type to use for notification */
        if (snrIconLevel != -1 && rsrpIconLevel != -1) {
            /*
             * The number of bars displayed shall be the smaller of the bars
             * associated with LTE RSRP and the bars associated with the LTE
             * RS_SNR
             */
            return (rsrpIconLevel < snrIconLevel ? rsrpIconLevel : snrIconLevel);
        }

        if (snrIconLevel != -1) return snrIconLevel;

        if (rsrpIconLevel != -1) return rsrpIconLevel;

        /* Valid values are (0-63, 99) as defined in TS 36.331 */
        if (mLteSignalStrength > 63) rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (mLteSignalStrength >= 12) rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (mLteSignalStrength >= 8) rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (mLteSignalStrength >= 5) rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else if (mLteSignalStrength >= 0) rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        Log.i(TAG, "getLTELevel - rssi:" + mLteSignalStrength + " rssiIconLevel:"
                + rssiIconLevel);
        return rssiIconLevel;
    }
    //[ALPS01440836][ALPS01594704]-END: change level mapping rule of signal for CMCC

    public int mapGsmSignalDbm(int GsmRscpQdbm,int asu){
        int dBm;
        Log.d(TAG,"mapGsmSignalDbm() GsmRscpQdbm=" + GsmRscpQdbm + " asu=" + asu);
        if (GsmRscpQdbm < 0) {
            dBm = GsmRscpQdbm / 4; //Return raw value for 3G Network
        } else {
            dBm = -113 + (2 * asu);
        }
        return dBm;
    }

    public int setEmergencyCallsOnly(int state,int cid,int lac){
        if((cid == -1) || (state == 4)){
            /* state(4) is 'unknown'  and cid(-1) means cid was not provided in +creg URC */                
            Log.i(TAG,"No valid info to distinguish limited service and no service");                                        
            return -1;	
        }
        //[ALPS01451327] need to handle different format of Cid
        // if cid is 0x0fffffff means it is invalid 
        // for backward comptable we keep filter of oxffff 
        else if(((state ==0)||(state == 3)) && ( (((cid & 0xffff)!=0) && (cid!=0x0fffffff))&& (lac!=0xffff))){
            return 1;
        }
        else{
            return 0;
        }		
    }		
		
    public void log(String text) {
        Log.d(TAG, text);
    }

    public Map<String, String> loadSpnOverrides() {
        return null;
    }

    public boolean allowSpnDisplayed() {
        return true;
    }

    public boolean supportEccForEachSIM() {
        return false;
    }

    public void updateOplmn(Context context, Object ci) {
    }

    public String getEccPlmnValue() {
        return Resources.getSystem().getText(com.android.internal.R.string.emergency_calls_only).toString();
    }

    //[ALPS01558804] MTK-START: send notification for using some spcial icc card
    public boolean needIccCardTypeNotification(String iccCardType){
        return false;
    }
    //[ALPS01558804] MTK-END: send notification for using some special icc card

    //[ALPS01577029]-START:To support auto switch rat mode to 2G only for 3M TDD csfb project when we are not in china
    public int needSwitchRatModeInDesignateRegion(int mSimId,String nwPlmn){
        return -1;
    }

    private static final String[] MCC_TABLE_LAB_TEST = {
        "001", "002", "003", "004", "005", "006", 
        "007", "008", "009", "010", "011", "012"
    };
    private static final String[] PLMN_TABLE_LAB_TEST = {
        "46004", "46602", "50270", "46003"
    }; 

    private static final String PROPERTY_RIL_TEST_SIM[] = {
        "gsm.sim.ril.testsim",
        "gsm.sim.ril.testsim.2",
        "gsm.sim.ril.testsim.3",
        "gsm.sim.ril.testsim.4",
    };
	
    private boolean isInDesignateRegion(String baseMcc, String nwPlmn) {
        String mcc = nwPlmn.substring(0, 3);
        if (mcc.equals(baseMcc)) {
            Log.i(TAG,"nwPlmn: "+nwPlmn+ " is in MCC: "+baseMcc);
            return true;
        }

        Log.i(TAG,"nwPlmn: "+nwPlmn+ " NOT in MCC: "+baseMcc);
        return false;
    }

    private static boolean isLabTestPlmn(String nwPlmn) {
        String nwMcc = nwPlmn.substring(0, 3);
        for (String mcc : MCC_TABLE_LAB_TEST) {
            if (mcc.equals(nwMcc)) {
                Log.i(TAG,"Test MCC");
                return true;
            }
        }
        for (String plmn : PLMN_TABLE_LAB_TEST) {
            if (plmn.equals(nwPlmn)) {
                Log.i(TAG,"Test PLMN");
                return true;
            }
        }

        Log.i(TAG,"Not in Lab test PLMN list");

        return false;
    }
    //[ALPS01577029]-END:To support auto switch rat mode to 2G only for 3M TDD csfb project when we are not in china
    
    //[ALPS01602110] To support auto switch rat mode to 2G only for 3M TDD csfb project when inserting non-CMCC China operator Card
    public int needSwitchRatModeForCertainSIM(int mSimId,String mccmnc){
        return -1;
    }

    public boolean isSupportRatBalancing(){
        return false;
    }
}
