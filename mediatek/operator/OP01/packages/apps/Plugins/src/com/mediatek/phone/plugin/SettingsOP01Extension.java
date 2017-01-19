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

package com.mediatek.phone.plugin;

import android.app.Activity;
import android.preference.PreferenceActivity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.util.Log;
import java.util.List;
import android.widget.TextView;
import android.telephony.TelephonyManager;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.AsyncResult;
import android.os.RemoteException;
import android.os.ServiceManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.worldphone.LteModemSwitchHandler;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;//mmdc
import com.mediatek.op01.plugin.R;
import com.mediatek.phone.ext.SettingsExtension;


import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
//import com.android.phone.PhoneGlobals;
import com.mediatek.common.telephony.ITelephonyEx;
//import com.mediatek.phone.gemini.GeminiUtils;
//import com.mediatek.phone.wrapper.TelephonyManagerWrapper;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.phone.GeminiConstants;
import com.mediatek.common.featureoption.FeatureOption;





public class SettingsOP01Extension extends SettingsExtension {

    //lte vt
    PreferenceScreen mCallSettingP;
    Preference mVTSetting;
    Context mVTSettingcontext;
    Activity mVTAdvanceActivity;//vtadvance and advanceex
    public static final String BUTTON_NETWORK_MODE_KEY = "gsm_umts_preferred_network_mode_key";//single sim
    public static final String BUTTON_PLMN_LIST = "button_plmn_key";
    private String BUTTON_VEDIO_CALL_KEY = "button_vedio_call_key";	
    public static final String BUTTON_NETWORK_MODE_EX_KEY = "button_network_mode_ex_key";//dual sim
    public static final String BUTTON_PREFERED_NETWORK_MODE = "preferred_network_mode_key";
    public static final String BUTTON_3G_SERVICE = "button_3g_service_key";
    public static final String BUTTON_2G_ONLY = "button_prefer_2g_key";
    public static final String BUTTON_NETWORK_MODE_LTE_KEY = "button_network_mode_LTE_key";

    private static final String LOG_TAG = "My:::SettingsOP01Extension";
    private static final String KEY_MMDC_MODE = "mmdc_mode";
    private ListPreference mdcPreferenceMode = null;

    private static final int PREFERRED_NETWORK_MODE = Phone.NT_MODE_WCDMA_PREF;//0
    private static final int MODEM_MASK_LTE = 0x80;

    private static final String GEMINI_BASEBAND_PROP[] = {
        "gsm.baseband.capability",
        "gsm.baseband.capability2",
        "gsm.baseband.capability3",
        "gsm.baseband.capability4",
    };

    //private Phone mPhone = null;//mmdc cmm
    private Context mCxt = null;//mmdc cmm
    //private Activity mRootCxt = null;//mmdc cmm
    //private MyHandler mHandler;
    private boolean mIsUSIM = true;//mmdc cmm
    ITelephonyEx mITelephonyEx = null;//mmdc cmm
    private final ITelephony mTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));//mmdc cmm
    //ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
    //mTel.getIccCardType(simId)
    //mmdc cmm wait dialog
    private static final int CALL_LIST_DIALOG_WAIT = 0;
    private	ProgressDialog mDialog = null;


    /// Add for LTE @{
    public static final String USIM = "USIM";
    public static final String SIM = "SIM";
    private int mSlotId = -1;

//    void showDialog(int id) {
//        log("showDialog process");
//        mDialog = new ProgressDialog(mRootCxt);
//        mDialog.setMessage(mCxt.getResources().getString(R.string.please_wait));
//        mDialog.setCancelable(false);
//        mDialog.setIndeterminate(true);
//        mDialog.show();
//    }
    public void log(String msg) {
        Log.d(LOG_TAG, msg);
    }









