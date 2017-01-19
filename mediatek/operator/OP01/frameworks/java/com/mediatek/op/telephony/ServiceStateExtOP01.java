package com.mediatek.op.telephony;

import android.util.Log;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.content.Intent;
import android.content.Context;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;    //[ALPS01646248]
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.SIMRecords;
import android.telephony.SignalStrength;
import android.content.res.Resources;

//[ALPS01577029]-START
//To support auto switch rat mode to 2G only for 3M TDD csfb project when we are not in china
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.gemini.GeminiPhone;
import android.os.SystemProperties;
import android.provider.Settings;
//[ALPS01577029]-END

public class ServiceStateExtOP01 extends ServiceStateExt {
    private static final String ACTION_PS_RESUME = "com.mtk.ACTION_PS_STATE_RESUMED";
    private static final String ACTION_EMMRRS_PS_RESUME_INDICATOR = "android.intent.action.EMMRRS_PS_RESUME";
    private Context mContext;
    private String[] ITEL_PROPERTY_ICC_OPERATOR_DEFAULT_NAME = {
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME_2,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME_3,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME_4,
    };

    private String[] ITEL_PROPERTY_ICC_OPERATOR_ALPHA = {
        TelephonyProperties.PROPERTY_ICC_OPERATOR_ALPHA,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_ALPHA_2,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_ALPHA_3,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_ALPHA_4,
    };

    private String[] ITEL_PROPERTY_OPERATOR_ISROAMING = {
        TelephonyProperties.PROPERTY_OPERATOR_ISROAMING,
        TelephonyProperties.PROPERTY_OPERATOR_ISROAMING_2,
        TelephonyProperties.PROPERTY_OPERATOR_ISROAMING_3,
        TelephonyProperties.PROPERTY_OPERATOR_ISROAMING_4,
    };

    //[ALPS01646248] To support auto switch rat mode to 2G only for 3M TDD csfb project when inserting non-CMCC China operator Card
    private TelephonyManagerEx mTelephonyManagerEx;
    private String[] GSM_ONLY_PLMN_SIM = {"46001", "46006", "46009", "45407", "46005", "45502","46003","46011"};

    public ServiceStateExtOP01() {
    }

    public ServiceStateExtOP01(Context context) {
        mContext = context;
    }

    public void onPollStateDone(ServiceState oldSS, ServiceState newSS,
        int gprsState, int newGprsState, int psNetworkType, int newPsNetworkType) 
    {
        if ((newGprsState == ServiceState.STATE_IN_SERVICE && gprsState == ServiceState.STATE_OUT_OF_SERVICE) ||
            (psNetworkType >= TelephonyManager.NETWORK_TYPE_UMTS &&
             newPsNetworkType > TelephonyManager.NETWORK_TYPE_UNKNOWN &&
             newPsNetworkType <= TelephonyManager.NETWORK_TYPE_EDGE) ||
             (psNetworkType <= TelephonyManager.NETWORK_TYPE_EDGE &&
             psNetworkType > TelephonyManager.NETWORK_TYPE_UNKNOWN &&
             newPsNetworkType >= TelephonyManager.NETWORK_TYPE_UMTS)) {
            //this is a workaround for MM. when 3G->2G or 2G->3G RAU, PS temporary unavailable
            //it will trigger TCP delay retry and make MM is resumed slowly
            //PS status is recovered from unknown to in service
            //we could trigger MM retry mechanism by socket timeout

            log("PS resumed and broadcast resume intent");
            Intent intent = new Intent(ACTION_PS_RESUME);
            mContext.sendBroadcast(intent);
        }
    }

