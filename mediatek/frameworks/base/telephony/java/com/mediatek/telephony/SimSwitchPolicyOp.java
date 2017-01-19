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

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyCapabilities;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ISimSwitchPolicy;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.SimInfo;
import android.os.SystemProperties;
import android.content.Context;
import android.util.Log;
import android.provider.Settings;

/**
 * Sim Switch Policy (for Operator)
 * 
 * @hide
 */
public class SimSwitchPolicyOp extends SimSwitchPolicyDefault {
    
    private static final String LOG_TAG = "SimSwitchPolicyOp";
    
    private int telephonyMode = 0;
    
    private static final String ICC_CARD_TYPE_SIM = "SIM";
    private static final String ICC_CARD_TYPE_USIM = "USIM";
    
    public SimSwitchPolicyOp() {
    	// check current telephony mode
        if ( FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_UMTS_TDD128_MODE )
        	telephonyMode = 2;
        Log.i(LOG_TAG, "policy object created. (telephonyMode="+telephonyMode+")");
    }
    
    @Override
    public int selectCapabilitySim(Context ctx, int capability, String iccId3GSim, int id3GSim, String iccId1, String iccId2, String iccId3, String iccId4) {
        
        int selected = ISimSwitchPolicy.CAPABILITY_INVALID;
        
        Log.i(LOG_TAG, "selectCapabilitySim (telephonyMode="+telephonyMode+", capability="+capability+", iccId3GSim="+iccId3GSim+", id3GSim="+id3GSim+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (capability == TelephonyCapabilities.CAPABILITY_34G && !FeatureOption.MTK_LTE_SUPPORT)
        {
            Log.i(LOG_TAG, "wrong capability param, no change. ("+capability+")");
            return ISimSwitchPolicy.CAPABILITY_NOCHANGE;
        }
        
        // ADD CUSTOM POLICY HERE
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
        	
//        	TelephonyManagerEx tmEx = TelephonyManagerEx.getDefault();
//        	String sim1CardType = tmEx.getIccCardType(PhoneConstants.GEMINI_SIM_1);
//        	String sim2CardType = tmEx.getIccCardType(PhoneConstants.GEMINI_SIM_2);
            String sim1CardType = SystemProperties.get("gsm.ril.uicctype");
            String sim2CardType = SystemProperties.get("gsm.ril.uicctype.2");
         
            Log.i(LOG_TAG, "selectCapabilitySim (sim1Op="+sim1Op+", sim2Op="+sim2Op+", sim1CardType="+sim1CardType+", sim2CardType="+sim2CardType+")");
            
            int miscFeatureConfig = Settings.Global.getInt(ctx.getContentResolver(),
                    Settings.Global.TELEPHONY_MISC_FEATURE_CONFIG, 0);
            Log.i(LOG_TAG, "needSwitchRatModeInDesignateRegion: miscFeatureConfig=" + miscFeatureConfig);     
                    
            if (iccId1 != null && iccId2 != null)
            {
                if (isOperatorOp01(sim1Op) && isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "selectCapabilitySim check OP01,OP01");
                    if (ICC_CARD_TYPE_USIM.equals(sim1CardType) && ICC_CARD_TYPE_SIM.equals(sim2CardType)) {
                        selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM1;
                    } else if (ICC_CARD_TYPE_SIM.equals(sim1CardType) && ICC_CARD_TYPE_USIM.equals(sim2CardType)) {
                        selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM2;
                    } // ELSE GO DEFAULT
                } else if (isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "selectCapabilitySim check OP01,non-OP01");
                    selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM1;
                } else if (!isOperatorOp01(sim1Op) && isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "selectCapabilitySim check non-OP01,OP01");
                    selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM2;
                } else if (!isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op)) {
                    
                    if (isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "selectCapabilitySim check CN-non-OP01,CN-non-OP01");
                        if ((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1) {
                            selected = ISimSwitchPolicy.CAPABILITY_OFF;
                        } else {
                            selected = ISimSwitchPolicy.CAPABILITY_NOCHANGE;
                        }
                    } else if (!isChinaOperator(sim1Op) && !isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "selectCapabilitySim check NCN-non-OP01,NCN-non-OP01");
                        if (ICC_CARD_TYPE_USIM.equals(sim1CardType) && ICC_CARD_TYPE_SIM.equals(sim2CardType)) {
                            selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM1;
                        } else if (ICC_CARD_TYPE_SIM.equals(sim1CardType) && ICC_CARD_TYPE_USIM.equals(sim2CardType)) {
                            selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM2;
                        } // ELSE GO DEFAULT
                    } else if (isChinaOperator(sim1Op) && !isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "selectCapabilitySim check CN-non-OP01,NCN-non-OP01");
                        selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM2;
                    } else if (!isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "selectCapabilitySim check NCN-non-OP01,CN-non-OP01");
                        selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM1;
                    }
                } // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                
                if (iccId1!=null) {
                    if (!isOperatorOp01(sim1Op) && isChinaOperator(sim1Op)) {
                        Log.i(LOG_TAG, "selectCapabilitySim check CN-non-OP01,(null)");
                        if (id3GSim==PhoneConstants.GEMINI_SIM_1) {
                            if ((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1) {
                                selected = ISimSwitchPolicy.CAPABILITY_OFF;
                            } else {
                                selected = ISimSwitchPolicy.CAPABILITY_NOCHANGE;
                            }
                        } else {
                            if ((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1) {
                                selected = ISimSwitchPolicy.CAPABILITY_NOCHANGE;
                            } else {
                                selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM1;
                            }
                        }
                    } else {
                        Log.i(LOG_TAG, "selectCapabilitySim check OP01/NCN,(null)");
                        selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM1;
                    }
                    
                } else if (iccId2!=null) {
                    if (!isOperatorOp01(sim2Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "selectCapabilitySim check (null),CN-non-OP01");
                        if (id3GSim==PhoneConstants.GEMINI_SIM_2) {
                            if ((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1) {
                                selected = ISimSwitchPolicy.CAPABILITY_OFF;
                            } else {
                                selected = ISimSwitchPolicy.CAPABILITY_NOCHANGE;
                            }
                        } else {
                            if ((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1) {
                                selected = ISimSwitchPolicy.CAPABILITY_NOCHANGE;
                            } else {
                                selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM2;
                            }
                        }
                    } else {
                        Log.i(LOG_TAG, "selectCapabilitySim check (null),OP01/NCN");
                        selected = ISimSwitchPolicy.CAPABILITY_GEMINI_SIM2;
                    }
                    
                }
            }
        }
        
        // GO DEFAULT IF CUSTOM POLICY NOT MATCHED
        if (selected == ISimSwitchPolicy.CAPABILITY_INVALID)
        {
            selected = super.selectCapabilitySim(ctx, capability, iccId3GSim, id3GSim, iccId1, iccId2, iccId3, iccId4);
        }
        
        return selected;
    }

    @Override
    public boolean isManualModeEnabled(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        
        boolean bRet = super.isManualModeEnabled(ctx, iccId1, iccId2, iccId3, iccId4);
        
        Log.i(LOG_TAG, "isManualModeEnabled (telephonyMode="+telephonyMode+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
            
            Log.i(LOG_TAG, "isManualModeEnabled (sim1Op="+sim1Op+", sim2Op="+sim2Op+")");
    
            if (iccId1 != null && iccId2 != null)
            {
                if (!isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op) && isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeEnabled check CN-non-OP01,CN-non-OP01");
                    bRet = false;
                } // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                if (iccId1!=null) {
                    
                    if (!isOperatorOp01(sim1Op) && isChinaOperator(sim1Op)) {
                        Log.i(LOG_TAG, "isManualModeEnabled check CN-non-OP01,(null)");
                        bRet = false;
                    } // ELSE GO DEFAULT
                    
                } else if (iccId2!=null) {
                    
                    if (!isOperatorOp01(sim2Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeEnabled check (null),CN-non-OP01");
                        bRet = false;
                    } // ELSE GO DEFAULT
                    
                }
            }
        }
        
        return bRet;
    }
    
    public boolean isManualModeChangeSlotAllowed(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        boolean bRet = super.isManualModeChangeSlotAllowed(ctx, iccId1, iccId2, iccId3, iccId4);
        
        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed (telephonyMode="+telephonyMode+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
            
//            TelephonyManagerEx tmEx = TelephonyManagerEx.getDefault();
//            String sim1CardType = tmEx.getIccCardType(PhoneConstants.GEMINI_SIM_1);
//            String sim2CardType = tmEx.getIccCardType(PhoneConstants.GEMINI_SIM_2);
            String sim1CardType = SystemProperties.get("gsm.ril.uicctype");
            String sim2CardType = SystemProperties.get("gsm.ril.uicctype.2");
         
            Log.i(LOG_TAG, "isManualModeChangeSlotAllowed (sim1Op="+sim1Op+", sim2Op="+sim2Op+", sim1CardType="+sim1CardType+", sim2CardType="+sim2CardType+")");
    
            if (iccId1 != null && iccId2 != null)
            {
                if (isOperatorOp01(sim1Op) && isOperatorOp01(sim2Op)) {
                    if (ICC_CARD_TYPE_USIM.equals(sim1CardType) && ICC_CARD_TYPE_SIM.equals(sim2CardType)) {
                        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check OP01-USIM,OP01-SIM");
                        bRet = false;
                    } else if (ICC_CARD_TYPE_SIM.equals(sim1CardType) && ICC_CARD_TYPE_USIM.equals(sim2CardType)) {
                        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check OP01-SIM,OP01-USIM");
                        bRet = false;
                    } // ELSE GO DEFAULT
                } else if (isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check OP01,non-OP01");
                    bRet = false;
                } else if (!isOperatorOp01(sim1Op) && isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check non-OP01,OP01");
                    bRet = false;
                } else if (!isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op)) {
                    
                    if (isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check CN-non-OP01,CN-non-OP01");
                        bRet = false;
                    } else if (!isChinaOperator(sim1Op) && !isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check NCN-non-OP01,NCN-non-OP01");
                        if (ICC_CARD_TYPE_USIM.equals(sim1CardType) && ICC_CARD_TYPE_SIM.equals(sim2CardType)) {
                            bRet = false;
                        } else if (ICC_CARD_TYPE_SIM.equals(sim1CardType) && ICC_CARD_TYPE_USIM.equals(sim2CardType)) {
                            bRet = false;
                        } // ELSE GO DEFAULT
                    } else if (isChinaOperator(sim1Op) && !isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check CN-non-OP01,NCN-non-OP01");
                        bRet = false;
                    } else if (!isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check NCN-non-OP01,CN-non-OP01");
                        bRet = false;
                    }
                } // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                
                if (iccId1!=null) {
                    Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check (any),(null)");
                    bRet = false;
                    
                } else if (iccId2!=null) {
                    Log.i(LOG_TAG, "isManualModeChangeSlotAllowed check (null),(any)");
                    bRet = false;
                    
                }
                
            }
        }

        return bRet;
    };
    
    public int getAllowedSwitchSlots(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        int nRet = super.getAllowedSwitchSlots(ctx, iccId1, iccId2, iccId3, iccId4);;
        
        Log.i(LOG_TAG, "getAllowedSwitchSlots (telephonyMode="+telephonyMode+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
            
//            TelephonyManagerEx tmEx = TelephonyManagerEx.getDefault();
//            String sim1CardType = tmEx.getIccCardType(PhoneConstants.GEMINI_SIM_1);
//            String sim2CardType = tmEx.getIccCardType(PhoneConstants.GEMINI_SIM_2);
            String sim1CardType = SystemProperties.get("gsm.ril.uicctype");
            String sim2CardType = SystemProperties.get("gsm.ril.uicctype.2");
         
            Log.i(LOG_TAG, "getAllowedSwitchSlots (sim1Op="+sim1Op+", sim2Op="+sim2Op+", sim1CardType="+sim1CardType+", sim2CardType="+sim2CardType+")");
    
            if (iccId1 != null && iccId2 != null)
            {
                if (isOperatorOp01(sim1Op) && isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitchSlots check OP01,OP01");
                    if (ICC_CARD_TYPE_USIM.equals(sim1CardType) && ICC_CARD_TYPE_SIM.equals(sim2CardType)) {
                        nRet = 1;
                    } else if (ICC_CARD_TYPE_SIM.equals(sim1CardType) && ICC_CARD_TYPE_USIM.equals(sim2CardType)) {
                        nRet = 2;
                    } // ELSE GO DEFAULT
                } else if (isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitchSlots check OP01,non-OP01");
                    nRet = 1;
                } else if (!isOperatorOp01(sim1Op) && isOperatorOp01(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitchSlots check non-OP01,OP01");
                    nRet = 2;
                } else if (!isOperatorOp01(sim1Op) && !isOperatorOp01(sim2Op)) {
                    if (isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check CN-non-OP01,CN-non-OP01");
                        nRet = 0;
                    } else if (!isChinaOperator(sim1Op) && !isChinaOperator(sim2Op)) {
                        if (ICC_CARD_TYPE_USIM.equals(sim1CardType) && ICC_CARD_TYPE_SIM.equals(sim2CardType)) {
                            Log.i(LOG_TAG, "getAllowedSwitchSlots check NCN-non-OP01-USIM,NCN-non-OP01-SIM");
                            nRet = 1;
                        } else if (ICC_CARD_TYPE_SIM.equals(sim1CardType) && ICC_CARD_TYPE_USIM.equals(sim2CardType)) {
                            Log.i(LOG_TAG, "getAllowedSwitchSlots check NCN-non-OP01-SIM,NCN-non-OP01-USIM");
                            nRet = 2;
                        } // ELSE GO DEFAULT
                    } else if (isChinaOperator(sim1Op) && !isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check CN-non-OP01,NCN-non-OP01");
                        nRet = 2;
                    } else if (!isChinaOperator(sim1Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check NCN-non-OP01,CN-non-OP01");
                        nRet = 1;
                    }
                } // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                if (iccId1!=null) {
                    if (!isOperatorOp01(sim1Op) && isChinaOperator(sim1Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check CN-non-OP01,(null)");
                        nRet = 0;
                    } else {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check OP01/NCN,(null)");
                        nRet = 1;
                    }
                    
                } else if (iccId2!=null) {
                    if (!isOperatorOp01(sim2Op) && isChinaOperator(sim2Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check (null),CN-non-OP01");
                        nRet = 0;
                    } else {
                        Log.i(LOG_TAG, "getAllowedSwitchSlots check (null),OP01/NCN");
                        nRet = 2;
                    }
                    
                }
            } // ELSE GO DEFAULT
            
        }

        return nRet;
    };
    
    private String getEarlySimOperatorById(int simIdx) {
    	String opStr = "";
        if(FeatureOption.MTK_RILD_READ_IMSI == true) {
        	String propName = "gsm.sim.ril.op" + (simIdx==PhoneConstants.GEMINI_SIM_1 ? "" : "."+(simIdx+1));
        	opStr = SystemProperties.get(propName, "");
        	if (opStr==null) opStr = "";
        }
    	Log.d(LOG_TAG, "getEarlySimOperatorById (simIdx="+simIdx+", opStr="+opStr+")");
    	return opStr;
    }
    
    private String getSimOperatorByICCId(Context ctx, String iccId) {
    	SIMInfo simInfo = SIMInfo.getSIMInfoByICCId(ctx, iccId);
		String opStr = ((simInfo==null || simInfo.mOperator==null) ? "" : simInfo.mOperator);
		Log.d(LOG_TAG, "getSimOperatorByICCId (iccId="+iccId+", opStr="+opStr+")");
		return opStr;
    }
    
    private String getSimOperator(Context ctx, int simIdx, String iccId) {
    	String opStr = getSimOperatorByICCId(ctx, iccId);
    	if(FeatureOption.MTK_RILD_READ_IMSI == true) {
        	if (opStr == null || "".equals(opStr)) {
        		opStr = getEarlySimOperatorById(simIdx);
        	}
    	}
    	Log.d(LOG_TAG, "getSimOperator (simIdx="+simIdx+", iccId="+iccId+", opStr="+opStr+")");
    	return opStr;
    }
    
    private static boolean isChinaOperator(String op) {
        boolean ret = false;
        if (SimInfo.OPERATOR_OP01.equals(op) || SimInfo.OPERATOR_OP02.equals(op) || SimInfo.OPERATOR_OP09.equals(op))
            ret = true;
        return ret;
    }
    private static boolean isOperatorOp01(String op) {
        boolean ret = false;
        if (SimInfo.OPERATOR_OP01.equals(op))
            ret = true;
        return ret;
    }
}