////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////




    private static final String[] MCCMNC_TABLE_TYPE_CU = {
        "46001", "46006", "45407", "46005", "45502"};

   /**
	* app use to judge the isCUCard is CU
	* @param void
	* @return true is CU
	*/
    private boolean isCUCard(int slotId) {
        log(":::isCUCard::sLotId = " + slotId);
        String simOperator = null;
        simOperator = getSimOperator(slotId);
        if (simOperator != null) {
            log(":::isCUSim:" + simOperator);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CU) {
                if (simOperator.equals(mccmnc)) {
                    return true;
                } 
            }
        }
        return false;	   
    }

    private boolean isSimStateReady(int slot) {
        boolean isSimStateReady = false;
        isSimStateReady = TelephonyManager.SIM_STATE_READY == getSimState(slot);
        log(":::isSimStateReady isSimStateReady: "  + isSimStateReady);
        return isSimStateReady;
    }

    /**
    * PhoneConstants.GEMINI_SIM_1, PhoneConstants.GEMINI_SIM_2...
    * 
	* @return
    */
    private int[] getSlots() {
        return GeminiConstants.SLOTS;
    }

    /**
    * @see FeatureOption.MTK_GEMINI_SUPPORT
    * @see FeatureOption.MTK_GEMINI_3SIM_SUPPORT
    * @see FeatureOption.MTK_GEMINI_4SIM_SUPPORT
    * @return true if the device has 2 or more slots
    */
    private boolean isGeminiSupport() {
       return GeminiConstants.SOLT_NUM >= 2;
    }

    /**
     * check the slotId value.
     * 
     * @param slotId
     * @return
     */
    private boolean isValidSlot(int slotId) {
        final int[] geminiSlots = getSlots();
        for (int i = 0; i < geminiSlots.length; i++) {
            if (geminiSlots[i] == slotId) {
                return true;
            }
        }
        return false;
    }



	/**
     * Returns a constant indicating the state of the
     * device SIM card.
     *
     * @param slotId
     * @return
     */
    private int getSimState(int slotId){
        int status;
        if (isGeminiSupport() && isValidSlot(slotId)) {
            // M: GEMINI API should be put in TelephonyManagerEx
            status = TelephonyManagerEx.getDefault().getSimState(slotId);
        } else {
            status = TelephonyManager.getDefault().getSimState();
        }
        log(":::getSimState, slotId = " + slotId + "; status = " + status);
        return status;
    }

   /**
	* Gets the MCC+MNC (mobile country code + mobile network code) of the provider of the SIM. 5 or 6 decimal digits. 
	* <p>
	* Availability: The result of calling getSimState() must be android.telephony.TelephonyManager.SIM_STATE_READY.
	* <p>
	* @param slotId  Indicates which SIM to query. 
	*				Value of simId:
	*				  0 for SIM1
	*				  1 for SIM2  
	* <p>
	* @return		MCC+MNC (mobile country code + mobile network code) of the provider of the SIM. 5 or 6 decimal digits.
	*/
    private String getSimOperator(int slotId) {
	   String simOperator = null;
	   
	   if (isSimStateReady(slotId)) {
		   int status;
		   if (isGeminiSupport()) {
			   simOperator = TelephonyManagerEx.getDefault().getSimOperator(slotId);
		   } else {
			   simOperator = TelephonyManager.getDefault().getSimOperator();
		   }
	   }
	   log(":::getSimOperator::simOperator = " + simOperator + " slotId = " + slotId);
	   return simOperator;
   }

   /**
	* Gets the MCC+MNC (mobile country code + mobile network code) of the provider of the SIM. 5 or 6 decimal digits. 
	* <p>
	* Availability: The result of calling getSimState() must be android.telephony.TelephonyManager.SIM_STATE_PIN_REQUIRED.
	* <p>
	* @param slotId  Indicates which SIM to query. 
	*				Value of simId:
	*				  0 for SIM1
	*				  1 for SIM2  
	* <p>
	* @return 
	*/
    private boolean isPinLock(int slotId) {
        boolean isPinLock = false;
        isPinLock = TelephonyManager.SIM_STATE_PIN_REQUIRED == getSimState(slotId);
        log(":::isPinLock::isPinLock = " + isPinLock + " slotId = " + slotId + 
            ":::getSimState= " + getSimState(slotId));
        return isPinLock;
    }

   /**
	* app use to judge LTE or LTE_DC open
	* @param void
	* @return true is LTE or LTE_DC open
	*/
    private boolean isLteSupport() {
        log(":::isLte::" + FeatureOption.MTK_LTE_SUPPORT);
        return FeatureOption.MTK_LTE_SUPPORT;
    }
   
   /**
	* app use to judge wheather is in china
	* @param void
	* @return true is in china
	*/
    private boolean isInChina() {
	    log(":::isInChina");
        ITelephony iTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));//PhoneGlobals.getInstance().phoneMgrEx;
        String strLocatedPlmn = null;
        try {
            strLocatedPlmn = iTelephony.getLocatedPlmn();
            if (strLocatedPlmn != null) {
                String strLocatedMcc = strLocatedPlmn.substring(0,3);
                log(":::isInChina::strLocatedPlmn:" + strLocatedPlmn + " strLocatedMcc:" + strLocatedMcc);
                if (strLocatedMcc.equals("460"))
                    return true;
            } else {
                log(":::isInChina::LocatedMcc is null but now as true");
                return true;
            }
        } catch (RemoteException e) {
            log(":::isCsfbMode exception: ");
        }
	    
       return false;
   }
   
   /**
	* app use to which is 5 mode or 3 mode
	* @param void
	* @return true is 5 mode, fals is 3 mode
	*/
    private boolean isWorldPhone() {
        log(":::isWorldPhone:" + PhoneFactory.isWorldPhone());
        return PhoneFactory.isWorldPhone();
    }

   /**
	* app use to judge LTE or LTE_DC open
	* @param void
	* @return true is LTE or LTE_DC open
	*/
    private boolean isLteDC() {
	    log(":::isLteDC::" + PhoneFactory.isLteDcSupport());
	    return PhoneFactory.isLteDcSupport();
    }

   /**
	* app use to judge the isValidSimOperator
	* @param void
	* @return true is CU
	*/
    private boolean isValidSimOperator(int slotId) {
        log(":::isValidSimOperator::sLotId = " + slotId);
        if (slotId == -1) {
            log(":::isValidSimOperator the slotid is invalid");
            return false;
        }
	    String simOperator = null;
	    simOperator = getSimOperator(slotId);
	    if (simOperator != null && !(simOperator.isEmpty())) {
		    log(":::isValidSimOperator: not null");
		    return true;
	    }
        return false;	   
    }

    private boolean is4GSimSlot(int slot) {
        log(":::is4GSimSlot::");
        boolean is4GSimSlot = false;
        log("[is4GSimSlot] slot = " + slot);
        if (slot == -1) {
            //Do nothing
            log("[is4GSimSlot] slot = UNDEFINED_SLOT_ID(-1) !!");
        } else if (getBaseband(slot) >= MODEM_MASK_LTE) {
            log("baseband: " + getBaseband(slot));
            is4GSimSlot = true;
        }
        return is4GSimSlot;
    }

    private int getBaseband(int slot) {
        log(":::getBaseband::");
        String propertyKey = GEMINI_BASEBAND_PROP[slot];

        int baseband = 0;
        try {
            String capability = SystemProperties.get(propertyKey);
            if (capability != null) {
                baseband = Integer.parseInt(capability);
            }
        } catch (NumberFormatException e) {
            Log.e(LOG_TAG, "getBaseband exception1: ", e);
        }
        log("[slot = " + slot + "]" + "[propertyKey = " + propertyKey + "]" + "[baseband = " + baseband + "]");
        return baseband;
    }

    private boolean isLteDcModeEnable(int slotId) {
	    log(":::isLteDcModeEnable::slotId = " + slotId);
        // for sglte or tdd csfb
        //sim operator is null?
        if (!isValidSimOperator(slotId)) {
            log(":::isLteDcModeEnable:: at tdd surport, sim operator is null,shoule disable");
            return false;
		}

        // for sim
        String type = "";
        try {
            type = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx")).getIccCardType(slotId);//PhoneGlobals.getInstance().phoneMgrEx.getIccCardType(slotId);
        } catch(RemoteException e) {
            Log.e(LOG_TAG, "customizeMmdcPreferenceMode exception1: ", e);
		}
        if (SettingsOP01Extension.SIM.equals(type)) {
            log(":::isLteDcModeEnable:: sim shoule disable");
            return false;
        }

        // for sglte or tdd csfb, and cu card
        if (isCUCard(slotId)) {
            log(":::isLteDcModeEnable:: shoule disable");
            return false;
        }

        // for sglte or tdd csfb forigen
        if (!isWorldPhone() && !isInChina()) {
            log(":::isLteDcModeEnable:: shoule disable");
            return false;
        }

        // for sglte dsds forigen
        int state = LteModemSwitchHandler.MD_TYPE_UNKNOWN;
        try {
            state = ITelephonyEx.Stub.asInterface(ServiceManager.checkService("phoneEx")).getActiveModemType(); 
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "customizeMmdcPreferenceMode exception: ", e);
        }
        boolean isWG = (state == LteModemSwitchHandler.MD_TYPE_WG) ? true: false;
        log(":::isLteDcModeEnable:: isWG = " + isWG + " state = " + state); 
        if (!is4GSimSlot(slotId) && isWG) {
            log(":::isLteDcModeEnable::is forigen and not 4g shoule disable");
            return false;
        }

        log(":::isLteDcModeEnable:: shoule enable");
        return true;
    }


   /**
	* app use to update sglte mode value and summary
	* @param ListPreference preference,
	* @param int modemNetworkMode,
	* @return void
	* just for sglte, only show 0 and 9
	* Phone.NT_MODE_LTE_GSM_WCDMA---9
	* Phone.NT_MODE_WCDMA_PREF------0
	* but Phone.NT_MODE_GSM_UMTS is 3 trans to 0
	* because sim the networkmode for sglet is disable, this only fit to usim
    * For ALPS01461533
    */
    private void updateSglteMode(ListPreference preference, int modemNetworkMode) {
	    log(":::updateSglteMode: FeatureOption.MTK_LTE_SUPPORT = " + PhoneFactory.isLteSupport());
	    mSlotId = get3GCapabilitySIM();
	    String type = null;
	    if (mSlotId != -1) {
		    try {
	   	        //ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx")).get3GCapabilitySIM();//PhoneGlobals.getInstance().phoneMgrEx.get3GCapabilitySIM();
	   	        type = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx")).getIccCardType(mSlotId);//PhoneGlobals.getInstance().phoneMgrEx.getIccCardType(slotId);
	        } catch(RemoteException e) {
	            Log.e(LOG_TAG, "updateSglteMode exception3: ", e);
	        }
	    } else {
	        log ("[:::updateSglteMode:sim type: return slotId:" + mSlotId);
            return;
	    }
	    log ("[:::updateSgletSummary:sim type:" + type + " mode:" +	modemNetworkMode + " slotId:" + mSlotId);
	    if (isLteSupport()) {
		    if (mSlotId == -1 || SettingsOP01Extension.SIM.equals(type) || isCUCard(mSlotId)) {
                preference.setSummary("");
			    log(":::1 updateSglteMode0:sumary set empty and disable");
		    } else {
			    if (modemNetworkMode == Phone.NT_MODE_WCDMA_PREF || modemNetworkMode == Phone.NT_MODE_LTE_GSM_WCDMA) {
                    preference.setValue(Integer.toString(modemNetworkMode));
                    preference.setSummary(preference.getEntry());
			        log(":::1 updateSglteMode1:sumary::" + preference.getEntry());
                } else {
                    preference.setSummary("");
                    log(":::1 updateSglteMode2:sumary::" + preference.getEntry());
                }
           }	   
        }
    }

    private void dealInitNetworkMode(PreferenceScreen prefSet, ListPreference listLteNetworkMode) {
	    log(":::dealInitNetworkMode:");
	    setPreferredNetworkModeEntriesAndValues(listLteNetworkMode);
	    int settingsNetworkMode = android.provider.Settings.Global.getInt(
			prefSet.getContext().getContentResolver(),
			android.provider.Settings.Global.PREFERRED_NETWORK_MODE,
			Phone.NT_MODE_LTE_GSM_WCDMA);//Phone.NT_MODE_LTE_GSM_WCDMA is 9
	    log ("::dealInitNetworkMode mode:" +  settingsNetworkMode);
	    updateSglteMode(listLteNetworkMode, settingsNetworkMode);
	    if(listLteNetworkMode.isEnabled()) {
		    boolean isEnable = isLteDcModeEnable(mSlotId);
			listLteNetworkMode.setEnabled(isEnable);
			log ("::dealInitNetworkMode isEnable:" +  isEnable);					
	    }
    }

    private void getOp01Cxt(Context context) {
	    try {
		    log("getOp01Cxt try to catch op01 context");
		    mCxt = context.createPackageContext("com.mediatek.op01.plugin", Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);	
	    } catch(NameNotFoundException e) {
		    log("getOp01Cxt catch no found com.mediatek.op01.plugin");
	    }
    }


    /**
     * get 3G capability slotId by ITelephony.get3GCapabilitySIM();
     * 
     * @return the SIM id which support 3G.
     */
    private int get3GCapabilitySIM() {
	    ITelephonyEx iTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
	    try {
		    final int slot3G = iTelephonyEx.get3GCapabilitySIM();
		    log("get3GCapabilitySIM, slot3G" + slot3G);
		    return slot3G;
	    } catch (RemoteException re) {
		    log("get3GCapabilitySIM, " + re.getMessage() + ", return -1");
		    return -1;
        }
    }