    public String onUpdateSpnDisplay(String plmn, int radioTechnology, int simId) {
        // for LTE
        if (radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_LTE && 
            plmn != Resources.getSystem().getText(com.android.internal.R.string.lockscreen_carrier_default).toString()){
            plmn = plmn + " 4G";
        } else if (radioTechnology > ServiceState.RIL_RADIO_TECHNOLOGY_EDGE &&
                   plmn != Resources.getSystem().getText(com.android.internal.R.string.lockscreen_carrier_default).toString()){
            plmn = plmn + " 3G";
        }
        if ("true".equals(SystemProperties.get(ITEL_PROPERTY_OPERATOR_ISROAMING[simId]))){
            String prop1 = SystemProperties.get(ITEL_PROPERTY_ICC_OPERATOR_ALPHA[simId]);
            log("getSimOperatorName simId = " + simId + " prop1 = " + prop1);
            if (prop1.equals("")){
                String prop2 = SystemProperties.get(ITEL_PROPERTY_ICC_OPERATOR_DEFAULT_NAME[simId]);
                log("getMTKdefinedSimOperatorName simId = " + simId + " prop2 = " + prop2);
                if (!prop2.equals("")){
                    plmn = plmn + "(" + prop2 + ")";
                }
            } else {
                plmn = plmn + "(" + prop1 + ")";
            }
        }
        log("Current PLMN: " + plmn);
        return plmn;
    }
    
    public int mapGsmSignalLevel(int asu,int GsmRscpQdbm){
        int level;

        if (asu <= 2 || asu == 99) level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (asu >= 12) level = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (asu >= 8)  level = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (asu >= 5)  level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else level = SignalStrength.SIGNAL_STRENGTH_POOR;

        return level;
    }

    public int mapLteSignalLevel(int mLteRsrp, int mLteRssnr, int mLteSignalStrength){        
        int rsrpIconLevel;
        
        if (mLteRsrp < -140 || mLteRsrp > -44) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (mLteRsrp >= -97) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (mLteRsrp >= -105) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (mLteRsrp >= -113) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else if (mLteRsrp >= -120) rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        else rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        Log.i(TAG,"op01_mapLteSignalLevel=" + rsrpIconLevel); 
        return rsrpIconLevel;
    }

    public int mapGsmSignalDbm(int GsmRscpQdbm,int asu){
        int dBm;

        if(GsmRscpQdbm < 0)
            dBm = GsmRscpQdbm / 4; // Return raw value for TDD 3G network.
        else
            dBm = -113 + (2 * asu);        
		
        return dBm;
    }		

    public boolean isBroadcastEmmrrsPsResume(int value) {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (value == 1) {
                Intent intent = new Intent(ACTION_EMMRRS_PS_RESUME_INDICATOR);
                mContext.sendBroadcast(intent);
                return true;
            }
        }
        return false;
    }

    public boolean needEMMRRS() {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            return true;
        } else {
            return false;
        }
    }

    public boolean needSpnRuleShowPlmnOnly() {
        return true;
    }

    //[ALPS01577029]-START:To support auto switch rat mode to 2G only for 3M TDD csfb project when we are not in china
    public int needSwitchRatModeInDesignateRegion(int mSimId,String nwPlmn){
        boolean isTdd = false;
        String basebandCapability;
        String property_name = "gsm.baseband.capability";
        String testSimProperty = "gsm.sim.ril.testsim";
        int modemType;
        int testMode = SystemProperties.getInt("gsm.gcf.testmode", 0);
        int currentNetworkMode;
        int userNetworkMode;
        boolean isTestIccCard = false;

        //[ALPS01646248]
        String simOperator = null;
        boolean isCertainSim = false;

        if (PhoneFactory.isWorldPhone()||PhoneFactory. isLteDcSupport()|| !(PhoneFactory. isLteSupport())){
            log("needSwitchRatModeInDesignateRegion()=-1 cause not 3M TDD CSFB");
            return -1;
        }

        //Get Telephony Misc Feature Config
        int miscFeatureConfig = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.TELEPHONY_MISC_FEATURE_CONFIG, 0);

        log("needSwitchRatModeInDesignateRegion: miscFeatureConfig=" + miscFeatureConfig + ",mSimId=" + mSimId);        
        if (GeminiPhone.get3GSimId() == mSimId){

            //[ALPS01646248]-Start: auto switch rat mode to 2G only when inserting non-CMCC China operator Card
            mTelephonyManagerEx = TelephonyManagerEx.getDefault();
            if (mTelephonyManagerEx != null) {
                simOperator = mTelephonyManagerEx.getSimOperator(mSimId);

                if (simOperator == null || simOperator.equals("")) {
                    Phone mPhone = null;
                    SIMRecords simRecords = null;
                    if (PhoneFactory.isGeminiSupport()) {
                        mPhone = ((PhoneProxy)(((GeminiPhone)PhoneFactory.getDefaultPhone()).getPhonebyId(mSimId))).getActivePhone();
                    } else {
                        mPhone = ((PhoneProxy)(PhoneFactory.getDefaultPhone())).getActivePhone();
                    }

                    IccRecords r = ((PhoneBase)mPhone).mIccRecords.get();
                    if (r != null) {
                        simRecords = (SIMRecords)r;
                        String imsi = (simRecords != null) ? simRecords.getIMSI() : null;
                        if (imsi != null && !imsi.equals("")) {
                            simOperator = imsi.substring(0,5);
                            Log.d(TAG,"get simOperator from IMSI = "+simOperator);
                        }
                    }
                }

                if (simOperator != null && !simOperator.equals("")){
                    for (String plmn : GSM_ONLY_PLMN_SIM) {
                        if (simOperator.equals(plmn)) {
                            isCertainSim = true;
                            break;
                        }
                    }
                } else {
                    log("needSwitchRatModeInDesignateRegion: get simOpertor return null!!");
                }
            }else{
                log("needSwitchRatModeInDesignateRegion: get instance of TelephonyManagerEx failed!!");
            }
            //[ALPS01646248]-End

            if((mSimId < PhoneConstants.GEMINI_SIM_NUM) && (SystemProperties.getInt(PROPERTY_RIL_TEST_SIM[mSimId], 0) == 1)){
                isTestIccCard = true;
            }

            if(mSimId > PhoneConstants.GEMINI_SIM_1){
                property_name = property_name + (mSimId+1) ;
            }

            basebandCapability = SystemProperties.get(property_name);
            if ((basebandCapability != null) && (!(basebandCapability.equals("")))){
                modemType = Integer.valueOf(basebandCapability);
                log("needSwitchRatModeInDesignateRegion: modemType="+modemType);

                if ((modemType & PhoneConstants.MODEM_MASK_TDSCDMA) == PhoneConstants.MODEM_MASK_TDSCDMA){
                   isTdd = true;
                }
            }

            log("isTdd: "+isTdd+",isWorldPhone:"+PhoneFactory.isWorldPhone()+",isLte: "+PhoneFactory.isLteSupport()+
                ",isLteDc: "+PhoneFactory.isLteDcSupport()+",nwPlmn: "+nwPlmn +",testMode: "+testMode +
                ",isTestIccCard: "+isTestIccCard+",isCertainSim: "+isCertainSim);

            if ((PhoneFactory.isWorldPhone() == false) && (PhoneFactory.isLteSupport() == true) && (PhoneFactory.isLteDcSupport() == false) && (isTdd == true) && (testMode == 0) && (isTestIccCard == false)){
                //[ALPS01646248]-Start: auto switch rat mode to 2G only when inserting non-CMCC China operator Card
                if((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1){
                    if (isCertainSim){
                        /* For 3M TDD CSFB project , we switch Rat Mode to 2G only when inserting non-CMCC China operator Card */
                        log("needSwitchRatModeInDesignateRegion: set Rat to 2G only when inserting non-CMCC China operator Card");                        
                        return Phone.NT_MODE_GSM_ONLY;
                    } else if (!(isInDesignateRegion("460", nwPlmn)) && !(isLabTestPlmn(nwPlmn))) {
                        /* For 3M TDD CSFB project , we switch Rat Mode to 2G only when NOT in China */
                        log("needSwitchRatModeInDesignateRegion: set Rat to 2G only when not in china");
                        return Phone.NT_MODE_GSM_ONLY;
                    } else {
                        /* For 3M TDD CSFB project , we switch Rat Mode to 4/3/2G when in China */
                        //get rat mode if user has change it
                        userNetworkMode = Settings.Global.getInt(mContext.getContentResolver(),
                                                Settings.Global.USER_PREFERRED_NETWORK_MODE, -1);

                        if (userNetworkMode >= Phone.NT_MODE_WCDMA_PREF){
                            log("needSwitchRatModeInDesignateRegion: set Rat to user preffered network mode=" + userNetworkMode);
                            return userNetworkMode;
                        } else {
                            log("needSwitchRatModeInDesignateRegion: set Rat to 4/3/2G");
                            return Phone.NT_MODE_LTE_GSM_WCDMA;
                        }
                    }
                } else {
                    log("needSwitchRatModeInDesignateRegion: EM setting off");
                }
                //[ALPS01646248]-End
            }
        }
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
            Log.i(TAG,"nwPlmn: "+nwPlmn+ "is in MCC: "+baseMcc);
            return true;
        }

        Log.i(TAG,"nwPlmn: "+nwPlmn+ "NOT in MCC: "+baseMcc);
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
    //[ALPS01577029]-END

    //[ALPS01602110] To support auto switch rat mode to 2G only for 3M TDD csfb project when inserting non-CMCC China operator Card
    public int needSwitchRatModeForCertainSIM(int mSimId,String mccmnc){
        if(mccmnc == null){
            Log.i(TAG,"Null mccmnc, cannot check sim type");
            return -1;
        }			

        //Get Telephony Misc Feature Config
        int miscFeatureConfig = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.TELEPHONY_MISC_FEATURE_CONFIG, 0);

        log("needSwitchRatModeInDesignateRegion: miscFeatureConfig=" + miscFeatureConfig + ",mSimId= " + mSimId + "RIL.get3GSimId()= "+RIL.get3GSimId());        
        if (((miscFeatureConfig & PhoneConstants.MISC_FEATURE_CONFIG_MASK_AUTO_SWITCH_RAT) == 1) &&
            (RIL.get3GSimId() == mSimId)){
            
            if((PhoneFactory.isWorldPhone()!= true) && (PhoneFactory.isLteSupport()== true) && (PhoneFactory.isLteDcSupport()!= true)){
                //remark for [ALPS01646248]
                //String[] GSM_ONLY_PLMN_SIM = {"46001", "46006", "46009", "45407", "46005", "45502"};

                Log.i(TAG,"needSwitchRatModeForCertainSIM check SIM with MCC/MNC="+mccmnc);

                for (String plmn : GSM_ONLY_PLMN_SIM) {
                    if (mccmnc.equals(plmn)) {
                        Log.i(TAG,"needSwitchRatModeForCertainSIM set 2G ONLY");
                        return Phone.NT_MODE_GSM_ONLY;
                    }
                }

                //get rat mode if user has change it
                int userNetworkMode = Settings.Global.getInt(mContext.getContentResolver(),
                                                             Settings.Global.USER_PREFERRED_NETWORK_MODE, -1);

                if (userNetworkMode >= Phone.NT_MODE_WCDMA_PREF){
                    Log.i(TAG,"needSwitchRatModeForCertainSIM: set Rat to user preffered network mode=" + userNetworkMode);
                    return userNetworkMode;
                } else {
                    Log.i(TAG,"needSwitchRatModeForCertainSIM: set Rat to 4/3/2G");
                    return Phone.NT_MODE_LTE_GSM_WCDMA;
                }
            }
        }
        return -1;
    }
    //[ALPS01602110] END

}