//up is internal use, under is host to call
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
    public void updateModeSummary(ListPreference listLteNetworkMode) {
        log(":::updateModeSummary: FeatureOption.MTK_LTE_SUPPORT = ");
        mSlotId = get3GCapabilitySIM();
        String type = null;
        if (mSlotId != -1) {
            try {
                type = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx")).getIccCardType(mSlotId);
            } catch(RemoteException e) {
                Log.e(LOG_TAG, "updateSglteModeSummary exception3: ", e);
            }
        } else {
            log ("[:::updateSglteModeSummary:sim type: return slotId:" + mSlotId);
            return;
        }
        log ("[:::updateSgletSummary:sim type:" + type  + " slotId:" + mSlotId);
        if (isLteSupport()) {
            if (mSlotId == -1 || SettingsOP01Extension.SIM.equals(type) || isCUCard(mSlotId)) {
                listLteNetworkMode.setSummary("");
                log(":::1 updateSglteMode0:sumary set empty and disable");
            }   
        }

    }

    public void setPreferredNetworkModeEntriesAndValues(ListPreference listLteNetworkMode) {
        log(":::setPreferredNetworkModeEntriesAndValues");
        if (mCxt == null) {
            log("[setPreferredNetworkModeEntriesAndValues]	mCxt is null");
            return;
        }
        listLteNetworkMode.setEntries(
            mCxt.getResources().getStringArray(R.array.sglte_network_mode_choices));
        listLteNetworkMode.setEntryValues(
            mCxt.getResources().getStringArray(R.array.sglte_network_mode_values));
    }

    public void updateSglteModeStatus(ListPreference preference, Preference preferenceEx) {
        log("[...updateSglteModeStatus enter]");
        if (FeatureOption.MTK_LTE_SUPPORT) {
            mSlotId = get3GCapabilitySIM();
            boolean isEnable = isLteDcModeEnable(mSlotId);
            log("updateSglteModeStatus: isEnable = " + isEnable);
            if (preference != null && preference.isEnabled()) {
                preference.setEnabled(isEnable);
                log("updateSglteModeStatus: preference status = " + isEnable);
                if (!preference.isEnabled()) {
                    Dialog dialog = preference.getDialog();
                    if (dialog != null && dialog.isShowing()) {
                        dialog.dismiss();
                        log("updateSglteModeStatus: dismiss dialog ");
                    }
                }
            }
            if (preferenceEx != null && preferenceEx.isEnabled()) {
                preferenceEx.setEnabled(isEnable);
                log("updateSglteModeStatus: preferenceEx status = " + isEnable);
                if (!preference.isEnabled()) {
                    Dialog dialog = preference.getDialog();
                    if (dialog != null && dialog.isShowing()) {
                        dialog.dismiss();
                        log("updateSglteModeStatusEx: dismiss dialog ");
                    }
                }
            }
        }
        log("[...updateSglteModeStatus:exit]..."); 		
    }

    public void customizeFeatureForOperator(PreferenceScreen prefSet) {
        log("[...customizeFeatureForOperator enter]");
        if (prefSet == null) {
            log("customizeFeatureForOperator prefSet null");
            return;
        } else {
            getOp01Cxt(prefSet.getContext());
        }
   
        Preference buttonPreferredNetworkModeEx = prefSet.findPreference(BUTTON_NETWORK_MODE_EX_KEY);
        ListPreference listPreferredNetworkMode = (ListPreference) prefSet.findPreference(BUTTON_PREFERED_NETWORK_MODE);
        Preference buttonPreferredGSMOnly = (CheckBoxPreference) prefSet.findPreference(BUTTON_2G_ONLY);
        ListPreference listgsmumtsPreferredNetworkMode = (ListPreference) prefSet.findPreference(BUTTON_NETWORK_MODE_KEY);
        ListPreference listLteNetworkMode = (ListPreference) prefSet.findPreference(BUTTON_NETWORK_MODE_LTE_KEY);
        if (buttonPreferredNetworkModeEx != null) {
            log("button_network_mode_ex_key");
            if (FeatureOption.MTK_LTE_SUPPORT) {
                log("Ex:MTK_LTE_SUPPORT:" + FeatureOption.MTK_LTE_SUPPORT);
            } else {
                prefSet.removePreference(buttonPreferredNetworkModeEx);
            }
        }
        if (listPreferredNetworkMode != null) {
            log("preferred_network_mode_key");
            prefSet.removePreference(listPreferredNetworkMode);
        }
        if (buttonPreferredGSMOnly != null) {
            log("button_prefer_2g_key");
            prefSet.removePreference(buttonPreferredGSMOnly);
        }
        if (listgsmumtsPreferredNetworkMode != null) {
            log("gsm_umts_preferred_network_mode_key");
            prefSet.removePreference(listgsmumtsPreferredNetworkMode);
        }

        if (listLteNetworkMode != null) {
            log("button_network_mode_LTE_key");
            if (FeatureOption.MTK_LTE_SUPPORT) {
                log("MTK_LTE_SUPPORT:" + FeatureOption.MTK_LTE_SUPPORT);
                dealInitNetworkMode(prefSet, listLteNetworkMode);
                return;
            }
            prefSet.removePreference(listLteNetworkMode);
        }
        log("[...customizeFeatureForOperator exit]...");
    }

   
    /**
     * for change feature ALPS00783794 add "removeNMOpFor3GSwitch" funtion,
     * should remove
     * @param prefsc
     * @param networkMode
     */
    public void removeNMOpFor3GSwitch(PreferenceScreen prefsc, Preference networkMode) {
        log("[...removeNMOpFor3GSwitch enter]");

        if (prefsc != null && networkMode != null) {
            log("networkMode is not null");
            if (FeatureOption.MTK_LTE_SUPPORT) {
                log("MTK_LTE_SUPPORT:" + FeatureOption.MTK_LTE_SUPPORT);
				ListPreference wwitchNWMode = (ListPreference) prefsc.findPreference(BUTTON_PREFERED_NETWORK_MODE);
                if (wwitchNWMode !=null) {
                    log("find networkMode is not null");
                    dealInitNetworkMode(prefsc, wwitchNWMode);
                    return;
                }
        }
            prefsc.removePreference(networkMode);
        }
        log("[...removeNMOpFor3GSwitch exit]...");
    }

    /**
     * for change feature ALPS00783794 add "remove 3g switch off radio" funtion,
     * should remove
     */
    public boolean isRemoveRadioOffFor3GSwitchFlag() {
        log("[...isRemoveRadioOffFor3GSwitchFlag true]");
        return true;
    }
    
    /**
     * indicator whether show dialog when switch slot capility
     * @return
     */
    public boolean isShowHintDialogForSimSwitch(long selectedServiceSim) {
        if (isGeminiSupport()) {
            long simid = Settings.System.getLong(mCxt.getContentResolver(),
                    Settings.System.GPRS_CONNECTION_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
            if(simid != selectedServiceSim && simid != Settings.System.GPRS_CONNECTION_SIM_SETTING_NEVER){
                return true;
            }
        }
        return false;
    }

    /**
     * get indicator string 
     * @return
     */
    public String getHintString() {
        return mCxt.getString(R.string.gemini_4g_3g_change_hint_change_data_message);
    }
}
